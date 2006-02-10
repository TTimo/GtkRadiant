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

#include "imageq2.h"

#include "debugging/debugging.h"
#include "ifilesystem.h"
#include "iimage.h"

#include "wal.h"
#include "wal32.h"

#include "modulesystem/singletonmodule.h"


class ImageDependencies : public GlobalFileSystemModuleRef
{
};

class ImageWalAPI
{
  _QERPlugImageTable m_imagewal;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "wal");

  ImageWalAPI()
  {
    m_imagewal.loadImage = LoadWal;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagewal;
  }
};

typedef SingletonModule<ImageWalAPI, ImageDependencies> ImageWalModule;

ImageWalModule g_ImageWalModule;

////////////////////////////////////////////////////

class ImageM8API
{
  _QERPlugImageTable m_imagem8;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "m8");

  ImageM8API()
  {
    m_imagem8.loadImage = LoadM8;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagem8;
  }
};

typedef SingletonModule<ImageM8API, ImageDependencies> ImageM8Module;

ImageM8Module g_ImageM8Module;

////////////////////////////////////////////////////////

class ImageM32API
{
  _QERPlugImageTable m_imagem32;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "m32");

  ImageM32API()
  {
    m_imagem32.loadImage = LoadM32;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagem32;
  }
};

typedef SingletonModule<ImageM32API, ImageDependencies> ImageM32Module;

ImageM32Module g_ImageM32Module;

////////////////////////////////////////////////////////

extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_ImageWalModule.selfRegister();
  g_ImageM8Module.selfRegister();
  g_ImageM32Module.selfRegister();
}
