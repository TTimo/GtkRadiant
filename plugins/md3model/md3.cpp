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

#include "md3.h"

#include "ifilesystem.h"
#include "imodel.h"

#include "imagelib.h"
#include "bytestreamutils.h"

#include "model.h"
#include "ident.h"
#include "md3normals.h"

// the maximum size of game reletive pathnames
#define	MAX_QPATH		64

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

const unsigned char MD3_IDENT[4] = { 'I', 'D', 'P', '3', };
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		4
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.f / 64)

typedef float float3[3];

void istream_read_float3(PointerInputStream& inputStream, float3 f)
{
  f[0] = istream_read_float32_le(inputStream);
  f[1] = istream_read_float32_le(inputStream);
  f[2] = istream_read_float32_le(inputStream);
}

typedef struct md3Frame_s {
	float3		bounds[2];
	float3		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

void istream_read_md3Frame(PointerInputStream& inputStream, md3Frame_t& frame)
{
  istream_read_float3(inputStream, frame.bounds[0]);
  istream_read_float3(inputStream, frame.bounds[1]);
  istream_read_float3(inputStream, frame.localOrigin);
  frame.radius = istream_read_float32_le(inputStream);
  inputStream.read(reinterpret_cast<unsigned char*>(frame.name), 16);
}

typedef struct md3Tag_s {
	char		name[MAX_QPATH];	// tag name
	float3		origin;
	float3		axis[3];
} md3Tag_t;

void istream_read_md3Shader(PointerInputStream& inputStream, md3Tag_t& tag)
{
  inputStream.read(reinterpret_cast<unsigned char*>(tag.name), MAX_QPATH);
  istream_read_float3(inputStream, tag.origin);
  istream_read_float3(inputStream, tag.axis[0]);
  istream_read_float3(inputStream, tag.axis[1]);
  istream_read_float3(inputStream, tag.axis[2]);
}

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct {
  char ident[4];				// 

  char name[MAX_QPATH];	// polyset name

  int flags;
  int numFrames; // all surfaces in a model should have the same

  int numShaders; // all surfaces in a model should have the same
  int numVerts;

  int numTriangles;
  int ofsTriangles;

  int ofsShaders; // offset from start of md3Surface_t
  int ofsSt; // texture coords are common for all frames
  int ofsXyzNormals; // numVerts * numFrames

  int ofsEnd; // next surface follows
} md3Surface_t;

void istream_read_md3Surface(PointerInputStream& inputStream, md3Surface_t& surface)
{
  inputStream.read(reinterpret_cast<unsigned char*>(surface.ident), 4);
  inputStream.read(reinterpret_cast<unsigned char*>(surface.name), MAX_QPATH);
  surface.flags = istream_read_int32_le(inputStream);
  surface.numFrames = istream_read_int32_le(inputStream);
  surface.numShaders = istream_read_int32_le(inputStream);
  surface.numVerts = istream_read_int32_le(inputStream);
  surface.numTriangles = istream_read_int32_le(inputStream);
  surface.ofsTriangles = istream_read_int32_le(inputStream);
  surface.ofsShaders = istream_read_int32_le(inputStream);
  surface.ofsSt = istream_read_int32_le(inputStream);
  surface.ofsXyzNormals = istream_read_int32_le(inputStream);
  surface.ofsEnd = istream_read_int32_le(inputStream);
}

typedef struct {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
} md3Shader_t;

void istream_read_md3Shader(PointerInputStream& inputStream, md3Shader_t& shader)
{
  inputStream.read(reinterpret_cast<unsigned char*>(shader.name), MAX_QPATH);
  shader.shaderIndex = istream_read_int32_le(inputStream);
}

typedef struct {
	int			indexes[3];
} md3Triangle_t;

void istream_read_md3Triangle(PointerInputStream& inputStream, md3Triangle_t& triangle)
{
  triangle.indexes[0] = istream_read_int32_le(inputStream);
  triangle.indexes[1] = istream_read_int32_le(inputStream);
  triangle.indexes[2] = istream_read_int32_le(inputStream);
}

typedef struct {
	float		st[2];
} md3St_t;

void istream_read_md3St(PointerInputStream& inputStream, md3St_t& st)
{
  st.st[0] = istream_read_float32_le(inputStream);
  st.st[1] = istream_read_float32_le(inputStream);
}

typedef struct {
	short		xyz[3];
	short		normal;
} md3XyzNormal_t;

void istream_read_md3XyzNormal(PointerInputStream& inputStream, md3XyzNormal_t& xyz)
{
  xyz.xyz[0] = istream_read_int16_le(inputStream);
  xyz.xyz[1] = istream_read_int16_le(inputStream);
  xyz.xyz[2] = istream_read_int16_le(inputStream);
  xyz.normal = istream_read_int16_le(inputStream);
}

typedef struct {
	char		ident[4];
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
} md3Header_t;

void istream_read_md3Header(PointerInputStream& inputStream, md3Header_t& header)
{
  inputStream.read(reinterpret_cast<unsigned char*>(header.ident), 4);
  header.version = istream_read_int32_le(inputStream);
  inputStream.read(reinterpret_cast<unsigned char*>(header.name), MAX_QPATH);
  header.flags = istream_read_int32_le(inputStream);
  header.numFrames = istream_read_int32_le(inputStream);
  header.numTags = istream_read_int32_le(inputStream);
  header.numSurfaces = istream_read_int32_le(inputStream);
  header.numSkins = istream_read_int32_le(inputStream);
  header.ofsFrames = istream_read_int32_le(inputStream);
  header.ofsTags = istream_read_int32_le(inputStream);
  header.ofsSurfaces = istream_read_int32_le(inputStream);
  header.ofsEnd = istream_read_int32_le(inputStream);
}

int MD3Surface_read(Surface& surface, unsigned char* buffer)
{
	md3Surface_t md3Surface;
  {
    PointerInputStream inputStream(buffer);
    istream_read_md3Surface(inputStream, md3Surface);
  }

  {
    surface.vertices().reserve(md3Surface.numVerts);

    PointerInputStream xyzNormalStream(buffer + md3Surface.ofsXyzNormals);
    PointerInputStream stStream(buffer + md3Surface.ofsSt);

     // read verts into vertex array - xyz, st, normal
    for(int i = 0; i < md3Surface.numVerts; i++)
	  {
	    md3XyzNormal_t md3Xyz;
      istream_read_md3XyzNormal(xyzNormalStream, md3Xyz);

      md3St_t md3St;
      istream_read_md3St(stStream, md3St);

      surface.vertices().push_back(
        ArbitraryMeshVertex(
          Vertex3f( md3Xyz.xyz[0] * MD3_XYZ_SCALE, md3Xyz.xyz[1] * MD3_XYZ_SCALE, md3Xyz.xyz[2] * MD3_XYZ_SCALE),
          DecodeNormal(reinterpret_cast<byte*>(&md3Xyz.normal)),
          TexCoord2f(md3St.st[0], md3St.st[1])
        )
      );
    }
  }

  {
	  surface.indices().reserve(md3Surface.numTriangles * 3);

    PointerInputStream inputStream(buffer + md3Surface.ofsTriangles);
    for(int i = 0; i < md3Surface.numTriangles; i++)
    {
	    md3Triangle_t md3Triangle;
      istream_read_md3Triangle(inputStream, md3Triangle);
      surface.indices().insert(md3Triangle.indexes[0]);
      surface.indices().insert(md3Triangle.indexes[1]);
      surface.indices().insert(md3Triangle.indexes[2]);
    }
  }

  {
	  md3Shader_t md3Shader;
    {
      PointerInputStream inputStream(buffer + md3Surface.ofsShaders);
      istream_read_md3Shader(inputStream, md3Shader);
    }
    surface.setShader(md3Shader.name);
  }
	
	surface.updateAABB();

  return md3Surface.ofsEnd;
}

void MD3Model_read(Model& model, unsigned char* buffer)
{
  md3Header_t md3Header;
  {
    PointerInputStream inputStream(buffer);
    istream_read_md3Header(inputStream, md3Header);
  }

  unsigned char* surfacePosition = buffer + md3Header.ofsSurfaces;

  for(int i = 0; i != md3Header.numSurfaces; ++i)
	{
    surfacePosition += MD3Surface_read(model.newSurface(), surfacePosition);
  }

  model.updateAABB();
}

scene::Node& MD3Model_new(unsigned char* buffer)
{
  ModelNode* modelNode = new ModelNode();
  MD3Model_read(modelNode->model(), buffer);
  return modelNode->node();
}

scene::Node& MD3Model_default()
{
  ModelNode* modelNode = new ModelNode();
  Model_constructNull(modelNode->model());
  return modelNode->node();
}

scene::Node& MD3Model_fromBuffer(unsigned char* buffer)
{
  if (!ident_equal(buffer, MD3_IDENT))
  {
	  globalErrorStream() << "MD3 read error: incorrect ident\n";
    return MD3Model_default();
  }
  else
  {
    return MD3Model_new(buffer);
  }
}

scene::Node& loadMD3Model(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return MD3Model_fromBuffer(buffer.buffer);
}

