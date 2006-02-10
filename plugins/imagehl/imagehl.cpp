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

#include "imagehl.h"

#include "debugging/debugging.h"
#include "ifilesystem.h"
#include "iimage.h"

#include "hlw.h"
#include "mip.h"
#include "sprite.h"

#include "modulesystem/singletonmodule.h"


class ImageDependencies : public GlobalFileSystemModuleRef
{
};

class ImageHLWAPI
{
  _QERPlugImageTable m_imagehlw;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "hlw");

  ImageHLWAPI()
  {
    m_imagehlw.loadImage = LoadHLW;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagehlw;
  }
};

typedef SingletonModule<ImageHLWAPI, ImageDependencies> ImageHLWModule;

ImageHLWModule g_ImageHLWModule;


class ImageMipAPI
{
  _QERPlugImageTable m_imagemip;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "mip");

  ImageMipAPI()
  {
    m_imagemip.loadImage = LoadMIP;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagemip;
  }
};

typedef SingletonModule<ImageMipAPI, ImageDependencies> ImageMipModule;

ImageMipModule g_ImageMipModule;


class ImageSpriteAPI
{
  _QERPlugImageTable m_imagesprite;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "spr");

  ImageSpriteAPI()
  {
    m_imagesprite.loadImage = LoadIDSP;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagesprite;
  }
};

typedef SingletonModule<ImageSpriteAPI, ImageDependencies> ImageSpriteModule;

ImageSpriteModule g_ImageSpriteModule;



extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_ImageHLWModule.selfRegister();
  g_ImageMipModule.selfRegister();
  g_ImageSpriteModule.selfRegister();
}
