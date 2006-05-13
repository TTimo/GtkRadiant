/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "renderstate.h"

#include "debugging/debugging.h"
#include "warnings.h"

#include "ishaders.h"
#include "irender.h"
#include "itextures.h"
#include "igl.h"
#include "iglrender.h"
#include "renderable.h"
#include "qerplugin.h"

#include <set>
#include <vector>
#include <list>
#include <map>

#include "math/matrix.h"
#include "math/aabb.h"
#include "generic/callback.h"
#include "texturelib.h"
#include "string/string.h"
#include "container/hashfunc.h"
#include "container/cache.h"
#include "generic/reference.h"
#include "moduleobservers.h"
#include "stream/filestream.h"
#include "stream/stringstream.h"
#include "os/file.h"
#include "preferences.h"

#include "xywindow.h"



#define DEBUG_RENDER 0

inline void debug_string(const char* string)
{
#if(DEBUG_RENDER)
  globalOutputStream() << string << "\n";
#endif
}

inline void debug_int(const char* comment, int i)
{
#if(DEBUG_RENDER)
  globalOutputStream() << comment << " " << i << "\n";
#endif
}

inline void debug_colour(const char* comment)
{
#if(DEBUG_RENDER)
  Vector4 v;
  glGetFloatv(GL_CURRENT_COLOR, reinterpret_cast<float*>(&v));
  globalOutputStream() << comment << " colour: "
    << v[0] << " "
    << v[1] << " "
    << v[2] << " "
    << v[3];
  if(glIsEnabled(GL_COLOR_ARRAY))
  {
    globalOutputStream() << " ARRAY";
  }
  if(glIsEnabled(GL_COLOR_MATERIAL))
  {
    globalOutputStream() << " MATERIAL";
  }
  globalOutputStream() << "\n";
#endif
}

#include "timer.h"

StringOutputStream g_renderer_stats;
std::size_t g_count_prims;
std::size_t g_count_states;
std::size_t g_count_transforms;
Timer g_timer;

inline void count_prim()
{
  ++g_count_prims;
}

inline void count_state()
{
  ++g_count_states;
}

inline void count_transform()
{
  ++g_count_transforms;
}

void Renderer_ResetStats()
{
  g_count_prims = 0;
  g_count_states = 0;
  g_count_transforms = 0;
  g_timer.start();
}

const char* Renderer_GetStats()
{
  g_renderer_stats.clear();
  g_renderer_stats << "prims: " << Unsigned(g_count_prims)
    << " | states: " << Unsigned(g_count_states)
    << " | transforms: " << Unsigned(g_count_transforms)
    << " | msec: " << g_timer.elapsed_msec();
  return g_renderer_stats.c_str();
}


void printShaderLog(GLhandleARB object)
{
  GLint log_length = 0;
  glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &log_length);
  
  Array<char> log(log_length);
  glGetInfoLogARB(object, log_length, &log_length, log.data());
  
  globalErrorStream() << StringRange(log.begin(), log.begin() + log_length) << "\n";
}

void createShader(GLhandleARB program, const char* filename, GLenum type)
{
  GLhandleARB shader = glCreateShaderObjectARB(type);
  GlobalOpenGL_debugAssertNoErrors();

  // load shader
  {
    std::size_t size = file_size(filename);
    FileInputStream file(filename);
    ASSERT_MESSAGE(!file.failed(), "failed to open " << makeQuoted(filename));
    Array<GLcharARB> buffer(size);
    size = file.read(reinterpret_cast<StreamBase::byte_type*>(buffer.data()), size);

    const GLcharARB* string = buffer.data();
    GLint length = GLint(size);
    glShaderSourceARB(shader, 1, &string, &length);
  }

  // compile shader
  {
    glCompileShaderARB(shader);
    
    GLint compiled = 0;
    glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if(!compiled)
    {
      printShaderLog(shader);
    }

    ASSERT_MESSAGE(compiled, "shader compile failed: " << makeQuoted(filename));
  }
  
  // attach shader
  glAttachObjectARB(program, shader);
  
  glDeleteObjectARB(shader);

  GlobalOpenGL_debugAssertNoErrors();
}

void GLSLProgram_link(GLhandleARB program)
{
  glLinkProgramARB(program);
  
  GLint linked = false;
  glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);

  if(!linked)
  {
    printShaderLog(program);
  }

  ASSERT_MESSAGE(linked, "program link failed");
}

void GLSLProgram_validate(GLhandleARB program)
{
  glValidateProgramARB(program);
  
  GLint validated = false;
  glGetObjectParameterivARB(program, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);

  if(!validated)
  {
    printShaderLog(program);
  }

  ASSERT_MESSAGE(validated, "program validation failed");
}

bool g_bumpGLSLPass_enabled = false;
bool g_depthfillPass_enabled = false;

class GLSLBumpProgram : public GLProgram
{
public:
  GLhandleARB m_program;
  qtexture_t* m_light_attenuation_xy;
  qtexture_t* m_light_attenuation_z;
  GLint u_view_origin;
  GLint u_light_origin;
  GLint u_light_color;
  GLint u_bump_scale;
  GLint u_specular_exponent;

  GLSLBumpProgram() : m_program(0), m_light_attenuation_xy(0), m_light_attenuation_z(0)
  {
  }

  void create()
  {
    // create program
    m_program = glCreateProgramObjectARB();
    
    // create shader
    {
      StringOutputStream filename(256);
      filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_vp.glsl";
      createShader(m_program, filename.c_str(), GL_VERTEX_SHADER_ARB);
      filename.clear();
      filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_fp.glsl";
      createShader(m_program, filename.c_str(), GL_FRAGMENT_SHADER_ARB);
    }

    GLSLProgram_link(m_program);
    GLSLProgram_validate(m_program);

    glUseProgramObjectARB(m_program);
    
    glBindAttribLocationARB(m_program, c_attr_TexCoord0, "attr_TexCoord0");
    glBindAttribLocationARB(m_program, c_attr_Tangent, "attr_Tangent");
    glBindAttribLocationARB(m_program, c_attr_Binormal, "attr_Binormal");

    glUniform1iARB(glGetUniformLocationARB(m_program, "u_diffusemap"), 0);
    glUniform1iARB(glGetUniformLocationARB(m_program, "u_bumpmap"), 1);
    glUniform1iARB(glGetUniformLocationARB(m_program, "u_specularmap"), 2);
    glUniform1iARB(glGetUniformLocationARB(m_program, "u_attenuationmap_xy"), 3);
    glUniform1iARB(glGetUniformLocationARB(m_program, "u_attenuationmap_z"), 4);
    
    u_view_origin = glGetUniformLocationARB(m_program, "u_view_origin");
    u_light_origin = glGetUniformLocationARB(m_program, "u_light_origin");
    u_light_color = glGetUniformLocationARB(m_program, "u_light_color");
    u_bump_scale = glGetUniformLocationARB(m_program, "u_bump_scale");
    u_specular_exponent = glGetUniformLocationARB(m_program, "u_specular_exponent");

    glUseProgramObjectARB(0);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void destroy()
  {
    glDeleteObjectARB(m_program);
    m_program = 0;
  }

  void enable()
  {
    glUseProgramObjectARB(m_program);

    glEnableVertexAttribArrayARB(c_attr_TexCoord0);
    glEnableVertexAttribArrayARB(c_attr_Tangent);
    glEnableVertexAttribArrayARB(c_attr_Binormal);

    GlobalOpenGL_debugAssertNoErrors();

    debug_string("enable bump");
    g_bumpGLSLPass_enabled = true;
  }

  void disable()
  {
    glUseProgramObjectARB(0);

    glDisableVertexAttribArrayARB(c_attr_TexCoord0);
    glDisableVertexAttribArrayARB(c_attr_Tangent);
    glDisableVertexAttribArrayARB(c_attr_Binormal);

    GlobalOpenGL_debugAssertNoErrors();

    debug_string("disable bump");
    g_bumpGLSLPass_enabled = false;
  }

  void setParameters(const Vector3& viewer, const Matrix4& localToWorld, const Vector3& origin, const Vector3& colour, const Matrix4& world2light)
  {
    Matrix4 world2local(localToWorld);
    matrix4_affine_invert(world2local);

    Vector3 localLight(origin);
    matrix4_transform_point(world2local, localLight);

    Vector3 localViewer(viewer);
    matrix4_transform_point(world2local, localViewer);

    Matrix4 local2light(world2light);
    matrix4_multiply_by_matrix4(local2light, localToWorld); // local->world->light

    glUniform3fARB(u_view_origin, localViewer.x(), localViewer.y(), localViewer.z());
    glUniform3fARB(u_light_origin, localLight.x(), localLight.y(), localLight.z());
    glUniform3fARB(u_light_color, colour.x(), colour.y(), colour.z());
    glUniform1fARB(u_bump_scale, 1.0);
    glUniform1fARB(u_specular_exponent, 32.0);

    glActiveTexture(GL_TEXTURE3);
    glClientActiveTexture(GL_TEXTURE3);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(reinterpret_cast<const float*>(&local2light));
    glMatrixMode(GL_MODELVIEW);

    GlobalOpenGL_debugAssertNoErrors();
  }
};

GLSLBumpProgram g_bumpGLSL;


class GLSLDepthFillProgram : public GLProgram
{
public:
  GLhandleARB m_program;

  void create()
  {
    // create program
    m_program = glCreateProgramObjectARB();

    // create shader
    {
      StringOutputStream filename(256);
      filename << GlobalRadiant().getAppPath() << "gl/zfill_vp.glsl";
      createShader(m_program, filename.c_str(), GL_VERTEX_SHADER_ARB);
      filename.clear();
      filename << GlobalRadiant().getAppPath() << "gl/zfill_fp.glsl";
      createShader(m_program, filename.c_str(), GL_FRAGMENT_SHADER_ARB);
    }

    GLSLProgram_link(m_program);
    GLSLProgram_validate(m_program);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void destroy()
  {
    glDeleteObjectARB(m_program);
    m_program = 0;
  }
  void enable()
  {
    glUseProgramObjectARB(m_program);
    GlobalOpenGL_debugAssertNoErrors();
    debug_string("enable depthfill");
    g_depthfillPass_enabled = true;
  }
  void disable()
  {
    glUseProgramObjectARB(0);
    GlobalOpenGL_debugAssertNoErrors();
    debug_string("disable depthfill");
    g_depthfillPass_enabled = false;
  }
  void setParameters(const Vector3& viewer, const Matrix4& localToWorld, const Vector3& origin, const Vector3& colour, const Matrix4& world2light)
  {
  }
};

GLSLDepthFillProgram g_depthFillGLSL;


// ARB path

void createProgram(const char* filename, GLenum type)
{
  std::size_t size = file_size(filename);
  FileInputStream file(filename);
  ASSERT_MESSAGE(!file.failed(), "failed to open " << makeQuoted(filename));
  Array<GLcharARB> buffer(size);
  size = file.read(reinterpret_cast<StreamBase::byte_type*>(buffer.data()), size);

  glProgramStringARB(type, GL_PROGRAM_FORMAT_ASCII_ARB, GLsizei(size), buffer.data());

  if(GL_INVALID_OPERATION == glGetError())
  {
    GLint errPos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
    const GLubyte* errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

    globalErrorStream() << reinterpret_cast<const char*>(filename) << ":" <<  errPos << "\n" << reinterpret_cast<const char*>(errString);

    ERROR_MESSAGE("error in gl program");
  }
}

class ARBBumpProgram : public GLProgram
{
public:
  GLuint m_vertex_program;
  GLuint m_fragment_program;

  void create()
  {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    {
      glGenProgramsARB(1, &m_vertex_program);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vertex_program);
      StringOutputStream filename(256);
      filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_vp.glp";
      createProgram(filename.c_str(), GL_VERTEX_PROGRAM_ARB);

      glGenProgramsARB(1, &m_fragment_program);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fragment_program);
      filename.clear();
      filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_fp.glp";
      createProgram(filename.c_str(), GL_FRAGMENT_PROGRAM_ARB);
    }

    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void destroy()
  {
    glDeleteProgramsARB(1, &m_vertex_program);
    glDeleteProgramsARB(1, &m_fragment_program);
    GlobalOpenGL_debugAssertNoErrors();
  }

  void enable()
  {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vertex_program);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fragment_program);

    glEnableVertexAttribArrayARB(8);
    glEnableVertexAttribArrayARB(9);
    glEnableVertexAttribArrayARB(10);
    glEnableVertexAttribArrayARB(11);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void disable()
  {
    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    glDisableVertexAttribArrayARB(8);
    glDisableVertexAttribArrayARB(9);
    glDisableVertexAttribArrayARB(10);
    glDisableVertexAttribArrayARB(11);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void setParameters(const Vector3& viewer, const Matrix4& localToWorld, const Vector3& origin, const Vector3& colour, const Matrix4& world2light)
  {
    Matrix4 world2local(localToWorld);
    matrix4_affine_invert(world2local);

    Vector3 localLight(origin);
    matrix4_transform_point(world2local, localLight);

    Vector3 localViewer(viewer);
    matrix4_transform_point(world2local, localViewer);

    Matrix4 local2light(world2light);
    matrix4_multiply_by_matrix4(local2light, localToWorld); // local->world->light

    // view origin
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, localViewer.x(), localViewer.y(), localViewer.z(), 0);

    // light origin
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, localLight.x(), localLight.y(), localLight.z(), 1);

    // light colour
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, colour.x(), colour.y(), colour.z(), 0);

    // bump scale
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, 1, 0, 0, 0);

    // specular exponent
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 5, 32, 0, 0, 0);


    glActiveTexture(GL_TEXTURE3);
    glClientActiveTexture(GL_TEXTURE3);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(reinterpret_cast<const float*>(&local2light));
    glMatrixMode(GL_MODELVIEW);

    GlobalOpenGL_debugAssertNoErrors();
  }
};

class ARBDepthFillProgram : public GLProgram
{
public:
  GLuint m_vertex_program;
  GLuint m_fragment_program;

  void create()
  {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    {
      glGenProgramsARB(1, &m_vertex_program);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vertex_program);
      StringOutputStream filename(256);
      filename << GlobalRadiant().getAppPath() << "gl/zfill_vp.glp";
      createProgram(filename.c_str(), GL_VERTEX_PROGRAM_ARB);

      glGenProgramsARB(1, &m_fragment_program);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fragment_program);
      filename.clear();
      filename << GlobalRadiant().getAppPath() << "gl/zfill_fp.glp";
      createProgram(filename.c_str(), GL_FRAGMENT_PROGRAM_ARB);
    }

    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void destroy()
  {
    glDeleteProgramsARB(1, &m_vertex_program);
    glDeleteProgramsARB(1, &m_fragment_program);
    GlobalOpenGL_debugAssertNoErrors();
  }

  void enable()
  {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vertex_program);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fragment_program);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void disable()
  {
    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    GlobalOpenGL_debugAssertNoErrors();
  }

  void setParameters(const Vector3& viewer, const Matrix4& localToWorld, const Vector3& origin, const Vector3& colour, const Matrix4& world2light)
  {
  }
};

ARBBumpProgram g_bumpARB;
ARBDepthFillProgram g_depthFillARB;


#if 0
// NV20 path (unfinished)

void createProgram(GLint program, const char* filename, GLenum type)
{
  std::size_t size = file_size(filename);
  FileInputStream file(filename);
  ASSERT_MESSAGE(!file.failed(), "failed to open " << makeQuoted(filename));
  Array<GLubyte> buffer(size);
  size = file.read(reinterpret_cast<StreamBase::byte_type*>(buffer.data()), size);

  glLoadProgramNV(type, program, GLsizei(size), buffer.data());

  if(GL_INVALID_OPERATION == glGetError())
  {
    GLint errPos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &errPos);
    const GLubyte* errString = glGetString(GL_PROGRAM_ERROR_STRING_NV);

    globalErrorStream() << filename << ":" <<  errPos << "\n" << errString;

    ERROR_MESSAGE("error in gl program");
  }
}

GLuint m_vertex_program;
GLuint m_fragment_program;
qtexture_t* g_cube = 0;
qtexture_t* g_specular_lookup = 0;
qtexture_t* g_attenuation_xy = 0;
qtexture_t* g_attenuation_z = 0;

void createVertexProgram()
{
  {
    glGenProgramsNV(1, &m_vertex_program);
    glBindProgramNV(GL_VERTEX_PROGRAM_NV, m_vertex_program);
    StringOutputStream filename(256);
    filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_vp.nv30";
    createProgram(m_vertex_program, filename.c_str(), GL_VERTEX_PROGRAM_NV);

    glGenProgramsNV(1, &m_fragment_program);
    glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, m_fragment_program);
    filename.clear();
    filename << GlobalRadiant().getAppPath() << "gl/lighting_DBS_omni_fp.nv30";
    createProgram(m_fragment_program, filename.c_str(), GL_FRAGMENT_PROGRAM_NV);
  }

  g_cube = GlobalTexturesCache().capture("generated/cube");
  g_specular_lookup = GlobalTexturesCache().capture("generated/specular");

  g_attenuation_xy = GlobalTexturesCache().capture("lights/squarelight1");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_attenuation_xy->texture_number);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  g_attenuation_z = GlobalTexturesCache().capture("lights/squarelight1a");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_attenuation_z->texture_number);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GlobalOpenGL_debugAssertNoErrors();
}

void destroyVertexProgram()
{
  glDeleteProgramsNV(1, &m_vertex_program);
  glDeleteProgramsNV(1, &m_fragment_program);
  GlobalOpenGL_debugAssertNoErrors();

  GlobalTexturesCache().release(g_cube);
  GlobalTexturesCache().release(g_specular_lookup);
  GlobalTexturesCache().release(g_attenuation_xy);
  GlobalTexturesCache().release(g_attenuation_z);
}

bool g_vertexProgram_enabled = false;

void enableVertexProgram()
{
	//set up the register combiners
	//two general combiners
	glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 2);

	//combiner 0 does tex0+tex1 -> spare0
	glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
						GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO,
						GL_UNSIGNED_INVERT_NV, GL_RGB);
	glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_TEXTURE1_ARB,
						GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO,
						GL_UNSIGNED_INVERT_NV, GL_RGB);
	glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_SPARE0_NV,
						GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);

	//combiner 1 does tex2 dot tex3 -> spare1
	glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE2_ARB,
						GL_EXPAND_NORMAL_NV, GL_RGB);
	glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE3_ARB,
						GL_EXPAND_NORMAL_NV, GL_RGB);
	glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB, GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV,
						GL_NONE, GL_NONE, GL_TRUE, GL_FALSE, GL_FALSE);



	//final combiner outputs (1-spare0)*constant color 0*spare1
	//do constant color 0*spare1 in the EF multiplier
	glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	
	//now do (1-spare0)*EF
	glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);

  glEnable(GL_VERTEX_PROGRAM_NV);
	glEnable(GL_REGISTER_COMBINERS_NV);
  glBindProgramNV(GL_VERTEX_PROGRAM_NV, m_vertex_program);
  glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, m_fragment_program);

  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_1D);
  glActiveTexture(GL_TEXTURE2);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE3);
  glEnable(GL_TEXTURE_2D);

  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY10_NV);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY11_NV);

  GlobalOpenGL_debugAssertNoErrors();
  g_vertexProgram_enabled = true;
}

void disableVertexProgram()
{
  glDisable(GL_VERTEX_PROGRAM_NV);
  glDisable(GL_REGISTER_COMBINERS_NV);

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_1D);
  glActiveTexture(GL_TEXTURE2);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE3);
  glDisable(GL_TEXTURE_2D);

  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);
  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);
  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY10_NV);
  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY11_NV);

  GlobalOpenGL_debugAssertNoErrors();
  g_vertexProgram_enabled = false;
}

class GLstringNV
{
public:
  const GLubyte* m_string;
  const GLint m_length;
  GLstringNV(const char* string) : m_string(reinterpret_cast<const GLubyte*>(string)), m_length(GLint(string_length(string)))
  {
  }
};

GLstringNV g_light_origin("light_origin");
GLstringNV g_view_origin("view_origin");
GLstringNV g_light_color("light_color");
GLstringNV g_bumpGLSL_scale("bump_scale");
GLstringNV g_specular_exponent("specular_exponent");

void setVertexProgramEnvironment(const Vector3& localViewer)
{
  Matrix4 local2light(g_matrix4_identity);
  matrix4_translate_by_vec3(local2light, Vector3(0.5, 0.5, 0.5));
  matrix4_scale_by_vec3(local2light, Vector3(0.5, 0.5, 0.5));
  matrix4_scale_by_vec3(local2light, Vector3(1.0 / 512.0, 1.0 / 512.0, 1.0 / 512.0));
  matrix4_translate_by_vec3(local2light, vector3_negated(localViewer));

  glActiveTexture(GL_TEXTURE3);
  glClientActiveTexture(GL_TEXTURE3);

  glMatrixMode(GL_TEXTURE);
  glLoadMatrixf(reinterpret_cast<const float*>(&local2light));
  glMatrixMode(GL_MODELVIEW);

  glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
  glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);

  // view origin
  //qglProgramNamedParameter4fNV(m_fragment_program, g_view_origin.m_length, g_view_origin.m_string, localViewer.x(), localViewer.y(), localViewer.z(), 0);

  // light origin
  glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 8, localViewer.x(), localViewer.y(), localViewer.z(), 1.0f);

  // light colour
  glCombinerParameterfNV(GL_CONSTANT_COLOR0_NV, 1, 1, 1, 1)

  // bump scale
  //qglProgramNamedParameter4fNV(m_fragment_program, g_bumpGLSL_scale.m_length, g_bumpGLSL_scale.m_string, 1, 0, 0, 0);

  // specular exponent
  //qglProgramNamedParameter4fNV(m_fragment_program, g_specular_exponent.m_length, g_specular_exponent.m_string, 32, 0, 0, 0);

  GlobalOpenGL_debugAssertNoErrors();
}

#endif


bool g_vertexArray_enabled = false;
bool g_normalArray_enabled = false;
bool g_texcoordArray_enabled = false;
bool g_colorArray_enabled = false;

inline bool OpenGLState_less(const OpenGLState& self, const OpenGLState& other)
{
  //! Sort by sort-order override.
  if(self.m_sort != other.m_sort)
  {
    return self.m_sort < other.m_sort;
  }
  //! Sort by texture handle.
  if(self.m_texture != other.m_texture)
  {
    return self.m_texture < other.m_texture;
  }
  if(self.m_texture1 != other.m_texture1)
  {
    return self.m_texture1 < other.m_texture1;
  }
  if(self.m_texture2 != other.m_texture2)
  {
    return self.m_texture2 < other.m_texture2;
  }
  if(self.m_texture3 != other.m_texture3)
  {
    return self.m_texture3 < other.m_texture3;
  }
  if(self.m_texture4 != other.m_texture4)
  {
    return self.m_texture4 < other.m_texture4;
  }
  if(self.m_texture5 != other.m_texture5)
  {
    return self.m_texture5 < other.m_texture5;
  }
  if(self.m_texture6 != other.m_texture6)
  {
    return self.m_texture6 < other.m_texture6;
  }
  if(self.m_texture7 != other.m_texture7)
  {
    return self.m_texture7 < other.m_texture7;
  }
  //! Sort by state bit-vector.
  if(self.m_state != other.m_state)
  {
    return self.m_state < other.m_state;
  }
  //! Comparing address makes sure states are never equal.
  return &self < &other;
}

void OpenGLState_constructDefault(OpenGLState& state)
{
  state.m_state = RENDER_DEFAULT;

  state.m_texture = 0;
  state.m_texture1 = 0;
  state.m_texture2 = 0;
  state.m_texture3 = 0;
  state.m_texture4 = 0;
  state.m_texture5 = 0;
  state.m_texture6 = 0;
  state.m_texture7 = 0;

  state.m_colour[0] = 1;
  state.m_colour[1] = 1;
  state.m_colour[2] = 1;
  state.m_colour[3] = 1;

  state.m_depthfunc = GL_LESS;

  state.m_blend_src = GL_SRC_ALPHA;
  state.m_blend_dst = GL_ONE_MINUS_SRC_ALPHA;

  state.m_alphafunc = GL_ALWAYS;
  state.m_alpharef = 0;

  state.m_linewidth = 1;
  state.m_pointsize = 1;

  state.m_linestipple_factor = 1;
  state.m_linestipple_pattern = 0xaaaa;

  state.m_fog = OpenGLFogState();
}




/// \brief A container of Renderable references.
/// May contain the same Renderable multiple times, with different transforms.
class OpenGLStateBucket
{
public:
  struct RenderTransform
  {
    const Matrix4* m_transform;
    const OpenGLRenderable *m_renderable;
    const RendererLight* m_light;

    RenderTransform(const OpenGLRenderable& renderable, const Matrix4& transform, const RendererLight* light)
      : m_transform(&transform), m_renderable(&renderable), m_light(light)
    {
    }
  };

  typedef std::vector<RenderTransform> Renderables;

private:

  OpenGLState m_state;
  Renderables m_renderables;

public:
  OpenGLStateBucket()
  {
  }
  void addRenderable(const OpenGLRenderable& renderable, const Matrix4& modelview, const RendererLight* light = 0)
  {
    m_renderables.push_back(RenderTransform(renderable, modelview, light));
  }

  OpenGLState& state()
  {
    return m_state;
  }

  void render(OpenGLState& current, unsigned int globalstate, const Vector3& viewer);
};

#define LIGHT_SHADER_DEBUG 0

#if LIGHT_SHADER_DEBUG
typedef std::vector<Shader*> LightDebugShaders;
LightDebugShaders g_lightDebugShaders;
#endif

class OpenGLStateLess
{
public:
  bool operator()(const OpenGLState& self, const OpenGLState& other) const
  {
    return OpenGLState_less(self, other);
  }
};

typedef ConstReference<OpenGLState> OpenGLStateReference;
typedef std::map<OpenGLStateReference, OpenGLStateBucket*, OpenGLStateLess> OpenGLStates;
OpenGLStates g_state_sorted;

class OpenGLStateBucketAdd
{
  OpenGLStateBucket& m_bucket;
  const OpenGLRenderable& m_renderable;
  const Matrix4& m_modelview;
public:
  typedef const RendererLight& first_argument_type;

  OpenGLStateBucketAdd(OpenGLStateBucket& bucket, const OpenGLRenderable& renderable, const Matrix4& modelview) :
    m_bucket(bucket), m_renderable(renderable), m_modelview(modelview)
  {
  }
  void operator()(const RendererLight& light)
  {
    m_bucket.addRenderable(m_renderable, m_modelview, &light);
  }
};

class CountLights
{
  std::size_t m_count;
public:
  typedef RendererLight& first_argument_type;

  CountLights() : m_count(0)
  {
  }
  void operator()(const RendererLight& light)
  {
    ++m_count;
  }
  std::size_t count() const
  {
    return m_count;
  }
};

class OpenGLShader : public Shader
{
  typedef std::list<OpenGLStateBucket*> Passes;
  Passes m_passes;
  IShader* m_shader;
  std::size_t m_used;
  ModuleObservers m_observers;
public:
  OpenGLShader() : m_shader(0), m_used(0)
  {
  }
  ~OpenGLShader()
  {
  }
  void construct(const char* name);
  void destroy()
  {
    if(m_shader)
    {
      m_shader->DecRef();
    }
    m_shader = 0;

    for(Passes::iterator i = m_passes.begin(); i != m_passes.end(); ++i)
    {
      delete *i;
    }
    m_passes.clear();
  }
  void addRenderable(const OpenGLRenderable& renderable, const Matrix4& modelview, const LightList* lights)
  {
    for(Passes::iterator i = m_passes.begin(); i != m_passes.end(); ++i)
    {
#if LIGHT_SHADER_DEBUG
      if(((*i)->state().m_state & RENDER_BUMP) != 0)
      {
        if(lights != 0)
        {
          CountLights counter;
          lights->forEachLight(makeCallback1(counter));
          globalOutputStream() << "count = " << counter.count() << "\n";
          for(std::size_t i = 0; i < counter.count(); ++i)
          {
            g_lightDebugShaders[counter.count()]->addRenderable(renderable, modelview);
          }
        }
      }
      else
#else
      if(((*i)->state().m_state & RENDER_BUMP) != 0)
      {
        if(lights != 0)
        {
          OpenGLStateBucketAdd add(*(*i), renderable, modelview);
          lights->forEachLight(makeCallback1(add));
        }
      }
      else
#endif
      {
        (*i)->addRenderable(renderable, modelview);
      }
    }
  }
  void incrementUsed()
  {
    if(++m_used == 1 && m_shader != 0)
    { 
      m_shader->SetInUse(true);
    }
  }
  void decrementUsed()
  {
    if(--m_used == 0 && m_shader != 0)
    {
      m_shader->SetInUse(false);
    }
  }
  bool realised() const
  {
    return m_shader != 0;
  }
  void attach(ModuleObserver& observer)
  {
    if(realised())
    {
      observer.realise();
    }
    m_observers.attach(observer);
  }
  void detach(ModuleObserver& observer)
  {
    if(realised())
    {
      observer.unrealise();
    }
    m_observers.detach(observer);
  }
  void realise(const CopiedString& name)
  {
    construct(name.c_str());

    if(m_used != 0 && m_shader != 0)
    {
      m_shader->SetInUse(true);
    }
    
    for(Passes::iterator i = m_passes.begin(); i != m_passes.end(); ++i)
    {
      g_state_sorted.insert(OpenGLStates::value_type(OpenGLStateReference((*i)->state()), *i));
    }

    m_observers.realise();
  }
  void unrealise()
  {
    m_observers.unrealise();

    for(Passes::iterator i = m_passes.begin(); i != m_passes.end(); ++i)
    {
      g_state_sorted.erase(OpenGLStateReference((*i)->state()));
    }

    destroy();
  }
  qtexture_t& getTexture() const
  {
    ASSERT_NOTNULL(m_shader);
    return *m_shader->getTexture();
  }
  unsigned int getFlags() const
  {
    ASSERT_NOTNULL(m_shader);
    return m_shader->getFlags();
  }
  IShader& getShader() const
  {
    ASSERT_NOTNULL(m_shader);
    return *m_shader;
  }
  OpenGLState& appendDefaultPass()
  {
    m_passes.push_back(new OpenGLStateBucket);
    OpenGLState& state = m_passes.back()->state();
    OpenGLState_constructDefault(state);
    return state;
  }
};


inline bool lightEnabled(const RendererLight& light, const LightCullable& cullable)
{
  return cullable.testLight(light);
}

typedef std::set<RendererLight*> RendererLights;

#define DEBUG_LIGHT_SYNC 0

class LinearLightList : public LightList
{
  LightCullable& m_cullable;
  RendererLights& m_allLights;
  Callback m_evaluateChanged;

  typedef std::list<RendererLight*> Lights;
  mutable Lights m_lights;
  mutable bool m_lightsChanged;
public:
  LinearLightList(LightCullable& cullable, RendererLights& lights, const Callback& evaluateChanged) :
    m_cullable(cullable), m_allLights(lights), m_evaluateChanged(evaluateChanged)
  {
    m_lightsChanged = true;
  }
  void evaluateLights() const
  {
    m_evaluateChanged();
    if(m_lightsChanged)
    {
      m_lightsChanged = false;

      m_lights.clear();
      m_cullable.clearLights();
      for(RendererLights::const_iterator i = m_allLights.begin(); i != m_allLights.end(); ++i)
      {
        if(lightEnabled(*(*i), m_cullable))
        {
          m_lights.push_back(*i);
          m_cullable.insertLight(*(*i));
        }
      }
    }
#if(DEBUG_LIGHT_SYNC)
    else
    {
      Lights lights;
      for(RendererLights::const_iterator i = m_allLights.begin(); i != m_allLights.end(); ++i)
      {
        if(lightEnabled(*(*i), m_cullable))
        {
          lights.push_back(*i);
        }
      }
      ASSERT_MESSAGE(
        !std::lexicographical_compare(lights.begin(), lights.end(), m_lights.begin(), m_lights.end())
        && !std::lexicographical_compare(m_lights.begin(), m_lights.end(), lights.begin(), lights.end()),
        "lights out of sync"
      );
    }
#endif
  }
  void forEachLight(const RendererLightCallback& callback) const
  {
    evaluateLights();

    for(Lights::const_iterator i = m_lights.begin(); i != m_lights.end(); ++i)
    {
      callback(*(*i));
    }
  }
  void lightsChanged() const
  {
    m_lightsChanged = true;
  }
};

inline void setFogState(const OpenGLFogState& state)
{
	glFogi(GL_FOG_MODE, state.mode);
	glFogf(GL_FOG_DENSITY, state.density);
	glFogf(GL_FOG_START, state.start);
	glFogf(GL_FOG_END, state.end);
	glFogi(GL_FOG_INDEX, state.index);
	glFogfv(GL_FOG_COLOR, vector4_to_array(state.colour));
}

#define DEBUG_SHADERS 0

class OpenGLShaderCache : public ShaderCache, public TexturesCacheObserver, public ModuleObserver
{
  class CreateOpenGLShader
  {
    OpenGLShaderCache* m_cache;
  public:
    explicit CreateOpenGLShader(OpenGLShaderCache* cache = 0)
      : m_cache(cache)
    {
    }
    OpenGLShader* construct(const CopiedString& name)
    {
      OpenGLShader* shader = new OpenGLShader;
      if(m_cache->realised())
      {
        shader->realise(name);
      }
      return shader;
    }
    void destroy(OpenGLShader* shader)
    {
      if(m_cache->realised())
      {
        shader->unrealise();
      }
      delete shader;
    }
  };

  typedef HashedCache<CopiedString, OpenGLShader, HashString, std::equal_to<CopiedString>, CreateOpenGLShader> Shaders;
  Shaders m_shaders;
  std::size_t m_unrealised;

  bool m_lightingEnabled;
  bool m_lightingSupported;
  bool m_useShaderLanguage;

public:
  OpenGLShaderCache()
    : m_shaders(CreateOpenGLShader(this)),
    m_unrealised(3), // wait until shaders, gl-context and textures are realised before creating any render-states
    m_lightingEnabled(true),
    m_lightingSupported(false),
    m_useShaderLanguage(false),
    m_lightsChanged(true),
    m_traverseRenderablesMutex(false)
  {
  }
  ~OpenGLShaderCache()
  {
    for(Shaders::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i)
    {
      globalOutputStream() << "leaked shader: " << makeQuoted((*i).key.c_str()) << "\n";
    }
  }
  Shader* capture(const char* name)
  {
    ASSERT_MESSAGE(name[0] == '$'
      || *name == '['
      || *name == '<'
      || *name == '('
      || strchr(name, '\\') == 0, "shader name contains invalid characters: \"" << name << "\""); 
#if DEBUG_SHADERS
    globalOutputStream() << "shaders capture: " << makeQuoted(name) << '\n';
#endif
    return m_shaders.capture(name).get();
  }
  void release(const char *name)
  {
#if DEBUG_SHADERS
    globalOutputStream() << "shaders release: " << makeQuoted(name) << '\n';
#endif
    m_shaders.release(name);
  }
  void render(RenderStateFlags globalstate, const Matrix4& modelview, const Matrix4& projection, const Vector3& viewer)
  {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<const float*>(&projection));
  #if 0
    //qglGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<float*>(&projection));
  #endif

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(reinterpret_cast<const float*>(&modelview));
  #if 0
    //qglGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<float*>(&modelview));
  #endif
 
    ASSERT_MESSAGE(realised(), "render states are not realised");

    // global settings that are not set in renderstates
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glPolygonOffset(-1, 1);
    {
      const GLubyte pattern[132] = {
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
 	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
 	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
 	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
 	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55
     };
      glPolygonStipple(pattern);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    g_vertexArray_enabled = true;
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    if(GlobalOpenGL().GL_1_3())
    {
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
    }

    if(GlobalOpenGL().ARB_shader_objects())
    {
      glUseProgramObjectARB(0);
      glDisableVertexAttribArrayARB(c_attr_TexCoord0);
      glDisableVertexAttribArrayARB(c_attr_Tangent);
      glDisableVertexAttribArrayARB(c_attr_Binormal);
    }

    if(globalstate & RENDER_TEXTURE)
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLState current;
    OpenGLState_constructDefault(current);
    current.m_sort = OpenGLState::eSortFirst;

    // default renderstate settings
    glLineStipple(current.m_linestipple_factor, current.m_linestipple_pattern);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    g_texcoordArray_enabled = false;
    glDisableClientState(GL_COLOR_ARRAY);
    g_colorArray_enabled = false;
    glDisableClientState(GL_NORMAL_ARRAY);
    g_normalArray_enabled = false;
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_POLYGON_OFFSET_LINE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4f(1,1,1,1);
    glDepthFunc(GL_LESS);
    glAlphaFunc(GL_ALWAYS, 0);
    glLineWidth(1);
    glPointSize(1);

	  glHint(GL_FOG_HINT, GL_NICEST);
    glDisable(GL_FOG);
    setFogState(OpenGLFogState());

    GlobalOpenGL_debugAssertNoErrors();

    debug_string("begin rendering");
    for(OpenGLStates::iterator i = g_state_sorted.begin(); i != g_state_sorted.end(); ++i)
    {
      (*i).second->render(current, globalstate, viewer);
    }
    debug_string("end rendering");
  }
  void realise()
  {
    if(--m_unrealised == 0)
    {
      if(lightingSupported() && lightingEnabled())
      {
        if(useShaderLanguage())
        {
          g_bumpGLSL.create();
          g_depthFillGLSL.create();
        }
        else
        {
          g_bumpARB.create();
          g_depthFillARB.create();
        }
      }

      for(Shaders::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i)
      {
        if(!(*i).value.empty())
        {
          (*i).value->realise(i->key);
        }
      }
    }
  }
  void unrealise()
  {
    if(++m_unrealised == 1)
    {
      for(Shaders::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i)
      {
        if(!(*i).value.empty())
        {
          (*i).value->unrealise();
        }
      }
      if(GlobalOpenGL().contextValid && lightingSupported() && lightingEnabled())
      {
        if(useShaderLanguage())
        {
          g_bumpGLSL.destroy();
          g_depthFillGLSL.destroy();
        }
        else
        {
          g_bumpARB.destroy();
          g_depthFillARB.destroy();
        }
      }
    }
  }
  bool realised()
  {
    return m_unrealised == 0;
  }


  bool lightingEnabled() const
  {
    return m_lightingEnabled;
  }
  bool lightingSupported() const
  {
    return m_lightingSupported;
  }
  bool useShaderLanguage() const
  {
    return m_useShaderLanguage;
  }
  void setLighting(bool supported, bool enabled)
  {
    bool refresh = (m_lightingSupported && m_lightingEnabled) != (supported && enabled);

    if(refresh)
    {
      unrealise();
      GlobalShaderSystem().setLightingEnabled(supported && enabled);
    }

    m_lightingSupported = supported;
    m_lightingEnabled = enabled;

    if(refresh)
    {
      realise();
    }
  }
  void extensionsInitialised()
  {
    setLighting(GlobalOpenGL().GL_1_3()
      && GlobalOpenGL().ARB_vertex_program()
      && GlobalOpenGL().ARB_fragment_program()
      && GlobalOpenGL().ARB_shader_objects()
      && GlobalOpenGL().ARB_vertex_shader()
      && GlobalOpenGL().ARB_fragment_shader()
      && GlobalOpenGL().ARB_shading_language_100(),
      m_lightingEnabled
    );

    if(!lightingSupported())
    {
      globalOutputStream() << "Lighting mode requires OpenGL features not supported by your graphics drivers:\n";
      if(!GlobalOpenGL().GL_1_3())
      {
        globalOutputStream() << "  GL version 1.3 or better\n";
      }
      if(!GlobalOpenGL().ARB_vertex_program())
      {
        globalOutputStream() << "  GL_ARB_vertex_program\n";
      }
      if(!GlobalOpenGL().ARB_fragment_program())
      {
        globalOutputStream() << "  GL_ARB_fragment_program\n";
      }
      if(!GlobalOpenGL().ARB_shader_objects())
      {
        globalOutputStream() << "  GL_ARB_shader_objects\n";
      }
      if(!GlobalOpenGL().ARB_vertex_shader())
      {
        globalOutputStream() << "  GL_ARB_vertex_shader\n";
      }
      if(!GlobalOpenGL().ARB_fragment_shader())
      {
        globalOutputStream() << "  GL_ARB_fragment_shader\n";
      }
      if(!GlobalOpenGL().ARB_shading_language_100())
      {
        globalOutputStream() << "  GL_ARB_shading_language_100\n";
      }
    }
  }
  void setLightingEnabled(bool enabled)
  {
    setLighting(m_lightingSupported, enabled);
  }

  // light culling

  RendererLights m_lights;
  bool m_lightsChanged;
  typedef std::map<LightCullable*, LinearLightList> LightLists;
  LightLists m_lightLists;

  const LightList& attach(LightCullable& cullable)
  {
    return (*m_lightLists.insert(LightLists::value_type(&cullable, LinearLightList(cullable, m_lights, EvaluateChangedCaller(*this)))).first).second;
  }
  void detach(LightCullable& cullable)
  {
    m_lightLists.erase(&cullable);
  }
  void changed(LightCullable& cullable)
  {
    LightLists::iterator i = m_lightLists.find(&cullable);
    ASSERT_MESSAGE(i != m_lightLists.end(), "cullable not attached");
    (*i).second.lightsChanged();
  }
  void attach(RendererLight& light)
  {
    ASSERT_MESSAGE(m_lights.find(&light) == m_lights.end(), "light could not be attached");
    m_lights.insert(&light);
    changed(light);
  }
  void detach(RendererLight& light)
  {
    ASSERT_MESSAGE(m_lights.find(&light) != m_lights.end(), "light could not be detached");
    m_lights.erase(&light);
    changed(light);
  }
  void changed(RendererLight& light)
  {
    m_lightsChanged = true;
  }
  void evaluateChanged()
  {
    if(m_lightsChanged)
    {
      m_lightsChanged = false;
      for(LightLists::iterator i = m_lightLists.begin(); i != m_lightLists.end(); ++i)
      {
        (*i).second.lightsChanged();
      }
    }
  }
  typedef MemberCaller<OpenGLShaderCache, &OpenGLShaderCache::evaluateChanged> EvaluateChangedCaller;

  typedef std::set<const Renderable*> Renderables; 
  Renderables m_renderables;
  mutable bool m_traverseRenderablesMutex;

  // renderables
  void attachRenderable(const Renderable& renderable)
  {
    ASSERT_MESSAGE(!m_traverseRenderablesMutex, "attaching renderable during traversal");
    ASSERT_MESSAGE(m_renderables.find(&renderable) == m_renderables.end(), "renderable could not be attached");
    m_renderables.insert(&renderable);
  }
  void detachRenderable(const Renderable& renderable)
  {
    ASSERT_MESSAGE(!m_traverseRenderablesMutex, "detaching renderable during traversal");
    ASSERT_MESSAGE(m_renderables.find(&renderable) != m_renderables.end(), "renderable could not be detached");
    m_renderables.erase(&renderable);
  }
  void forEachRenderable(const RenderableCallback& callback) const
  {
    ASSERT_MESSAGE(!m_traverseRenderablesMutex, "for-each during traversal");
    m_traverseRenderablesMutex = true;
    for(Renderables::const_iterator i = m_renderables.begin(); i != m_renderables.end(); ++i)
    {
      callback(*(*i));
    }
    m_traverseRenderablesMutex = false;
  }
};

static OpenGLShaderCache* g_ShaderCache;

void ShaderCache_extensionsInitialised()
{
  g_ShaderCache->extensionsInitialised();
}

void ShaderCache_setBumpEnabled(bool enabled)
{
  g_ShaderCache->setLightingEnabled(enabled);
}


Vector3 g_DebugShaderColours[256];
Shader* g_defaultPointLight = 0;

void ShaderCache_Construct()
{
  g_ShaderCache = new OpenGLShaderCache;
  GlobalTexturesCache().attach(*g_ShaderCache);
  GlobalShaderSystem().attach(*g_ShaderCache);

  if(g_pGameDescription->mGameType == "doom3")
  {
    g_defaultPointLight = g_ShaderCache->capture("lights/defaultPointLight");
    //Shader* overbright =
    g_ShaderCache->capture("$OVERBRIGHT");

#if LIGHT_SHADER_DEBUG
    for(std::size_t i = 0; i < 256; ++i)
    {
      g_DebugShaderColours[i] = Vector3(i / 256.0, i / 256.0, i / 256.0);
    }

    g_DebugShaderColours[0] = Vector3(1, 0, 0);
    g_DebugShaderColours[1] = Vector3(1, 0.5, 0);
    g_DebugShaderColours[2] = Vector3(1, 1, 0);
    g_DebugShaderColours[3] = Vector3(0.5, 1, 0);
    g_DebugShaderColours[4] = Vector3(0, 1, 0);
    g_DebugShaderColours[5] = Vector3(0, 1, 0.5);
    g_DebugShaderColours[6] = Vector3(0, 1, 1);
    g_DebugShaderColours[7] = Vector3(0, 0.5, 1);
    g_DebugShaderColours[8] = Vector3(0, 0, 1);
    g_DebugShaderColours[9] = Vector3(0.5, 0, 1);
    g_DebugShaderColours[10] = Vector3(1, 0, 1);
    g_DebugShaderColours[11] = Vector3(1, 0, 0.5);

    g_lightDebugShaders.reserve(256);
    StringOutputStream buffer(256);
    for(std::size_t i = 0; i < 256; ++i)
    {
      buffer << "(" << g_DebugShaderColours[i].x() << " " << g_DebugShaderColours[i].y() << " " << g_DebugShaderColours[i].z() << ")";
      g_lightDebugShaders.push_back(g_ShaderCache->capture(buffer.c_str()));
      buffer.clear();
    }
#endif
  }
}

void ShaderCache_Destroy()
{
  if(g_pGameDescription->mGameType == "doom3")
  {
    g_ShaderCache->release("lights/defaultPointLight");
    g_ShaderCache->release("$OVERBRIGHT");
    g_defaultPointLight = 0;

#if LIGHT_SHADER_DEBUG
    g_lightDebugShaders.clear();
    StringOutputStream buffer(256);
    for(std::size_t i = 0; i < 256; ++i)
    {
      buffer << "(" << g_DebugShaderColours[i].x() << " " << g_DebugShaderColours[i].y() << " " << g_DebugShaderColours[i].z() << ")";
      g_ShaderCache->release(buffer.c_str());
    }
#endif
  }

  GlobalShaderSystem().detach(*g_ShaderCache);
  GlobalTexturesCache().detach(*g_ShaderCache);
  delete g_ShaderCache;
}

ShaderCache* GetShaderCache()
{
  return g_ShaderCache;
}

inline void setTextureState(GLint& current, const GLint& texture, GLenum textureUnit)
{
  if(texture != current)
  {
    glActiveTexture(textureUnit);
    glClientActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);
    GlobalOpenGL_debugAssertNoErrors();
    current = texture;
  }
}

inline void setTextureState(GLint& current, const GLint& texture)
{
  if(texture != current)
  {
    glBindTexture(GL_TEXTURE_2D, texture);
    GlobalOpenGL_debugAssertNoErrors();
    current = texture;
  }
}

inline void setState(unsigned int state, unsigned int delta, unsigned int flag, GLenum glflag)
{
  if(delta & state & flag)
  {
    glEnable(glflag);
    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & flag)
  {
    glDisable(glflag);
    GlobalOpenGL_debugAssertNoErrors();
  }
}

void OpenGLState_apply(const OpenGLState& self, OpenGLState& current, unsigned int globalstate)
{
  debug_int("sort", int(self.m_sort));
  debug_int("texture", self.m_texture);
  debug_int("state", self.m_state);
  debug_int("address", int(std::size_t(&self)));

  count_state();

  if(self.m_state & RENDER_OVERRIDE)
  {
    globalstate |= RENDER_FILL | RENDER_DEPTHWRITE;
  }

  const unsigned int state = self.m_state & globalstate;
  const unsigned int delta = state ^ current.m_state;

  GlobalOpenGL_debugAssertNoErrors();

  GLProgram* program = (state & RENDER_PROGRAM) != 0 ? self.m_program : 0;

  if(program != current.m_program)
  {
    if(current.m_program != 0)
    {
      current.m_program->disable();
      glColor4fv(vector4_to_array(current.m_colour));
      debug_colour("cleaning program");
    }

    current.m_program = program;

    if(current.m_program != 0)
    {
      current.m_program->enable();
    }
  }

  if(delta & state & RENDER_FILL)
  {
    //qglPolygonMode (GL_BACK, GL_LINE);
    //qglPolygonMode (GL_FRONT, GL_FILL);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & RENDER_FILL)
  {
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    GlobalOpenGL_debugAssertNoErrors();
  }

  setState(state, delta, RENDER_OFFSETLINE, GL_POLYGON_OFFSET_LINE);

  if(delta & state & RENDER_LIGHTING)
  {
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    //qglEnable(GL_RESCALE_NORMAL);
    glEnableClientState(GL_NORMAL_ARRAY);
    GlobalOpenGL_debugAssertNoErrors();
    g_normalArray_enabled = true;
  }
  else if(delta & ~state & RENDER_LIGHTING)
  {
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    //qglDisable(GL_RESCALE_NORMAL);
    glDisableClientState(GL_NORMAL_ARRAY);
    GlobalOpenGL_debugAssertNoErrors();
    g_normalArray_enabled = false;
  }

  if(delta & state & RENDER_TEXTURE)
  {
    GlobalOpenGL_debugAssertNoErrors();

    if(GlobalOpenGL().GL_1_3())
    {
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
    }

    glEnable(GL_TEXTURE_2D);

    glColor4f(1,1,1,self.m_colour[3]);
    debug_colour("setting texture");

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    GlobalOpenGL_debugAssertNoErrors();
    g_texcoordArray_enabled = true;
  }
  else if(delta & ~state & RENDER_TEXTURE)
  {
    if(GlobalOpenGL().GL_1_3())
    {
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
    }

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    GlobalOpenGL_debugAssertNoErrors();
    g_texcoordArray_enabled = false;
  }

  if(delta & state & RENDER_BLEND)
  {
// FIXME: some .TGA are buggy, have a completely empty alpha channel
// if such brushes are rendered in this loop they would be totally transparent with GL_MODULATE
// so I decided using GL_DECAL instead
// if an empty-alpha-channel or nearly-empty texture is used. It will be blank-transparent.
// this could get better if you can get glTexEnviv (GL_TEXTURE_ENV, to work .. patches are welcome

    glEnable(GL_BLEND);
    if(GlobalOpenGL().GL_1_3())
    {
      glActiveTexture(GL_TEXTURE0);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & RENDER_BLEND)
  {
    glDisable(GL_BLEND);
    if(GlobalOpenGL().GL_1_3())
    {
      glActiveTexture(GL_TEXTURE0);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    GlobalOpenGL_debugAssertNoErrors();
  }

  setState(state, delta, RENDER_CULLFACE, GL_CULL_FACE);

  if(delta & state & RENDER_SMOOTH)
  {
    glShadeModel(GL_SMOOTH);
    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & RENDER_SMOOTH)
  {
    glShadeModel(GL_FLAT);
    GlobalOpenGL_debugAssertNoErrors();
  }

  setState(state, delta, RENDER_SCALED, GL_NORMALIZE); // not GL_RESCALE_NORMAL

  setState(state, delta, RENDER_DEPTHTEST, GL_DEPTH_TEST);

  if(delta & state & RENDER_DEPTHWRITE)
  {
    glDepthMask(GL_TRUE);

#if DEBUG_RENDER
    GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthEnabled);
    ASSERT_MESSAGE(depthEnabled, "failed to set depth buffer mask bit");
#endif
    debug_string("enabled depth-buffer writing");

    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & RENDER_DEPTHWRITE)
  {
    glDepthMask(GL_FALSE);

#if DEBUG_RENDER
    GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthEnabled);
    ASSERT_MESSAGE(!depthEnabled, "failed to set depth buffer mask bit");
#endif
    debug_string("disabled depth-buffer writing");

    GlobalOpenGL_debugAssertNoErrors();
  }

  if(delta & state & RENDER_COLOURWRITE)
  {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    GlobalOpenGL_debugAssertNoErrors();
  }
  else if(delta & ~state & RENDER_COLOURWRITE)
  {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    GlobalOpenGL_debugAssertNoErrors();
  }

  setState(state, delta, RENDER_ALPHATEST, GL_ALPHA_TEST);
  
  if(delta & state & RENDER_COLOURARRAY)
  {
    glEnableClientState(GL_COLOR_ARRAY);
    GlobalOpenGL_debugAssertNoErrors();
    debug_colour("enabling color_array");
    g_colorArray_enabled = true;
  }
  else if(delta & ~state & RENDER_COLOURARRAY)
  {
    glDisableClientState(GL_COLOR_ARRAY);
    glColor4fv(vector4_to_array(self.m_colour));
    debug_colour("cleaning color_array");
    GlobalOpenGL_debugAssertNoErrors();
    g_colorArray_enabled = false;
  }

  if(delta & ~state & RENDER_COLOURCHANGE)
  {
    glColor4fv(vector4_to_array(self.m_colour));
    GlobalOpenGL_debugAssertNoErrors();
  }

  setState(state, delta, RENDER_LINESTIPPLE, GL_LINE_STIPPLE);
  setState(state, delta, RENDER_LINESMOOTH, GL_LINE_SMOOTH);

  setState(state, delta, RENDER_POLYGONSTIPPLE, GL_POLYGON_STIPPLE);
  setState(state, delta, RENDER_POLYGONSMOOTH, GL_POLYGON_SMOOTH);

  setState(state, delta, RENDER_FOG, GL_FOG);

  if((state & RENDER_FOG) != 0)
  {
    setFogState(self.m_fog);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_fog = self.m_fog;
  }

  if(state & RENDER_DEPTHTEST && self.m_depthfunc != current.m_depthfunc)
  {
    glDepthFunc(self.m_depthfunc);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_depthfunc = self.m_depthfunc;
  }

  if(state & RENDER_LINESTIPPLE
    && (self.m_linestipple_factor != current.m_linestipple_factor
    || self.m_linestipple_pattern != current.m_linestipple_pattern))
  {
    glLineStipple(self.m_linestipple_factor, self.m_linestipple_pattern);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_linestipple_factor = self.m_linestipple_factor;
    current.m_linestipple_pattern = self.m_linestipple_pattern;
  }


  if(state & RENDER_ALPHATEST
    && ( self.m_alphafunc != current.m_alphafunc
    || self.m_alpharef != current.m_alpharef ) )
  {
    glAlphaFunc(self.m_alphafunc, self.m_alpharef);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_alphafunc = self.m_alphafunc;
    current.m_alpharef = self.m_alpharef;
  }

  {
    GLint texture0 = 0;
    GLint texture1 = 0;
    GLint texture2 = 0;
    GLint texture3 = 0;
    GLint texture4 = 0;
    GLint texture5 = 0;
    GLint texture6 = 0;
    GLint texture7 = 0;
    //if(state & RENDER_TEXTURE) != 0)
    {
      texture0 = self.m_texture;
      texture1 = self.m_texture1;
      texture2 = self.m_texture2;
      texture3 = self.m_texture3;
      texture4 = self.m_texture4;
      texture5 = self.m_texture5;
      texture6 = self.m_texture6;
      texture7 = self.m_texture7;
    }

    if(GlobalOpenGL().GL_1_3())
    {
      setTextureState(current.m_texture, texture0, GL_TEXTURE0);
      setTextureState(current.m_texture1, texture1, GL_TEXTURE1);
      setTextureState(current.m_texture2, texture2, GL_TEXTURE2);
      setTextureState(current.m_texture3, texture3, GL_TEXTURE3);
      setTextureState(current.m_texture4, texture4, GL_TEXTURE4);
      setTextureState(current.m_texture5, texture5, GL_TEXTURE5);
      setTextureState(current.m_texture6, texture6, GL_TEXTURE6);
      setTextureState(current.m_texture7, texture7, GL_TEXTURE7);
    }
    else
    {
      setTextureState(current.m_texture, texture0);
    }
  }


  if(state & RENDER_TEXTURE && self.m_colour[3] != current.m_colour[3])
  {
    debug_colour("setting alpha");
    glColor4f(1,1,1,self.m_colour[3]);
    GlobalOpenGL_debugAssertNoErrors();
  }

  if(!(state & RENDER_TEXTURE)
    && (self.m_colour[0] != current.m_colour[0]
    || self.m_colour[1] != current.m_colour[1]
    || self.m_colour[2] != current.m_colour[2]
    || self.m_colour[3] != current.m_colour[3]))
  {
    glColor4fv(vector4_to_array(self.m_colour));
    debug_colour("setting non-texture");
    GlobalOpenGL_debugAssertNoErrors();
  }
  current.m_colour = self.m_colour;

  if(state & RENDER_BLEND
    && (self.m_blend_src != current.m_blend_src || self.m_blend_dst != current.m_blend_dst))
  {
    glBlendFunc(self.m_blend_src, self.m_blend_dst);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_blend_src = self.m_blend_src;
    current.m_blend_dst = self.m_blend_dst;
  }

  if(!(state & RENDER_FILL)
    && self.m_linewidth != current.m_linewidth)
  {
    glLineWidth(self.m_linewidth);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_linewidth = self.m_linewidth;
  }

  if(!(state & RENDER_FILL)
    && self.m_pointsize != current.m_pointsize)
  {
    glPointSize(self.m_pointsize);
    GlobalOpenGL_debugAssertNoErrors();
    current.m_pointsize = self.m_pointsize;
  }

  current.m_state = state;

  GlobalOpenGL_debugAssertNoErrors();
}

void Renderables_flush(OpenGLStateBucket::Renderables& renderables, OpenGLState& current, unsigned int globalstate, const Vector3& viewer)
{
  const Matrix4* transform = 0;
  glPushMatrix();
  for(OpenGLStateBucket::Renderables::const_iterator i = renderables.begin(); i != renderables.end(); ++i)
  {
    //qglLoadMatrixf(i->m_transform);
    if(!transform || (transform != (*i).m_transform && !matrix4_affine_equal(*transform, *(*i).m_transform)))
    {
      count_transform();
      transform = (*i).m_transform;
      glPopMatrix();
      glPushMatrix();
      glMultMatrixf(reinterpret_cast<const float*>(transform));
      glFrontFace(((current.m_state & RENDER_CULLFACE) != 0 && matrix4_handedness(*transform) == MATRIX4_RIGHTHANDED) ? GL_CW : GL_CCW);
    }

    count_prim();

    if(current.m_program != 0 && (*i).m_light != 0)
    {
      const IShader& lightShader = static_cast<OpenGLShader*>((*i).m_light->getShader())->getShader();
      if(lightShader.firstLayer() != 0)
      {
        GLuint attenuation_xy = lightShader.firstLayer()->texture()->texture_number;
        GLuint attenuation_z = lightShader.lightFalloffImage() != 0
          ? lightShader.lightFalloffImage()->texture_number
          : static_cast<OpenGLShader*>(g_defaultPointLight)->getShader().lightFalloffImage()->texture_number;

        setTextureState(current.m_texture3, attenuation_xy, GL_TEXTURE3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, attenuation_xy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        setTextureState(current.m_texture4, attenuation_z, GL_TEXTURE4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, attenuation_z);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


        AABB lightBounds((*i).m_light->aabb());

        Matrix4 world2light(g_matrix4_identity);

        if((*i).m_light->isProjected())
        {
          world2light = (*i).m_light->projection();
          matrix4_multiply_by_matrix4(world2light, matrix4_transposed((*i).m_light->rotation()));
          matrix4_translate_by_vec3(world2light, vector3_negated(lightBounds.origin)); // world->lightBounds
        }
        if(!(*i).m_light->isProjected())
        {
          matrix4_translate_by_vec3(world2light, Vector3(0.5f, 0.5f, 0.5f));
          matrix4_scale_by_vec3(world2light, Vector3(0.5f, 0.5f, 0.5f));
          matrix4_scale_by_vec3(world2light, Vector3(1.0f / lightBounds.extents.x(), 1.0f / lightBounds.extents.y(), 1.0f / lightBounds.extents.z()));
          matrix4_multiply_by_matrix4(world2light, matrix4_transposed((*i).m_light->rotation()));
          matrix4_translate_by_vec3(world2light, vector3_negated(lightBounds.origin)); // world->lightBounds
        }

        current.m_program->setParameters(viewer, *(*i).m_transform, lightBounds.origin + (*i).m_light->offset(), (*i).m_light->colour(), world2light);
        debug_string("set lightBounds parameters");
      }
    }

    (*i).m_renderable->render(current.m_state);
  }
  glPopMatrix();
  renderables.clear();
}

void OpenGLStateBucket::render(OpenGLState& current, unsigned int globalstate, const Vector3& viewer)
{
  if((globalstate & m_state.m_state & RENDER_SCREEN) != 0)
  {
    OpenGLState_apply(m_state, current, globalstate);
    debug_colour("screen fill");

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(reinterpret_cast<const float*>(&g_matrix4_identity));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(reinterpret_cast<const float*>(&g_matrix4_identity));

    glBegin(GL_QUADS);
    glVertex3f(-1, -1, 0);
    glVertex3f(1, -1, 0);
    glVertex3f(1, 1, 0);
    glVertex3f(-1, 1, 0);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  else if(!m_renderables.empty())
  {
    OpenGLState_apply(m_state, current, globalstate);
    Renderables_flush(m_renderables, current, globalstate, viewer);
  }
}


class OpenGLStateMap : public OpenGLStateLibrary
{
  typedef std::map<CopiedString, OpenGLState> States;
  States m_states;
public:
  ~OpenGLStateMap()
  {
    ASSERT_MESSAGE(m_states.empty(), "OpenGLStateMap::~OpenGLStateMap: not empty");
  }

  typedef States::iterator iterator;
  iterator begin()
  {
    return m_states.begin();
  }
  iterator end()
  {
    return m_states.end();
  }

  void getDefaultState(OpenGLState& state) const
  {
    OpenGLState_constructDefault(state);
  }

  void insert(const char* name, const OpenGLState& state)
  {
    bool inserted = m_states.insert(States::value_type(name, state)).second;
    ASSERT_MESSAGE(inserted, "OpenGLStateMap::insert: " << name << " already exists");
  }
  void erase(const char* name)
  {
    std::size_t count = m_states.erase(name);
    ASSERT_MESSAGE(count == 1, "OpenGLStateMap::erase: " << name << " does not exist");
  }

  iterator find(const char* name)
  {
    return m_states.find(name);
  }
};

OpenGLStateMap* g_openglStates = 0;

inline GLenum convertBlendFactor(BlendFactor factor)
{
  switch(factor)
  {
  case BLEND_ZERO:
    return GL_ZERO;
  case BLEND_ONE:
    return GL_ONE;
  case BLEND_SRC_COLOUR:
    return GL_SRC_COLOR;
  case BLEND_ONE_MINUS_SRC_COLOUR:
    return GL_ONE_MINUS_SRC_COLOR;
  case BLEND_SRC_ALPHA:
    return GL_SRC_ALPHA;
  case BLEND_ONE_MINUS_SRC_ALPHA:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BLEND_DST_COLOUR:
    return GL_DST_COLOR;
  case BLEND_ONE_MINUS_DST_COLOUR:
    return GL_ONE_MINUS_DST_COLOR;
  case BLEND_DST_ALPHA:
    return GL_DST_ALPHA;
  case BLEND_ONE_MINUS_DST_ALPHA:
    return GL_ONE_MINUS_DST_ALPHA;
  case BLEND_SRC_ALPHA_SATURATE:
    return GL_SRC_ALPHA_SATURATE;
  }
  return GL_ZERO;
}

/// \todo Define special-case shaders in a data file.
void OpenGLShader::construct(const char* name)
{
  OpenGLState& state = appendDefaultPass();
  switch(name[0])
  {
  case '(':
    sscanf(name, "(%g %g %g)", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
    state.m_colour[3] = 1.0f;
    state.m_state = RENDER_FILL|RENDER_LIGHTING|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
    state.m_sort = OpenGLState::eSortFullbright;
    break;

  case '[':
    sscanf(name, "[%g %g %g]", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
    state.m_colour[3] = 0.5f;
    state.m_state = RENDER_FILL|RENDER_LIGHTING|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_BLEND;
    state.m_sort = OpenGLState::eSortTranslucent;
    break;

  case '<':
    sscanf(name, "<%g %g %g>", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
    state.m_colour[3] = 1;
    state.m_state = RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
    state.m_sort = OpenGLState::eSortFullbright;
    state.m_depthfunc = GL_LESS;
    state.m_linewidth = 1;
    state.m_pointsize = 1;
    break;

  case '$':
    {
      OpenGLStateMap::iterator i = g_openglStates->find(name);
      if(i != g_openglStates->end())
      {
        state = (*i).second;
        break;
      }
    }
    if(string_equal(name+1, "POINT"))
    {
      state.m_state = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortControlFirst;
      state.m_pointsize = 4;
    }
    else if(string_equal(name+1, "SELPOINT"))
    {
      state.m_state = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortControlFirst + 1;
      state.m_pointsize = 4;
    }
    else if(string_equal(name+1, "BIGPOINT"))
    {
      state.m_state = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortControlFirst;
      state.m_pointsize = 6;
    }
    else if(string_equal(name+1, "PIVOT"))
    {
      state.m_state = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHTEST|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortGUI1;
      state.m_linewidth = 2;
      state.m_depthfunc = GL_LEQUAL;

      OpenGLState& hiddenLine = appendDefaultPass();
      hiddenLine.m_state = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHTEST|RENDER_LINESTIPPLE;
      hiddenLine.m_sort = OpenGLState::eSortGUI0;
      hiddenLine.m_linewidth = 2;
      hiddenLine.m_depthfunc = GL_GREATER;
    }
    else if(string_equal(name+1, "LATTICE"))
    {
      state.m_colour[0] = 1;
      state.m_colour[1] = 0.5;
      state.m_colour[2] = 0;
      state.m_colour[3] = 1;
      state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortControlFirst;
    }
    else if(string_equal(name+1, "WIREFRAME"))
    {
      state.m_state = RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortFullbright;
    }
    else if(string_equal(name+1, "CAM_HIGHLIGHT"))
    {
      state.m_colour[0] = 1;
      state.m_colour[1] = 0;
      state.m_colour[2] = 0;
      state.m_colour[3] = 0.3f;
      state.m_state = RENDER_FILL|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortHighlight;
      state.m_depthfunc = GL_LEQUAL;
    }
    else if(string_equal(name+1, "CAM_OVERLAY"))
    {
#if 0
      state.m_state = RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortOverlayFirst;
#else
      state.m_state = RENDER_CULLFACE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_OFFSETLINE;
      state.m_sort = OpenGLState::eSortOverlayFirst + 1;
      state.m_depthfunc = GL_LEQUAL;

      OpenGLState& hiddenLine = appendDefaultPass();
      hiddenLine.m_colour[0] = 0.75;
      hiddenLine.m_colour[1] = 0.75;
      hiddenLine.m_colour[2] = 0.75;
      hiddenLine.m_colour[3] = 1;
      hiddenLine.m_state = RENDER_CULLFACE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_OFFSETLINE|RENDER_LINESTIPPLE;
      hiddenLine.m_sort = OpenGLState::eSortOverlayFirst;
      hiddenLine.m_depthfunc = GL_GREATER;
      hiddenLine.m_linestipple_factor = 2;
#endif
    }
    else if(string_equal(name+1, "XY_OVERLAY"))
    {
      state.m_colour[0] = g_xywindow_globals.color_selbrushes[0];
      state.m_colour[1] = g_xywindow_globals.color_selbrushes[1];
      state.m_colour[2] = g_xywindow_globals.color_selbrushes[2];
      state.m_colour[3] = 1;
      state.m_state = RENDER_COLOURWRITE | RENDER_LINESTIPPLE;
      state.m_sort = OpenGLState::eSortOverlayFirst;
      state.m_linewidth = 2;
      state.m_linestipple_factor = 3;
    }
    else if(string_equal(name+1, "DEBUG_CLIPPED"))
    {
      state.m_state = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortLast;
    }
    else if(string_equal(name+1, "POINTFILE"))
    {
      state.m_colour[0] = 1;
      state.m_colour[1] = 0;
      state.m_colour[2] = 0;
      state.m_colour[3] = 1;
      state.m_state = RENDER_DEPTHTEST | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortFullbright;
      state.m_linewidth = 4;
    }
    else if(string_equal(name+1, "LIGHT_SPHERE"))
    {
      state.m_colour[0] = .15f * .95f;
      state.m_colour[1] = .15f * .95f;
      state.m_colour[2] = .15f * .95f;
      state.m_colour[3] = 1;
      state.m_state = RENDER_CULLFACE | RENDER_DEPTHTEST | RENDER_BLEND | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
      state.m_blend_src = GL_ONE;
      state.m_blend_dst = GL_ONE;
      state.m_sort = OpenGLState::eSortTranslucent;
    }
    else if(string_equal(name+1, "Q3MAP2_LIGHT_SPHERE"))
    {
      state.m_colour[0] = .05f;
      state.m_colour[1] = .05f;
      state.m_colour[2] = .05f;
      state.m_colour[3] = 1;
      state.m_state = RENDER_CULLFACE | RENDER_DEPTHTEST | RENDER_BLEND | RENDER_FILL;
      state.m_blend_src = GL_ONE;
      state.m_blend_dst = GL_ONE;
      state.m_sort = OpenGLState::eSortTranslucent;
    }
    else if(string_equal(name+1, "WIRE_OVERLAY"))
    {
#if 0
      state.m_state = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
      state.m_sort = OpenGLState::eSortOverlayFirst;
#else
      state.m_state = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
      state.m_sort = OpenGLState::eSortGUI1;
      state.m_depthfunc = GL_LEQUAL;

      OpenGLState& hiddenLine = appendDefaultPass();
      hiddenLine.m_state = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE | RENDER_LINESTIPPLE;
      hiddenLine.m_sort = OpenGLState::eSortGUI0;
      hiddenLine.m_depthfunc = GL_GREATER;
#endif
    }
    else if(string_equal(name+1, "FLATSHADE_OVERLAY"))
    {
      state.m_state = RENDER_CULLFACE | RENDER_LIGHTING | RENDER_SMOOTH | RENDER_SCALED | RENDER_COLOURARRAY | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
      state.m_sort = OpenGLState::eSortGUI1;
      state.m_depthfunc = GL_LEQUAL;

      OpenGLState& hiddenLine = appendDefaultPass();
      hiddenLine.m_state = RENDER_CULLFACE | RENDER_LIGHTING | RENDER_SMOOTH | RENDER_SCALED | RENDER_COLOURARRAY | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE | RENDER_POLYGONSTIPPLE;
      hiddenLine.m_sort = OpenGLState::eSortGUI0;
      hiddenLine.m_depthfunc = GL_GREATER;
    }
    else if(string_equal(name+1, "CLIPPER_OVERLAY"))
    {
      state.m_colour[0] = g_xywindow_globals.color_clipper[0];
      state.m_colour[1] = g_xywindow_globals.color_clipper[1];
      state.m_colour[2] = g_xywindow_globals.color_clipper[2];
      state.m_colour[3] = 1;
      state.m_state = RENDER_CULLFACE | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_FILL | RENDER_POLYGONSTIPPLE;
      state.m_sort = OpenGLState::eSortOverlayFirst;
    }
    else if(string_equal(name+1, "OVERBRIGHT"))
    {
      const float lightScale = 2;
      state.m_colour[0] = lightScale * 0.5f;
      state.m_colour[1] = lightScale * 0.5f;
      state.m_colour[2] = lightScale * 0.5f;
      state.m_colour[3] = 0.5;
      state.m_state = RENDER_FILL|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_SCREEN;
      state.m_sort = OpenGLState::eSortOverbrighten;
      state.m_blend_src = GL_DST_COLOR;
      state.m_blend_dst = GL_SRC_COLOR;
    }
    else
    {
      // default to something recognisable.. =)
      ERROR_MESSAGE("hardcoded renderstate not found");
      state.m_colour[0] = 1;
      state.m_colour[1] = 0;
      state.m_colour[2] = 1;
      state.m_colour[3] = 1;
      state.m_state = RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
      state.m_sort = OpenGLState::eSortFirst;
    }
    break;
  default:
    // construction from IShader
    m_shader = QERApp_Shader_ForName(name);

    if(g_ShaderCache->lightingSupported() && g_ShaderCache->lightingEnabled() && m_shader->getBump() != 0 && m_shader->getBump()->texture_number != 0) // is a bump shader
    {
      state.m_state = RENDER_FILL | RENDER_CULLFACE | RENDER_TEXTURE | RENDER_DEPTHTEST | RENDER_DEPTHWRITE | RENDER_COLOURWRITE | RENDER_PROGRAM;
      state.m_colour[0] = 0;
      state.m_colour[1] = 0;
      state.m_colour[2] = 0;
      state.m_colour[3] = 1;
      state.m_sort = OpenGLState::eSortOpaque;

      if(g_ShaderCache->useShaderLanguage())
      {
        state.m_program = &g_depthFillGLSL;
      }
      else
      {
        state.m_program = &g_depthFillARB;
      }

      OpenGLState& bumpPass = appendDefaultPass();
      bumpPass.m_texture = m_shader->getDiffuse()->texture_number;
      bumpPass.m_texture1 = m_shader->getBump()->texture_number;
      bumpPass.m_texture2 = m_shader->getSpecular()->texture_number;

      bumpPass.m_state = RENDER_BLEND|RENDER_FILL|RENDER_CULLFACE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_SMOOTH|RENDER_BUMP|RENDER_PROGRAM;

      if(g_ShaderCache->useShaderLanguage())
      {
        bumpPass.m_state |= RENDER_LIGHTING;
        bumpPass.m_program = &g_bumpGLSL;
      }
      else
      {
        bumpPass.m_program = &g_bumpARB;
      }

      bumpPass.m_depthfunc = GL_LEQUAL;
      bumpPass.m_sort = OpenGLState::eSortMultiFirst;
      bumpPass.m_blend_src = GL_ONE;
      bumpPass.m_blend_dst = GL_ONE;
    }
    else
    {
      state.m_texture = m_shader->getTexture()->texture_number;

      state.m_state = RENDER_FILL|RENDER_TEXTURE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_LIGHTING|RENDER_SMOOTH;
      if((m_shader->getFlags() & QER_CULL) != 0)
      {
        if(m_shader->getCull() == IShader::eCullBack)
        {
          state.m_state |= RENDER_CULLFACE;
        }
      }
      else
      {
        state.m_state |= RENDER_CULLFACE;
      }
      if((m_shader->getFlags() & QER_ALPHATEST) != 0)
      {
        state.m_state |= RENDER_ALPHATEST;
        IShader::EAlphaFunc alphafunc;
        m_shader->getAlphaFunc(&alphafunc, &state.m_alpharef);
        switch(alphafunc)
        {
        case IShader::eAlways:
          state.m_alphafunc = GL_ALWAYS;
        case IShader::eEqual:
          state.m_alphafunc = GL_EQUAL;
        case IShader::eLess:
          state.m_alphafunc = GL_LESS;
        case IShader::eGreater:
          state.m_alphafunc = GL_GREATER;
        case IShader::eLEqual:
          state.m_alphafunc = GL_LEQUAL;
        case IShader::eGEqual:
          state.m_alphafunc = GL_GEQUAL;
        }
      }
      reinterpret_cast<Vector3&>(state.m_colour) = m_shader->getTexture()->color;
      state.m_colour[3] = 1.0f;
      
      if((m_shader->getFlags() & QER_TRANS) != 0)
      {
        state.m_state |= RENDER_BLEND;
        state.m_colour[3] = m_shader->getTrans();
        state.m_sort = OpenGLState::eSortTranslucent;
        BlendFunc blendFunc = m_shader->getBlendFunc();
        state.m_blend_src = convertBlendFactor(blendFunc.m_src);
        state.m_blend_dst = convertBlendFactor(blendFunc.m_dst);
        if(state.m_blend_src == GL_SRC_ALPHA || state.m_blend_dst == GL_SRC_ALPHA)
        {
          state.m_state |= RENDER_DEPTHWRITE;
        }
      }
      else
      {
        state.m_state |= RENDER_DEPTHWRITE;
        state.m_sort = OpenGLState::eSortFullbright;
      }
    }
  }
}


#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class OpenGLStateLibraryAPI
{
  OpenGLStateMap m_stateMap;
public:
  typedef OpenGLStateLibrary Type;
  STRING_CONSTANT(Name, "*");

  OpenGLStateLibraryAPI()
  {
    g_openglStates = &m_stateMap;
  }
  ~OpenGLStateLibraryAPI()
  {
    g_openglStates = 0;
  }
  OpenGLStateLibrary* getTable()
  {
    return &m_stateMap;
  }
};

typedef SingletonModule<OpenGLStateLibraryAPI> OpenGLStateLibraryModule;
typedef Static<OpenGLStateLibraryModule> StaticOpenGLStateLibraryModule;
StaticRegisterModule staticRegisterOpenGLStateLibrary(StaticOpenGLStateLibraryModule::instance());

class ShaderCacheDependencies : public GlobalShadersModuleRef, public GlobalTexturesModuleRef, public GlobalOpenGLStateLibraryModuleRef
{
public:
  ShaderCacheDependencies() :
    GlobalShadersModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("shaders"))
  {
  }
};

class ShaderCacheAPI
{
  ShaderCache* m_shaderCache;
public:
  typedef ShaderCache Type;
  STRING_CONSTANT(Name, "*");

  ShaderCacheAPI()
  {
    ShaderCache_Construct();

    m_shaderCache = GetShaderCache();
  }
  ~ShaderCacheAPI()
  {
    ShaderCache_Destroy();
  }
  ShaderCache* getTable()
  {
    return m_shaderCache;
  }
};

typedef SingletonModule<ShaderCacheAPI, ShaderCacheDependencies> ShaderCacheModule;
typedef Static<ShaderCacheModule> StaticShaderCacheModule;
StaticRegisterModule staticRegisterShaderCache(StaticShaderCacheModule::instance());


