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

#if !defined(INCLUDED_ENTITYXML_H)
#define INCLUDED_ENTITYXML_H

#include "ientity.h"
#include "xml/ixml.h"
#include "xml/xmlelement.h"

class entity_import : public XMLImporter
{
  Entity& m_entity;
public:
  entity_import(Entity& entity)
    : m_entity(entity)
  {
  }
  void pushElement(const XMLElement& element)
  {
    if(strcmp(element.name(), "epair") == 0)
      m_entity.setKeyValue(element.attribute("key"), element.attribute("value")); 
  }
  void popElement(const char* name)
  {
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
};

class entity_export : public XMLExporter
{
  class ExportXMLVisitor : public Entity::Visitor
  {
    XMLImporter& m_importer;
  public:
    ExportXMLVisitor(XMLImporter& importer) : m_importer(importer)
    {
    }
    void visit(const char* key, const char* value)
    {
      StaticElement element("epair");
      element.insertAttribute("key", key);
      element.insertAttribute("value", value);
      m_importer.pushElement(element);
      m_importer.popElement(element.name());
    }
  };

  const Entity& m_entity;

public:
  entity_export(const Entity& entity) : m_entity(entity)
  {
  }
  void exportXML(XMLImporter& observer)
  {
    ExportXMLVisitor visitor(observer);

    m_entity.forEachKeyValue(visitor);
  }
};

inline void entity_copy(Entity& entity, const Entity& other)
{
  entity_export exporter(other);
  entity_import importer(entity);
  exporter.exportXML(importer);
}

template<typename EntityType>
class EntityConstruction
{
public:
  typedef EntityClass* type;
  static type get(const EntityType& entity)
  {
    return &entity.getEntity().getEntityClass();
  }
  static void copy(EntityType& entity, const EntityType& other)
  {
    entity_copy(entity.getEntity(), other.getEntity());
  }
};



#endif
