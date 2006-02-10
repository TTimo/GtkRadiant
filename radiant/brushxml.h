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

#if !defined(INCLUDED_BRUSHXML_H)
#define INCLUDED_BRUSHXML_H

#include "stream/stringstream.h"
#include "xml/xmlelement.h"

#include "brush.h"

inline void FaceTexdef_BP_importXML(FaceTexdef& texdef, const char* xmlContent)
{
  StringTokeniser content(xmlContent);

  texdef.m_projection.m_brushprimit_texdef.coords[0][0] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_brushprimit_texdef.coords[0][1] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_brushprimit_texdef.coords[0][2] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_brushprimit_texdef.coords[1][0] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_brushprimit_texdef.coords[1][1] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_brushprimit_texdef.coords[1][2] = static_cast<float>(atof(content.getToken()));
}
inline void FaceTexdef_importXML(FaceTexdef& texdef, const char* xmlContent)
{
  StringTokeniser content(xmlContent);

  texdef.m_projection.m_texdef.shift[0] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_texdef.shift[1] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_texdef.rotate = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_texdef.scale[0] = static_cast<float>(atof(content.getToken()));
  texdef.m_projection.m_texdef.scale[1] = static_cast<float>(atof(content.getToken()));

  ASSERT_MESSAGE(texdef_sane(texdef.m_projection.m_texdef), "FaceTexdef_importXML: bad texdef");
}

inline void FacePlane_importXML(FacePlane& facePlane, const char* xmlContent)
{
  StringTokeniser content(xmlContent);

  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      facePlane.planePoints()[i][j] = atof(content.getToken());
    }
  }
  facePlane.MakePlane();
}


class FaceXMLImporter
{
  struct xml_state_t
  {
    enum EState
    {
      eDefault,
      ePlanePts,
      eTexdef,
      eBPMatrix,
      eFlags,
      eShader,
    };

    EState m_state;
    StringOutputStream m_content;

    xml_state_t(EState state)
      : m_state(state)
    {}

    EState state() const
    {
      return m_state;
    }
    const char* content() const
    {
      return m_content.c_str();
    }
    std::size_t write(const char* buffer, std::size_t length)
    {
      return m_content.write(buffer, length);
    }
  };

  std::vector<xml_state_t> m_xml_state;
  Face& m_face;
public:
  FaceXMLImporter(Face& face) : m_face(face)
  {
    m_xml_state.push_back(xml_state_t::eDefault);
  }
  ~FaceXMLImporter()
  {
    m_face.planeChanged();
  }

  void pushElement(const XMLElement& element)
  {
    ASSERT_MESSAGE(m_xml_state.back().state() == xml_state_t::eDefault, "parse error");

    if(strcmp(element.name(), "planepts") == 0)
    {
      m_xml_state.push_back(xml_state_t::ePlanePts);
    }
    else if(strcmp(element.name(), "texdef") == 0)
    {
      m_xml_state.push_back(xml_state_t::eTexdef);
    }
    else if(strcmp(element.name(), "bpmatrix") == 0)
    {
      m_xml_state.push_back(xml_state_t::eBPMatrix);
    }
    else if(strcmp(element.name(), "flags") == 0)
    {
      m_xml_state.push_back(xml_state_t::eFlags);
    }
    else if(strcmp(element.name(), "shader") == 0)
    {
      m_xml_state.push_back(xml_state_t::eShader);
    }
  }
  void popElement(const char* name)
  {
    ASSERT_MESSAGE(m_xml_state.back().state() != xml_state_t::eDefault, "parse error");

    switch(m_xml_state.back().state())
    {
    case xml_state_t::ePlanePts:
      {
        FacePlane_importXML(m_face.getPlane(), m_xml_state.back().content());
      }
      break;
    case xml_state_t::eTexdef:
      {
        FaceTexdef_importXML(m_face.getTexdef(), m_xml_state.back().content());
      }
      break;
    case xml_state_t::eBPMatrix:
      {
        FaceTexdef_BP_importXML(m_face.getTexdef(), m_xml_state.back().content());
      }
      break;
   case xml_state_t::eFlags:
     {
        StringTokeniser content(m_xml_state.back().content());

        m_face.getShader().m_flags.m_contentFlags = atoi(content.getToken());
        m_face.getShader().m_flags.m_surfaceFlags = atoi(content.getToken());
        m_face.getShader().m_flags.m_value = atoi(content.getToken());
      }
      break;
    case xml_state_t::eShader:
      {
        m_face.getShader().setShader(m_xml_state.back().content());
      }
      break;
    default:
      break;
    }

    m_xml_state.pop_back();
  }
  std::size_t write(const char* data, std::size_t length)
  {
    ASSERT_MESSAGE(!m_xml_state.empty(), "parse error");
    return m_xml_state.back().write(data, length);
  }
};


inline void FaceTexdef_exportXML(const FaceTexdef& texdef, XMLImporter& importer)
{
  StaticElement element("texdef");
  importer.pushElement(element);

  ASSERT_MESSAGE(texdef_sane(texdef.m_projection.m_texdef), "FaceTexdef_exportXML: bad texdef");

  importer << texdef.m_projection.m_texdef.shift[0]
    << ' ' << texdef.m_projection.m_texdef.shift[1]
    << ' ' << texdef.m_projection.m_texdef.rotate
    << ' ' << texdef.m_projection.m_texdef.scale[0]
    << ' ' << texdef.m_projection.m_texdef.scale[1];

  importer.popElement(element.name());
}
inline void FaceTexdef_BP_exportXML(const FaceTexdef& texdef, XMLImporter& importer)
{
  StaticElement element("texdef");
  importer.pushElement(element);

  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      importer << texdef.m_projection.m_brushprimit_texdef.coords[i][j] << ' ';
    }
  }

  importer.popElement(element.name());
}
inline void FaceShader_ContentsFlagsValue_exportXML(const FaceShader& faceShader, XMLImporter& importer)
{
  StaticElement element("flags");
  importer.pushElement(element);

  {
    importer << faceShader.m_flags.m_contentFlags
      << ' ' << faceShader.m_flags.m_surfaceFlags
      << ' ' << faceShader.m_flags.m_value;
  }

  importer.popElement(element.name());
}

inline void FacePlane_exportXML(const FacePlane& facePlane, XMLImporter& importer)
{
  StaticElement element("planepts");
  importer.pushElement(element);

  {
    // write planepts
    for (int i=0 ; i<3 ; i++)
    {
      for (int j=0 ; j<3 ; j++)
      {
        importer << Face::m_quantise(facePlane.planePoints()[i][j]) << ' ';
      }
    }
  }

  importer.popElement(element.name());
}

class FaceXMLExporter
{
  const Face& m_face;
public:
  FaceXMLExporter(const Face& face) : m_face(face)
  {
  }
  void exportXML(XMLImporter& importer)
  {
    bool bAlternateTexdef = (Face::m_type == eBrushTypeQuake3BP || Face::m_type == eBrushTypeDoom3 || Face::m_type == eBrushTypeQuake4);
 
    // write shader
    {
      StaticElement element("shader"); 
      importer.pushElement(element);
      importer << m_face.getShader().getShader();
      importer.popElement(element.name());
    }

    FacePlane_exportXML(m_face.getPlane(), importer);

    if(!bAlternateTexdef)
    {
      FaceTexdef_exportXML(m_face.getTexdef(), importer);
    }
    else
    {
      FaceTexdef_BP_exportXML(m_face.getTexdef(), importer);
    }

    FaceShader_ContentsFlagsValue_exportXML(m_face.getShader(), importer);
  }
};


class BrushXMLImporter : public XMLImporter
{
  class xml_state_t
  {
  public:
    enum EState
    {
      eDefault,
      eBrush,
      eFace,
    };

  private:
    EState m_state;

  public:
    xml_state_t(EState state)
      : m_state(state)
    {
    }
    EState state() const
    {
      return m_state;
    }
  };

  std::vector<xml_state_t> m_xml_state;
  char m_faceImporter[sizeof(FaceXMLImporter)];
  Brush& m_brush;

  FaceXMLImporter& faceImporter()
  {
    return *reinterpret_cast<FaceXMLImporter*>(m_faceImporter);
  }

public:
  BrushXMLImporter(Brush& brush) : m_brush(brush)
  {
    m_xml_state.push_back(xml_state_t::eDefault);
  }
  void pushElement(const XMLElement& element)
  {
    switch(m_xml_state.back().state())
    {
    case xml_state_t::eDefault:
      ASSERT_MESSAGE(strcmp(element.name(), "brush") == 0, "parse error");
      m_xml_state.push_back(xml_state_t::eBrush);
      break;
    case xml_state_t::eBrush:
      ASSERT_MESSAGE(strcmp(element.name(), "plane") == 0, "parse error");
      m_xml_state.push_back(xml_state_t::eFace);
      m_brush.push_back(FaceSmartPointer(new Face(&m_brush)));
      constructor(faceImporter(), makeReference(*m_brush.back()));
      m_brush.planeChanged();
      m_brush.shaderChanged();
      break;
    case xml_state_t::eFace:
      m_xml_state.push_back(xml_state_t::eFace);
      faceImporter().pushElement(element);
      break;
    }
  }
  void popElement(const char* name)
  {
    ASSERT_MESSAGE(!m_xml_state.empty(), "parse error");
    m_xml_state.pop_back();

    switch(m_xml_state.back().state())
    {
    case xml_state_t::eDefault:
      break;
    case xml_state_t::eBrush:
      destructor(faceImporter());
      break;
    case xml_state_t::eFace:
      faceImporter().popElement(name);
      break;
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    switch(m_xml_state.back().state())
    {
    case xml_state_t::eFace:
      return faceImporter().write(data, length);
      break;
    default:
      break;
    }
    return length;
  }
};

class BrushXMLExporter : public XMLExporter
{
  const Brush& m_brush;

public:
  BrushXMLExporter(const Brush& brush) : m_brush(brush)
  {
  }
  void exportXML(XMLImporter& importer)
  {
    m_brush.evaluateBRep(); // ensure b-rep is up-to-date, so that non-contributing faces can be identified.
    ASSERT_MESSAGE(m_brush.hasContributingFaces(), "exporting an empty brush");

    const StaticElement brushElement("brush");
    importer.pushElement(brushElement);

    for(Brush::const_iterator i = m_brush.begin(); i != m_brush.end(); ++i)
    {
      if((*i)->contributes())
      {
        const StaticElement element("plane");
        importer.pushElement(element);
        FaceXMLExporter(*(*i)).exportXML(importer);
        importer.popElement(element.name());
      }
    }

    importer.popElement(brushElement.name());
  }
};


#endif
