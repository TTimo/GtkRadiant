/*
Copyright (C) 2006, Thomas Nitschke.
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
#include "plugin.h"

#include "iplugin.h"
#include "qerplugin.h"

#include <gtk/gtk.h>

#include "debugging/debugging.h"
#include "string/string.h"
#include "modulesystem/singletonmodule.h"
#include "stream/textfilestream.h"
#include "stream/stringstream.h"
#include "gtkutil/messagebox.h"
#include "gtkutil/filechooser.h"

#include "ibrush.h"
#include "iscenegraph.h"
#include "iselection.h"
#include "ifilesystem.h"
#include "ifiletypes.h"

#include "../../radiant/brush.h"

namespace BrushExport
{ 
  GtkWindow* g_mainwnd;
  
  class CExportFormatWavefront : public BrushVisitor
  {
    TextFileOutputStream& m_file;

    StringOutputStream vertexbuffer;
    StringOutputStream texcoordbuffer;
    StringOutputStream facebuffer;
    
    size_t vertices;
    size_t exported;
    
  public:
    
    CExportFormatWavefront(TextFileOutputStream& file)
        : m_file(file)
    {
      exported = 0;
      vertices = 0;
    }
    
    virtual ~CExportFormatWavefront(void) {}
    
    void visit(scene::Instance& instance)
    {
      BrushInstance* bptr = InstanceTypeCast<BrushInstance>::cast(instance);
      if(bptr)
      {
        Brush& brush(bptr->getBrush());
        
        m_file << "\ng " << brush.name() << exported << "\n";
        
        brush.forEachFace(*this);
    
        m_file << vertexbuffer.c_str() << "\n";
        m_file << texcoordbuffer.c_str();
        m_file << facebuffer.c_str() << "\n";
        
        vertexbuffer.clear();
        texcoordbuffer.clear();
        facebuffer.clear();
        ++exported;
      }
    }
   
    void visit(Face& face) const
    {
      // cast the stupid const away
      const_cast<CExportFormatWavefront*>(this)->visit(face);
    }
    
    void visit(Face& face)
    {
      size_t v_start = vertices;
      const Winding& w(face.getWinding());
      for(size_t i = 0; i < w.numpoints; ++i)
      {
        vertexbuffer << "v " << w[i].vertex.x() << " " << w[i].vertex.y() << " " << w[i].vertex.z() << "\n";
        texcoordbuffer << "vt " << w[i].texcoord.x() << " " << w[i].texcoord.y() << "\n";
        ++vertices;
      }
      
      facebuffer << "\nf";
      for(size_t i = v_start; i < vertices; ++i)
        facebuffer << " " << i+1 << "/" << i+1;
    }
  };
  
  /**
    Exporterclass which will pass every visit-call
    to a special formatexporter.
  */
  template<class TExporterFormat>
  class CExporter : public SelectionSystem::Visitor
  {
  public:
    CExporter(TextFileOutputStream& file)
      : m_exporter(file)
    {}
    
    virtual ~CExporter(void) {}
    
    void visit(scene::Instance& instance) const
    {
      m_exporter.visit(instance);
    }
    
  private:
    mutable TExporterFormat m_exporter;
  };
  
  template<class T>
  void export_selected(TextFileOutputStream& file)
  {
    CExporter<T> exporter(file);
    GlobalSelectionSystem().foreachSelected(exporter);
  }

  const char* init(void* hApp, void* pMainWidget)
  {
    g_mainwnd = (GtkWindow*)pMainWidget;
    ASSERT_NOTNULL(g_mainwnd);
    return "";
  }
  const char* getName()
  {
    return "Brush export Plugin";
  }
  const char* getCommandList()
  {
    return "Export selected as Wavefront Object;About";
  }
  const char* getCommandTitleList()
  {
    return "";
  }
  
  void dispatch(const char* command, float* vMin, float* vMax, bool bSingleBrush)
  {
    if(string_equal(command, "About"))
    {
      GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_mainwnd), "Brushexport plugin v 1.0 by namespace (www.codecreator.net)\n"
                                        "Enjoy!\n\nSend feedback to spam@codecreator.net", "About me...",
                                        eMB_OK,
                                        eMB_ICONDEFAULT);
    }
    else if(string_equal(command, "Export selected as Wavefront Object"))
    {
      if(const char* path = GlobalRadiant().m_pfnFileDialog(GTK_WIDGET(g_mainwnd), false, "Save as Obj", 0, 0))
      {
        TextFileOutputStream file(path); 
        if(file.failed())
        {
          GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_mainwnd), "Unable to write to file", "Error",
                                        eMB_OK,
                                        eMB_ICONERROR);
        }
        else
        {
          export_selected<CExportFormatWavefront>(file);
        }
      }
    }
  }
  
} // namespace

class BrushExportDependencies :
  public GlobalRadiantModuleRef,
  public GlobalFiletypesModuleRef,
  public GlobalBrushModuleRef,
  public GlobalFileSystemModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalSelectionModuleRef
{
public:
  BrushExportDependencies(void)
    : GlobalBrushModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("brushtypes"))
  {}
};

class BrushExportModule : public TypeSystemRef
{
  _QERPluginTable m_plugin;
public:
  typedef _QERPluginTable Type;
  STRING_CONSTANT(Name, "brushexport");

  BrushExportModule()
  {
    m_plugin.m_pfnQERPlug_Init = &BrushExport::init;
    m_plugin.m_pfnQERPlug_GetName = &BrushExport::getName;
    m_plugin.m_pfnQERPlug_GetCommandList = &BrushExport::getCommandList;
    m_plugin.m_pfnQERPlug_GetCommandTitleList = &BrushExport::getCommandTitleList;
    m_plugin.m_pfnQERPlug_Dispatch = &BrushExport::dispatch;
  }
  _QERPluginTable* getTable()
  {
    return &m_plugin;
  }
};

typedef SingletonModule<BrushExportModule, BrushExportDependencies> SingletonBrushExportModule;
SingletonBrushExportModule g_BrushExportModule;

extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);
  g_BrushExportModule.selfRegister();
}
