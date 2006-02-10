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

#include "plugin.h"

#include "iarchive.h"

#include "debugging/debugging.h"
#include "modulesystem/singletonmodule.h"

#include "archive.h"

class ArchiveWadAPI
{
  _QERArchiveTable m_archivewad;
public:
  typedef _QERArchiveTable Type;
  STRING_CONSTANT(Name, "wad");

  ArchiveWadAPI()
  {
    m_archivewad.m_pfnOpenArchive = &OpenArchive;
  }
  _QERArchiveTable* getTable()
  {
    return &m_archivewad;
  }
};

typedef SingletonModule<ArchiveWadAPI> ArchiveWadModule;

ArchiveWadModule g_ArchiveWadModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_ArchiveWadModule.selfRegister();
}
