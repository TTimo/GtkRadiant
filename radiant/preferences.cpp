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
// User preferences
//
// Leonardo Zide (leo@lokigames.com)
//

#include "preferences.h"

#include "debugging/debugging.h"

#include <gtk/gtkmain.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtknotebook.h>

#include "generic/callback.h"
#include "math/vector.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "os/file.h"
#include "os/path.h"
#include "os/dir.h"
#include "gtkutil/filechooser.h"
#include "gtkutil/messagebox.h"
#include "cmdlib.h"

#include "error.h"
#include "console.h"
#include "xywindow.h"
#include "mainframe.h"
#include "qe3.h"
#include "gtkdlgs.h"



void Global_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox("Console", "Enable Logging", g_Console_enableLogging);
}

void Interface_constructPreferences(PreferencesPage& page)
{
#ifdef WIN32
  page.appendCheckBox("", "Native File-Chooser", g_FileChooser_nativeGUI);
  page.appendCheckBox("", "Default Text Editor", g_TextEditor_useWin32Editor);
#else
  {
    GtkWidget* use_custom = page.appendCheckBox("Text Editor", "Custom", g_TextEditor_useCustomEditor);
    GtkWidget* custom_editor = page.appendPathEntry("Text Editor Command", g_TextEditor_editorCommand, true);
    Widget_connectToggleDependency(custom_editor, use_custom);
  }
#endif
}

void Mouse_constructPreferences(PreferencesPage& page)
{
  {
    const char* buttons[] = { "2 button", "3 button", };
    page.appendRadio("Mouse Type",  g_glwindow_globals.m_nMouseType, STRING_ARRAY_RANGE(buttons));
  }
  page.appendCheckBox("Right Button", "Activates Context Menu", g_xywindow_globals.m_bRightClick);
}
void Mouse_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Mouse", "Mouse Preferences"));
  Mouse_constructPreferences(page);
}
void Mouse_registerPreferencesPage()
{
  PreferencesDialog_addInterfacePage(FreeCaller1<PreferenceGroup&, Mouse_constructPage>());
}


/*!
=========================================================
Games selection dialog
=========================================================
*/

#include <map>

inline const char* xmlAttr_getName(xmlAttrPtr attr)
{
  return reinterpret_cast<const char*>(attr->name);
}

inline const char* xmlAttr_getValue(xmlAttrPtr attr)
{
  return reinterpret_cast<const char*>(attr->children->content);
}

CGameDescription::CGameDescription(xmlDocPtr pDoc, const CopiedString& gameFile)
{
  // read the user-friendly game name 
  xmlNodePtr pNode = pDoc->children;

  while (strcmp((const char*)pNode->name, "game") && pNode != 0)
  {
    pNode=pNode->next;
  }
  if (!pNode)
  {
    Error("Didn't find 'game' node in the game description file '%s'\n", pDoc->URL);
  }

  for(xmlAttrPtr attr = pNode->properties; attr != 0; attr = attr->next)
  {
    m_gameDescription.insert(GameDescription::value_type(xmlAttr_getName(attr), xmlAttr_getValue(attr)));
  }

  {
    StringOutputStream path(256);
    path << AppPath_get() << gameFile.c_str() << "/";
    mGameToolsPath = path.c_str();
  }

  ASSERT_MESSAGE(file_exists(mGameToolsPath.c_str()), "game directory not found: " << makeQuoted(mGameToolsPath.c_str()));

  mGameFile = gameFile;
 
  {
    GameDescription::iterator i = m_gameDescription.find("type");
    if(i == m_gameDescription.end())
    {
      globalErrorStream() << "Warning, 'type' attribute not found in '" << reinterpret_cast<const char*>(pDoc->URL) << "'\n";
      // default
      mGameType = "q3";
    }
    else
    {
      mGameType = (*i).second.c_str();
    }
  }
}

void CGameDescription::Dump()
{
  globalOutputStream() << "game description file: " << makeQuoted(mGameFile.c_str()) << "\n";
  for(GameDescription::iterator i = m_gameDescription.begin(); i != m_gameDescription.end(); ++i)
  {
    globalOutputStream() << (*i).first.c_str() << " = " << makeQuoted((*i).second.c_str()) << "\n";
  }
}

CGameDescription *g_pGameDescription; ///< shortcut to g_GamesDialog.m_pCurrentDescription


#include "warnings.h"
#include "stream/textfilestream.h"
#include "container/array.h"
#include "xml/ixml.h"
#include "xml/xmlparser.h"
#include "xml/xmlwriter.h"

#include "preferencedictionary.h"
#include "stringio.h"

const char* const PREFERENCES_VERSION = "1.0";

bool Preferences_Load(PreferenceDictionary& preferences, const char* filename)
{
  TextFileInputStream file(filename);
  if(!file.failed())
  {
    XMLStreamParser parser(file);
    XMLPreferenceDictionaryImporter importer(preferences, PREFERENCES_VERSION);
    parser.exportXML(importer);
    return true;
  }
  return false;
}

bool Preferences_Save(PreferenceDictionary& preferences, const char* filename)
{
  TextFileOutputStream file(filename);
  if(!file.failed())
  {
    XMLStreamWriter writer(file);
    XMLPreferenceDictionaryExporter exporter(preferences, PREFERENCES_VERSION);
    exporter.exportXML(writer);
    return true;
  }
  return false;
}

bool Preferences_Save_Safe(PreferenceDictionary& preferences, const char* filename)
{
  Array<char> tmpName(filename, filename + strlen(filename) + 1 + 3);
  *(tmpName.end() - 4) = 'T';
  *(tmpName.end() - 3) = 'M';
  *(tmpName.end() - 2) = 'P';
  *(tmpName.end() - 1) = '\0';

  return Preferences_Save(preferences, tmpName.data())
    && (!file_exists(filename) || file_remove(filename))
    && file_move(tmpName.data(), filename);
}



void LogConsole_importString(const char* string)
{
  g_Console_enableLogging = string_equal(string, "true");
  Sys_LogFile(g_Console_enableLogging);
}
typedef FreeCaller1<const char*, LogConsole_importString> LogConsoleImportStringCaller;


void RegisterGlobalPreferences(PreferenceSystem& preferences)
{
  preferences.registerPreference("gamefile", CopiedStringImportStringCaller(g_GamesDialog.m_sGameFile), CopiedStringExportStringCaller(g_GamesDialog.m_sGameFile));
  preferences.registerPreference("gamePrompt", BoolImportStringCaller(g_GamesDialog.m_bGamePrompt), BoolExportStringCaller(g_GamesDialog.m_bGamePrompt));
  preferences.registerPreference("log console", LogConsoleImportStringCaller(), BoolExportStringCaller(g_Console_enableLogging));
}


PreferenceDictionary g_global_preferences;

void GlobalPreferences_Init()
{
  RegisterGlobalPreferences(g_global_preferences);
}

void CGameDialog::LoadPrefs()
{
  // load global .pref file
  StringOutputStream strGlobalPref(256);
  strGlobalPref << g_Preferences.m_global_rc_path->str << "global.pref";

  globalOutputStream() << "loading global preferences from " << makeQuoted(strGlobalPref.c_str()) << "\n";

  if(!Preferences_Load(g_global_preferences, strGlobalPref.c_str()))
  {
    globalOutputStream() << "failed to load global preferences from " << strGlobalPref.c_str() << "\n";
  }
}

void CGameDialog::SavePrefs()
{
  StringOutputStream strGlobalPref(256);
  strGlobalPref << g_Preferences.m_global_rc_path->str << "global.pref";

  globalOutputStream() << "saving global preferences to " << strGlobalPref.c_str() << "\n";

  if(!Preferences_Save_Safe(g_global_preferences, strGlobalPref.c_str()))
  {
    globalOutputStream() << "failed to save global preferences to " << strGlobalPref.c_str() << "\n";
  }
}

void CGameDialog::DoGameDialog()
{
  // show the UI
  DoModal();

  // we save the prefs file
  SavePrefs();
}

void CGameDialog::GameFileImport(int value)
{
  m_nComboSelect = value;
  // use value to set m_sGameFile
  std::list<CGameDescription *>::iterator iGame = mGames.begin();
  int i;
  for(i=0; i<value; i++)
  {
    ++iGame;
  }
  m_sGameFile = (*iGame)->mGameFile;
}

void CGameDialog::GameFileExport(const IntImportCallback& importCallback) const
{
  // use m_sGameFile to set value
  std::list<CGameDescription *>::const_iterator iGame;
  int i = 0;
  for(iGame=mGames.begin(); iGame!=mGames.end(); ++iGame)
  {
    if ((*iGame)->mGameFile == m_sGameFile)
    {
      m_nComboSelect = i;
      break;
    }
    i++;
  }
  importCallback(m_nComboSelect);
}

void CGameDialog_GameFileImport(CGameDialog& self, int value)
{
  self.GameFileImport(value);
}

void CGameDialog_GameFileExport(CGameDialog& self, const IntImportCallback& importCallback)
{
  self.GameFileExport(importCallback);
}

void CGameDialog::CreateGlobalFrame(PreferencesPage& page)
{
  std::vector<const char*> games;
  games.reserve(mGames.size());
  for(std::list<CGameDescription *>::iterator i = mGames.begin(); i != mGames.end(); ++i)
  {
    games.push_back((*i)->getRequiredKeyValue("name"));
  }
  page.appendCombo(
    "Select the game",
    StringArrayRange(&(*games.begin()), &(*games.end())),
    ReferenceCaller1<CGameDialog, int, CGameDialog_GameFileImport>(*this),
    ReferenceCaller1<CGameDialog, const IntImportCallback&, CGameDialog_GameFileExport>(*this)
  );
  page.appendCheckBox("Startup", "Show Global Preferences", m_bGamePrompt);
}

GtkWindow* CGameDialog::BuildDialog()
{
  GtkFrame* frame = create_dialog_frame("Game settings", GTK_SHADOW_ETCHED_IN);

  GtkVBox* vbox2 = create_dialog_vbox(0, 4);
  gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox2));

  {
    PreferencesPage preferencesPage(*this, GTK_WIDGET(vbox2));
    Global_constructPreferences(preferencesPage);
    CreateGlobalFrame(preferencesPage);
  }

  return create_simple_modal_dialog_window("Global Preferences", m_modal, GTK_WIDGET(frame));
}

class LoadGameFile
{
  std::list<CGameDescription*>& mGames;
  const char* mPath;
public:
  LoadGameFile(std::list<CGameDescription*>& games, const char* path) : mGames(games), mPath(path)
  {
  }
  void operator()(const char* name) const
  {
    if(!extension_equal(path_get_extension(name), "game"))
    {
      return;
    }
    StringOutputStream strPath(256);
    strPath << mPath << name;
    globalOutputStream() << strPath.c_str() << '\n';

    xmlDocPtr pDoc = xmlParseFile(strPath.c_str());
    if(pDoc)
    {
      mGames.push_front(new CGameDescription(pDoc, name));
      xmlFreeDoc(pDoc);
    }
    else
    {
      globalErrorStream() << "XML parser failed on '" << strPath.c_str() << "'\n";
    }
  }
};

void CGameDialog::ScanForGames()
{
  StringOutputStream strGamesPath(256);
  strGamesPath << AppPath_get() << "games/";
  const char *path = strGamesPath.c_str();

  globalOutputStream() << "Scanning for game description files: " << path << '\n';

  /*!
  \todo FIXME LINUX:
  do we put game description files below AppPath, or in ~/.radiant
  i.e. read only or read/write?
  my guess .. readonly cause it's an install
  we will probably want to add ~/.radiant/<version>/games/ scanning on top of that for developers
  (if that's really needed)
  */

  Directory_forEach(path, LoadGameFile(mGames, path));
}

CGameDescription* CGameDialog::GameDescriptionForComboItem()
{
  std::list<CGameDescription *>::iterator iGame;
  int i=0;
  for(iGame=mGames.begin(); iGame!=mGames.end(); ++iGame,i++)
  {
    if (i == m_nComboSelect)
    {
      return (*iGame);
    }
  }
  return 0; // not found
}

void CGameDialog::InitGlobalPrefPath()
{
  g_Preferences.m_global_rc_path = g_string_new(SettingsPath_get());
}

void CGameDialog::Reset()
{
  if (!g_Preferences.m_global_rc_path)
    InitGlobalPrefPath();
  StringOutputStream strGlobalPref(256);
  strGlobalPref << g_Preferences.m_global_rc_path->str << "global.pref";
  file_remove(strGlobalPref.c_str());
}

void CGameDialog::Init()
{
  InitGlobalPrefPath();
  LoadPrefs();
  ScanForGames();
  if (mGames.empty())
  {
    Error("Didn't find any valid game file descriptions, aborting\n");
  }
 
  CGameDescription* currentGameDescription = 0;

  if (!m_bGamePrompt)
  {
    // search by .game name
    std::list<CGameDescription *>::iterator iGame;
    for(iGame=mGames.begin(); iGame!=mGames.end(); ++iGame)
    {
      if ((*iGame)->mGameFile == m_sGameFile)
      {
        currentGameDescription = (*iGame);
        break;
      }
    }
  }
  if (m_bGamePrompt || !currentGameDescription)
  {
    Create();
    DoGameDialog();
    // use m_nComboSelect to identify the game to run as and set the globals
    currentGameDescription = GameDescriptionForComboItem();
    ASSERT_NOTNULL(currentGameDescription);
  }
  g_pGameDescription = currentGameDescription;

  g_pGameDescription->Dump();
}

CGameDialog::~CGameDialog()
{
  // free all the game descriptions
  std::list<CGameDescription *>::iterator iGame;
  for(iGame=mGames.begin(); iGame!=mGames.end(); ++iGame)
  {
    delete (*iGame);
    *iGame = 0;
  }
  if(GetWidget() != 0)
  {
    Destroy();
  }
}

inline const char* GameDescription_getIdentifier(const CGameDescription& gameDescription)
{
  const char* identifier = gameDescription.getKeyValue("index");
  if(string_empty(identifier))
  {
    identifier = "1";
  }
  return identifier;
}

void CGameDialog::AddPacksURL(StringOutputStream &URL)
{
  // add the URLs for the list of game packs installed
  // FIXME: this is kinda hardcoded for now..
  std::list<CGameDescription *>::iterator iGame;
  for(iGame=mGames.begin(); iGame!=mGames.end(); ++iGame)
  {
    URL << "&Games_dlup%5B%5D=" << GameDescription_getIdentifier(*(*iGame));
  }
}

CGameDialog g_GamesDialog;


// =============================================================================
// Widget callbacks for PrefsDlg

static void OnButtonClean (GtkWidget *widget, gpointer data) 
{
  // make sure this is what the user wants
  if (gtk_MessageBox(GTK_WIDGET(g_Preferences.GetWidget()), "This will close Radiant and clean the corresponding registry entries.\n"
      "Next time you start Radiant it will be good as new. Do you wish to continue?",
      "Reset Registry", eMB_YESNO, eMB_ICONASTERISK) == eIDYES)
  {
    PrefsDlg *dlg = (PrefsDlg*)data;
    dlg->EndModal (eIDCANCEL);

    g_preferences_globals.disable_ini = true;
    Preferences_Reset();
    gtk_main_quit();
  }
}

// =============================================================================
// PrefsDlg class

/*
========

very first prefs init deals with selecting the game and the game tools path
then we can load .ini stuff

using prefs / ini settings:
those are per-game

look in ~/.radiant/<version>/gamename
========
*/

#define PREFS_LOCAL_FILENAME "local.pref"

void PrefsDlg::Init()
{
  // m_global_rc_path has been set above
  // m_rc_path is for game specific preferences
  // takes the form: global-pref-path/gamename/prefs-file

  // this is common to win32 and Linux init now
  m_rc_path = g_string_new (m_global_rc_path->str);
  
  // game sub-dir
  g_string_append (m_rc_path, g_pGameDescription->mGameFile.c_str());
  g_string_append (m_rc_path, "/");
  Q_mkdir (m_rc_path->str);
  
  // then the ini file
  m_inipath = g_string_new (m_rc_path->str);
  g_string_append (m_inipath, PREFS_LOCAL_FILENAME);
}

void notebook_set_page(GtkWidget* notebook, GtkWidget* page)
{
  int pagenum = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), page);
  if(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) != pagenum)
  {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pagenum);
  }
}

void PrefsDlg::showPrefPage(GtkWidget* prefpage)
{
  notebook_set_page(m_notebook, prefpage);
  return;
}

static void treeSelection(GtkTreeSelection* selection, gpointer data)
{
  PrefsDlg *dlg = (PrefsDlg*)data;

  GtkTreeModel* model;
  GtkTreeIter selected;
  if(gtk_tree_selection_get_selected(selection, &model, &selected))
  {
    GtkWidget* prefpage;
    gtk_tree_model_get(model, &selected, 1, (gpointer*)&prefpage, -1);
    dlg->showPrefPage(prefpage);
  }
}

typedef std::list<PreferenceGroupCallback> PreferenceGroupCallbacks;

inline void PreferenceGroupCallbacks_constructGroup(const PreferenceGroupCallbacks& callbacks, PreferenceGroup& group)
{
  for(PreferenceGroupCallbacks::const_iterator i = callbacks.begin(); i != callbacks.end(); ++i)
  {
    (*i)(group);
  }
}


inline void PreferenceGroupCallbacks_pushBack(PreferenceGroupCallbacks& callbacks, const PreferenceGroupCallback& callback)
{
  callbacks.push_back(callback);
}

typedef std::list<PreferencesPageCallback> PreferencesPageCallbacks;

inline void PreferencesPageCallbacks_constructPage(const PreferencesPageCallbacks& callbacks, PreferencesPage& page)
{
  for(PreferencesPageCallbacks::const_iterator i = callbacks.begin(); i != callbacks.end(); ++i)
  {
    (*i)(page);
  }
}

inline void PreferencesPageCallbacks_pushBack(PreferencesPageCallbacks& callbacks, const PreferencesPageCallback& callback)
{
  callbacks.push_back(callback);
}

PreferencesPageCallbacks g_interfacePreferences;
void PreferencesDialog_addInterfacePreferences(const PreferencesPageCallback& callback)
{
  PreferencesPageCallbacks_pushBack(g_interfacePreferences, callback);
}
PreferenceGroupCallbacks g_interfaceCallbacks;
void PreferencesDialog_addInterfacePage(const PreferenceGroupCallback& callback)
{
  PreferenceGroupCallbacks_pushBack(g_interfaceCallbacks, callback);
}

PreferencesPageCallbacks g_displayPreferences;
void PreferencesDialog_addDisplayPreferences(const PreferencesPageCallback& callback)
{
  PreferencesPageCallbacks_pushBack(g_displayPreferences, callback);
}
PreferenceGroupCallbacks g_displayCallbacks;
void PreferencesDialog_addDisplayPage(const PreferenceGroupCallback& callback)
{
  PreferenceGroupCallbacks_pushBack(g_displayCallbacks, callback);
}

PreferencesPageCallbacks g_settingsPreferences;
void PreferencesDialog_addSettingsPreferences(const PreferencesPageCallback& callback)
{
  PreferencesPageCallbacks_pushBack(g_settingsPreferences, callback);
}
PreferenceGroupCallbacks g_settingsCallbacks;
void PreferencesDialog_addSettingsPage(const PreferenceGroupCallback& callback)
{
  PreferenceGroupCallbacks_pushBack(g_settingsCallbacks, callback);
}

void Widget_updateDependency(GtkWidget* self, GtkWidget* toggleButton)
{
  gtk_widget_set_sensitive(self, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggleButton)) && GTK_WIDGET_IS_SENSITIVE(toggleButton));
}

void ToggleButton_toggled_Widget_updateDependency(GtkWidget *toggleButton, GtkWidget* self)
{
  Widget_updateDependency(self, toggleButton);
}

void ToggleButton_state_changed_Widget_updateDependency(GtkWidget* toggleButton, GtkStateType state, GtkWidget* self)
{
  if(state == GTK_STATE_INSENSITIVE)
  {
    Widget_updateDependency(self, toggleButton);
  }
}

void Widget_connectToggleDependency(GtkWidget* self, GtkWidget* toggleButton)
{
  g_signal_connect(G_OBJECT(toggleButton), "state_changed", G_CALLBACK(ToggleButton_state_changed_Widget_updateDependency), self);
  g_signal_connect(G_OBJECT(toggleButton), "toggled", G_CALLBACK(ToggleButton_toggled_Widget_updateDependency), self);
  Widget_updateDependency(self, toggleButton);
}


inline GtkWidget* getVBox(GtkWidget* page)
{
  return gtk_bin_get_child(GTK_BIN(page));
}

GtkTreeIter PreferenceTree_appendPage(GtkTreeStore* store, GtkTreeIter* parent, const char* name, GtkWidget* page)
{
  GtkTreeIter group;
  gtk_tree_store_append(store, &group, parent);
  gtk_tree_store_set(store, &group, 0, name, 1, page, -1);
  return group;
}

GtkWidget* PreferencePages_addPage(GtkWidget* notebook, const char* name)
{
  GtkWidget* preflabel = gtk_label_new(name);
  gtk_widget_show(preflabel);

  GtkWidget* pageframe = gtk_frame_new(name);
  gtk_container_set_border_width(GTK_CONTAINER(pageframe), 4);
  gtk_widget_show(pageframe);

  GtkWidget* vbox = gtk_vbox_new(FALSE, 4);
  gtk_widget_show(vbox);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
  gtk_container_add(GTK_CONTAINER(pageframe), vbox);

  // Add the page to the notebook
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), pageframe, preflabel);

  return pageframe;
}

class PreferenceTreeGroup : public PreferenceGroup
{
  Dialog& m_dialog;
  GtkWidget* m_notebook;
  GtkTreeStore* m_store;
  GtkTreeIter m_group;
public:
  PreferenceTreeGroup(Dialog& dialog, GtkWidget* notebook, GtkTreeStore* store, GtkTreeIter group) :
    m_dialog(dialog),
    m_notebook(notebook),
    m_store(store),
    m_group(group)
  {
  }
  PreferencesPage createPage(const char* treeName, const char* frameName)
  {
    GtkWidget* page = PreferencePages_addPage(m_notebook, frameName);
    PreferenceTree_appendPage(m_store, &m_group, treeName, page);
    return PreferencesPage(m_dialog, getVBox(page));
  }
};

GtkWindow* PrefsDlg::BuildDialog()
{
  PreferencesDialog_addInterfacePreferences(FreeCaller1<PreferencesPage&, Interface_constructPreferences>());
  Mouse_registerPreferencesPage();

  GtkWindow* dialog = create_floating_window("GtkRadiant Preferences", m_parent);

  {
    GtkWidget* mainvbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(dialog), mainvbox);
    gtk_container_set_border_width(GTK_CONTAINER(mainvbox), 5);
    gtk_widget_show(mainvbox);
  
    {
      GtkWidget* hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_end(GTK_BOX(mainvbox), hbox, FALSE, TRUE, 0);

      {
        GtkButton* button = create_dialog_button("OK", G_CALLBACK(dialog_button_ok), &m_modal);
        gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
      {
        GtkButton* button = create_dialog_button("Cancel", G_CALLBACK(dialog_button_cancel), &m_modal);
        gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
      {
        GtkButton* button = create_dialog_button("Clean", G_CALLBACK(OnButtonClean), this);
        gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);
      }
    }
  
    {
      GtkWidget* hbox = gtk_hbox_new(FALSE, 5);
      gtk_box_pack_start(GTK_BOX(mainvbox), hbox, TRUE, TRUE, 0);
      gtk_widget_show(hbox);
  
      {
        GtkWidget* sc_win = gtk_scrolled_window_new(0, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_win), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(hbox), sc_win, FALSE, FALSE, 0);
        gtk_widget_show(sc_win);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sc_win), GTK_SHADOW_IN);

        // prefs pages notebook
        m_notebook = gtk_notebook_new();
        // hide the notebook tabs since its not supposed to look like a notebook
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_notebook), FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), m_notebook, TRUE, TRUE, 0);
        gtk_widget_show(m_notebook);


        {
          GtkTreeStore* store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

          GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
          gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

          {
            GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Preferences", renderer, "text", 0, 0);
            gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
          }

          {
            GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
            g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(treeSelection), this);
          }

          gtk_widget_show(view);

          gtk_container_add(GTK_CONTAINER (sc_win), view);

          {
            /********************************************************************/
            /* Add preference tree options                                      */
            /********************************************************************/
            // Front page... 
            //GtkWidget* front =
            PreferencePages_addPage(m_notebook, "Front Page");

            {
              GtkWidget* global = PreferencePages_addPage(m_notebook, "Global Preferences");
              {
                PreferencesPage preferencesPage(*this, getVBox(global));
                Global_constructPreferences(preferencesPage);
              }
              GtkTreeIter group = PreferenceTree_appendPage(store, 0, "Global", global);
              {
                GtkWidget* game = PreferencePages_addPage(m_notebook, "Game");
                PreferencesPage preferencesPage(*this, getVBox(game));
                g_GamesDialog.CreateGlobalFrame(preferencesPage);

                PreferenceTree_appendPage(store, &group, "Game", game);
              }
            }

            {
              GtkWidget* interfacePage = PreferencePages_addPage(m_notebook, "Interface Preferences");
              {
                PreferencesPage preferencesPage(*this, getVBox(interfacePage));
                PreferencesPageCallbacks_constructPage(g_interfacePreferences, preferencesPage);
              }

              GtkTreeIter group = PreferenceTree_appendPage(store, 0, "Interface", interfacePage);
              PreferenceTreeGroup preferenceGroup(*this, m_notebook, store, group);

              PreferenceGroupCallbacks_constructGroup(g_interfaceCallbacks, preferenceGroup);
            }

            {
              GtkWidget* display = PreferencePages_addPage(m_notebook, "Display Preferences");
              {
                PreferencesPage preferencesPage(*this, getVBox(display));
                PreferencesPageCallbacks_constructPage(g_displayPreferences, preferencesPage);
              }
              GtkTreeIter group = PreferenceTree_appendPage(store, 0, "Display", display);
              PreferenceTreeGroup preferenceGroup(*this, m_notebook, store, group);

              PreferenceGroupCallbacks_constructGroup(g_displayCallbacks, preferenceGroup);
            }

            {
              GtkWidget* settings = PreferencePages_addPage(m_notebook, "General Settings");
              {
                PreferencesPage preferencesPage(*this, getVBox(settings));
                PreferencesPageCallbacks_constructPage(g_settingsPreferences, preferencesPage);
              }

              GtkTreeIter group = PreferenceTree_appendPage(store, 0, "Settings", settings);
              PreferenceTreeGroup preferenceGroup(*this, m_notebook, store, group);

              PreferenceGroupCallbacks_constructGroup(g_settingsCallbacks, preferenceGroup);
            }
          }

          gtk_tree_view_expand_all(GTK_TREE_VIEW(view));
    
          g_object_unref(G_OBJECT(store));
        }
      }
    }
  }

  gtk_notebook_set_page(GTK_NOTEBOOK(m_notebook), 0);

  return dialog;
}

preferences_globals_t g_preferences_globals;

PrefsDlg g_Preferences;               // global prefs instance


void PreferencesDialog_constructWindow(GtkWindow* main_window)
{
  g_Preferences.m_parent = main_window;
  g_Preferences.Create();
}
void PreferencesDialog_destroyWindow()
{
  g_Preferences.Destroy();
}


PreferenceDictionary g_preferences;

PreferenceSystem& GetPreferenceSystem()
{
  return g_preferences;
}

class PreferenceSystemAPI
{
  PreferenceSystem* m_preferencesystem;
public:
  typedef PreferenceSystem Type;
  STRING_CONSTANT(Name, "*");

  PreferenceSystemAPI()
  {
    m_preferencesystem = &GetPreferenceSystem();
  }
  PreferenceSystem* getTable()
  {
    return m_preferencesystem;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<PreferenceSystemAPI> PreferenceSystemModule;
typedef Static<PreferenceSystemModule> StaticPreferenceSystemModule;
StaticRegisterModule staticRegisterPreferenceSystem(StaticPreferenceSystemModule::instance());

void Preferences_Load()
{
  g_GamesDialog.LoadPrefs();

  globalOutputStream() << "loading local preferences from " << g_Preferences.m_inipath->str << "\n";

  if(!Preferences_Load(g_preferences, g_Preferences.m_inipath->str))
  {
    globalOutputStream() << "failed to load local preferences from " << g_Preferences.m_inipath->str << "\n";
  }
}

void Preferences_Save()
{
  if (g_preferences_globals.disable_ini)
    return;

  g_GamesDialog.SavePrefs();

  globalOutputStream() << "saving local preferences to " << g_Preferences.m_inipath->str << "\n";

  if(!Preferences_Save_Safe(g_preferences, g_Preferences.m_inipath->str))
  {
    globalOutputStream() << "failed to save local preferences to " << g_Preferences.m_inipath->str << "\n";
  }
}

void Preferences_Reset()
{
  file_remove(g_Preferences.m_inipath->str);
}


void PrefsDlg::PostModal (EMessageBoxReturn code)
{
  if (code == eIDOK)
  {
    Preferences_Save();
    UpdateAllWindows();
  }
}

std::vector<const char*> g_restart_required;

void PreferencesDialog_restartRequired(const char* staticName)
{
  g_restart_required.push_back(staticName);
}

void PreferencesDialog_showDialog()
{
  if(ConfirmModified("Edit Preferences") && g_Preferences.DoModal() == eIDOK)
  {
    if(!g_restart_required.empty())
    {
      StringOutputStream message(256);
      message << "Preference changes require a restart:\n";
      for(std::vector<const char*>::iterator i = g_restart_required.begin(); i != g_restart_required.end(); ++i)
      {
        message << (*i) << '\n';
      }
      gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()), message.c_str());
      g_restart_required.clear();
    }
  }
}





void GameName_importString(const char* value)
{
  gamename_set(value);
}
typedef FreeCaller1<const char*, GameName_importString> GameNameImportStringCaller;
void GameName_exportString(const StringImportCallback& importer)
{
  importer(gamename_get());
}
typedef FreeCaller1<const StringImportCallback&, GameName_exportString> GameNameExportStringCaller;

void GameMode_importString(const char* value)
{
  gamemode_set(value);
}
typedef FreeCaller1<const char*, GameMode_importString> GameModeImportStringCaller;
void GameMode_exportString(const StringImportCallback& importer)
{
  importer(gamemode_get());
}
typedef FreeCaller1<const StringImportCallback&, GameMode_exportString> GameModeExportStringCaller;


void RegisterPreferences(PreferenceSystem& preferences)
{

#ifdef WIN32
  preferences.registerPreference("NativeGUI", BoolImportStringCaller(g_FileChooser_nativeGUI), BoolExportStringCaller(g_FileChooser_nativeGUI));
#endif


#ifdef WIN32
  preferences.registerPreference("UseCustomShaderEditor", BoolImportStringCaller(g_TextEditor_useWin32Editor), BoolExportStringCaller(g_TextEditor_useWin32Editor));
#else
  preferences.registerPreference("UseCustomShaderEditor", BoolImportStringCaller(g_TextEditor_useCustomEditor), BoolExportStringCaller(g_TextEditor_useCustomEditor));
  preferences.registerPreference("CustomShaderEditorCommand", CopiedStringImportStringCaller(g_TextEditor_editorCommand), CopiedStringExportStringCaller(g_TextEditor_editorCommand));
#endif

  preferences.registerPreference("GameName", GameNameImportStringCaller(), GameNameExportStringCaller());
  preferences.registerPreference("GameMode", GameModeImportStringCaller(), GameModeExportStringCaller());
}

void Preferences_Init()
{
  RegisterPreferences(GetPreferenceSystem());
}
