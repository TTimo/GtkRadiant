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

#include "md5.h"

#include "iscriplib.h"
#include "imodel.h"

#include "archivelib.h"
#include "stringio.h"

#include "model.h"

#define MD5_RETURN_FALSE_IF_FAIL(expression) if(!(expression)) { globalErrorStream() << "md5 parse failed: " #expression "\n"; return false; } else

bool MD5_parseToken(Tokeniser& tokeniser, const char* string)
{
  const char* token = tokeniser.getToken();
  MD5_RETURN_FALSE_IF_FAIL(token != 0);
  return string_equal(token, string);
}

bool MD5_parseFloat(Tokeniser& tokeniser, float& f)
{
  const char* token = tokeniser.getToken();
  MD5_RETURN_FALSE_IF_FAIL(token != 0);
  return string_parse_float(token, f);
}

bool MD5_parseString(Tokeniser& tokeniser, const char*& s)
{
  const char* token = tokeniser.getToken();
  MD5_RETURN_FALSE_IF_FAIL(token != 0);
  s = token;
  return true;
}

bool MD5_parseInteger(Tokeniser& tokeniser, int& i)
{
  const char* token = tokeniser.getToken();
  MD5_RETURN_FALSE_IF_FAIL(token != 0);
  return string_parse_int(token, i);
}

bool MD5_parseSize(Tokeniser& tokeniser, std::size_t& i)
{
  const char* token = tokeniser.getToken();
  MD5_RETURN_FALSE_IF_FAIL(token != 0);
  return string_parse_size(token, i);
}

bool MD5_parseVector3(Tokeniser& tokeniser, Vector3& v)
{
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "("));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, v.x()));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, v.y()));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, v.z()));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, ")"));
  return true;
}

template<typename Element>
inline Element float_squared(const Element& f)
{
  return f * f;
}

class MD5Joint
{
public:
  int parent;
  Vector3 position;
  Vector4 rotation;
};

typedef Array<MD5Joint> MD5Joints;

class MD5Vert
{
public:
  std::size_t index;
  float u;
  float v;
  std::size_t weight_index;
  std::size_t weight_count;
};

typedef Array<MD5Vert> MD5Verts;

class MD5Tri
{
public:
  std::size_t index;
  std::size_t a;
  std::size_t b;
  std::size_t c;
};

typedef Array<MD5Tri> MD5Tris;

class MD5Weight
{
public:
  std::size_t index;
  std::size_t joint;
  float t;
  Vector3 v;
};

typedef Array<MD5Weight> MD5Weights;

typedef float MD5Component;
typedef Array<MD5Component> MD5Components;

class MD5Frame
{
public:
  MD5Components m_components;
};

typedef Array<MD5Weight> MD5Weights;

bool MD5_parseVersion(Tokeniser& tokeniser)
{
  {
    const char* versionKey = tokeniser.getToken();
    if(versionKey == 0 || !string_equal(versionKey, "MD5Version"))
    {
      globalErrorStream() << "not a valid md5 file\n";
      return false;
    }
  }
  {
    const char* versionValue = tokeniser.getToken();
    if(versionValue == 0 || !string_equal(versionValue, "10"))
    {
      globalErrorStream() << "only md5 version 10 supported\n";
      return false;
    }
  }

  return true;
}

bool MD5Anim_parse(Tokeniser& tokeniser)
{
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseVersion(tokeniser));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "commandline"));
  const char* commandline;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseString(tokeniser, commandline));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numFrames"));
  std::size_t numFrames;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numFrames));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numJoints"));
  std::size_t numJoints;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numJoints));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "frameRate"));
  std::size_t frameRate;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, frameRate));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numAnimatedComponents"));
  std::size_t numAnimatedComponents;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numAnimatedComponents));
  tokeniser.nextLine();

  // parse heirarchy
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "hierarchy"));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
  tokeniser.nextLine();

  for(std::size_t i = 0; i < numJoints; ++i)
  {
    const char* name;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseString(tokeniser, name));
    int parent;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseInteger(tokeniser, parent));
    std::size_t flags;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, flags));
    std::size_t index;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, index));
    tokeniser.nextLine();
  }

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
  tokeniser.nextLine();

  // parse bounds
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "bounds"));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
  tokeniser.nextLine();

  for(std::size_t i = 0; i < numFrames; ++i)
  {
    Vector3 mins;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, mins));
    Vector3 maxs;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, maxs));
    tokeniser.nextLine();
  }

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
  tokeniser.nextLine();

  // parse baseframe
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "baseframe"));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
  tokeniser.nextLine();

  for(std::size_t i = 0; i < numJoints; ++i)
  {
    Vector3 position;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, position));
    Vector3 rotation;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, rotation));
    tokeniser.nextLine();
  }

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
  tokeniser.nextLine();

  // parse frames
  for(std::size_t i = 0; i < numFrames; ++i)
  {
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "frame"));
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
    tokeniser.nextLine();

    for(std::size_t i = 0; i < numAnimatedComponents; ++i)
    {
      float component;
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, component));
      tokeniser.nextLine();
    }

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
    tokeniser.nextLine();
  }

  return true;
}

bool MD5Model_parse(Model& model, Tokeniser& tokeniser)
{
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseVersion(tokeniser));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "commandline"));
  const char* commandline;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseString(tokeniser, commandline));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numJoints"));
  std::size_t numJoints;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numJoints));
  tokeniser.nextLine();

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numMeshes"));
  std::size_t numMeshes;
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numMeshes));
  tokeniser.nextLine();

  MD5Joints joints(numJoints);

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "joints"));
  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
  tokeniser.nextLine();

  for(MD5Joints::iterator i = joints.begin(); i != joints.end(); ++i)
  {
    const char* jointName;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseString(tokeniser, jointName));
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseInteger(tokeniser, (*i).parent));
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, (*i).position));
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, (*i).rotation));
    (*i).rotation.w() = -static_cast<float>(sqrt(1.0f - (float_squared((*i).rotation.x()) + float_squared((*i).rotation.y()) + float_squared((*i).rotation.z()))));
    tokeniser.nextLine();
  }

  MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
  tokeniser.nextLine();

  for(std::size_t i = 0; i < numMeshes; ++i)
  {
    Surface& surface = model.newSurface();

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "mesh"));
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "{"));
    tokeniser.nextLine();

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "shader"));
    const char* shader;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseString(tokeniser, shader));
    surface.setShader(shader);
    tokeniser.nextLine();
 
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numverts"));
    std::size_t numVerts;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numVerts));
    tokeniser.nextLine();

    MD5Verts verts(numVerts);

    for(MD5Verts::iterator j = verts.begin(); j != verts.end(); ++j)
    {
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "vert"));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).index));
      MD5_RETURN_FALSE_IF_FAIL((*j).index == std::size_t(j - verts.begin()));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "("));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, (*j).u));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, (*j).v));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, ")"));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).weight_index));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).weight_count));
      tokeniser.nextLine();
    }

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numtris"));
    std::size_t numTris;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numTris));
    tokeniser.nextLine();

    MD5Tris tris(numTris);

    for(MD5Tris::iterator j = tris.begin(); j != tris.end(); ++j)
    {
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "tri"));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).index));
      MD5_RETURN_FALSE_IF_FAIL((*j).index == std::size_t(j - tris.begin()));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).a));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).b));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).c));
      tokeniser.nextLine();
    }

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "numweights"));
    std::size_t numWeights;
    MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, numWeights));
    tokeniser.nextLine();

    MD5Weights weights(numWeights);

    for(MD5Weights::iterator j = weights.begin(); j != weights.end(); ++j)
    {
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "weight"));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).index));
      MD5_RETURN_FALSE_IF_FAIL((*j).index == std::size_t(j - weights.begin()));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseSize(tokeniser, (*j).joint));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseFloat(tokeniser, (*j).t));
      MD5_RETURN_FALSE_IF_FAIL(MD5_parseVector3(tokeniser, (*j).v));
      tokeniser.nextLine();
    }

    MD5_RETURN_FALSE_IF_FAIL(MD5_parseToken(tokeniser, "}"));
    tokeniser.nextLine();

    for(MD5Verts::iterator j = verts.begin(); j != verts.end(); ++j)
    {
      MD5Vert& vert = (*j);

      Vector3 skinned(0, 0, 0);
      for(std::size_t k = 0; k != vert.weight_count; ++k)
      {
        MD5Weight& weight = weights[vert.weight_index + k];
        MD5Joint& joint = joints[weight.joint];

        skinned += (quaternion_transformed_point(joint.rotation, weight.v) + joint.position) * weight.t;
      }
      
      surface.vertices().push_back(ArbitraryMeshVertex(vertex3f_for_vector3(skinned), Normal3f(0, 0, 0), TexCoord2f(vert.u, vert.v)));
    }

    for(MD5Tris::iterator j = tris.begin(); j != tris.end(); ++j)
    {
      MD5Tri& tri = (*j);
      surface.indices().insert(RenderIndex(tri.a));
      surface.indices().insert(RenderIndex(tri.b));
      surface.indices().insert(RenderIndex(tri.c));
    }

    for(Surface::indices_t::iterator j = surface.indices().begin(); j != surface.indices().end(); j += 3)
    {
			ArbitraryMeshVertex& a = surface.vertices()[*(j + 0)];
			ArbitraryMeshVertex& b = surface.vertices()[*(j + 1)];
			ArbitraryMeshVertex& c = surface.vertices()[*(j + 2)];
			Vector3 weightedNormal(
        vector3_cross(
          reinterpret_cast<const Vector3&>(c.vertex) - reinterpret_cast<const Vector3&>(a.vertex),
          reinterpret_cast<const Vector3&>(b.vertex) - reinterpret_cast<const Vector3&>(a.vertex)
        )
      );
      reinterpret_cast<Vector3&>(a.normal) += weightedNormal;
      reinterpret_cast<Vector3&>(b.normal) += weightedNormal;
      reinterpret_cast<Vector3&>(c.normal) += weightedNormal;
    }

    for(Surface::vertices_t::iterator j = surface.vertices().begin(); j != surface.vertices().end(); ++j)
    {
      vector3_normalise(reinterpret_cast<Vector3&>((*j).normal));
    }

    surface.updateAABB();
  }

  model.updateAABB();

  return true;
}

void MD5Model_construct(Model& model, TextInputStream& inputStream)
{
  Tokeniser& tokeniser = GlobalScriptLibrary().m_pfnNewSimpleTokeniser(inputStream);
  MD5Model_parse(model, tokeniser);
  tokeniser.release();
}

scene::Node& MD5Model_new(TextInputStream& inputStream)
{
  ModelNode* modelNode = new ModelNode();
  MD5Model_construct(modelNode->model(), inputStream);
  return modelNode->node();
}

scene::Node& loadMD5Model(ArchiveFile& file)
{
  BinaryToTextInputStream<InputStream> inputStream(file.getInputStream());
  return MD5Model_new(inputStream);
}
