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

#if 0

#include "preferencesystem.h"
#include "preferencedictionary.h"

#include "xml/xmlparser.h"
#include "xml/xmlwriter.h"


void LoadPrefs(PreferenceDictionary& preferences, const char* filename)
{
  TextFileInputStream file(filename);
  if(!file.failed())
  {
    XMLStreamParser parser(file);
    XMLPreferenceDictionaryImporter importer(preferences);
    parser.exportXML(importer);
  }
  else
  {
    // error
  }
}

void SavePrefs(PreferenceDictionary& preferences, const char* filename)
{
  TextFileOutputStream file(filename);
  if(!file.failed())
  {
    XMLStreamWriter writer(file);
    XMLPreferenceDictionaryExporter exporter(preferences, "1");
    exporter.exportXML(writer);
  }
  else
  {
    // error
  }
}


class StringPreference
{
public:
  class Observer
  {
  public:
    virtual void onChanged() = 0;
  };

private:
  CopiedString m_string;
  Observer& m_observer;
public:
  StringPreference(Observer& observer)
    : m_observer(observer)
  {
  }
  void importString(const char* value)
  {
    m_string = value;
    m_observer.onChanged();
  }
  typedef MemberCaller1<StringPreference, const char*, &StringPreference::importString> ImportStringCaller;
  void exportString(StringImportCallback& importer)
  {
    importer(m_string.c_str());
  }
  typedef MemberCaller1<StringPreference, StringImportCallback&, &StringPreference::exportString> ExportStringCaller;
};

inline void int_export(int i, StringImportCallback& importer)
{
  char buffer[16];
  sprintf(buffer, "%d", i);
  importer(buffer);
}

inline int int_import(const char* value)
{
  return atoi(value);
}

class IntPreference
{
public:
  class Observer
  {
  public:
    virtual void onChanged() = 0;
  };

private:
  int m_int;
  Observer& m_observer;
public:

  IntPreference(Observer& observer)
    : m_observer(observer)
  {
  }
  void importString(const char* value)
  {
    m_int = int_import(value);
    m_observer.onChanged();
  }
  typedef MemberCaller1<IntPreference, const char*, &IntPreference::importString> ImportStringCaller;
  void exportString(StringImportCallback& importer)
  {
    int_export(m_int, importer);
  }
  typedef MemberCaller1<IntPreference, StringImportCallback&, &IntPreference::exportString> ExportStringCaller;
};

class IntPreferenceImporter
{
  int& m_i;
public:

  IntPreferenceImporter(int& i)
    : m_i(i)
  {
  }
  void importString(const char* value)
  {
    m_i = int_import(value);
  }
};


class TestPrefs
{
public:
  TestPrefs()
  {
    PreferenceDictionary preferences;

    class StringObserver : public StringPreference::Observer
    {
    public:
      void onChanged()
      {
        int bleh = 0;
      }
    } string_observer;
    StringPreference string1(string_observer);
    string1.importString("twenty-three");

    class IntObserver : public IntPreference::Observer
    {
    public:
      void onChanged()
      {
        int bleh = 0;
      }

    } int_observer;
    IntPreference int1(int_observer);
    int1.importString("23");

    preferences.registerPreference("string1", StringPreference::ImportStringCaller(string1), StringPreference::ExportStringCaller(string1));
    preferences.registerPreference("int1", IntPreference::ImportStringCaller(int1), IntPreference::ExportStringCaller(int1));

    LoadPrefs(preferences, "test.pref");
    SavePrefs(preferences, "test.pref");

  }
};

#if 0
TestPrefs g_TestPrefs;
#endif

void readpref(PreferenceDictionary& preferences, int& int_variable)
{
  PreferenceDictionary::iterator i = preferences.find("int_variable");
  IntPreferenceImporter importer(int_variable);
  (*i).second.exporter().exportString(importer);
}

void writepref(PreferenceDictionary& preferences, int& int_variable)
{
  PreferenceDictionary::iterator i = preferences.find("int_variable");
  int_export(int_variable, (*i).second.importer());
}
#endif
