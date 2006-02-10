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

#include "mdc.h"

#include "ifilesystem.h"
#include "imodel.h"

#include "imagelib.h"
#include "bytestreamutils.h"

#include "model.h"
#include "ident.h"
#include "md3normals.h"


const unsigned char MDC_IDENT[4] = { 'I', 'D', 'P', 'C', };
const float MDC_XYZ_SCALE = 0.015625f;
#define	MAX_QPATH			64		// max length of a quake game pathname

typedef float float3[3];

struct mdcTriangle_t
{
	unsigned int indexes[3]; // not my spelling
};

void istream_read_mdcTriangle(PointerInputStream& inputStream, mdcTriangle_t& triangle)
{
  triangle.indexes[0] = istream_read_uint32_le(inputStream);
  triangle.indexes[1] = istream_read_uint32_le(inputStream);
  triangle.indexes[2] = istream_read_uint32_le(inputStream);
}

struct mdcXyzNormal_t
{
	short xyz[3]; // divide by 64
	short normal; // packed same way as md3
};
	
void istream_read_mdcXyzNormal(PointerInputStream& inputStream, mdcXyzNormal_t& xyz)
{
  xyz.xyz[0] = istream_read_int16_le(inputStream);
  xyz.xyz[1] = istream_read_int16_le(inputStream);
  xyz.xyz[2] = istream_read_int16_le(inputStream);
  xyz.normal = istream_read_int16_le(inputStream);
}

struct mdcSt_t
{
	float st[2]; // may need to reverse t
};

void istream_read_mdcSt(PointerInputStream& inputStream, mdcSt_t& st)
{
  st.st[0] = istream_read_float32_le(inputStream);
  st.st[1] = istream_read_float32_le(inputStream);
}

struct mdcShader_t
{
	char name[MAX_QPATH];
	unsigned int flags;
};

void istream_read_mdcShader(PointerInputStream& inputStream, mdcShader_t& shader)
{
  inputStream.read(reinterpret_cast<byte*>(shader.name), MAX_QPATH);
  shader.flags = istream_read_uint32_le(inputStream);
}

struct mdcTagName_t
{
	char name[MAX_QPATH];
};

struct mdcTagFrame_t
{
	short xyz[3]; // divide by 64
	short angles[3]; // euler in z x y order...   deg = * (360.0 / 32767.0) .. rad = * (PI / 32767.0)
};

struct mdcFrame_t
{
	float3 bboxMin;
	float3 bboxMax;
	float3 localOrigin;
	float radius;
	char name[16];
};

struct mdcSurface_t
{
	byte ident[4];
	char name[MAX_QPATH];
	unsigned int flags;
	unsigned int numCompFrames;
	unsigned int numBaseFrames;
	unsigned int numShaders;
	unsigned int numVerts;
	unsigned int numTriangles;
	unsigned int ofsTriangles;
	unsigned int ofsShaders;
	unsigned int ofsSt;
	unsigned int ofsXyzNormals;
	unsigned int ofsCompVerts;
	unsigned int ofsFrameBaseFrames;
	unsigned int ofsFrameCompFrames;
	unsigned int ofsEnd;
};
	
void istream_read_mdcSurface(PointerInputStream& inputStream, mdcSurface_t& surface)
{
  inputStream.read(surface.ident, 4);
  inputStream.read(reinterpret_cast<byte*>(surface.name), MAX_QPATH);
  surface.flags = istream_read_uint32_le(inputStream);
  surface.numCompFrames = istream_read_uint32_le(inputStream);
  surface.numBaseFrames = istream_read_uint32_le(inputStream);
  surface.numShaders = istream_read_uint32_le(inputStream);
  surface.numVerts = istream_read_uint32_le(inputStream);
  surface.numTriangles = istream_read_uint32_le(inputStream);
  surface.ofsTriangles = istream_read_uint32_le(inputStream);
  surface.ofsShaders = istream_read_uint32_le(inputStream);
  surface.ofsSt = istream_read_uint32_le(inputStream);
  surface.ofsXyzNormals = istream_read_uint32_le(inputStream);
  surface.ofsCompVerts = istream_read_uint32_le(inputStream);
  surface.ofsFrameBaseFrames = istream_read_uint32_le(inputStream);
  surface.ofsFrameCompFrames = istream_read_uint32_le(inputStream);
  surface.ofsEnd = istream_read_uint32_le(inputStream);
}

struct mdcHeader_t
{
	byte ident[4];
	unsigned int version;
	char name[MAX_QPATH];
	unsigned int flags;
	unsigned int numFrames;
	unsigned int numTags;
	unsigned int numSurfaces;
	unsigned int numSkins;
	unsigned int ofsFrames;
	unsigned int ofsTagNames;
	unsigned int ofsTags;
	unsigned int ofsSurfaces;
	unsigned int ofsEnd;
};

void istream_read_mdcHeader(PointerInputStream& inputStream, mdcHeader_t& header)
{
  inputStream.read(header.ident, 4);
  header.version = istream_read_uint32_le(inputStream);
  inputStream.read(reinterpret_cast<byte*>(header.name), MAX_QPATH);
  header.flags = istream_read_uint32_le(inputStream);
  header.numFrames = istream_read_uint32_le(inputStream);
  header.numTags = istream_read_uint32_le(inputStream);
  header.numSurfaces = istream_read_uint32_le(inputStream);
  header.numSkins = istream_read_uint32_le(inputStream);
  header.ofsFrames = istream_read_uint32_le(inputStream);
  header.ofsTagNames = istream_read_uint32_le(inputStream);
  header.ofsTags = istream_read_uint32_le(inputStream);
  header.ofsSurfaces = istream_read_uint32_le(inputStream);
  header.ofsEnd = istream_read_uint32_le(inputStream);
}

unsigned int MDCSurface_read(Surface& surface, const byte* buffer)
{
  mdcSurface_t mdcSurface;
  {
    PointerInputStream inputStream(buffer);
    istream_read_mdcSurface(inputStream, mdcSurface);
  }

  {
    surface.vertices().reserve(mdcSurface.numVerts);

    PointerInputStream xyzStream(buffer + mdcSurface.ofsXyzNormals);
    PointerInputStream stStream(buffer + mdcSurface.ofsSt);
    // read verts into vertex array - xyz, st, normal
    for(std::size_t i = 0; i < mdcSurface.numVerts; i++)
	  {
      mdcXyzNormal_t mdcXyzNormal;
      istream_read_mdcXyzNormal(xyzStream, mdcXyzNormal);
      mdcSt_t mdcSt;
      istream_read_mdcSt(stStream, mdcSt);

      surface.vertices().push_back(
        ArbitraryMeshVertex(
          Vertex3f( mdcXyzNormal.xyz[0] * MDC_XYZ_SCALE, mdcXyzNormal.xyz[1] * MDC_XYZ_SCALE, mdcXyzNormal.xyz[2] * MDC_XYZ_SCALE),
          DecodeNormal(reinterpret_cast<byte*>(&mdcXyzNormal.normal)),
          TexCoord2f(mdcSt.st[0], mdcSt.st[1])
        )
      );
    }
  }

  {
 	  surface.indices().reserve(mdcSurface.numTriangles * 3);

    PointerInputStream triangleStream(buffer + mdcSurface.ofsTriangles);

    for(std::size_t i = 0; i < mdcSurface.numTriangles; i++)
    {
      mdcTriangle_t triangle;
      istream_read_mdcTriangle(triangleStream, triangle);
      surface.indices().insert(triangle.indexes[0]);
      surface.indices().insert(triangle.indexes[1]);
      surface.indices().insert(triangle.indexes[2]);
    }
  }

  {
    mdcShader_t shader;
    PointerInputStream inputStream(buffer + mdcSurface.ofsShaders);
    istream_read_mdcShader(inputStream, shader);
    surface.setShader(shader.name);
  }
	
	surface.updateAABB();

  return mdcSurface.ofsEnd;
}

void MDCModel_read(Model& model, const byte* buffer)
{
  mdcHeader_t header;
  {
    PointerInputStream inputStream(buffer);
    istream_read_mdcHeader(inputStream, header);
  }

  const byte* surfacePosition = buffer + header.ofsSurfaces;

  for(std::size_t i = 0; i < header.numSurfaces; i++)
	{
    surfacePosition += MDCSurface_read(model.newSurface(), surfacePosition);
  }

  model.updateAABB();
}

scene::Node& MDCModel_new(const byte* buffer)
{
  ModelNode* modelNode = new ModelNode();
  MDCModel_read(modelNode->model(), buffer);
  return modelNode->node();
}

scene::Node& MDCModel_default()
{
  ModelNode* modelNode = new ModelNode();
  Model_constructNull(modelNode->model());
  return modelNode->node();
}

scene::Node& MDCModel_fromBuffer(unsigned char* buffer)
{
  if (!ident_equal(buffer, MDC_IDENT))
  {
	  globalErrorStream() << "MDC read error: incorrect ident\n";
    return MDCModel_default();
  }
  else
  {
    return MDCModel_new(buffer);
  }
}

scene::Node& loadMDCModel(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return MDCModel_fromBuffer(buffer.buffer);
}

