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
// parses xml tree format into internal objects
//

#include "xmlparse.h"

#include <vector>

#include "ientity.h"
#include "ibrush.h"
#include "ipatch.h"
#include "ieclass.h"
#include "eclasslib.h"

#include "xml/xmlparser.h"
#include "scenelib.h"
#include "generic/reference.h"
#include "generic/object.h"


#define PARSE_ERROR "XML PARSE ERROR"


inline XMLImporter* Node_getXMLImporter(scene::Node& node)
{
  return NodeTypeCast<XMLImporter>::cast(node);
}


scene::Node& createPrimitive(const char* name)
{
  if(string_equal(name, "brush"))
  {
    return GlobalBrushCreator().createBrush();
  }
  else if(string_equal(name, "patch"))
  {
    return GlobalPatchCreator().createPatch();
  }

  ASSERT_MESSAGE(0, PARSE_ERROR << ": primitive type not supported: \"" << name << "\"\n");
  scene::Node* node = 0;
  return *node;
}

class TreeXMLImporter : public XMLImporter
{
public:
  virtual TreeXMLImporter& child() = 0;
};

class SubPrimitiveImporter : public TreeXMLImporter
{
  XMLImporter* m_importer;
public:
  SubPrimitiveImporter(XMLImporter* importer) : m_importer(importer)
  {
  }
  void pushElement(const XMLElement& element)
  {
    m_importer->pushElement(element);
  }
  void popElement(const char* name)
  {
    m_importer->popElement(name);
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return m_importer->write(buffer, length);
  }
  SubPrimitiveImporter& child()
  {
    return *this;
  }
};

class PrimitiveImporter : public TreeXMLImporter
{
  scene::Node& m_parent;
  XMLImporter* m_importer;
  char m_child[sizeof(SubPrimitiveImporter)];

  SubPrimitiveImporter& subprimitive()
  {
    return *reinterpret_cast<SubPrimitiveImporter*>(m_child);
  }
public:
  PrimitiveImporter(scene::Node& parent) : m_parent(parent), m_importer(0)
  {
  }
  void pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), "epair"))
    {
      ASSERT_MESSAGE(string_equal(element.name(), "epair"), PARSE_ERROR);
      Node_getEntity(m_parent)->setKeyValue(element.attribute("key"), element.attribute("value"));
    }
    else
    {
      NodeSmartReference node(createPrimitive(element.name()));

      m_importer = Node_getXMLImporter(node);

      constructor(subprimitive(), m_importer);

      m_importer->pushElement(element);

      Node_getTraversable(m_parent)->insert(node);
    }
  }
  void popElement(const char* name)
  {
    if(string_equal(name, "epair"))
    {
    }
    else
    {
      m_importer->popElement(name);

      destructor(subprimitive());
      m_importer = 0;
    }
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return m_importer->write(buffer, length);
  }
  TreeXMLImporter& child()
  {
    return subprimitive();
  }
};

class EntityImporter : public TreeXMLImporter
{
  scene::Node& m_parent;
  char m_node[sizeof(NodeSmartReference)];
  char m_child[sizeof(PrimitiveImporter)];
  EntityCreator& m_entityTable;

  NodeSmartReference& node()
  {
    return *reinterpret_cast<NodeSmartReference*>(m_node);
  }
  PrimitiveImporter& primitive()
  {
    return *reinterpret_cast<PrimitiveImporter*>(m_child);
  }

public:
  EntityImporter(scene::Node& parent, EntityCreator& entityTable) : m_parent(parent), m_entityTable(entityTable)
  {
  }
  void pushElement(const XMLElement& element)
  {
    ASSERT_MESSAGE(string_equal(element.name(), "entity"), PARSE_ERROR);
    constructor(node(), NodeSmartReference(m_entityTable.createEntity(GlobalEntityClassManager().findOrInsert("", true))));
    constructor(primitive(), makeReference(node().get()));
  }
  void popElement(const char* name)
  {
    ASSERT_MESSAGE(string_equal(name, "entity"), PARSE_ERROR);
    NodeSmartReference entity(m_entityTable.createEntity(GlobalEntityClassManager().findOrInsert(Node_getEntity(node())->getKeyValue("classname"), node_is_group(node()))));

    {
      EntityCopyingVisitor visitor(*Node_getEntity(entity));
      Node_getEntity(node())->forEachKeyValue(visitor);
    }

    if(Node_getTraversable(entity) != 0 && !Node_getEntity(entity)->getEntityClass().fixedsize)
    {
      parentBrushes(node(), entity);
    }

    Node_getTraversable(m_parent)->insert(entity);

    destructor(primitive());
    destructor(node());
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return length;
  }
  TreeXMLImporter& child()
  {
    return primitive();
  }
};

class MapQ3Importer : public TreeXMLImporter
{
  scene::Node& m_root;
  char m_child[sizeof(EntityImporter)];
  EntityCreator& m_entityTable;

  EntityImporter& getEntity()
  {
    return *reinterpret_cast<EntityImporter*>(m_child);
  }
public:
  MapQ3Importer(scene::Node& root, EntityCreator& entityTable) : m_root(root), m_entityTable(entityTable)
  {
  }
  void pushElement(const XMLElement& element)
  {
    ASSERT_MESSAGE(string_equal(element.name(), "mapq3"), PARSE_ERROR);
    constructor(getEntity(), makeReference(m_root), makeReference(m_entityTable));
  }
  void popElement(const char* name)
  {
    ASSERT_MESSAGE(string_equal(name, "mapq3"), PARSE_ERROR);
    destructor(getEntity());
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
  TreeXMLImporter& child()
  {
    return getEntity();
  }
};

class TreeXMLImporterStack : public XMLImporter
{
  std::vector< Reference<TreeXMLImporter> > m_importers;
public:
  TreeXMLImporterStack(TreeXMLImporter& importer)
  {
    m_importers.push_back(makeReference(importer));
  }
  void pushElement(const XMLElement& element)
  {
    m_importers.back().get().pushElement(element);
    m_importers.push_back(makeReference(m_importers.back().get().child()));
  }
  void popElement(const char* name)
  {
    m_importers.pop_back();
    m_importers.back().get().popElement(name);
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return (*(m_importers.end() - 2)).get().write(buffer, length);
  }
};


void Map_Read(scene::Node& root, TextInputStream& in, EntityCreator& entityTable)
{
  XMLStreamParser parser(in);

  MapQ3Importer importer(root, entityTable);
  TreeXMLImporterStack stack(importer);
  parser.exportXML(stack);
}
