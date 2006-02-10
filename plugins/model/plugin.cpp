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

#include <stdio.h>
#include "picomodel.h"
typedef unsigned char byte;
#include <stdlib.h>
#include <algorithm>
#include <list>

#include "iscenegraph.h"
#include "irender.h"
#include "iselection.h"
#include "iimage.h"
#include "imodel.h"
#include "igl.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "ifiletypes.h"

#include "modulesystem/singletonmodule.h"
#include "stream/textstream.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "typesystem.h"

#include "model.h"

void PicoPrintFunc( int level, const char *str )
{
	if( str == 0 )
		return;
	switch( level )
	{
		case PICO_NORMAL:
			globalOutputStream() << str << "\n";
			break;
		
		case PICO_VERBOSE:
			//globalOutputStream() << "PICO_VERBOSE: " << str << "\n";
			break;
		
		case PICO_WARNING:
			globalErrorStream() << "PICO_WARNING: " << str << "\n";
			break;
		
		case PICO_ERROR:
			globalErrorStream() << "PICO_ERROR: " << str << "\n";
			break;
		
		case PICO_FATAL:
			globalErrorStream() << "PICO_FATAL: " << str << "\n";
			break;
	}
}

void PicoLoadFileFunc( char *name, byte **buffer, int *bufSize )
{
	*bufSize = vfsLoadFile( (const char*) name, (void**) buffer);
}

void PicoFreeFileFunc( void* file )
{
	vfsFreeFile(file);
}

void pico_initialise()
{
	PicoInit();
	PicoSetMallocFunc( malloc );
	PicoSetFreeFunc( free );
	PicoSetPrintFunc( PicoPrintFunc );
	PicoSetLoadFileFunc( PicoLoadFileFunc );
	PicoSetFreeFileFunc( PicoFreeFileFunc );
}


class PicoModelLoader : public ModelLoader
{
  const picoModule_t* m_module;
public:
  PicoModelLoader(const picoModule_t* module) : m_module(module)
  {
  }
  scene::Node& loadModel(ArchiveFile& file)
  {
    return loadPicoModel(m_module, file);
  }
};

class ModelPicoDependencies :
  public GlobalFileSystemModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalUndoModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalFiletypesModuleRef
{
};

class ModelPicoAPI : public TypeSystemRef
{
  PicoModelLoader m_modelLoader;
public:
  typedef ModelLoader Type;

  ModelPicoAPI(const char* extension, const picoModule_t* module) :
    m_modelLoader(module)
  {
    StringOutputStream filter(128);
    filter << "*." << extension;
    GlobalFiletypesModule::getTable().addType(Type::Name(), extension, filetype_t(module->displayName, filter.c_str()));
  }
  ModelLoader* getTable()
  {
    return &m_modelLoader;
  }
};

class PicoModelAPIConstructor
{
  CopiedString m_extension;
  const picoModule_t* m_module;
public:
  PicoModelAPIConstructor(const char* extension, const picoModule_t* module) :
    m_extension(extension), m_module(module)
  {
  }
  const char* getName()
  {
    return m_extension.c_str();
  }
  ModelPicoAPI* constructAPI(ModelPicoDependencies& dependencies)
  {
    return new ModelPicoAPI(m_extension.c_str(), m_module);
  }
  void destroyAPI(ModelPicoAPI* api)
  {
    delete api;
  }
};


typedef SingletonModule<ModelPicoAPI, ModelPicoDependencies, PicoModelAPIConstructor> PicoModelModule;
typedef std::list<PicoModelModule> PicoModelModules;
PicoModelModules g_PicoModelModules;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  pico_initialise();

  const picoModule_t** modules = PicoModuleList( 0 );
  while(*modules != 0)
  {
    const picoModule_t* module = *modules++;
    if(module->canload && module->load)
    {
      for(char*const* ext = module->defaultExts; *ext != 0; ++ext)
      {
        g_PicoModelModules.push_back(PicoModelModule(PicoModelAPIConstructor(*ext, module)));
        g_PicoModelModules.back().selfRegister();
      }
    }
  }
}
