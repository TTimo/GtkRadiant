/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

// PlugInManager.cpp: implementation of the CPlugInManager class.
//
//////////////////////////////////////////////////////////////////////

#include "pluginmanager.h"

#include "modulesystem.h"
#include "qerplugin.h"
#include "iplugin.h"

#include "math/vector.h"
#include "string/string.h"

#include "error.h"
#include "select.h"
#include "plugin.h"

#include "modulesystem.h"

#include <list>

/* plugin manager --------------------------------------- */
class CPluginSlot : public IPlugIn
{
  CopiedString m_menu_name;
  const _QERPluginTable *mpTable;
  std::list<CopiedString> m_CommandStrings;
  std::list<CopiedString> m_CommandTitleStrings;
  std::list<std::size_t> m_CommandIDs;
  
public:
  /*!
  build directly from a SYN_PROVIDE interface
  */
  CPluginSlot(GtkWidget* main_window, const char* name, const _QERPluginTable& table);
  /*!
  dispatching a command by name to the plugin
  */
  void Dispatch(const char *p);

  // IPlugIn ------------------------------------------------------------
  const char* getMenuName();
  std::size_t getCommandCount();
  const char* getCommand(std::size_t n);  
  const char* getCommandTitle(std::size_t n);
  void addMenuID(std::size_t n);
  bool ownsCommandID(std::size_t n);
  
};

CPluginSlot::CPluginSlot(GtkWidget* main_window, const char* name, const _QERPluginTable& table)
{
  mpTable = &table;
  m_menu_name = name;

  const char* commands = mpTable->m_pfnQERPlug_GetCommandList();
  const char* titles = mpTable->m_pfnQERPlug_GetCommandTitleList();

  StringTokeniser commandTokeniser(commands, ",;");
  StringTokeniser titleTokeniser(titles, ",;");
  
  const char* cmdToken = commandTokeniser.getToken();
  const char *titleToken = titleTokeniser.getToken();
  while (!string_empty(cmdToken))
  {
    if(string_empty(titleToken))
    {
      m_CommandStrings.push_back(cmdToken);
      m_CommandTitleStrings.push_back(cmdToken);
      cmdToken = commandTokeniser.getToken();
      titleToken = "";
    }
    else
    {
      m_CommandStrings.push_back(cmdToken);
      m_CommandTitleStrings.push_back(titleToken);
      cmdToken = commandTokeniser.getToken();
      titleToken = titleTokeniser.getToken();
    }
  }
  mpTable->m_pfnQERPlug_Init(0, (void*)main_window);
}

const char* CPluginSlot::getMenuName()
{
  return m_menu_name.c_str();
}

std::size_t CPluginSlot::getCommandCount()
{
  return m_CommandStrings.size();  
}
  
const char* CPluginSlot::getCommand(std::size_t n)
{
  std::list<CopiedString>::iterator i = m_CommandStrings.begin();
  while(n-- != 0)
    ++i;
  return (*i).c_str();  
}

const char* CPluginSlot::getCommandTitle(std::size_t n)
{
  std::list<CopiedString>::iterator i = m_CommandTitleStrings.begin();
  while(n-- != 0)
    ++i;
  return (*i).c_str();  
}

void CPluginSlot::addMenuID(std::size_t n)
{
  m_CommandIDs.push_back(n);
}

bool CPluginSlot::ownsCommandID(std::size_t n)
{
  for(std::list<std::size_t>::iterator i = m_CommandIDs.begin(); i != m_CommandIDs.end(); ++i)
  {
    if (*i == n)
      return true;
  }
  return false;
}

void CPluginSlot::Dispatch(const char *p)
{
  Vector3 vMin, vMax;
  Select_GetBounds (vMin, vMax);
  mpTable->m_pfnQERPlug_Dispatch(p, reinterpret_cast<float*>(&vMin), reinterpret_cast<float*>(&vMax), true);//QE_SingleBrush(true));
}


class CPluginSlots
{
  std::list<CPluginSlot *> mSlots;
public:
  virtual ~CPluginSlots();

  void AddPluginSlot(GtkWidget* main_window, const char* name, const _QERPluginTable& table)
  {
    mSlots.push_back(new CPluginSlot(main_window, name, table));
  }
  
  void PopulateMenu(PluginsVisitor& menu);
  bool Dispatch(std::size_t n, const char* p);
};

CPluginSlots::~CPluginSlots()
{
  std::list<CPluginSlot *>::iterator iSlot;
  for(iSlot=mSlots.begin(); iSlot!=mSlots.end(); ++iSlot)
  {
    delete *iSlot;
    *iSlot = 0;
  }
}

void CPluginSlots::PopulateMenu(PluginsVisitor& menu)
{
  std::list<CPluginSlot *>::iterator iPlug;
  for(iPlug=mSlots.begin(); iPlug != mSlots.end(); ++iPlug)
  {
    menu.visit(*(*iPlug));
  }
}

bool CPluginSlots::Dispatch(std::size_t n, const char* p)
{
  std::list<CPluginSlot *>::iterator iPlug;
  for(iPlug=mSlots.begin(); iPlug!=mSlots.end(); ++iPlug)
  {
    CPluginSlot *pPlug = *iPlug;
    if (pPlug->ownsCommandID(n))
    {
      pPlug->Dispatch(p);
      return true;
    }
  }
  return false;
}

CPluginSlots g_plugin_slots;


void FillPluginSlots(CPluginSlots& slots, GtkWidget* main_window)
{
  class AddPluginVisitor : public PluginModules::Visitor
  {
    CPluginSlots& m_slots;
    GtkWidget* m_main_window;
  public:
    AddPluginVisitor(CPluginSlots& slots, GtkWidget* main_window)
      : m_slots(slots), m_main_window(main_window)
    {
    }
    void visit(const char* name, const _QERPluginTable& table)
    {
      m_slots.AddPluginSlot(m_main_window, name, table);
    }
  } visitor(slots, main_window);

  Radiant_getPluginModules().foreachModule(visitor);
}


#include "pluginmanager.h"

CPlugInManager g_PlugInMgr;

CPlugInManager& GetPlugInMgr()
{
  return g_PlugInMgr;
}

void CPlugInManager::Dispatch(std::size_t n, const char * p)
{
  g_plugin_slots.Dispatch(n, p);
}

void CPlugInManager::Init(GtkWidget* main_window)
{
  FillPluginSlots(g_plugin_slots, main_window);
}

void CPlugInManager::constructMenu(PluginsVisitor& menu)
{
  g_plugin_slots.PopulateMenu(menu);
}

void CPlugInManager::Shutdown()
{
}
