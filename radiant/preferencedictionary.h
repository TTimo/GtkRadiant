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

#if !defined(INCLUDED_PREFERENCEDICTIONARY_H)
#define INCLUDED_PREFERENCEDICTIONARY_H

#include "preferencesystem.h"
#include "xml/ixml.h"
#include "stream/stringstream.h"
#include "generic/callback.h"
#include "versionlib.h"
#include <map>

class PreferenceDictionary : public PreferenceSystem
{
  class PreferenceEntry
  {
    StringImportCallback m_importer;
    StringExportCallback m_exporter;
  public:
    PreferenceEntry(const StringImportCallback& importer, const StringExportCallback& exporter)
      : m_importer(importer), m_exporter(exporter)
    {
    }
    void importString(const char* string)
    {
      m_importer(string);
    }
    void exportString(const StringImportCallback& importer)
    {
      m_exporter(importer);
    }
  };

  typedef std::map<CopiedString, PreferenceEntry> PreferenceEntries;
  PreferenceEntries m_preferences;

  typedef std::map<CopiedString, CopiedString> PreferenceCache;
  PreferenceCache m_cache;

public:
  typedef PreferenceEntries::iterator iterator;

  iterator begin()
  {
    return m_preferences.begin();
  }
  iterator end()
  {
    return m_preferences.end();
  }
  iterator find(const char* name)
  {
    return m_preferences.find(name);
  }

  void registerPreference(const char* name, const StringImportCallback& importer, const StringExportCallback& exporter)
  {
    m_preferences.insert(PreferenceEntries::value_type(name, PreferenceEntry(importer, exporter)));
    PreferenceCache::iterator i = m_cache.find(name);
    if(i != m_cache.end())
    {
      importer((*i).second.c_str());
      m_cache.erase(i);
    }
  }

  void importPref(const char* name, const char* value)
  {
    PreferenceEntries::iterator i = m_preferences.find(name);
    if(i != m_preferences.end())
    {
      (*i).second.importString(value);
    }
    else
    {
      m_cache.insert(PreferenceCache::value_type(name, value));
    }
  }
};

inline void XMLPreference_importString(XMLImporter& importer, const char* value)
{
  importer.write(value, string_length(value));
}
typedef ReferenceCaller1<XMLImporter, const char*, XMLPreference_importString> XMLPreferenceImportStringCaller;

class XMLPreferenceDictionaryExporter : public XMLExporter
{
  class XMLQPrefElement : public XMLElement
  {
    const char* m_version;
  public:
    XMLQPrefElement(const char* version) : m_version(version)
    {
    }
    const char* name() const
    {
      return "qpref";
    }
    const char* attribute(const char* name) const
    {
      if(string_equal(name, "version"))
      {
        return m_version;
      }
      return "";
    }
    void forEachAttribute(XMLAttrVisitor& visitor) const
    {
      visitor.visit("version", m_version);
    }
  };
    
  class XMLPreferenceElement : public XMLElement
  {
    const char* m_name;
  public:
    XMLPreferenceElement(const char* name)
      : m_name(name)
    {
    }
    const char* name() const
    {
      return "epair";
    }
    const char* attribute(const char* name) const
    {
      if(string_equal(name, "name"))
        return m_name;
      return "";
    }
    void forEachAttribute(XMLAttrVisitor& visitor) const
    {
      visitor.visit("name", m_name);
    }
  };
  
  typedef PreferenceDictionary PreferenceEntries;
  PreferenceEntries& m_preferences;
  const char* m_version;
public:
  XMLPreferenceDictionaryExporter(PreferenceDictionary& preferences, const char* version)
    : m_preferences(preferences), m_version(version)
  {
  }

  void exportXML(XMLImporter& importer)
  {
    importer.write("\n", 1);

    XMLQPrefElement qpref_element(m_version);
    importer.pushElement(qpref_element);
    importer.write("\n", 1);

    for(PreferenceEntries::iterator i = m_preferences.begin(); i != m_preferences.end(); ++i)
    {
      XMLPreferenceElement epair_element((*i).first.c_str());

      importer.pushElement(epair_element);

      (*i).second.exportString(XMLPreferenceImportStringCaller(importer));

      importer.popElement(epair_element.name());
      importer.write("\n", 1);
    }

    importer.popElement(qpref_element.name());
    importer.write("\n", 1);
  }
};

class XMLPreferenceDictionaryImporter : public XMLImporter
{
  struct xml_state_t
  {
    enum ETag
    {
      tag_qpref,
      tag_qpref_ignore,
      tag_epair,
      tag_epair_ignore
    };

    xml_state_t(ETag tag)
      : m_tag(tag)
    {
    }

    ETag m_tag;
    CopiedString m_name;
    StringOutputStream m_ostream;
  };

  typedef std::vector<xml_state_t> xml_stack_t;
  xml_stack_t m_xml_stack;

  typedef PreferenceDictionary PreferenceEntries;
  PreferenceEntries& m_preferences;
  Version m_version;
public:
  XMLPreferenceDictionaryImporter(PreferenceDictionary& preferences, const char* version)
    : m_preferences(preferences), m_version(version_parse(version))
  {
  }

  void pushElement(const XMLElement& element)
  {
    if(m_xml_stack.empty())
    {
      if(string_equal(element.name(), "qpref"))
      {
        Version dataVersion(version_parse(element.attribute("version")));
        if(!version_compatible(m_version, dataVersion))
        {
          globalOutputStream() << "qpref import: data version " << dataVersion << " is not compatible with code version " << m_version << "\n";
          m_xml_stack.push_back(xml_state_t::tag_qpref_ignore);
        }
        else
        {
          globalOutputStream() << "qpref import: data version " << dataVersion << " is compatible with code version " << m_version << "\n";
          m_xml_stack.push_back(xml_state_t::tag_qpref);
        }
      }
      else
      {
        // not valid
      }
    }
    else
    {
      switch(m_xml_stack.back().m_tag)
      {
      case xml_state_t::tag_qpref:
        if(string_equal(element.name(), "epair"))
        {
          m_xml_stack.push_back(xml_state_t::tag_epair);
          m_xml_stack.back().m_name = element.attribute("name");
        }
        else
        {
          // not valid
        }
        break;
      case xml_state_t::tag_qpref_ignore:
        if(string_equal(element.name(), "epair"))
        {
          m_xml_stack.push_back(xml_state_t::tag_epair_ignore);
        }
        else
        {
          // not valid
        }
        break;
      case xml_state_t::tag_epair:
      case xml_state_t::tag_epair_ignore:
        // not valid
        break;
      }
    }

  }
  void popElement(const char* name)
  {
    if(m_xml_stack.back().m_tag == xml_state_t::tag_epair)
    {
      m_preferences.importPref(m_xml_stack.back().m_name.c_str(), m_xml_stack.back().m_ostream.c_str());
    }
    m_xml_stack.pop_back();
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return m_xml_stack.back().m_ostream.write(buffer, length);
  }
};

#endif
