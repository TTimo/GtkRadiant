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

///\file
///\brief EntityClass plugin that supports the .ent xml entity-definition format.
/// 
/// the .ent xml format expresses entity-definitions.
/// 
/// <!-- defines an entity which cannot have brushes grouped with it -->
/// <point name="[name of entity type]" colour="[RGB floating-point colour shown in editor]"
///   box="[minXYZ maxXYZ floating point bounding-box]" model="[model path]">
///   <!-- attribute definitions go here -->
/// </point>
/// 
/// <!-- defines an entity which can have brushes grouped with it -->
/// <group name="[name of entity type]" colour="[RGB floating-point colour shown in editor]">
///   <!-- attribute definitions go here -->
/// </group>
/// 
/// 
/// the attributes of an entity type are defined like this:
/// 
///   <[name of attribute type]
///     key="[entity key name]"
///     name="[name shown in gui]"
///     value="[default entity key value]"
///   >[comment text shown in gui]</[name of attribute type]>
/// 
/// each attribute type has a specialised attribute-editor GUI
/// 
/// currently-supported attribute types:
/// 
/// string          a string
/// array           an array of strings - value is a semi-colon-delimited string
/// integer         an integer value
/// boolean         an integer - shows as a checkbox - true = non-zero
/// integer2        two integer values
/// integer3        three integer values
/// real3           three floating-point values
/// angle           specialisation of real - Yaw angle
/// direction       specialisation of real - Yaw angle, -1 = down, -2 = up
/// angles          specialisation of real3 - Pitch Yaw Roll
/// color           specialisation of real3 - RGB floating-point colour
/// target          a string that uniquely identifies an entity or group of entities
/// targetname      a string that uniquely identifies an entity or group of entities
/// sound           the VFS path to a sound file
/// texture         the VFS path to a texture file or a shader name
/// model           the VFS path to a model file
/// skin            the VFS path to a skin file
/// 
/// 
/// flag attributes define a flag in the "spawnflags" key:          
///
///   <flag
///     key="[flag name]"
///     name="[name shown in gui]"
///     bit="[bit-index in spawnflags]"
///   >[comment text shown in gui]<flag>
///
/// the default value for a flag bit is always 0.
///
///
/// List attributes have a set of valid values.
/// Create new list attribute types like this:
/// 
/// <list name="[name of list attribute type]">
///   <item name="[first name shown in menu]" value="[entity key value]"/>
///   <item name="[second name shown in menu]" value="[entity key value]"/>
/// </list>
/// 
/// these can then be used as attribute types.
/// 
/// 
/// An attribute definition should specify a default value that corresponds
/// with the default value given by the game. If the default value is not 
/// specified in the attribute definition, it is assumed to be an empty string.
///
/// If the currently-selected entity in Radiant does not specify a value for
/// the key of an attribute, the default value from the attribute-definition
/// will be displayed in the attribute-editor and used when visualising the
/// entity in the preview windows. E.g. the Doom3 "light" entity has a
/// "light_radius" key. Light entities without a "light_radius" key are
/// displayed in Doom3 with a radius of 300. The default value for the
/// "light_radius" attribute definition should be specified as "300 300 300".
///




#include "eclass_xml.h"

#include "ieclass.h"
#include "irender.h"
#include "ifilesystem.h"
#include "iarchive.h"

#include "xml/xmlparser.h"
#include "generic/object.h"
#include "generic/reference.h"
#include "stream/stringstream.h"
#include "stream/textfilestream.h"
#include "os/path.h"
#include "eclasslib.h"
#include "modulesystem/moduleregistry.h"
#include "stringio.h"

#define PARSE_ERROR(elementName, name) makeQuoted(elementName) << " is not a valid child of " << makeQuoted(name)

class IgnoreBreaks
{
public:
  const char* m_first;
  const char* m_last;
  IgnoreBreaks(const char* first, const char* last) : m_first(first), m_last(last)
  {
  }
};

template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const IgnoreBreaks& ignoreBreaks)
{
  for(const char* i = ignoreBreaks.m_first; i != ignoreBreaks.m_last; ++i)
  {
    if(*i != '\n')
    {
      ostream << *i;
    }
  }
  return ostream;
}

namespace
{

class TreeXMLImporter : public TextOutputStream
{
public:
  virtual TreeXMLImporter& pushElement(const XMLElement& element) = 0;
  virtual void popElement(const char* name) = 0;
};

template<typename Type>
class Storage
{
  char m_storage[sizeof(Type)];
public:
  Type& get()
  {
    return *reinterpret_cast<Type*>(m_storage);
  }
  const Type& get() const
  {
    return *reinterpret_cast<const Type*>(m_storage);
  }
};

class BreakImporter : public TreeXMLImporter
{
public:
  BreakImporter(StringOutputStream& comment)
  {
    comment << '\n';
  }
  static const char* name()
  {
    return "n";
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    ERROR_MESSAGE(PARSE_ERROR(element.name(), name()));
    return *this;
  }
  void popElement(const char* elementName)
  {
    ERROR_MESSAGE(PARSE_ERROR(elementName, name()));
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
};

class AttributeImporter : public TreeXMLImporter
{
  StringOutputStream& m_comment;

public:
  AttributeImporter(StringOutputStream& comment, EntityClass* entityClass, const XMLElement& element) : m_comment(comment)
  {
    const char* type = element.name();
    const char* key = element.attribute("key");
    const char* name = element.attribute("name");
    const char* value = element.attribute("value");

    ASSERT_MESSAGE(!string_empty(key), "key attribute not specified");
    ASSERT_MESSAGE(!string_empty(name), "name attribute not specified");

    if(string_equal(type, "flag"))
    {
      std::size_t bit = atoi(element.attribute("bit"));
      ASSERT_MESSAGE(bit < MAX_FLAGS, "invalid flag bit");
      ASSERT_MESSAGE(string_empty(entityClass->flagnames[bit]), "non-unique flag bit");
      strcpy(entityClass->flagnames[bit], key);
    }

    m_comment << key;
    m_comment << " : ";

    EntityClass_insertAttribute(*entityClass, key, EntityClassAttribute(type, name, value));
  }
  ~AttributeImporter()
  {
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    ERROR_MESSAGE(PARSE_ERROR(element.name(), "attribute"));
    return *this;
  }
  void popElement(const char* elementName)
  {
    ERROR_MESSAGE(PARSE_ERROR(elementName, "attribute"));
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return m_comment.write(data, length);
  }
};

bool attributeSupported(const char* name)
{
  return string_equal(name, "real")
    || string_equal(name, "integer")
    || string_equal(name, "boolean")
    || string_equal(name, "string")
    || string_equal(name, "array")
    || string_equal(name, "flag")
    || string_equal(name, "real3")
    || string_equal(name, "integer3")
    || string_equal(name, "direction")
    || string_equal(name, "angle")
    || string_equal(name, "angles")
    || string_equal(name, "color")
    || string_equal(name, "target")
    || string_equal(name, "targetname")
    || string_equal(name, "sound")
    || string_equal(name, "texture")
    || string_equal(name, "model")
    || string_equal(name, "skin")
    || string_equal(name, "integer2");
}

typedef std::map<CopiedString, ListAttributeType> ListAttributeTypes;

bool listAttributeSupported(ListAttributeTypes& listTypes, const char* name)
{
  return listTypes.find(name) != listTypes.end();
}


class ClassImporter : public TreeXMLImporter
{
  EntityClassCollector& m_collector;
  EntityClass* m_eclass;
  StringOutputStream m_comment;
  Storage<AttributeImporter> m_attribute;
  ListAttributeTypes& m_listTypes;

public:
  ClassImporter(EntityClassCollector& collector, ListAttributeTypes& listTypes, const XMLElement& element) : m_collector(collector), m_listTypes(listTypes)
  {
    m_eclass = Eclass_Alloc();
    m_eclass->free = &Eclass_Free;

    const char* name = element.attribute("name");
    ASSERT_MESSAGE(!string_empty(name), "name attribute not specified for class");
    m_eclass->m_name = name;

    const char* color = element.attribute("color");
    ASSERT_MESSAGE(!string_empty(name), "color attribute not specified for class " << name);
    string_parse_vector3(color, m_eclass->color);
    eclass_capture_state(m_eclass);

    const char* model = element.attribute("model");
    if(!string_empty(model))
    {
      StringOutputStream buffer(256);
      buffer << PathCleaned(model);
      m_eclass->m_modelpath = buffer.c_str();
    }

    const char* type = element.name();
    if(string_equal(type, "point"))
    {
      const char* box = element.attribute("box");
      ASSERT_MESSAGE(!string_empty(box), "box attribute not found for class " << name);
      m_eclass->fixedsize = true;
      string_parse_vector(box, &m_eclass->mins.x(), &m_eclass->mins.x() + 6);
    }
  }
  ~ClassImporter()
  {
    m_eclass->m_comments = m_comment.c_str();
    m_collector.insert(m_eclass);

    for(ListAttributeTypes::iterator i = m_listTypes.begin(); i != m_listTypes.end(); ++i)
    {
      m_collector.insert((*i).first.c_str(), (*i).second);
    }
  }
  static const char* name()
  {
    return "class";
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    if(attributeSupported(element.name()) || listAttributeSupported(m_listTypes, element.name()))
    {
      constructor(m_attribute.get(), makeReference(m_comment), m_eclass, element);
      return m_attribute.get();
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(element.name(), name()));
      return *this;
    }
  }
  void popElement(const char* elementName)
  {
    if(attributeSupported(elementName) || listAttributeSupported(m_listTypes, elementName))
    {
      destructor(m_attribute.get());
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(elementName, name()));
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return m_comment.write(data, length);
  }
};

class ItemImporter : public TreeXMLImporter
{
public:
  ItemImporter(ListAttributeType& list, const XMLElement& element)
  {
    const char* name = element.attribute("name");
    const char* value = element.attribute("value");
    list.push_back(name, value);
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    ERROR_MESSAGE(PARSE_ERROR(element.name(), "item"));
    return *this;
  }
  void popElement(const char* elementName)
  {
    ERROR_MESSAGE(PARSE_ERROR(elementName, "item"));
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
};

bool isItem(const char* name)
{
  return string_equal(name, "item");
}

class ListAttributeImporter : public TreeXMLImporter
{
  ListAttributeType* m_listType;
  Storage<ItemImporter> m_item;
public:
  ListAttributeImporter(ListAttributeTypes& listTypes, const XMLElement& element)
  {
    const char* name = element.attribute("name");
    m_listType = &listTypes[name];
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    if(isItem(element.name()))
    {
      constructor(m_item.get(), makeReference(*m_listType), element);
      return m_item.get();
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(element.name(), "list"));
      return *this;
    }
  }
  void popElement(const char* elementName)
  {
    if(isItem(elementName))
    {
      destructor(m_item.get());
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(elementName, "list"));
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
};

bool classSupported(const char* name)
{
  return string_equal(name, "group")
    || string_equal(name, "point");
}

bool listSupported(const char* name)
{
  return string_equal(name, "list");
}

class ClassesImporter : public TreeXMLImporter
{
  EntityClassCollector& m_collector;
  Storage<ClassImporter> m_class;
  Storage<ListAttributeImporter> m_list;
  ListAttributeTypes m_listTypes;

public:
  ClassesImporter(EntityClassCollector& collector) : m_collector(collector)
  {
  }
  static const char* name()
  {
    return "classes";
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    if(classSupported(element.name()))
    {
      constructor(m_class.get(), makeReference(m_collector), makeReference(m_listTypes), element);
      return m_class.get();
    }
    else if(listSupported(element.name()))
    {
      constructor(m_list.get(), makeReference(m_listTypes), element);
      return m_list.get();
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(element.name(), name()));
      return *this;
    }
  }
  void popElement(const char* elementName)
  {
    if(classSupported(elementName))
    {
      destructor(m_class.get());
    }
    else if(listSupported(elementName))
    {
      destructor(m_list.get());
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(elementName, name()));
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
  }
};

class EclassXMLImporter : public TreeXMLImporter
{
  EntityClassCollector& m_collector;
  Storage<ClassesImporter> m_classes;

public:
  EclassXMLImporter(EntityClassCollector& collector) : m_collector(collector)
  {
  }
  static const char* name()
  {
    return "classes";
  }
  TreeXMLImporter& pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), ClassesImporter::name()))
    {
      constructor(m_classes.get(), makeReference(m_collector));
      return m_classes.get();
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(element.name(), name()));
      return *this;
    }
  }
  void popElement(const char* elementName)
  {
    if(string_equal(elementName, ClassesImporter::name()))
    {
      destructor(m_classes.get());
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR(elementName, name()));
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    return length;
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
    m_importers.push_back(makeReference(m_importers.back().get().pushElement(element)));
  }
  void popElement(const char* name)
  {
    m_importers.pop_back();
    m_importers.back().get().popElement(name);
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return m_importers.back().get().write(buffer, length);
  }
};



const char* GetExtension()
{
  return "ent";
}
void ScanFile(EntityClassCollector& collector, const char *filename)
{
  TextFileInputStream inputFile(filename);
  if(!inputFile.failed())
  {
    XMLStreamParser parser(inputFile);

    EclassXMLImporter importer(collector);
    TreeXMLImporterStack stack(importer);
    parser.exportXML(stack);
  }
}


}

#include "modulesystem/singletonmodule.h"

class EntityClassXMLDependencies : public GlobalFileSystemModuleRef, public GlobalShaderCacheModuleRef
{
};

class EclassXMLAPI
{
  EntityClassScanner m_eclassxml;
public:
  typedef EntityClassScanner Type;
  STRING_CONSTANT(Name, "xml");

  EclassXMLAPI()
  {
    m_eclassxml.scanFile = &ScanFile;
    m_eclassxml.getExtension = &GetExtension;
  }
  EntityClassScanner* getTable()
  {
    return &m_eclassxml;
  }
};

typedef SingletonModule<EclassXMLAPI, EntityClassXMLDependencies> EclassXMLModule;
typedef Static<EclassXMLModule> StaticEclassXMLModule;
StaticRegisterModule staticRegisterEclassXML(StaticEclassXMLModule::instance());
