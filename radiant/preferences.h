/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include "dialog.h"
#include "gtkr_list.h"
//#include "profile.h"

#ifdef _WIN32
#define NVIDIA_AERO_HACK
#endif

#define MAX_TEXTURE_QUALITY 3

enum PrefTypes_t
{
	PREF_STR,
	PREF_INT,
	PREF_BOOL,
	PREF_FLOAT,
	PREF_VEC3,
	PREF_WNDPOS,
};

/*!
   a preference assignment, name, type and pointer to value
   we don't store the xmlNodePtr because the document itself can be thrown away upon any LoadPref
   (see CGameDialog::UpdatePrefTree)
 */
class CPrefAssignment
{
public:
Str mName;
PrefTypes_t mType;
void *mVal;

CPrefAssignment( const char *name, PrefTypes_t Type, void *Val ){
	mName = name; mType = Type; mVal = Val;
}
CPrefAssignment() { mVal = NULL; }
CPrefAssignment( const CPrefAssignment& ass );
virtual ~CPrefAssignment() { }
virtual CPrefAssignment& operator =( const CPrefAssignment& ass );
};


/*!
   generic preferences storage class, using xml files
 */
class CXMLPropertyBag
{
private:
/*!
   local prefs file
 */
xmlDocPtr mpDoc;
xmlNodePtr mpDocNode;

/*!
   prefs assignments (what pref name, what type, what variable)
 */
list<CPrefAssignment> mPrefAssignments;

/*!
   name of file to load/save as
 */
Str mStrFilename;

/*!
   store assignment in the property list if not already there
 */
void PushAssignment( const char *name, PrefTypes_t type, void *pV );

/*!
   find the xmlnode relating to the epair name
 */
xmlNodePtr EpairForName( const char *name );

public:
CXMLPropertyBag();
virtual ~CXMLPropertyBag(){
	if ( InUse() ) {
		Clear();
	}
};

/*!
   read a pref setting, if doesn't exist, will add it to the xml tree (using default value provided)
   \arg name the name of the pref
   \arg pV pointer to the value
   \arg V default value
   those functions will fill in the list of preferences assignments
   (name, type and pointer to value)
   this is used in UpdatePrefTree
 */
void GetPref( const char *name, Str *pV, const char *V );
void GetPref( const char *name, int *pV, int V );
void GetPref( const char *name, bool *pV, bool V );
void GetPref( const char *name, float *pV, float V );
void GetPref( const char *name, float *pV, float* V );
void GetPref( const char *name, window_position_t* pV, window_position_t V );

/*!
   returns whether or not the property bag is already open
 */
qboolean InUse() { return ( mpDoc != NULL ); };

/*!
   unload the xml doc, and free the tree
 */
void Clear();

/*|
   read data from our XML file
 */
void ReadXMLFile( const char* pFilename );

/*|
   write out the property bag to an XML data file
   return is success/fail
 */
qboolean WriteXMLFile( const char* pFilename );

/*!
   update the xml tree with data form the property list, usually in preparation for a write
 */
void UpdatePrefTree();

/*!
   did the file have any data or not?
 */
qboolean mbEmpty;
};

/*!
   holds information for a given game
   I'm a bit unclear on that still
   it holds game specific configuration stuff
   such as base names, engine names, some game specific features to activate in the various modules
   it is not strictly a prefs thing since the user is not supposed to edit that (unless he is hacking
   support for a new game)

   what we do now is fully generate the information for this during the setup. We might want to
   generate a piece that just says "the game pack is there", but put the rest of the config somwhere
   else (i.e. not generated, copied over during setup .. for instance in the game tools directory)
 */
class CGameDescription
{
public:
xmlDocPtr mpDoc;   ///< the game description xml tree
Str mGameToolsPath;   ///< the explicit path to the game-dependent modules
Str mGameName;   ///< name of the game used in dialogs
Str mGameFile;   ///< the .game file that describes this game
Str mBaseGame;   ///< basegame directory
Str mEnginePath;   ///< path to the engine
Str mExecutablesPath;   ///< path to external executables, e.g. /usr/local/bin
Str mEngine;   ///< engine name
Str mMultiplayerEngine;   ///< engine name
Str mUserPathPrefix;   ///< prefix for ~/.q3a ~/.wolf init on *nix, or \My Document\My Games\ on Windows
Str mShaderPath;   ///< the path in which to look for shaders
Str mShaderlist;   ///< shaderlist file
float mTextureDefaultScale;   ///< default scale (0.5 in q3, 1.0 in q1/q2, 0.25 in JK2 ..)
bool mEClassSingleLoad;   ///< only load a single eclass definition file
bool mNoPatch;   ///< this game doesn't support patch technology
Str mCaulkShader;   ///< the shader to use for caulking
bool noMapsInHome;   ///< set this if you want to open the engine path/base dir/maps dir for map open/save dialoges */
bool idTech2;  // set this to true for idTech2 games

CGameDescription() { mpDoc = NULL; }
/*!
   \todo parse basic info from the node
   user-friendly name of the game
   essential parameters (such as the start dir)
 */
CGameDescription( xmlDocPtr pDoc, const Str &GameFile );
virtual ~CGameDescription() { xmlFreeDoc( mpDoc ); }

void Dump();
};

/*!
   select games, copy editing assets and write out configuration files
 */

#define Q3_GAME "q3.game"
#define URT_GAME "urt.game"
#define UFOAI_GAME "ufoai.game"
#define QUETOO_GAME "quetoo.game"
#define WARSOW_GAME "warsow.game"
#define NEXUIZ_GAME "nexuiz.game"
#define Q2_GAME "q2.game"
#define TREMULOUS_GAME "tremulous.game"
#define JA_GAME "ja.game"
#define REACTION_GAME "reaction.game"
#define ET_GAME "et.game"
#define QL_GAME "ql.game"
#define STVEF_GAME "stvef.game"
#define WOLF_GAME "wolf.game"
#define Q1_GAME "q1.game"

#define Q3_PACK "Q3Pack"
#define URT_PACK "UrTPack"
#define UFOAI_PACK "UFOAIPack"
#define QUETOO_PACK "QuetooPack"
#define WARSOW_PACK "WarsowPack"
#define NEXUIZ_PACK "NexuizPack"
#define Q2_PACK "Q2Pack"
#define TREMULOUS_PACK "TremulousPack"
#define JA_PACK "JAPack"
#define REACTION_PACK "ReactionPack"
#define ET_PACK "ETPack"
#define QL_PACK "QLPack"
#define STVEF_PACK "STVEFPack"
#define WOLF_PACK "WolfPack"
#define Q1_PACK "Q1Pack"

class CGameInstall : public Dialog {
public:
  CGameInstall();
  void ScanGames();
  void Run();
  void BuildDialog();

  static void OnBtnBrowseEngine( GtkWidget *widget, gpointer data );
  static void OnBtnBrowseExecutables( GtkWidget *widget, gpointer data );
  static void OnGameSelectChanged( GtkWidget *widget, gpointer data );

  enum gameType_e {
	GAME_NONE = 0,
	GAME_Q3 = 1,
	GAME_URT,
	GAME_UFOAI,
	GAME_QUETOO,
	GAME_WARSOW,
	GAME_NEXUIZ,
	GAME_Q2,
	GAME_TREMULOUS,
	GAME_JA,
	GAME_REACTION,
	GAME_ET,
	GAME_QL,
	GAME_STVEF,
	GAME_WOLF,
	GAME_Q1,
	GAME_COUNT
  };

protected:
  Str m_strName;
  Str m_strMod;
  Str m_strEngine;
  Str m_strExecutables;
  int m_nComboSelect;

  // maps from m_nComboSelect to the games
  int m_availGames[GAME_COUNT];

  GtkWidget * m_executablesVBox;
};

/*!
   standalone dialog for games selection, and more generally global settings
 */
class CGameDialog : public Dialog
{
GtkWidget *mFrame;   ///< this is built on-demand first time it's used
GtkWidget *mTopBox;   ///< top level box used to store the dialog frame, must unhook after modal use

GtkComboBox   *mGameCombo;      // combo box holds the selection of available game

/*!
   global prefs storage
 */
CXMLPropertyBag mGlobalPrefs;

#ifdef _WIN32
/*!
   run from a network share
   this one is not being saved out in prefs, since we need to know before we load prefs
   we use a dummy file NETRUN_FILENAME as flag
   all done with static stuff
 */
static bool m_bNetRun;
#endif

bool m_bDoGameInstall;

CGameInstall mGameInstall;

protected:

int m_nComboSelect;   ///< intermediate int value for combo in dialog box

public:

/*!
   those settings are saved in the global prefs file
   I'm too lazy to wrap behind protected access, not sure this needs to be public
   NOTE: those are preference settings. if you change them it is likely that you would
   have to restart the editor for them to take effect
 */
/*@{*/
/*!
   what game has been selected
   this is the name of the .game file
 */
Str m_sGameFile;
/*!
   auto-load the game on startup
   this is linked to auto-load checkbox
 */
bool m_bAutoLoadGame;
/*!
   log console to radiant.log
   m_bForceLogConsole is an obscure forced latching situation
 */
bool m_bLogConsole;
bool m_bForceLogConsole;
/*@}*/

/*!
   points somewhere in mGames, set once at startup
 */
CGameDescription *m_pCurrentGameDescription;

/*!
   the list of game descriptions we scanned from the game/ dir
 */
list<CGameDescription *> mGames;

CGameDialog() {
	mFrame = NULL;
	m_pCurrentGameDescription = NULL;
	m_bLogConsole = false;
	m_bForceLogConsole = false;
	m_bDoGameInstall = true;    // go through DoModal at least once
	mGameCombo = NULL;
}
virtual ~CGameDialog();

void AddPacksURL( Str &s );

/*!
   intialize the game dialog, called at CPrefsDlg::Init
   will scan for games, load prefs, and do game selection dialog if needed
 */
void Init();

/*!
   reset the global settings by removing the file
 */
void Reset();

/*!
   run the dialog UI for the list of games
 */
void DoGameDialog();

/*!
   call out to the game installation dialog
 */
void DoGameInstall();

/*!
   Dialog API
   this is only called when the dialog is built at startup for main engine select
 */
void BuildDialog();
void UpdateData( bool retrieve );

/*!
   construction of the dialog frame
   this is the part to be re-used in prefs dialog
   for the standalone dialog, we include this in a modal box
   for prefs, we hook the frame in the main notebook
   build the frame on-demand (only once)
 */
GtkWidget *GetGlobalFrame();

/*!
   global preferences subsystem
   XML-based this time, hopefully this will generalize to other prefs
   LoadPrefs has hardcoded defaults
   NOTE: it may not be strictly 'CGameDialog' to put the global prefs here
   could have named the class differently I guess
 */
/*@{*/
void LoadPrefs();   ///< load from file into variables
void SavePrefs();   ///< save pref variables to file
/*@}*/

/*!
   read or set netrun (check file)
   \param retrieve
   if false, will check if netrun file is present and will set m_bNetRun
   if true, will create/erase the netrun file depending on m_bNetRun
   NOTE: this is not backwards, 'retrieve' means 'retrieve from settings dialog' - in terms of UI
 */
static void UpdateNetrun( bool retrieve );
/*!
   get current netrun setting
 */
static bool GetNetrun();

private:
/*!
   scan for .game files, load them
 */
void ScanForGames();

/*!
   inits g_PrefsDlg.m_global_rc_path
 */
void InitGlobalPrefPath();

/*!
   uses m_nComboItem to find the right mGames
 */
CGameDescription *GameDescriptionForComboItem();

/*!
   callback for the game install button
 */
static void SInstallCallback( GtkWidget *widget, gpointer data );

void UpdateGameCombo();
};

typedef struct {
	int nEntitySplit1;
	int nEntitySplit2;

	window_position_t position;

	window_position_t posEntityWnd;
	window_position_t posMapInfoWnd;
	window_position_t posCamWnd;
	window_position_t posZWnd;
	window_position_t posXYWnd;
	window_position_t posXZWnd;
	window_position_t posYZWnd;
	window_position_t posPatchWnd;
	window_position_t posSurfaceWnd;
	window_position_t posEntityInfoWnd;

	int nXYHeight;
	int nZWidth;
	int nXYWidth;
	int nCamWidth;
	int nCamHeight;
	int nZFloatWidth;
	int nState;
} windowPosInfo_t;

class PrefsDlg : public Dialog
{

public:
/*!
   local prefs file
 */
CXMLPropertyBag mLocalPrefs;

// will enable/disable stuff according to the situation
void DoSensitivity();
void PreModal() { DoSensitivity(); }

// enable/disable custom editor entry
void DoEditorSensitivity();

/*!
   this holds global level preferences
 */
CGameDialog mGamesDialog;
protected:
// warning about old project files
bool m_bWarn;
list<CGameDescription *> mGames;

public:
// last light intensity used in the CLightPrompt dialog, stored in registry
int m_iLastLightIntensity;
// these mirror what goes in the combo box
// see PrefDlg::m_nShader, tells wether to load NONE / COMMON or ALL shaders at parsing stage
enum {SHADER_NONE = 0, SHADER_COMMON, SHADER_ALL};

// Gef: updated preferences dialog
/*! Preference notebook page numbers */
enum {PTAB_FRONT = 0, PTAB_GAME_SETTINGS, PTAB_2D, PTAB_CAMERA, PTAB_TEXTURE, PTAB_LAYOUT, PTAB_MOUSE,
	  PTAB_EDITING, PTAB_STARTUP, PTAB_PATHS, PTAB_BRUSH, PTAB_MISC, PTAB_BSPMONITOR} pref_tabs;

GtkWidget *notebook;

void UpdateTextureCompression();

#ifdef ATIHACK_812
void UpdateATIHack();
#endif

#ifdef NVIDIA_AERO_HACK
void UpdateNvidiaAeroHack();
#endif

void LoadPrefs();
void SavePrefs();
void LoadTexdefPref( texdef_t* pTexdef, const char* pName );

PrefsDlg ();
virtual ~PrefsDlg (){
	g_string_free( m_rc_path, true );
	g_string_free( m_inipath, true );
}

/*!
   path for global settings
   win32: g_strAppPath
   linux: ~/.radiant/<version>/
 */
GString *m_global_rc_path;

/*!
   path to per-game settings
   used for various game dependant storage
   win32: g_strGameToolsPath
   linux: ~/.radiant/<version>/<gamename>/
 */
GString *m_rc_path;

/*!
   holds per-game settings
   m_rc_path+"local.pref"
   \todo FIXME at some point this should become XML property bag code too
 */
GString *m_inipath;

// initialize the above paths
void Init();

#if 0
// DEPRECATED: use engine path from the current game description instead
// path to the top-level installation
Str m_strEnginePath;
// name of executable
// quake2 quake3 etc
Str m_strEngine;
// we use this Str to store the full path to the engine: m_strEnginePath + m_strEngine
// it's not stored in the registry or anything, just ued for display in prefs
Str m_strPrefsDlgEngine;
#endif

// Dialog Data
int m_nMouse;
MainFrame::EViewStyle m_nView;
bool m_bTextureLock;
bool m_bLoadLast;
// path to the project loaded at startup
// if g_PrefsDlg can't find the information in the ini file
// it will try to guess and eventually ask the user
Str m_strLastProject;
/*!
   version of last loaded project file
   says -1 if there's no version loaded
   if it's a manually constructed project file, will be 0
   otherwise the actual 'version' epair
 */
int m_nLastProjectVer;
Str m_strLastMap;
bool m_bInternalBSP;
bool m_bRightClick;
bool m_bSetGame;
bool m_bAutoSave;
bool m_bLoadLastMap;
bool m_bTextureWindow;
bool m_bSnapShots;
float m_fTinySize;
bool m_bCleanTiny;
bool m_bCamXYUpdate;
int m_nCamDragMultiSelect;
bool m_bCamDragMultiSelect;
bool m_bCamFreeLook;
bool m_bCamFreeLookStrafe;
bool m_bCamInverseMouse;
bool m_bCamDiscrete;
bool m_bNewLightDraw;
Str m_strPrefabPath;
int m_nWhatGame;
bool m_bALTEdge;
bool m_bFaceColors;
bool m_bXZVis;
bool m_bYZVis;
bool m_bZVis;
bool m_bSizePaint;
bool m_bDLLEntities;
bool m_bRotateLock;
bool m_bDetachableMenus;
bool m_bPatchToolbar;
bool m_bWideToolbar;
bool m_bPluginToolbar;
bool m_bNoClamp;
//++timo this is most likely broken, I don't know what it's supposed to do
bool m_bSnap;
Str m_strUserPath;
int m_nRotation;
bool m_bChaseMouse;
bool m_bMousewheelZoom;
bool m_bTextureScrollbar;
bool m_bDisplayLists;
bool m_bAntialiasedPointsAndLines;    // Fishman - Add antialiazed points and lines support. 09/03/00
bool m_bShowShaders;
int m_nShader;
bool m_bNoStipple;
int m_nUndoLevels;
bool m_bVertexSplit;

int m_nMouseButtons;
int m_nAngleSpeed;
int m_nMoveSpeed;
int m_nAutoSave;
bool m_bCubicClipping;
int m_nCubicScale;
bool m_bSelectCurves;
bool m_bSelectModels;
int m_nEntityShowState;
int m_nTextureScale;
bool m_bNormalizeColors;
bool m_bSwitchClip;
bool m_bSelectWholeEntities;
int m_nTextureQuality;
bool m_bGLLighting;
bool m_bTexturesShaderlistOnly;
int m_nSubdivisions;
float m_fDefTextureScale;
bool m_bCaulkNewBrushes;
bool m_bFloatingZ;
bool m_bLatchedFloatingZ;
// Gef: Kyro GL_POINT workaround
bool m_bGlPtWorkaround;

// how many menus in the texture thing before we split?
int m_nTextureMenuSplit;

// watch the BSP process through network connections
// true: trigger the BSP steps one by one and monitor them through the network
// false: create a BAT / .sh file and execute it. don't bother monitoring it.
bool m_bWatchBSP;
// do we stop the compilation process if we come accross a leak?
bool m_bLeakStop;
// timeout when beginning a step (in seconds)
// if we don't get a connection quick enough we assume something failed and go back to idling
int m_iTimeout;
bool m_bRunQuake;
// store prefs setting for automatic sleep mode activation
bool m_bDoSleep;

bool m_bClipCaulk;

// make the texture increments match the grid changes
bool m_bSnapTToGrid;

// try to fix the target/targetname conflicts when importing a map (default true)
bool m_bDoTargetFix;

// the increment step we use against the wheel mouse
int m_nWheelInc;

#ifdef _WIN32
// use the file associations to open files instead of builtin Gtk editor
bool m_bUseWin32Editor;
#else
// custom shader editor
bool m_bUseCustomEditor;
Str m_strEditorCommand;     // this is the command executed
#endif

#ifdef _WIN32
bool m_bNativeGUI;
bool m_bStartOnPrimMon;
#endif

bool m_bPatchBBoxSelect;

// RR2DO2: latched data, for settings that require a restart. We don't want to set
// these directly in case users set them under preferences and then continue working
// with the editor.
MainFrame::EViewStyle m_nLatchedView;
int m_nMRUCount;
Str m_strMRUFiles[4];

windowPosInfo_t mWindowInfo;

bool m_bLatchedDetachableMenus;
bool m_bLatchedPatchToolbar;
bool m_bLatchedWideToolbar;
bool m_bLatchedPluginToolbar;
int m_nLatchedShader;
int m_nLatchedTextureQuality;

// RIANT
// texture compression format
int m_nTextureCompressionFormat;

int m_nLightRadiuses;

bool m_bQ3Map2Texturing;
#ifdef _WIN32
bool m_bx64q3map2;
#endif

#ifdef ATIHACK_812
bool m_bGlATIHack;
#endif

#ifdef NVIDIA_AERO_HACK
bool m_bGlNvidiaAeroHack;
int m_bGlNvidiaAeroHackPrevState;
#endif

void UpdateData( bool retrieve );

/*! Utility function for swapping notebook pages for tree list selections */
void showPrefPage( int prefpage );

protected:
/*! Scan for game description files and build a list */
void ScanForGames();

/*! Dialog API */
void BuildDialog();
void PostModal( int code );
};

#endif // _PREFERENCES_H_
