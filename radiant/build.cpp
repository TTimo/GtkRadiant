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

#include "build.h"
#include "debugging/debugging.h"

#include <map>
#include <list>
#include "stream/stringstream.h"
#include "versionlib.h"

#include "mainframe.h"

typedef std::map<CopiedString, CopiedString> Variables;
Variables g_build_variables;

void build_clear_variables()
{
  g_build_variables.clear();
}

void build_set_variable(const char* name, const char* value)
{
  g_build_variables[name] = value;
}

const char* build_get_variable(const char* name)
{
  Variables::iterator i = g_build_variables.find(name);
  if(i != g_build_variables.end())
  {
    return (*i).second.c_str();
  }
  globalErrorStream() << "undefined build variable: " << makeQuoted(name) << "\n";
  return "";
}

#include "xml/ixml.h"
#include "xml/xmlelement.h"

class Evaluatable
{
public:
  virtual void evaluate(StringBuffer& output) = 0;
  virtual void exportXML(XMLImporter& importer) = 0;
};

class VariableString : public Evaluatable
{
  CopiedString m_string;
public:
  VariableString() : m_string()
  {
  }
  VariableString(const char* string) : m_string(string)
  {
  }
  const char* c_str() const
  {
    return m_string.c_str();
  }
  void setString(const char* string)
  {
    m_string = string;
  }
  void evaluate(StringBuffer& output)
  {
    StringBuffer variable;
    bool in_variable = false;
    for(const char* i = m_string.c_str(); *i != '\0'; ++i)
    {
      if(!in_variable)
      {
        switch(*i)
        {
        case '[':
          in_variable = true;
          break;
        default:
          output.push_back(*i);
          break;
        }
      }
      else
      {
        switch(*i)
        {
        case ']':
          in_variable = false;
          output.push_string(build_get_variable(variable.c_str()));
          variable.clear();
          break;
        default:
          variable.push_back(*i);
          break;
        }
      }
    }
  }
  void exportXML(XMLImporter& importer)
  {
    importer << c_str();
  }
};

class Conditional : public Evaluatable
{
  VariableString* m_test;
public:
  Evaluatable* m_result;
  Conditional(VariableString* test) : m_test(test)
  {
  }
  ~Conditional()
  {
    delete m_test;
    delete m_result;
  }
  void evaluate(StringBuffer& output)
  {
    StringBuffer buffer;
    m_test->evaluate(buffer);
    if(!string_empty(buffer.c_str()))
    {
      m_result->evaluate(output);
    }
  }
  void exportXML(XMLImporter& importer)
  {
    StaticElement conditionElement("cond");
    conditionElement.insertAttribute("value", m_test->c_str());
    importer.pushElement(conditionElement);
    m_result->exportXML(importer);
    importer.popElement(conditionElement.name());
  }
};

typedef std::vector<Evaluatable*> Evaluatables;

class Tool : public Evaluatable
{
  Evaluatables m_evaluatables;
public:
  ~Tool()
  {
    for(Evaluatables::iterator i = m_evaluatables.begin(); i != m_evaluatables.end(); ++i)
    {
      delete (*i);
    }
  }
  void push_back(Evaluatable* evaluatable)
  {
    m_evaluatables.push_back(evaluatable);
  }
  void evaluate(StringBuffer& output)
  {
    for(Evaluatables::iterator i = m_evaluatables.begin(); i != m_evaluatables.end(); ++i)
    {
      (*i)->evaluate(output);
    }
  }
  void exportXML(XMLImporter& importer)
  {
    for(Evaluatables::iterator i = m_evaluatables.begin(); i != m_evaluatables.end(); ++i)
    {
      (*i)->exportXML(importer);
    }
  }
};

#include "xml/ixml.h"

class XMLElementParser : public TextOutputStream
{
public:
  virtual XMLElementParser& pushElement(const XMLElement& element) = 0;
  virtual void popElement(const char* name) = 0;
};

class VariableStringXMLConstructor : public XMLElementParser
{
  StringBuffer m_buffer;
  VariableString& m_variableString;
public:
  VariableStringXMLConstructor(VariableString& variableString) : m_variableString(variableString)
  {
  }
  ~VariableStringXMLConstructor()
  {
    m_variableString.setString(m_buffer.c_str());
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    m_buffer.push_range(buffer, buffer + length);
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    ERROR_MESSAGE("parse error: invalid element \"" << element.name() << "\"");
    return *this;
  }
  void popElement(const char* name)
  {
  }
};

class ConditionalXMLConstructor : public XMLElementParser
{
  StringBuffer m_buffer;
  Conditional& m_conditional;
public:
  ConditionalXMLConstructor(Conditional& conditional) : m_conditional(conditional)
  {
  }
  ~ConditionalXMLConstructor()
  {
    m_conditional.m_result = new VariableString(m_buffer.c_str());
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    m_buffer.push_range(buffer, buffer + length);
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    ERROR_MESSAGE("parse error: invalid element \"" << element.name() << "\"");
    return *this;
  }
  void popElement(const char* name)
  {
  }
};

class ToolXMLConstructor : public XMLElementParser
{
  StringBuffer m_buffer;
  Tool& m_tool;
  ConditionalXMLConstructor* m_conditional;
public:
  ToolXMLConstructor(Tool& tool) : m_tool(tool)
  {
  }
  ~ToolXMLConstructor()
  {
    flush();
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    m_buffer.push_range(buffer, buffer + length);
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), "cond"))
    {
      flush();
      Conditional* conditional = new Conditional(new VariableString(element.attribute("value")));
      m_tool.push_back(conditional);
      m_conditional = new ConditionalXMLConstructor(*conditional);
      return *m_conditional;
    }
    else
    {
      ERROR_MESSAGE("parse error: invalid element \"" << element.name() << "\"");
      return *this;
    }
  }
  void popElement(const char* name)
  {
    if(string_equal(name, "cond"))
    {
      delete m_conditional;
    }
  }

  void flush()
  {
    if(!m_buffer.empty())
    {
      m_tool.push_back(new VariableString(m_buffer.c_str()));
      m_buffer.clear();
    }
  }
};

typedef VariableString BuildCommand;
typedef std::list<BuildCommand> Build;

class BuildXMLConstructor : public XMLElementParser
{
  VariableStringXMLConstructor* m_variableString;
  Build& m_build;
public:
  BuildXMLConstructor(Build& build) : m_build(build)
  {
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), "command"))
    {
      m_build.push_back(BuildCommand());
      m_variableString = new VariableStringXMLConstructor(m_build.back());
      return *m_variableString;
    }
    else
    {
      ERROR_MESSAGE("parse error: invalid element");
      return *this;
    }
  }
  void popElement(const char* name)
  {
    delete m_variableString;
  }
};

typedef std::pair<CopiedString, Build> BuildPair;

class BuildPairEqual
{
  const char* m_name;
public:
  BuildPairEqual(const char* name) : m_name(name)
  {
  }
  bool operator()(const BuildPair& self) const
  {
    return string_equal(self.first.c_str(), m_name);
  }
};

typedef std::list<BuildPair> Project;

Project::iterator Project_find(Project& project, const char* name)
{
  return std::find_if(project.begin(), project.end(), BuildPairEqual(name));
}

Project::iterator Project_find(Project& project, std::size_t index)
{
  Project::iterator i = project.begin();
  while(index-- != 0 && i != project.end())
  {
    ++i;
  }
  return i;
}

Build& project_find(Project& project, const char* build)
{
  Project::iterator i = Project_find(project, build);
  ASSERT_MESSAGE(i != project.end(), "error finding build command");
  return (*i).second;
}

Build::iterator Build_find(Build& build, std::size_t index)
{
  Build::iterator i = build.begin();
  while(index-- != 0 && i != build.end())
  {
    ++i;
  }
  return i;
}

typedef std::map<CopiedString, Tool> Tools;

class ProjectXMLConstructor : public XMLElementParser
{
  ToolXMLConstructor* m_tool;
  BuildXMLConstructor* m_build;
  Project& m_project;
  Tools& m_tools;
public:
  ProjectXMLConstructor(Project& project, Tools& tools) : m_project(project), m_tools(tools)
  {
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), "var"))
    {
      Tools::iterator i = m_tools.insert(Tools::value_type(element.attribute("name"), Tool())).first;
      m_tool = new ToolXMLConstructor((*i).second);
      return *m_tool;
    }
    else if(string_equal(element.name(), "build"))
    {
      m_project.push_back(Project::value_type(element.attribute("name"), Build()));
      m_build = new BuildXMLConstructor(m_project.back().second);
      return *m_build;
    }
    else
    {
      ERROR_MESSAGE("parse error: invalid element");
      return *this;
    }
  }
  void popElement(const char* name)
  {
    if(string_equal(name, "var"))
    {
      delete m_tool;
    }
    else if(string_equal(name, "build"))
    {
      delete m_build;
    }
  }
};

class SkipAllParser : public XMLElementParser
{
public:
  std::size_t write(const char* buffer, std::size_t length)
  {
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    return *this;
  }
  void popElement(const char* name)
  {
  }
};

class RootXMLConstructor : public XMLElementParser
{
  CopiedString m_elementName;
  XMLElementParser& m_parser;
  SkipAllParser m_skip;
  Version m_version;
  bool m_compatible;
public:
  RootXMLConstructor(const char* elementName, XMLElementParser& parser, const char* version) :
    m_elementName(elementName),
    m_parser(parser),
    m_version(version_parse(version)),
    m_compatible(false)
  {
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return length;
  }
  XMLElementParser& pushElement(const XMLElement& element)
  {
    if(string_equal(element.name(), m_elementName.c_str()))
    {
      Version dataVersion(version_parse(element.attribute("version")));
      if(version_compatible(m_version, dataVersion))
      {
        m_compatible = true;
        return m_parser;
      }
      else
      {
        return m_skip;
      }
    }
    else
    {
      //ERROR_MESSAGE("parse error: invalid element \"" << element.name() << "\"");
      return *this;
    }
  }
  void popElement(const char* name)
  {
  }

  bool versionCompatible() const
  {
    return m_compatible;
  }
};

namespace
{
  Project g_build_project;
  Tools g_build_tools;
  bool g_build_changed = false;
}

void build_error_undefined_tool(const char* build, const char* tool)
{
  globalErrorStream() << "build " << makeQuoted(build) << " refers to undefined tool " << makeQuoted(tool) << '\n';
}

void project_verify(Project& project, Tools& tools)
{
#if 0
  for(Project::iterator i = project.begin(); i != project.end(); ++i)
  {
    Build& build = (*i).second;
    for(Build::iterator j = build.begin(); j != build.end(); ++j)
    {
      Tools::iterator k = tools.find((*j).first);
      if(k == g_build_tools.end())
      {
        build_error_undefined_tool((*i).first.c_str(), (*j).first.c_str());
      }
    }
  }
#endif
}

void build_run(const char* name, CommandListener& listener)
{
  for(Tools::iterator i = g_build_tools.begin(); i != g_build_tools.end(); ++i)
  {
    StringBuffer output;
    (*i).second.evaluate(output);
    build_set_variable((*i).first.c_str(), output.c_str());
  }

  {
    Project::iterator i = Project_find(g_build_project, name);
    if(i != g_build_project.end())
    {
      Build& build = (*i).second;
      for(Build::iterator j = build.begin(); j != build.end(); ++j)
      {
        StringBuffer output;
        (*j).evaluate(output);
        listener.execute(output.c_str());
      }
    }
    else
    {
      globalErrorStream() << "build " << makeQuoted(name) << " not defined";
    }
  }
}


typedef std::vector<XMLElementParser*> XMLElementStack;

class XMLParser : public XMLImporter
{
  XMLElementStack m_stack;
public:
  XMLParser(XMLElementParser& parser)
  {
    m_stack.push_back(&parser);
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    return m_stack.back()->write(buffer, length);
  }
  void pushElement(const XMLElement& element)
  {
    m_stack.push_back(&m_stack.back()->pushElement(element));
  }
  void popElement(const char* name)
  {
    m_stack.pop_back();
    m_stack.back()->popElement(name);
  }
};

#include "stream/textfilestream.h"
#include "xml/xmlparser.h"

const char* const BUILDMENU_VERSION = "2.0";

bool build_commands_parse(const char* filename)
{
  TextFileInputStream projectFile(filename);
  if(!projectFile.failed())
  {
    ProjectXMLConstructor projectConstructor(g_build_project, g_build_tools);
    RootXMLConstructor rootConstructor("project", projectConstructor, BUILDMENU_VERSION);
    XMLParser importer(rootConstructor);
    XMLStreamParser parser(projectFile);
    parser.exportXML(importer);

    if(rootConstructor.versionCompatible())
    {
      project_verify(g_build_project, g_build_tools);

      return true;
    }
    globalErrorStream() << "failed to parse build menu: " << makeQuoted(filename);
  }
  return false;
}

void build_commands_clear()
{
  g_build_project.clear();
  g_build_tools.clear();
}

class BuildXMLExporter
{
  Build& m_build;
public:
  BuildXMLExporter(Build& build) : m_build(build)
  {
  }
  void exportXML(XMLImporter& importer)
  {
    importer << "\n";
    for(Build::iterator i = m_build.begin(); i != m_build.end(); ++i)
    {
      StaticElement commandElement("command");
      importer.pushElement(commandElement);
      (*i).exportXML(importer);
      importer.popElement(commandElement.name());
      importer << "\n";
    }
  }
};

class ProjectXMLExporter
{
  Project& m_project;
  Tools& m_tools;
public:
  ProjectXMLExporter(Project& project, Tools& tools) : m_project(project), m_tools(tools)
  {
  }
  void exportXML(XMLImporter& importer)
  {
    StaticElement projectElement("project");
    projectElement.insertAttribute("version", BUILDMENU_VERSION);
    importer.pushElement(projectElement);
    importer << "\n";

    for(Tools::iterator i = m_tools.begin(); i != m_tools.end(); ++i)
    {
      StaticElement toolElement("var");
      toolElement.insertAttribute("name", (*i).first.c_str());
      importer.pushElement(toolElement);
      (*i).second.exportXML(importer);
      importer.popElement(toolElement.name());
      importer << "\n";
    }
    for(Project::iterator i = m_project.begin(); i != m_project.end(); ++i)
    {
      StaticElement buildElement("build");
      buildElement.insertAttribute("name", (*i).first.c_str());
      importer.pushElement(buildElement);
      BuildXMLExporter buildExporter((*i).second);
      buildExporter.exportXML(importer);
      importer.popElement(buildElement.name());
      importer << "\n";
    }
    importer.popElement(projectElement.name());
  }
};

#include "xml/xmlwriter.h"

void build_commands_write(const char* filename)
{
  TextFileOutputStream projectFile(filename);
  if(!projectFile.failed())
  {
    XMLStreamWriter writer(projectFile);
    ProjectXMLExporter projectExporter(g_build_project, g_build_tools);
    writer << "\n";
    projectExporter.exportXML(writer);
    writer << "\n";
  }
}


#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkscrolledwindow.h>

#include "gtkutil/dialog.h"
#include "gtkutil/closure.h"
#include "gtkutil/window.h"
#include "gtkdlgs.h"

void Build_refreshMenu(GtkMenu* menu);


void BSPCommandList_Construct(GtkListStore* store, Project& project)
{
  gtk_list_store_clear(store);

  for(Project::iterator i = project.begin(); i != project.end(); ++i)
  {
    const char* buildName = (*i).first.c_str();

    GtkTreeIter buildIter;
    gtk_list_store_append(store, &buildIter);
    gtk_list_store_set(store, &buildIter, 0, const_cast<char*>(buildName), -1);
  }

  GtkTreeIter lastIter;
  gtk_list_store_append(store, &lastIter);
}

class ProjectList
{
public:
  Project& m_project;
  GtkListStore* m_store;
  bool m_changed;
  ProjectList(Project& project) : m_project(project), m_changed(false)
  {
  }
};

gboolean project_cell_edited(GtkCellRendererText* cell, gchar* path_string, gchar* new_text, ProjectList* projectList)
{
  Project& project = projectList->m_project;

  GtkTreePath* path = gtk_tree_path_new_from_string(path_string);

  ASSERT_MESSAGE(gtk_tree_path_get_depth(path) == 1, "invalid path length");

  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(projectList->m_store), &iter, path);

  Project::iterator i = Project_find(project, gtk_tree_path_get_indices(path)[0]);
  if(i != project.end())
  {
    projectList->m_changed = true;
    if(string_empty(new_text))
    {
      project.erase(i);
      Build_refreshMenu(g_bsp_menu);

      gtk_list_store_remove(projectList->m_store, &iter);
    }
    else
    {
      (*i).first = new_text;
      Build_refreshMenu(g_bsp_menu);

      gtk_list_store_set(projectList->m_store, &iter, 0, new_text, -1);
    }
  }
  else if(!string_empty(new_text))
  {
    projectList->m_changed = true;
    project.push_back(Project::value_type(new_text, Build()));
    Build_refreshMenu(g_bsp_menu);

    gtk_list_store_set(projectList->m_store, &iter, 0, new_text, -1);
    GtkTreeIter lastIter;
    gtk_list_store_append(projectList->m_store, &lastIter);
  }

  gtk_tree_path_free(path);

  return FALSE;
}

gboolean project_key_press(GtkWidget* widget, GdkEventKey* event, ProjectList* projectList)
{
  Project& project = projectList->m_project;

  if(event->keyval == GDK_Delete)
  {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeIter iter;
    GtkTreeModel* model;
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
      Project::iterator x = Project_find(project, gtk_tree_path_get_indices(path)[0]);
      gtk_tree_path_free(path);

      if(x != project.end())
      {
        projectList->m_changed = true;
        project.erase(x);
        Build_refreshMenu(g_bsp_menu);

        gtk_list_store_remove(projectList->m_store, &iter);
      }
    }
  }
  return FALSE;
}


Build* g_current_build = 0;

gboolean project_selection_changed(GtkTreeSelection* selection, GtkListStore* store)
{
  Project& project = g_build_project;

  gtk_list_store_clear(store);

  GtkTreeIter iter;
  GtkTreeModel* model;
  if(gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
    Project::iterator x = Project_find(project, gtk_tree_path_get_indices(path)[0]);
    gtk_tree_path_free(path);

    if(x != project.end())
    {
      Build& build = (*x).second;
      g_current_build = &build;

      for(Build::iterator i = build.begin(); i != build.end(); ++i)
      {
        GtkTreeIter commandIter;
        gtk_list_store_append(store, &commandIter);
        gtk_list_store_set(store, &commandIter, 0, const_cast<char*>((*i).c_str()), -1);
      }
      GtkTreeIter lastIter;
      gtk_list_store_append(store, &lastIter);
    }
    else
    {
      g_current_build = 0;
    }
  }
  else
  {
    g_current_build = 0;
  }

  return FALSE;
}

gboolean commands_cell_edited(GtkCellRendererText* cell, gchar* path_string, gchar* new_text, GtkListStore* store)
{
  if(g_current_build == 0)
  {
    return FALSE;
  }
  Build& build = *g_current_build;

  GtkTreePath* path = gtk_tree_path_new_from_string(path_string);

  ASSERT_MESSAGE(gtk_tree_path_get_depth(path) == 1, "invalid path length");

  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);

  Build::iterator i = Build_find(build, gtk_tree_path_get_indices(path)[0]);
  if(i != build.end())
  {
    g_build_changed = true;
    (*i).setString(new_text);

    gtk_list_store_set(store, &iter, 0, new_text, -1);
  }
  else if(!string_empty(new_text))
  {
    g_build_changed = true;
    build.push_back(Build::value_type(VariableString(new_text)));

    gtk_list_store_set(store, &iter, 0, new_text, -1);

    GtkTreeIter lastIter;
    gtk_list_store_append(store, &lastIter);
  }

  gtk_tree_path_free(path);

  return FALSE;
}

gboolean commands_key_press(GtkWidget* widget, GdkEventKey* event, GtkListStore* store)
{
  if(g_current_build == 0)
  {
    return FALSE;
  }
  Build& build = *g_current_build;

  if(event->keyval == GDK_Delete)
  {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeIter iter;
    GtkTreeModel* model;
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
      Build::iterator i = Build_find(build, gtk_tree_path_get_indices(path)[0]);
      gtk_tree_path_free(path);

      if(i != build.end())
      {
        g_build_changed = true;
        build.erase(i);

        gtk_list_store_remove(store, &iter);
      }
    }
  }
  return FALSE;
}


GtkWindow* BuildMenuDialog_construct(ModalDialog& modal, ProjectList& projectList)
{
  GtkWindow* window = create_dialog_window(MainFrame_getWindow(), "Build Menu", G_CALLBACK(dialog_delete_callback), &modal, -1, 400);

  GtkWidget* buildView = 0;

  {
    GtkTable* table1 = create_dialog_table(2, 2, 4, 4, 4);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(table1));
    {
      GtkVBox* vbox = create_dialog_vbox(4);
      gtk_table_attach(table1, GTK_WIDGET(vbox), 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (GTK_FILL), 0, 0);
      {
        GtkButton* button = create_dialog_button("OK", G_CALLBACK(dialog_button_ok), &modal);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
      {
        GtkButton* button = create_dialog_button("Cancel", G_CALLBACK(dialog_button_cancel), &modal);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
    }
    {
      GtkFrame* frame = create_dialog_frame("Build menu");
      gtk_table_attach(table1, GTK_WIDGET(frame), 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
      {
        GtkScrolledWindow* scr = create_scrolled_window(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC, 4);
        gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(scr));

        {
          GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

          GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
          gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

          GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
          object_set_boolean_property(G_OBJECT(renderer), "editable", TRUE);
          g_signal_connect(renderer, "edited", G_CALLBACK(project_cell_edited), &projectList);
 
          GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, 0);
          gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

          GtkTreeSelection* selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
          gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

          gtk_widget_show(view);

          buildView = view;
          projectList.m_store = store;
          gtk_container_add(GTK_CONTAINER (scr), view);
  
          g_signal_connect(G_OBJECT(view), "key_press_event", G_CALLBACK(project_key_press), &projectList);

          g_object_unref(G_OBJECT(store));
        }
      }
    }
    {
      GtkFrame* frame = create_dialog_frame("Commandline");
      gtk_table_attach(table1, GTK_WIDGET(frame), 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
      {
        GtkScrolledWindow* scr = create_scrolled_window(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC, 4);
        gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(scr));

        {
          GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

          GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
          gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

          GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
          object_set_boolean_property(G_OBJECT(renderer), "editable", TRUE);
          g_signal_connect(renderer, "edited", G_CALLBACK(commands_cell_edited), store);
 
          GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, 0);
          gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

          GtkTreeSelection* selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
          gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

          gtk_widget_show(view);

          gtk_container_add(GTK_CONTAINER (scr), view);
  
          g_object_unref(G_OBJECT(store));

          g_signal_connect(G_OBJECT(view), "key_press_event", G_CALLBACK(commands_key_press), store);
         
          g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(buildView))), "changed", G_CALLBACK(project_selection_changed), store);
        }
      }
    }
  }

  BSPCommandList_Construct(projectList.m_store, g_build_project);

  return window;
}

namespace
{
  CopiedString g_buildMenu;
}

void LoadBuildMenu();

void DoBuildMenu()
{
  ModalDialog modal;

  ProjectList projectList(g_build_project);

  GtkWindow* window = BuildMenuDialog_construct(modal, projectList);

  if(modal_dialog_show(window, modal) == eIDCANCEL)
  {
    build_commands_clear();
    LoadBuildMenu();

    Build_refreshMenu(g_bsp_menu);
  }
  else if(projectList.m_changed)
  {
    g_build_changed = true;
  }

  gtk_widget_destroy(GTK_WIDGET(window));
}



#include "gtkutil/menu.h"
#include "mainframe.h"
#include "preferences.h"
#include "qe3.h"

typedef struct _GtkMenuItem GtkMenuItem;

class BuildMenuItem
{
  const char* m_name;
public:
  GtkMenuItem* m_item;
  BuildMenuItem(const char* name, GtkMenuItem* item)
    : m_name(name), m_item(item)
  {
  }
  void run()
  {
    RunBSP(m_name);
  }
  typedef MemberCaller<BuildMenuItem, &BuildMenuItem::run> RunCaller;
};

typedef std::list<BuildMenuItem> BuildMenuItems;
BuildMenuItems g_BuildMenuItems;


GtkMenu* g_bsp_menu;

void Build_constructMenu(GtkMenu* menu)
{
  for(Project::iterator i = g_build_project.begin(); i != g_build_project.end(); ++i)
  {
    g_BuildMenuItems.push_back(BuildMenuItem((*i).first.c_str(), 0));
    g_BuildMenuItems.back().m_item = create_menu_item_with_mnemonic(menu, (*i).first.c_str(), BuildMenuItem::RunCaller(g_BuildMenuItems.back()));
  }
}


void Build_refreshMenu(GtkMenu* menu)
{
  for(BuildMenuItems::iterator i = g_BuildMenuItems.begin(); i != g_BuildMenuItems.end(); ++i)
  {
    gtk_container_remove(GTK_CONTAINER(menu), GTK_WIDGET((*i).m_item));
  }

  g_BuildMenuItems.clear();

  Build_constructMenu(menu);
}


void LoadBuildMenu()
{
  if(string_empty(g_buildMenu.c_str()) || !build_commands_parse(g_buildMenu.c_str()))
  {
    {
      StringOutputStream buffer(256);
      buffer << GameToolsPath_get() << "default_build_menu.xml";

      bool success = build_commands_parse(buffer.c_str());
      ASSERT_MESSAGE(success, "failed to parse default build commands: " << buffer.c_str());
    }
    {
      StringOutputStream buffer(256);
      buffer << SettingsPath_get() << g_pGameDescription->mGameFile.c_str() << "/build_menu.xml";

      g_buildMenu = buffer.c_str();
    }
  }
}

void SaveBuildMenu()
{
  if(g_build_changed)
  {
    g_build_changed = false;
    build_commands_write(g_buildMenu.c_str());
  }
}

#include "preferencesystem.h"
#include "stringio.h"

void BuildMenu_Construct()
{
  GlobalPreferenceSystem().registerPreference("BuildMenu", CopiedStringImportStringCaller(g_buildMenu), CopiedStringExportStringCaller(g_buildMenu));
  LoadBuildMenu();
}
void BuildMenu_Destroy()
{
  SaveBuildMenu();
}
