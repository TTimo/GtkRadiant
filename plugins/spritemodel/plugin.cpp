/*
Copyright (C) 2002 Dominic Clifton.

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

//
// Sprite Model Plugin
//
// Code by Hydra aka Dominic Clifton
//
// Based on MD3Model source code by SPoG
//

/*
    Overview
    ========


    Why ?
    -----

    It allows the user to see a graphical representation of the entity in the 3D view (maybe 2D views later) where the entity would just otherwise be a non-descriptive coloured box.

    It is designed to be used with the entity view set to WireFrame (as the sprite images are rendered in the middle of the entity's bbox.

    How ?
    -----

    Implemented as a model module, without any ISelect stuff.

    For an entity to use an image (instead of a model) you just update the entity defintion file so that the eclass_t's modelpath is filled in with a relative path and filename of an image file.

    e.g:

      baseq3/scripts/entities.def
      ===========================

      \/\*QUAKED ammo_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED
      ...
      -------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
      model="sprites/powerups/ammo/bfgam.bmp"\*\/


      valve/scripts/halflife.fgd
      ==========================

      @PointClass iconsprite("sprites/lightbulb.spr") base(Target, Targetname, Light) = light : "Invisible   lightsource"
      [
              ...
      ]

    What image formats are supported ?
    ----------------------------------

    This module can load any image format that there is an active image module for.  For q3 this would be bmp, tga and jpg.  For Half-Life this would be hlw and spr.

    Version History
    ===============

    v0.1 - 27/May/2002
      - Created an inital implementation of a sprite model plugin.
        According to the powers that be, it seems creating a model
        plugin is hackish.
        It works ok, but there is no way to attach models (sprites if you will)
        to non-fixedsize entities (like func_bombtarget)
        Also, I can't get the alpha map stuff right so I had to invert the alpha
        mask in the spr loader so that 0xff = not drawn pixel.

    ToDo
    ====

    * make sprites always face the camera (is this done in camwindow.cpp ?)

    * maybe add an option to scale the sprites in the prefs ?

    * un-hack the default fall-though to "sprite" model version (see m_version)

    * maybe convert to a new kind of class not based on model.

    * allow sprites on non-fixedsize ents

    * fix reversed alpha map in spr loader

    * allow an entity to have both an .md? and a sprite model.
*/

#include "plugin.h"
#include "spritemodel.h"

// =============================================================================
// Globals

// function tables
_QERFuncTable_1 __QERTABLENAME;
OpenGLBinding g_QglTable;
_QERShadersTable g_ShadersTable;

// =============================================================================
// SYNAPSE

#include "synapse.h"


char *supportedmodelformats[] = {"spr","bmp","tga","jpg","hlw",NULL}; // NULL is list delimiter

static void add_model_apis(CSynapseClient& client)
{
  char **ext;
  for (ext = supportedmodelformats; *ext != NULL; ext++)
  {
    client.AddAPI(MODEL_MAJOR, *ext, sizeof(_QERPlugModelTable));
  }
}

static bool model_is_supported(const char* extension)
{
  char **ext;
  for (ext = supportedmodelformats; *ext != NULL; ext++)
  {
    if (stricmp(extension,*ext)==0)
      return true;
  }
  return false;
}

void init_filetypes()
{
  char **ext;
  for (ext = supportedmodelformats; *ext != NULL; ext++)
  {
    GetFileTypeRegistry()->addType(MODEL_MAJOR, filetype_t("sprite", *ext));
  }
}

extern CSynapseServer* g_pSynapseServer;

class CSynapseClientModel : public CSynapseClient
{
public:
  // CSynapseClient API
  bool RequestAPI(APIDescriptor_t *pAPI);
  const char* GetInfo();
  const char* GetName();

  CSynapseClientModel() { }
  virtual ~CSynapseClientModel() { }

  bool OnActivate()
  {
    init_filetypes(); // see todo list above.
    return true;
  }
};

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientModel g_SynapseClient;
    
extern "C" CSynapseClient* SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces (const char *version, CSynapseServer *pServer)
{
  if (strcmp(version, SYNAPSE_VERSION))
  {
    Syn_Printf("ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version);
    return NULL;
  }
  g_pSynapseServer = pServer;
  g_pSynapseServer->IncRef();
  Set_Syn_Printf(g_pSynapseServer->Get_Syn_Printf());

  add_model_apis(g_SynapseClient); // see todo list above.

  g_SynapseClient.AddAPI( PLUGIN_MAJOR, "sprite", sizeof( _QERPluginTable ) );
  g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
  g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( g_QglTable ), SYN_REQUIRE, &g_QglTable );
  g_SynapseClient.AddAPI( SHADERS_MAJOR, "*", sizeof( g_ShadersTable ), SYN_REQUIRE, &g_ShadersTable );

  return &g_SynapseClient;
}

bool CSynapseClientModel::RequestAPI(APIDescriptor_t *pAPI)
{
  if (!strcmp(pAPI->major_name, MODEL_MAJOR))
  {
    _QERPlugModelTable* pTable= static_cast<_QERPlugModelTable*>(pAPI->mpTable);

    if (!strcmp(pAPI->minor_name, "sprite"))
    {
      pTable->m_pfnLoadModel = &LoadSpriteModel;
      return true;
    }
  }

  Syn_Printf("ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo());
  return false;
}

#include "version.h"

const char* CSynapseClientModel::GetInfo()
{
  return "Sprite Model module built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientModel::GetName()
{
  return "sprite";
}
