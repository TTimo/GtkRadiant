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

#include "iscenegraph.h"
#include "irender.h"
#include "iselection.h"
#include "iimage.h"
#include "imodel.h"
#include "igl.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "ifiletypes.h"
#include "iscriplib.h"

#include "modulesystem/singletonmodule.h"
#include "typesystem.h"

#include "md3.h"
#include "mdl.h"
#include "md2.h"
#include "mdc.h"
#include "mdlimage.h"
#include "md5.h"


class MD3ModelLoader : public ModelLoader
{
public:
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadMD3Model(file);
  }
};

class ModelDependencies :
  public GlobalFileSystemModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalUndoModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalFiletypesModuleRef
{
};

class ModelMD3API : public TypeSystemRef
{
  MD3ModelLoader m_modelmd3;
public:
  typedef ModelLoader Type;
  STRING_CONSTANT(Name, "md3");

  ModelMD3API()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("md3 models", "*.md3"));
  }
  ModelLoader* getTable()
  {
    return &m_modelmd3;
  }
};

typedef SingletonModule<ModelMD3API, ModelDependencies> ModelMD3Module;

ModelMD3Module g_ModelMD3Module;



class MD2ModelLoader : public ModelLoader
{
public:
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadMD2Model(file);
  }
};

class ModelMD2API : public TypeSystemRef
{
  MD2ModelLoader m_modelmd2;
public:
  typedef ModelLoader Type;
  STRING_CONSTANT(Name, "md2");

  ModelMD2API()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("md2 models", "*.md2"));
  }
  ModelLoader* getTable()
  {
    return &m_modelmd2;
  }
};

typedef SingletonModule<ModelMD2API, ModelDependencies> ModelMD2Module;

ModelMD2Module g_ModelMD2Module;

class MDLModelLoader : public ModelLoader
{
public:
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadMDLModel(file);
  }
};

class ModelMDLAPI : public TypeSystemRef
{
  MDLModelLoader m_modelmdl;
public:
  typedef ModelLoader Type;
  STRING_CONSTANT(Name, "mdl");

  ModelMDLAPI()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("mdl models", "*.mdl"));
  }
  ModelLoader* getTable()
  {
    return &m_modelmdl;
  }
};

typedef SingletonModule<ModelMDLAPI, ModelDependencies> ModelMDLModule;

ModelMDLModule g_ModelMDLModule;

class MDCModelLoader : public ModelLoader
{
public:
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadMDCModel(file);
  }
};

class ModelMDCAPI : public TypeSystemRef
{
  MDCModelLoader m_modelmdc;
public:
  typedef ModelLoader Type;
  STRING_CONSTANT(Name, "mdc");

  ModelMDCAPI()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("mdc models", "*.mdc"));
  }
  ModelLoader* getTable()
  {
    return &m_modelmdc;
  }
};

typedef SingletonModule<ModelMDCAPI, ModelDependencies> ModelMDCModule;

ModelMDCModule g_ModelMDCModule;


class ImageMDLAPI
{
  _QERPlugImageTable m_imagemdl;
public:
  typedef _QERPlugImageTable Type;
  STRING_CONSTANT(Name, "mdl");

  ImageMDLAPI()
  {
    m_imagemdl.loadImage = &LoadMDLImage;
  }
  _QERPlugImageTable* getTable()
  {
    return &m_imagemdl;
  }
};

typedef SingletonModule<ImageMDLAPI, GlobalFileSystemModuleRef> ImageMDLModule;

ImageMDLModule g_ImageMDLModule;


class MD5ModelLoader : public ModelLoader
{
public:
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadMD5Model(file);
  }
};

class ModelMD5Dependencies : public ModelDependencies, public GlobalScripLibModuleRef
{
};

class ModelMD5API : public TypeSystemRef
{
  MD5ModelLoader m_modelmd5;
public:
  typedef ModelLoader Type;
  STRING_CONSTANT(Name, "md5mesh");

  ModelMD5API()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("md5 meshes", "*.md5mesh"));
  }
  ModelLoader* getTable()
  {
    return &m_modelmd5;
  }
};

typedef SingletonModule<ModelMD5API, ModelMD5Dependencies> ModelMD5Module;

ModelMD5Module g_ModelMD5Module;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_ModelMD3Module.selfRegister();
  g_ModelMD2Module.selfRegister();
  g_ModelMDLModule.selfRegister();
  g_ModelMDCModule.selfRegister();
  g_ImageMDLModule.selfRegister();
  g_ModelMD5Module.selfRegister();
}
