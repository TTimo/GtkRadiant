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

#include "md2.h"

#include "ifilesystem.h"
#include "imodel.h"

#include "imagelib.h"
#include "bytestreamutils.h"

#include "model.h"
#include "ident.h"
#include "mdlnormals.h"

typedef unsigned char byte;

/*
========================================================================

.MD2 triangle model file format

========================================================================
*/
#define MD2_NUMVERTEXNORMALS 162
#define	MD2_MAX_SKINNAME	64

const unsigned char MD2_IDENT[4] = { 'I', 'D', 'P', '2', };
#define MD2_VERSION	8

#define	MD2_MAX_TRIANGLES	4096
#define MD2_MAX_VERTS		2048
#define MD2_MAX_FRAMES		512
#define MD2_MAX_MD2SKINS	32
#define	MD2_MAX_SKINNAME	64

typedef struct
{
	short	s;
	short	t;
} md2St_t;

void istream_read_md2St(PointerInputStream& inputStream, md2St_t& st)
{
  st.s = istream_read_int16_le(inputStream);
  st.t = istream_read_int16_le(inputStream);
}

typedef struct 
{
	short	index_xyz[3];
	short	index_st[3];
} md2Triangle_t;

void istream_read_md2Triangle(PointerInputStream& inputStream, md2Triangle_t& triangle)
{
  triangle.index_xyz[0] = istream_read_int16_le(inputStream);
  triangle.index_xyz[1] = istream_read_int16_le(inputStream);
  triangle.index_xyz[2] = istream_read_int16_le(inputStream);
  triangle.index_st[0] = istream_read_int16_le(inputStream);
  triangle.index_st[1] = istream_read_int16_le(inputStream);
  triangle.index_st[2] = istream_read_int16_le(inputStream);
}

typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} md2XyzNormal_t;

void istream_read_md2XyzNormal(PointerInputStream& inputStream, md2XyzNormal_t& xyz)
{
  inputStream.read(xyz.v, 3);
  inputStream.read(&xyz.lightnormalindex, 1);
}

#define MD2_XYZNORMAL_V0   0
#define MD2_XYZNORMAL_V1   1
#define MD2_XYZNORMAL_V2   2
#define MD2_XYZNORMAL_LNI  3
#define MD2_XYZNORMAL_SIZE 4

typedef struct
{
	float		scale[3];	// multiply byte verts by this
	float		translate[3];	// then add this
	char		name[16];	// frame name from grabbing
	md2XyzNormal_t	verts[1];	// variable sized
} md2Frame_t;

void istream_read_md2Frame(PointerInputStream& inputStream, md2Frame_t& frame)
{
  frame.scale[0] = istream_read_float32_le(inputStream);
  frame.scale[1] = istream_read_float32_le(inputStream);
  frame.scale[2] = istream_read_float32_le(inputStream);
  frame.translate[0] = istream_read_float32_le(inputStream);
  frame.translate[1] = istream_read_float32_le(inputStream);
  frame.translate[2] = istream_read_float32_le(inputStream);
  inputStream.read(reinterpret_cast<unsigned char*>(frame.name), 16);
}


// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.


typedef struct
{
	unsigned char ident[4];
	int			version;

	int			skinwidth;
	int			skinheight;
	int			framesize;		// byte size of each frame

	int			num_skins;
	int			num_xyz;
	int			num_st;			// greater than num_xyz for seams
	int			num_tris;
	int			num_glcmds;		// dwords in strip/fan command list
	int			num_frames;

	int			ofs_skins;		// each skin is a MAX_SKINNAME string
	int			ofs_st;			// byte offset from start for md2St_t
	int			ofs_tris;		// offset for md2triangle_t
	int			ofs_frames;		// offset for first md2Frame_t
	int			ofs_glcmds;	
	int			ofs_end;		// end of file

} md2Header_t;

void istream_read_md2Header(PointerInputStream& inputStream, md2Header_t& header)
{
  inputStream.read(header.ident, 4);
  header.version = istream_read_int32_le(inputStream);
  header.skinwidth = istream_read_int32_le(inputStream);
  header.skinheight = istream_read_int32_le(inputStream);
  header.framesize = istream_read_int32_le(inputStream);
  header.num_skins = istream_read_int32_le(inputStream);
  header.num_xyz = istream_read_int32_le(inputStream);
  header.num_st = istream_read_int32_le(inputStream);
  header.num_tris = istream_read_int32_le(inputStream);
  header.num_glcmds = istream_read_int32_le(inputStream);
  header.num_frames = istream_read_int32_le(inputStream);
  header.ofs_skins = istream_read_int32_le(inputStream);
  header.ofs_st = istream_read_int32_le(inputStream);
  header.ofs_tris = istream_read_int32_le(inputStream);
  header.ofs_frames = istream_read_int32_le(inputStream);
  header.ofs_glcmds = istream_read_int32_le(inputStream);
  header.ofs_end = istream_read_int32_le(inputStream);
}


ArbitraryMeshVertex MD2Vertex_construct(const md2Header_t* pHeader, const md2Frame_t* pFrame, const md2XyzNormal_t* xyz, const md2St_t* st)
{
  return ArbitraryMeshVertex(
    Vertex3f(
      xyz->v[0] * pFrame->scale[0] + pFrame->translate[0],
      xyz->v[1] * pFrame->scale[1] + pFrame->translate[1],
      xyz->v[2] * pFrame->scale[2] + pFrame->translate[2]
    ),
    Normal3f(
      g_mdl_normals[xyz->lightnormalindex][0],
      g_mdl_normals[xyz->lightnormalindex][1],
      g_mdl_normals[xyz->lightnormalindex][2]
    ),
    TexCoord2f(
      (float)st->s / pHeader->skinwidth,
      (float)st->t / pHeader->skinheight
    )
  );
}

void MD2Surface_read(Surface& surface, const byte* buffer)
{
  md2Header_t header;
  {
    PointerInputStream inputStream(buffer);
    istream_read_md2Header(inputStream, header);
  }

  {

    md2Frame_t frame;
    PointerInputStream frameStream(buffer + header.ofs_frames);
    istream_read_md2Frame(frameStream, frame);


    surface.indices().reserve(header.num_tris * 3);

    Array<md2XyzNormal_t> md2Xyz(header.num_xyz);
    for(Array<md2XyzNormal_t>::iterator i = md2Xyz.begin(); i != md2Xyz.end(); ++i)
    {
      istream_read_md2XyzNormal(frameStream, *i);
    }

    Array<md2St_t> md2St(header.num_st);
    PointerInputStream stStream(buffer + header.ofs_st);
    for(Array<md2St_t>::iterator i = md2St.begin(); i != md2St.end(); ++i)
    {
      istream_read_md2St(stStream, *i);
    }

    UniqueVertexBuffer<ArbitraryMeshVertex> inserter(surface.vertices());
    inserter.reserve(header.num_st);

    PointerInputStream triangleStream(buffer + header.ofs_tris);
	  for(int i = 0; i < header.num_tris; ++i)
    {
      md2Triangle_t triangle;
      istream_read_md2Triangle(triangleStream, triangle);
      surface.indices().insert(inserter.insert(MD2Vertex_construct(&header, &frame, &md2Xyz[triangle.index_xyz[0]], &md2St[triangle.index_st[0]])));
      surface.indices().insert(inserter.insert(MD2Vertex_construct(&header, &frame, &md2Xyz[triangle.index_xyz[1]], &md2St[triangle.index_st[1]])));
      surface.indices().insert(inserter.insert(MD2Vertex_construct(&header, &frame, &md2Xyz[triangle.index_xyz[2]], &md2St[triangle.index_st[2]])));
    }
  }

  char skinname[MD2_MAX_SKINNAME];
  PointerInputStream inputStream(buffer + header.ofs_skins);
  inputStream.read(reinterpret_cast<byte*>(skinname), MD2_MAX_SKINNAME);
  surface.setShader(skinname);
  surface.updateAABB();
}

void MD2Model_read(Model& model, const byte* buffer)
{
  MD2Surface_read(model.newSurface(), buffer);
  model.updateAABB();
}

scene::Node& MD2Model_new(const byte* buffer)
{
  ModelNode* modelNode = new ModelNode();
  MD2Model_read(modelNode->model(), buffer);
  return modelNode->node();
}

scene::Node& MD2Model_default()
{
  ModelNode* modelNode = new ModelNode();
  Model_constructNull(modelNode->model());
  return modelNode->node();
}

scene::Node& MD2Model_fromBuffer(unsigned char* buffer)
{
  if (!ident_equal(buffer, MD2_IDENT))
  {
	  globalErrorStream() << "MD2 read error: incorrect ident\n";
    return MD2Model_default();
  }
  else
  {
    return MD2Model_new(buffer);
  }
}

scene::Node& loadMD2Model(ArchiveFile& file)
{
  ScopedArchiveBuffer buffer(file);
  return MD2Model_fromBuffer(buffer.buffer);
}
