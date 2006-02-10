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

#if !defined(INCLUDED_PLUGINMANAGER_H)
#define INCLUDED_PLUGINMANAGER_H

#include <cstddef>

typedef struct _GtkWidget GtkWidget;

/*!
\class IPlugin
pure virtual interface for a plugin
temporary solution for migration from old plugin tech to synapse plugins
*/
class IPlugIn
{
public:
  IPlugIn() { }
  virtual ~IPlugIn() { }  
  
  virtual const char* getMenuName() = 0;
  virtual std::size_t getCommandCount() = 0;
  virtual const char* getCommand(std::size_t) = 0;
  virtual const char* getCommandTitle(std::size_t) = 0;
  virtual void addMenuID(std::size_t) = 0;
  virtual bool ownsCommandID(std::size_t n) = 0;
};

class PluginsVisitor
{
public:
  virtual void visit(IPlugIn& plugin) = 0;
};

class CPlugInManager  
{
public:
  void Dispatch(std::size_t n, const char *p);
  void Init(GtkWidget* main_window);
  void constructMenu(PluginsVisitor& menu);
  void Shutdown();
};

CPlugInManager& GetPlugInMgr();

#endif
