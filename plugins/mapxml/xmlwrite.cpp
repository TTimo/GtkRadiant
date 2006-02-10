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

//
// writes xml tree format from internal objects
//

#include "xmlwrite.h"

#include "xml/xmlwriter.h"
#include "scenelib.h"
#include "entityxml.h"

inline XMLExporter* Node_getXMLExporter(scene::Node& node)
{
  return NodeTypeCast<XMLExporter>::cast(node);
}


class write_all : public scene::Traversable::Walker
{
  XMLImporter& m_importer;
public:
  write_all(XMLImporter& importer) : m_importer(importer)
  {
  }
  bool pre(scene::Node& node) const
  {
    Entity* entity = Node_getEntity(node);
    if(entity != 0)
    {
      m_importer.write("\n", 1);
      StaticElement element("entity");
      m_importer.pushElement(element);
      entity_export exporter(*entity);
      exporter.exportXML(m_importer);
    }
    else
    {
      XMLExporter* exporter = Node_getXMLExporter(node);
      if(exporter != 0)
      {
        m_importer.write("\n", 1);
        exporter->exportXML(m_importer);
        m_importer.write("\n", 1);
      }
    }
    return true;
  }
  void post(scene::Node& node) const
  {
    if(Node_getEntity(node) != 0)
    {
      m_importer.write("\n", 1);
      m_importer.popElement("entity");
    }
  }
};

void Map_Write(scene::Node& root, GraphTraversalFunc traverse, TextOutputStream& out)
{
  XMLStreamWriter writer(out);
  writer.write("\n", 1);
  {
    StaticElement element("mapq3");
    writer.pushElement(element);

    traverse(root, write_all(writer));

    writer.write("\n", 1);
    writer.popElement(element.name());
  }
}
