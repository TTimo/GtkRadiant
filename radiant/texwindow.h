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

#if !defined(INCLUDED_TEXWINDOW_H)
#define INCLUDED_TEXWINDOW_H

#include "math/vector.h"

// textures menu

typedef struct _GSList GSList;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkMenu GtkMenu;
typedef struct _GtkMenuItem GtkMenuItem;

extern GtkMenu* g_textures_menu;
extern GtkMenuItem* g_textures_menu_separator;
void TextureGroupsMenu_Construct();
void TextureGroupsMenu_Destroy();
void TextureGroupsMenu_ListItems(GSList*& items);
const char* TextureGroupsMenu_GetName(std::size_t menunum);


// texture browser

class TextureBrowser;
TextureBrowser& GlobalTextureBrowser();

typedef struct _GtkWindow GtkWindow;
GtkWidget* TextureBrowser_constructWindow(GtkWindow* toplevel);
void TextureBrowser_destroyWindow();


void TextureBrowser_ShowDirectory(TextureBrowser& textureBrowser, const char* name);
void TextureBrowser_ShowStartupShaders(TextureBrowser& textureBrowser);

const char* TextureBrowser_GetSelectedShader(TextureBrowser& textureBrower);

void TextureBrowser_Construct();
void TextureBrowser_Destroy();

template<typename FirstArgument>
class Callback1;
typedef Callback1<const char*> StringImportCallback;
template<typename FirstArgument, void (*func)(FirstArgument)> 
class FreeCaller1;

extern GtkWidget* g_page_textures;
void TextureBrowser_exportTitle(const StringImportCallback& importer);
typedef FreeCaller1<const StringImportCallback&, TextureBrowser_exportTitle> TextureBrowserExportTitleCaller;

const Vector3& TextureBrowser_getBackgroundColour(TextureBrowser& textureBrowser);
void TextureBrowser_setBackgroundColour(TextureBrowser& textureBrowser, const Vector3& colour);

class Callback;
void TextureBrowser_addActiveShadersChangedCallback(const Callback& callback);
void TextureBrowser_addShadersRealiseCallback(const Callback& callback);

#endif
