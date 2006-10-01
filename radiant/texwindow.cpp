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

//
// Texture Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "texwindow.h"

#include "debugging/debugging.h"
#include "warnings.h"

#include "ifilesystem.h"
#include "iundo.h"
#include "igl.h"
#include "iarchive.h"
#include "moduleobserver.h"

#include <set>
#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <gtk/gtkrange.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvscrollbar.h>

#include "signal/signal.h"
#include "math/vector.h"
#include "texturelib.h"
#include "string/string.h"
#include "shaderlib.h"
#include "os/file.h"
#include "os/path.h"
#include "stream/memstream.h"
#include "stream/textfilestream.h"
#include "stream/stringstream.h"
#include "cmdlib.h"
#include "texmanip.h"
#include "textures.h"
#include "convert.h"

#include "gtkutil/menu.h"
#include "gtkutil/nonmodal.h"
#include "gtkutil/cursor.h"
#include "gtkutil/widget.h"
#include "gtkutil/glwidget.h"
#include "gtkutil/messagebox.h"

#include "error.h"
#include "map.h"
#include "qgl.h"
#include "select.h"
#include "brush_primit.h"
#include "brushmanip.h"
#include "patchmanip.h"
#include "plugin.h"
#include "qe3.h"
#include "gtkdlgs.h"
#include "gtkmisc.h"
#include "mainframe.h"
#include "findtexturedialog.h"
#include "surfacedialog.h"
#include "patchdialog.h"
#include "groupdialog.h"
#include "preferences.h"
#include "shaders.h"
#include "commands.h"

bool TextureBrowser_showWads()
{
  return !string_empty(g_pGameDescription->getKeyValue("show_wads"));
}

void TextureBrowser_queueDraw(TextureBrowser& textureBrowser);

bool string_equal_start(const char* string, StringRange start)
{
  return string_equal_n(string, start.first, start.last - start.first);
}

typedef std::set<CopiedString> TextureGroups;

void TextureGroups_addWad(TextureGroups& groups, const char* archive)
{
  if(extension_equal(path_get_extension(archive), "wad"))
  {
#if 1
    groups.insert(archive);
#else
    CopiedString archiveBaseName(path_get_filename_start(archive), path_get_filename_base_end(archive));
    groups.insert(archiveBaseName);
#endif
  }
}
typedef ReferenceCaller1<TextureGroups, const char*, TextureGroups_addWad> TextureGroupsAddWadCaller;

void TextureGroups_addShader(TextureGroups& groups, const char* shaderName)
{
  const char* texture = path_make_relative(shaderName, "textures/");
  if(texture != shaderName)
  {
    const char* last = path_remove_directory(texture);
    if(!string_empty(last))
    {
      groups.insert(CopiedString(StringRange(texture, --last)));
    }
  }
}
typedef ReferenceCaller1<TextureGroups, const char*, TextureGroups_addShader> TextureGroupsAddShaderCaller;

void TextureGroups_addDirectory(TextureGroups& groups, const char* directory)
{
  groups.insert(directory);
}
typedef ReferenceCaller1<TextureGroups, const char*, TextureGroups_addDirectory> TextureGroupsAddDirectoryCaller;

namespace
{
  bool g_TextureBrowser_shaderlistOnly = false;
}

class DeferredAdjustment
{
  gdouble m_value;
  guint m_handler;
  typedef void (*ValueChangedFunction)(void* data, gdouble value);
  ValueChangedFunction m_function;
  void* m_data;

  static gboolean deferred_value_changed(gpointer data)
  {
    reinterpret_cast<DeferredAdjustment*>(data)->m_function(
      reinterpret_cast<DeferredAdjustment*>(data)->m_data,
      reinterpret_cast<DeferredAdjustment*>(data)->m_value
    );
    reinterpret_cast<DeferredAdjustment*>(data)->m_handler = 0;
    reinterpret_cast<DeferredAdjustment*>(data)->m_value = 0;
    return FALSE;
  }
public:
  DeferredAdjustment(ValueChangedFunction function, void* data) : m_value(0), m_handler(0), m_function(function), m_data(data)
  {
  }
  void flush()
  {
    if(m_handler != 0)
    {
      g_source_remove(m_handler);
      deferred_value_changed(this);
    }
  }
  void value_changed(gdouble value)
  {
    m_value = value;
    if(m_handler == 0)
    {
      m_handler = g_idle_add(deferred_value_changed, this);
    }
  }
  static void adjustment_value_changed(GtkAdjustment *adjustment, DeferredAdjustment* self)
  {
    self->value_changed(adjustment->value);
  }
};



class TextureBrowser;

typedef ReferenceCaller<TextureBrowser, TextureBrowser_queueDraw> TextureBrowserQueueDrawCaller;

void TextureBrowser_scrollChanged(void* data, gdouble value);


enum StartupShaders
{
  STARTUPSHADERS_NONE = 0,
  STARTUPSHADERS_COMMON,
};

void TextureBrowser_hideUnusedExport(const BoolImportCallback& importer);
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_hideUnusedExport> TextureBrowserHideUnusedExport;

void TextureBrowser_showShadersExport(const BoolImportCallback& importer);
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_showShadersExport> TextureBrowserShowShadersExport;

void TextureBrowser_showShaderlistOnly(const BoolImportCallback& importer);
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_showShaderlistOnly> TextureBrowserShowShaderlistOnlyExport;

class TextureBrowser
{
public:
	int width, height;
	int originy;
	int m_nTotalHeight;

  CopiedString shader;

  GtkWindow* m_parent;
  GtkWidget* m_gl_widget;
  GtkWidget* m_texture_scroll;
  GtkWidget* m_treeViewTree;
  GtkWidget* m_treeViewTags;
  GtkWidget* m_tag_frame;
  GtkListStore* m_assigned_store;
  GtkListStore* m_available_store;
  GtkWidget* m_assigned_tree;
  GtkWidget* m_available_tree;
  GtkWidget* m_scr_win_tree;
  GtkWidget* m_scr_win_tags;
  GtkWidget* m_shader_info_item;

  std::set<CopiedString> m_all_tags;
  GtkListStore* m_all_tags_list;
  std::vector<CopiedString> m_copied_tags;
  std::set<CopiedString> m_found_shaders;

  ToggleItem m_hideunused_item;
  ToggleItem m_showshaders_item;
  ToggleItem m_showshaderlistonly_item;

  guint m_sizeHandler;
  guint m_exposeHandler;

  bool m_heightChanged;
  bool m_originInvalid;

  DeferredAdjustment m_scrollAdjustment;
  FreezePointer m_freezePointer;

  Vector3 color_textureback;
  // the increment step we use against the wheel mouse
  std::size_t m_mouseWheelScrollIncrement;
  std::size_t m_textureScale;
  // make the texture increments match the grid changes
  bool m_showShaders;
  bool m_showTextureScrollbar;
  StartupShaders m_startupShaders;
  // if true, the texture window will only display in-use shaders
  // if false, all the shaders in memory are displayed
  bool m_hideUnused;
  bool m_rmbSelected;
  bool m_searchedTags;
  bool m_tags;
  bool m_showTags;

  TextureBrowser() :
    m_texture_scroll(0),
    m_hideunused_item(TextureBrowserHideUnusedExport()),
	m_showshaders_item(TextureBrowserShowShadersExport()),
	m_showshaderlistonly_item(TextureBrowserShowShaderlistOnlyExport()),
    m_heightChanged(true),
    m_originInvalid(true),
    m_scrollAdjustment(TextureBrowser_scrollChanged, this),
    color_textureback(0.25f, 0.25f, 0.25f),
    m_mouseWheelScrollIncrement(64),
    m_textureScale(50),
	m_showShaders(true),
    m_showTextureScrollbar(true),
    m_startupShaders(STARTUPSHADERS_NONE),
	m_hideUnused(false),
	m_rmbSelected(false),
	m_searchedTags(false),
	m_tags(false),
	m_showTags(false)
  {
  }
};

void(*TextureBrowser_textureSelected)(const char* shader);


void TextureBrowser_updateScroll(TextureBrowser& textureBrowser);


const char* TextureBrowser_getComonShadersName()
{
  const char* value = g_pGameDescription->getKeyValue("common_shaders_name");
  if(!string_empty(value))
  {
    return value;
  }
  return "Common";
}

const char* TextureBrowser_getComonShadersDir()
{
  const char* value = g_pGameDescription->getKeyValue("common_shaders_dir");
  if(!string_empty(value))
  {
    return value;
  }
  return "common/";
}

inline int TextureBrowser_fontHeight(TextureBrowser& textureBrowser)
{
  return GlobalOpenGL().m_fontHeight;
}

const char* TextureBrowser_GetSelectedShader(TextureBrowser& textureBrowser)
{
  return textureBrowser.shader.c_str();
}

void TextureBrowser_SetStatus(TextureBrowser& textureBrowser, const char* name)
{
  IShader* shader = QERApp_Shader_ForName( name);
  qtexture_t* q = shader->getTexture();
  StringOutputStream strTex(256);
  strTex << name << " W: " << Unsigned(q->width) << " H: " << Unsigned(q->height);
  shader->DecRef();
  g_pParentWnd->SetStatusText(g_pParentWnd->m_texture_status, strTex.c_str());
}

void TextureBrowser_Focus(TextureBrowser& textureBrowser, const char* name);

void TextureBrowser_SetSelectedShader(TextureBrowser& textureBrowser, const char* shader)
{
  textureBrowser.shader = shader;
  TextureBrowser_SetStatus(textureBrowser, shader);
  TextureBrowser_Focus(textureBrowser, shader);

  if(FindTextureDialog_isOpen())
  {
    FindTextureDialog_selectTexture(shader);
  }

  // disable the menu item "shader info" if no shader was selected
  IShader* ishader = QERApp_Shader_ForName(shader);
  CopiedString filename = ishader->getShaderFileName();

  if(filename.empty())
  {
    gtk_widget_set_sensitive(textureBrowser.m_shader_info_item, FALSE);
  } else {
    gtk_widget_set_sensitive(textureBrowser.m_shader_info_item, TRUE);
  }

  ishader->DecRef();
}


CopiedString g_TextureBrowser_currentDirectory;

/*
============================================================================

TEXTURE LAYOUT

TTimo: now based on a rundown through all the shaders
NOTE: we expect the Active shaders count doesn't change during a Texture_StartPos .. Texture_NextPos cycle
  otherwise we may need to rely on a list instead of an array storage
============================================================================
*/

class TextureLayout
{
public:
  // texture layout functions
  // TTimo: now based on shaders
  int current_x, current_y, current_row;
};

void Texture_StartPos(TextureLayout& layout)
{
  layout.current_x = 8;
  layout.current_y = -8;
  layout.current_row = 0;
}

void Texture_NextPos(TextureBrowser& textureBrowser, TextureLayout& layout, qtexture_t* current_texture, int *x, int *y)
{
  qtexture_t* q = current_texture;

  int nWidth = (int)(q->width * ((float)textureBrowser.m_textureScale / 100));
  int nHeight = (int)(q->height * ((float)textureBrowser.m_textureScale / 100));
  if (layout.current_x + nWidth > textureBrowser.width-8 && layout.current_row)
  { // go to the next row unless the texture is the first on the row
    layout.current_x = 8;
    layout.current_y -= layout.current_row + TextureBrowser_fontHeight(textureBrowser) + 4;
    layout.current_row = 0;
  }

  *x = layout.current_x;
  *y = layout.current_y;

  // Is our texture larger than the row? If so, grow the
  // row height to match it

  if (layout.current_row < nHeight)
    layout.current_row = nHeight;

  // never go less than 64, or the names get all crunched up
  layout.current_x += nWidth < 64 ? 64 : nWidth;
  layout.current_x += 8;
}

bool TextureSearch_IsShown(const char* name)
{
  std::set<CopiedString>::iterator iter;

  iter = GlobalTextureBrowser().m_found_shaders.find(name);

  if(iter == GlobalTextureBrowser().m_found_shaders.end())
  {
    return false;
  } else {
    return true;
  }
}

// if texture_showinuse jump over non in-use textures
bool Texture_IsShown(IShader* shader, bool show_shaders, bool hideUnused)
{
  if(g_TextureBrowser_currentDirectory == "Untagged")
  {
	std::set<CopiedString>::iterator iter;

	iter = GlobalTextureBrowser().m_found_shaders.find(shader->getName());

	if(iter == GlobalTextureBrowser().m_found_shaders.end())
	{
      return false;
	} else {
	  return true;
	}
  }

  if(!shader_equal_prefix(shader->getName(), "textures/"))
    return false;

  if (!show_shaders && !shader->IsDefault())
    return false;

  if(hideUnused && !shader->IsInUse())
    return false;

  if(GlobalTextureBrowser().m_searchedTags)
  {
    if(!TextureSearch_IsShown(shader->getName()))
	{
	  return false;
	} else {
	  return true;
	}
  } else {
    if(!shader_equal_prefix(shader_get_textureName(shader->getName()), g_TextureBrowser_currentDirectory.c_str()))
    {
	  return false;
	}
  }

  return true;
}

void TextureBrowser_heightChanged(TextureBrowser& textureBrowser)
{
  textureBrowser.m_heightChanged = true;

  TextureBrowser_updateScroll(textureBrowser);
  TextureBrowser_queueDraw(textureBrowser);
}

void TextureBrowser_evaluateHeight(TextureBrowser& textureBrowser)
{
  if(textureBrowser.m_heightChanged)
  {
    textureBrowser.m_heightChanged = false;

    textureBrowser.m_nTotalHeight = 0;

    TextureLayout layout;
    Texture_StartPos(layout);
    for(QERApp_ActiveShaders_IteratorBegin(); !QERApp_ActiveShaders_IteratorAtEnd(); QERApp_ActiveShaders_IteratorIncrement())
    {
      IShader* shader = QERApp_ActiveShaders_IteratorCurrent();

      if(!Texture_IsShown(shader, textureBrowser.m_showShaders, textureBrowser.m_hideUnused))
        continue;

      int   x, y;
      Texture_NextPos(textureBrowser, layout, shader->getTexture(), &x, &y);
      textureBrowser.m_nTotalHeight = std::max(textureBrowser.m_nTotalHeight, abs(layout.current_y) + TextureBrowser_fontHeight(textureBrowser) + (int)(shader->getTexture()->height * ((float)textureBrowser.m_textureScale / 100)) + 4);
    }
  }
}

int TextureBrowser_TotalHeight(TextureBrowser& textureBrowser)
{
  TextureBrowser_evaluateHeight(textureBrowser);
  return textureBrowser.m_nTotalHeight;
}

inline const int& min_int(const int& left, const int& right)
{
  return std::min(left, right);
}

void TextureBrowser_clampOriginY(TextureBrowser& textureBrowser)
{
  if(textureBrowser.originy > 0)
  {
    textureBrowser.originy = 0;
  }
  int lower = min_int(textureBrowser.height - TextureBrowser_TotalHeight(textureBrowser), 0);
  if(textureBrowser.originy < lower)
  {
    textureBrowser.originy = lower;
  }
}

int TextureBrowser_getOriginY(TextureBrowser& textureBrowser)
{
  if(textureBrowser.m_originInvalid)
  {
    textureBrowser.m_originInvalid = false;
    TextureBrowser_clampOriginY(textureBrowser);
    TextureBrowser_updateScroll(textureBrowser);
  }
  return textureBrowser.originy;
}

void TextureBrowser_setOriginY(TextureBrowser& textureBrowser, int originy)
{
  textureBrowser.originy = originy;
  TextureBrowser_clampOriginY(textureBrowser);
  TextureBrowser_updateScroll(textureBrowser);
  TextureBrowser_queueDraw(textureBrowser);
}


Signal0 g_activeShadersChangedCallbacks;

void TextureBrowser_addActiveShadersChangedCallback(const SignalHandler& handler)
{
  g_activeShadersChangedCallbacks.connectLast(handler);
}

class ShadersObserver : public ModuleObserver
{
  Signal0 m_realiseCallbacks;
public:
  void realise()
  {
    m_realiseCallbacks();
  }
  void unrealise()
  {
  }
  void insert(const SignalHandler& handler)
  {
    m_realiseCallbacks.connectLast(handler);
  }
};

namespace
{
  ShadersObserver g_ShadersObserver;
}

void TextureBrowser_addShadersRealiseCallback(const SignalHandler& handler)
{
  g_ShadersObserver.insert(handler);
}

void TextureBrowser_activeShadersChanged(TextureBrowser& textureBrowser)
{
  TextureBrowser_heightChanged(textureBrowser);
  textureBrowser.m_originInvalid = true;

  g_activeShadersChangedCallbacks();
}

void TextureBrowser_importShowScrollbar(TextureBrowser& textureBrowser, bool value)
{
  textureBrowser.m_showTextureScrollbar = value;
  if(textureBrowser.m_texture_scroll != 0)
  {
    widget_set_visible(textureBrowser.m_texture_scroll, textureBrowser.m_showTextureScrollbar);
    TextureBrowser_updateScroll(textureBrowser);
  }
}
typedef ReferenceCaller1<TextureBrowser, bool, TextureBrowser_importShowScrollbar> TextureBrowserImportShowScrollbarCaller;


/*
==============
TextureBrowser_ShowDirectory
relies on texture_directory global for the directory to use
1) Load the shaders for the given directory
2) Scan the remaining texture, load them and assign them a default shader (the "noshader" shader)
NOTE: when writing a texture plugin, or some texture extensions, this function may need to be overriden, and made
  available through the IShaders interface
NOTE: for texture window layout:
  all shaders are stored with alphabetical order after load
  previously loaded and displayed stuff is hidden, only in-use and newly loaded is shown
  ( the GL textures are not flushed though)
==============
*/
bool texture_name_ignore(const char* name)
{
  StringOutputStream strTemp(string_length(name));
  strTemp << LowerCase(name);

  return strstr(strTemp.c_str(), ".specular") != 0 ||
    strstr(strTemp.c_str(), ".glow") != 0 ||
    strstr(strTemp.c_str(), ".bump") != 0 ||
    strstr(strTemp.c_str(), ".diffuse") != 0 ||
    strstr(strTemp.c_str(), ".blend") != 0 ||
	  strstr(strTemp.c_str(), ".alpha") != 0;
}

class LoadShaderVisitor : public Archive::Visitor
{
public:
  void visit(const char* name)
  {
    IShader* shader = QERApp_Shader_ForName(CopiedString(StringRange(name, path_get_filename_base_end(name))).c_str());
    shader->DecRef();
  }
};

void TextureBrowser_SetHideUnused(TextureBrowser& textureBrowser, bool hideUnused);

GtkWidget* g_page_textures;

void TextureBrowser_toggleShow() 
{
  GroupDialog_showPage(g_page_textures);
}


void TextureBrowser_updateTitle()
{
  GroupDialog_updatePageTitle(g_page_textures);
}



class TextureCategoryLoadShader
{
  const char* m_directory;
  std::size_t& m_count;
public:
  typedef const char* first_argument_type;

  TextureCategoryLoadShader(const char* directory, std::size_t& count)
    : m_directory(directory), m_count(count)
  {
    m_count = 0;
  }
  void operator()(const char* name) const
  {
    if(shader_equal_prefix(name, "textures/")
      && shader_equal_prefix(name + string_length("textures/"), m_directory))
    {
      ++m_count;
      // request the shader, this will load the texture if needed
      // this Shader_ForName call is a kind of hack
      IShader *pFoo = QERApp_Shader_ForName(name);
      pFoo->DecRef();
    }
  }
};

void TextureDirectory_loadTexture(const char* directory, const char* texture)
{
  StringOutputStream name(256);
  name << directory << StringRange(texture, path_get_filename_base_end(texture));

  if(texture_name_ignore(name.c_str()))
  {
    return;
  }

  if (!shader_valid(name.c_str()))
  {
    globalOutputStream() << "Skipping invalid texture name: [" << name.c_str() << "]\n";
    return;
  }

  // if a texture is already in use to represent a shader, ignore it
  IShader* shader = QERApp_Shader_ForName(name.c_str());
  shader->DecRef();
}
typedef ConstPointerCaller1<char, const char*, TextureDirectory_loadTexture> TextureDirectoryLoadTextureCaller;

class LoadTexturesByTypeVisitor : public ImageModules::Visitor
{
  const char* m_dirstring;
public:
  LoadTexturesByTypeVisitor(const char* dirstring)
    : m_dirstring(dirstring)
  {
  }
  void visit(const char* minor, const _QERPlugImageTable& table) const
  {
    GlobalFileSystem().forEachFile(m_dirstring, minor, TextureDirectoryLoadTextureCaller(m_dirstring));
  }
};

void TextureBrowser_ShowDirectory(TextureBrowser& textureBrowser, const char* directory)
{
  if(TextureBrowser_showWads())
  {
    Archive* archive = GlobalFileSystem().getArchive(directory);
    ASSERT_NOTNULL(archive);
    LoadShaderVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(visitor, Archive::eFiles, 0), "textures/");
  }
  else
  {
    g_TextureBrowser_currentDirectory = directory;
    TextureBrowser_heightChanged(textureBrowser);

    std::size_t shaders_count;
    GlobalShaderSystem().foreachShaderName(makeCallback1(TextureCategoryLoadShader(directory, shaders_count)));
    globalOutputStream() << "Showing " << Unsigned(shaders_count) << " shaders.\n";

    if(g_pGameDescription->mGameType != "doom3")
    {
      // load remaining texture files

      StringOutputStream dirstring(64);
      dirstring << "textures/" << directory;

      Radiant_getImageModules().foreachModule(LoadTexturesByTypeVisitor(dirstring.c_str()));
    }
  }

  // we'll display the newly loaded textures + all the ones already in use
  TextureBrowser_SetHideUnused(textureBrowser, false);

  TextureBrowser_updateTitle();
}

void TextureBrowser_ShowTagSearchResult(TextureBrowser& textureBrowser, const char* directory)
{
  g_TextureBrowser_currentDirectory = directory;
  TextureBrowser_heightChanged(textureBrowser);

  std::size_t shaders_count;
  GlobalShaderSystem().foreachShaderName(makeCallback1(TextureCategoryLoadShader(directory, shaders_count)));
  globalOutputStream() << "Showing " << Unsigned(shaders_count) << " shaders.\n";

  if(g_pGameDescription->mGameType != "doom3")
  {
    // load remaining texture files
    StringOutputStream dirstring(64);
    dirstring << "textures/" << directory;

    {
      LoadTexturesByTypeVisitor visitor(dirstring.c_str());
      Radiant_getImageModules().foreachModule(visitor);
    }
  }

  // we'll display the newly loaded textures + all the ones already in use
  TextureBrowser_SetHideUnused(textureBrowser, false);
}


bool TextureBrowser_hideUnused();

void TextureBrowser_hideUnusedExport(const BoolImportCallback& importer)
{
  importer(TextureBrowser_hideUnused());
}
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_hideUnusedExport> TextureBrowserHideUnusedExport;

void TextureBrowser_showShadersExport(const BoolImportCallback& importer)
{
  importer(GlobalTextureBrowser().m_showShaders);
}
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_showShadersExport> TextureBrowserShowShadersExport;

void TextureBrowser_showShaderlistOnly(const BoolImportCallback& importer)
{
  importer(g_TextureBrowser_shaderlistOnly);
}
typedef FreeCaller1<const BoolImportCallback&, TextureBrowser_showShaderlistOnly> TextureBrowserShowShaderlistOnlyExport;

void TextureBrowser_SetHideUnused(TextureBrowser& textureBrowser, bool hideUnused)
{
  if(hideUnused)
  {
    textureBrowser.m_hideUnused = true;
  }
  else
  {
    textureBrowser.m_hideUnused = false;
  }

  textureBrowser.m_hideunused_item.update();

  TextureBrowser_heightChanged(textureBrowser);
  textureBrowser.m_originInvalid = true;
}

void TextureBrowser_ShowStartupShaders(TextureBrowser& textureBrowser)
{
  if(textureBrowser.m_startupShaders == STARTUPSHADERS_COMMON)
  {
    TextureBrowser_ShowDirectory(textureBrowser, TextureBrowser_getComonShadersDir());
  }
}


//++timo NOTE: this is a mix of Shader module stuff and texture explorer
// it might need to be split in parts or moved out .. dunno
// scroll origin so the specified texture is completely on screen
// if current texture is not displayed, nothing is changed
void TextureBrowser_Focus(TextureBrowser& textureBrowser, const char* name)
{
  TextureLayout layout;
  // scroll origin so the texture is completely on screen
  Texture_StartPos(layout);
  
  for(QERApp_ActiveShaders_IteratorBegin(); !QERApp_ActiveShaders_IteratorAtEnd(); QERApp_ActiveShaders_IteratorIncrement())
  {
    IShader* shader = QERApp_ActiveShaders_IteratorCurrent();

    if(!Texture_IsShown(shader, textureBrowser.m_showShaders, textureBrowser.m_hideUnused))
      continue;

    int x, y;
    Texture_NextPos(textureBrowser, layout, shader->getTexture(), &x, &y);
    qtexture_t* q = shader->getTexture();
    if (!q)
      break;

    // we have found when texdef->name and the shader name match
    // NOTE: as everywhere else for our comparisons, we are not case sensitive
    if (shader_equal(name, shader->getName()))
    {
      int textureHeight = (int)(q->height * ((float)textureBrowser.m_textureScale / 100))
        + 2 * TextureBrowser_fontHeight(textureBrowser);

      int originy = TextureBrowser_getOriginY(textureBrowser);
      if (y > originy)
      {
        originy = y;
      }

      if (y - textureHeight < originy - textureBrowser.height)
      {
        originy = (y - textureHeight) + textureBrowser.height;
      }

      TextureBrowser_setOriginY(textureBrowser, originy);
      return;
    }
  }
}

IShader* Texture_At(TextureBrowser& textureBrowser, int mx, int my)
{
  my += TextureBrowser_getOriginY(textureBrowser) - textureBrowser.height;

  TextureLayout layout;
  Texture_StartPos(layout);
  for(QERApp_ActiveShaders_IteratorBegin(); !QERApp_ActiveShaders_IteratorAtEnd(); QERApp_ActiveShaders_IteratorIncrement())
  {
    IShader* shader = QERApp_ActiveShaders_IteratorCurrent();

    if(!Texture_IsShown(shader, textureBrowser.m_showShaders, textureBrowser.m_hideUnused))
      continue;

    int   x, y;
    Texture_NextPos(textureBrowser, layout, shader->getTexture(), &x, &y);
    qtexture_t  *q = shader->getTexture();
    if (!q)
      break;

    int nWidth = (int)(q->width * ((float)textureBrowser.m_textureScale / 100));
    int nHeight = (int)(q->height * ((float)textureBrowser.m_textureScale / 100));
    if (mx > x && mx - x < nWidth
      && my < y && y - my < nHeight + TextureBrowser_fontHeight(textureBrowser))
    {
      return shader;
    }
  }

  return 0;
}

/*
==============
SelectTexture

  By mouse click
==============
*/
void SelectTexture(TextureBrowser& textureBrowser, int mx, int my, bool bShift)
{
  IShader* shader = Texture_At(textureBrowser, mx, my);
  if(shader != 0)
  {
    if (bShift)
    {
      if (shader->IsDefault())
        globalOutputStream() << "ERROR: " << shader->getName() << " is not a shader, it's a texture.\n";
      else
        ViewShader( shader->getShaderFileName(), shader->getName() );
    }
    else
    {
      TextureBrowser_SetSelectedShader(textureBrowser, shader->getName());
      TextureBrowser_textureSelected(shader->getName());

      if (!FindTextureDialog_isOpen() && !textureBrowser.m_rmbSelected)
      {
        UndoableCommand undo("textureNameSetSelected");
        Select_SetShader(shader->getName());
      }
    }
  }
}

/*
============================================================================

  MOUSE ACTIONS

============================================================================
*/

void TextureBrowser_trackingDelta(int x, int y, unsigned int state, void* data)
{
  TextureBrowser& textureBrowser = *reinterpret_cast<TextureBrowser*>(data);
  if(y != 0)
  {
    int scale = 1;

    if(state & GDK_SHIFT_MASK)
      scale = 4;

    int originy = TextureBrowser_getOriginY(textureBrowser);
    originy += y * scale;
    TextureBrowser_setOriginY(textureBrowser, originy);
  }
}

void TextureBrowser_Tracking_MouseDown(TextureBrowser& textureBrowser)
{
  textureBrowser.m_freezePointer.freeze_pointer(textureBrowser.m_parent, TextureBrowser_trackingDelta, &textureBrowser);
}

void TextureBrowser_Tracking_MouseUp(TextureBrowser& textureBrowser)
{
  textureBrowser.m_freezePointer.unfreeze_pointer(textureBrowser.m_parent);
}

void TextureBrowser_Selection_MouseDown(TextureBrowser& textureBrowser, guint32 flags, int pointx, int pointy)
{
  SelectTexture(textureBrowser, pointx, textureBrowser.height - 1 - pointy, (flags & GDK_SHIFT_MASK) != 0);
}

/*
============================================================================

DRAWING

============================================================================
*/

/*
============
Texture_Draw
TTimo: relying on the shaders list to display the textures
we must query all qtexture_t* to manage and display through the IShaders interface
this allows a plugin to completely override the texture system
============
*/
void Texture_Draw(TextureBrowser& textureBrowser)
{
  int originy = TextureBrowser_getOriginY(textureBrowser);

  glClearColor(textureBrowser.color_textureback[0],
    textureBrowser.color_textureback[1],
    textureBrowser.color_textureback[2],
    0);
  glViewport(0, 0, textureBrowser.width, textureBrowser.height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable (GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glOrtho (0, textureBrowser.width, originy-textureBrowser.height, originy, -100, 100);
  glEnable (GL_TEXTURE_2D);

  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  int last_y = 0, last_height = 0;

  TextureLayout layout;
  Texture_StartPos(layout);
  for(QERApp_ActiveShaders_IteratorBegin(); !QERApp_ActiveShaders_IteratorAtEnd(); QERApp_ActiveShaders_IteratorIncrement())
  {
    IShader* shader = QERApp_ActiveShaders_IteratorCurrent();

    if(!Texture_IsShown(shader, textureBrowser.m_showShaders, textureBrowser.m_hideUnused))
      continue;

    int x, y;
    Texture_NextPos(textureBrowser, layout, shader->getTexture(), &x, &y);
    qtexture_t *q = shader->getTexture();
    if (!q)
      break;

    int nWidth = (int)(q->width * ((float)textureBrowser.m_textureScale / 100));
    int nHeight = (int)(q->height * ((float)textureBrowser.m_textureScale / 100));

    if (y != last_y)
    {
      last_y = y;
      last_height = 0;
    }
    last_height = std::max (nHeight, last_height);

    // Is this texture visible?
    if ((y-nHeight-TextureBrowser_fontHeight(textureBrowser) < originy)
        && (y > originy - textureBrowser.height))
    {
      // borders rules:
      // if it's the current texture, draw a thick red line, else:
      // shaders have a white border, simple textures don't
      // if !texture_showinuse: (some textures displayed may not be in use)
      // draw an additional square around with 0.5 1 0.5 color
      if (shader_equal(TextureBrowser_GetSelectedShader(textureBrowser), shader->getName()))
      {
	      glLineWidth (3);
		  if(textureBrowser.m_rmbSelected)
		  {
			  glColor3f (0,0,1);
		  } else {
              glColor3f (1,0,0);
		  }
	      glDisable (GL_TEXTURE_2D);

	      glBegin (GL_LINE_LOOP);
	      glVertex2i (x-4,y-TextureBrowser_fontHeight(textureBrowser)+4);
	      glVertex2i (x-4,y-TextureBrowser_fontHeight(textureBrowser)-nHeight-4);
	      glVertex2i (x+4+nWidth,y-TextureBrowser_fontHeight(textureBrowser)-nHeight-4);
	      glVertex2i (x+4+nWidth,y-TextureBrowser_fontHeight(textureBrowser)+4);
	      glEnd();

	      glEnable (GL_TEXTURE_2D);
	      glLineWidth (1);
      }
      else
      {
	      glLineWidth (1);
	      // shader border:
	      if (!shader->IsDefault())
	      {
	        glColor3f (1,1,1);
	        glDisable (GL_TEXTURE_2D);

	        glBegin (GL_LINE_LOOP);
	        glVertex2i (x-1,y+1-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x-1,y-nHeight-1-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x+1+nWidth,y-nHeight-1-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x+1+nWidth,y+1-TextureBrowser_fontHeight(textureBrowser));
	        glEnd();
	        glEnable (GL_TEXTURE_2D);
	      }

	      // highlight in-use textures
	      if (!textureBrowser.m_hideUnused && shader->IsInUse())
	      {
	        glColor3f (0.5,1,0.5);
	        glDisable (GL_TEXTURE_2D);
	        glBegin (GL_LINE_LOOP);
	        glVertex2i (x-3,y+3-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x-3,y-nHeight-3-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x+3+nWidth,y-nHeight-3-TextureBrowser_fontHeight(textureBrowser));
	        glVertex2i (x+3+nWidth,y+3-TextureBrowser_fontHeight(textureBrowser));
	        glEnd();
	        glEnable (GL_TEXTURE_2D);
	      }
      }

      // Draw the texture
      glBindTexture (GL_TEXTURE_2D, q->texture_number);
      GlobalOpenGL_debugAssertNoErrors();
      glColor3f (1,1,1);
      glBegin (GL_QUADS);
      glTexCoord2i (0,0);
      glVertex2i (x,y-TextureBrowser_fontHeight(textureBrowser));
      glTexCoord2i (1,0);
      glVertex2i (x+nWidth,y-TextureBrowser_fontHeight(textureBrowser));
      glTexCoord2i (1,1);
      glVertex2i (x+nWidth,y-TextureBrowser_fontHeight(textureBrowser)-nHeight);
      glTexCoord2i (0,1);
      glVertex2i (x,y-TextureBrowser_fontHeight(textureBrowser)-nHeight);
      glEnd();

      // draw the texture name
      glDisable (GL_TEXTURE_2D);
      glColor3f (1,1,1);

      glRasterPos2i (x, y-TextureBrowser_fontHeight(textureBrowser)+2);

      // don't draw the directory name
      const char* name = shader->getName();
      name += strlen(name);
      while(name != shader->getName() && *(name-1) != '/' && *(name-1) != '\\')
        name--;

      GlobalOpenGL().drawString(name);
      glEnable (GL_TEXTURE_2D);
    }

    //int totalHeight = abs(y) + last_height + TextureBrowser_fontHeight(textureBrowser) + 4;
  }


  // reset the current texture
  glBindTexture(GL_TEXTURE_2D, 0);
  //qglFinish();
}

void TextureBrowser_queueDraw(TextureBrowser& textureBrowser)
{
  if(textureBrowser.m_gl_widget != 0)
  {
    gtk_widget_queue_draw(textureBrowser.m_gl_widget);
  }
}


void TextureBrowser_setScale(TextureBrowser& textureBrowser, std::size_t scale)
{
  textureBrowser.m_textureScale = scale;

  TextureBrowser_queueDraw(textureBrowser);
}


void TextureBrowser_MouseWheel(TextureBrowser& textureBrowser, bool bUp)
{
  int originy = TextureBrowser_getOriginY(textureBrowser);

  if (bUp)
  {
    originy += int(textureBrowser.m_mouseWheelScrollIncrement);
  }
  else
  {
    originy -= int(textureBrowser.m_mouseWheelScrollIncrement);
  }

  TextureBrowser_setOriginY(textureBrowser, originy);
}

XmlTagBuilder TagBuilder;

enum
{
   TAG_COLUMN,
   N_COLUMNS
};

void BuildStoreAssignedTags(GtkListStore* store, const char* shader, TextureBrowser* textureBrowser)
{
  GtkTreeIter iter;

  gtk_list_store_clear(store);

  std::vector<CopiedString> assigned_tags;
  TagBuilder.GetShaderTags(shader, assigned_tags);

  for (size_t i = 0; i < assigned_tags.size(); i++)
  {
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, TAG_COLUMN, assigned_tags[i].c_str(), -1);
  }
}

void BuildStoreAvailableTags(	GtkListStore* storeAvailable,
												GtkListStore* storeAssigned,
												const std::set<CopiedString>& allTags,
												TextureBrowser* textureBrowser)
{
  GtkTreeIter iterAssigned;
  GtkTreeIter iterAvailable;
  std::set<CopiedString>::const_iterator iterAll;
  gchar* tag_assigned;

  gtk_list_store_clear(storeAvailable);

  bool row = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(storeAssigned), &iterAssigned) != 0;

  if(!row) // does the shader have tags assigned?
  {
	for (iterAll = allTags.begin(); iterAll != allTags.end(); ++iterAll)
	{
		gtk_list_store_append (storeAvailable, &iterAvailable);
		gtk_list_store_set (storeAvailable, &iterAvailable, TAG_COLUMN, (*iterAll).c_str(), -1);
	}
  }
  else
  {
    while(row) // available tags = all tags - assigned tags
	{
	  gtk_tree_model_get(GTK_TREE_MODEL(storeAssigned), &iterAssigned, TAG_COLUMN, &tag_assigned, -1);

	  for (iterAll = allTags.begin(); iterAll != allTags.end(); ++iterAll)
	  {
		if(strcmp((char*)tag_assigned, (*iterAll).c_str()) != 0)
		{
		  gtk_list_store_append (storeAvailable, &iterAvailable);
		  gtk_list_store_set (storeAvailable, &iterAvailable, TAG_COLUMN, (*iterAll).c_str(), -1);
		}
		else 
		{
		  row = gtk_tree_model_iter_next(GTK_TREE_MODEL(storeAssigned), &iterAssigned) != 0;

		  if(row)
		  {
			gtk_tree_model_get(GTK_TREE_MODEL(storeAssigned), &iterAssigned, TAG_COLUMN, &tag_assigned, -1);
		  }
		}
	  }
	}
  }
}

gboolean TextureBrowser_button_press(GtkWidget* widget, GdkEventButton* event, TextureBrowser* textureBrowser)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    if(event->button == 3)
    {
	  if(GlobalTextureBrowser().m_tags)
	  {
        textureBrowser->m_rmbSelected = true;
        TextureBrowser_Selection_MouseDown (*textureBrowser, event->state, static_cast<int>(event->x), static_cast<int>(event->y));

        BuildStoreAssignedTags(textureBrowser->m_assigned_store, textureBrowser->shader.c_str(), textureBrowser);
        BuildStoreAvailableTags(textureBrowser->m_available_store, textureBrowser->m_assigned_store, textureBrowser->m_all_tags, textureBrowser);
		textureBrowser->m_heightChanged = true;
	    gtk_widget_show(textureBrowser->m_tag_frame);

		process_gui();
		
		TextureBrowser_Focus(*textureBrowser, textureBrowser->shader.c_str());
	  }
	  else
	  {
        TextureBrowser_Tracking_MouseDown(*textureBrowser);
	  }
    }
    else if(event->button == 1)
    {
      TextureBrowser_Selection_MouseDown(*textureBrowser, event->state, static_cast<int>(event->x), static_cast<int>(event->y));

	  if(GlobalTextureBrowser().m_tags)
	  {
        textureBrowser->m_rmbSelected = false;
	    gtk_widget_hide(textureBrowser->m_tag_frame);
	  }
    }
  }
  return FALSE;
}

gboolean TextureBrowser_button_release(GtkWidget* widget, GdkEventButton* event, TextureBrowser* textureBrowser)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    if(event->button == 3)
    {
	  if(!GlobalTextureBrowser().m_tags)
	  {
        TextureBrowser_Tracking_MouseUp(*textureBrowser);
	  }
    }
  }
  return FALSE;
}

gboolean TextureBrowser_motion(GtkWidget *widget, GdkEventMotion *event, TextureBrowser* textureBrowser)
{
  return FALSE;
}

gboolean TextureBrowser_scroll(GtkWidget* widget, GdkEventScroll* event, TextureBrowser* textureBrowser)
{
  if(event->direction == GDK_SCROLL_UP)
  {
    TextureBrowser_MouseWheel(*textureBrowser, true);
  }
  else if(event->direction == GDK_SCROLL_DOWN)
  {
    TextureBrowser_MouseWheel(*textureBrowser, false);
  }
  return FALSE;
}

void TextureBrowser_scrollChanged(void* data, gdouble value)
{
  //globalOutputStream() << "vertical scroll\n";
  TextureBrowser_setOriginY(*reinterpret_cast<TextureBrowser*>(data), -(int)value);
}

static void TextureBrowser_verticalScroll(GtkAdjustment *adjustment, TextureBrowser* textureBrowser)
{
  textureBrowser->m_scrollAdjustment.value_changed(adjustment->value);
}

void TextureBrowser_updateScroll(TextureBrowser& textureBrowser)
{
  if(textureBrowser.m_showTextureScrollbar)
  {
    int totalHeight = TextureBrowser_TotalHeight(textureBrowser);

    totalHeight = std::max(totalHeight, textureBrowser.height);

    GtkAdjustment *vadjustment = gtk_range_get_adjustment(GTK_RANGE(textureBrowser.m_texture_scroll));

    vadjustment->value = -TextureBrowser_getOriginY(textureBrowser);
    vadjustment->page_size = textureBrowser.height;
    vadjustment->page_increment = textureBrowser.height/2;
    vadjustment->step_increment = 20;
    vadjustment->lower = 0;
    vadjustment->upper = totalHeight;

    g_signal_emit_by_name(G_OBJECT (vadjustment), "changed");
  }
}

gboolean TextureBrowser_size_allocate(GtkWidget* widget, GtkAllocation* allocation, TextureBrowser* textureBrowser)
{
  textureBrowser->width = allocation->width;
  textureBrowser->height = allocation->height;
  TextureBrowser_heightChanged(*textureBrowser);
  textureBrowser->m_originInvalid = true;
  TextureBrowser_queueDraw(*textureBrowser);
  return FALSE;
}

gboolean TextureBrowser_expose(GtkWidget* widget, GdkEventExpose* event, TextureBrowser* textureBrowser)
{
  if(glwidget_make_current(textureBrowser->m_gl_widget) != FALSE)
  {
    GlobalOpenGL_debugAssertNoErrors();
    TextureBrowser_evaluateHeight(*textureBrowser);
    Texture_Draw(*textureBrowser);
    GlobalOpenGL_debugAssertNoErrors();
    glwidget_swap_buffers(textureBrowser->m_gl_widget);
  }
  return FALSE;
}


TextureBrowser g_TextureBrowser;

TextureBrowser& GlobalTextureBrowser()
{
  return g_TextureBrowser;
}

bool TextureBrowser_hideUnused()
{
  return g_TextureBrowser.m_hideUnused;
}

void TextureBrowser_ToggleHideUnused()
{
  if(g_TextureBrowser.m_hideUnused)
  {
    TextureBrowser_SetHideUnused(g_TextureBrowser, false);
  }
  else
  {
    TextureBrowser_SetHideUnused(g_TextureBrowser, true);
  }
}

void TextureGroups_constructTreeModel(TextureGroups groups, GtkTreeStore* store)
{
  // put the information from the old textures menu into a treeview 
  GtkTreeIter iter, child;

  TextureGroups::const_iterator i = groups.begin();
  while (i != groups.end())
  {
    const char* dirName = (*i).c_str();
    const char* firstUnderscore = strchr(dirName, '_');
    StringRange dirRoot (dirName, (firstUnderscore == 0) ? dirName : firstUnderscore + 1);

    TextureGroups::const_iterator next = i;
    ++next;
    if(firstUnderscore != 0
      && next != groups.end()
      && string_equal_start((*next).c_str(), dirRoot))
    {
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set (store, &iter, 0, CopiedString(StringRange(dirName, firstUnderscore)).c_str(), -1);

	    // keep going...
	    while (i != groups.end() && string_equal_start((*i).c_str(), dirRoot))
	    {
		  gtk_tree_store_append(store, &child, &iter);
		  gtk_tree_store_set (store, &child, 0, (*i).c_str(), -1);
	      ++i;
	    }
    }
    else
    {
	  gtk_tree_store_append(store, &iter, NULL);
	  gtk_tree_store_set (store, &iter, 0, dirName, -1);
      ++i;
    }
  }
}

TextureGroups TextureGroups_constructTreeView()
{
  TextureGroups groups;

  if (TextureBrowser_showWads())
  {
    GlobalFileSystem().forEachArchive (TextureGroupsAddWadCaller (groups));
  }
  else
  {
    // scan texture dirs and pak files only if not restricting to shaderlist
    if (g_pGameDescription->mGameType != "doom3" && !g_TextureBrowser_shaderlistOnly)
    {
      GlobalFileSystem().forEachDirectory ("textures/", TextureGroupsAddDirectoryCaller(groups));
    }

    GlobalShaderSystem().foreachShaderName(TextureGroupsAddShaderCaller(groups));
  }

  return groups;
}

void TextureBrowser_constructTreeStore()
{
  TextureGroups groups = TextureGroups_constructTreeView();
  GtkTreeStore* store = gtk_tree_store_new(1, G_TYPE_STRING);
  TextureGroups_constructTreeModel(groups, store);
  std::set<CopiedString>::iterator iter;

  GtkTreeModel* model = GTK_TREE_MODEL(store);

  gtk_tree_view_set_model(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTree), model);

  g_object_unref(G_OBJECT(store));
}

void TextureBrowser_constructTreeStoreTags()
{
  TextureGroups groups;
  GtkTreeStore* store = gtk_tree_store_new(1, G_TYPE_STRING);
  GtkTreeModel* model = GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list);

  gtk_tree_view_set_model(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags), model);

  g_object_unref(G_OBJECT(store));
}

void TreeView_onRowActivated(GtkTreeView* treeview, GtkTreePath* path, GtkTreeViewColumn* col, gpointer userdata)
{
  GtkTreeIter iter;

  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

  if (gtk_tree_model_get_iter (model, &iter, path))
  {
    gchar* dirName;
    gtk_tree_model_get(model, &iter, 0, &dirName, -1);

	g_TextureBrowser.m_searchedTags = false;

    if(!TextureBrowser_showWads())
    {
      char buffer[1024];
      strcpy(buffer, dirName);
      strcat(buffer, "/");
      dirName = buffer;
    }

    ScopeDisableScreenUpdates disableScreenUpdates(dirName, "Loading Textures");
    TextureBrowser_ShowDirectory(GlobalTextureBrowser (), dirName);
    TextureBrowser_queueDraw(GlobalTextureBrowser ());
  }
}

void TextureBrowser_createTreeViewTree()
{
  GtkCellRenderer* renderer;
  g_TextureBrowser.m_treeViewTree = GTK_WIDGET(gtk_tree_view_new());

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTree), FALSE);
  g_signal_connect(g_TextureBrowser.m_treeViewTree, "row-activated", (GCallback) TreeView_onRowActivated, NULL);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTree), -1, "", renderer, "text", 0, NULL);

  TextureBrowser_constructTreeStore();
}

void TextureBrowser_createTreeViewTags()
{
  GtkCellRenderer* renderer;
  g_TextureBrowser.m_treeViewTags = GTK_WIDGET(gtk_tree_view_new());

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags), FALSE);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags), -1, "", renderer, "text", 0, NULL);

  TextureBrowser_constructTreeStoreTags();
}

GtkMenuItem* TextureBrowser_constructViewMenu(GtkMenu* menu)
{
  GtkMenuItem* textures_menu_item = new_sub_menu_item_with_mnemonic("_View");
  GtkWidget* separator = gtk_separator_menu_item_new();

  if(g_Layout_enableDetachableMenus.m_value)
    menu_tearoff (menu);

  create_check_menu_item_with_mnemonic(menu, "Hide _Unused", "ShowInUse");
  create_menu_item_with_mnemonic(menu, "Show All", "ShowAllTextures");

  // we always want to show shaders but don't want a "Show Shaders" menu for doom3 games 
  if(string_equal(g_pGameDescription->getRequiredKeyValue("shaders"), "doom3"))
  {
    g_TextureBrowser.m_showShaders = true;
  }
  else
  {
    create_check_menu_item_with_mnemonic(menu, "Show shaders", "ToggleShowShaders");
  }

  create_check_menu_item_with_mnemonic (menu, "Shaders Only", "ToggleShowShaderlistOnly");
  if(g_TextureBrowser.m_tags)
  {
    create_menu_item_with_mnemonic(menu, "Show Untagged", "ShowUntagged");
  }
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
  gtk_widget_show(separator);

  g_TextureBrowser.m_shader_info_item = GTK_WIDGET(create_menu_item_with_mnemonic(menu, "Shader Info", "ShaderInfo"));
  gtk_widget_set_sensitive(g_TextureBrowser.m_shader_info_item, FALSE);

  return textures_menu_item;
}

GtkMenuItem* TextureBrowser_constructToolsMenu(GtkMenu* menu)
{
  GtkMenuItem* textures_menu_item = new_sub_menu_item_with_mnemonic("_Tools");

  if (g_Layout_enableDetachableMenus.m_value)
    menu_tearoff (menu);

  create_menu_item_with_mnemonic(menu, "Flush & Reload Shaders", "RefreshShaders");
  create_menu_item_with_mnemonic(menu, "Find / Replace...", "FindReplaceTextures");

  return textures_menu_item;
}

GtkMenuItem* TextureBrowser_constructTagsMenu(GtkMenu* menu)
{
  GtkMenuItem* textures_menu_item = new_sub_menu_item_with_mnemonic("T_ags");
  GtkWidget* separator = gtk_separator_menu_item_new();

  if (g_Layout_enableDetachableMenus.m_value)
    menu_tearoff (menu);

  create_menu_item_with_mnemonic(menu, "Add tag", "AddTag");
  create_menu_item_with_mnemonic(menu, "Rename tag", "RenameTag");
  create_menu_item_with_mnemonic(menu, "Delete tag", "DeleteTag");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
  gtk_widget_show(separator);
  create_menu_item_with_mnemonic(menu, "Copy tags from selected", "CopyTag");
  create_menu_item_with_mnemonic(menu, "Paste tags to selected", "PasteTag");

  return textures_menu_item;
}

gboolean TextureBrowser_tagMoveHelper(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, GSList** selected)
{
  g_assert(selected != NULL);
	
  GtkTreeRowReference* rowref = gtk_tree_row_reference_new (model, path);
  *selected = g_slist_append(*selected, rowref);

  return FALSE;
}

void TextureBrowser_assignTags()
{
  GSList* selected = NULL;
  GSList* node;
  gchar* tag_assigned;

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_available_tree));

  gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)TextureBrowser_tagMoveHelper, &selected);

  if(selected != NULL)
  {
    for (node = selected; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

      if(path)
      {
        GtkTreeIter iter;
            
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(g_TextureBrowser.m_available_store), &iter, path))
        {
          gtk_tree_model_get(GTK_TREE_MODEL(g_TextureBrowser.m_available_store), &iter, TAG_COLUMN, &tag_assigned, -1);
		  if(!TagBuilder.CheckShaderTag(g_TextureBrowser.shader.c_str()))
		  {
		    // create a custom shader/texture entry
            IShader* ishader = QERApp_Shader_ForName(g_TextureBrowser.shader.c_str());
            CopiedString filename = ishader->getShaderFileName();

            if(filename.empty())
            {
			  // it's a texture
			  TagBuilder.AddShaderNode(g_TextureBrowser.shader.c_str(), CUSTOM, TEXTURE);
            } else {
			  // it's a shader
			  TagBuilder.AddShaderNode(g_TextureBrowser.shader.c_str(), CUSTOM, SHADER);
			}
			ishader->DecRef();
		  }
		  TagBuilder.AddShaderTag(g_TextureBrowser.shader.c_str(), (char*)tag_assigned, TAG);

          gtk_list_store_remove(g_TextureBrowser.m_available_store, &iter);
          gtk_list_store_append (g_TextureBrowser.m_assigned_store, &iter);
          gtk_list_store_set (g_TextureBrowser.m_assigned_store, &iter, TAG_COLUMN, (char*)tag_assigned, -1);
        }
      }
	}

    g_slist_foreach(selected, (GFunc)gtk_tree_row_reference_free, NULL);

    // Save changes
    TagBuilder.SaveXmlDoc();
  }
  g_slist_free(selected);
}

void TextureBrowser_removeTags()
{
  GSList* selected = NULL;
  GSList* node;
  gchar* tag;

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_assigned_tree));

  gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)TextureBrowser_tagMoveHelper, &selected);

  if(selected != NULL)
  {
    for (node = selected; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

      if (path)
      {
        GtkTreeIter iter;
            
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(g_TextureBrowser.m_assigned_store), &iter, path))
        {
          gtk_tree_model_get(GTK_TREE_MODEL(g_TextureBrowser.m_assigned_store), &iter, TAG_COLUMN, &tag, -1);
          TagBuilder.DeleteShaderTag(g_TextureBrowser.shader.c_str(), tag);
          gtk_list_store_remove(g_TextureBrowser.m_assigned_store, &iter);
        }
	  }
	}

    g_slist_foreach(selected, (GFunc)gtk_tree_row_reference_free, NULL);

    // Update the "available tags list"
    BuildStoreAvailableTags(g_TextureBrowser.m_available_store, g_TextureBrowser.m_assigned_store, g_TextureBrowser.m_all_tags, &g_TextureBrowser);

    // Save changes
    TagBuilder.SaveXmlDoc();
  }
  g_slist_free(selected);
}

void TextureBrowser_buildTagList()
{
  GtkTreeIter treeIter;
  gtk_list_store_clear(g_TextureBrowser.m_all_tags_list);

  std::set<CopiedString>::iterator iter;

  for (iter = g_TextureBrowser.m_all_tags.begin(); iter != g_TextureBrowser.m_all_tags.end(); ++iter)
  {
    gtk_list_store_append(g_TextureBrowser.m_all_tags_list, &treeIter);
    gtk_list_store_set(g_TextureBrowser.m_all_tags_list, &treeIter, TAG_COLUMN, (*iter).c_str(), -1);
  }
}

void toggle_tags_textures()
{
  if(g_TextureBrowser.m_showTags)
  {
    gtk_widget_hide(GTK_WIDGET(g_TextureBrowser.m_scr_win_tags));
    gtk_widget_show(GTK_WIDGET(g_TextureBrowser.m_scr_win_tree));
  } else {
    gtk_widget_hide(GTK_WIDGET(g_TextureBrowser.m_scr_win_tree));
    gtk_widget_show(GTK_WIDGET(g_TextureBrowser.m_scr_win_tags));
  }
  g_TextureBrowser.m_showTags ^= 1;
}

void TextureBrowser_searchTags()
{
  GSList* selected = NULL;
  GSList* node;
  gchar* tag;
  char buffer[256];
  char tags_searched[256];

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags));

  gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)TextureBrowser_tagMoveHelper, &selected);

  if(selected != NULL)
  {
    strcpy(buffer, "/root/*/*[tag='");
	strcpy(tags_searched, "[TAGS] ");

    for (node = selected; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

      if (path)
      {
        GtkTreeIter iter;
            
	    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iter, path))
        {
		  gtk_tree_model_get(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iter, TAG_COLUMN, &tag, -1);

		  strcat(buffer, tag);
		  strcat(tags_searched, tag);
		  if(node != g_slist_last(node))
		  {
		    strcat(buffer, "' and tag='");
		    strcat(tags_searched, ", ");
		  }
        }
	  }
	}

	strcat(buffer, "']");

    g_slist_foreach(selected, (GFunc)gtk_tree_row_reference_free, NULL);

    g_TextureBrowser.m_found_shaders.clear(); // delete old list
	TagBuilder.TagSearch(buffer, g_TextureBrowser.m_found_shaders);

	if(!g_TextureBrowser.m_found_shaders.empty())  // found something
    {
	   size_t shaders_found = g_TextureBrowser.m_found_shaders.size();

       globalOutputStream() << "Found " << shaders_found << " textures and shaders with " << tags_searched << "\n";
	   ScopeDisableScreenUpdates disableScreenUpdates("Searching...", "Loading Textures");

	  std::set<CopiedString>::iterator iter;

      for(iter = g_TextureBrowser.m_found_shaders.begin(); iter != g_TextureBrowser.m_found_shaders.end(); iter++)
      {
	    std::string path = (*iter).c_str();
	    size_t pos = path.find_last_of("/", path.size());
	    std::string name = path.substr(pos + 1, path.size());
	    path = path.substr(0, pos + 1);
	    TextureDirectory_loadTexture(path.c_str(), name.c_str());
      }

	  g_TextureBrowser.m_searchedTags = true;
	  g_TextureBrowser_currentDirectory = tags_searched;

	  g_TextureBrowser.m_nTotalHeight = 0;
	  TextureBrowser_setOriginY(g_TextureBrowser, 0);
	  TextureBrowser_heightChanged(g_TextureBrowser);
	  TextureBrowser_updateTitle();
    }
  }
  g_slist_free(selected);
}

GtkWidget* TextureBrowser_constructTagToolbar()
{
  GtkWidget* toolbar = gtk_toolbar_new();
  GtkTooltips* toolbar_tips = gtk_tooltips_new();

  GtkWidget* image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_SMALL_TOOLBAR);
  GtkWidget* button = gtk_button_new();
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(TextureBrowser_searchTags), NULL);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(toolbar_tips), button, "Search with selected tags", "Search with selected tags");
  gtk_container_add(GTK_CONTAINER(button), image);
  gtk_container_add(GTK_CONTAINER(toolbar), button);
  gtk_widget_show_all(button);

  image = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_SMALL_TOOLBAR);
  button = gtk_toggle_button_new();
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(toggle_tags_textures), NULL);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(toolbar_tips), button, "Toggle tag/texture view", "Toggle tag/texture view");
  gtk_container_add(GTK_CONTAINER(button), image);
  gtk_container_add(GTK_CONTAINER(toolbar), button);
  gtk_widget_show_all(button);
  return toolbar;
}


void TextureBrowser_checkTagFile()
{
  const char SHADERTAG_FILE[] = "shadertags.xml";
  CopiedString default_filename, rc_filename;
  StringOutputStream stream(256);

  stream << LocalRcPath_get();
  stream << SHADERTAG_FILE;
  rc_filename = stream.c_str();

  if(file_exists(rc_filename.c_str()))
  {
    g_TextureBrowser.m_tags = TagBuilder.OpenXmlDoc(rc_filename.c_str());

    if(g_TextureBrowser.m_tags)
    {
      globalOutputStream() << "Loading tag file " << rc_filename.c_str() << ".\n";
    }
  }
  else
  {
    // load default tagfile
	stream.clear();
    stream << g_pGameDescription->mGameToolsPath.c_str();
    stream << SHADERTAG_FILE;
    default_filename = stream.c_str();

    if(file_exists(default_filename.c_str()))
    {
      g_TextureBrowser.m_tags = TagBuilder.OpenXmlDoc(default_filename.c_str(), rc_filename.c_str());
      
      if(g_TextureBrowser.m_tags)
      {
        globalOutputStream() << "Loading default tag file " << default_filename.c_str() << ".\n";
      }
    }
    else
    {
      globalErrorStream() << "Unable to find default tag file " << default_filename.c_str() << ". No tag support.\n";
    }
  }
}

GtkWidget* TextureBrowser_constructWindow(GtkWindow* toplevel)
{
  // The gl_widget and the tag assignment frame should be packed into a GtkVPaned with the slider
  // position stored in local.pref. gtk_paned_get_position() and gtk_paned_set_position() don't
  // seem to work in gtk 2.4 and the arrow buttons don't handle GTK_FILL, so here's another thing
  // for the "once-the-gtk-libs-are-updated-TODO-list" :x

  TextureBrowser_checkTagFile();

  if(g_TextureBrowser.m_tags)
  {
    g_TextureBrowser.m_all_tags_list = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
    GtkTreeSortable* sortable = GTK_TREE_SORTABLE(g_TextureBrowser.m_all_tags_list);
    gtk_tree_sortable_set_sort_column_id(sortable, TAG_COLUMN, GTK_SORT_ASCENDING);

    TagBuilder.GetAllTags(g_TextureBrowser.m_all_tags);
    TextureBrowser_buildTagList();
  }

  GlobalShaderSystem().setActiveShadersChangedNotify(ReferenceCaller<TextureBrowser, TextureBrowser_activeShadersChanged>(g_TextureBrowser));

  g_TextureBrowser.m_parent = toplevel;

  GtkWidget* table = gtk_table_new(3, 3, FALSE);
  GtkWidget* frame_table = NULL;
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_table_attach(GTK_TABLE(table), vbox, 0, 1, 1, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show(vbox);

  { // menu bar
    GtkWidget* menu_bar = gtk_menu_bar_new();
    GtkWidget* menu_view = gtk_menu_new();
    GtkWidget* view_item = (GtkWidget*)TextureBrowser_constructViewMenu(GTK_MENU(menu_view));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), menu_view);
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), view_item);

    GtkWidget* menu_tools = gtk_menu_new();
    GtkWidget* tools_item = (GtkWidget*)TextureBrowser_constructToolsMenu(GTK_MENU(menu_tools));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_item), menu_tools);
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), tools_item);

	if(g_TextureBrowser.m_tags)
	{
      GtkWidget* menu_tags = gtk_menu_new();
      GtkWidget* tags_item = (GtkWidget*)TextureBrowser_constructTagsMenu(GTK_MENU(menu_tags));
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(tags_item), menu_tags);
      gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), tags_item);
	}

	gtk_table_attach(GTK_TABLE (table), menu_bar, 0, 3, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_widget_show(menu_bar);
  }
  { // tag tool bar
    if(g_TextureBrowser.m_tags)
	{
	  GtkWidget* toolbar = TextureBrowser_constructTagToolbar();

      gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	  gtk_widget_show(toolbar);
	}
  }
  { // gl_widget scrollbar
	GtkWidget* w = gtk_vscrollbar_new(GTK_ADJUSTMENT(gtk_adjustment_new (0,0,0,1,1,1)));
	gtk_table_attach(GTK_TABLE (table), w, 2, 3, 1, 2, GTK_SHRINK, GTK_FILL, 0, 0);
	gtk_widget_show(w);
	g_TextureBrowser.m_texture_scroll = w;

    GtkAdjustment *vadjustment = gtk_range_get_adjustment (GTK_RANGE (g_TextureBrowser.m_texture_scroll));
    g_signal_connect(G_OBJECT(vadjustment), "value_changed", G_CALLBACK(TextureBrowser_verticalScroll), &g_TextureBrowser);

    widget_set_visible(g_TextureBrowser.m_texture_scroll, g_TextureBrowser.m_showTextureScrollbar);
  }
  { // TreeView
	g_TextureBrowser.m_scr_win_tree = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(g_TextureBrowser.m_scr_win_tree), 0);

	// vertical only scrolling for treeview
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(g_TextureBrowser.m_scr_win_tree), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	gtk_box_pack_end(GTK_BOX(vbox), g_TextureBrowser.m_scr_win_tree, TRUE, TRUE, 0);
	gtk_widget_show(g_TextureBrowser.m_scr_win_tree);

	TextureBrowser_createTreeViewTree();

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(g_TextureBrowser.m_scr_win_tree), GTK_WIDGET(g_TextureBrowser.m_treeViewTree));
	gtk_widget_show(GTK_WIDGET(g_TextureBrowser.m_treeViewTree));
  }
  { // TreeView for tags
    if(g_TextureBrowser.m_tags)
	{
	  g_TextureBrowser.m_scr_win_tags = gtk_scrolled_window_new(NULL, NULL);
	  gtk_container_set_border_width(GTK_CONTAINER(g_TextureBrowser.m_scr_win_tags), 0);

	  // vertical only scrolling for treeview
	  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(g_TextureBrowser.m_scr_win_tags), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	  gtk_box_pack_end(GTK_BOX(vbox), g_TextureBrowser.m_scr_win_tags, TRUE, TRUE, 0);

	  TextureBrowser_createTreeViewTags();

      GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags));
	  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (g_TextureBrowser.m_scr_win_tags), GTK_WIDGET (g_TextureBrowser.m_treeViewTags));
	  gtk_widget_show(GTK_WIDGET(g_TextureBrowser.m_treeViewTags));
	}
  }
  { // gl_widget
    g_TextureBrowser.m_gl_widget = glwidget_new(FALSE);
    gtk_widget_ref(g_TextureBrowser.m_gl_widget);

    gtk_widget_set_events(g_TextureBrowser.m_gl_widget, GDK_DESTROY | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
    GTK_WIDGET_SET_FLAGS(g_TextureBrowser.m_gl_widget, GTK_CAN_FOCUS);

	gtk_table_attach_defaults(GTK_TABLE(table), g_TextureBrowser.m_gl_widget, 1, 2, 1, 2);
    gtk_widget_show(g_TextureBrowser.m_gl_widget);

    g_TextureBrowser.m_sizeHandler = g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "size_allocate", G_CALLBACK(TextureBrowser_size_allocate), &g_TextureBrowser);
    g_TextureBrowser.m_exposeHandler = g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "expose_event", G_CALLBACK(TextureBrowser_expose), &g_TextureBrowser);

    g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "button_press_event", G_CALLBACK(TextureBrowser_button_press), &g_TextureBrowser);
    g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "button_release_event", G_CALLBACK(TextureBrowser_button_release), &g_TextureBrowser);
    g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "motion_notify_event", G_CALLBACK(TextureBrowser_motion), &g_TextureBrowser);
    g_signal_connect(G_OBJECT(g_TextureBrowser.m_gl_widget), "scroll_event", G_CALLBACK(TextureBrowser_scroll), &g_TextureBrowser);
  }
  { // tag frame
    if(g_TextureBrowser.m_tags)
	{
      frame_table = gtk_table_new(3, 3, FALSE);

	  g_TextureBrowser.m_tag_frame = gtk_frame_new("Tag assignment");
	  gtk_frame_set_label_align(GTK_FRAME(g_TextureBrowser.m_tag_frame), 0.5, 0.5);
	  gtk_frame_set_shadow_type(GTK_FRAME(g_TextureBrowser.m_tag_frame), GTK_SHADOW_NONE);

	  gtk_table_attach(GTK_TABLE(table), g_TextureBrowser.m_tag_frame, 1, 3, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);

	  // set the size of the tag frame
	  gtk_widget_show(frame_table);

	  gtk_container_add (GTK_CONTAINER(g_TextureBrowser.m_tag_frame), frame_table);
	}
  }
  {  // assigned tag list
    if(g_TextureBrowser.m_tags)
	{
	  GtkWidget* scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	  gtk_container_set_border_width(GTK_CONTAINER (scrolled_win), 0);
	  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_win), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	  g_TextureBrowser.m_assigned_store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);

	  GtkTreeSortable* sortable = GTK_TREE_SORTABLE(g_TextureBrowser.m_assigned_store);
	  gtk_tree_sortable_set_sort_column_id(sortable, TAG_COLUMN, GTK_SORT_ASCENDING);

	  GtkCellRenderer* renderer = gtk_cell_renderer_text_new();

	  g_TextureBrowser.m_assigned_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (g_TextureBrowser.m_assigned_store));
	  g_object_unref(G_OBJECT (g_TextureBrowser.m_assigned_store));
	  g_signal_connect(g_TextureBrowser.m_assigned_tree, "row-activated", (GCallback) TextureBrowser_removeTags, NULL);
	  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (g_TextureBrowser.m_assigned_tree), FALSE);

	  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_assigned_tree));
	  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	  GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", TAG_COLUMN, NULL);
	  gtk_tree_view_append_column(GTK_TREE_VIEW (g_TextureBrowser.m_assigned_tree), column);
	  gtk_widget_show(g_TextureBrowser.m_assigned_tree);

	  gtk_widget_show(scrolled_win);
	  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (scrolled_win), GTK_WIDGET (g_TextureBrowser.m_assigned_tree));

	  gtk_table_attach(GTK_TABLE(frame_table), scrolled_win, 0, 1, 1, 3, GTK_FILL, GTK_FILL, 0, 0);
	}
  }
  {  // available tag list
    if(g_TextureBrowser.m_tags)
	{
	  GtkWidget* scrolled_win = gtk_scrolled_window_new (NULL, NULL);
	  gtk_container_set_border_width (GTK_CONTAINER (scrolled_win), 0);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	  g_TextureBrowser.m_available_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
	  GtkTreeSortable* sortable = GTK_TREE_SORTABLE(g_TextureBrowser.m_available_store);
	  gtk_tree_sortable_set_sort_column_id(sortable, TAG_COLUMN, GTK_SORT_ASCENDING);

	  GtkCellRenderer* renderer = gtk_cell_renderer_text_new ();

	  g_TextureBrowser.m_available_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (g_TextureBrowser.m_available_store));
	  g_object_unref (G_OBJECT (g_TextureBrowser.m_available_store));
	  g_signal_connect(g_TextureBrowser.m_available_tree, "row-activated", (GCallback) TextureBrowser_assignTags, NULL);
	  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (g_TextureBrowser.m_available_tree), FALSE);

	  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_available_tree));
	  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	  GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes ("", renderer, "text", TAG_COLUMN, NULL);
	  gtk_tree_view_append_column (GTK_TREE_VIEW (g_TextureBrowser.m_available_tree), column);
	  gtk_widget_show (g_TextureBrowser.m_available_tree);

	  gtk_widget_show (scrolled_win);
	  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_win), GTK_WIDGET (g_TextureBrowser.m_available_tree));

	  gtk_table_attach (GTK_TABLE (frame_table), scrolled_win, 2, 3, 1, 3, GTK_FILL, GTK_FILL, 0, 0);
	}
  }
  { // arrow buttons
    if(g_TextureBrowser.m_tags)
	{
	  GtkWidget* m_btn_left = gtk_button_new();
	  GtkWidget* m_btn_right = gtk_button_new();
	  GtkWidget* m_arrow_left = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
	  GtkWidget* m_arrow_right = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
	  gtk_container_add(GTK_CONTAINER(m_btn_left), m_arrow_left);
	  gtk_container_add(GTK_CONTAINER(m_btn_right), m_arrow_right);

	  // workaround. the size of the tag frame depends of the requested size of the arrow buttons.
	  gtk_widget_set_size_request(m_arrow_left, -1, 68);
	  gtk_widget_set_size_request(m_arrow_right, -1, 68);

	  gtk_table_attach(GTK_TABLE(frame_table), m_btn_left, 1, 2, 1, 2, GTK_SHRINK, GTK_EXPAND, 0, 0);
	  gtk_table_attach(GTK_TABLE(frame_table), m_btn_right, 1, 2, 2, 3, GTK_SHRINK, GTK_EXPAND, 0, 0);

	  g_signal_connect(G_OBJECT (m_btn_left), "clicked", G_CALLBACK(TextureBrowser_assignTags), NULL);
	  g_signal_connect(G_OBJECT (m_btn_right), "clicked", G_CALLBACK(TextureBrowser_removeTags), NULL);

	  gtk_widget_show(m_btn_left);
	  gtk_widget_show(m_btn_right);
	  gtk_widget_show(m_arrow_left);
	  gtk_widget_show(m_arrow_right);
	}
  }
  { // tag frame labels
    if(g_TextureBrowser.m_tags)
	{
	  GtkWidget* m_lbl_assigned = gtk_label_new ("Assigned");
	  GtkWidget* m_lbl_unassigned = gtk_label_new ("Available");

	  gtk_table_attach (GTK_TABLE (frame_table), m_lbl_assigned, 0, 1, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
	  gtk_table_attach (GTK_TABLE (frame_table), m_lbl_unassigned, 2, 3, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0);

	  gtk_widget_show (m_lbl_assigned);
	  gtk_widget_show (m_lbl_unassigned);
	}
  }

  // TODO do we need this?
  //gtk_container_set_focus_chain(GTK_CONTAINER(hbox_table), NULL);

  return table;
}

void TextureBrowser_destroyWindow()
{
  GlobalShaderSystem().setActiveShadersChangedNotify(Callback());

  g_signal_handler_disconnect(G_OBJECT(g_TextureBrowser.m_gl_widget), g_TextureBrowser.m_sizeHandler);
  g_signal_handler_disconnect(G_OBJECT(g_TextureBrowser.m_gl_widget), g_TextureBrowser.m_exposeHandler);

  gtk_widget_unref(g_TextureBrowser.m_gl_widget);
}

const Vector3& TextureBrowser_getBackgroundColour(TextureBrowser& textureBrowser)
{
  return textureBrowser.color_textureback;
}

void TextureBrowser_setBackgroundColour(TextureBrowser& textureBrowser, const Vector3& colour)
{
  textureBrowser.color_textureback = colour;
  TextureBrowser_queueDraw(textureBrowser);
}

void TextureBrowser_selectionHelper(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, GSList** selected)
{
  g_assert(selected != NULL);

  gchar* name;
  gtk_tree_model_get(model, iter, TAG_COLUMN, &name, -1);
  *selected = g_slist_append(*selected, name);
}

void TextureBrowser_shaderInfo()
{
  const char* name = TextureBrowser_GetSelectedShader(g_TextureBrowser);
  IShader* shader = QERApp_Shader_ForName(name);

  DoShaderInfoDlg(name, shader->getShaderFileName(), "Shader Info");

  shader->DecRef();
}

void TextureBrowser_addTag()
{
  CopiedString tag;

  EMessageBoxReturn result = DoShaderTagDlg(&tag, "Add shader tag");

  if (result == eIDOK && !tag.empty())
  {
    GtkTreeIter iter, iter2;
    g_TextureBrowser.m_all_tags.insert(tag.c_str());
    gtk_list_store_append(g_TextureBrowser.m_available_store, &iter);
    gtk_list_store_set(g_TextureBrowser.m_available_store, &iter, TAG_COLUMN, tag.c_str(), -1);

    // Select the currently added tag in the available list
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_available_tree));
    gtk_tree_selection_select_iter(selection, &iter);

    gtk_list_store_append(g_TextureBrowser.m_all_tags_list, &iter2);
    gtk_list_store_set(g_TextureBrowser.m_all_tags_list, &iter2, TAG_COLUMN, tag.c_str(), -1);
  }
}

void TextureBrowser_renameTag()
{
  /* WORKAROUND: The tag treeview is set to GTK_SELECTION_MULTIPLE. Because
     gtk_tree_selection_get_selected() doesn't work with GTK_SELECTION_MULTIPLE,
     we need to count the number of selected rows first and use
     gtk_tree_selection_selected_foreach() then to go through the list of selected
     rows (which always containins a single row).
  */

  GSList* selected = NULL;

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags));
  gtk_tree_selection_selected_foreach(selection, GtkTreeSelectionForeachFunc(TextureBrowser_selectionHelper), &selected);
 
  if(g_slist_length(selected) == 1) // we only rename a single tag
  {
    CopiedString newTag;
    EMessageBoxReturn result = DoShaderTagDlg(&newTag, "Rename shader tag");

    if (result == eIDOK && !newTag.empty())
    {
      GtkTreeIter iterList;
      gchar* rowTag;
      gchar* oldTag = (char*)selected->data;

      bool row = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterList) != 0;

      while(row)
      {
        gtk_tree_model_get(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterList, TAG_COLUMN, &rowTag, -1);

        if(strcmp(rowTag, oldTag) == 0)
        {
          gtk_list_store_set(g_TextureBrowser.m_all_tags_list, &iterList, TAG_COLUMN, newTag.c_str(), -1);
        }
        row = gtk_tree_model_iter_next(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterList) != 0;
      }

      TagBuilder.RenameShaderTag(oldTag, newTag.c_str());

      g_TextureBrowser.m_all_tags.erase((CopiedString)oldTag);
      g_TextureBrowser.m_all_tags.insert(newTag);

      BuildStoreAssignedTags(g_TextureBrowser.m_assigned_store, g_TextureBrowser.shader.c_str(), &g_TextureBrowser);
      BuildStoreAvailableTags(g_TextureBrowser.m_available_store, g_TextureBrowser.m_assigned_store, g_TextureBrowser.m_all_tags, &g_TextureBrowser);
    }
  }
  else
  {
    gtk_MessageBox(GTK_WIDGET(g_TextureBrowser.m_parent), "Select a single tag for renaming.");
  }
}

void TextureBrowser_deleteTag()
{
  GSList* selected = NULL;

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_TextureBrowser.m_treeViewTags));
  gtk_tree_selection_selected_foreach(selection, GtkTreeSelectionForeachFunc(TextureBrowser_selectionHelper), &selected);
 
  if(g_slist_length(selected) == 1) // we only delete a single tag
  {
    EMessageBoxReturn result = gtk_MessageBox(GTK_WIDGET(g_TextureBrowser.m_parent), "Are you sure you want to delete the selected tag?", "Delete Tag", eMB_YESNO, eMB_ICONQUESTION);

    if(result == eIDYES)
    {
      GtkTreeIter iterSelected;
      gchar *rowTag;

      gchar* tagSelected = (char*)selected->data;

      bool row = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterSelected) != 0;

      while(row)
      {
        gtk_tree_model_get(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterSelected, TAG_COLUMN, &rowTag, -1);

        if(strcmp(rowTag, tagSelected) == 0)
        {
          gtk_list_store_remove(g_TextureBrowser.m_all_tags_list, &iterSelected);
          break;
        }
        row = gtk_tree_model_iter_next(GTK_TREE_MODEL(g_TextureBrowser.m_all_tags_list), &iterSelected) != 0;
      }
      
      TagBuilder.DeleteTag(tagSelected);
      g_TextureBrowser.m_all_tags.erase((CopiedString)tagSelected);

	    BuildStoreAssignedTags(g_TextureBrowser.m_assigned_store, g_TextureBrowser.shader.c_str(), &g_TextureBrowser);
      BuildStoreAvailableTags(g_TextureBrowser.m_available_store, g_TextureBrowser.m_assigned_store, g_TextureBrowser.m_all_tags, &g_TextureBrowser);
    }
  } else {
    gtk_MessageBox(GTK_WIDGET(g_TextureBrowser.m_parent), "Select a single tag for deletion.");
  }
}

void TextureBrowser_copyTag()
{
  g_TextureBrowser.m_copied_tags.clear();
  TagBuilder.GetShaderTags(g_TextureBrowser.shader.c_str(), g_TextureBrowser.m_copied_tags);
}

void TextureBrowser_pasteTag()
{
  IShader* ishader = QERApp_Shader_ForName(g_TextureBrowser.shader.c_str());
  CopiedString shader = g_TextureBrowser.shader.c_str();

  if(!TagBuilder.CheckShaderTag(shader.c_str()))
  {
    CopiedString shaderFile = ishader->getShaderFileName();
    if(shaderFile.empty())
    {
      // it's a texture
      TagBuilder.AddShaderNode(shader.c_str(), CUSTOM, TEXTURE);
    }
    else
    {
      // it's a shader
      TagBuilder.AddShaderNode(shader.c_str(), CUSTOM, SHADER);
    }

    for(size_t i = 0; i < g_TextureBrowser.m_copied_tags.size(); ++i)
    {
      TagBuilder.AddShaderTag(shader.c_str(), g_TextureBrowser.m_copied_tags[i].c_str(), TAG);
    }
  }
  else
  {
    for(size_t i = 0; i < g_TextureBrowser.m_copied_tags.size(); ++i)
    {
      if(!TagBuilder.CheckShaderTag(shader.c_str(), g_TextureBrowser.m_copied_tags[i].c_str()))
      {
        // the tag doesn't exist - let's add it
        TagBuilder.AddShaderTag(shader.c_str(), g_TextureBrowser.m_copied_tags[i].c_str(), TAG);
      }
    }
  }

  ishader->DecRef();

  TagBuilder.SaveXmlDoc();
  BuildStoreAssignedTags(g_TextureBrowser.m_assigned_store, shader.c_str(), &g_TextureBrowser);
  BuildStoreAvailableTags (g_TextureBrowser.m_available_store, g_TextureBrowser.m_assigned_store, g_TextureBrowser.m_all_tags, &g_TextureBrowser);
}

void RefreshShaders()
{
  ScopeDisableScreenUpdates disableScreenUpdates("Processing...", "Loading Shaders");
  GlobalShaderSystem().refresh();
  UpdateAllWindows();
}

void TextureBrowser_ToggleShowShaders() 
{
  g_TextureBrowser.m_showShaders ^= 1;
  g_TextureBrowser.m_showshaders_item.update();
  TextureBrowser_queueDraw(g_TextureBrowser);
}

void TextureBrowser_ToggleShowShaderListOnly() 
{
  g_TextureBrowser_shaderlistOnly ^= 1;
  g_TextureBrowser.m_showshaderlistonly_item.update();

  TextureBrowser_constructTreeStore();
}

void TextureBrowser_showAll()
{
  g_TextureBrowser_currentDirectory = "";
  g_TextureBrowser.m_searchedTags = false;
  TextureBrowser_heightChanged(g_TextureBrowser);
  TextureBrowser_updateTitle();
}

void TextureBrowser_showUntagged()
{
  EMessageBoxReturn result = gtk_MessageBox(GTK_WIDGET(g_TextureBrowser.m_parent), "WARNING! This function might need a lot of memory and time. Are you sure you want to use it?", "Show Untagged", eMB_YESNO, eMB_ICONWARNING);

  if(result == eIDYES)
  {
    g_TextureBrowser.m_found_shaders.clear();
    TagBuilder.GetUntagged(g_TextureBrowser.m_found_shaders);
    std::set<CopiedString>::iterator iter;

    ScopeDisableScreenUpdates disableScreenUpdates("Searching untagged textures...", "Loading Textures");

    for(iter = g_TextureBrowser.m_found_shaders.begin(); iter != g_TextureBrowser.m_found_shaders.end(); iter++)
    {
      std::string path = (*iter).c_str();
      size_t pos = path.find_last_of("/", path.size());
      std::string name = path.substr(pos + 1, path.size());
      path = path.substr(0, pos + 1);
	  TextureDirectory_loadTexture(path.c_str(), name.c_str());
	  globalErrorStream() << path.c_str() << name.c_str() << "\n";
    }

    g_TextureBrowser_currentDirectory = "Untagged";
	TextureBrowser_queueDraw(GlobalTextureBrowser());
    TextureBrowser_heightChanged(g_TextureBrowser);
    TextureBrowser_updateTitle();
  }
}

void TextureBrowser_exportTitle(const StringImportCallback& importer)
{
  StringOutputStream buffer(64);
  buffer << "Textures: ";
  if(!string_empty(g_TextureBrowser_currentDirectory.c_str()))
  {
    buffer << g_TextureBrowser_currentDirectory.c_str();
  }
  else
  {
    buffer << "all";
  }
  importer(buffer.c_str());
}


void TextureScaleImport(TextureBrowser& textureBrowser, int value)
{
  switch(value)
  {
  case 0:
    TextureBrowser_setScale(textureBrowser, 10);
    break;
  case 1:
    TextureBrowser_setScale(textureBrowser, 25);
    break;
  case 2:
    TextureBrowser_setScale(textureBrowser, 50);
    break;
  case 3:
    TextureBrowser_setScale(textureBrowser, 100);
    break;
  case 4:
    TextureBrowser_setScale(textureBrowser, 200);
    break;
  }
}
typedef ReferenceCaller1<TextureBrowser, int, TextureScaleImport> TextureScaleImportCaller;

void TextureScaleExport(TextureBrowser& textureBrowser, const IntImportCallback& importer)
{
  switch(textureBrowser.m_textureScale)
  {
  case 10:
    importer(0);
    break;
  case 25:
    importer(1);
    break;
  case 50:
    importer(2);
    break;
  case 100:
    importer(3);
    break;
  case 200:
    importer(4);
    break;
  }
}
typedef ReferenceCaller1<TextureBrowser, const IntImportCallback&, TextureScaleExport> TextureScaleExportCaller;

void TextureBrowser_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox(
    "", "Texture scrollbar",
    TextureBrowserImportShowScrollbarCaller(GlobalTextureBrowser()),
    BoolExportCaller(GlobalTextureBrowser().m_showTextureScrollbar)
  );
  {
    const char* texture_scale[] = { "10%", "25%", "50%", "100%", "200%" };
    page.appendCombo(
      "Texture Thumbnail Scale",
      STRING_ARRAY_RANGE(texture_scale),
      IntImportCallback(TextureScaleImportCaller(GlobalTextureBrowser())),
      IntExportCallback(TextureScaleExportCaller(GlobalTextureBrowser()))
    );
  }
  page.appendEntry("Mousewheel Increment", GlobalTextureBrowser().m_mouseWheelScrollIncrement);
  {
    const char* startup_shaders[] = { "None", TextureBrowser_getComonShadersName() };
    page.appendCombo("Load Shaders at Startup", reinterpret_cast<int&>(GlobalTextureBrowser().m_startupShaders), STRING_ARRAY_RANGE(startup_shaders));
  }
}
void TextureBrowser_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Texture Browser", "Texture Browser Preferences"));
  TextureBrowser_constructPreferences(page);
}
void TextureBrowser_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, TextureBrowser_constructPage>());
}


#include "preferencesystem.h"
#include "stringio.h"

typedef ReferenceCaller1<TextureBrowser, std::size_t, TextureBrowser_setScale> TextureBrowserSetScaleCaller;



void TextureClipboard_textureSelected(const char* shader);

void TextureBrowser_Construct()
{
  GlobalCommands_insert("ShaderInfo", FreeCaller<TextureBrowser_shaderInfo>());
  GlobalCommands_insert("ShowUntagged", FreeCaller<TextureBrowser_showUntagged>());
  GlobalCommands_insert("AddTag", FreeCaller<TextureBrowser_addTag>());
  GlobalCommands_insert("RenameTag", FreeCaller<TextureBrowser_renameTag>());
  GlobalCommands_insert("DeleteTag", FreeCaller<TextureBrowser_deleteTag>());
  GlobalCommands_insert("CopyTag", FreeCaller<TextureBrowser_copyTag>());
  GlobalCommands_insert("PasteTag", FreeCaller<TextureBrowser_pasteTag>());
  GlobalCommands_insert("RefreshShaders", FreeCaller<RefreshShaders>());
  GlobalToggles_insert("ShowInUse", FreeCaller<TextureBrowser_ToggleHideUnused>(), ToggleItem::AddCallbackCaller(g_TextureBrowser.m_hideunused_item), Accelerator('U'));
  GlobalCommands_insert("ShowAllTextures", FreeCaller<TextureBrowser_showAll>(), Accelerator('A', (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("ToggleTextures", FreeCaller<TextureBrowser_toggleShow>(), Accelerator('T'));
  GlobalToggles_insert("ToggleShowShaders", FreeCaller<TextureBrowser_ToggleShowShaders>(), ToggleItem::AddCallbackCaller(g_TextureBrowser.m_showshaders_item));
  GlobalToggles_insert("ToggleShowShaderlistOnly", FreeCaller<TextureBrowser_ToggleShowShaderListOnly>(), ToggleItem::AddCallbackCaller(g_TextureBrowser.m_showshaderlistonly_item));

  GlobalPreferenceSystem().registerPreference("TextureScale",
    makeSizeStringImportCallback(TextureBrowserSetScaleCaller(g_TextureBrowser)),
    SizeExportStringCaller(g_TextureBrowser.m_textureScale)
  );
  GlobalPreferenceSystem().registerPreference("TextureScrollbar",
    makeBoolStringImportCallback(TextureBrowserImportShowScrollbarCaller(g_TextureBrowser)),
    BoolExportStringCaller(GlobalTextureBrowser().m_showTextureScrollbar)
  );
  GlobalPreferenceSystem().registerPreference("ShowShaders", BoolImportStringCaller(GlobalTextureBrowser().m_showShaders), BoolExportStringCaller(GlobalTextureBrowser().m_showShaders));
  GlobalPreferenceSystem().registerPreference("ShowShaderlistOnly", BoolImportStringCaller(g_TextureBrowser_shaderlistOnly), BoolExportStringCaller(g_TextureBrowser_shaderlistOnly));
  GlobalPreferenceSystem().registerPreference("LoadShaders", IntImportStringCaller(reinterpret_cast<int&>(GlobalTextureBrowser().m_startupShaders)), IntExportStringCaller(reinterpret_cast<int&>(GlobalTextureBrowser().m_startupShaders)));
  GlobalPreferenceSystem().registerPreference("WheelMouseInc", SizeImportStringCaller(GlobalTextureBrowser().m_mouseWheelScrollIncrement), SizeExportStringCaller(GlobalTextureBrowser().m_mouseWheelScrollIncrement));
  GlobalPreferenceSystem().registerPreference("SI_Colors0", Vector3ImportStringCaller(GlobalTextureBrowser().color_textureback), Vector3ExportStringCaller(GlobalTextureBrowser().color_textureback));

  g_TextureBrowser.shader = texdef_name_default();

  Textures_setModeChangedNotify(ReferenceCaller<TextureBrowser, TextureBrowser_queueDraw>(g_TextureBrowser));

  TextureBrowser_registerPreferencesPage();

  GlobalShaderSystem().attach(g_ShadersObserver);

  TextureBrowser_textureSelected = TextureClipboard_textureSelected;
}
void TextureBrowser_Destroy()
{
  GlobalShaderSystem().detach(g_ShadersObserver);

  Textures_setModeChangedNotify(Callback());
}
