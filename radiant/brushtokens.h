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

#if !defined(INCLUDED_BRUSHTOKENS_H)
#define INCLUDED_BRUSHTOKENS_H

#include "stringio.h"
#include "stream/stringstream.h"
#include "brush.h"

inline bool FaceShader_importContentsFlagsValue(FaceShader& faceShader, Tokeniser& tokeniser)
{
  // parse the optional contents/flags/value
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, faceShader.m_flags.m_contentFlags));
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, faceShader.m_flags.m_surfaceFlags));
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, faceShader.m_flags.m_value));
  return true;
}

inline bool FaceTexdef_importTokens(FaceTexdef& texdef, Tokeniser& tokeniser)
{
  // parse texdef
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.shift[0]));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.shift[1]));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.rotate));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.scale[0]));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.scale[1]));

  ASSERT_MESSAGE(texdef_sane(texdef.m_projection.m_texdef), "FaceTexdef_importTokens: bad texdef");
  return true;
}

inline bool FaceTexdef_BP_importTokens(FaceTexdef& texdef, Tokeniser& tokeniser)
{
  // parse alternate texdef
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
  {
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[0][0]));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[0][1]));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[0][2]));
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  }
  {
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[1][0]));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[1][1]));
    RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_brushprimit_texdef.coords[1][2]));
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  }
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  return true;
}

inline bool FaceTexdef_HalfLife_importTokens(FaceTexdef& texdef, Tokeniser& tokeniser)
{
  // parse texdef
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "["));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_s.x()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_s.y()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_s.z()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.shift[0]));
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "]"));
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "["));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_t.x()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_t.y()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_basis_t.z()));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.shift[1]));
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "]"));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.rotate));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.scale[0]));
  RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, texdef.m_projection.m_texdef.scale[1]));

  texdef.m_projection.m_texdef.rotate = -texdef.m_projection.m_texdef.rotate;

  ASSERT_MESSAGE(texdef_sane(texdef.m_projection.m_texdef), "FaceTexdef_importTokens: bad texdef");
  return true;
}

inline bool FacePlane_importTokens(FacePlane& facePlane, Tokeniser& tokeniser)
{
  // parse planepts
  for(std::size_t i = 0; i<3; i++)
  {
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
    for(std::size_t j = 0; j < 3; ++j)
    {
      RETURN_FALSE_IF_FAIL(Tokeniser_getDouble(tokeniser, facePlane.planePoints()[i][j]));
    }
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  }
  facePlane.MakePlane();
  return true;
}

inline bool FacePlane_Doom3_importTokens(FacePlane& facePlane, Tokeniser& tokeniser)
{
  Plane3 plane;
  // parse plane equation
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
  RETURN_FALSE_IF_FAIL(Tokeniser_getDouble(tokeniser, plane.a));
  RETURN_FALSE_IF_FAIL(Tokeniser_getDouble(tokeniser, plane.b));
  RETURN_FALSE_IF_FAIL(Tokeniser_getDouble(tokeniser, plane.c));
  RETURN_FALSE_IF_FAIL(Tokeniser_getDouble(tokeniser, plane.d));
  plane.d = -plane.d;
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));

  facePlane.setDoom3Plane(plane);
  return true;
}

inline bool FaceShader_Doom3_importTokens(FaceShader& faceShader, Tokeniser& tokeniser)
{
  const char *shader = tokeniser.getToken();
  if(shader == 0)
  {
    Tokeniser_unexpectedError(tokeniser, shader, "#shader-name");
    return false;
  }
  if(string_equal(shader, "_emptyname"))
  {
    shader = texdef_name_default();
  }
  faceShader.setShader(shader);
  return true;
}

inline bool FaceShader_importTokens(FaceShader& faceShader, Tokeniser& tokeniser)
{
  const char* texture = tokeniser.getToken();
  if(texture == 0)
  {
    Tokeniser_unexpectedError(tokeniser, texture, "#texture-name");
    return false;
  }
  if(string_equal(texture, "NULL"))
  {
    faceShader.setShader(texdef_name_default());
  }
  else
  {
    StringOutputStream shader(string_length(GlobalTexturePrefix_get()) + string_length(texture));
    shader << GlobalTexturePrefix_get() << texture;
    faceShader.setShader(shader.c_str());
  }
  return true;
}




class Doom3FaceTokenImporter
{
  Face& m_face;
public:
  Doom3FaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_Doom3_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_BP_importTokens(m_face.getTexdef(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_Doom3_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importContentsFlagsValue(m_face.getShader(), tokeniser));

    m_face.getTexdef().m_projectionInitialised = true;
    m_face.getTexdef().m_scaleApplied = true;

    return true;
  }
};

class Quake4FaceTokenImporter
{
  Face& m_face;
public:
  Quake4FaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_Doom3_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_BP_importTokens(m_face.getTexdef(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_Doom3_importTokens(m_face.getShader(), tokeniser));

    m_face.getTexdef().m_projectionInitialised = true;
    m_face.getTexdef().m_scaleApplied = true;

    return true;
  }
};

class Quake2FaceTokenImporter
{
  Face& m_face;
public:
  Quake2FaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_importTokens(m_face.getTexdef(), tokeniser));
    if(Tokeniser_nextTokenIsDigit(tokeniser))
    {
      m_face.getShader().m_flags.m_specified = true;
      RETURN_FALSE_IF_FAIL(FaceShader_importContentsFlagsValue(m_face.getShader(), tokeniser));
    }
    m_face.getTexdef().m_scaleApplied = true;
    return true;
  }
};

class Quake3FaceTokenImporter
{
  Face& m_face;
public:
  Quake3FaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_importTokens(m_face.getTexdef(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importContentsFlagsValue(m_face.getShader(), tokeniser));
    m_face.getTexdef().m_scaleApplied = true;
    return true;
  }
};

class Quake3BPFaceTokenImporter
{
  Face& m_face;
public:
  Quake3BPFaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_BP_importTokens(m_face.getTexdef(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importContentsFlagsValue(m_face.getShader(), tokeniser));

    m_face.getTexdef().m_projectionInitialised = true;
    m_face.getTexdef().m_scaleApplied = true;

    return true;
  }
};

class QuakeFaceTokenImporter
{
  Face& m_face;
public:
  QuakeFaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_importTokens(m_face.getTexdef(), tokeniser));
    m_face.getTexdef().m_scaleApplied = true;
    return true;
  }
};

class HalfLifeFaceTokenImporter
{
  Face& m_face;
public:
  HalfLifeFaceTokenImporter(Face& face) : m_face(face)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(FacePlane_importTokens(m_face.getPlane(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceShader_importTokens(m_face.getShader(), tokeniser));
    RETURN_FALSE_IF_FAIL(FaceTexdef_HalfLife_importTokens(m_face.getTexdef(), tokeniser));
    m_face.getTexdef().m_scaleApplied = true;
    return true;
  }
};


inline void FacePlane_Doom3_exportTokens(const FacePlane& facePlane, TokenWriter& writer)
{
  // write plane equation
  writer.writeToken("(");
  writer.writeFloat(facePlane.getDoom3Plane().a);
  writer.writeFloat(facePlane.getDoom3Plane().b);
  writer.writeFloat(facePlane.getDoom3Plane().c);
  writer.writeFloat(-facePlane.getDoom3Plane().d);
  writer.writeToken(")");
}

inline void FacePlane_exportTokens(const FacePlane& facePlane, TokenWriter& writer)
{
  // write planepts
  for(std::size_t i=0; i<3; i++)
  {
    writer.writeToken("(");
    for(std::size_t j=0; j<3; j++)
    {
      writer.writeFloat(Face::m_quantise(facePlane.planePoints()[i][j]));
    }
    writer.writeToken(")");
  }
}

inline void FaceTexdef_BP_exportTokens(const FaceTexdef& faceTexdef, TokenWriter& writer)
{
  // write alternate texdef
  writer.writeToken("(");
  {
    writer.writeToken("(");
    for(std::size_t i=0;i<3;i++)
    {
      writer.writeFloat(faceTexdef.m_projection.m_brushprimit_texdef.coords[0][i]);
    }
    writer.writeToken(")");
  }
  {
    writer.writeToken("(");
    for(std::size_t i=0;i<3;i++)
    {
      writer.writeFloat(faceTexdef.m_projection.m_brushprimit_texdef.coords[1][i]);
    }
    writer.writeToken(")");
  }
  writer.writeToken(")");
}

inline void FaceTexdef_exportTokens(const FaceTexdef& faceTexdef, TokenWriter& writer)
{
  ASSERT_MESSAGE(texdef_sane(faceTexdef.m_projection.m_texdef), "FaceTexdef_exportTokens: bad texdef");
  // write texdef
  writer.writeFloat(faceTexdef.m_projection.m_texdef.shift[0]);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.shift[1]);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.rotate);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.scale[0]);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.scale[1]);
}

inline void FaceTexdef_HalfLife_exportTokens(const FaceTexdef& faceTexdef, TokenWriter& writer)
{
  ASSERT_MESSAGE(texdef_sane(faceTexdef.m_projection.m_texdef), "FaceTexdef_exportTokens: bad texdef");
  // write texdef
  writer.writeToken("[");
  writer.writeFloat(faceTexdef.m_projection.m_basis_s.x());
  writer.writeFloat(faceTexdef.m_projection.m_basis_s.y());
  writer.writeFloat(faceTexdef.m_projection.m_basis_s.z());
  writer.writeFloat(faceTexdef.m_projection.m_texdef.shift[0]);
  writer.writeToken("]");
  writer.writeToken("[");
  writer.writeFloat(faceTexdef.m_projection.m_basis_t.x());
  writer.writeFloat(faceTexdef.m_projection.m_basis_t.y());
  writer.writeFloat(faceTexdef.m_projection.m_basis_t.z());
  writer.writeFloat(faceTexdef.m_projection.m_texdef.shift[1]);
  writer.writeToken("]");
  writer.writeFloat(-faceTexdef.m_projection.m_texdef.rotate);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.scale[0]);
  writer.writeFloat(faceTexdef.m_projection.m_texdef.scale[1]);
}

inline void FaceShader_ContentsFlagsValue_exportTokens(const FaceShader& faceShader, TokenWriter& writer)
{
  // write surface flags
  writer.writeInteger(faceShader.m_flags.m_contentFlags);
  writer.writeInteger(faceShader.m_flags.m_surfaceFlags);
  writer.writeInteger(faceShader.m_flags.m_value);
}

inline void FaceShader_exportTokens(const FaceShader& faceShader, TokenWriter& writer)
{
  // write shader name  
  if(string_empty(shader_get_textureName(faceShader.getShader())))
  {
    writer.writeToken("NULL");
  }
  else
  {
    writer.writeToken(shader_get_textureName(faceShader.getShader()));
  }
}

inline void FaceShader_Doom3_exportTokens(const FaceShader& faceShader, TokenWriter& writer)
{
  // write shader name  
  if(string_empty(shader_get_textureName(faceShader.getShader())))
  {
    writer.writeString("_emptyname");
  }
  else
  {
    writer.writeString(faceShader.getShader());
  }
}

class Doom3FaceTokenExporter
{
  const Face& m_face;
public:
  Doom3FaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_Doom3_exportTokens(m_face.getPlane(), writer);
    FaceTexdef_BP_exportTokens(m_face.getTexdef(), writer);
    FaceShader_Doom3_exportTokens(m_face.getShader(), writer);
    FaceShader_ContentsFlagsValue_exportTokens(m_face.getShader(), writer);
    writer.nextLine();
  }
};

class Quake4FaceTokenExporter
{
  const Face& m_face;
public:
  Quake4FaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_Doom3_exportTokens(m_face.getPlane(), writer);
    FaceTexdef_BP_exportTokens(m_face.getTexdef(), writer);
    FaceShader_Doom3_exportTokens(m_face.getShader(), writer);
    writer.nextLine();
  }
};

class Quake2FaceTokenExporter
{
  const Face& m_face;
public:
  Quake2FaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_exportTokens(m_face.getPlane(), writer);
    FaceShader_exportTokens(m_face.getShader(), writer);
    FaceTexdef_exportTokens(m_face.getTexdef(), writer);
    if(m_face.getShader().m_flags.m_specified || m_face.isDetail())
    {
      FaceShader_ContentsFlagsValue_exportTokens(m_face.getShader(), writer);
    }
    writer.nextLine();
  }
};

class Quake3FaceTokenExporter
{
  const Face& m_face;
public:
  Quake3FaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_exportTokens(m_face.getPlane(), writer);
    FaceShader_exportTokens(m_face.getShader(), writer);
    FaceTexdef_exportTokens(m_face.getTexdef(), writer);
    FaceShader_ContentsFlagsValue_exportTokens(m_face.getShader(), writer);
    writer.nextLine();
  }
};

class Quake3BPFaceTokenExporter
{
  const Face& m_face;
public:
  Quake3BPFaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_exportTokens(m_face.getPlane(), writer);
    FaceTexdef_BP_exportTokens(m_face.getTexdef(), writer);
    FaceShader_exportTokens(m_face.getShader(), writer);
    FaceShader_ContentsFlagsValue_exportTokens(m_face.getShader(), writer);
    writer.nextLine();
  }
};

class QuakeFaceTokenExporter
{
  const Face& m_face;
public:
  QuakeFaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_exportTokens(m_face.getPlane(), writer);
    FaceShader_exportTokens(m_face.getShader(), writer);
    FaceTexdef_exportTokens(m_face.getTexdef(), writer);
    writer.nextLine();
  }
};

class HalfLifeFaceTokenExporter
{
  const Face& m_face;
public:
  HalfLifeFaceTokenExporter(const Face& face) : m_face(face)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    FacePlane_exportTokens(m_face.getPlane(), writer);
    FaceShader_exportTokens(m_face.getShader(), writer);
    FaceTexdef_HalfLife_exportTokens(m_face.getTexdef(), writer);
    writer.nextLine();
  }
};


class BrushTokenImporter : public MapImporter
{
  Brush& m_brush;

public:
  BrushTokenImporter(Brush& brush) : m_brush(brush)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    if(Brush::m_type == eBrushTypeQuake3BP || Brush::m_type == eBrushTypeDoom3 || Brush::m_type == eBrushTypeQuake4)
    {
      tokeniser.nextLine();
      RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "{"));
    }
    while(1)
    {
      // check for end of brush
      tokeniser.nextLine();
      const char* token = tokeniser.getToken();
      if(string_equal(token, "}"))
      {
        break;
      }

      tokeniser.ungetToken();

      m_brush.push_back(FaceSmartPointer(new Face(&m_brush)));

      //!todo BP support
      tokeniser.nextLine();

      Face& face = *m_brush.back();

      switch(Brush::m_type)
      {
      case eBrushTypeDoom3:
        {
          Doom3FaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeQuake4:
        {
          Quake4FaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeQuake2:
        {
          Quake2FaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeQuake3:
        {
          Quake3FaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeQuake3BP:
        {
          Quake3BPFaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeQuake:
        {
          QuakeFaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      case eBrushTypeHalfLife:
        {
          HalfLifeFaceTokenImporter importer(face);
          RETURN_FALSE_IF_FAIL(importer.importTokens(tokeniser));
        }
        break;
      }
      face.planeChanged();
    }
    if(Brush::m_type == eBrushTypeQuake3BP || Brush::m_type == eBrushTypeDoom3 || Brush::m_type == eBrushTypeQuake4)
    {
      tokeniser.nextLine();
      RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "}"));
    }

    m_brush.planeChanged();
    m_brush.shaderChanged();

    return true;
  }
};


class BrushTokenExporter : public MapExporter
{
  const Brush& m_brush;

public:
  BrushTokenExporter(const Brush& brush) : m_brush(brush)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    m_brush.evaluateBRep(); // ensure b-rep is up-to-date, so that non-contributing faces can be identified.

    if(!m_brush.hasContributingFaces())
    {
      return;
    }

    writer.writeToken("{");
    writer.nextLine();

    if(Brush::m_type == eBrushTypeQuake3BP)
    {
      writer.writeToken("brushDef");
      writer.nextLine();
      writer.writeToken("{");
      writer.nextLine();
    }
    
    if(Brush::m_type == eBrushTypeDoom3 || Brush::m_type == eBrushTypeQuake4)
    {
      writer.writeToken("brushDef3");
      writer.nextLine();
      writer.writeToken("{");
      writer.nextLine();
    }

    for(Brush::const_iterator i = m_brush.begin(); i != m_brush.end(); ++i)
    {
      const Face& face = *(*i);

      if(face.contributes())
      {
        switch(Brush::m_type)
        {
        case eBrushTypeDoom3:
          {
            Doom3FaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeQuake4:
          {
            Quake4FaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeQuake2:
          {
            Quake2FaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeQuake3:
          {
            Quake3FaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeQuake3BP:
          {
            Quake3BPFaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeQuake:
          {
            QuakeFaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        case eBrushTypeHalfLife:
          {
            HalfLifeFaceTokenExporter exporter(face);
            exporter.exportTokens(writer);
          }
          break;
        }
      }
    }

    if(Brush::m_type == eBrushTypeQuake3BP || Brush::m_type == eBrushTypeDoom3 || Brush::m_type == eBrushTypeQuake4)
    {
      writer.writeToken("}");
      writer.nextLine();
    }

    writer.writeToken("}");
    writer.nextLine();
  }
};

    
#endif
