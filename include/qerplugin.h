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

// QERadiant PlugIns
//
//

#ifndef __QERPLUGIN_H__
#define __QERPLUGIN_H__

#include "generic/constant.h"


// ========================================
// GTK+ helper functions

// NOTE: parent can be 0 in all functions but it's best to set them

// this API does not depend on gtk+ or glib
typedef struct _GtkWidget GtkWidget;

enum EMessageBoxType
{
  eMB_OK,
  eMB_OKCANCEL,
  eMB_YESNO,
  eMB_YESNOCANCEL,
  eMB_NOYES,
};

enum EMessageBoxIcon
{
  eMB_ICONDEFAULT,
  eMB_ICONERROR,
  eMB_ICONWARNING,
  eMB_ICONQUESTION,
  eMB_ICONASTERISK,
};

enum EMessageBoxReturn
{
  eIDOK,
  eIDCANCEL,
  eIDYES,
  eIDNO,
};

// simple Message Box, see above for the 'type' flags

typedef EMessageBoxReturn (* PFN_QERAPP_MESSAGEBOX) (GtkWidget *parent, const char* text, const char* caption/* = "GtkRadiant"*/, EMessageBoxType type/* = eMB_OK*/, EMessageBoxIcon icon/* = eMB_ICONDEFAULT*/);

// file and directory selection functions return null if the user hits cancel
// - 'title' is the dialog title (can be null)
// - 'path' is used to set the initial directory (can be null)
// - 'pattern': the first pattern is for the win32 mode, then comes the Gtk pattern list, see Radiant source for samples
typedef const char* (* PFN_QERAPP_FILEDIALOG) (GtkWidget *parent, bool open, const char* title, const char* path/* = 0*/, const char* pattern/* = 0*/);

// returns a gchar* string that must be g_free'd by the user
typedef char* (* PFN_QERAPP_DIRDIALOG) (GtkWidget *parent, const char* title/* = "Choose Directory"*/, const char* path/* = 0*/);

// return true if the user closed the dialog with 'Ok'
// 'color' is used to set the initial value and store the selected value
template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
typedef bool (* PFN_QERAPP_COLORDIALOG) (GtkWidget *parent, Vector3& color,
                                               const char* title/* = "Choose Color"*/);

// load a .bmp file and create a GtkImage widget from it
// NOTE: 'filename' is relative to <radiant_path>/plugins/bitmaps/
typedef struct _GtkImage GtkImage;
typedef GtkImage* (* PFN_QERAPP_NEWIMAGE) (const char* filename);

// ========================================

class ModuleObserver;

// the radiant core API
struct _QERFuncTable_1
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "radiant");

  const char* (*getEnginePath)();
  const char* (*getGameToolsPath)();
  const char* (*getAppPath)();
  const char* (*getSettingsPath)();

  const char* (*getGameName)();
  const char* (*getGameMode)();

  const char* (*getGameDescriptionKeyValue)(const char* key);
  const char* (*getRequiredGameDescriptionKeyValue)(const char* key);

  void (*attachGameToolsPathObserver)(ModuleObserver& observer);
  void (*detachGameToolsPathObserver)(ModuleObserver& observer);
  void (*attachEnginePathObserver)(ModuleObserver& observer);
  void (*detachEnginePathObserver)(ModuleObserver& observer);
  void (*attachGameNameObserver)(ModuleObserver& observer);
  void (*detachGameNameObserver)(ModuleObserver& observer);
  void (*attachGameModeObserver)(ModuleObserver& observer);
  void (*detachGameModeObserver)(ModuleObserver& observer);

  // GTK+ functions
  PFN_QERAPP_MESSAGEBOX  m_pfnMessageBox;
  PFN_QERAPP_FILEDIALOG  m_pfnFileDialog;
  PFN_QERAPP_DIRDIALOG   m_pfnDirDialog;
  PFN_QERAPP_COLORDIALOG m_pfnColorDialog;
  PFN_QERAPP_NEWIMAGE  m_pfnNewImage;

};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<_QERFuncTable_1> GlobalRadiantModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<_QERFuncTable_1> GlobalRadiantModuleRef;

inline _QERFuncTable_1& GlobalRadiant()
{
  return GlobalRadiantModule::getTable();
}

#endif
