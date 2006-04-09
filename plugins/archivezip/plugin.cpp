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


class ArchiveZipAPI
{
  _QERArchiveTable m_archivezip;
public:
  typedef _QERArchiveTable Type;
  STRING_CONSTANT(Name, "pk3");

  ArchiveZipAPI()
  {
    m_archivezip.m_pfnOpenArchive = &OpenArchive;
  }
  _QERArchiveTable* getTable()
  {
    return &m_archivezip;
  }
};

typedef SingletonModule<ArchiveZipAPI> ArchiveZipModule;

ArchiveZipModule g_ArchiveZipModule;


class ArchivePK4API
{
  _QERArchiveTable m_archivepk4;
public:
  typedef _QERArchiveTable Type;
  STRING_CONSTANT(Name, "pk4");

  ArchivePK4API()
  {
    m_archivepk4.m_pfnOpenArchive = &OpenArchive;
  }
  _QERArchiveTable* getTable()
  {
    return &m_archivepk4;
  }
};

typedef SingletonModule<ArchivePK4API> ArchivePK4Module;

ArchivePK4Module g_ArchivePK4Module;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);

  g_ArchiveZipModule.selfRegister();
  g_ArchivePK4Module.selfRegister();
}
