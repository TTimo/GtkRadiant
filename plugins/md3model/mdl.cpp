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

#include "mdl.h"

#include "ifilesystem.h"
#include "imodel.h"

#include "imagelib.h"
#include "bytestreamutils.h"

#include "model.h"
#include "ident.h"
#include "mdlnormals.h"
#include "mdlformat.h"

void istream_read_mdlHeader(PointerInputStream& inputStream, mdlHeader_t& header)
{
	inputStream.read(header.ident, 4);
	header.version = istream_read_int32_le(inputStream);
	header.scale[0] = istream_read_float32_le(inputStream);
	header.scale[1] = istream_read_float32_le(inputStream);
	header.scale[2] = istream_read_float32_le(inputStream);
	header.scale_origin[0] = istream_read_float32_le(inputStream);
	header.scale_origin[1] = istream_read_float32_le(inputStream);
	header.scale_origin[2] = istream_read_float32_le(inputStream);
	header.boundingradius = istream_read_float32_le(inputStream);
	header.eyeposition[0] = istream_read_float32_le(inputStream);
	header.eyeposition[1] = istream_read_float32_le(inputStream);
	header.eyeposition[2] = istream_read_float32_le(inputStream);
	header.numskins = istream_read_int32_le(inputStream);
	header.skinwidth = istream_read_int32_le(inputStream);
	header.skinheight = istream_read_int32_le(inputStream);
	header.numverts = istream_read_int32_le(inputStream);
	header.numtris = istream_read_int32_le(inputStream);
	header.numframes = istream_read_int32_le(inputStream);
	header.synctype = istream_read_int32_le(inputStream);
	header.flags = istream_read_int32_le(inputStream);
	header.size = istream_read_float32_le(inputStream);
}

inline ArbitraryMeshVertex MDLVertex_construct(const mdlHeader_t& header, const mdlXyzNormal_t& xyz, const mdlSt_t& st, bool facesfront)
{
  return ArbitraryMeshVertex(
    Vertex3f(
      xyz.v[0] * header.scale[0] + header.scale_origin[0],
      xyz.v[1] * header.scale[1] + header.scale_origin[1],
      xyz.v[2] * header.scale[2] + header.scale_origin[2]
    ),
    Normal3f(
      g_mdl_normals[xyz.lightnormalindex][0],
      g_mdl_normals[xyz.lightnormalindex][1],
      g_mdl_normals[xyz.lightnormalindex][2]
    ),
    TexCoord2f(
    ((float)st.s / header.skinwidth) + ((st.onseam == MDL_ONSEAM && !facesfront) ? 0.5f : 0.0f),
      (float)st.t / header.skinheight
    )
  );
}

class mdlVertex_t
{
public:
  inline mdlVertex_t(int vertindex, int facesfront)
    : m_vertindex(vertindex), m_facesfront(facesfront)
  {}
  inline bool operator<(const mdlVertex_t& other) const
  {
    if(m_facesfront < other.m_facesfront)
      return true;
    if(other.m_facesfront < m_facesfront)
      return false;

    if(m_vertindex < other.m_vertindex)
      return true;
    if(other.m_vertindex < m_vertindex)
      return false;

    return false;
  }
  inline bool operator==(const mdlVertex_t& other) const
  {
    return m_vertindex == other.m_vertindex
      && m_facesfront == other.m_facesfront;
  }

  int m_vertindex;
  int m_facesfront;
};

typedef const mdlTriangle_t* mdlTriangleIterator;

void MDLSurface_read(Surface& surface, const byte* buffer, const char* name)
{
  mdlHeader_t header;

  PointerInputStream inputStream(buffer);
  istream_read_mdlHeader(inputStream, header);

  for(int i = 0; i < header.numskins; ++i)
  {
    switch(istream_read_int32_le(inputStream))
    {
    case MDL_SKIN_SINGLE:
      inputStream.seek(header.skinwidth * header.skinheight);
      break;
    case MDL_SKIN_GROUP:
      int numskins = istream_read_int32_le(inputStream);
      inputStream.seek(numskins * (4 + (header.skinwidth * header.skinheight)));
      break;
    }
  }

  Array<mdlSt_t> mdlSts(header.numverts);
  for(Array<mdlSt_t>::iterator i = mdlSts.begin(); i != mdlSts.end(); ++i)
  {
	  (*i).onseam = istream_read_int32_le(inputStream);
	  (*i).s = istream_read_int32_le(inputStream);
	  (*i).t = istream_read_int32_le(inputStream);
  }

  Array<mdlTriangle_t> mdlTriangles(header.numtris);
  for(Array<mdlTriangle_t>::iterator i = mdlTriangles.begin(); i != mdlTriangles.end(); ++i)
  {
	  (*i).facesfront = istream_read_int32_le(inputStream);
	  (*i).vertindex[0] = istream_read_int32_le(inputStream);
	  (*i).vertindex[1] = istream_read_int32_le(inputStream);
	  (*i).vertindex[2] = istream_read_int32_le(inputStream);
  }

  {
    bool found = false;
    for(int i = 0; i < header.numframes && found == false; i++)
    {
      switch(istream_read_int32_le(inputStream))
      {
      case MDL_FRAME_SINGLE:
        inputStream.seek(MDL_FRAME_SIZE);
        found = true;
        break;
      case MDL_FRAME_GROUP:
        int numframes = istream_read_int32_le(inputStream);
        inputStream.seek((MDL_XYZNORMAL_SIZE * 2) + (numframes * 4));
        found = true;
        break;
      }
    }
  }

  Array<mdlXyzNormal_t> mdlXyzNormals(header.numtris);
  for(Array<mdlXyzNormal_t>::iterator i = mdlXyzNormals.begin(); i != mdlXyzNormals.end(); ++i)
  {
	  inputStream.read((*i).v, 3);
	  inputStream.read(&(*i).lightnormalindex, 1);
  }

  {
    VertexBuffer<mdlVertex_t> mdl_vertices;

    {
      UniqueVertexBuffer<mdlVertex_t> inserter(mdl_vertices);
      for(Array<mdlTriangle_t>::iterator i = mdlTriangles.begin(); i != mdlTriangles.end(); ++i)
      {
        surface.indices().insert(inserter.insert(mdlVertex_t((*i).vertindex[0], (*i).facesfront)));
        surface.indices().insert(inserter.insert(mdlVertex_t((*i).vertindex[1], (*i).facesfront)));
        surface.indices().insert(inserter.insert(mdlVertex_t((*i).vertindex[2], (*i).facesfront)));
      }
    }

    {
      surface.vertices().reserve(mdl_vertices.size());

      for(VertexBuffer<mdlVertex_t>::iterator i = mdl_vertices.begin(); i != mdl_vertices.end(); ++i)
      {
        surface.vertices().push_back(MDLVertex_construct(header, mdlXyzNormals[(*i).m_vertindex], mdlSts[(*i).m_vertindex], (*i).m_facesfront == MDL_FACES_FRONT));
      }
    }
  }

  surface.setShader(name);
  surface.updateAABB();
}

void MDLModel_read(Model& model, const byte* buffer, const char* name)
{
  MDLSurface_read(model.newSurface(), buffer, name);
  model.updateAABB();
}

scene::Node& MDLModel_new(const byte* buffer, const char* name)
{
  ModelNode* modelNode = new ModelNode();
  MDLModel_read(modelNode->model(), buffer, name);
  return modelNode->node();
}

scene::Node& MDLModel_default()
{
  ModelNode* modelNode = new ModelNode();
  Model_constructNull(modelNode->model());
  return modelNode->node();
}

scene::Node& MDLModel_fromBuffer(unsigned char* buffer, const char* name)
{
  if (!ident_equal(buffer, MDL_IDENT))
  {
	  globalErrorStream() << "MDL read error: incorrect ident\n";
    return MDLModel_default();
  }
  else
  {
    return MDLModel_new(buffer, name);
  }
}

scene::Node& loadMDLModel(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return MDLModel_fromBuffer(buffer.buffer, file.getName());
}

