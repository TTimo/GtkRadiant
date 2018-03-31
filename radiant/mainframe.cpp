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

//
// Main Window for GtkRadiant
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkprivate.h>
#include <sys/stat.h>
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <unistd.h>
#endif
#include "gtkmisc.h"
#include "groupdialog.h"
#include "patchdialog.h"
#include "filters.h"

// use this to verbose what happens with the keyboard
#ifdef _DEBUG
//  #define DBG_KBD
#endif

// globals
CString g_strAppPath;                   ///< holds the full path of the executable
CString g_strDTDPath;                   ///< path to the DTD files
CString g_pidFile;                      ///< the global .pid file (only for global part of the startup)
CString g_pidGameFile;                  ///< the game-specific .pid file
CString g_strBitmapsPath;               // directory where the bitmaps are stored

/*!
   points to the game tools directory, for instance
   C:\Program Files\Quake III Arena\GtkRadiant
   (or other games)
   this is one of the main variables that are configured by the game selection on startup
   <g_strGameToolsPath>/plugins
   <g_strGameToolsPath>/modules
   and also q3map, bspc
 */
CString g_strGameToolsPath;             ///< this is set by g_PrefsDlg.mGamesDialog
CGameDescription *g_pGameDescription;   ///< shortcut to g_PrefsDlg.mGamesDialog.m_pCurrentDescription
CString g_strPluginsDir;                ///< name of plugins directory, always sub-directory of toolspath
CString g_strModulesDir;                ///< name of modules directory, always sub-directory of toolspath

/*!
   Points to an optional directory where external (read: not local to the game
   install) may reside. For example, on Linux, a game's executables might live
   in /usr/local/bin. On Mac, they might be in /Applications/Game.app/Contents/MacOS.
 */
CString g_strExecutablesPath;

/*!
   directory for temp files
   NOTE: on *nix this is were we check for .pid
 */
CString g_strTempPath;
MainFrame* g_pParentWnd = NULL;         // used to precast to CMainFrame
PrefsDlg g_Preferences;                 // global prefs instance
PrefsDlg& g_PrefsDlg = g_Preferences;   // reference used throughout
int g_nUpdateBits = 0;                  // window update flags
bool g_bScreenUpdates = true;           // whether window painting is active, used in a few places
                                        // to disable updates for speed reasons
                                        // both of the above should be made members of CMainFrame
int g_bIgnoreCommands;                  // Used to ignore commands when checking menus/toolbars
GSList *g_BSPFrontendCommands;          // List of commands in the BSP menu

const int CMD_TEXTUREWAD_END = CMD_TEXTUREWAD + MAX_TEXTUREDIRS - 1;
const int CMD_BSPCOMMAND_END = CMD_BSPCOMMAND + 127;

extern bool g_bCrossHairs;
extern int g_argc;
extern char** g_argv;
extern PatchDialog g_PatchDialog;

GtkAccelGroup* global_accel;

void Select_Ungroup();

// command mapping stuff
//
// m_strCommand is the command string
// m_nKey is the GDK_??? equivelant
// m_nModifiers are key states as follows
//  bit
//    1 - shift
//    2 - alt
//    4 - control
//    8 - press only
//
#define SPEED_MOVE  32
#define SPEED_TURN  22.5

// NOTE: the menu item field is REQUIRED, Gtk uses it to bind the keyboard shortcut
// - if you add a command here and you don't want a menu item, use the "hidden" menu
// - if you decide to add a menu item, check if it's not in the "hidden" menu already
SCommandInfo g_Commands[] =
{
	{"CycleOutlineStyle", GDK_KEY_J, 0x00, ID_SELECTION_OUTLINESTYLE, "menu_selection_outlinestyle"},
	{"CSGMerge", GDK_KEY_U, 0x04, ID_SELECTION_CSGMERGE, "menu_selection_csgmerge"},
	{"CSGSubtract", GDK_KEY_U, 0x01, ID_SELECTION_CSGSUBTRACT, "menu_selection_csgsubstract"},
	//  {"ViewGroups", GDK_KEY_G, 0x00, ID_VIEW_GROUPS, "menu_view_groups"}, (temporary disabled)
	{"SelectFuncGroup", GDK_KEY_G, 0x00, ID_SELECT_FUNC_GROUP, "menu_select_func_group"},
	{"HideSelected", GDK_KEY_H, 0x00, ID_VIEW_HIDESHOW_HIDESELECTED, "menu_view_hideshow_hideselected"},
	{"ShowHidden", GDK_KEY_H, 0x01, ID_VIEW_HIDESHOW_SHOWHIDDEN, "menu_view_hideshow_showhidden"},
	{"BendMode", GDK_KEY_B, 0x00, ID_PATCH_BEND, "menu_patch_bend"},
	{"FitTexture", GDK_KEY_B, 0x01, IDC_BTN_FACEFIT, "menu_idc_btn_facefit"},
	{"ViewTextures", GDK_KEY_T, 0, ID_VIEW_TEXTURE, "menu_view_texture"},
	{"ThickenPatch", GDK_KEY_T, 0x04, ID_CURVE_THICKEN, "menu_curve_thicken"},
	{"MakeOverlayPatch", GDK_KEY_Y, 0, ID_CURVE_OVERLAY_SET, "menu_curve_overlay_set"},
	{"ClearPatchOverlays", GDK_KEY_L, 0x04, ID_CURVE_OVERLAY_CLEAR, "menu_curve_overlay_clear"},
	{"SurfaceInspector", GDK_KEY_S, 0, ID_TEXTURES_INSPECTOR, "menu_textures_inspector"},
	{"PatchInspector", GDK_KEY_S, 0x01, ID_PATCH_INSPECTOR, "menu_patch_inspector"},
	{"RedisperseRows", GDK_KEY_E, 0x04, ID_CURVE_REDISPERSE_ROWS, "menu_curve_redisperse_rows"},
	{"RedisperseIntermediateCols", GDK_KEY_E, 0x05, ID_CURVE_REDISPERSE_INTERMEDIATE_COLS, "menu_curve_redisperse_cols"},
	{"InvertCurveTextureX", GDK_KEY_I, 0x05, ID_CURVE_NEGATIVETEXTUREY, "menu_curve_negativetexturey"},
	{"InvertCurveTextureY", GDK_KEY_I, 0x01, ID_CURVE_NEGATIVETEXTUREX, "menu_curve_negativetexturex"},
	{"InvertCurve", GDK_KEY_I, 0x04, ID_CURVE_NEGATIVE, "menu_curve_negative"},
	{"IncPatchColumn", GDK_KEY_KP_Add, 0x05, ID_CURVE_INSERTCOLUMN, "menu_curve_insertcolumn"},
	{"IncPatchRow", GDK_KEY_KP_Add, 0x04, ID_CURVE_INSERTROW, "menu_curve_insertrow"},
	{"DecPatchColumn", GDK_KEY_KP_Subtract, 0x05, ID_CURVE_DELETECOLUMN, "menu_curve_deletecolumn"},
	{"DecPatchRow", GDK_KEY_KP_Subtract, 0x04, ID_CURVE_DELETEROW, "menu_curve_deleterow"},
	{"Patch TAB", GDK_KEY_Tab, 0x00, ID_PATCH_TAB, "menu_patch_tab"},
	{"Patch TAB", GDK_KEY_Tab, 0x01, ID_PATCH_TAB, "menu_patch_tab"},
	{"SelectNudgeDown", GDK_KEY_Down, 0x02, ID_SELECTION_SELECT_NUDGEDOWN, "menu_selection_select_nudgedown"},
	{"EntityColor",GDK_KEY_K, 0, ID_MISC_SELECTENTITYCOLOR, "menu_misc_select_entitycolor"},
	{"CameraForward", GDK_KEY_Up, 0, ID_CAMERA_FORWARD, "menu_camera_forward"},
	{"CameraBack", GDK_KEY_Down, 0, ID_CAMERA_BACK, "menu_camera_back"},
	{"CameraLeft", GDK_KEY_Left, 0, ID_CAMERA_LEFT, "menu_camera_left"},
	{"CameraRight", GDK_KEY_Right, 0, ID_CAMERA_RIGHT, "menu_camera_right"},
	{"CameraUp", GDK_KEY_D, 0, ID_CAMERA_UP, "menu_camera_up"},
	{"CameraDown", GDK_KEY_C, 0, ID_CAMERA_DOWN, "menu_camera_down"},
	{"CameraAngleUp", GDK_KEY_A, 0, ID_CAMERA_ANGLEUP, "menu_camera_angleup"},
	{"CameraAngleDown", GDK_KEY_Z, 0, ID_CAMERA_ANGLEDOWN, "menu_camera_angledown"},
	{"CameraStrafeRight", GDK_KEY_period, 0, ID_CAMERA_STRAFERIGHT, "menu_camera_straferight"},
	{"CameraStrafeLeft", GDK_KEY_comma, 0, ID_CAMERA_STRAFELEFT, "menu_camera_strafeleft"},
	{"ToggleGrid", '0', 0, ID_GRID_TOGGLE, "menu_grid_toggle"},
	{"ToggleGrid", GDK_KEY_0, 0, ID_GRID_TOGGLE, "menu_grid_toggle"},
	{"SetGrid1", '1', 0, ID_GRID_1, "menu_grid_1"},
	{"SetGrid1", GDK_KEY_1, 0, ID_GRID_1, "menu_grid_1"},
	{"SetGrid2", '2', 0, ID_GRID_2, "menu_grid_2"},
	{"SetGrid2", GDK_KEY_2, 0, ID_GRID_2, "menu_grid_2"},
	{"SetGrid4", '3', 0, ID_GRID_4, "menu_grid_4"},
	{"SetGrid4", GDK_KEY_3, 0, ID_GRID_4, "menu_grid_4"},
	{"SetGrid8", '4', 0, ID_GRID_8, "menu_grid_8"},
	{"SetGrid8", GDK_KEY_4, 0, ID_GRID_8, "menu_grid_8"},
	{"SetGrid16", '5', 0, ID_GRID_16, "menu_grid_16"},
	{"SetGrid16", GDK_KEY_5, 0, ID_GRID_16, "menu_grid_16"},
	{"SetGrid32", '6', 0, ID_GRID_32, "menu_grid_32"},
	{"SetGrid32", GDK_KEY_6, 0, ID_GRID_32, "menu_grid_32"},
	{"SetGrid64", '7', 0, ID_GRID_64, "menu_grid_64"},
	{"SetGrid64", GDK_KEY_7, 0, ID_GRID_64, "menu_grid_64"},
	{"SetGrid128", '8', 0, ID_GRID_128, "menu_grid_128"},
	{"SetGrid128", GDK_KEY_8, 0, ID_GRID_128, "menu_grid_128"},
	{"SetGrid256", '9', 0, ID_GRID_256, "menu_grid_256"},
	{"SetGrid256", GDK_KEY_9, 0, ID_GRID_256, "menu_grid_256"},
	{"DragEdges", GDK_KEY_E, 0, ID_SELECTION_DRAGEDGES, "menu_selection_dragedges"},
	{"DragVertices", GDK_KEY_V, 0, ID_SELECTION_DRAGVERTECIES, "menu_selection_dragvertecies"},
	{"ViewEntityInfo", GDK_KEY_N, 0, ID_VIEW_ENTITY, "menu_view_entity"},
	//  {"ViewConsole", 'O', 0, ID_VIEW_CONSOLE, "menu_0,"},
	{"CloneSelection", GDK_KEY_space, 0, ID_SELECTION_CLONE, "menu_selection_clone"},
	{"DeleteSelection", GDK_KEY_BackSpace, 0, ID_SELECTION_DELETE, "menu_selection_delete"},
	{"UnSelectSelection", GDK_KEY_Escape, 0, ID_SELECTION_DESELECT, "menu_selection_deselect"},
	{"CenterView", GDK_KEY_End, 0, ID_VIEW_CENTER, "menu_view_center"},
	{"ZoomOut", GDK_KEY_Insert, 0, ID_VIEW_ZOOMOUT, "menu_view_zoomout"},
	{"ZoomIn", GDK_KEY_Delete, 0, ID_VIEW_ZOOMIN, "menu_view_zoomin"},
	{"UpFloor", GDK_KEY_Prior, 0, ID_VIEW_UPFLOOR, "menu_view_upfloor"},
	{"DownFloor", GDK_KEY_Next, 0, ID_VIEW_DOWNFLOOR, "menu_view_downfloor"},
	{"ToggleClipper", GDK_KEY_X, 0, ID_VIEW_CLIPPER, "menu_view_clipper"},
	{"ToggleCrosshairs", GDK_KEY_X, 0x01, ID_VIEW_CROSSHAIR, "menu_view_crosshair"},
	{"TogTexLock", GDK_KEY_T, 0x01, ID_TOGGLE_LOCK, "menu_toggle_lock"},
	{"TogTexRotLock", GDK_KEY_R, 0x01, ID_TOGGLE_ROTATELOCK, "menu_toggle_rotatelock"},
	{"ToggleRealtime", GDK_KEY_R, 0x04, ID_VIEW_CAMERAUPDATE, "menu_view_cameraupdate"},
	{"EntityList", GDK_KEY_L, 0, ID_EDIT_ENTITYINFO, "menu_edit_entityinfo"},
	{"Preferences", GDK_KEY_P, 0, ID_PREFS, "menu_prefs"},
	{"ToggleCamera", GDK_KEY_C, 0x05, ID_TOGGLECAMERA, "menu_togglecamera"},
	{"ToggleConsole", GDK_KEY_O, 0, ID_TOGGLECONSOLE, "menu_toggleconsole"},
	{"ToggleView", GDK_KEY_V, 0x05, ID_TOGGLEVIEW, "menu_toggleview"},
	{"ToggleZ", GDK_KEY_Z, 0x05, ID_TOGGLEZ, "menu_togglez"},
	{"ConnectSelection", GDK_KEY_K, 0x04, ID_SELECTION_CONNECT, "menu_selection_connect"},
	{"Brush3Sided", GDK_KEY_3, 0x04, ID_BRUSH_3SIDED, "menu_brush_3sided"},
	{"Brush4Sided", GDK_KEY_4, 0x04, ID_BRUSH_4SIDED, "menu_brush_4sided"},
	{"Brush5Sided", GDK_KEY_5, 0x04, ID_BRUSH_5SIDED, "menu_brush_5sided"},
	{"Brush6Sided", GDK_KEY_6, 0x04, ID_BRUSH_6SIDED, "menu_brush_6sided"},
	{"Brush7Sided", GDK_KEY_7, 0x04, ID_BRUSH_7SIDED, "menu_brush_7sided"},
	{"Brush8Sided", GDK_KEY_8, 0x04, ID_BRUSH_8SIDED, "menu_brush_8sided"},
	{"Brush9Sided", GDK_KEY_9, 0x04, ID_BRUSH_9SIDED, "menu_brush_9sided"},
	{"MatrixTranspose", GDK_KEY_M, 0x05, ID_CURVE_MATRIX_TRANSPOSE, "menu_curve_matrix_transpose"},
	{"MakeDetail", GDK_KEY_M, 0x04, ID_SELECTION_MAKE_DETAIL, "menu_selection_make_detail"},
	{"MapInfo", GDK_KEY_M, 0, ID_EDIT_MAPINFO, "menu_edit_mapinfo"},
	{"NextLeakSpot", GDK_KEY_K, 0x05, ID_MISC_NEXTLEAKSPOT, "menu_misc_nextleakspot"},
	{"PrevLeakSpot", GDK_KEY_L, 0x05, ID_MISC_PREVIOUSLEAKSPOT, "menu_misc_previousleakspot"},
	{"FileOpen", GDK_KEY_O, 0x04, ID_FILE_OPEN, "menu_file_open"},
	{"FileSave", GDK_KEY_S, 0x04, ID_FILE_SAVE, "menu_file_save"},
	//% {"Exit", GDK_KEY_X, 0x04, ID_FILE_EXIT, "menu_file_exit"}, // ydnar: Ctrl+X should be cut
	{"CenterXYView", GDK_KEY_Tab, 0x05, ID_VIEW_CENTERVIEW, "menu_view_centerview"},
	{"NextView", GDK_KEY_Tab, 0x04, ID_VIEW_NEXTVIEW, "menu_view_nextview"},
	{"ClipSelected", GDK_KEY_Return, 0x00, ID_CLIP_SELECTED, "menu_clip_selected"},
	{"SplitSelected", GDK_KEY_Return, 0x01, ID_SPLIT_SELECTED, "menu_split_selected"},
	{"FlipClip", GDK_KEY_Return, 0x04, ID_FLIP_CLIP, "menu_flip_clip"},
	{"MouseRotate", GDK_KEY_R, 0x00, ID_SELECT_MOUSEROTATE, "menu_select_mouserotate"},
	{"Copy", GDK_KEY_C, 0x04, ID_EDIT_COPYBRUSH, "menu_edit_copybrush"},
	{"Paste", GDK_KEY_V, 0x04, ID_EDIT_PASTEBRUSH, "menu_edit_pastebrush"},
	{"PasteToCamera", GDK_KEY_V, RAD_ALT, ID_EDIT_PASTEBRUSHTOCAMERA, "menu_edit_pastebrushtocamera"},
	{"Undo", GDK_KEY_Z, 0x04, ID_EDIT_UNDO, "menu_edit_undo"},
	{"Redo", GDK_KEY_Y, 0x04, ID_EDIT_REDO, "menu_edit_redo"},
	{"ZZoomOut", GDK_KEY_Insert, 0x04, ID_VIEW_ZZOOMOUT, "menu_view_zzoomout"},
	{"ZZoomIn", GDK_KEY_Delete, 0x04, ID_VIEW_ZZOOMIN, "menu_view_zzoomin"},
	{"TexRotateClock", GDK_KEY_Next, 0x01, ID_SELECTION_TEXTURE_ROTATECLOCK, "menu_selection_texture_rotateclock"},
	{"TexRotateCounter", GDK_KEY_Prior, 0x01, ID_SELECTION_TEXTURE_ROTATECOUNTER, "menu_selection_texture_rotatecounter"},
	{"TexScaleUp", GDK_KEY_Up, 0x04, ID_SELECTION_TEXTURE_SCALEUP, "menu_selection_texture_scaleup"},
	{"TexScaleDown", GDK_KEY_Down, 0x04, ID_SELECTION_TEXTURE_SCALEDOWN, "menu_selection_texture_scaledown"},
	{"TexShiftLeft", GDK_KEY_Left, 0x01, ID_SELECTION_TEXTURE_SHIFTLEFT, "menu_selection_texture_shiftleft"},
	{"TexShiftRight", GDK_KEY_Right, 0x01, ID_SELECTION_TEXTURE_SHIFTRIGHT, "menu_selection_texture_shiftright"},
	{"TexShiftUp", GDK_KEY_Up, 0x01, ID_SELECTION_TEXTURE_SHIFTUP, "menu_selection_texture_shiftup"},
	{"TexShiftDown", GDK_KEY_Down, 0x01, ID_SELECTION_TEXTURE_SHIFTDOWN, "menu_selection_texture_shiftdown"},
	{"GridDown", GDK_KEY_bracketleft, 0x00, ID_GRID_PREV, "menu_grid_prev"},
	{"GridUp", GDK_KEY_bracketright, 0x00, ID_GRID_NEXT, "menu_grid_next"},
	{"TexScaleLeft", GDK_KEY_Left, 0x04, ID_SELECTION_TEXTURE_SCALELEFT, "menu_selection_texture_scaleleft"},
	{"TexScaleRight", GDK_KEY_Right, 0x04, ID_SELECTION_TEXTURE_SCALERIGHT, "menu_selection_texture_scaleright"},
	{"CubicClipZoomOut", GDK_KEY_bracketright, 0x04, ID_VIEW_CUBEOUT, "menu_view_cubeout"},
	{"CubicClipZoomIn", GDK_KEY_bracketleft, 0x04, ID_VIEW_CUBEIN, "menu_view_cubein"},
	{"ToggleCubicClip", GDK_KEY_backslash, 0x04, ID_VIEW_CUBICCLIPPING, "menu_view_cubicclipping"},
	{"MoveSelectionDOWN", GDK_KEY_KP_Subtract, 0x00, ID_SELECTION_MOVEDOWN, "menu_selection_movedown"},
	{"MoveSelectionUP", GDK_KEY_KP_Add, 0x00, ID_SELECTION_MOVEUP, "menu_selection_moveup"},
	{"DumpSelectedBrush", GDK_KEY_D, 0x01, ID_SELECTION_PRINT, "menu_selection_print"},
	{"ToggleSizePaint", GDK_KEY_Q, 0x00, ID_SELECTION_TOGGLESIZEPAINT, "menu_selection_togglesizepaint"},
	{"SelectNudgeLeft", GDK_KEY_Left, 0x02, ID_SELECTION_SELECT_NUDGELEFT, "menu_selection_select_nudgeleft"},
	{"SelectNudgeRight", GDK_KEY_Right, 0x02, ID_SELECTION_SELECT_NUDGERIGHT, "menu_selection_select_nudgeright"},
	{"SelectNudgeUp", GDK_KEY_Up, 0x02, ID_SELECTION_SELECT_NUDGEUP, "menu_selection_select_nudgeup"},
	{"CycleCapTexturePatch", GDK_KEY_N, 0x05, ID_CURVE_CYCLECAP, "menu_curve_cyclecap"},
	{"NaturalizePatch", GDK_KEY_N, 0x04, ID_PATCH_NATURALIZE, "menu_patch_naturalize"},
	{"SnapToGrid", GDK_KEY_G, 0x04, ID_SELECT_SNAPTOGRID, "menu_select_snaptogrid"},
	{"ShowAllTextures", GDK_KEY_A, 0x04, ID_TEXTURES_SHOWALL, "menu_textures_showall"},
	{"SelectAllOfType", GDK_KEY_A, 0x01, ID_SELECT_ALL, "menu_select_all"},
	{"CapCurrentCurve", GDK_KEY_C, 0x01, ID_CURVE_CAP, "menu_curve_cap"},
	{"MakeStructural", GDK_KEY_S, 0x05, ID_SELECTION_MAKE_STRUCTURAL, "menu_selection_make_structural"},
	{"RegionSetSelection", GDK_KEY_R, 0x05, ID_REGION_SETSELECTION, "menu_region_setselection"},
	{"ShowInUse", GDK_KEY_U, 0, ID_TEXTURES_SHOWINUSE, "menu_textures_showinuse"},
	{"InvertSelection", GDK_KEY_I, 0, ID_SELECTION_INVERT, "menu_selection_invert"},
	{"Sleep", GDK_KEY_P, 0x05, ID_FILE_SLEEP, "menu_file_sleep"},
	{"SimplePatchMesh", GDK_KEY_P, 0x01, ID_CURVE_SIMPLEPATCHMESH, "menu_simplepatchmesh"},
	{"FilterWorldBrushes", '1', RAD_ALT, ID_FILTER_WORLD, "menu_filter_world"},
	{"FilterWorldBrushes", GDK_KEY_1, RAD_ALT, ID_FILTER_WORLD, "menu_filter_world"},
	{"FilterEntities", '2', RAD_ALT, ID_FILTER_ENTITIES, "menu_filter_entities"},
	{"FilterEntities", GDK_KEY_2, RAD_ALT, ID_FILTER_ENTITIES, "menu_filter_entities"},
	{"FilterAreaportals", '3', RAD_ALT, ID_FILTER_AREAPORTALS, "menu_filter_areaportals"},
	{"FilterAreaportals", GDK_KEY_3, RAD_ALT, ID_FILTER_AREAPORTALS, "menu_filter_areaportals"},
	{"FilterTranslucent", '4', RAD_ALT, ID_FILTER_TRANSLUCENT, "menu_filter_translucent"},
	{"FilterTranslucent", GDK_KEY_4, RAD_ALT, ID_FILTER_TRANSLUCENT, "menu_filter_translucent"},
	{"FilterLiquids", '5', RAD_ALT, ID_FILTER_LIQUIDS, "menu_filter_liquids"},
	{"FilterLiquids", GDK_KEY_5, RAD_ALT, ID_FILTER_LIQUIDS, "menu_filter_liquids"},
	{"FilterCaulk", '6', RAD_ALT, ID_FILTER_CAULK, "menu_filter_caulk"},
	{"FilterCaulk", GDK_KEY_6, RAD_ALT, ID_FILTER_CAULK, "menu_filter_caulk"},
	{"FilterClips", '7', RAD_ALT, ID_FILTER_CLIPS, "menu_filter_clips"},
	{"FilterClips", GDK_KEY_7, RAD_ALT, ID_FILTER_CLIPS, "menu_filter_clips"},
	{"FilterBotClips", GDK_KEY_M, RAD_ALT, ID_FILTER_BOTCLIPS, "menu_filter_botclips"},
	{"FilterPaths", '8', RAD_ALT, ID_FILTER_PATHS, "menu_filter_paths"},
	{"FilterPaths", GDK_KEY_8, RAD_ALT, ID_FILTER_PATHS, "menu_filter_paths"},
	{"FilterClusterportals", '9', RAD_ALT, ID_FILTER_CLUSTERPORTALS, "menu_filter_clusterportals"},
	{"FilterClusterportals", GDK_KEY_9, RAD_ALT, ID_FILTER_CLUSTERPORTALS, "menu_filter_clusterportals"},
	{"FilterLights", '0', RAD_ALT, ID_FILTER_LIGHTS, "menu_filter_lights"},
	{"FilterLights", GDK_KEY_0, RAD_ALT, ID_FILTER_LIGHTS, "menu_filter_lights"},
	{"FilterPatches", GDK_KEY_P, RAD_CONTROL, ID_FILTER_PATCHES, "menu_filter_patches"},
	{"FilterDetails", GDK_KEY_D, RAD_CONTROL, ID_FILTER_DETAILS, "menu_filter_details"},
	{"FilterStructural", GDK_KEY_D, RAD_CONTROL | RAD_SHIFT, ID_FILTER_STRUCTURAL, "menu_filter_structural"},
	{"FilterHintsSkips", GDK_KEY_H, RAD_CONTROL, ID_FILTER_HINTSSKIPS, "menu_filter_hintsskips"},
	{"FilterModels", GDK_KEY_M, RAD_SHIFT, ID_FILTER_MODELS, "menu_filter_models"},
	{"FilterTriggers", GDK_KEY_T, RAD_CONTROL | RAD_SHIFT, ID_FILTER_TRIGGERS, "menu_filter_triggers"},
	{"LoadPointfile", GDK_KEY_L, RAD_SHIFT, ID_FILE_POINTFILE, "menu_load_pointfile"},
	{"TextureWindowScaledown", GDK_KEY_Insert, RAD_ALT, ID_TEXTUREWINDOW_SCALEDOWN, "menu_texturewindow_scaledown"},
	{"TextureWindowScaleup", GDK_KEY_Delete, RAD_ALT, ID_TEXTUREWINDOW_SCALEUP, "menu_texturewindow_scaleup"},
	{"Help", GDK_KEY_F1, 0, ID_HELP, "menu_help"},
};

int g_nCommandCount = sizeof( g_Commands ) / sizeof( SCommandInfo );

SKeyInfo g_Keys[] =
{
	{"Space", GDK_KEY_space},
	{"Backspace", GDK_KEY_BackSpace},
	{"Escape", GDK_KEY_Escape},
	{"End", GDK_KEY_End},
	{"Insert", GDK_KEY_Insert},
	{"Delete", GDK_KEY_Delete},
	{"PageUp", GDK_KEY_Prior},
	{"PageDown", GDK_KEY_Next},
	{"Up", GDK_KEY_Up},
	{"Down", GDK_KEY_Down},
	{"Left", GDK_KEY_Left},
	{"Right", GDK_KEY_Right},
	{"F1", GDK_KEY_F1},
	{"F2", GDK_KEY_F2},
	{"F3", GDK_KEY_F3},
	{"F4", GDK_KEY_F4},
	{"F5", GDK_KEY_F5},
	{"F6", GDK_KEY_F6},
	{"F7", GDK_KEY_F7},
	{"F8", GDK_KEY_F8},
	{"F9", GDK_KEY_F9},
	{"F10", GDK_KEY_F10},
	{"F11", GDK_KEY_F11},
	{"F12", GDK_KEY_F12},
	{"Tab", GDK_KEY_Tab},
	{"Return", GDK_KEY_Return},
	{"Comma", GDK_KEY_comma},
	{"Period", GDK_KEY_period},
	{"Plus", GDK_KEY_KP_Add},
	{"Multiply", GDK_KEY_multiply},
	{"Subtract", GDK_KEY_KP_Subtract},
	{"NumPad0", GDK_KEY_KP_0},
	{"NumPad1", GDK_KEY_KP_1},
	{"NumPad2", GDK_KEY_KP_2},
	{"NumPad3", GDK_KEY_KP_3},
	{"NumPad4", GDK_KEY_KP_4},
	{"NumPad5", GDK_KEY_KP_5},
	{"NumPad6", GDK_KEY_KP_6},
	{"NumPad7", GDK_KEY_KP_7},
	{"NumPad8", GDK_KEY_KP_8},
	{"NumPad9", GDK_KEY_KP_9},
	{"[", GDK_KEY_braceleft},
	{"]", GDK_KEY_braceright},
	{"\\", GDK_KEY_backslash},
	{"Home", GDK_KEY_Home}
};

int g_nKeyCount = sizeof( g_Keys ) / sizeof( SKeyInfo );

// =============================================================================
// global functions

void WINAPI Sys_UpdateWindows( int nBits ){
	g_nUpdateBits |= nBits;
}

// =============================================================================
// Static functions

// Gef: Separate handling for keyup events
void HandleKeyUp( GtkWidget *widget, gpointer data ){
	int id = GPOINTER_TO_INT( data );
#ifdef DBG_KBD
	Sys_Printf( "HandleKeyUp: %d\n", id );
#endif

	if ( g_bIgnoreCommands ) {
		return;
	}

	switch ( id )
	{
	case ID_CAMERA_FORWARD: g_pParentWnd->OnCameraForward( FALSE ); break;
	case ID_CAMERA_BACK: g_pParentWnd->OnCameraBack( FALSE ); break;
	case ID_CAMERA_LEFT: g_pParentWnd->OnCameraLeft( FALSE ); break;
	case ID_CAMERA_RIGHT: g_pParentWnd->OnCameraRight( FALSE ); break;
	case ID_CAMERA_STRAFELEFT: g_pParentWnd->OnCameraStrafeleft( FALSE ); break;
	case ID_CAMERA_STRAFERIGHT: g_pParentWnd->OnCameraStraferight( FALSE ); break;
	}
}

gint HandleCommand( GtkWidget *widget, gpointer data ){
	int id = GPOINTER_TO_INT( data );
#ifdef DBG_KBD
	Sys_Printf( "HandleCommand %d\n", id );
#endif

	if ( g_bIgnoreCommands ) {
#ifdef DBG_KBD
		Sys_Printf( "g_bIgnoreCommands %d, returning FALSE\n", g_bIgnoreCommands );
#endif
		return FALSE;
	}

	if ( id >= CMD_TEXTUREWAD && id <= CMD_TEXTUREWAD_END ) {
		g_pParentWnd->OnTextureWad( id );
	}
	else if ( id >= CMD_BSPCOMMAND && id <= CMD_BSPCOMMAND_END ) {
		g_pParentWnd->OnBspCommand( id );
	}
	else if ( id >= ID_FILE_RECENT1 && id <= ID_FILE_RECENT4 ) {
		g_pParentWnd->OnMru( id );
	}
	else if ( id >= ID_VIEW_NEAREST && id <= ID_TEXTURES_FLATSHADE ) {
		if ( gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM( widget  ) ) ) {
			g_pParentWnd->OnViewNearest( id );
		}
	}
	else if ( id >= ID_GRID_025 && id <= ID_GRID_256 ) {
		g_pParentWnd->OnGrid( id );
	}
	else if ( id >= ID_PLUGIN_START && id <= ID_PLUGIN_END ) {
		const char *str;
		str = gtk_menu_item_get_label( GTK_MENU_ITEM( widget ) );
		g_pParentWnd->OnPlugIn( id, str );
	}
	else if ( id >= ID_ENTITY_START && id <= ID_ENTITY_END ) {
		const char *str;

		if( GTK_IS_MENU_ITEM( widget ) ) {
			GtkWidget *label = GTK_WIDGET( g_object_get_data( G_OBJECT( widget ), "classname-label" ) );
			if( label )
			{
				str = gtk_label_get_text( GTK_LABEL( label ) );	
			} else
			{
				str = gtk_menu_item_get_label( GTK_MENU_ITEM( widget ) );
			}
		} else if( GTK_IS_LABEL( widget ) ) {
			str = gtk_label_get_text( GTK_LABEL( widget ) );
		} else {
			str = gtk_label_get_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN( widget ) ) ) );
		}
		g_pParentWnd->ActiveXY()->OnEntityCreate( str );
	}
	else{ switch ( id )
		  {
		  case ID_FILE_NEW: g_pParentWnd->OnFileNew(); break;
		  case ID_FILE_SLEEP: g_pParentWnd->OnSleep(); break;
		  case ID_FILE_OPEN: g_pParentWnd->OnFileOpen(); break;
		  case ID_FILE_SAVE: g_pParentWnd->OnFileSave(); break;
		  case ID_FILE_SAVEAS: g_pParentWnd->OnFileSaveas(); break;
		  case ID_FILE_EXPORTMAP: g_pParentWnd->OnFileExportmap(); break;
		  case ID_FILE_SAVEREGION: g_pParentWnd->OnFileSaveregion(); break;
		  case ID_FILE_NEWPROJECT: g_pParentWnd->OnFileNewproject(); break;
		  case ID_FILE_LOADPROJECT: g_pParentWnd->OnFileLoadproject(); break;
		  case ID_FILE_PROJECTSETTINGS: g_pParentWnd->OnFileProjectsettings(); break;
		  case ID_FILE_POINTFILE: g_pParentWnd->OnFilePointfile(); break;
		  case ID_FILE_CHECKUPDATE: g_pParentWnd->OnFileCheckUpdate(); break;
		  case ID_FILE_EXIT: g_pParentWnd->OnFileExit(); break;
		  case ID_FILE_IMPORTMAP: g_pParentWnd->OnFileImportmap(); break;
		  case ID_EDIT_UNDO: g_pParentWnd->OnEditUndo(); break;
		  case ID_EDIT_REDO: g_pParentWnd->OnEditRedo(); break;
		  case ID_EDIT_COPYBRUSH: g_pParentWnd->OnEditCopybrush(); break;
		  case ID_EDIT_PASTEBRUSH: g_pParentWnd->OnEditPastebrush(); break;
		  case ID_EDIT_PASTEBRUSHTOCAMERA: g_pParentWnd->OnEditPastebrushToCamera(); break;
		  case ID_SELECTION_DELETE: g_pParentWnd->OnSelectionDelete(); break;
		  case ID_EDIT_MAPINFO: g_pParentWnd->OnEditMapinfo(); break;
		  case ID_EDIT_ENTITYINFO: g_pParentWnd->OnEditEntityinfo(); break;
		  case ID_BRUSH_SCRIPTS: g_pParentWnd->OnBrushScripts(); break;
		  case ID_EDIT_LOADPREFAB: g_pParentWnd->OnEditLoadprefab(); break;
		  case ID_EDIT_SAVEPREFAB: g_pParentWnd->OnEditSaveprefab(); break;
		  case ID_PREFS: g_pParentWnd->OnPrefs(); break;
		  case ID_TOGGLECAMERA: g_pParentWnd->OnTogglecamera(); break;
		  case ID_TOGGLECONSOLE: g_pParentWnd->OnToggleconsole(); break;
		  case ID_VIEW_ENTITY: g_pParentWnd->OnViewEntity(); break;
		  case ID_VIEW_GROUPS: g_pParentWnd->OnViewGroups(); break;
		  case ID_SELECT_FUNC_GROUP: g_pParentWnd->OnSelectFuncGroup(); break;
		  case ID_TOGGLEVIEW: g_pParentWnd->OnToggleview(); break;
		  case ID_TOGGLEVIEW_YZ: g_pParentWnd->OnToggleviewYz(); break;
		  case ID_TOGGLEVIEW_XZ: g_pParentWnd->OnToggleviewXz(); break;
		  case ID_TOGGLEZ: g_pParentWnd->OnTogglez(); break;
		  case ID_VIEW_CENTER: g_pParentWnd->OnViewCenter(); break;
		  case ID_VIEW_UPFLOOR: g_pParentWnd->OnViewUpfloor(); break;
		  case ID_VIEW_DOWNFLOOR: g_pParentWnd->OnViewDownfloor(); break;
		  case ID_VIEW_CENTERVIEW: g_pParentWnd->OnViewCenterview(); break;
		  case ID_VIEW_NEXTVIEW: g_pParentWnd->OnViewNextview(); break;
		  case ID_VIEW_XY: g_pParentWnd->OnViewXy(); break;
		  case ID_VIEW_SIDE: g_pParentWnd->OnViewSide(); break;
		  case ID_VIEW_FRONT: g_pParentWnd->OnViewFront(); break;
		  case ID_VIEW_100: g_pParentWnd->OnView100(); break;
		  case ID_VIEW_ZOOMIN: g_pParentWnd->OnViewZoomin(); break;
		  case ID_VIEW_ZOOMOUT: g_pParentWnd->OnViewZoomout(); break;
		  case ID_VIEW_Z100: g_pParentWnd->OnViewZ100(); break;
		  case ID_VIEW_ZZOOMIN: g_pParentWnd->OnViewZzoomin(); break;
		  case ID_VIEW_ZZOOMOUT: g_pParentWnd->OnViewZzoomout(); break;
		  case ID_VIEW_CUBEIN: g_pParentWnd->OnViewCubein(); break;
		  case ID_VIEW_CUBEOUT: g_pParentWnd->OnViewCubeout(); break;
		  case ID_VIEW_SHOWNAMES: g_pParentWnd->OnViewShownames(); break;
		  case ID_VIEW_SHOWBLOCKS: g_pParentWnd->OnViewShowblocks(); break;
		  case ID_VIEW_SHOWCOORDINATES: g_pParentWnd->OnViewShowcoordinates(); break;
		  case ID_VIEW_SHOWOUTLINE: g_pParentWnd->OnViewShowOutline(); break;
		  case ID_VIEW_SHOWAXES: g_pParentWnd->OnViewShowAxes(); break;
		  case ID_VIEW_SHOWWORKZONE: g_pParentWnd->OnViewShowWorkzone(); break;
		  case ID_VIEW_SHOWANGLES: g_pParentWnd->OnViewShowAngles(); break;
		  case ID_VIEW_HIDESHOW_HIDESELECTED: g_pParentWnd->OnViewHideshowHideselected(); break;
		  case ID_VIEW_HIDESHOW_SHOWHIDDEN: g_pParentWnd->OnViewHideshowShowhidden(); break;
		  case ID_VIEW_ENTITIESAS_BOUNDINGBOX:
		  case ID_VIEW_ENTITIESAS_WIREFRAME:
		  case ID_VIEW_ENTITIESAS_SELECTEDWIREFRAME:
		  case ID_VIEW_ENTITIESAS_SELECTEDSKINNED:
		  case ID_VIEW_ENTITIESAS_SKINNED:
		  case ID_VIEW_ENTITIESAS_SKINNEDANDBOXED:
			  g_pParentWnd->OnEntitiesSetViewAs( id );
			  break;
		  case ID_VIEW_CUBICCLIPPING: g_pParentWnd->OnViewCubicclipping(); break;
		  case ID_VIEW_OPENGLLIGHTING: g_pParentWnd->OnViewOpengllighting(); break;
		  case ID_SELECTION_DRAGEDGES: g_pParentWnd->OnSelectionDragedges(); break;
		  case ID_SELECTION_DRAGVERTECIES: g_pParentWnd->OnSelectionDragvertecies(); break;
		  case ID_SELECTION_CLONE: g_pParentWnd->OnSelectionClone(); break;
		  case ID_SELECTION_DESELECT: g_pParentWnd->OnSelectionDeselect(); break;
		  case ID_BRUSH_FLIPX: g_pParentWnd->OnBrushFlipx(); break;
		  case ID_BRUSH_FLIPY: g_pParentWnd->OnBrushFlipy(); break;
		  case ID_BRUSH_FLIPZ: g_pParentWnd->OnBrushFlipz(); break;
		  case ID_BRUSH_ROTATEX: g_pParentWnd->OnBrushRotatex(); break;
		  case ID_BRUSH_ROTATEY: g_pParentWnd->OnBrushRotatey(); break;
		  case ID_BRUSH_ROTATEZ: g_pParentWnd->OnBrushRotatez(); break;
		  case ID_SELECTION_ARBITRARYROTATION: g_pParentWnd->OnSelectionArbitraryrotation(); break;
		  case ID_SELECT_SCALE: g_pParentWnd->OnSelectScale(); break;
		  case ID_SELECTION_MAKEHOLLOW: g_pParentWnd->OnSelectionMakehollow(); break;
		  case ID_SELECTION_MAKEHOLLOW_TOUCH: g_pParentWnd->OnSelectionMakehollowTouch(); break;
		  case ID_SELECTION_CSGSUBTRACT: g_pParentWnd->OnSelectionCsgsubtract(); break;
		  case ID_SELECTION_CSGMERGE: g_pParentWnd->OnSelectionCsgmerge(); break;
		  case ID_SELECTION_NOOUTLINE: g_pParentWnd->OnSelectionNoOutline(); break;
		  case ID_SELECTION_OUTLINESTYLE: g_pParentWnd->OnSelectionOutlineStyle(); break;
		  case ID_SELECTION_SELECTCOMPLETETALL: g_pParentWnd->OnSelectionSelectcompletetall(); break;
		  case ID_SELECTION_SELECTTOUCHING: g_pParentWnd->OnSelectionSelecttouching(); break;
		  case ID_SELECTION_SELECTPARTIALTALL: g_pParentWnd->OnSelectionSelectpartialtall(); break;
		  case ID_SELECTION_SELECTINSIDE: g_pParentWnd->OnSelectionSelectinside(); break;
		  case ID_SELECTION_SELECT_NUDGELEFT: g_pParentWnd->OnSelectionSelectNudgeleft(); break;
		  case ID_SELECTION_SELECT_NUDGERIGHT: g_pParentWnd->OnSelectionSelectNudgeright(); break;
		  case ID_SELECTION_SELECT_NUDGEUP: g_pParentWnd->OnSelectionSelectNudgeup(); break;
		  case ID_SELECTION_SELECT_NUDGEDOWN: g_pParentWnd->OnSelectionSelectNudgedown(); break;
		  case ID_VIEW_CLIPPER: g_pParentWnd->OnViewClipper(); break;
		  case ID_CLIP_SELECTED: g_pParentWnd->OnClipSelected(); break;
		  case ID_SPLIT_SELECTED: g_pParentWnd->OnSplitSelected(); break;
		  case ID_FLIP_CLIP: g_pParentWnd->OnFlipClip(); break;
		  case ID_SELECTION_CONNECT: g_pParentWnd->OnSelectionConnect(); break;
		  case ID_SELECTION_UNGROUPENTITY: g_pParentWnd->OnSelectionUngroupentity(); break;
		  case ID_SELECTION_MERGE: Select_MergeEntity(); break;
		  case ID_SELECTION_SEPERATE: Select_Seperate(); break;
		  case ID_SELECTION_MAKE_DETAIL: g_pParentWnd->OnSelectionMakeDetail(); break;
		  case ID_SELECTION_MAKE_STRUCTURAL: g_pParentWnd->OnSelectionMakeStructural(); break;
		  case ID_SNAPTOGRID: g_pParentWnd->OnSnaptogrid(); break;
		  case ID_TEXTURES_SHOWINUSE: g_pParentWnd->OnTexturesShowinuse(); break;
		  case ID_TEXTURES_SHOWALL: g_pParentWnd->OnTexturesShowall(); break;
		  case ID_TEXTURES_INSPECTOR: g_pParentWnd->OnTexturesInspector(); break;
		  case ID_TEXTURE_REPLACEALL: g_pParentWnd->OnTextureReplaceall(); break;
		  case ID_TOGGLE_LOCK: g_pParentWnd->OnToggleLock(); break;
		  case ID_TOGGLE_ROTATELOCK: g_pParentWnd->OnToggleRotatelock(); break;
		  case ID_TEXTURES_LOAD: g_pParentWnd->OnTexturesLoad(); break;
		  case ID_TEXTURES_RELOADSHADERS: g_pParentWnd->OnTexturesReloadshaders(); break;
		  case ID_TEXTURES_SHADERS_SHOW: g_pParentWnd->OnTexturesShadersShow(); break;
		  case ID_TEXTURES_EMPTYDIRS_HIDE: g_pParentWnd->OnTexturesEmptyDirsHide(); break;
		  case ID_TEXTURES_TEXTUREWINDOWSCALE_200:
		  case ID_TEXTURES_TEXTUREWINDOWSCALE_100:
		  case ID_TEXTURES_TEXTUREWINDOWSCALE_50:
		  case ID_TEXTURES_TEXTUREWINDOWSCALE_25:
		  case ID_TEXTURES_TEXTUREWINDOWSCALE_10:
			  g_pParentWnd->SetTextureScale( id );
			  break;
		  case ID_TEXTURES_LOADLIST: g_pParentWnd->OnTexturesLoadlist(); break;
		  case ID_TEXTURES_SHADERLISTONLY: g_pParentWnd->OnTexturesShaderlistonly(); break;
		  case ID_TEXTUREWINDOW_SCALEUP: g_pParentWnd->OnTexturewindowScaleup(); break;
		  case ID_TEXTUREWINDOW_SCALEDOWN: g_pParentWnd->OnTexturewindowScaledown(); break;
		  case ID_MISC_BENCHMARK: g_pParentWnd->OnMiscBenchmark(); break;
		  case ID_COLOR_SETORIGINAL: g_pParentWnd->OnColorSetoriginal(); break;
		  case ID_COLOR_SETQER: g_pParentWnd->OnColorSetqer(); break;
		  case ID_COLOR_SETBLACK: g_pParentWnd->OnColorSetblack(); break;
		  case ID_COLOR_SETYDNAR: g_pParentWnd->OnColorSetydnar(); break; /* ydnar */
		  case ID_TEXTUREBK: g_pParentWnd->OnTexturebk(); break;
		  case ID_COLORS_XYBK: g_pParentWnd->OnColorsXybk(); break;
		  case ID_COLORS_MAJOR: g_pParentWnd->OnColorsMajor(); break;
		  case ID_COLORS_MINOR: g_pParentWnd->OnColorsMinor(); break;
		  case ID_COLORS_GRIDTEXT: g_pParentWnd->OnColorsGridtext(); break;
		  case ID_COLORS_GRIDBLOCK: g_pParentWnd->OnColorsGridblock(); break;
		  case ID_COLORS_CAMERABACK: g_pParentWnd->OnColorsCameraBack(); break;
		  case ID_COLORS_BRUSH: g_pParentWnd->OnColorsBrush(); break;
		  case ID_COLORS_SELECTEDBRUSH: g_pParentWnd->OnColorsSelectedbrush(); break;
		  case ID_COLORS_SELECTEDBRUSH3D: g_pParentWnd->OnColorsSelectedbrush3D(); break;
		  case ID_COLORS_CLIPPER: g_pParentWnd->OnColorsClipper(); break;
		  case ID_COLORS_VIEWNAME: g_pParentWnd->OnColorsViewname(); break;
		  case ID_COLORS_DETAIL: g_pParentWnd->OnColorsDetail(); break;
		  case ID_MISC_GAMMA: g_pParentWnd->OnMiscGamma(); break;
		  case ID_MISC_FINDBRUSH: g_pParentWnd->OnMiscFindbrush(); break;
		  case ID_MISC_NEXTLEAKSPOT: g_pParentWnd->OnMiscNextleakspot(); break;
		  case ID_MISC_PREVIOUSLEAKSPOT: g_pParentWnd->OnMiscPreviousleakspot(); break;
		  case ID_MISC_PRINTXY: g_pParentWnd->OnMiscPrintxy(); break;
		  case ID_MISC_SELECTENTITYCOLOR: g_pParentWnd->OnMiscSelectentitycolor(); break;
		  case ID_CONVERTCURVES: g_pParentWnd->OnConvertcurves(); break;
		  case ID_REGION_OFF: g_pParentWnd->OnRegionOff(); break;
		  case ID_REGION_SETXY: g_pParentWnd->OnRegionSetxy(); break;
		  case ID_REGION_SETTALLBRUSH: g_pParentWnd->OnRegionSettallbrush(); break;
		  case ID_REGION_SETBRUSH: g_pParentWnd->OnRegionSetbrush(); break;
		  case ID_REGION_SETSELECTION: g_pParentWnd->OnRegionSetselection(); break;
		  case ID_BRUSH_3SIDED: g_pParentWnd->OnBrush3sided(); break;
		  case ID_BRUSH_4SIDED: g_pParentWnd->OnBrush4sided(); break;
		  case ID_BRUSH_5SIDED: g_pParentWnd->OnBrush5sided(); break;
		  case ID_BRUSH_6SIDED: g_pParentWnd->OnBrush6sided(); break;
		  case ID_BRUSH_7SIDED: g_pParentWnd->OnBrush7sided(); break;
		  case ID_BRUSH_8SIDED: g_pParentWnd->OnBrush8sided(); break;
		  case ID_BRUSH_9SIDED: g_pParentWnd->OnBrush9sided(); break;
		  case ID_BRUSH_ARBITRARYSIDED: g_pParentWnd->OnBrushArbitrarysided(); break;
		  case ID_BRUSH_MAKECONE: g_pParentWnd->OnBrushMakecone(); break;
		  case ID_BRUSH_PRIMITIVES_SPHERE: g_pParentWnd->OnBrushPrimitivesSphere(); break;
		  case ID_CURVE_PATCHTUBE: g_pParentWnd->OnCurvePatchtube(); break;
		  case ID_CURVE_PATCHDENSETUBE: g_pParentWnd->OnCurvePatchdensetube(); break;
		  case ID_CURVE_PATCHVERYDENSETUBE: g_pParentWnd->OnCurvePatchverydensetube(); break;
		  case ID_CURVE_PATCHSQUARE: g_pParentWnd->OnCurvePatchsquare(); break;
		  case ID_CURVE_PATCHENDCAP: g_pParentWnd->OnCurvePatchendcap(); break;
		  case ID_CURVE_PATCHBEVEL: g_pParentWnd->OnCurvePatchbevel(); break;
		  case ID_CURVE_MOREENDCAPSBEVELS_SQUAREBEVEL: g_pParentWnd->OnCurveMoreendcapsbevelsSquarebevel(); break;
		  case ID_CURVE_MOREENDCAPSBEVELS_SQUAREENDCAP: g_pParentWnd->OnCurveMoreendcapsbevelsSquareendcap(); break;
		  case ID_CURVE_PATCHCONE: g_pParentWnd->OnCurvePatchcone(); break;
		  case ID_CURVE_SIMPLEPATCHMESH: g_pParentWnd->OnCurveSimplepatchmesh(); break;
		  case ID_CURVE_INSERT_INSERTCOLUMN: g_pParentWnd->OnCurveInsertInsertcolumn(); break;
		  case ID_CURVE_INSERT_ADDCOLUMN: g_pParentWnd->OnCurveInsertAddcolumn(); break;
		  case ID_CURVE_INSERT_INSERTROW: g_pParentWnd->OnCurveInsertInsertrow(); break;
		  case ID_CURVE_INSERT_ADDROW: g_pParentWnd->OnCurveInsertAddrow(); break;
		  case ID_CURVE_DELETE_FIRSTCOLUMN: g_pParentWnd->OnCurveDeleteFirstcolumn(); break;
		  case ID_CURVE_DELETE_LASTCOLUMN: g_pParentWnd->OnCurveDeleteLastcolumn(); break;
		  case ID_CURVE_DELETE_FIRSTROW: g_pParentWnd->OnCurveDeleteFirstrow(); break;
		  case ID_CURVE_DELETE_LASTROW: g_pParentWnd->OnCurveDeleteLastrow(); break;
		  case ID_CURVE_NEGATIVE: g_pParentWnd->OnCurveNegative(); break;
		  case ID_CURVE_REDISPERSE_ROWS: g_pParentWnd->OnCurveRedisperseRows(); break;
		  case ID_CURVE_REDISPERSE_INTERMEDIATE_COLS: g_pParentWnd->OnCurveRedisperseIntermediateCols(); break;
		  case ID_CURVE_REDISPERSE_INTERMEDIATE_ROWS: g_pParentWnd->OnCurveRedisperseIntermediateRows(); break;
		  case ID_CURVE_MATRIX_TRANSPOSE: g_pParentWnd->OnCurveMatrixTranspose(); break;
		  case ID_CURVE_CAP: g_pParentWnd->OnCurveCap(); break;
		  case ID_CURVE_CYCLECAP: g_pParentWnd->OnCurveCyclecap(); break;
		  case ID_CURVE_OVERLAY_SET: g_pParentWnd->OnCurveOverlaySet(); break;
		  case ID_CURVE_OVERLAY_CLEAR: g_pParentWnd->OnCurveOverlayClear(); break;
		  case ID_CURVE_THICKEN: g_pParentWnd->OnCurveThicken(); break;
		  case ID_PLUGINS_REFRESH: g_pParentWnd->OnPluginsRefresh(); break;
		  case ID_HELP: g_pParentWnd->OnHelp(); break;
		  case ID_HELP_LINKS: g_pParentWnd->OnHelpLinks(); break;
		  case ID_HELP_BUGREPORT: g_pParentWnd->OnHelpBugreport(); break;
		  case ID_HELP_COMMANDLIST: g_pParentWnd->OnHelpCommandlist(); break;
		  case ID_HELP_ABOUT: g_pParentWnd->OnHelpAbout(); break;
		  case ID_DONTSELECTMODEL: g_pParentWnd->OnDontselectmodel(); break;
		  case ID_FILTER_AREAPORTALS: g_pParentWnd->OnFilterAreaportals(); break;
		  case ID_FILTER_CAULK: g_pParentWnd->OnFilterCaulk(); break;
		  case ID_FILTER_STRUCTURAL: g_pParentWnd->OnFilterStructural(); break;
		  case ID_FILTER_CLIPS: g_pParentWnd->OnFilterClips(); break;
		  case ID_FILTER_BOTCLIPS: g_pParentWnd->OnFilterBotClips(); break;
		  case ID_FILTER_DETAILS: g_pParentWnd->OnFilterDetails(); break;
		  case ID_FILTER_ENTITIES: g_pParentWnd->OnFilterEntities(); break;
		  case ID_FILTER_HINTSSKIPS: g_pParentWnd->OnFilterHintsskips(); break;
		  case ID_FILTER_LIGHTS: g_pParentWnd->OnFilterLights(); break;
		  case ID_FILTER_LIQUIDS: g_pParentWnd->OnFilterLiquids(); break;
		  case ID_FILTER_MODELS: g_pParentWnd->OnFilterModels(); break;
		  case ID_FILTER_PATCHES: g_pParentWnd->OnFilterPatches(); break;
		  case ID_FILTER_TRANSLUCENT: g_pParentWnd->OnFilterTranslucent(); break;
		  case ID_FILTER_TRIGGERS: g_pParentWnd->OnFilterTriggers(); break;
		  case ID_FILTER_WORLD: g_pParentWnd->OnFilterWorld(); break;
		  case ID_FILTER_PATHS: g_pParentWnd->OnFilterPaths(); break;
		  case ID_FILTER_CLUSTERPORTALS: g_pParentWnd->OnFilterClusterportals(); break;
		  case ID_FILTER_LIGHTGRID: g_pParentWnd->OnFilterLightgrid(); break;

		  case ID_POPUP_SELECTION: g_pParentWnd->OnPopupSelection(); break;
		  case ID_VIEW_CHANGE: g_pParentWnd->OnViewChange(); break;
		  case ID_TEXTURES_POPUP: g_pParentWnd->OnTexturesPopup(); break;
		  case ID_VIEW_CAMERATOGGLE: g_pParentWnd->ToggleCamera(); break;
		  case ID_VIEW_CAMERAUPDATE: g_pParentWnd->OnViewCameraupdate(); break;
		  case ID_SELECT_MOUSEROTATE: g_pParentWnd->OnSelectMouserotate(); break;
		  case ID_SELECT_MOUSESCALE: g_pParentWnd->OnSelectMousescale(); break;
		  case ID_SCALELOCKX: g_pParentWnd->OnScalelockx(); break;
		  case ID_SCALELOCKY: g_pParentWnd->OnScalelocky(); break;
		  case ID_SCALELOCKZ: g_pParentWnd->OnScalelockz(); break;
		  case ID_DONTSELECTCURVE: g_pParentWnd->OnDontselectcurve(); break;
		  case ID_PATCH_SHOWBOUNDINGBOX: g_pParentWnd->OnPatchToggleBox(); break;
		  case ID_PATCH_WIREFRAME: g_pParentWnd->OnPatchWireframe(); break;
		  case ID_PATCH_BEND: g_pParentWnd->OnPatchBend(); break;
		  case ID_PATCH_WELD: g_pParentWnd->OnPatchWeld(); break;
		  case ID_PATCH_DRILLDOWN: g_pParentWnd->OnPatchDrilldown(); break;
		  case ID_DROP_GROUP_NAME: g_pParentWnd->OnDropGroupName(); break;
		  case ID_DROP_GROUP_NEWGROUP: g_pParentWnd->OnDropGroupNewgroup(); break;
		  case ID_DROP_GROUP_REMOVE: g_pParentWnd->OnDropGroupRemove(); break;
		  case ID_SHOW_ENTITIES: g_pParentWnd->OnShowEntities(); break;

		  case IDC_BTN_FACEFIT: g_pParentWnd->OnFaceFit(); break;
		  case ID_VIEW_TEXTURE: g_pParentWnd->OnViewTexture(); break;
		  case ID_PATCH_INSPECTOR: g_pParentWnd->OnPatchInspector(); break;
		  case ID_CURVE_NEGATIVETEXTUREX: g_pParentWnd->OnCurveNegativeTextureX(); break;
		  case ID_CURVE_NEGATIVETEXTUREY: g_pParentWnd->OnCurveNegativeTextureY(); break;
		  case ID_CURVE_INSERTCOLUMN: g_pParentWnd->OnCurveInsertcolumn(); break;
		  case ID_CURVE_INSERTROW: g_pParentWnd->OnCurveInsertrow(); break;
		  case ID_CURVE_DELETECOLUMN: g_pParentWnd->OnCurveDeletecolumn(); break;
		  case ID_CURVE_DELETEROW: g_pParentWnd->OnCurveDeleterow(); break;
		  case ID_PATCH_TAB: g_pParentWnd->OnPatchTab(); break;
		  case ID_CAMERA_FORWARD: g_pParentWnd->OnCameraForward( TRUE ); break;
		  case ID_CAMERA_BACK: g_pParentWnd->OnCameraBack( TRUE ); break;
		  case ID_CAMERA_LEFT: g_pParentWnd->OnCameraLeft( TRUE ); break;
		  case ID_CAMERA_RIGHT: g_pParentWnd->OnCameraRight( TRUE ); break;
		  case ID_CAMERA_UP: g_pParentWnd->OnCameraUp(); break;
		  case ID_CAMERA_DOWN: g_pParentWnd->OnCameraDown(); break;
		  case ID_CAMERA_ANGLEUP: g_pParentWnd->OnCameraAngleup(); break;
		  case ID_CAMERA_ANGLEDOWN: g_pParentWnd->OnCameraAngledown(); break;
		  case ID_CAMERA_STRAFELEFT: g_pParentWnd->OnCameraStrafeleft( TRUE ); break;
		  case ID_CAMERA_STRAFERIGHT: g_pParentWnd->OnCameraStraferight( TRUE ); break;
		  case ID_GRID_TOGGLE: g_pParentWnd->OnGridToggle(); break;
		  case ID_VIEW_CONSOLE: g_pParentWnd->OnViewConsole(); break;
		  case ID_VIEW_CROSSHAIR: g_pParentWnd->OnViewCrosshair(); break;
		  case ID_SELECTION_TEXTURE_FIT: g_pParentWnd->OnSelectionTextureFit(); break;
		  case ID_SELECTION_TEXTURE_ROTATECLOCK: g_pParentWnd->OnSelectionTextureRotateclock(); break;
		  case ID_SELECTION_TEXTURE_ROTATECOUNTER: g_pParentWnd->OnSelectionTextureRotatecounter(); break;
		  case ID_SELECTION_TEXTURE_SCALEUP: g_pParentWnd->OnSelectionTextureScaleup(); break;
		  case ID_SELECTION_TEXTURE_SCALEDOWN: g_pParentWnd->OnSelectionTextureScaledown(); break;
		  case ID_SELECTION_TEXTURE_SHIFTLEFT: g_pParentWnd->OnSelectionTextureShiftleft(); break;
		  case ID_SELECTION_TEXTURE_SHIFTRIGHT: g_pParentWnd->OnSelectionTextureShiftright(); break;
		  case ID_SELECTION_TEXTURE_SHIFTUP: g_pParentWnd->OnSelectionTextureShiftup(); break;
		  case ID_SELECTION_TEXTURE_SHIFTDOWN: g_pParentWnd->OnSelectionTextureShiftdown(); break;
		  case ID_GRID_PREV: g_pParentWnd->OnGridPrev(); break;
		  case ID_GRID_NEXT: g_pParentWnd->OnGridNext(); break;
		  case ID_SELECTION_TEXTURE_SCALELEFT: g_pParentWnd->OnSelectionTextureScaleLeft(); break;
		  case ID_SELECTION_TEXTURE_SCALERIGHT: g_pParentWnd->OnSelectionTextureScaleRight(); break;
		  case ID_SELECTION_MOVEDOWN: g_pParentWnd->OnSelectionMovedown(); break;
		  case ID_SELECTION_MOVEUP: g_pParentWnd->OnSelectionMoveup(); break;
		  case ID_SELECTION_PRINT: g_pParentWnd->OnSelectionPrint(); break;
		  case ID_SELECTION_TOGGLESIZEPAINT: g_pParentWnd->OnSelectionTogglesizepaint(); break;
		  case ID_PATCH_NATURALIZE: g_pParentWnd->OnPatchNaturalize(); break;
		  case ID_SELECT_SNAPTOGRID: g_pParentWnd->OnSnapToGrid(); break;
		  case ID_SELECT_ALL: g_pParentWnd->OnSelectAll(); break;
		  case ID_SELECTION_INVERT: g_pParentWnd->OnSelectionInvert(); break;
		  case ID_TOGGLE_DETAIL: g_pParentWnd->OnToggleDetail(); break;
		  }}

	return TRUE;
}

static gint timer( gpointer data ){
	MainFrame *wnd = (MainFrame*)data;
	wnd->OnTimer();
	return TRUE;
}

static gint mainframe_delete( GtkWidget *widget, GdkEvent *event, gpointer data ){
	MainFrame *wnd = (MainFrame*)data;

	wnd->OnDelete();

	if ( ConfirmModified() ) {
		return FALSE;
	}

	g_qeglobals_gui.d_edit = NULL;

	return TRUE;
}

static void mainframe_destroy( GtkWidget *widget, gpointer data ){
	MainFrame *wnd = (MainFrame*)data;

	// avoid saving prefs when the app is minimized
	if ( g_pParentWnd->IsSleeping() ) {
		Sys_Printf( "Shutdown while sleeping, not saving prefs\n" );
		g_qeglobals.disable_ini = true;
	}

	// NOTE TTimo this is very clumsy, in MainFrame::OnDestroy we might call SavePrefs again
	//   we will do more stuff in OnDestroy for window position saving too, so I guess this call is still relevant?
	g_PrefsDlg.SavePrefs();

	wnd->OnDestroy();

	// shutdown modules
	// NOTE: I've decided to do this before SavePrefs in case we broadcast some shutdown info
	// and modules / plugins decide to save some stuff
	g_pParentWnd->GetPlugInMgr().Shutdown();

	delete wnd;

	QGL_Shutdown();
	g_PrefsDlg.Destroy();
	g_dlgSurface.Destroy();
	g_dlgFind.Destroy();
	g_PatchDialog.Destroy();

	gtk_main_quit();
}

static gint mainframe_keypress( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	unsigned int code = gdk_keyval_to_upper( event->keyval );

	if ( code == GDK_KEY_ISO_Left_Tab ) {
		code = GDK_KEY_Tab;
	}

#ifdef DBG_KBD
	Sys_Printf( "key: %d (keyval: %d) (ctrl: %d)\n", code, event->keyval, event->state & GDK_CONTROL_MASK );
#endif

	// return only if Texture Viewport  is in main window, otherwise if Tex viewport is in it's own window
	// the Filter GtkEntry won't release focus
	if ( g_pParentWnd->GetTexWnd()->m_pFilter == gtk_window_get_focus( GTK_WINDOW( widget ) ) ) {
		if ( gtk_widget_is_focus( g_pParentWnd->GetTexWnd()->m_pFilter ) ) {
			return FALSE;
		}
	}

#ifdef DBG_KBD
	Sys_Printf( "mainframe_keypress processing into a command\n" );
#endif
	for ( int i = 0; i < g_nCommandCount; i++ )
	{
		if ( g_Commands[i].m_nKey == code ) { // find a match?
			// check modifiers
			unsigned int nState = 0;
			if ( Sys_AltDown() ) {
				nState |= RAD_ALT;
			}
			if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
				nState |= RAD_CONTROL;
			}
			if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
				nState |= RAD_SHIFT;
			}
			if ( ( g_Commands[i].m_nModifiers & 0x7 ) == nState ) {
				HandleCommand( NULL, GINT_TO_POINTER( g_Commands[i].m_nCommand ) );
				g_signal_stop_emission_by_name( G_OBJECT( widget ), "key-press-event" );
				return FALSE;
			}
		}
	}

	return TRUE;
}

static gint mainframe_keyrelease( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	unsigned int code = gdk_keyval_to_upper( event->keyval );

	if ( gtk_accelerator_valid( event->keyval, (GdkModifierType)0 ) ) {
		return TRUE;
	}

	for ( int i = 0; i < g_nCommandCount; i++ )
	{
		if ( g_Commands[i].m_nKey == code ) { // find a match?
			if ( !g_Commands[i].m_nModifiers ) {
				// Gef: Only call the handler if it's a key that needs keyup events
				switch ( g_Commands[i].m_nCommand )
				{
				case ID_CAMERA_FORWARD:
				case ID_CAMERA_BACK:
				case ID_CAMERA_LEFT:
				case ID_CAMERA_RIGHT:
				case ID_CAMERA_STRAFELEFT:
				case ID_CAMERA_STRAFERIGHT:
				{
					HandleKeyUp( NULL, GINT_TO_POINTER( g_Commands[i].m_nCommand ) );
					g_signal_stop_emission_by_name( G_OBJECT( widget ), "key-release-event" );
				}

				}
				return FALSE;
			}
		}
	}

	return TRUE;
}


// =============================================================================
// Window creation functions

void AddMenuItem( GtkWidget* item, unsigned int id ){
	for ( int i = 0; i < g_nCommandCount; i++ )
		if ( g_Commands[i].m_nCommand == id ) {
			g_object_set_data( G_OBJECT( g_pParentWnd->m_pWidget ), g_Commands[i].m_strMenu, item );
			break;
		}
}

void MainFrame::handle_help_command( int id ){
	OpenURL( m_pWidget, mHelpURLs[id]->GetBuffer() );
}

/*!
   needed for hooking in Gtk+
 */
void HandleHelpCommand( GtkWidget *widget, gpointer data ){
	int id = GPOINTER_TO_INT( data );
	g_pParentWnd->handle_help_command( id );
}

void MainFrame::process_xlink( Str &FileName, const char *menu_name, const char *base_url, GtkWidget *menu, GtkAccelGroup *accel ){
	xmlDocPtr pDoc;
	pDoc = xmlParseFile( FileName.GetBuffer() );
	if ( pDoc ) {
		Sys_Printf( "Processing .xlink file '%s'\n", FileName.GetBuffer() );
		// create sub menu
		GtkWidget* menu_in_menu = create_menu_in_menu_with_mnemonic( menu, menu_name );
		// start walking the nodes, find the 'links' one
		xmlNodePtr pNode = pDoc->children;
		while ( pNode && strcmp( (const char*)pNode->name, "links" ) )
			pNode = pNode->next;
		if ( pNode ) {
			pNode = pNode->children;
			while ( pNode )
			{
				if ( !strcmp( (const char*)pNode->name, "item" ) ) {
					// process the URL
					Str *url;
					if ( strstr( (char *)xmlGetProp( pNode, (xmlChar *)"url" ), "http://" )
						|| strstr( (char *)xmlGetProp( pNode, (xmlChar *)"url" ), "https://" ) ) {
						// complete URL
						url = new Str;
						*url = (char *)xmlGetProp( pNode, (xmlChar *)"url" );
					}
					else
					{
						// relative URL
						url = new Str;
						*url = base_url;
						*url += (char *)xmlGetProp( pNode, (xmlChar *)"url" );
					}
					mHelpURLs.push_back( url );
					create_menu_item_with_mnemonic( menu_in_menu, (char *)xmlGetProp( pNode, (xmlChar *)"name" ), G_CALLBACK( HandleHelpCommand ), mHelpURLs.size() - 1 );
				}
				pNode = pNode->next;
			}
		}
		xmlFreeDoc( pDoc );
	}
	else
	{
		Sys_Printf( "'%s' not found / parse failed\n", FileName.GetBuffer() );
	}
}

void MainFrame::create_game_help_menu( GtkWidget *menu, GtkAccelGroup *accel ){
	Str FileName;
	list<CGameDescription *>::iterator iGame;

	// start in the global dir
	FileName = g_strAppPath;
	FileName += "global.xlink";
	process_xlink( FileName, "General", g_strAppPath.GetBuffer(), menu, accel );

	for ( iGame = g_PrefsDlg.mGamesDialog.mGames.begin(); iGame != g_PrefsDlg.mGamesDialog.mGames.end(); iGame++ )
	{
		FileName = ( *iGame )->mGameToolsPath;
		FileName += "game.xlink";
		process_xlink( FileName, ( *iGame )->mGameName, ( *iGame )->mGameToolsPath.GetBuffer(), menu, accel );
	}
}

void MainFrame::create_main_menu( GtkWidget *window, GtkWidget *vbox ){
	GtkWidget *menu_bar, *menu, *menu_in_menu, *menu_3, *item;
	GtkAccelGroup *accel;

	g_bIgnoreCommands++;
	accel = gtk_accel_group_new();
	global_accel = accel;
	gtk_window_add_accel_group( GTK_WINDOW( window ), accel );

	menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start( GTK_BOX( vbox ), menu_bar, FALSE, FALSE, 0 );
	gtk_widget_show( menu_bar );

	// File menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_File" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	create_menu_item_with_mnemonic( menu, _( "_New Map" ),
									G_CALLBACK( HandleCommand ), ID_FILE_NEW );
	menu_separator( menu );

	//++timo temporary experimental stuff for sleep mode..
	item = create_menu_item_with_mnemonic( menu, _( "_Sleep" ),
										   G_CALLBACK( HandleCommand ), ID_FILE_SLEEP );
	g_object_set_data( G_OBJECT( window ), "menu_file_sleep", item );
	menu_separator( menu );
	// end experimental

	item = create_menu_item_with_mnemonic( menu, _( "_Open..." ),
										   G_CALLBACK( HandleCommand ), ID_FILE_OPEN );
	g_object_set_data( G_OBJECT( window ), "menu_file_open", item );
	create_menu_item_with_mnemonic( menu, _( "_Import..." ), // Hydra: give it it's proper name
									G_CALLBACK( HandleCommand ), ID_FILE_IMPORTMAP );
	item = create_menu_item_with_mnemonic( menu, _( "_Save" ),
										   G_CALLBACK( HandleCommand ), ID_FILE_SAVE );
	g_object_set_data( G_OBJECT( window ), "menu_file_save", item );
	create_menu_item_with_mnemonic( menu, _( "Save _as..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_SAVEAS );
	create_menu_item_with_mnemonic( menu, _( "Save s_elected..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_EXPORTMAP );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "Save re_gion..." ),
										   G_CALLBACK( HandleCommand ), ID_FILE_SAVEREGION );
	g_object_set_data( G_OBJECT( window ), "menu_file_saveregion", item );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "New p_roject..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_NEWPROJECT );
	create_menu_item_with_mnemonic( menu, _( "Load _project..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_LOADPROJECT );
	create_menu_item_with_mnemonic( menu, _( "Pro_ject settings..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_PROJECTSETTINGS );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "_Pointfile..." ),
									G_CALLBACK( HandleCommand ), ID_FILE_POINTFILE );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "Recent Files" ),
										   G_CALLBACK( HandleCommand ), ID_FILE_RECENT1 );
	g_object_set_data( G_OBJECT( item ), "accel", accel );
	gtk_widget_set_sensitive( item, FALSE );
	MRU_AddWidget( item, 0 );
	item = create_menu_item_with_mnemonic( menu, "2",
										   G_CALLBACK( HandleCommand ), ID_FILE_RECENT2 );
	gtk_widget_hide( item );
	MRU_AddWidget( item, 1 );
	item = create_menu_item_with_mnemonic( menu, "3",
										   G_CALLBACK( HandleCommand ), ID_FILE_RECENT3 );
	gtk_widget_hide( item );
	MRU_AddWidget( item, 2 );
	item = create_menu_item_with_mnemonic( menu, "4",
										   G_CALLBACK( HandleCommand ), ID_FILE_RECENT4 );
	gtk_widget_hide( item );
	MRU_AddWidget( item, 3 );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "Check for GtkRadiant update (web)" ),
										   G_CALLBACK( HandleCommand ), ID_FILE_CHECKUPDATE );
	// disable, the functionality is no longer available
	gtk_widget_set_sensitive( item, FALSE );

	create_menu_item_with_mnemonic( menu, _( "E_xit" ),
									G_CALLBACK( HandleCommand ), ID_FILE_EXIT );

	// Edit menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Edit" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}
	item = create_menu_item_with_mnemonic( menu, _( "_Undo" ),
										   G_CALLBACK( HandleCommand ), ID_EDIT_UNDO );
	g_object_set_data( G_OBJECT( window ), "menu_edit_undo", item );
	item = create_menu_item_with_mnemonic( menu, _( "_Redo" ),
										   G_CALLBACK( HandleCommand ), ID_EDIT_REDO );
	g_object_set_data( G_OBJECT( window ), "menu_edit_redo", item );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "_Copy" ), G_CALLBACK( HandleCommand ), ID_EDIT_COPYBRUSH );
	item = create_menu_item_with_mnemonic( menu, _( "_Paste" ), G_CALLBACK( HandleCommand ), ID_EDIT_PASTEBRUSH );
	item = create_menu_item_with_mnemonic( menu, _( "P_aste To Camera" ), G_CALLBACK( HandleCommand ), ID_EDIT_PASTEBRUSHTOCAMERA );
	item = create_menu_item_with_mnemonic( menu, _( "_Delete" ), G_CALLBACK( HandleCommand ), ID_SELECTION_DELETE );
	g_object_set_data( G_OBJECT( window ), "menu_selection_delete", item );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Map Info..." ), G_CALLBACK( HandleCommand ), ID_EDIT_MAPINFO );
	create_menu_item_with_mnemonic( menu, _( "Entity Info..." ), G_CALLBACK( HandleCommand ), ID_EDIT_ENTITYINFO );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Brush Scripts..." ), G_CALLBACK( HandleCommand ), ID_BRUSH_SCRIPTS );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Load Pre_fab..." ), G_CALLBACK( HandleCommand ), ID_EDIT_LOADPREFAB );
	create_menu_item_with_mnemonic( menu, _( "Save Selection as Prefab..." ), G_CALLBACK( HandleCommand ), ID_EDIT_SAVEPREFAB );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Preferences..." ), G_CALLBACK( HandleCommand ), ID_PREFS );

	// View menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_View" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Toggle" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Camera View" ), G_CALLBACK( HandleCommand ), ID_TOGGLECAMERA );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Console View" ), G_CALLBACK( HandleCommand ), ID_TOGGLECONSOLE );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "Entity View" ), G_CALLBACK( HandleCommand ), ID_VIEW_ENTITY );
	g_object_set_data( G_OBJECT( window ), "menu_view_entity", item );
	//  create_menu_item_with_mnemonic (menu_in_menu, "Groups View", G_CALLBACK (HandleCommand), ID_VIEW_GROUPS);
	create_menu_item_with_mnemonic( menu_in_menu, _( "XY (Top)" ), G_CALLBACK( HandleCommand ), ID_TOGGLEVIEW );
	create_menu_item_with_mnemonic( menu_in_menu, _( "YZ (Side)" ), G_CALLBACK( HandleCommand ), ID_TOGGLEVIEW_YZ );
	create_menu_item_with_mnemonic( menu_in_menu, _( "XZ (Front)" ), G_CALLBACK( HandleCommand ), ID_TOGGLEVIEW_XZ );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Z View" ), G_CALLBACK( HandleCommand ), ID_TOGGLEZ );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "_Center" ), G_CALLBACK( HandleCommand ), ID_VIEW_CENTER );
	item = create_menu_item_with_mnemonic( menu, _( "_Center 2d" ), G_CALLBACK( HandleCommand ), ID_VIEW_CENTERVIEW );
	item = create_menu_item_with_mnemonic( menu, _( "_Up Floor" ), G_CALLBACK( HandleCommand ), ID_VIEW_UPFLOOR );
	item = create_menu_item_with_mnemonic( menu, _( "_Down Floor" ), G_CALLBACK( HandleCommand ), ID_VIEW_DOWNFLOOR );
	menu_separator( menu );
	item = create_menu_item_with_mnemonic( menu, _( "_Next (XY, YZ, XY)" ), G_CALLBACK( HandleCommand ), ID_VIEW_NEXTVIEW );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Layout" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "XY (Top)" ), G_CALLBACK( HandleCommand ), ID_VIEW_XY );
	create_menu_item_with_mnemonic( menu_in_menu, _( "YZ" ), G_CALLBACK( HandleCommand ), ID_VIEW_SIDE );
	create_menu_item_with_mnemonic( menu_in_menu, _( "XZ" ), G_CALLBACK( HandleCommand ), ID_VIEW_FRONT );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Zoom" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "_XY 100%" ), G_CALLBACK( HandleCommand ), ID_VIEW_100 );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "XY Zoom _In" ), G_CALLBACK( HandleCommand ), ID_VIEW_ZOOMIN );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "XY Zoom _Out" ), G_CALLBACK( HandleCommand ), ID_VIEW_ZOOMOUT );
	menu_separator( menu_in_menu );
	create_menu_item_with_mnemonic( menu_in_menu, _( "_Z 100%" ), G_CALLBACK( HandleCommand ), ID_VIEW_Z100 );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "Z Zoo_m In" ), G_CALLBACK( HandleCommand ), ID_VIEW_ZZOOMIN );
	g_object_set_data( G_OBJECT( window ), "menu_view_zzoomin", item );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "Z Zoom O_ut" ), G_CALLBACK( HandleCommand ), ID_VIEW_ZZOOMOUT );
	g_object_set_data( G_OBJECT( window ), "menu_view_zzoomout", item );
	menu_separator( menu_in_menu );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "Cubic Clip Zoom In" ), G_CALLBACK( HandleCommand ), ID_VIEW_CUBEIN );
	item = create_menu_item_with_mnemonic( menu_in_menu, _( "Cubic Clip Zoom Out" ), G_CALLBACK( HandleCommand ), ID_VIEW_CUBEOUT );
	menu_separator( menu );

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Show" ) );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show _Angles" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWANGLES, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showangles", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show _Names" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWNAMES, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_view_shownames", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show Blocks" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWBLOCKS, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showblocks", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show C_oordinates" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWCOORDINATES, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showcoordinates", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show Window Outline" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWOUTLINE, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showoutline", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show ZBuffered Outline" ), G_CALLBACK( HandleCommand ), ID_SELECTION_NOOUTLINE, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_selection_nooutline", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show Axes" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWAXES, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showaxes", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Show Workzone" ), G_CALLBACK( HandleCommand ), ID_VIEW_SHOWWORKZONE, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_showworkzone", item );

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Filter" ) );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "World" ), G_CALLBACK( HandleCommand ), ID_FILTER_WORLD, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Entities" ), G_CALLBACK( HandleCommand ), ID_FILTER_ENTITIES, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Areaportals" ), G_CALLBACK( HandleCommand ), ID_FILTER_AREAPORTALS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Translucent" ), G_CALLBACK( HandleCommand ), ID_FILTER_TRANSLUCENT, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Liquids" ), G_CALLBACK( HandleCommand ), ID_FILTER_LIQUIDS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Caulk" ), G_CALLBACK( HandleCommand ), ID_FILTER_CAULK, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Clips" ), G_CALLBACK( HandleCommand ), ID_FILTER_CLIPS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Paths" ), G_CALLBACK( HandleCommand ), ID_FILTER_PATHS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Clusterportals" ), G_CALLBACK( HandleCommand ), ID_FILTER_CLUSTERPORTALS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Lights" ), G_CALLBACK( HandleCommand ), ID_FILTER_LIGHTS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Structural" ), G_CALLBACK( HandleCommand ), ID_FILTER_STRUCTURAL, FALSE );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Lightgrid" ), G_CALLBACK( HandleCommand ), ID_FILTER_LIGHTGRID, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_filter_lightgrid", item );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Patches" ), G_CALLBACK( HandleCommand ), ID_FILTER_PATCHES, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Details" ), G_CALLBACK( HandleCommand ), ID_FILTER_DETAILS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Hints" ), G_CALLBACK( HandleCommand ), ID_FILTER_HINTSSKIPS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Models" ), G_CALLBACK( HandleCommand ), ID_FILTER_MODELS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Triggers" ), G_CALLBACK( HandleCommand ), ID_FILTER_TRIGGERS, FALSE );
	create_check_menu_item_with_mnemonic( menu_in_menu, _( "Botclips" ), G_CALLBACK( HandleCommand ), ID_FILTER_BOTCLIPS, FALSE );

	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Hide/Show" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Hide Selected" ),
									G_CALLBACK( HandleCommand ), ID_VIEW_HIDESHOW_HIDESELECTED );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Show Hidden" ),
									G_CALLBACK( HandleCommand ), ID_VIEW_HIDESHOW_SHOWHIDDEN );
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Entities as" ) );
	g_object_set_data( G_OBJECT( window ), "view_entitiesas_menu", menu_in_menu );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, NULL, _( "Bounding box" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_ENTITIESAS_BOUNDINGBOX,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_boundingbox", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Wireframe" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_ENTITIESAS_WIREFRAME,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_wireframe", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Selected Wireframe" ),
												 G_CALLBACK( HandleCommand ),ID_VIEW_ENTITIESAS_SELECTEDWIREFRAME,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_selectedwireframe", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Selected Skinned" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_ENTITIESAS_SELECTEDSKINNED,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_selectedskinned", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Skinned" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_ENTITIESAS_SKINNED,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_skinned", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Skinned and Boxed" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_ENTITIESAS_SKINNEDANDBOXED,FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_entitiesas_skinnedandboxed", item );
	menu_separator( menu );
	item = create_check_menu_item_with_mnemonic( menu, _( "Cubic Clipping" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_CUBICCLIPPING, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_view_cubicclipping", item );
	menu_separator( menu );
	item = create_check_menu_item_with_mnemonic( menu, _( "OpenGL Lighting" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_OPENGLLIGHTING, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_opengllighting", item );

	// Selection menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Selection" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Drag" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Drag _Edges" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_DRAGEDGES );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Drag _Vertices" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_DRAGVERTECIES );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "_Clone" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_CLONE );
	item = create_menu_item_with_mnemonic( menu, _( "Deselect" ),
										   G_CALLBACK( HandleCommand ), ID_SELECTION_DESELECT );
	item = create_menu_item_with_mnemonic( menu, _( "Invert" ),
										   G_CALLBACK( HandleCommand ), ID_SELECTION_INVERT );
#ifndef QUAKE3
	create_menu_item_with_mnemonic( menu, _( "_Delete" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_DELETE );
#endif
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Flip" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Flip _X" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_FLIPX );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Flip _Y" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_FLIPY );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Flip _Z" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_FLIPZ );
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Rotate" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Rotate X" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_ROTATEX );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Rotate Y" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_ROTATEY );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Rotate Z" ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_ROTATEZ );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Arbitrary rotation..." ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_ARBITRARYROTATION );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Scale..." ), G_CALLBACK( HandleCommand ), ID_SELECT_SCALE );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "CSG" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Make _Hollow Overlap" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_MAKEHOLLOW );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Make _Hollow Touch" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_MAKEHOLLOW_TOUCH );
	create_menu_item_with_mnemonic( menu_in_menu, _( "CSG _Subtract" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_CSGSUBTRACT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "CSG _Merge" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_CSGMERGE );
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Select" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Select Complete _Tall" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTCOMPLETETALL );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Select T_ouching" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTTOUCHING );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Select _Partial Tall" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTPARTIALTALL );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Select _Inside" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTINSIDE );
	create_menu_item_with_mnemonic( menu_in_menu, "Select Func _Group", G_CALLBACK( HandleCommand ), ID_SELECT_FUNC_GROUP );
#ifndef QUAKE3
	create_menu_item_with_mnemonic( menu_in_menu, _( "Nudge Left" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGELEFT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Nudge Right" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGERIGHT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Nudge Up" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGEUP );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Nudge Down" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGEDOWN );
#endif
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Clipper" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Toggle Clipper" ),
									G_CALLBACK( HandleCommand ), ID_VIEW_CLIPPER );
	menu_separator( menu_in_menu );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Clip selection" ),
									G_CALLBACK( HandleCommand ), ID_CLIP_SELECTED );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Split selection" ),
									G_CALLBACK( HandleCommand ), ID_SPLIT_SELECTED );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Flip Clip orientation" ),
									G_CALLBACK( HandleCommand ), ID_FLIP_CLIP );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Connect entities" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_CONNECT );
	create_menu_item_with_mnemonic( menu, _( "Ungroup entity" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_UNGROUPENTITY );
	create_menu_item_with_mnemonic( menu, _( "Make detail" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_MAKE_DETAIL );
	create_menu_item_with_mnemonic( menu, _( "Make structural" ),
									G_CALLBACK( HandleCommand ), ID_SELECTION_MAKE_STRUCTURAL );

	// BSP menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Bsp" ) );

	menu_separator( menu );
	g_object_set_data( G_OBJECT( window ), "menu_bsp", menu );

	// Grid menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Grid" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	item = create_radio_menu_item_with_mnemonic( menu, NULL, _( "Grid0.25" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_025, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_025", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid0.5" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_05, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_05", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid1" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_1, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_1", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid2" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_2, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_2", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid4" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_4, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_4", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid8" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_8, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_8", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid16" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_16, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_16", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid32" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_32, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_32", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid64" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_64, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_64", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid128" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_128, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_128", item );
	item = create_radio_menu_item_with_mnemonic( menu, item, _( "Grid256" ),
												 G_CALLBACK( HandleCommand ), ID_GRID_256, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_grid_256", item );
	menu_separator( menu );
	item = create_check_menu_item_with_mnemonic( menu, _( "Snap to grid" ),
												 G_CALLBACK( HandleCommand ), ID_SNAPTOGRID, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_snaptogrid", item );

	// Textures menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Textures" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	item = create_check_menu_item_with_mnemonic( menu, _( "Show In _Use" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_SHOWINUSE, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_showinuse", item );
	item = create_check_menu_item_with_mnemonic( menu, _( "Show _All" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_SHOWALL, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_showall", item );
	menu_separator( menu );
	item = create_check_menu_item_with_mnemonic( menu, _( "Show shaders" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_SHADERS_SHOW, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_shaders_show", item );
	item = create_menu_item_with_mnemonic( menu, _( "Flush & Reload Shaders" ),
										   G_CALLBACK( HandleCommand ), ID_TEXTURES_RELOADSHADERS );
	g_object_set_data( G_OBJECT( window ), "menu_textures_reloadshaders", item );
	item = create_menu_item_with_mnemonic( menu, _( "Load directory..." ),
										   G_CALLBACK( HandleCommand ), ID_TEXTURES_LOAD );
	g_object_set_data( G_OBJECT( window ), "menu_textures_load", item );
	item = create_menu_item_with_mnemonic( menu, _( "Directory list..." ),
										   G_CALLBACK( HandleCommand ), ID_TEXTURES_LOADLIST );
	menu_separator( menu );

	item = create_menu_item_with_mnemonic( menu, _( "_Surface Inspector" ),
										   G_CALLBACK( HandleCommand ), ID_TEXTURES_INSPECTOR );
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Render Quality" ) );
	g_object_set_data( G_OBJECT( window ), "render_quality_menu", menu_in_menu );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, NULL, _( "_Wireframe" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_WIREFRAME, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_wireframe", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "_Flat shade" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_FLATSHADE, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_flatshade", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "_Nearest" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_NEAREST, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_nearest", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "Nearest _Mipmap" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_NEARESTMIPMAP, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_nearestmipmap", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "_Linear" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_LINEAR, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_linear", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "_Bilinear" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_BILINEAR, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_bilinear", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "B_ilinear Mipmap" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_BILINEARMIPMAP, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_bilinearmipmap", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "T_rilinear" ),
												 G_CALLBACK( HandleCommand ), ID_VIEW_TRILINEAR, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_view_trilinear", item );
	create_menu_item_with_mnemonic( menu, _( "Find / Replace..." ),
									G_CALLBACK( HandleCommand ), ID_TEXTURE_REPLACEALL );

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Texture Lock" ) );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Moves" ),
												 G_CALLBACK( HandleCommand ), ID_TOGGLE_LOCK, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_toggle_lock", item );
	item = create_check_menu_item_with_mnemonic( menu_in_menu, _( "Rotations" ),
												 G_CALLBACK( HandleCommand ), ID_TOGGLE_ROTATELOCK, TRUE );
	g_object_set_data( G_OBJECT( window ), "menu_toggle_rotatelock", item );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Texture Window Scale" ) );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, NULL, _( "200%" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_TEXTUREWINDOWSCALE_200, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_texturewindowscale_200", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "100%" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_TEXTUREWINDOWSCALE_100, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_texturewindowscale_100", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "50%" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_TEXTUREWINDOWSCALE_50, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_texturewindowscale_50", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "25%" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_TEXTUREWINDOWSCALE_25, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_texturewindowscale_25", item );
	item = create_radio_menu_item_with_mnemonic( menu_in_menu, item, _( "10%" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_TEXTUREWINDOWSCALE_10, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_texturewindowscale_10", item );
	item = menu_separator( menu );
	item = create_check_menu_item_with_mnemonic( menu, _( "shaderlist.txt only" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_SHADERLISTONLY, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_shaderlistonly", item );
	item = create_check_menu_item_with_mnemonic( menu, _( "Hide empty directories" ),
												 G_CALLBACK( HandleCommand ), ID_TEXTURES_EMPTYDIRS_HIDE, FALSE );
	g_object_set_data( G_OBJECT( window ), "menu_textures_emptydirs_hide", item );
	item = menu_separator( menu );

	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Texture Directories" ) );
	g_object_set_data( G_OBJECT( window ), "menu_texture_dirs", menu_in_menu );

	// Misc menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Misc" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	create_menu_item_with_mnemonic( menu, _( "_Benchmark" ), G_CALLBACK( HandleCommand ), ID_MISC_BENCHMARK );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Colors" ) );
	menu_3 = create_menu_in_menu_with_mnemonic( menu_in_menu, _( "Themes" ) );
	create_menu_item_with_mnemonic( menu_3, _( "QE4 Original" ), G_CALLBACK( HandleCommand ), ID_COLOR_SETORIGINAL );
	create_menu_item_with_mnemonic( menu_3, _( "Q3Radiant Original" ), G_CALLBACK( HandleCommand ), ID_COLOR_SETQER );
	create_menu_item_with_mnemonic( menu_3, _( "Black and Green" ), G_CALLBACK( HandleCommand ), ID_COLOR_SETBLACK );
	create_menu_item_with_mnemonic( menu_3, _( "Maya/Max/Lightwave Emulation" ), G_CALLBACK( HandleCommand ), ID_COLOR_SETYDNAR );

	menu_separator( menu_in_menu );
	create_menu_item_with_mnemonic( menu_in_menu, _( "_Texture Background..." ),
									G_CALLBACK( HandleCommand ), ID_TEXTUREBK );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Background..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_XYBK );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Major..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_MAJOR );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Minor..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_MINOR );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Major Small..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_MAJOR_ALT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Minor Small..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_MINOR_ALT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Text..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_GRIDTEXT );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Grid Block..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_GRIDBLOCK );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Default Brush..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_BRUSH );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Camera Background..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_CAMERABACK );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Selected Brush..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_SELECTEDBRUSH );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Selected Brush (Camera)..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_SELECTEDBRUSH3D );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Clipper..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_CLIPPER );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Active View name..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_VIEWNAME );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Detail Brush..." ),
									G_CALLBACK( HandleCommand ), ID_COLORS_DETAIL );

	create_menu_item_with_mnemonic( menu, _( "_Gamma..." ),
									G_CALLBACK( HandleCommand ), ID_MISC_GAMMA );
	create_menu_item_with_mnemonic( menu, _( "Find brush..." ),
									G_CALLBACK( HandleCommand ), ID_MISC_FINDBRUSH );
	item = create_menu_item_with_mnemonic( menu, _( "Next leak spot" ),
										   G_CALLBACK( HandleCommand ), ID_MISC_NEXTLEAKSPOT );
	item = create_menu_item_with_mnemonic( menu, _( "Previous leak spot" ),
										   G_CALLBACK( HandleCommand ), ID_MISC_PREVIOUSLEAKSPOT );
	create_menu_item_with_mnemonic( menu, _( "_Print XY View" ), G_CALLBACK( HandleCommand ), ID_MISC_PRINTXY );
	item = create_menu_item_with_mnemonic( menu, _( "_Select Entity Color..." ),
										   G_CALLBACK( HandleCommand ), ID_MISC_SELECTENTITYCOLOR );
	g_object_set_data( G_OBJECT( window ), "menu_misc_selectentitycolor", item );

	// Region menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Region" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	create_menu_item_with_mnemonic( menu, _( "_Off" ),
									G_CALLBACK( HandleCommand ), ID_REGION_OFF );
	create_menu_item_with_mnemonic( menu, _( "_Set XY" ),
									G_CALLBACK( HandleCommand ), ID_REGION_SETXY );
	create_menu_item_with_mnemonic( menu, _( "Set _Tall Brush" ),
									G_CALLBACK( HandleCommand ), ID_REGION_SETTALLBRUSH );
	create_menu_item_with_mnemonic( menu, _( "Set _Brush" ),
									G_CALLBACK( HandleCommand ), ID_REGION_SETBRUSH );
	create_menu_item_with_mnemonic( menu, _( "Set Se_lected Brushes" ),
									G_CALLBACK( HandleCommand ), ID_REGION_SETSELECTION );

	// Brush menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Brush" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	item = create_menu_item_with_mnemonic( menu, _( "3 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_3SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "4 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_4SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "5 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_5SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "6 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_6SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "7 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_7SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "8 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_8SIDED );
	item = create_menu_item_with_mnemonic( menu, _( "9 sided" ), G_CALLBACK( HandleCommand ), ID_BRUSH_9SIDED );
	menu_separator( menu );
	create_menu_item_with_mnemonic( menu, _( "Arbitrary sided..." ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_ARBITRARYSIDED );
	menu_separator( menu );
	menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Primitives" ) );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Cone..." ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_MAKECONE );
	create_menu_item_with_mnemonic( menu_in_menu, _( "Sphere..." ),
									G_CALLBACK( HandleCommand ), ID_BRUSH_PRIMITIVES_SPHERE );

	// Curve menu
	if ( !g_pGameDescription->mNoPatch ) {
		menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Curve" ) );
		if ( g_PrefsDlg.m_bDetachableMenus ) {
			menu_tearoff( menu );
		}

		create_menu_item_with_mnemonic( menu, _( "Cylinder" ), G_CALLBACK( HandleCommand ), ID_CURVE_PATCHTUBE );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "More Cylinders" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Dense Cylinder" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_PATCHDENSETUBE );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Very Dense Cylinder" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_PATCHVERYDENSETUBE );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Square Cylinder" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_PATCHSQUARE );
		menu_separator( menu );
		create_menu_item_with_mnemonic( menu, _( "End cap" ), G_CALLBACK( HandleCommand ), ID_CURVE_PATCHENDCAP );
		create_menu_item_with_mnemonic( menu, _( "Bevel" ), G_CALLBACK( HandleCommand ), ID_CURVE_PATCHBEVEL );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "More End caps, Bevels" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Square Endcap" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_MOREENDCAPSBEVELS_SQUAREBEVEL );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Square Bevel" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_MOREENDCAPSBEVELS_SQUAREENDCAP );
		menu_separator( menu );
		create_menu_item_with_mnemonic( menu, _( "Cone" ), G_CALLBACK( HandleCommand ), ID_CURVE_PATCHCONE );
		item = create_menu_item_with_mnemonic( menu, _( "Sphere" ),
											   G_CALLBACK( HandleCommand ), ID_CURVE_PRIMITIVES_SPHERE );
		gtk_widget_set_sensitive( item, FALSE );
		menu_separator( menu );
		item = create_menu_item_with_mnemonic( menu, _( "Simple Patch Mesh..." ),
											   G_CALLBACK( HandleCommand ), ID_CURVE_SIMPLEPATCHMESH );
		g_object_set_data( G_OBJECT( window ), "menu_simplepatchmesh", item );
		create_menu_item_with_mnemonic( menu, _( "Patch Inspector..." ), G_CALLBACK( HandleCommand ), ID_PATCH_INSPECTOR );
		menu_separator( menu );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Insert" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Insert (2) Columns" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_INSERT_INSERTCOLUMN );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Add (2) Columns" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_INSERT_ADDCOLUMN );
		menu_separator( menu_in_menu );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Insert (2) Rows" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_INSERT_INSERTROW );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Add (2) Rows" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_INSERT_ADDROW );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Delete" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "First (2) Columns" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_DELETE_FIRSTCOLUMN );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Last (2) Columns" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_DELETE_LASTCOLUMN );
		menu_separator( menu_in_menu );
		create_menu_item_with_mnemonic( menu_in_menu, _( "First (2) Rows" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_DELETE_FIRSTROW );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Last (2) Rows" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_DELETE_LASTROW );
		menu_separator( menu );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Matrix" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Invert" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_NEGATIVE );
		menu_3 = create_menu_in_menu_with_mnemonic( menu_in_menu, _( "Re-disperse" ) );
		create_menu_item_with_mnemonic( menu_3, _( "Rows" ), G_CALLBACK( HandleCommand ), ID_CURVE_REDISPERSE_ROWS );
		create_menu_item_with_mnemonic( menu_3, _( "Cols (Intermediate)" ), G_CALLBACK( HandleCommand ), ID_CURVE_REDISPERSE_INTERMEDIATE_COLS );
		create_menu_item_with_mnemonic( menu_3, _( "Rows (Intermediate)" ), G_CALLBACK( HandleCommand ), ID_CURVE_REDISPERSE_INTERMEDIATE_ROWS );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Transpose" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_MATRIX_TRANSPOSE );
		menu_separator( menu );
		create_menu_item_with_mnemonic( menu, _( "Cap Selection" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_CAP );
		create_menu_item_with_mnemonic( menu, _( "Cycle Cap Texture" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_CYCLECAP );
		menu_separator( menu );
		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, _( "Overlay" ) );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Set" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_OVERLAY_SET );
		create_menu_item_with_mnemonic( menu_in_menu, _( "Clear" ),
										G_CALLBACK( HandleCommand ), ID_CURVE_OVERLAY_CLEAR );
		menu_separator( menu );
		create_menu_item_with_mnemonic( menu, _( "Thicken..." ), G_CALLBACK( HandleCommand ), ID_CURVE_THICKEN );
	}
	// Plugins menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Plugins" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	/*
	   create_menu_item_with_mnemonic (menu, "Refresh", G_CALLBACK (HandleCommand), ID_PLUGINS_REFRESH);
	 */
	// NOTE: the seperator is used when doing a refresh of the list, everything past the seperator is removed
	item = menu_separator( menu );
	g_object_set_data( G_OBJECT( window ), "menu_plugin_separator", item );
	g_object_set_data( G_OBJECT( window ), "menu_plugin", menu );

	// Help menu
	menu = create_sub_menu_with_mnemonic( menu_bar, _( "_Help" ) );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	item = create_menu_item_with_mnemonic( menu, _( "GtkRadiant Manual" ),
										   G_CALLBACK( HandleCommand ), ID_HELP );
	// does not work, using g_Commands for the key binding
	//gtk_widget_add_accelerator( item, "activate", accel, GDK_F1, (GdkModifierType)0, GTK_ACCEL_VISIBLE );

	// this creates all the per-game drop downs for the game pack helps
	// it will take care of hooking the Sys_OpenURL calls etc.
	create_game_help_menu( menu, accel );

	// TTimo: this is in global.xlink now
	//create_menu_item_with_mnemonic (menu, "Links",
	//                  G_CALLBACK (HandleCommand), ID_HELP_LINKS);
	create_menu_item_with_mnemonic( menu, _( "Report a Bug" ),
									G_CALLBACK( HandleCommand ), ID_HELP_BUGREPORT );
	create_menu_item_with_mnemonic( menu, _( "View Shortcuts" ),
									G_CALLBACK( HandleCommand ), ID_HELP_COMMANDLIST );
	create_menu_item_with_mnemonic( menu, _( "_About GtkRadiant" ),
									G_CALLBACK( HandleCommand ), ID_HELP_ABOUT );


	// leo: Hidden menu to make the accelerators work,
	// this is a hack that needs to be changed later if someone has a better idea.
	// NOTE TTimo
	// maybe the better idea would be NOT to use any such accelerator scheme and do all key listening and interpret ourselves
	menu = create_sub_menu_with_mnemonic( menu_bar, "Hidden" );
	if ( g_PrefsDlg.m_bDetachableMenus ) {
		menu_tearoff( menu );
	}

	gtk_widget_hide( gtk_menu_get_attach_widget( GTK_MENU( menu ) ) );

	create_menu_item_with_mnemonic( menu, "BendMode", G_CALLBACK( HandleCommand ), ID_PATCH_BEND );
	create_menu_item_with_mnemonic( menu, "FitTexture", G_CALLBACK( HandleCommand ), IDC_BTN_FACEFIT );
	create_menu_item_with_mnemonic( menu, "ViewTextures", G_CALLBACK( HandleCommand ), ID_VIEW_TEXTURE );
	create_menu_item_with_mnemonic( menu, "InvertCurveTextureX", G_CALLBACK( HandleCommand ), ID_CURVE_NEGATIVETEXTUREY );
	create_menu_item_with_mnemonic( menu, "InvertCurveTextureY", G_CALLBACK( HandleCommand ), ID_CURVE_NEGATIVETEXTUREX );
	create_menu_item_with_mnemonic( menu, "IncPatchColumn", G_CALLBACK( HandleCommand ), ID_CURVE_INSERTCOLUMN );
	create_menu_item_with_mnemonic( menu, "IncPatchRow", G_CALLBACK( HandleCommand ), ID_CURVE_INSERTROW );
	create_menu_item_with_mnemonic( menu, "DecPatchColumn", G_CALLBACK( HandleCommand ), ID_CURVE_DELETECOLUMN );
	create_menu_item_with_mnemonic( menu, "DecPatchRow", G_CALLBACK( HandleCommand ), ID_CURVE_DELETEROW );
	create_menu_item_with_mnemonic( menu, "Patch TAB", G_CALLBACK( HandleCommand ), ID_PATCH_TAB );
	create_menu_item_with_mnemonic( menu, "Patch TAB", G_CALLBACK( HandleCommand ), ID_PATCH_TAB );
	create_menu_item_with_mnemonic( menu, "SelectNudgeDown", G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGEDOWN );
	create_menu_item_with_mnemonic( menu, "CameraForward", G_CALLBACK( HandleCommand ), ID_CAMERA_FORWARD );
	create_menu_item_with_mnemonic( menu, "CameraBack", G_CALLBACK( HandleCommand ), ID_CAMERA_BACK );
	create_menu_item_with_mnemonic( menu, "CameraLeft", G_CALLBACK( HandleCommand ), ID_CAMERA_LEFT );
	create_menu_item_with_mnemonic( menu, "CameraRight", G_CALLBACK( HandleCommand ), ID_CAMERA_RIGHT );
	create_menu_item_with_mnemonic( menu, "CameraUp", G_CALLBACK( HandleCommand ), ID_CAMERA_UP );
	create_menu_item_with_mnemonic( menu, "CameraDown", G_CALLBACK( HandleCommand ), ID_CAMERA_DOWN );
	create_menu_item_with_mnemonic( menu, "CameraAngleUp", G_CALLBACK( HandleCommand ), ID_CAMERA_ANGLEUP );
	create_menu_item_with_mnemonic( menu, "CameraAngleDown", G_CALLBACK( HandleCommand ), ID_CAMERA_ANGLEDOWN );
	create_menu_item_with_mnemonic( menu, "CameraStrafeRight", G_CALLBACK( HandleCommand ), ID_CAMERA_STRAFERIGHT );
	create_menu_item_with_mnemonic( menu, "CameraStrafeLeft", G_CALLBACK( HandleCommand ), ID_CAMERA_STRAFELEFT );
	create_menu_item_with_mnemonic( menu, "ToggleGrid", G_CALLBACK( HandleCommand ), ID_GRID_TOGGLE );
	create_menu_item_with_mnemonic( menu, "ToggleCrosshairs", G_CALLBACK( HandleCommand ), ID_VIEW_CROSSHAIR );
	create_menu_item_with_mnemonic( menu, "ToggleRealtime", G_CALLBACK( HandleCommand ), ID_VIEW_CAMERAUPDATE );
	create_menu_item_with_mnemonic( menu, "MouseRotate", G_CALLBACK( HandleCommand ), ID_SELECT_MOUSEROTATE );
	create_menu_item_with_mnemonic( menu, "TexRotateClock", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_ROTATECLOCK );
	create_menu_item_with_mnemonic( menu, "TexRotateCounter", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_ROTATECOUNTER );
	create_menu_item_with_mnemonic( menu, "TexScaleUp", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SCALEUP );
	create_menu_item_with_mnemonic( menu, "TexScaleDown", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SCALEDOWN );
	create_menu_item_with_mnemonic( menu, "TexShiftLeft", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SHIFTLEFT );
	create_menu_item_with_mnemonic( menu, "TexShiftRight", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SHIFTRIGHT );
	create_menu_item_with_mnemonic( menu, "TexShiftUp", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SHIFTUP );
	create_menu_item_with_mnemonic( menu, "TexShiftDown", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SHIFTDOWN );
	create_menu_item_with_mnemonic( menu, "GridDown", G_CALLBACK( HandleCommand ), ID_GRID_PREV );
	create_menu_item_with_mnemonic( menu, "GridUp", G_CALLBACK( HandleCommand ), ID_GRID_NEXT );
	create_menu_item_with_mnemonic( menu, "TexScaleLeft", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SCALELEFT );
	create_menu_item_with_mnemonic( menu, "TexScaleRight", G_CALLBACK( HandleCommand ), ID_SELECTION_TEXTURE_SCALERIGHT );
	create_menu_item_with_mnemonic( menu, "MoveSelectionDOWN", G_CALLBACK( HandleCommand ), ID_SELECTION_MOVEDOWN );
	create_menu_item_with_mnemonic( menu, "MoveSelectionUP", G_CALLBACK( HandleCommand ), ID_SELECTION_MOVEUP );
	create_menu_item_with_mnemonic( menu, "DumpSelectedBrush", G_CALLBACK( HandleCommand ), ID_SELECTION_PRINT );
	create_menu_item_with_mnemonic( menu, "ToggleSizePaint", G_CALLBACK( HandleCommand ), ID_SELECTION_TOGGLESIZEPAINT );
	create_menu_item_with_mnemonic( menu, "SelectNudgeLeft", G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGELEFT );
	create_menu_item_with_mnemonic( menu, "SelectNudgeRight", G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGERIGHT );
	create_menu_item_with_mnemonic( menu, "SelectNudgeUp", G_CALLBACK( HandleCommand ), ID_SELECTION_SELECT_NUDGEUP );
	create_menu_item_with_mnemonic( menu, "NaturalizePatch", G_CALLBACK( HandleCommand ), ID_PATCH_NATURALIZE );
	create_menu_item_with_mnemonic( menu, "SnapPatchToGrid", G_CALLBACK( HandleCommand ), ID_SELECT_SNAPTOGRID );
	create_menu_item_with_mnemonic( menu, "SelectAllOfType", G_CALLBACK( HandleCommand ), ID_SELECT_ALL );
	create_menu_item_with_mnemonic( menu, "CycleOutlineStyle", G_CALLBACK( HandleCommand ), ID_SELECTION_OUTLINESTYLE );
	create_menu_item_with_mnemonic( menu, "TextureWindowScaleup", G_CALLBACK( HandleCommand ), ID_TEXTUREWINDOW_SCALEUP );
	create_menu_item_with_mnemonic( menu, "TextureWindowScaledown", G_CALLBACK( HandleCommand ), ID_TEXTUREWINDOW_SCALEDOWN );

	g_bIgnoreCommands--;
}

static GtkWidget * toolbar_append_item( GtkToolbar *toolbar, const gchar *text, const gchar *tooltip_text, const gchar *private_text, GtkWidget *icon, GCallback callback, gpointer data )
{
	GtkToolItem *item;
	
	item = gtk_tool_button_new( icon, text );
	gtk_widget_set_tooltip_text( GTK_WIDGET( item ), tooltip_text );
	g_signal_connect( item, "clicked", callback, data );
	gtk_toolbar_insert( GTK_TOOLBAR( toolbar ), item, -1 ); //-1 append
	gtk_widget_show( GTK_WIDGET( item ) );

	return GTK_WIDGET( item );
}
static GtkWidget * toolbar_append_space( GtkToolbar *toolbar )
{
	GtkToolItem *sep_item;

	sep_item = gtk_separator_tool_item_new();
	gtk_toolbar_insert( GTK_TOOLBAR( toolbar ), sep_item, -1 ); //-1 append
	gtk_widget_show( GTK_WIDGET( sep_item ) );

	return GTK_WIDGET( sep_item );
}
static GtkWidget * toolbar_append_element( GtkToolbar *toolbar, short childtype, const char* unused, const gchar *text, const gchar *tooltip_text, const gchar *private_text, GtkWidget *icon, GCallback callback, gpointer data )
{
	GtkToolItem *item;
	
	item = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( item ), icon );
	gtk_tool_button_set_label( GTK_TOOL_BUTTON( item ), text );
	gtk_widget_set_tooltip_text( GTK_WIDGET( item ), tooltip_text );
	g_signal_connect( item, "clicked", callback, data );
	gtk_toolbar_insert( GTK_TOOLBAR( toolbar ), item, -1 ); //-1 append
	gtk_widget_show( GTK_WIDGET( item ) );

	return GTK_WIDGET( item );

}

void MainFrame::create_main_toolbar( GtkWidget *window, GtkWidget *vbox ){
	GtkWidget *toolbar, *w;
	const short TOOLBAR_CHILD_TOGGLEBUTTON = 1;

	toolbar = gtk_toolbar_new();
	gtk_orientable_set_orientation( GTK_ORIENTABLE( toolbar ), GTK_ORIENTATION_HORIZONTAL );
	gtk_toolbar_set_style( GTK_TOOLBAR( toolbar ), GTK_TOOLBAR_ICONS );
	//  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), user_rc.toolbar_style);
	gtk_box_pack_start( GTK_BOX( vbox ), toolbar, FALSE, FALSE, 0 );


	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), _( "Open" ), _( "Open an existing map" ), "",
								 new_image_icon("file_open.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_FILE_OPEN ) );
	g_object_set_data( G_OBJECT( window ), "tb_file_open", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), _( "Save" ), _( "Save the active map" ), "",
								 new_image_icon("file_save.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_FILE_SAVE ) );
	g_object_set_data( G_OBJECT( window ), "tb_file_save", w );
	toolbar_append_space( GTK_TOOLBAR( toolbar ) );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "x-axis Flip" ), "",
								 new_image_icon("brush_flipx.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_FLIPX ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_flipx", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "x-axis Rotate" ), "",
								 new_image_icon("brush_rotatex.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_ROTATEX ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_rotatex", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "y-axis Flip" ), "",
								 new_image_icon("brush_flipy.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_FLIPY ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_flipy", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "y-axis Rotate" ), "",
								 new_image_icon("brush_rotatey.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_ROTATEY ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_rotatey", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "z-axis Flip" ), "",
								 new_image_icon("brush_flipz.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_FLIPZ ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_flipz", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "z-axis Rotate" ), "",
								 new_image_icon("brush_rotatez.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_BRUSH_ROTATEZ ) );
	g_object_set_data( G_OBJECT( window ), "tb_brush_rotatez", w );
	toolbar_append_space( GTK_TOOLBAR( toolbar ) );

	if ( g_PrefsDlg.m_bWideToolbar ) {
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Complete Tall" ), "",
									 new_image_icon("selection_selectcompletetall.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_SELECTION_SELECTCOMPLETETALL ) );
		g_object_set_data( G_OBJECT( window ), "tb_selection_selectcompletetall", w );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Select Touching" ), "",
									 new_image_icon("selection_selecttouching.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_SELECTION_SELECTTOUCHING ) );
		g_object_set_data( G_OBJECT( window ), "tb_selection_selecttouching", w );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Select Partial Tall" ), "",
									 new_image_icon("selection_selectpartialtall.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_SELECTION_SELECTPARTIALTALL ) );
		g_object_set_data( G_OBJECT( window ), "tb_selection_selectpartialtall", w );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Select Inside" ), "",
									 new_image_icon("selection_selectinside.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_SELECTION_SELECTINSIDE ) );
		g_object_set_data( G_OBJECT( window ), "tb_selection_selectinside", w );
	}
	else
	{
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Selection" ), "",
									 new_image_icon("popup_selection.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_POPUP_SELECTION ) );
		g_object_set_data( G_OBJECT( window ), "tb_popup_selection", w );
	}
	toolbar_append_space( GTK_TOOLBAR( toolbar ) );

	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "CSG Subtract" ), "",
								 new_image_icon("selection_csgsubtract.png"),
								 G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECTION_CSGSUBTRACT ) );
	g_object_set_data( G_OBJECT( window ), "tb_selection_csgsubtract", w );

	if ( g_PrefsDlg.m_bWideToolbar ) {
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "CSG Merge" ), "",
									 new_image_icon("selection_csgmerge.png"),
									 G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECTION_CSGMERGE ) );
		g_object_set_data( G_OBJECT( window ), "tb_selection_csgmerge", w );
	}

	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Hollow Overlap" ), "",
								 new_image_icon("selection_makehollow.png"),
								 G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECTION_MAKEHOLLOW ) );
	g_object_set_data( G_OBJECT( window ), "tb_selection_makehollow", w );
	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Hollow Touch" ), "",
								 new_image_icon("selection_makehollowtouch.png"),
								 G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECTION_MAKEHOLLOW_TOUCH ) );
	g_object_set_data( G_OBJECT( window ), "tb_selection_makehollow_touch", w );

	if ( g_PrefsDlg.m_bWideToolbar ) {
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Clipper" ), "", new_image_icon("view_clipper.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_CLIPPER ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_clipper", w );
	}

	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Make Detail Brushes" ), "", new_image_icon("toggle_struct.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_TOGGLE_DETAIL ) );
		g_object_set_data( G_OBJECT( window ), "ttb_toggle_detail", w );

	toolbar_append_space( GTK_TOOLBAR( toolbar ) );

	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Change views" ), "",
								 new_image_icon("view_change.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_VIEW_CHANGE ) );
	g_object_set_data( G_OBJECT( window ), "tb_view_change", w );

	if ( !g_PrefsDlg.m_bWideToolbar ) {
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
	}

	w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Texture view mode" ), "",
								 new_image_icon("textures_popup.png"), G_CALLBACK( HandleCommand ),
								 GINT_TO_POINTER( ID_TEXTURES_POPUP ) );
	g_object_set_data( G_OBJECT( window ), "tb_textures_popup", w );

	if ( g_PrefsDlg.m_bWideToolbar ) {
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Cubic clip the camera view" ), "",
										new_image_icon("view_cubicclipping.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_CUBICCLIPPING ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_cubicclipping", w );
	}

	toolbar_append_space( GTK_TOOLBAR( toolbar ) );

	if ( !g_PrefsDlg.m_bWideToolbar ) {
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Camera preview" ), "", new_image_icon("view_cameratoggle.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_CAMERATOGGLE ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_cameratoggle", w );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Update Camera" ), "",
									 new_image_icon("view_cameraupdate.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_VIEW_CAMERAUPDATE ) );
		g_object_set_data( G_OBJECT( window ), "tb_view_cameraupdate", w );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Cubic clip the camera view" ), "",
										new_image_icon("view_cubicclipping.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_CUBICCLIPPING ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_cubicclipping", w );
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Entity inspector" ), "", new_image_icon("view_entity.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_ENTITY ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_entity", w );
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Clipper" ), "", new_image_icon("view_clipper.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_VIEW_CLIPPER ) );
		g_object_set_data( G_OBJECT( window ), "ttb_view_clipper", w );
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
	}

	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
									"", _( "Free Rotation" ), "", new_image_icon("select_mouserotate.png"),
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECT_MOUSEROTATE ) );
	g_object_set_data( G_OBJECT( window ), "ttb_select_mouserotate", w );
	toolbar_append_space( GTK_TOOLBAR( toolbar ) );
	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
									"", _( "Free Scaling" ), "", new_image_icon("select_mousescale.png"),
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SELECT_MOUSESCALE ) );
	g_object_set_data( G_OBJECT( window ), "ttb_select_mousescale", w );
	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
									"", _( "Disable Scaling Along X" ), "", new_image_icon("scalelockx.png"),
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SCALELOCKX ) );
	g_object_set_data( G_OBJECT( window ), "ttb_scalelockx", w );
	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
									"", _( "Disable Scaling Along Y" ), "", new_image_icon("scalelocky.png"),
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SCALELOCKY ) );
	g_object_set_data( G_OBJECT( window ), "ttb_scalelocky", w );
	w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
									"", _( "Disable Scaling Along Z" ), "", new_image_icon("scalelockz.png"),
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_SCALELOCKZ ) );
	g_object_set_data( G_OBJECT( window ), "ttb_scalelockz", w );

	if ( g_PrefsDlg.m_bWideToolbar ) {
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Don't select model brushes" ), "",
										new_image_icon("dontselectmodel.png"), G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( ID_DONTSELECTMODEL ) );
		g_object_set_data( G_OBJECT( window ), "ttb_dontselectmodel", w );
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );

		if ( !g_pGameDescription->mNoPatch ) {
			w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
											"", _( "Don't select curved brushes" ), "",
											new_image_icon("dontselectcurve.png"), G_CALLBACK( HandleCommand ),
											GINT_TO_POINTER( ID_DONTSELECTCURVE ) );
			g_object_set_data( G_OBJECT( window ), "ttb_dontselectcurve", w );
		}
	}

	// bug #292, patch toolbar option
	if ( g_PrefsDlg.m_bPatchToolbar ) {
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Show patch bounding box" ), "",
										new_image_icon("patch_showboundingbox.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_PATCH_SHOWBOUNDINGBOX ) );
		g_object_set_data( G_OBJECT( window ), "ttb_patch_showboundingbox", w );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Show patches as wireframes" ), "",
										new_image_icon("patch_wireframe.png"),
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( ID_PATCH_WIREFRAME ) );
		g_object_set_data( G_OBJECT( window ), "ttb_patch_wireframe", w );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Patch Bend mode" ), "",
										new_image_icon("patch_bend.png"), G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( ID_PATCH_BEND ) );
		g_object_set_data( G_OBJECT( window ), "ttb_patch_bend", w );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Put caps on the current patch" ), "",
									 new_image_icon("curve_cap.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_CURVE_CAP ) );
		g_object_set_data( G_OBJECT( window ), "tb_curve_cap", w );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Welds equal patch points during moves" ), "",
										new_image_icon("patch_weld.png"), G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( ID_PATCH_WELD ) );
		g_object_set_data( G_OBJECT( window ), "ttb_patch_weld", w );
		w = toolbar_append_element( GTK_TOOLBAR( toolbar ), TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
										"", _( "Selects drill down rows and columns" ), "",
										new_image_icon("patch_drilldown.png"), G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( ID_PATCH_DRILLDOWN ) );
		g_object_set_data( G_OBJECT( window ), "ttb_patch_drilldown", w );
	}

	if ( g_PrefsDlg.m_bWideToolbar ) {
		toolbar_append_space( GTK_TOOLBAR( toolbar ) );
		w = toolbar_append_item( GTK_TOOLBAR( toolbar ), "", _( "Show Entities as" ), "",
									 new_image_icon("show_entities.png"), G_CALLBACK( HandleCommand ),
									 GINT_TO_POINTER( ID_SHOW_ENTITIES ) );
		g_object_set_data( G_OBJECT( window ), "tb_show_entities", w );
	}

	/*
	   uh? that is OLD
	   #ifndef QUAKE3
	   w = g_object_get_data (G_OBJECT (window), "tb_dontselectcurve");
	   gtk_widget_hide (w);
	   w = g_object_get_data (G_OBJECT (window), "tb_patch_showboundingbox");
	   gtk_widget_hide (w);
	   w = g_object_get_data (G_OBJECT (window), "tb_patch_weld");
	   gtk_widget_hide (w);
	   w = g_object_get_data (G_OBJECT (window), "tb_patch_wireframe");
	   gtk_widget_hide (w);
	   #endif
	 */

	m_bCamPreview = true;
	g_nScaleHow = ( SCALE_X | SCALE_Y | SCALE_Z );

	gtk_widget_show( toolbar );
}

void MainFrame::create_plugin_toolbar( GtkWidget *window, GtkWidget *vbox ){
	GtkWidget *toolbar;

	toolbar = gtk_toolbar_new();
	gtk_orientable_set_orientation( GTK_ORIENTABLE( toolbar ), GTK_ORIENTATION_HORIZONTAL );
	gtk_toolbar_set_style( GTK_TOOLBAR( toolbar ), GTK_TOOLBAR_ICONS );
	//  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), user_rc.toolbar_style);
	gtk_box_pack_start( GTK_BOX( vbox ), toolbar, FALSE, FALSE, 0 );
	g_object_set_data( G_OBJECT( window ), "toolbar_plugin", toolbar );
	if ( g_PrefsDlg.m_bPluginToolbar ) {
		gtk_widget_show( toolbar );
	}
}

void MainFrame::create_main_statusbar( GtkWidget *window, GtkWidget *vbox ){
	GtkWidget *hbox, *hbox1;
	GtkWidget *frame;
	GtkWidget *label;

	hbox = gtk_hbox_new( FALSE, 0 );
	gtk_widget_set_size_request( hbox, -1, 24 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 1 );
	gtk_box_pack_end( GTK_BOX( vbox ), hbox, FALSE, TRUE, 2 );
	gtk_widget_show( hbox );

	frame = gtk_frame_new( (char*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
	gtk_widget_show( frame );

	hbox1 = gtk_hbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( frame ), hbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox1 ), 0 );
	gtk_widget_show( hbox1 );

	label = gtk_label_new( _( " Label " ) );
	gtk_box_pack_start( GTK_BOX( hbox1 ), label, FALSE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );
	m_pStatusLabel[0] = label;

	for ( int i = 1; i < 6; i++ )
	{
		frame = gtk_frame_new( (char*)NULL );
		gtk_box_pack_start( GTK_BOX( hbox ), frame, FALSE, TRUE, 0 );
		gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
		gtk_widget_show( frame );

		label = gtk_label_new( _( " Label " ) );
		gtk_container_add( GTK_CONTAINER( frame ), label );
		gtk_misc_set_padding( GTK_MISC( label ), 3, 0 );
		gtk_widget_show( label );
		m_pStatusLabel[i] = label;
	}
}

guint s_idle_id;
static gint mainframe_idle( gpointer user_data ){
	g_pParentWnd->RoutineProcessing();
	return TRUE;
}

static void Sys_Iconify( GtkWidget *w );
static void Sys_Restore( GtkWidget *w );

inline void CHECK_RESTORE( GtkWidget* w ){
	if ( g_object_get_data( G_OBJECT( w ), "was_mapped" ) != NULL ) {
		gtk_widget_show( w );
	}
}


// this is called when the window is restored from the iconified state
static void mainframe_map( GtkWidget *widget ){
	if ( g_pParentWnd->IsSleeping() ) {
		g_pParentWnd->OnSleep();
	}

	if ( ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) && ( widget == g_pParentWnd->m_pWidget ) ) {
		// restore previously visible windows
		CHECK_RESTORE( g_pParentWnd->GetCamWnd()->m_pParent );
		if ( g_PrefsDlg.m_bFloatingZ ) {
			CHECK_RESTORE( g_pParentWnd->GetZWnd()->m_pParent );
		}
		CHECK_RESTORE( g_pParentWnd->GetXYWnd()->m_pParent );
		CHECK_RESTORE( g_pParentWnd->GetXZWnd()->m_pParent );
		CHECK_RESTORE( g_pParentWnd->GetYZWnd()->m_pParent );
		CHECK_RESTORE( g_pGroupDlg->m_pWidget );
	}
}

inline void CHECK_MINIMIZE( GtkWidget* w ){
	g_object_set_data( G_OBJECT( w ), "was_mapped", (void*)( gtk_widget_get_visible( w ) != 0 ) );
	gtk_widget_hide( w );
}

static void mainframe_unmap( GtkWidget *widget ){

	if ( ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) && ( widget == g_pParentWnd->m_pWidget ) ) {
		// minimize all other windows when the main window is minimized
		CHECK_MINIMIZE( g_pParentWnd->GetCamWnd()->m_pParent );
		if ( g_PrefsDlg.m_bFloatingZ ) {
			CHECK_MINIMIZE( g_pParentWnd->GetZWnd()->m_pParent );
		}
		CHECK_MINIMIZE( g_pParentWnd->GetXYWnd()->m_pParent );
		CHECK_MINIMIZE( g_pParentWnd->GetXZWnd()->m_pParent );
		CHECK_MINIMIZE( g_pParentWnd->GetYZWnd()->m_pParent );
		CHECK_MINIMIZE( g_pGroupDlg->m_pWidget );
	}
}
static gboolean mainframe_state( GtkWidget *widget, GdkEventWindowState *e, gpointer user_data ){

	if( e->changed_mask & GDK_WINDOW_STATE_ICONIFIED && !( e->new_window_state & GDK_WINDOW_STATE_ICONIFIED ) ) {
		mainframe_map( widget );
	}
	return FALSE;
}

static GtkWidget* create_floating( MainFrame* mainframe ){
	GtkWidget *wnd = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	//workaround for a bug with set_transient_for in GTK - resulting behaviour is not perfect but better than the bug.
	//(see https://bugzilla.gnome.org/show_bug.cgi?id=658975 regarding the bug)
	if (mainframe->CurrentStyle() != MainFrame::eFloating)
		gtk_window_set_transient_for( GTK_WINDOW( wnd ), GTK_WINDOW( mainframe->m_pWidget ) );
	gtk_widget_set_events( wnd, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );
	g_signal_connect( G_OBJECT( wnd ), "delete-event", G_CALLBACK( widget_delete_hide ), NULL );
	g_signal_connect( G_OBJECT( wnd ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );
	g_signal_connect( G_OBJECT( wnd ), "key-press-event",
						G_CALLBACK( mainframe_keypress ), mainframe );
	g_signal_connect( G_OBJECT( wnd ), "key-release-event",
						G_CALLBACK( mainframe_keyrelease ), mainframe );
	g_signal_connect( G_OBJECT( wnd ), "map-event",
						G_CALLBACK( mainframe_map ), mainframe );

	gtk_window_set_default_size( GTK_WINDOW( wnd ), 100, 100 );

#ifdef DBG_WINDOWPOS
	Sys_Printf( "create_floating: %p, gtk_window_set_default_size 100, 100\n", wnd );
#endif

	return wnd;
}

void console_populate_popup( GtkTextView* textview, GtkMenu* menu, gpointer user_data ){
	menu_separator( GTK_WIDGET( menu ) );

	GtkWidget* item = gtk_menu_item_new_with_label( _( "Clear" ) );
	g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( Sys_ClearPrintf ), NULL );
	gtk_widget_show( item );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
}

void console_construct( GtkWidget* textview ){
	g_signal_connect( G_OBJECT( textview ), "populate-popup", G_CALLBACK( console_populate_popup ), NULL );
}

extern MemStream g_Clipboard;

void Clipboard_CopyMap(){
	g_Clipboard.SetLength( 0 );
	Map_Export( &g_Clipboard, "xmap", false, true );
}

void Clipboard_PasteMap(){
	if ( g_Clipboard.GetLength() > 0 ) {
		g_Clipboard.Seek( 0, SEEK_SET );
		Map_Import( &g_Clipboard, "xmap", true );
	}
}

/*!
   Platform-independent GTK clipboard support.
   \todo Using GDK_SELECTION_CLIPBOARD fails on win32, so we use the win32 API directly for now.
 */
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

enum
{
	RADIANT_CLIPPINGS = 23,
};

static const GtkTargetEntry clipboard_targets[] = {
	{ (gchar *)"RADIANT_CLIPPINGS", 0, RADIANT_CLIPPINGS, },
};

static void clipboard_get( GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer user_data_or_owner ){
	guchar *buffer;
	gint len;
	GdkAtom type = GDK_NONE;

	len = g_Clipboard.GetLength();

	if ( !len ) {
		buffer = NULL;
	}
	else
	{
		buffer = g_Clipboard.GetBuffer();
	}

	if ( info == clipboard_targets[0].info ) {
		type = gdk_atom_intern( clipboard_targets[0].target, FALSE );
	}

	gtk_selection_data_set( selection_data, type, 8, buffer, len );
}

static void clipboard_clear( GtkClipboard *clipboard, gpointer user_data_or_owner ){
}

static void clipboard_received( GtkClipboard *clipboard, GtkSelectionData *data, gpointer user_data ){
	//g_Clipboard.SetLength( 0 );

	if ( gtk_selection_data_get_length( data ) < 0 ) {
		Sys_FPrintf( SYS_ERR, "Error retrieving selection\n" );
	}
	else if ( strcmp( gdk_atom_name( gtk_selection_data_get_data_type( data ) ), clipboard_targets[0].target ) == 0 ) {
		g_Clipboard.SetLength( 0 );
		g_Clipboard.Write( gtk_selection_data_get_data( data ), gtk_selection_data_get_length( data ) );
	}

	Clipboard_PasteMap();
}

void clipboard_copy(){
	Clipboard_CopyMap();

	GtkClipboard* clipboard = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );

	gtk_clipboard_set_with_data( clipboard, clipboard_targets, 1, clipboard_get, clipboard_clear, NULL );
}

void clipboard_paste(){
	GtkClipboard* clipboard = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );

	gtk_clipboard_request_contents( clipboard, gdk_atom_intern( clipboard_targets[0].target, FALSE ), clipboard_received, NULL );
}


#elif defined( WIN32 )

void clipboard_copy(){
	Clipboard_CopyMap();

	bool bClipped = false;
	UINT nClipboard = ::RegisterClipboardFormat( "RadiantClippings" );
	if ( nClipboard > 0 ) {
		if ( ::OpenClipboard( NULL ) ) {
			EmptyClipboard();
			long lSize = g_Clipboard.GetLength();
			HANDLE h = ::GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, lSize + sizeof( long ) );
			if ( h != NULL ) {
				unsigned char *cp = reinterpret_cast<unsigned char*>( ::GlobalLock( h ) );
				memcpy( cp, &lSize, sizeof( long ) );
				cp += sizeof( long );
				g_Clipboard.Seek( 0, SEEK_SET );
				g_Clipboard.Read( cp, lSize );
				::GlobalUnlock( h );
				::SetClipboardData( nClipboard, h );
				::CloseClipboard();
				bClipped = true;
			}
		}
	}

	if ( !bClipped ) {
		Sys_Printf( "Unable to register Windows clipboard formats, copy/paste between editors will not be possible\n" );
	}
}

void clipboard_paste(){
	bool bPasted = false;
	UINT nClipboard = ::RegisterClipboardFormat( "RadiantClippings" );
	if ( nClipboard > 0 && ::OpenClipboard( NULL ) ) {
		if ( IsClipboardFormatAvailable( nClipboard ) ) {
			HANDLE h = ::GetClipboardData( nClipboard );
			if ( h ) {
				g_Clipboard.SetLength( 0 );
				unsigned char *cp = reinterpret_cast<unsigned char*>( ::GlobalLock( h ) );
				long lSize = 0;
				memcpy( &lSize, cp, sizeof( long ) );
				cp += sizeof( long );
				g_Clipboard.Write( cp, lSize );
				::GlobalUnlock( h );
			}
		}
		::CloseClipboard();
	}

	Clipboard_PasteMap();
}

#endif

void MainFrame::Copy(){
	clipboard_copy();
}

void MainFrame::Paste(){
	clipboard_paste();
	UpdateSurfaceDialog();
}


#ifdef DBG_WINDOWPOS
GtkWidget *watchit = NULL;

void CheckWatchit( char *msg ){
	static int width = 0;
	if ( ( watchit != NULL ) && ( watchit->allocation.width != width ) ) {
		Sys_Printf( "CheckWatchit %s: %d\n", msg, watchit->allocation.width );
		width = watchit->allocation.width;
	}
}
#endif

#ifdef _WIN32
BOOL CALLBACK m_pCountMonitor( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ){
	int *n = (int *) dwData;

	( *n )++;

	return TRUE;
}

struct monitorInfo_s {
	GdkRectangle *win_monitors;
	int i_win_mon;
};

BOOL CALLBACK m_pEnumMonitor( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ){
	monitorInfo_s *monitorInfo = (monitorInfo_s *) dwData;
	GdkRectangle *monitor;
	MONITORINFOEX lpmi;

	monitor = monitorInfo->win_monitors + monitorInfo->i_win_mon;

	memset( &lpmi, 0, sizeof( MONITORINFOEX ) );
	lpmi.cbSize = sizeof( MONITORINFOEX );

	GetMonitorInfo( hMonitor, &lpmi );

	if ( lpmi.dwFlags & MONITORINFOF_PRIMARY ) {
		RECT rect;

		SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 );
		monitor->x = rect.left;
		monitor->y = rect.top;
		monitor->width = rect.right - rect.left;
		monitor->height = rect.bottom - rect.top;

		if ( monitorInfo->i_win_mon != 0 ) {
			GdkRectangle temp = *monitor;
			*monitor = monitorInfo->win_monitors[0];
			monitorInfo->win_monitors[0] = temp;
		}
	}
	else {
		monitor->x = lpmi.rcMonitor.left;
		monitor->y = lpmi.rcMonitor.top;
		monitor->width = lpmi.rcMonitor.right - lpmi.rcMonitor.left;
		monitor->height = lpmi.rcMonitor.bottom - lpmi.rcMonitor.top;
	}

	monitorInfo->i_win_mon++;

	return TRUE;
}

void PositionWindowOnPrimaryScreen( window_position_t& position ){
	const GdkRectangle primaryMonitorRect = g_pParentWnd->GetPrimaryMonitorRect();

	if ( position.x <= primaryMonitorRect.x + 6 ) {
		position.x = primaryMonitorRect.x + 6;
	}
	else if ( position.x >= ( primaryMonitorRect.x + primaryMonitorRect.width ) - 6 ) {
		position.x = primaryMonitorRect.x + 6;
	}

	if ( position.y <= primaryMonitorRect.y + 6 ) {
		position.y = primaryMonitorRect.y + 6;
	}
	else if ( position.y >= ( primaryMonitorRect.y + primaryMonitorRect.height ) - 6 ) {
		position.y = primaryMonitorRect.y + 6;
	}

	if ( position.x + position.w >= ( primaryMonitorRect.x + primaryMonitorRect.width ) - 18 ) {
		position.w = primaryMonitorRect.width - 18;
	}
	if ( position.y + position.h >= ( primaryMonitorRect.y + primaryMonitorRect.height ) - 18 ) {
		position.h = primaryMonitorRect.height - 18;
	}
}
#endif

GtkWidget* create_framed_widget( GtkWidget* widget ){
	GtkWidget* frame = gtk_frame_new( (char*)NULL );
	gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
	gtk_container_add( GTK_CONTAINER( frame ), widget );
	gtk_widget_show( widget );
	gtk_widget_show( frame );
	return frame;
}

static void textdirlist_activate( GtkTreeView *tree_view )
{
	GtkTreeSelection* selection;

	GtkTreeModel* model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( tree_view ) );

	if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
		GtkTreePath* path = gtk_tree_model_get_path( model, &iter );
		if ( gtk_tree_path_get_depth( path ) == 1 ) {
			char* p;
			gtk_tree_model_get( model, &iter, 0, &p, -1 );
			
			Texture_ShowDirectory_by_path( p );
			g_free( p );
		}
		gtk_tree_path_free( path );
	}
}

static void textdirlist_row_activated( GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data )
{
	textdirlist_activate( tree_view );
}
static void textdirlist_cursor_changed( GtkTreeView *tree_view, gpointer user_data )
{
	textdirlist_activate( tree_view );
}

GtkWidget* create_texdirlist_widget()
{
	GtkWidget *scr;
	GtkWidget* view;

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );

	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "Textures", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_set_data( G_OBJECT( g_qeglobals_gui.d_main_window ), "dirlist_treeview" , view );

		GSList *texdirs = NULL;
		FillTextureList( &texdirs );
		FillTextureDirListWidget( texdirs );
		ClearGSList( texdirs );

		g_object_unref( G_OBJECT( store ) );

		gtk_tree_selection_set_mode( gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) ), GTK_SELECTION_SINGLE );
#if GTK_CHECK_VERSION( 3,12,0 )
		gtk_tree_view_set_activate_on_single_click( GTK_TREE_VIEW( view ), TRUE );
#else
		g_signal_connect( view, "cursor-changed", G_CALLBACK( textdirlist_cursor_changed ), view );
#endif
		g_signal_connect( view, "row-activated", G_CALLBACK( textdirlist_row_activated ), view );

		gtk_widget_show( view );

	}

	gtk_widget_show( scr );

	return scr;
}

gboolean entry_focus_in( GtkWidget *widget, GdkEventFocus *event, gpointer user_data ){
	gtk_window_remove_accel_group( GTK_WINDOW( g_pParentWnd->m_pWidget ), global_accel );
	return FALSE;
}

gboolean entry_focus_out( GtkWidget *widget, GdkEventFocus *event, gpointer user_data ){
	gtk_window_add_accel_group( GTK_WINDOW( g_pParentWnd->m_pWidget ), global_accel );
	return FALSE;
}

GtkWidget* create_framed_texwnd( TexWnd* texwnd ){
	GtkWidget* frame = gtk_frame_new( (char*)NULL );
	gtk_widget_show( frame );
	gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );

	GtkWidget* hbox = gtk_hbox_new( FALSE, 0 );
	gtk_widget_show( hbox );
	gtk_container_add( GTK_CONTAINER( frame ), hbox );

	GtkWidget* w = gtk_vscrollbar_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0,0,0,1,1,1 ) ) );
	gtk_widget_show( w );
	gtk_box_pack_end( GTK_BOX( hbox ), w, FALSE, TRUE, 0 );
	g_qeglobals_gui.d_texture_scroll = w;

	GtkWidget* texbox = gtk_vbox_new( FALSE, 0 );
	gtk_widget_show( texbox );
	gtk_box_pack_start( GTK_BOX( hbox ), texbox, TRUE, TRUE, 0 );

	w = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( texbox ), w, FALSE, FALSE, 0 );
	texwnd->m_pFilter = w;
	g_signal_connect( G_OBJECT( w ), "focus-in-event", G_CALLBACK( entry_focus_in ), NULL );
	g_signal_connect( G_OBJECT( w ), "focus-out-event", G_CALLBACK( entry_focus_out ), NULL );

	w = texwnd->GetWidget();
	gtk_box_pack_start( GTK_BOX( texbox ), w, TRUE, TRUE, 0 );
	gtk_widget_show( w );

	return frame;
}

static ZWnd *create_floating_zwnd( MainFrame *mainframe ){
	ZWnd *pZWnd = new ZWnd();
	GtkWidget* wnd = create_floating( mainframe );

	gtk_window_set_title( GTK_WINDOW( wnd ), _( "Z" ) );

	pZWnd->m_pParent = wnd;

	{
		GtkWidget* frame = create_framed_widget( pZWnd->GetWidget() );
		gtk_container_add( GTK_CONTAINER( wnd ), frame );
	}

	gtk_widget_realize( wnd );

	// turn OFF minimize and maximize boxes.
	// Must be *after* realize, or gtk_widget_get_window( wnd ) is NULL
	// should do the right thing on *nix, need to verify.
	gdk_window_set_decorations( gtk_widget_get_window( wnd ),
								GdkWMDecoration( GDK_DECOR_ALL | GDK_DECOR_MINIMIZE | GDK_DECOR_MAXIMIZE ) );
	//TODO 50 by observation, will vary depending on decoration sizes
	{
		GdkGeometry geometry;
		geometry.min_width = 50;
		//we only care about width, but have to set this too, or get nasty bugs
		geometry.min_height = 10;
		gdk_window_set_geometry_hints( gtk_widget_get_window( wnd ), &geometry, GDK_HINT_MIN_SIZE );
	}

#ifdef _WIN32
	if ( g_PrefsDlg.m_bStartOnPrimMon ) {
		PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posZWnd );
	}
#endif
	load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posZWnd );

	if ( g_PrefsDlg.m_bZVis ) {
		gtk_widget_show( wnd );
	}

	return pZWnd;
}

static const int gutter = 12;

void MainFrame::Create(){
	GtkWidget* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	m_pWidget = window;
	gtk_widget_set_events( window, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );
	g_signal_connect( G_OBJECT( window ), "delete-event",
						G_CALLBACK( mainframe_delete ), this );
	g_signal_connect( G_OBJECT( window ), "destroy",
						G_CALLBACK( mainframe_destroy ), this );
	g_signal_connect( G_OBJECT( window ), "key-press-event",
						G_CALLBACK( mainframe_keypress ), this );
	g_signal_connect( G_OBJECT( window ), "key-release-event",
						G_CALLBACK( mainframe_keyrelease ), this );
	g_signal_connect( G_OBJECT( window ), "map-event",
						G_CALLBACK( mainframe_map ), this );
	g_signal_connect( G_OBJECT( window ), "unmap-event",
						G_CALLBACK( mainframe_unmap ), this );
	g_signal_connect( G_OBJECT( window ), "window-state-event",
						G_CALLBACK( mainframe_state ), this );

	g_qeglobals_gui.d_main_window = window;

#ifdef _WIN32
	// calculate gdk offset
	int n_win_monitors = 0;

	monitorInfo_s monitorInfo;

	// detect multiple monitors
	EnumDisplayMonitors( NULL, NULL, m_pCountMonitor, reinterpret_cast<LPARAM>( &n_win_monitors ) );

	monitorInfo.win_monitors = new GdkRectangle [ n_win_monitors ];
	monitorInfo.i_win_mon = 0;
	EnumDisplayMonitors( NULL, NULL, m_pEnumMonitor, reinterpret_cast<LPARAM>( &monitorInfo ) );

	gdk_offset_x = G_MININT;
	gdk_offset_y = G_MININT;

	// calculate offset
	for ( monitorInfo.i_win_mon = 0; monitorInfo.i_win_mon < n_win_monitors; monitorInfo.i_win_mon++ ) {
		gdk_offset_x = MAX( gdk_offset_x, -monitorInfo.win_monitors[monitorInfo.i_win_mon].x );
		gdk_offset_y = MAX( gdk_offset_y, -monitorInfo.win_monitors[monitorInfo.i_win_mon].y );
	}

	// We do not use these offsets anymore. This could all probably be trashed. I doubt the multi monitor does anything useful/works either.
	Sys_Printf( "GDK's coordinate system is offset by %d over the x-axis and %d over the y-axis from Windows' coordinate system.\n", gdk_offset_x, gdk_offset_y );

	if ( g_PrefsDlg.m_bStartOnPrimMon ) {
		// get gdk monitors
		GdkDisplay *display;
		GdkScreen *screen;
		gint n_gdk_monitors = 0;
		gint i_mon;
		GdkRectangle rect;

		// detect multiple monitors
		display = gdk_display_get_default();
		Sys_Printf( "GDK detects that server %s manages %d screens\n", gdk_display_get_name( display ), gdk_display_get_n_screens( display ) );

		screen = gdk_display_get_screen( display, 1 );
		n_gdk_monitors = gdk_screen_get_n_monitors( screen );

		Sys_Printf( "GDK detects that screen 1 has %d monitors\n", n_gdk_monitors );

		for ( i_mon = 0; i_mon < n_gdk_monitors; i_mon++ ) {
			memset( &rect, 0, sizeof( rect ) );
			gdk_screen_get_monitor_geometry( screen, i_mon, &rect );
			Sys_Printf( "  monitor %d: x: %d y: %d w: %d h: %d\n", i_mon, rect.x, rect.y, rect.width, rect.height );

			if ( i_mon == 0 ) {
				memcpy( &primaryMonitorRect, &rect, sizeof( primaryMonitorRect ) );
			}
		}

		PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.position );
	}
	else {
		primaryMonitorRect.x = primaryMonitorRect.y = 0;
		primaryMonitorRect.width = gdk_screen_width();
		primaryMonitorRect.height = gdk_screen_height();
	}

#endif

	load_window_pos( window, g_PrefsDlg.mWindowInfo.position );

	GtkWidget* vbox = gtk_vbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	create_main_menu( window, vbox );
	MRU_Load();
	create_main_toolbar( window, vbox );
	create_plugin_toolbar( window,vbox );
	create_main_statusbar( window, vbox );

	m_nCurrentStyle = g_PrefsDlg.m_nView;

	g_pGroupDlg->Create();
	OnPluginsRefresh();

	CreateQEChildren();

	gtk_widget_show( window );

	// not needed on win32, it's in the .rc
#ifndef _WIN32
	{
		CString icon = g_strBitmapsPath;
		icon += "icon.png";

		GError *error = NULL;

		gtk_window_set_icon_from_file( GTK_WINDOW( window ), icon.GetBuffer(), &error );
		if ( error != NULL ) {
			Sys_FPrintf( SYS_ERR, "ERROR: Failed to load icon: %s\n", error->message );
			g_error_free( error );
		}
	}
#endif

	if ( CurrentStyle() == eRegular || CurrentStyle() == eRegularLeft ) {
		{
			GtkWidget* vsplit = gtk_vpaned_new();
			m_pSplits[0] = vsplit;
			gtk_box_pack_start( GTK_BOX( vbox ), vsplit, TRUE, TRUE, 0 );
			gtk_widget_show( vsplit );

			{
				GtkWidget* hsplit = gtk_hpaned_new();
				m_pSplits[2] = hsplit;
				gtk_paned_add1( GTK_PANED( vsplit ), hsplit );
				gtk_widget_show( hsplit );

				{
					GtkWidget* hsplit2 = gtk_hpaned_new();
					m_pSplits[3] = hsplit2;
					gtk_paned_add2( GTK_PANED( hsplit ), hsplit2 );
					gtk_widget_show( hsplit2 );

					{
						GtkWidget* vsplit2 = gtk_vpaned_new();
						m_pSplits[1] = vsplit2;
						if ( CurrentStyle() == eRegular ) {
							gtk_paned_add2( GTK_PANED( hsplit2 ), vsplit2 );
						}
						else{
							gtk_paned_add1( GTK_PANED( hsplit ), vsplit2 );
						}
						gtk_widget_show( vsplit2 );

						// camera
						m_pCamWnd = new CamWnd();
						{
							GtkWidget* frame = create_framed_widget( m_pCamWnd->GetWidget() );
							gtk_paned_add1( GTK_PANED( vsplit2 ), frame );
						}

						// xy
						m_pXYWnd = new XYWnd();
						m_pXYWnd->SetViewType( XY );
						{
							GtkWidget* frame = create_framed_widget( m_pXYWnd->GetWidget() );
							gtk_paned_add1( GTK_PANED( hsplit2 ), frame );
						}

						// z
						m_pZWnd = new ZWnd();
						{
							GtkWidget* frame = create_framed_widget( m_pZWnd->GetWidget() );
							if ( CurrentStyle() == eRegular ) {
								gtk_paned_add1( GTK_PANED( hsplit ), frame );
							}
							else{
								gtk_paned_add2( GTK_PANED( hsplit2 ), frame );
							}
						}

						// textures
						m_pTexWnd = new TexWnd();
						{
							GtkWidget* frame = create_framed_texwnd( m_pTexWnd );
							if( g_PrefsDlg.m_bShowTexDirList ) {

								GtkWidget* texDirList = create_texdirlist_widget();

								GtkWidget* texSplit = gtk_hpaned_new();
								m_pSplits[4] = texSplit;

								gtk_paned_pack2( GTK_PANED( vsplit2 ), texSplit, TRUE, FALSE );
								gtk_paned_add1( GTK_PANED( texSplit ), texDirList );
								gtk_paned_add2( GTK_PANED( texSplit ), frame );

								if( g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth >= 0 ) {
									gtk_paned_set_position( GTK_PANED( texSplit ), g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth );
								}

								gtk_widget_show( texSplit );
							} else
							{
								m_pSplits[4] = NULL;
								gtk_paned_pack2( GTK_PANED( vsplit2 ), frame, TRUE, TRUE );
							}
						}

						// console
						{
							GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
							gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
							gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
							gtk_widget_show( scr );
							gtk_paned_pack2( GTK_PANED( vsplit ), scr, FALSE, TRUE );

							{
								GtkWidget* text = gtk_text_view_new();
								gtk_widget_set_size_request( text, 0, -1 ); // allow shrinking
								gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( text ), GTK_WRAP_WORD );
								gtk_text_view_set_editable( GTK_TEXT_VIEW( text ), FALSE );
								gtk_container_add( GTK_CONTAINER( scr ), text );
								gtk_widget_show( text );
								g_qeglobals_gui.d_edit = text;
							}
						}
					}
				}
			}
		}

		gtk_paned_set_position( GTK_PANED( m_pSplits[0] ), g_PrefsDlg.mWindowInfo.nXYHeight );

		if ( CurrentStyle() == eRegular ) {
			gtk_paned_set_position( GTK_PANED( m_pSplits[2] ), g_PrefsDlg.mWindowInfo.nZWidth );
			gtk_paned_set_position( GTK_PANED( m_pSplits[3] ), g_PrefsDlg.mWindowInfo.nXYWidth );
		}
		else
		{
			gtk_paned_set_position( GTK_PANED( m_pSplits[2] ), g_PrefsDlg.mWindowInfo.nCamWidth );
			gtk_paned_set_position( GTK_PANED( m_pSplits[3] ), g_PrefsDlg.mWindowInfo.nXYWidth );
		}

		gtk_paned_set_position( GTK_PANED( m_pSplits[1] ), g_PrefsDlg.mWindowInfo.nCamHeight );
	}
	else if ( CurrentStyle() == eFloating ) {
		{
			GtkWidget* wnd = create_floating( this );
			gtk_window_set_title( GTK_WINDOW( wnd ), _( "Camera" ) );

#ifdef _WIN32
			if ( g_PrefsDlg.m_bStartOnPrimMon ) {
				PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posCamWnd );
			}
#endif
			load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posCamWnd );

			gtk_widget_show( wnd );

			m_pCamWnd = new CamWnd();

			{
				GtkWidget* frame = create_framed_widget( m_pCamWnd->GetWidget() );
				gtk_container_add( GTK_CONTAINER( wnd ), frame );
			}

			m_pCamWnd->m_pParent = wnd;
		}

		if ( g_PrefsDlg.m_bFloatingZ ) {
			m_pZWnd = create_floating_zwnd( this );

			{
				GtkWidget* wnd = create_floating( this );
				gtk_window_set_title( GTK_WINDOW( wnd ), _( "XY View" ) );

#ifdef _WIN32
				if ( g_PrefsDlg.m_bStartOnPrimMon ) {
					PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posXYWnd );
				}
#endif
				load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posXYWnd );

				m_pXYWnd = new XYWnd();
				m_pXYWnd->SetViewType( XY );

				{
					GtkWidget* frame = create_framed_widget( m_pXYWnd->GetWidget() );
					gtk_container_add( GTK_CONTAINER( wnd ), frame );
				}

				m_pXYWnd->m_pParent = wnd;

				gtk_widget_show( wnd );
			}
		}
		else
		{
			GtkWidget* wnd = create_floating( this );
			gtk_window_set_title( GTK_WINDOW( wnd ), _( "XY View" ) );

#ifdef _WIN32
			if ( g_PrefsDlg.m_bStartOnPrimMon ) {
				PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posXYWnd );
			}
#endif
			load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posXYWnd );

			m_pZWnd = new ZWnd();
			m_pZWnd->m_pParent = wnd;

			m_pXYWnd = new XYWnd();
			m_pXYWnd->SetViewType( XY );
			m_pXYWnd->m_pParent = wnd;


			{
				GtkWidget* hsplit = gtk_hpaned_new();
				m_pSplits[0] = hsplit;
				gtk_container_add( GTK_CONTAINER( wnd ), hsplit );
				gtk_widget_show( hsplit );

				{
					GtkWidget* frame = create_framed_widget( m_pZWnd->GetWidget() );
					gtk_paned_add1( GTK_PANED( hsplit ), frame );
				}
				{
					GtkWidget* frame = create_framed_widget( m_pXYWnd->GetWidget() );
					gtk_paned_add2( GTK_PANED( hsplit ), frame );
				}
			}

			gtk_widget_show( wnd );

			gtk_paned_set_position( GTK_PANED( m_pSplits[0] ), g_PrefsDlg.mWindowInfo.nZFloatWidth );
		}

		{
			GtkWidget* wnd = create_floating( this );
			gtk_window_set_title( GTK_WINDOW( wnd ), _( "XZ View" ) );

#ifdef _WIN32
			if ( g_PrefsDlg.m_bStartOnPrimMon ) {
				PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posXZWnd );
			}
#endif
			load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posXZWnd );

			m_pXZWnd = new XYWnd();
			m_pXZWnd->m_pParent = wnd;
			m_pXZWnd->SetViewType( XZ );

			{
				GtkWidget* frame = create_framed_widget( m_pXZWnd->GetWidget() );
				gtk_container_add( GTK_CONTAINER( wnd ), frame );
			}

			if ( g_PrefsDlg.m_bXZVis ) {
				gtk_widget_show( wnd );
			}
		}

		{
			GtkWidget* wnd = create_floating( this );
			gtk_window_set_title( GTK_WINDOW( wnd ), _( "YZ View" ) );

#ifdef _WIN32
			if ( g_PrefsDlg.m_bStartOnPrimMon ) {
				PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posYZWnd );
			}
#endif
			load_window_pos( wnd, g_PrefsDlg.mWindowInfo.posYZWnd );

			m_pYZWnd = new XYWnd();
			m_pYZWnd->m_pParent = wnd;
			m_pYZWnd->SetViewType( YZ );

			{
				GtkWidget* frame = create_framed_widget( m_pYZWnd->GetWidget() );
				gtk_container_add( GTK_CONTAINER( wnd ), frame );
			}

			if ( g_PrefsDlg.m_bYZVis ) {
				gtk_widget_show( wnd );
			}
		}

		m_pTexWnd = new TexWnd();
		{
			GtkWidget* frame = create_framed_texwnd( m_pTexWnd );
			m_pTexWnd->m_pParent = g_pGroupDlg->m_pWidget;

			GtkWidget* w = gtk_label_new( _( "Textures" ) );
			gtk_widget_show( w );

			if( g_PrefsDlg.m_bShowTexDirList )
			{
				GtkWidget* texDirList = create_texdirlist_widget();

				GtkWidget* texSplit = gtk_hpaned_new();
				m_pSplits[4] = texSplit;

				gtk_paned_add1( GTK_PANED( texSplit ), texDirList );
				gtk_paned_add2( GTK_PANED( texSplit ), frame );

				if( g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth >= 0 ) {
					gtk_paned_set_position( GTK_PANED( texSplit ), g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth );
				}

				gtk_widget_show( texSplit );

				gtk_notebook_insert_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), texSplit, w, 1 );
			} else
			{
				m_pSplits[4] = NULL;
				gtk_notebook_insert_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), frame, w, 1 );
			}
		}

		g_pGroupDlg->Show();
	}
	else // 4 way
	{
		{
			GtkWidget* hsplit = gtk_hpaned_new();
			m_pSplits[0] = hsplit;
			gtk_box_pack_start( GTK_BOX( vbox ), hsplit, TRUE, TRUE, 0 );
			gtk_widget_show( hsplit );

			{
				GtkWidget* vsplit1 = gtk_vpaned_new();
				m_pSplits[1] = vsplit1;
				gtk_paned_add1( GTK_PANED( hsplit ), vsplit1 );
				gtk_widget_show( vsplit1 );

				{
					GtkWidget* vsplit2 = gtk_vpaned_new();
					m_pSplits[2] = vsplit2;
					gtk_paned_add2( GTK_PANED( hsplit ), vsplit2 );
					gtk_widget_show( vsplit2 );

					m_pCamWnd = new CamWnd();
					{
						GtkWidget* frame = create_framed_widget( m_pCamWnd->GetWidget() );
						gtk_paned_add1( GTK_PANED( vsplit1 ), frame );
					}

					m_pXYWnd = new XYWnd();
					m_pXYWnd->SetViewType( XY );
					{
						GtkWidget* frame = create_framed_widget( m_pXYWnd->GetWidget() );
						gtk_paned_add1( GTK_PANED( vsplit2 ), frame );
					}

					m_pYZWnd = new XYWnd();
					m_pYZWnd->SetViewType( YZ );
					{
						GtkWidget* frame = create_framed_widget( m_pYZWnd->GetWidget() );
						gtk_paned_add2( GTK_PANED( vsplit1 ), frame );
					}

					m_pXZWnd = new XYWnd();
					m_pXZWnd->SetViewType( XZ );
					{
						GtkWidget* frame = create_framed_widget( m_pXZWnd->GetWidget() );
						gtk_paned_add2( GTK_PANED( vsplit2 ), frame );
					}
				}
			}
		}

		{
			m_pTexWnd = new TexWnd();
			GtkWidget* frame = create_framed_texwnd( m_pTexWnd );

			GtkWidget* w = gtk_label_new( _( "Textures" ) );
			gtk_widget_show( w );

			if( g_PrefsDlg.m_bShowTexDirList )
			{
				GtkWidget* texDirList = create_texdirlist_widget();

				GtkWidget* texSplit = gtk_hpaned_new();
				m_pSplits[4] = texSplit;

				gtk_paned_add1( GTK_PANED( texSplit ), texDirList );
				gtk_paned_add2( GTK_PANED( texSplit ), frame );

				if( g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth >= 0 ) {
					gtk_paned_set_position( GTK_PANED( texSplit ), g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth );
				}

				gtk_widget_show( texSplit );

				gtk_notebook_insert_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), texSplit, w, 1 );
			} else
			{
				m_pSplits[4] = NULL;
				gtk_notebook_insert_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), frame, w, 1 );
			}
		}

		m_pTexWnd->m_pParent = g_pGroupDlg->m_pWidget;
		m_pZWnd = create_floating_zwnd( this );

		while ( gtk_events_pending() )
			gtk_main_iteration();

		{
			int x = GTK_PANED( m_pSplits[0] )->max_position / 2 - gutter;
			gtk_paned_set_position( GTK_PANED( m_pSplits[0] ), x );
		}

		{
			int y = GTK_PANED( m_pSplits[1] )->max_position / 2 - gutter;
			gtk_paned_set_position( GTK_PANED( m_pSplits[1] ), y );
			gtk_paned_set_position( GTK_PANED( m_pSplits[2] ), y );
		}
	}

	if ( g_PrefsDlg.mWindowInfo.nState & GDK_WINDOW_STATE_MAXIMIZED ) {
		gtk_window_maximize( GTK_WINDOW( window ) );
	}

	gtk_widget_show( window );

	Texture_Init();

	if ( m_pXYWnd ) { // this is always true?
		m_pXYWnd->SetActive( true );
	}
	m_bSplittersOK = true;
	Texture_SetMode( g_qeglobals.d_savedinfo.iTexMenu );

	g_pParentWnd->OnEntitiesSetViewAs( 0 );

	LoadCommandMap();
	ShowMenuItemKeyBindings( window );

	if ( g_qeglobals_gui.d_edit != NULL ) {
		console_construct( g_qeglobals_gui.d_edit );
	}

	//  bool load_last = FALSE;

	SetGridStatus();
	SetButtonMenuStates();

	// m_bShowShader and m_bTextureShaderlistOnly have a menu checkbox, update it now
	GtkWidget *item;
	g_bIgnoreCommands++;
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_shaders_show" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bShowShaders ? TRUE : FALSE );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_emptydirs_hide" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bHideEmptyDirs ? TRUE : FALSE );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_shaderlistonly" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bTexturesShaderlistOnly ? TRUE : FALSE );
	g_bIgnoreCommands--;

	SetActiveXY( m_pXYWnd );

	s_idle_id = g_timeout_add( 25, mainframe_idle, this );

	QGL_InitExtensions();

	if ( g_PrefsDlg.mLocalPrefs.mbEmpty ) {
		g_PrefsDlg.mLocalPrefs.mbEmpty = false;
		g_PrefsDlg.SavePrefs();
	}

	// remove the pid file
	remove( g_pidGameFile.GetBuffer() );

	Sys_Printf( "Entering message loop\n" );

	m_bDoLoop = true;

	m_nTimer = g_timeout_add( 1000, timer, this );
}

// =============================================================================
// MainFrame class

MainFrame::MainFrame(){
	m_bDoLoop = false;
	m_bSplittersOK = false;
	g_pParentWnd = this;
	m_pXYWnd = (XYWnd*)NULL;
	m_pCamWnd = NULL;
	m_pTexWnd = (TexWnd*)NULL;
	m_pZWnd = (ZWnd*)NULL;
	m_pYZWnd = (XYWnd*)NULL;
	m_pXZWnd = (XYWnd*)NULL;
	m_pActiveXY = (XYWnd*)NULL;
	m_bCamPreview = true;
	m_pWatchBSP = NULL;
	for ( int n = 0; n < 6; n++ )
		m_pStatusLabel[n] = NULL;
	m_bNeedStatusUpdate = false;
	m_nTimer = 0;
	m_bSleeping = false;
	Create();
}

MainFrame::~MainFrame(){
	while ( g_BSPFrontendCommands )
	{
		free( g_BSPFrontendCommands->data );
		g_BSPFrontendCommands = g_slist_remove( g_BSPFrontendCommands, g_BSPFrontendCommands->data );
	}
}

void MainFrame::ReleaseContexts(){
	if ( m_pXYWnd ) {
		m_pXYWnd->DestroyContext();
	}
	if ( m_pYZWnd ) {
		m_pYZWnd->DestroyContext();
	}
	if ( m_pXZWnd ) {
		m_pXZWnd->DestroyContext();
	}
	if ( m_pCamWnd ) {
		m_pCamWnd->DestroyContext();
	}
	if ( m_pTexWnd ) {
		m_pTexWnd->DestroyContext();
	}
	if ( m_pZWnd ) {
		m_pZWnd->DestroyContext();
	}
}

void MainFrame::CreateContexts(){
	if ( m_pCamWnd ) {
		m_pCamWnd->CreateContext();
	}
	if ( m_pXYWnd ) {
		m_pXYWnd->CreateContext();
	}
	if ( m_pYZWnd ) {
		m_pYZWnd->CreateContext();
	}
	if ( m_pXZWnd ) {
		m_pXZWnd->CreateContext();
	}
	if ( m_pTexWnd ) {
		m_pTexWnd->CreateContext();
	}
	if ( m_pZWnd ) {
		m_pZWnd->CreateContext();
	}
}

static void Sys_Iconify( GtkWidget *w ){
	// we might not have been realized yet
	if ( gtk_widget_get_window( w ) == NULL ) {
		return;
	}

	if ( !gtk_widget_get_mapped( w ) ) {
		return;
	}

	gtk_window_iconify( GTK_WINDOW( w ) );
}

static void Sys_Restore( GtkWidget *w ){
	// we might not have been realized yet
	if ( gtk_widget_get_window( w ) == NULL ) {
		return;
	}

	if ( !gtk_widget_get_visible( w ) ) {
		return;
	}

	gtk_window_deiconify( GTK_WINDOW( w ) );
}

#ifdef _DEBUG
//#define DBG_SLEEP
#endif

void RefreshModelSkin( GSList **pModels, entitymodel_t *model ){
	//++timo FIXME: the are some bogus entitymodel_t that appear in the list cause of buggy HasModel
	// so we avoid the fucked up ones, assuming they are at the end
	if ( !model->strSkin ) {
#ifdef DBG_SLEEP
		Sys_Printf( "Dropping model %p with empty skin in RefreshModelSkin\n", model );
#endif

		// and also keeping it so we have an actual count of empty models
		*pModels = g_slist_append( *pModels, model );
		return;
	}
	// do we have this model already?
	if ( g_slist_find( *pModels, model ) ) {
#ifdef DBG_SLEEP

		// looks like we don't have the filename for the model, only the skin name and tris.. so we put the adress
		Sys_Printf( "Already processed model: %p %s\n", model, ( (GString *)model->strSkin )->str );
#endif
		return;
	}
	model->nTextureBind = Texture_LoadSkin( ( (GString *)model->strSkin )->str, &model->nSkinWidth, &model->nSkinHeight );
	if ( model->nTextureBind != -1 ) {
		Sys_Printf( "LOADED SKIN: %s\n", ( (GString *)model->strSkin )->str );
	}
	else{
		Sys_Printf( "Load skin failed: %s\n", ( (GString *)model->strSkin )->str );
	}
	*pModels = g_slist_append( *pModels, model );
#ifdef DBG_SLEEP
	Sys_Printf( "Processed model %p %s\n", model, ( (GString *)model->strSkin )->str );
#endif
}

void MainFrame::OnSleep(){
	m_bSleeping ^= 1;
	if ( m_bSleeping ) {
		// useful when trying to debug crashes in the sleep code
		Sys_Printf( "Going into sleep mode..\n" );

		Sys_Printf( "Dispatching sleep msg..." );
		DispatchRadiantMsg( RADIANT_SLEEP );
		Sys_Printf( "Done.\n" );

		if ( CurrentStyle() == eSplit ) {
			Sys_Iconify( m_pZWnd->m_pParent );
		}

		Sys_Iconify( m_pWidget );
		Select_Deselect();
		QERApp_FreeShaders();
		g_bScreenUpdates = false;

		// release contexts
		Sys_Printf( "Releasing contexts..." );
		ReleaseContexts();
		Sys_Printf( "Done.\n" );

		// free all the skins in the caches
		// their GL resources have been freed but the structs are not, so Radiant would think they are still valid
		g_lstSkinCache.RemoveAll();
	}
	else
	{
		Sys_Printf( "Waking up\n" );
		if ( CurrentStyle() == eSplit ) {
			Sys_Restore( m_pZWnd->m_pParent );
		}

		Sys_Restore( m_pWidget );

		// create contexts
		Sys_Printf( "Creating contexts..." );
		CreateContexts();
		Sys_Printf( "Done.\n" );

		Sys_Printf( "Making current on camera..." );
		m_pCamWnd->MakeCurrent();
		Sys_Printf( "Done.\n" );

		Sys_Printf( "Reloading shaders..." );
		// reload the shader scripts and textures
		QERApp_ReloadShaders();
		// current shader
		// NOTE: we are kinda making it loop on itself, it will update the pShader and scroll the texture window
		Texture_SetTexture( &g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef, false, NULL, false );
		Sys_Printf( "Done.\n" );

		// rebuild the patches by setting the bDirty flag on them
		for ( brush_t* b = active_brushes.next ; b != &active_brushes ; b = b->next )
		{
			if ( b->patchBrush ) {
				b->pPatch->bDirty = true;
			}
		}

		Sys_Printf( "Reloading skins..." );
		// we have purged all the skins before going to sleep
		// to rebuild, go through everything that needs a skin and call Texture_LoadSkin
		// namely, all entitymodel_t
		// since there's no direct list we go through entities to get the eclass_t and from there the entitymodel_t
		// (a single eclass_t can reference several entitymodel_t)
		// FIXME: and what's up with md3Class then? what is it used for?
/*
    eclass_t *e;
    entity_t *ent;
    GSList *Models = NULL;
    for (ent = entities.next; ent != &entities; ent = ent->next)
    {
      // if it's a model with skin then the fixedsize flag must be on
      // only if there IS a model .. we are not trying to load
      if (ent->eclass->fixedsize)
      {
        if (ent->eclass->model)
        {
   #ifdef DBG_SLEEP
          if (ent->md3Class)
            Sys_FPrintf(SYS_WRN, "WARNING: unexpected ent->md3Class!=NULL with ent->eclass->model!=NULL\n");
   #endif
          entitymodel_t *model;
          for (model = ent->eclass->model; model; model=model->pNext)
            RefreshModelSkin (&Models, model);
        } else if (ent->md3Class)
        {
          entitymodel_t *model;
          for (model = ent->md3Class->model; model; model=model->pNext)
            RefreshModelSkin (&Models, model);
        }
   #ifdef DBG_SLEEP
        else
          Sys_FPrintf(SYS_WRN, "WARNING: entity %p %s with fixedsize and no model no md3Class\n", ent, ent->eclass->name);
   #endif
      }
    }
   #ifdef DBG_SLEEP
    for (e = g_md3Cache; e ; e = e->next)
    {
      entitymodel_t *model;
      for (model = e->model; model; model=model->pNext)
        if (!g_slist_find (Models, model))
        {
          Sys_Printf("model %p ", model);
          if (model->strSkin)
            Sys_Printf("%s not found in main loop\n", ((GString *)model->strSkin)->str);
          else
            Sys_Printf("not found in main loop (no skin)\n");
        }
    }
   #endif
    // clean the model list
    g_slist_free (Models);
 */
		Sys_Printf( "Done.\n" );

		g_bScreenUpdates = true;

		Sys_Printf( "Dispatching wake msg..." );
		DispatchRadiantMsg( RADIANT_WAKEUP );
		Sys_Printf( "Done\n" );
	}
}

void WINAPI QERApp_Sleep(){
	g_pParentWnd->OnSleep();
}

/*!
   NOTE TTimo
   the exit path is a bit complicated, I guess we have to run the window pos saving in OnDelete
   and not in OnDestroy because the info may be lost already?
   \todo try sinking OnDelete into OnDestroy and see if it breaks anything
 */
void MainFrame::OnDelete(){
	save_window_pos( m_pWidget, g_PrefsDlg.mWindowInfo.position );

	// surface inspector and patch inspector
	save_window_pos( g_dlgSurface.GetWidget(), g_PrefsDlg.mWindowInfo.posSurfaceWnd );
	save_window_pos( g_PatchDialog.GetWidget(), g_PrefsDlg.mWindowInfo.posPatchWnd );

	// entity inspector / group dialog
	// NOTE TTimo do we have to save a different window depending on the view mode?
	save_window_pos( g_pGroupDlg->m_pWidget, g_PrefsDlg.mWindowInfo.posEntityWnd );

	if ( g_PrefsDlg.m_bFloatingZ ) {
		save_window_pos( m_pZWnd->m_pParent, g_PrefsDlg.mWindowInfo.posZWnd );
	}
	else{
		g_PrefsDlg.mWindowInfo.nZFloatWidth = gtk_paned_get_position( GTK_PANED( m_pSplits[0] ) );
	}
	if( g_PrefsDlg.m_bShowTexDirList && m_pSplits[4] ) {
		g_PrefsDlg.mWindowInfo.nTextureDirectoryListWidth = gtk_paned_get_position( GTK_PANED( m_pSplits[4] ) );
	}

	if ( CurrentStyle() == eFloating ) {
		save_window_pos( m_pCamWnd->m_pParent, g_PrefsDlg.mWindowInfo.posCamWnd );
		save_window_pos( m_pXYWnd->m_pParent, g_PrefsDlg.mWindowInfo.posXYWnd );
		save_window_pos( m_pXZWnd->m_pParent, g_PrefsDlg.mWindowInfo.posXZWnd );
		save_window_pos( m_pYZWnd->m_pParent, g_PrefsDlg.mWindowInfo.posYZWnd );
	}

	g_PrefsDlg.mWindowInfo.nState = gdk_window_get_state( gtk_widget_get_window( g_pParentWnd->m_pWidget ) );
}

void MainFrame::OnDestroy(){
	// shut down console output first
	// (we'll still get the info if we are running a log file anyway)
	g_qeglobals_gui.d_edit = NULL;

#ifdef _DEBUG
	Sys_Printf( "MainFrame::OnDestroy\n" );
#endif
	if ( s_idle_id ) {
		g_source_remove( s_idle_id );
	}
	if ( m_nTimer ) {
		g_source_remove( m_nTimer );
	}

	if ( !g_qeglobals.disable_ini ) {
		Sys_Printf( "Start writing prefs\n" );
		Sys_Printf( "MRU_Save... " );
		MRU_Save();
		Sys_Printf( "OK\n" );

		gpointer w;

		w = g_object_get_data( G_OBJECT( g_pGroupDlg->m_pWidget ), "split1" );
		g_PrefsDlg.mWindowInfo.nEntitySplit1 = gtk_paned_get_position( GTK_PANED( w ) );
		w = g_object_get_data( G_OBJECT( g_pGroupDlg->m_pWidget ), "split2" );
		g_PrefsDlg.mWindowInfo.nEntitySplit2 = gtk_paned_get_position( GTK_PANED( w ) );

		if ( !FloatingGroupDialog() ) {
			GtkWidget *vsplit, *hsplit, *vsplit2, *hsplit2;

			vsplit = m_pSplits[0];
			vsplit2 = m_pSplits[1];
			hsplit = m_pSplits[2];
			hsplit2 = m_pSplits[3];

			g_PrefsDlg.mWindowInfo.nXYHeight  = gtk_paned_get_position( GTK_PANED( vsplit ) );
			g_PrefsDlg.mWindowInfo.nXYWidth   = gtk_paned_get_position( GTK_PANED( hsplit2 ) );

			if ( CurrentStyle() == eRegular ) {
				g_PrefsDlg.mWindowInfo.nZWidth = gtk_paned_get_position( GTK_PANED( hsplit ) );
			}
			else{
				g_PrefsDlg.mWindowInfo.nCamWidth = gtk_paned_get_position( GTK_PANED( hsplit ) );
			}

			g_PrefsDlg.mWindowInfo.nCamHeight = gtk_paned_get_position( GTK_PANED( vsplit2 ) );
		}
		else
		{
			if ( g_PrefsDlg.m_bFloatingZ || CurrentStyle() == eSplit ) {
				if ( gtk_widget_get_visible( m_pZWnd->m_pParent ) ) {
					g_PrefsDlg.m_bZVis = TRUE;
				}
				else{
					g_PrefsDlg.m_bZVis = FALSE;
				}
			}
		}
		g_PrefsDlg.SavePrefs();
		Sys_Printf( "Done prefs\n" );
	}

	// spog - this may be better in another place..
	// deletes filters list and assigns g_qeglobals.d_savedinfo.filters = NULL
	g_qeglobals.d_savedinfo.filters = FilterListDelete( g_qeglobals.d_savedinfo.filters );

	delete m_pXYWnd; m_pXYWnd = NULL;
	delete m_pYZWnd; m_pYZWnd = NULL;
	delete m_pXZWnd; m_pXZWnd = NULL;
	delete m_pZWnd; m_pZWnd = NULL;
	delete m_pTexWnd; m_pTexWnd = NULL;
	delete m_pCamWnd; m_pCamWnd = NULL;

	if ( g_pGroupDlg->m_pWidget ) {
		//!\todo fix "Gtk-CRITICAL **: file gtknotebook.c: line 4643 (gtk_notebook_get_tab_label): assertion `GTK_IS_WIDGET (child)' failed"
		gtk_widget_destroy( g_pGroupDlg->m_pWidget );
		g_pGroupDlg->m_pWidget = NULL;
	}

	if ( strcmpi( currentmap, "unnamed.map" ) != 0 ) {
		g_PrefsDlg.m_strLastMap = currentmap;
		g_PrefsDlg.SavePrefs();
	}
	Sys_Printf( "CleanUpEntities..." );
	CleanUpEntities();
	Sys_Printf( "Done.\n" );

	Sys_Printf( "Releasing brushes..." );
	while ( active_brushes.next != &active_brushes )
		Brush_Free( active_brushes.next, false );
	while ( selected_brushes.next != &selected_brushes )
		Brush_Free( selected_brushes.next, false );
	while ( filtered_brushes.next != &filtered_brushes )
		Brush_Free( filtered_brushes.next, false );
	Sys_Printf( "Done.\n" );

	Sys_Printf( "Releasing entities..." );
	while ( entities.next != &entities )
		Entity_Free( entities.next );
	Sys_Printf( "Done.\n" );

	epair_t* pEPair = g_qeglobals.d_project_entity->epairs;
	while ( pEPair )
	{
		epair_t* pNextEPair = pEPair->next;
		free( pEPair->key );
		free( pEPair->value );
		free( pEPair );
		pEPair = pNextEPair;
	}

	entity_t* pEntity = g_qeglobals.d_project_entity->next;
	while ( pEntity != NULL && pEntity != g_qeglobals.d_project_entity )
	{
		entity_t* pNextEntity = pEntity->next;
		Entity_Free( pEntity );
		pEntity = pNextEntity;
	}

	Sys_Printf( "Freeing world entity..." );
	if ( world_entity ) {
		Entity_Free( world_entity );
	}
	Sys_Printf( "Done.\n" );

	Sys_Printf( "Shutdown VFS..." );
	vfsShutdown();
	Sys_Printf( "Done.\n" );

	Sys_Printf( "FreeShaders..." );
	QERApp_FreeShaders();
	Sys_Printf( "Done.\n" );
}

// TTimo: now using profile.cpp code
void MainFrame::LoadCommandMap(){
	FILE *f;
	CString strINI;
	bool bUserCmdList = false;
	int nLen;
	// verbose a little: count of user commands we recognized
	int iCount = 0;
	int iOverrideCount = 0;
	int j;


#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	strINI = g_PrefsDlg.m_rc_path->str;
#elif defined( WIN32 )
	strINI = g_strGameToolsPath;
#else
#error "WTF are you compiling this on"
#endif
	AddSlash( strINI );
	strINI += "shortcuts.ini";

	f = fopen( strINI.GetBuffer(), "r" );
	if ( f != NULL ) {
		fclose( f );
		// loop through all the commands
		for ( int i = 0; i < g_nCommandCount; i++ )
		{
			char value[1024];
			if ( read_var( strINI.GetBuffer(), "Commands", g_Commands[i].m_strCommand, value ) ) {
				if ( !bUserCmdList ) {
					Sys_Printf( "Found user's shortcuts list at %s\n", strINI.GetBuffer() );
					bUserCmdList = true;
				}
				CString strBuff;
				strBuff = value;
				strBuff.TrimLeft();
				strBuff.TrimRight();
				strBuff.MakeLower();
				int nSpecial = strBuff.Find( "+alt" );
				g_Commands[i].m_nModifiers = 0;
				if ( nSpecial >= 0 ) {
					g_Commands[i].m_nModifiers |= RAD_ALT;
					FindReplace( strBuff, "+alt", "" );
				}
				nSpecial = strBuff.Find( "+ctrl" );
				if ( nSpecial >= 0 ) {
					g_Commands[i].m_nModifiers |= RAD_CONTROL;
					FindReplace( strBuff, "+ctrl", "" );
				}
				nSpecial = strBuff.Find( "+shift" );
				if ( nSpecial >= 0 ) {
					g_Commands[i].m_nModifiers |= RAD_SHIFT;
					FindReplace( strBuff, "+shift", "" );
				}
				strBuff.TrimLeft();
				strBuff.TrimRight();
				strBuff.MakeUpper();
				// strBuff has been cleaned of it's modifiers .. switch between a regular key and a virtual one
				// based on length
				nLen = strBuff.GetLength();
				if ( nLen == 1 ) { // most often case.. deal with first
					g_Commands[i].m_nKey = toascii( strBuff.GetAt( 0 ) );
					iCount++;
				}
				else // special key
				{
					for ( j = 0; j < g_nKeyCount; j++ )
					{
						if ( strBuff.CompareNoCase( g_Keys[j].m_strName ) == 0 ) {
							g_Commands[i].m_nKey = g_Keys[j].m_nVKKey;
							iCount++;
							break;
						}
					}
					if ( j == g_nKeyCount ) {
						Sys_FPrintf( SYS_WRN, "WARNING: failed to parse user command %s\n", value );
						continue;
					}
				}
				// maybe this new shortcut is overriding another one
				// then we need to disable the other binded key
				for ( j = 0; j < g_nCommandCount; j++ )
				{
					if ( j == i ) {
						continue;
					}
					if ( g_Commands[i].m_nKey == g_Commands[j].m_nKey && g_Commands[i].m_nModifiers == g_Commands[j].m_nModifiers ) {
						// found!
						g_Commands[j].m_nKey = 0;
						// verbose
						iOverrideCount++;
						// it's the only one
						break;
					}
				}
			}
		}
		if ( iOverrideCount ) {
			Sys_Printf( "User's command list overrides %d default commands\n", iOverrideCount );
		}
		Sys_Printf( "Parsed %d custom shortcuts\n", iCount );
	}
	else{
		Sys_Printf( "Looked for a '%s' keyboard shortcuts file, not found\n", strINI.GetBuffer() );
	}
}

// TTimo: an m_nKey can be set to zero if there's no shorcut binded
// we also output the count of commands that are not binded .. dunno if it's much use ..
// (non-binded keys are usually keys that were defined by shortcuts overriden by user prefs)
void MainFrame::ShowMenuItemKeyBindings( GtkWidget* window ){
	//!\todo Find a better way to get the global accelerator group..
	GtkAccelGroup *accel = GTK_ACCEL_GROUP( gtk_accel_groups_from_object( G_OBJECT( window ) )->data );
	gpointer item;
	guint mods;
	int i;
	int iCount = 0;

	for ( i = 0; i < g_nCommandCount; i++ )
	{
		if ( g_Commands[i].m_nKey == 0 ) {
			iCount++;
			continue;
		}

		item = g_object_get_data( G_OBJECT( m_pWidget ), g_Commands[i].m_strMenu );
		if ( item == NULL ) {
			Sys_FPrintf( SYS_WRN, "WARNING: keyboard shortcuts init, no menu item found for command: \"%s\"\n",
						 g_Commands[i].m_strCommand );
			continue;
		}

		mods = 0;
		if ( g_Commands[i].m_nModifiers ) { // are there modifiers present?
			if ( g_Commands[i].m_nModifiers & RAD_SHIFT ) {
				mods |= GDK_SHIFT_MASK;
			}
			if ( g_Commands[i].m_nModifiers & RAD_ALT ) {
				mods |= GDK_MOD1_MASK;
			}
			if ( g_Commands[i].m_nModifiers & RAD_CONTROL ) {
				mods |= GDK_CONTROL_MASK;
			}
		}

		// GTK won't add accelerators for some keys (ex.: delete), so we have to do it manually
		if ( gtk_accelerator_valid( g_Commands[i].m_nKey, (GdkModifierType)mods ) ) {
#ifdef DBG_KBD
			// NOTE TTimo this is the important place where all the shortcuts are binded
			Sys_Printf( "Calling gtk_widget_add_accelerator on command: %s menu: %s key: %d mods: %d\n", g_Commands[i].m_strCommand, g_Commands[i].m_strMenu, g_Commands[i].m_nKey, mods );
#endif
			gtk_widget_add_accelerator( GTK_WIDGET( item ), "activate", accel, g_Commands[i].m_nKey,
										(GdkModifierType)mods, GTK_ACCEL_VISIBLE );
		}
		else
		{
			GtkAccelLabel *accel_label = GTK_ACCEL_LABEL( GTK_BIN( item )->child );
			GString *gstring;
			gboolean had_mod;

			g_free( accel_label->accel_string );
			accel_label->accel_string = NULL;

			gstring = g_string_new( accel_label->accel_string );
			g_string_append( gstring, "   " );

			had_mod = FALSE;
			if ( mods & GDK_SHIFT_MASK ) {
				g_string_append( gstring, "Shft" );
				had_mod = TRUE;
			}
			if ( mods & GDK_CONTROL_MASK ) {
				if ( had_mod ) {
					g_string_append( gstring, "+" );
				}
				g_string_append( gstring, "Ctl" );
				had_mod = TRUE;
			}
			if ( mods & GDK_MOD1_MASK ) {
				if ( had_mod ) {
					g_string_append( gstring, "+" );
				}
				g_string_append( gstring, "Alt" );
				had_mod = TRUE;
			}

			if ( had_mod ) {
				g_string_append( gstring, "+" );
			}
			if ( g_Commands[i].m_nKey < 0x80 || ( g_Commands[i].m_nKey > 0x80 && g_Commands[i].m_nKey <= 0xff ) ) {
				switch ( g_Commands[i].m_nKey )
				{
				case ' ':
					g_string_append( gstring, "Space" );
					break;
				case '\\':
					g_string_append( gstring, "Backslash" );
					break;
				default:
					g_string_append_c( gstring, toupper( g_Commands[i].m_nKey ) );
					break;
				}
			}
			else
			{
				gchar *tmp;

				tmp = gtk_accelerator_name( g_Commands[i].m_nKey, (GdkModifierType)0 );
				if ( tmp[0] != 0 && tmp[1] == 0 ) {
					tmp[0] = toupper( tmp[0] );
				}
				g_string_append( gstring, tmp );
				g_free( tmp );
			}

			g_free( accel_label->accel_string );
			accel_label->accel_string = gstring->str;
			g_string_free( gstring, FALSE );

			if ( !accel_label->accel_string ) {
				accel_label->accel_string = g_strdup( "" );
			}

			gtk_widget_queue_resize( GTK_WIDGET( accel_label ) );
		}
	}

	if ( iCount ) {
		Sys_Printf( "%d commands not bound to a key\n", iCount );
	}
}

// Checks whether a given filename ends in .map
const bool IsMap(const char* filename){
	return strlen(filename) >= 4 && strcmp(filename + strlen(filename) - 4, ".map") == 0;
}

void MainFrame::CreateQEChildren(){
	// load the project file, if it is a project file. (Or at least no .map)
	if ( g_argc > 1 && !IsMap( g_argv[1] ) ) {
		Sys_Printf( "loading project file from the command line: %s\n", g_argv[1] );
		if ( !QE_LoadProject( g_argv[1] ) ) {
			Error( "Unable to load project file %s\n", g_argv[1] );
		}
	}
	else
	{
		const char* filename = NULL;
		char buf[PATH_MAX];
		const char *r;
		bool bTriedTemplate = false;
        int templateVersion = 0;

		if ( g_PrefsDlg.m_nLastProjectVer != 0 && g_PrefsDlg.m_nLastProjectVer != PROJECT_VERSION ) {
			// we need to regenerate from template
			Sys_Printf( "last project has version %d, this binary wants version %d - regenerating from the template\n", g_PrefsDlg.m_nLastProjectVer, PROJECT_VERSION );
			g_PrefsDlg.m_strLastProject = "";
		}

        
        // check to see if the project template is versioned
        strcpy( buf, g_pGameDescription->mEnginePath.GetBuffer() );
        strcat( buf, g_pGameDescription->mBaseGame.GetBuffer() );
        strcat( buf, "/scripts/" );
        strcat( buf, PROJECT_TEMPLATE_NAME );
        templateVersion = QE_GetTemplateVersionForProject( buf );

		r = g_PrefsDlg.m_strLastProject.GetBuffer();

		while ( r == NULL || *r == '\0' || access( r, R_OK ) != 0 || !QE_LoadProject( r ) || templateVersion != IntForKey( g_qeglobals.d_project_entity, "template_version" ) )
		{
			if ( !bTriedTemplate ) {
				// try default project location
				bTriedTemplate = true;
				// for all OSes, we look for the template in the base installation (no homepath here)
				strcpy( buf, g_pGameDescription->mEnginePath.GetBuffer() );
				strcat( buf, g_pGameDescription->mBaseGame.GetBuffer() );
				strcat( buf, "/scripts/" );
				strcat( buf, PROJECT_TEMPLATE_NAME );
				r = buf;
			}
			else
			{
				gtk_MessageBox( NULL, _( "Failed to load project file.\nPlease enter a valid project file." ), _( "Load Project" ) );

				filename = file_dialog( m_pWidget, TRUE, _( "Choose Project File" ), buf, "project" );
				if ( filename != NULL ) {
					r = filename;
				} else {
					Error( "Cannot continue without loading a project..." );
				}
			}
		}
	}

	QE_Init();
}

void MainFrame::OnTimer(){
	GdkModifierType mask;
#if GTK_CHECK_VERSION( 3, 0, 0 )
	GdkDeviceManager *device_manager;
	GdkDevice *pointer;
	GdkWindow *window;

	window = gtk_widget_get_window( m_pWidget );
	device_manager = gdk_display_get_device_manager( gdk_window_get_display( window ) );
	pointer = gdk_device_manager_get_client_pointer( device_manager );
	gdk_window_get_device_position( window, pointer, NULL, NULL, &mask );
#else
	GdkWindow *window;
	GdkDisplay *display;

	window = gtk_widget_get_window( m_pWidget );
	display = gdk_window_get_display( window );
	gdk_display_get_pointer( display, NULL, NULL, NULL, &mask );
#endif
	if ( ( mask & ( GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK ) ) == 0 ) {
		QE_CountBrushesAndUpdateStatusBar();
		QE_CheckAutoSave();
	}

	// see MainFrame::UpdateStatusText below
	if ( m_bNeedStatusUpdate ) {
		for ( int n = 0; n < 6; n++ )
		{
			if ( m_strStatus[n].GetLength() >= 0 && m_pStatusLabel[n] != NULL ) {
				gtk_label_set_text( GTK_LABEL( m_pStatusLabel[n] ), m_strStatus[n] );
			}
		}
		m_bNeedStatusUpdate = false;
	}
}

void MainFrame::UpdateStatusText(){
	m_bNeedStatusUpdate = true;
}

void MainFrame::SetStatusText( int nPane, const char* pText ){
	if ( pText && nPane <= 5 && nPane >= 0 ) {
		m_strStatus[nPane] = pText;
		UpdateStatusText();
	}
}
void MainFrame::SetButtonMenuStates(){
	GtkWidget *item;
	g_bIgnoreCommands++;

	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showangles" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.d_savedinfo.show_angles );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_shownames" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.d_savedinfo.show_names );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showcoordinates" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.d_savedinfo.show_coordinates );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showoutline" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.d_savedinfo.show_outline );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_selection_nooutline" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showaxes" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.d_savedinfo.show_axis );
	//item  = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_view_showpath"));
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS ) ? FALSE : TRUE );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_clusterportals" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLUSTERPORTALS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_lightgrid" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTGRID ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_world" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_entities" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_areaportals" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_AREAPORTALS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_translucent" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_TRANSLUCENT ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_liquids" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIQUIDS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_caulk" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_clips" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_botclips" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_BOTCLIP ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_structural" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_STRUCTURAL ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_paths" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_clusterportals" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLUSTERPORTALS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_lightgrid" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTGRID ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_lights" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_patches" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CURVES ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_details" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_DETAILS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_hintsskips" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_HINTSSKIPS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_models" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_MODELS ) != 0 );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_triggers" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									( g_qeglobals.d_savedinfo.exclude & EXCLUDE_TRIGGERS ) != 0 );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_toggle_lock" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_PrefsDlg.m_bTextureLock ) ? TRUE : FALSE );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_toggle_rotatelock" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_PrefsDlg.m_bRotateLock ) ? TRUE : FALSE );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_cubicclipping" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_PrefsDlg.m_bCubicClipping ) ? TRUE : FALSE );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_opengllighting" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_PrefsDlg.m_bGLLighting ) ? TRUE : FALSE );
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_snaptogrid" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_PrefsDlg.m_bSnap ) ? TRUE : FALSE );

	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_view_cubicclipping" ) );
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_PrefsDlg.m_bCubicClipping ) ? TRUE : FALSE );
	item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_dontselectmodel" ) );
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_PrefsDlg.m_bSelectModels ) ? FALSE : TRUE );

	if ( !g_pGameDescription->mNoPatch ) {
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_dontselectcurve" ) );
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_PrefsDlg.m_bSelectCurves ) ? FALSE : TRUE );

		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_showboundingbox" ) );
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchShowBounds ) ? TRUE : FALSE );
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_weld" ) );
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchWeld ) ? TRUE : FALSE );
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_drilldown" ) );
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchDrillDown ) ? TRUE : FALSE );
	}

	int id, n = g_PrefsDlg.m_nTextureScale;
	switch ( n )
	{
	case 10: id = ID_TEXTURES_TEXTUREWINDOWSCALE_10; break;
	case 25: id = ID_TEXTURES_TEXTUREWINDOWSCALE_25; break;
	case 50: id = ID_TEXTURES_TEXTUREWINDOWSCALE_50; break;
	case 200: id = ID_TEXTURES_TEXTUREWINDOWSCALE_200; break;
	default: id = ID_TEXTURES_TEXTUREWINDOWSCALE_100; break;
	}
	SetTextureScale( id );

	// FIXME TTimo cleaned up .. the right place to do this in QE_LoadProject?
/*
   if (g_qeglobals.d_project_entity)
   {
    FillTextureMenu();      // redundant but i'll clean it up later.. yeah right..
    FillBSPMenu();
   }
 */
	g_bIgnoreCommands--;
}

void MainFrame::UpdateWindows( int nBits ){
	if ( !g_bScreenUpdates ) {
		return;
	}
#ifdef DBG_WINDOWPOS
	static int bean_count = 0;
	char bean_buf[100];
	sprintf( bean_buf,"UpdateWindows %d",bean_count );
	CheckWatchit( bean_buf );
	bean_count++;
#endif

	if ( nBits & ( W_XY | W_XY_OVERLAY ) ) {
		if ( m_pXYWnd ) {
			m_pXYWnd->RedrawWindow();
		}
		if ( m_pXZWnd ) {
			m_pXZWnd->RedrawWindow();
		}
		if ( m_pYZWnd ) {
			m_pYZWnd->RedrawWindow();
		}
	}

	if ( nBits & W_CAMERA || ( ( nBits & W_CAMERA_IFON ) && m_bCamPreview ) ) {
		if ( m_pCamWnd ) {
			m_pCamWnd->RedrawWindow();
		}
	}

	if ( nBits & ( W_Z | W_Z_OVERLAY ) ) {
		if ( m_pZWnd ) {
			m_pZWnd->RedrawWindow();
		}
	}

	if ( nBits & W_TEXTURE ) {
		if ( m_pTexWnd ) {
			m_pTexWnd->RedrawWindow();
		}
	}
#ifdef DBG_WINDOWPOS
	sprintf( bean_buf,"%d (end UpdateWidows)",bean_count );
	CheckWatchit( bean_buf );
#endif
}

void MainFrame::RoutineProcessing(){
#ifdef DBG_WINDOWPOS
	static int bean_count = 0;
	char bean_buf[100];
	sprintf( bean_buf,"RoutineProcessing %d",bean_count );
	CheckWatchit( bean_buf );
	bean_count++;
#endif

	if ( m_bDoLoop ) {
		double time = 0.0;
		double oldtime = 0.0;
		double delta = 0.0;

/*    // checking KeyState works right
    static short a1,a2;
    a2 = GetKeyState(VK_MENU);
    if (a1!=a2)
    {
      Sys_Printf("VK_MENU: %d\n",a2);
      a1 = a2;
    }
    static short b1,b2;
    b2 = GetKeyState(VK_UP);
    if (b1!=b2)
    {
      Sys_Printf("VK_UP: %d\n",b2);
      b1 = b2;
    } */

		time = Sys_DoubleTime();
		delta = time - oldtime;
		oldtime = time;
		if ( delta > 0.2 ) {
			delta = 0.2;
		}

		// update the BSP process watcher
		if ( m_pWatchBSP ) {
			m_pWatchBSP->RoutineProcessing();
		}

		// run time dependant behavior
		if ( m_pCamWnd ) {
			m_pCamWnd->Cam_MouseControl( delta );
		}

		if ( g_nUpdateBits ) {
			int nBits = g_nUpdateBits; // this is done to keep this routine from being
			g_nUpdateBits = 0;        // re-entered due to the paint process.. only
			UpdateWindows( nBits );   // happens in rare cases but causes a stack overflow
		}
/*
    // Enable/disable the menu items
    GtkWidget *item;

    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_view_cameraupdate"));
    gtk_widget_set_sensitive (item, (m_bCamPreview == false));
    if (!g_PrefsDlg.m_bWideToolbar)
    {
      item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "ttb_view_cameraupdate"));
      gtk_widget_set_sensitive (item, (m_bCamPreview == false));
    }
    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_edit_undo"));
    gtk_widget_set_sensitive (item, Undo_UndoAvailable());
    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_edit_redo"));
    gtk_widget_set_sensitive (item, Undo_RedoAvailable());
    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_file_saveregion"));
    gtk_widget_set_sensitive (item, region_active);
    g_bIgnoreCommands++;
    // update the toolbar before displaying the menu:
    // show in use check box
    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_textures_showinuse"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), !g_bShowAllShaders);
    // show all check box
    item = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "menu_textures_showall"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), g_bShowAllShaders);
    g_bIgnoreCommands--;
 */
	}
#ifdef DBG_WINDOWPOS
	sprintf( bean_buf,"%d (end RoutineProcessing)",bean_count );
	CheckWatchit( bean_buf );
#endif
}

void MainFrame::DoWatchBSP(){
	// network monitoring of the BSP process
	if ( !m_pWatchBSP ) {
		m_pWatchBSP = new CWatchBSP();
	}
}

void MainFrame::CleanPlugInMenu(){
	GtkWidget *menu, *sep;
	GList *children, *seplst, *lst;

	// delete everything after the separator
	menu = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_plugin" ) );
	sep = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_plugin_separator" ) );
	m_nNextPlugInID = ID_PLUGIN_START;

	children = gtk_container_get_children( GTK_CONTAINER( menu ) );
	if( children ) {
		seplst = g_list_find( children, sep );
		if( seplst ) {
			for ( lst = g_list_next( seplst ); lst != NULL; lst = g_list_next( lst ) )
			{
				gtk_container_remove( GTK_CONTAINER( menu ), GTK_WIDGET( lst->data ) );
			}
		}
		g_list_free( children );
	}
}

void MainFrame::AddPlugInMenuItem( IPlugIn* pPlugIn ){
	GtkWidget *menu, *item, *parent;
	const char *menuText;

	parent = gtk_menu_item_new_with_label( pPlugIn->getMenuName() );
	gtk_widget_show( parent );
	gtk_container_add( GTK_CONTAINER( g_object_get_data( G_OBJECT( m_pWidget ), "menu_plugin" ) ), parent );

	int nCount = pPlugIn->getCommandCount();
	if ( nCount > 0 ) {
		menu = gtk_menu_new();
		while ( nCount > 0 )
		{
			menuText = pPlugIn->getCommand( --nCount );
			if ( menuText != NULL && strlen( menuText ) > 0 ) {
				if ( !strcmp( menuText, "-" ) ) {
					item = gtk_menu_item_new();
					gtk_widget_set_sensitive( item, FALSE );
				}
				else
				{
					item = gtk_menu_item_new_with_label( menuText );
					g_signal_connect( G_OBJECT( item ), "activate",
										G_CALLBACK( HandleCommand ), GINT_TO_POINTER( m_nNextPlugInID ) );
				}
				gtk_widget_show( item );
				gtk_container_add( GTK_CONTAINER( menu ), item );
				pPlugIn->addMenuID( m_nNextPlugInID++ );
			}
		}
		gtk_menu_item_set_submenu( GTK_MENU_ITEM( parent ), menu );
	}
}

void MainFrame::OnPlugIn( unsigned int nID, const char* str ){
	m_PlugInMgr.Dispatch( nID, str );
}

void toolbar_insert( GtkWidget *toolbar, const char* image, const char* text, const char* tooltip, IToolbarButton::EType type, GCallback callback, gpointer data ){
	GtkToolItem *item;
	
	switch ( type )
	{
	case IToolbarButton::eSpace:
		item = gtk_separator_tool_item_new();
		break;
	case IToolbarButton::eButton:
		item = gtk_tool_button_new( new_plugin_image_icon( image ), text );
		break;
	case IToolbarButton::eToggleButton:
		item = gtk_toggle_tool_button_new();
		gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( item ), new_plugin_image_icon( image ) );
		gtk_tool_button_set_label( GTK_TOOL_BUTTON( item ), text );
		break;
	case IToolbarButton::eRadioButton:
		item = gtk_radio_tool_button_new( NULL );
		gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( item ), new_plugin_image_icon( image ) );
		break;
	default:
		Error( "invalid toolbar button type" );
		break;
	}
	
	gtk_widget_set_tooltip_text( GTK_WIDGET( item ), tooltip );
	g_signal_connect( item, "clicked", callback, data );

	gtk_toolbar_insert( GTK_TOOLBAR( toolbar ), item, -1 );
	gtk_widget_show( GTK_WIDGET( item ) );

}

void SignalToolbarButton( GtkWidget *widget, gpointer data ){
	const_cast<const IToolbarButton*>( reinterpret_cast<IToolbarButton*>( data ) )->activate();
}

void MainFrame::AddPlugInToolbarButton( const IToolbarButton* button ){
	GtkWidget*const toolbar = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "toolbar_plugin" ) );
	toolbar_insert( toolbar, button->getImage(), button->getText(), button->getTooltip(), button->getType(), G_CALLBACK( SignalToolbarButton ), reinterpret_cast<gpointer>( const_cast<IToolbarButton*>( button ) ) );
}

void MainFrame::OnSelectionSelectNudgedown(){
	NudgeSelection( 3, g_qeglobals.d_gridsize );
}

void MainFrame::OnSelectionSelectNudgeleft(){
	NudgeSelection( 0, g_qeglobals.d_gridsize );
}

void MainFrame::OnSelectionSelectNudgeright(){
	NudgeSelection( 2, g_qeglobals.d_gridsize );
}

void MainFrame::OnSelectionSelectNudgeup(){
	NudgeSelection( 1, g_qeglobals.d_gridsize );
}

void MainFrame::NudgeSelection( int nDirection, float fAmount ){
	if ( ActiveXY()->RotateMode() ) {
		int nAxis = 0;
		if ( ActiveXY()->GetViewType() == XY ) {
			nAxis = 2;
		}
		else
		if ( g_pParentWnd->ActiveXY()->GetViewType() == XZ ) {
			nAxis = 1;
			fAmount = -fAmount;
		}

		if ( nDirection == 2 || nDirection == 3 ) {
			fAmount = -fAmount;
		}

		float fDeg = -fAmount;
		float fAdj = fAmount;

		g_pParentWnd->ActiveXY()->Rotation()[nAxis] += fAdj;
		CString strStatus;
		strStatus.Format( "Rotation x:: %.1f  y:: %.1f  z:: %.1f", g_pParentWnd->ActiveXY()->Rotation()[0],
						  g_pParentWnd->ActiveXY()->Rotation()[1], g_pParentWnd->ActiveXY()->Rotation()[2] );
		g_pParentWnd->SetStatusText( 2, strStatus );
		Select_RotateAxis( nAxis, fDeg, false, true );
		Sys_UpdateWindows( W_ALL );
	}
	else
	if ( ActiveXY()->ScaleMode() ) {
		if ( nDirection == 0 || nDirection == 3 ) {
			fAmount = -fAmount;
		}
		vec3_t v;
		v[0] = v[1] = v[2] = 1.0;
		if ( fAmount > 0 ) {
			v[0] = 1.1f;
			v[1] = 1.1f;
			v[2] = 1.1f;
		}
		else
		{
			v[0] = 0.9f;
			v[1] = 0.9f;
			v[2] = 0.9f;
		}

		Select_Scale( ( g_nScaleHow & SCALE_X ) ? v[0] : 1.0,
					  ( g_nScaleHow & SCALE_Y ) ? v[1] : 1.0,
					  ( g_nScaleHow & SCALE_Z ) ? v[2] : 1.0 );
		Sys_UpdateWindows( W_ALL );
	}
	else
	{
		// 0 - left, 1 - up, 2 - right, 3 - down
		int nDim;
		if ( nDirection == 0 ) {
			nDim = ActiveXY()->GetViewType() == YZ ? 1 : 0;
			fAmount = -fAmount;
		}
		else if ( nDirection == 1 ) {
			nDim = ActiveXY()->GetViewType() == XY ? 1 : 2;
		}
		else if ( nDirection == 2 ) {
			nDim = ActiveXY()->GetViewType() == YZ ? 1 : 0;
		}
		else
		{
			nDim = ActiveXY()->GetViewType() == XY ? 1 : 2;
			fAmount = -fAmount;
		}
		Nudge( nDim, fAmount );
	}
}

void MainFrame::Nudge( int nDim, float fNudge ){
	vec3_t vMove;
	vMove[0] = vMove[1] = vMove[2] = 0;
	vMove[nDim] = fNudge;

	if ( ( g_qeglobals.d_select_mode == sel_vertex ||
		   g_qeglobals.d_select_mode == sel_curvepoint )
		 && g_qeglobals.d_num_move_points ) {
		Select_NudgePoint( vMove, true );
	}
	else{
		Select_Move( vMove, true );
	}
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::SetGridStatus(){
	CString strStatus;
	char c1;
	char c2;
	c1 = ( g_PrefsDlg.m_bTextureLock ) ? 'M' : ' ';
	c2 = ( g_PrefsDlg.m_bRotateLock ) ? 'R' : ' ';
	strStatus.Format( "G:%g R:%i C:%i L:%c%c", g_qeglobals.d_gridsize,
					  g_PrefsDlg.m_nRotation, g_PrefsDlg.m_nCubicScale, c1, c2 );
	SetStatusText( 4, strStatus );
}

void MainFrame::UpdatePatchToolbarButtons(){
	GtkWidget *item;
	g_bIgnoreCommands++;
	item  = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_bend" ) );
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchBendMode ) ? TRUE : FALSE );
//  item  = GTK_WIDGET (g_object_get_data (G_OBJECT (m_pWidget), "ttb_patch_insdel"));
//  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (item), (g_bPatchInsertMode) ? TRUE : FALSE);
	g_bIgnoreCommands--;
}

// =============================================================================
// Command handlers

void MainFrame::OnFileNew(){
	if ( ConfirmModified() ) {
		Map_New();
	}
}

void MainFrame::OnFileOpen(){
	if ( !ConfirmModified() ) {
		return;
	}

	const char *str;
	char buf[NAME_MAX];

	if ( !g_pGameDescription->noMapsInHome ) {
		strcpy( buf, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buf, "maps/" );
	}
	else {
		buf[0] = '\0';
	}

	str = file_dialog( m_pWidget, TRUE, _( "Open Map" ), buf, MAP_MAJOR, "maps/" );

	if ( str != NULL ) {
		strcpy( currentmap,str );
		MRU_AddFile( str );
		Map_LoadFile( str );
	}
}

void MainFrame::OnFileImportmap(){
	const char *str;
	char buf[NAME_MAX];

	if ( !g_pGameDescription->noMapsInHome ) {
		strcpy( buf, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buf, "maps/" );
	}
	else {
		buf[0] = '\0';
	}

	str = file_dialog( m_pWidget, TRUE, _( "Import Map" ), buf, MAP_MAJOR, "maps/" );

	if ( str != NULL ) {
		Map_ImportFile( str );
	}
}

void MainFrame::OnFileSave(){
	if ( !strcmp( currentmap, "unnamed.map" ) ) {
		OnFileSaveas();
	}
	else{
		Map_SaveFile( currentmap, false );
	}
}

void MainFrame::OnFileSaveas(){
	const char* str;
	char buf[NAME_MAX];

	if ( !g_pGameDescription->noMapsInHome ) {
		strcpy( buf, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buf, "maps/" );
	}
	else {
		buf[0] = '\0';
	}

	str = file_dialog( g_pParentWnd->m_pWidget, FALSE, _( "Save Map" ), buf, MAP_MAJOR, "maps/" );

	if ( str != NULL ) {
		strcpy( currentmap, str );
		MRU_AddFile( str );
		Map_SaveFile( str, false ); // ignore region
	}
}

void MainFrame::OnFileExportmap(){
	const char* str;
	char buf[NAME_MAX];

	if ( !g_pGameDescription->noMapsInHome ) {
		strcpy( buf, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buf, "maps/" );
	}
	else {
		buf[0] = '\0';
	}

	str = file_dialog( m_pWidget, FALSE, _( "Export Selection" ), buf, MAP_MAJOR, "maps/" );

	if ( str != NULL ) {
		Map_SaveSelected( str );
	}
}

void MainFrame::OnFileSaveregion(){
	const char* str;
	char buf[NAME_MAX];

	if ( !g_pGameDescription->noMapsInHome ) {
		strcpy( buf, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buf, "maps/" );
	}
	else {
		buf[0] = '\0';
	}

	str = file_dialog( g_pParentWnd->m_pWidget, FALSE, _( "Export Region" ), buf, MAP_MAJOR, "maps/" );

	if ( str != NULL ) {
		Map_SaveFile( str, true ); // ignore region
	}
}

void MainFrame::OnFileNewproject(){
	char* name = DoNewProjectDlg();

	// create a new project:
	// create directories and grab current project, save it in new project tree in scripts/user.qe4
	// on linux we create under ~/.q3a, on win32 under strEnginePath
	// NOTE: working on a seperate project file might be broken, never did much experiment with that..
	if ( ( name != NULL ) && ( strlen( name ) > 0 ) ) {
		CString strNewBasePath;

		// NOTE TTimo this would probably not work right on *nix
		strNewBasePath = g_pGameDescription->mEnginePath.GetBuffer(); // assume paths end with '/'
		strNewBasePath += name;
		strNewBasePath += "/";

		CString strProjToLoad;
		CString strMapToLoad;

		// if the dir exists, ask the user if they want to continue anyway
		if ( Q_mkdir( strNewBasePath.GetBuffer(), 0755 ) != 0 ) {
			CString strMsg;
			strMsg.Format( "The directory name %s already exists\nContinue anyway ?\n", strNewBasePath.GetBuffer() );
			Sys_Printf( strMsg );
			if ( gtk_MessageBox( m_pWidget, _( strMsg ), _( "Error" ), MB_YESNO ) != IDYES ) {
				Sys_Printf( "New Project cancelled, directory already exists for project\n" );
				free( name );
				return;
			}
		}

		CString strDir;
		strDir = strNewBasePath;
		strDir += "maps/";
		Q_mkdir( strDir.GetBuffer(), 0755 );

		strDir = strNewBasePath;
		strDir += "textures/";
		Q_mkdir( strDir.GetBuffer(), 0755 );

		strDir = strNewBasePath;
		strDir += "scripts/";
		Q_mkdir( strDir.GetBuffer(), 0755 );

		// print a warning for total conversions, since setting the basepath when required files are
		// not there _will_ break things (ie; textures/radiant/notex.tga, scripts/entities.def)
		Sys_FPrintf( SYS_WRN, "*** Note: basepath unchanged\n" );

		SetKeyValue( g_qeglobals.d_project_entity, "gamename", name );

		strDir = strNewBasePath;
		strDir += "maps/autosave.map";
		SetKeyValue( g_qeglobals.d_project_entity, "autosave", strDir.GetBuffer() );

		// state that this is a user project file, no templating
		SetKeyValue( g_qeglobals.d_project_entity, "user_project", "1" );
		// create the project file
		strProjToLoad = strNewBasePath;
		strProjToLoad += "scripts/";
		strProjToLoad += name;
		strProjToLoad += ".";
		strProjToLoad += PROJECT_FILETYPE;
		QE_SaveProject( strProjToLoad.GetBuffer() );
		free( name );
	}
}

void MainFrame::OnFileLoadproject(){
	if ( ConfirmModified() ) {
		ProjectDialog();
	}
}

void MainFrame::OnFileProjectsettings(){
	DoProjectSettings();
}

void MainFrame::OnFilePointfile(){
	if ( g_qeglobals.d_pointfile_display_list ) {
		Pointfile_Clear();
	}
	else{
		Pointfile_Check();
	}
}

void MainFrame::OnMru( unsigned int nID ){
	if ( ConfirmModified() ) {
		MRU_Activate( nID - ID_FILE_RECENT1 );
	}
}

void MainFrame::OnFileExit(){
	if ( ConfirmModified() ) {
		// stop printing during shutdown
		// NOTE: we should cleanly release GL contexts and stuff when exiting

		OnDelete();

		g_qeglobals_gui.d_edit = NULL;
		gtk_widget_destroy( m_pWidget );
	}
}

void MainFrame::OnFileCheckUpdate(){
	// build the URL
	Str URL;
	URL = "http://www.qeradiant.com/index.php?data=dlupdate&query_dlup=1";
#ifdef _WIN32
	URL += "&OS_dlup=1";
#else
	URL += "&OS_dlup=2";
#endif
	URL += "&Version_dlup=" RADIANT_VERSION;
	g_PrefsDlg.mGamesDialog.AddPacksURL( URL );
	OpenURL( m_pWidget, URL.GetBuffer() );
}

void MainFrame::OnEditUndo(){
	Undo_Undo();
}

void MainFrame::OnEditRedo(){
	Undo_Redo();
}

void MainFrame::OnEditCopybrush(){
	Copy();
}

void MainFrame::OnEditPastebrush(){
	Select_Deselect();

	Undo_Start( "paste" );

	Paste();

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnEditPastebrushToCamera(){
	Select_Deselect();
	if ( ActiveXY() ) {
		vec3_t mid, camorigin, delta;

		ActiveXY()->Paste();

		// Work out the delta
		Select_GetMid( mid );

		// Snap camera origin to grid
		VectorCopy( m_pCamWnd->Camera()->origin, camorigin );
		camorigin[0] = floor( camorigin[0] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		camorigin[1] = floor( camorigin[1] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		camorigin[2] = floor( camorigin[2] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;

		VectorSubtract( camorigin, mid, delta );

		// Move to camera
		Select_Move( delta, false );

		Undo_Start( "paste to camera" );
		Undo_EndBrushList( &selected_brushes );
		Undo_End();
	}
}

void MainFrame::OnSelectionDelete(){
	brush_t *brush;
	//if (ActiveXY())
	//	ActiveXY()->UndoCopy();
	Undo_Start( "delete" );
	Undo_AddBrushList( &selected_brushes );
	//add all deleted entities to the undo
	for ( brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next )
	{
		Undo_AddEntity( brush->owner );
	}
	// NOTE: Select_Delete does NOT delete entities
	Select_Delete();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnEditMapinfo(){
	DoMapInfo();
}

void MainFrame::OnEditEntityinfo(){
	DoEntityList();
}

void MainFrame::OnBrushScripts(){
	DoScriptsDlg();
}

void MainFrame::OnEditLoadprefab(){
	const char *filename;
	CString CurPath;

	if ( g_PrefsDlg.m_strPrefabPath.GetLength() > 0 ) {
		CurPath = g_PrefsDlg.m_strPrefabPath;
		AddSlash( CurPath );
	}

	filename = file_dialog( m_pWidget, TRUE, _( "Import Prefab" ), CurPath.GetBuffer(), MAP_MAJOR, "prefabs/" );

	if ( filename != NULL ) {
		Map_ImportFile( filename );
	}
}

void MainFrame::OnEditSaveprefab(){
	const char *filename;
	CString CurPath;

	if ( g_PrefsDlg.m_strPrefabPath.GetLength() > 0 ) {
		CurPath = g_PrefsDlg.m_strPrefabPath;
	}
	else
	{
		char tmp[PATH_MAX];
		getcwd( tmp, PATH_MAX );
		CurPath = tmp;
	}
	AddSlash( CurPath );

	filename = file_dialog( m_pWidget, FALSE, _( "Export Prefab" ), CurPath.GetBuffer(), MAP_MAJOR, "prefabs/" );
	if ( filename != NULL ) {
		Map_SaveSelected( filename );
	}
}

void MainFrame::OnPrefs() {
    int     nView               = g_PrefsDlg.m_nView;
    int     nShader             = g_PrefsDlg.m_nShader;
    int     nTextureQuality     = g_PrefsDlg.m_nTextureQuality;
    bool    bToolbar            = g_PrefsDlg.m_bWideToolbar;
    bool    bPluginToolbar      = g_PrefsDlg.m_bPluginToolbar;
    bool    bDetachableMenus    = g_PrefsDlg.m_bDetachableMenus;
    bool    bFloatingZ          = g_PrefsDlg.m_bFloatingZ;
	bool    bShowTexDirList     = g_PrefsDlg.m_bShowTexDirList;

    g_PrefsDlg.LoadPrefs();

    if(g_PrefsDlg.DoModal() == IDOK) {
        if((g_PrefsDlg.m_nLatchedView               != nView            ) ||
           (g_PrefsDlg.m_bLatchedDetachableMenus    != bDetachableMenus ) ||
           (g_PrefsDlg.m_bLatchedWideToolbar        != bToolbar         ) ||
           (g_PrefsDlg.m_bLatchedPatchToolbar       != bToolbar         ) ||
           (g_PrefsDlg.m_bLatchedPluginToolbar      != bPluginToolbar   ) ||
           (g_PrefsDlg.m_nLatchedShader             != nShader          ) ||
           (g_PrefsDlg.m_nLatchedTextureQuality     != nTextureQuality  ) || 
           (g_PrefsDlg.m_bLatchedFloatingZ          != bFloatingZ       ) ||
		   (g_PrefsDlg.m_bShowTexDirList            != bShowTexDirList)) {
            gtk_MessageBoxNew(m_pWidget, _( "You must restart Radiant for the "
                              "changes to take effect." ), _( "Restart Radiant" ), 
                              MB_OK | MB_ICONINFORMATION);
        }

        // if the view mode was switched to floating, set the Z window on by 
        // default. this was originally intended as a bug fix, but the fix is 
        // elsewhere .. anyway making sure we force Z on each time is good
        // (and we simply hope there will be a SavePrefs before we die)
        if((g_PrefsDlg.m_nView != nView) && 
           ((EViewStyle)g_PrefsDlg.m_nView == (EViewStyle)eFloating)) {
            g_PrefsDlg.m_bZVis = true;
        }

        if(m_pTexWnd) {
            m_pTexWnd->UpdatePrefs();
        }

        GtkWidget *item = GTK_WIDGET(g_object_get_data(G_OBJECT(m_pWidget), 
                                                       "menu_snaptogrid"));
        g_bIgnoreCommands++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
                                      (g_PrefsDlg.m_bSnap) ? TRUE : FALSE);
        g_bIgnoreCommands--;
    }
}

void MainFrame::OnTogglecamera(){
	if ( CurrentStyle() == eFloating ) { // floating views
		if ( m_pCamWnd && m_pCamWnd->m_pParent ) {
			if ( gtk_widget_get_visible( m_pCamWnd->m_pParent ) ) {
				widget_delete_hide( m_pCamWnd->m_pParent );
			}
			else{
				gtk_widget_show( m_pCamWnd->m_pParent );
			}
		}
	}
	else
	{
		if ( gtk_widget_get_visible( m_pCamWnd->GetWidget() ) ) {
			gtk_widget_hide( m_pCamWnd->GetWidget() );
		}
		else{
			gtk_widget_show( m_pCamWnd->GetWidget() );
		}
	}
}

void MainFrame::OnToggleconsole(){
	if ( FloatingGroupDialog() ) { // QE4 style
		if ( inspector_mode == W_CONSOLE ) {
			if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
				widget_delete_hide( g_qeglobals_gui.d_entity );
			}
			else{
				gtk_widget_show( g_qeglobals_gui.d_entity );
			}
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_CONSOLE );
		}
	}
}

// trigger the entity inspector on/off
void MainFrame::OnViewEntity(){
	// make sure we're working with the current selection (bugzilla #436)
	if ( !gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
		Select_Reselect();
	}

	if ( !FloatingGroupDialog() ) {
		if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) && inspector_mode == W_ENTITY ) {
			widget_delete_hide( g_qeglobals_gui.d_entity );
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_ENTITY );
		}
	}
	else
	{
		if ( inspector_mode == W_ENTITY ) {
			if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
				widget_delete_hide( g_qeglobals_gui.d_entity );
			}
			else{
				gtk_widget_show( g_qeglobals_gui.d_entity );
			}
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_ENTITY );
		}
	}
}

void MainFrame::OnViewGroups(){
	if ( !FloatingGroupDialog() ) {
		if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) && inspector_mode == W_GROUP ) {
			widget_delete_hide( g_qeglobals_gui.d_entity );
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_GROUP );
		}
	}
	else
	{
		if ( inspector_mode == W_GROUP && CurrentStyle() != MainFrame::eFloating ) {
			if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
				widget_delete_hide( g_qeglobals_gui.d_entity );
			}
			else{
				gtk_widget_show( g_qeglobals_gui.d_entity );
			}
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_GROUP );
		}
	}
}

void MainFrame::OnToggleview(){
	if ( CurrentStyle() == eFloating ) { // QE4 style
		if ( m_pXYWnd && m_pXYWnd->m_pParent ) {
			if ( gtk_widget_get_visible( m_pXYWnd->m_pParent ) ) {
				widget_delete_hide( m_pXYWnd->m_pParent );
			}
			else{
				gtk_widget_show( m_pXYWnd->m_pParent );
			}
		}
	}
}

void MainFrame::OnToggleviewXz(){
	if ( CurrentStyle() == eFloating ) { // QE4 style
		if ( m_pXZWnd && m_pXZWnd->m_pParent ) {
			// get windowplacement doesn't actually save this so we will here
			g_PrefsDlg.m_bXZVis = gtk_widget_get_visible( m_pXZWnd->m_pParent );
			if ( g_PrefsDlg.m_bXZVis ) {
				widget_delete_hide( m_pXZWnd->m_pParent );
			}
			else{
				gtk_widget_show( m_pXZWnd->m_pParent );
			}
			g_PrefsDlg.m_bXZVis ^= 1;
			g_PrefsDlg.SavePrefs();
		}
	}
}

void MainFrame::OnToggleviewYz(){
	if ( CurrentStyle() == eFloating ) { // QE4 style
		if ( m_pYZWnd && m_pYZWnd->m_pParent ) {
			g_PrefsDlg.m_bYZVis = gtk_widget_get_visible( m_pYZWnd->m_pParent );
			if ( g_PrefsDlg.m_bYZVis ) {
				widget_delete_hide( m_pYZWnd->m_pParent );
			}
			else{
				gtk_widget_show( m_pYZWnd->m_pParent );
			}
			g_PrefsDlg.m_bYZVis ^= 1;
			g_PrefsDlg.SavePrefs();
		}
	}
}

void MainFrame::OnTogglez(){
	if ( g_pParentWnd->FloatingGroupDialog() ) { // QE4 style
		if ( m_pZWnd && m_pZWnd->m_pParent ) {
			if ( gtk_widget_get_visible( m_pZWnd->m_pParent ) ) {
				widget_delete_hide( m_pZWnd->m_pParent );
			}
			else{
				gtk_widget_show( m_pZWnd->m_pParent );
			}
			g_PrefsDlg.m_bZVis ^= 1;
			g_PrefsDlg.SavePrefs();
		}
	}
	else {
		Sys_FPrintf( SYS_WRN, "Z view toggle is only valid in floating views\n" );
	}
}

void MainFrame::OnViewCenter(){
	m_pCamWnd->Camera()->angles[ROLL] = m_pCamWnd->Camera()->angles[PITCH] = 0;
	m_pCamWnd->Camera()->angles[YAW] = 22.5 * floor( ( m_pCamWnd->Camera()->angles[YAW] + 11 ) / 22.5 );
	Sys_UpdateWindows( W_CAMERA | W_XY_OVERLAY );
}

void MainFrame::OnViewUpfloor(){
	m_pCamWnd->Cam_ChangeFloor( true );
}

void MainFrame::OnViewDownfloor(){
	m_pCamWnd->Cam_ChangeFloor( false );
}

void MainFrame::OnViewCenterview(){
	if ( CurrentStyle() == eSplit ) {
		GetXYWnd()->PositionView();
		GetXZWnd()->PositionView();
		GetYZWnd()->PositionView();
		Sys_UpdateWindows( W_XY | W_XZ | W_YZ );
	}
	else {
		m_pXYWnd->PositionView();
		Sys_UpdateWindows( W_XY );
	}
}

void MainFrame::OnViewNextview(){
	if ( CurrentStyle() == eSplit ) {
		GetXYWnd()->PositionView();
		GetXZWnd()->PositionView();
		GetYZWnd()->PositionView();
		Sys_UpdateWindows( W_XY | W_XZ | W_YZ );
	}
	else {
		if ( m_pXYWnd->GetViewType() == XY ) {
			m_pXYWnd->SetViewType( XZ );
		}
		else
		if ( m_pXYWnd->GetViewType() ==  XZ ) {
			m_pXYWnd->SetViewType( YZ );
		}
		else{
			m_pXYWnd->SetViewType( XY );
		}
		m_pXYWnd->PositionView();
		Sys_UpdateWindows( W_XY );
	}
}

void MainFrame::OnViewXy(){
	if ( !FloatingGroupDialog() ) {
		m_pXYWnd->SetViewType( XY );
		m_pXYWnd->PositionView();
	}
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewSide(){
	if ( !FloatingGroupDialog() ) {
		m_pXYWnd->SetViewType( XZ );
		m_pXYWnd->PositionView();
	}
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewFront(){
	if ( !FloatingGroupDialog() ) {
		m_pXYWnd->SetViewType( YZ );
		m_pXYWnd->PositionView();
	}
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnView100(){
	if ( m_pXYWnd ) {
		m_pXYWnd->SetScale( 1 );
	}
	if ( m_pXZWnd ) {
		m_pXZWnd->SetScale( 1 );
	}
	if ( m_pYZWnd ) {
		m_pYZWnd->SetScale( 1 );
	}
	Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
}

void MainFrame::OnViewZoomin(){
	if ( m_pXYWnd && m_pXYWnd->Active() ) {
		m_pXYWnd->SetScale( m_pXYWnd->Scale() * 5.0 / 4 );
		if ( m_pXYWnd->Scale() > 30 ) {
			m_pXYWnd->SetScale( 30 );
		}
	}

	if ( m_pXZWnd && m_pXZWnd->Active() ) {
		m_pXZWnd->SetScale( m_pXZWnd->Scale() * 5.0 / 4 );
		if ( m_pXZWnd->Scale() > 30 ) {
			m_pXZWnd->SetScale( 30 );
		}
	}

	if ( m_pYZWnd && m_pYZWnd->Active() ) {
		m_pYZWnd->SetScale( m_pYZWnd->Scale() * 5.0 / 4 );
		if ( m_pYZWnd->Scale() > 30 ) {
			m_pYZWnd->SetScale( 30 );
		}
	}

	Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
}

// NOTE: the zoom out factor is 4/5, we could think about customizing it
//  we don't go below a zoom factor corresponding to 10% of the max world size
//  (this has to be computed against the window size)
void MainFrame::OnViewZoomout(){
	float min_scale;
	if ( m_pXYWnd && m_pXYWnd->Active() ) {
		m_pXYWnd->SetScale( m_pXYWnd->Scale() * 4.0 / 5 );
		min_scale = MIN( m_pXYWnd->Width(),m_pXYWnd->Height() ) / ( 1.1 * ( g_MaxWorldCoord - g_MinWorldCoord ) );
		if ( m_pXYWnd->Scale() < min_scale ) {
			m_pXYWnd->SetScale( min_scale );
		}
	}

	if ( m_pXZWnd && m_pXZWnd->Active() ) {
		m_pXZWnd->SetScale( m_pXZWnd->Scale() * 4.0 / 5 );
		min_scale = MIN( m_pXZWnd->Width(),m_pXZWnd->Height() ) / ( 1.1 * ( g_MaxWorldCoord - g_MinWorldCoord ) );
		if ( m_pXZWnd->Scale() < min_scale ) {
			m_pXZWnd->SetScale( min_scale );
		}
	}

	if ( m_pYZWnd && m_pYZWnd->Active() ) {
		m_pYZWnd->SetScale( m_pYZWnd->Scale() * 4.0 / 5 );
		min_scale = MIN( m_pYZWnd->Width(),m_pYZWnd->Height() ) / ( 1.1 * ( g_MaxWorldCoord - g_MinWorldCoord ) );
		if ( m_pYZWnd->Scale() < min_scale ) {
			m_pYZWnd->SetScale( min_scale );
		}
	}
	Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
}

void MainFrame::OnViewZ100(){
	z.scale = 1;
	Sys_UpdateWindows( W_Z | W_Z_OVERLAY );
}

void MainFrame::OnViewZzoomin(){
	z.scale *= 5.0 / 4;
	if ( z.scale > 4 ) {
		z.scale = 4;
	}
	Sys_UpdateWindows( W_Z | W_Z_OVERLAY );
}

void MainFrame::OnViewZzoomout(){
	z.scale *= 4.0f / 5;
	if ( z.scale < 0.125 ) {
		z.scale = 0.125;
	}
	Sys_UpdateWindows( W_Z | W_Z_OVERLAY );
}

void MainFrame::OnViewCubein(){
	g_PrefsDlg.m_nCubicScale--;
	if ( g_PrefsDlg.m_nCubicScale < 1 ) {
		g_PrefsDlg.m_nCubicScale = 1;
	}
	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_CAMERA );
	SetGridStatus();
}

void MainFrame::OnViewCubeout(){
	g_PrefsDlg.m_nCubicScale++;
	if ( g_PrefsDlg.m_nCubicScale > 22 ) {
		g_PrefsDlg.m_nCubicScale = 22;
	}
	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_CAMERA );
	SetGridStatus();
}

void MainFrame::OnViewShownames(){
	g_qeglobals.d_savedinfo.show_names = !g_qeglobals.d_savedinfo.show_names;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_shownames" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									g_qeglobals.d_savedinfo.show_names ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewShowAngles(){
	g_qeglobals.d_savedinfo.show_angles = !g_qeglobals.d_savedinfo.show_angles;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showangles" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									g_qeglobals.d_savedinfo.show_angles ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewShowblocks(){
	g_qeglobals.show_blocks ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showblocks" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_qeglobals.show_blocks ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewShowcoordinates(){
	g_qeglobals.d_savedinfo.show_coordinates ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showcoordinates" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									g_qeglobals.d_savedinfo.show_coordinates ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY | W_Z );
}

void MainFrame::OnViewShowOutline(){
	g_qeglobals.d_savedinfo.show_outline ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showoutline" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									g_qeglobals.d_savedinfo.show_outline ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewShowAxes(){
	g_qeglobals.d_savedinfo.show_axis ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showaxes" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ),
									g_qeglobals.d_savedinfo.show_axis ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewShowWorkzone(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_showworkzone" ) );
	g_bIgnoreCommands++;
	if ( g_qeglobals.d_show_work ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
		g_qeglobals.d_show_work = false;
	}
	else
	{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
		g_qeglobals.d_show_work = true;
	}
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnViewHideshowHideselected(){
	Select_Hide();
	Select_Deselect();
}

void MainFrame::OnViewHideshowShowhidden(){
	Select_ShowAllHidden();
}

/**
   sets the view mode for the entities
   called upon LoadPrefs too
   NOTE TTimo previous implementation had a SavePrefs call
   .. I don't think it is relevant, removed (the prefs are saved upon exit)
   NOTE TTimo we activate the menu item, this is only needed when we are called upon a prefs load
   (otherwise we are always called following user action on the widget)
 */
void MainFrame::OnEntitiesSetViewAs( int mode ){
	gpointer item = NULL;
	if ( mode == 0 ) {
		switch ( g_PrefsDlg.m_nEntityShowState )
		{
		case ENTITY_BOX:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_boundingbox" );
			break;
		case ENTITY_WIRE:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_wireframe" );
			break;
		case ENTITY_SELECTED:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_selectedwireframe" );
			break;
		case ENTITY_SELECTED_SKIN:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_selectedskinned" );
			break;
		case ENTITY_SKINNED:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_skinned" );
			break;
		case ENTITY_SKINNED_BOXED:
			item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_skinnedandboxed" );
			break;
		}
		g_bIgnoreCommands++;
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
		g_bIgnoreCommands--;
		return;
	}

	switch ( mode )
	{
	case ID_VIEW_ENTITIESAS_BOUNDINGBOX:
		g_PrefsDlg.m_nEntityShowState = ENTITY_BOX;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_boundingbox" );
		break;
	case ID_VIEW_ENTITIESAS_WIREFRAME:
		g_PrefsDlg.m_nEntityShowState = ENTITY_WIRE;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_wireframe" );
		break;
	case ID_VIEW_ENTITIESAS_SELECTEDWIREFRAME:
		g_PrefsDlg.m_nEntityShowState = ENTITY_SELECTED;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_selectedwireframe" );
		break;
	case ID_VIEW_ENTITIESAS_SELECTEDSKINNED:
		g_PrefsDlg.m_nEntityShowState = ENTITY_SELECTED_SKIN;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_selectedskinned" );
		break;
	case ID_VIEW_ENTITIESAS_SKINNED:
		g_PrefsDlg.m_nEntityShowState = ENTITY_SKINNED;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_skinned" );
		break;
	case ID_VIEW_ENTITIESAS_SKINNEDANDBOXED:
		g_PrefsDlg.m_nEntityShowState = ENTITY_SKINNED_BOXED;
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_entitiesas_skinnedandboxed" );
		break;
	default:
		Sys_FPrintf( SYS_ERR, "Entity mode ID_ not found in MainFrame::Entities_SetViewAs\n" );
		return;
	}

	if ( !item ) {
		Sys_FPrintf( SYS_ERR, "menu not found in MainFrame::Entities_SetViewAs\n" );
		return;
	}

	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	g_bIgnoreCommands--;

	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnViewCubicclipping(){
	GtkWidget *w;

	g_PrefsDlg.m_bCubicClipping ^= 1;
	g_bIgnoreCommands++;
	w = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_cubicclipping" ) );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( w ), g_PrefsDlg.m_bCubicClipping ? TRUE : FALSE );
	w = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_view_cubicclipping" ) );
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( w ), g_PrefsDlg.m_bCubicClipping ? TRUE : FALSE );
	g_bIgnoreCommands--;
	g_PrefsDlg.SavePrefs();
	//Map_BuildBrushData ();
	Sys_UpdateWindows( W_CAMERA );
}

void MainFrame::OnViewOpengllighting(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_view_opengllighting" ) );
	g_PrefsDlg.m_bGLLighting ^= 1;
	g_PrefsDlg.SavePrefs();
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bGLLighting ? TRUE : FALSE );
	Sys_UpdateWindows( W_XY | W_CAMERA );
	g_bIgnoreCommands--;
}

void MainFrame::OnSelectionDragedges(){
	if ( g_qeglobals.d_select_mode == sel_edge ) {
		g_qeglobals.d_select_mode = sel_brush;
		Sys_UpdateWindows( W_ALL );
	}
	else
	{
		SetupVertexSelection();
		if ( g_qeglobals.d_numpoints ) {
			g_qeglobals.d_select_mode = sel_edge;
		}
		Sys_UpdateWindows( W_ALL );
	}
}

void MainFrame::OnSelectionDragvertecies(){
	if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_curvepoint ) {
		g_qeglobals.d_select_mode = sel_brush;
		Sys_UpdateWindows( W_ALL );
	}
	else
	{
		//--if (QE_SingleBrush() && selected_brushes.next->patchBrush)
		if ( OnlyPatchesSelected() ) {
			Patch_EditPatch();
		}
		else //if (!AnyPatchesSelected()) // allows vertex mode when patches are selected
		{
			SetupVertexSelection();
			if ( g_qeglobals.d_numpoints ) {
				g_qeglobals.d_select_mode = sel_vertex;
			}
		}
		Sys_UpdateWindows( W_ALL );
	}
}

void MainFrame::OnSelectionClone(){
	Select_Clone();
}

// called when the escape key is used (either on the main window or on an inspector)
void MainFrame::OnSelectionDeselect(){
	if ( g_bClipMode ) {
		OnViewClipper();
	}
	else
	if ( g_bRotateMode ) {
		OnSelectMouserotate();
	}
	else
	if ( g_bScaleMode ) {
		OnSelectMousescale();
	}
	else
	if ( g_bPathMode ) {
		if ( ActiveXY() ) {
			ActiveXY()->KillPathMode();
		}
	}
	else
	{
		if ( g_qeglobals.d_select_mode == sel_curvepoint && g_qeglobals.d_num_move_points > 0 ) {
			g_qeglobals.d_num_move_points = 0;
			Sys_UpdateWindows( W_ALL );
		}
		else
		{
			Select_Deselect();
			SetStatusText( 2, " " );
		}
	}
}

void MainFrame::OnBrushFlipx(){
	Undo_Start( "flip X" );
	Undo_AddBrushList( &selected_brushes );

	Select_FlipAxis( 0 );
	// spog - this does not work - it's a rotate not a flip
	/*
	   for (brush_t *b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
	   {
	   if(b->owner->eclass->fixedsize)
	   {
	    char buf[16];
	    float a = FloatForKey(b->owner, "angle");
	    a = div ((int)(180 - a), 180).rem;
	    sprintf (buf, "%d", (int)a);
	    SetKeyValue(b->owner, "angle", buf);
	    Brush_Build(b,true,true,false,false); // don't filter
	   }
	   }
	 */
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushFlipy(){
	Undo_Start( "flip Y" );
	Undo_AddBrushList( &selected_brushes );

	Select_FlipAxis( 1 );
	// spog - this does not work - it's a rotate not a flip
	/*
	   for (brush_t *b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
	   {
	   if(b->owner->eclass->fixedsize)
	   {
	    float a = FloatForKey(b->owner, "angle");
	    if (a == 0 || a == 180 || a == 360)
	   continue;
	    if ( a == 90 || a == 270)
	    {
	   a += 180;
	    }
	    else if (a > 270)
	   a += 90;
	    else if (a > 180)
	   a -= 90;
	    else if (a > 90)
	   a += 90;
	    else
	   a -= 90;
	    a = (int)a % 360;
	    char buf[16];
	    sprintf (buf, "%d", (int)a);
	    SetKeyValue(b->owner, "angle", buf);
	    Brush_Build(b,true,true,false,false); // don't filter
	   }

	   }
	 */
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushFlipz(){
	Undo_Start( "flip Z" );
	Undo_AddBrushList( &selected_brushes );
	Select_FlipAxis( 2 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushRotatex(){
	Undo_Start( "rotate X" );
	Undo_AddBrushList( &selected_brushes );
	Select_RotateAxis( 0, 90 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushRotatey(){
	Undo_Start( "rotate Y" );
	Undo_AddBrushList( &selected_brushes );
	Select_RotateAxis( 1, 90 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushRotatez(){
	Undo_Start( "rotate Z" );
	Undo_AddBrushList( &selected_brushes );
	Select_RotateAxis( 2, 90 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionArbitraryrotation(){
	Undo_Start( "arbitrary rotation" );
	Undo_AddBrushList( &selected_brushes );

	DoRotateDlg();

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectScale(){
	Undo_Start( "scale" );
	Undo_AddBrushList( &selected_brushes );
	DoScaleDlg();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMakehollow(){
	//if (ActiveXY())
	//	ActiveXY()->UndoCopy();
	Undo_Start( "hollow" );
	Undo_AddBrushList( &selected_brushes );
	CSG_MakeHollow();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMakehollowTouch(){
	//if (ActiveXY())
	//	ActiveXY()->UndoCopy();
	Undo_Start( "hollow" );
	Undo_AddBrushList( &selected_brushes );
	CSG_MakeHollowMode( CSG_HOLLOW_MODE_TOUCH );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionCsgsubtract(){
	Undo_Start( "CSG subtract" );
	CSG_Subtract();
	Undo_End();
}

void MainFrame::OnSelectionCsgmerge(){
	Undo_Start( "CSG merge" );
	Undo_AddBrushList( &selected_brushes );
	CSG_Merge();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionNoOutline(){
	//g_qeglobals.d_savedinfo.bNoSelectedOutlines ^= 1;
	g_qeglobals.d_savedinfo.iSelectedOutlinesStyle = ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) ^ OUTLINE_ZBUF;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_selection_nooutline" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_CAMERA );
}

void MainFrame::OnSelectionOutlineStyle(){
	if ( ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) && ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_BSEL ) ) {
		g_qeglobals.d_savedinfo.iSelectedOutlinesStyle &= ~OUTLINE_ZBUF;
	}
	else if ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_BSEL ) {
		g_qeglobals.d_savedinfo.iSelectedOutlinesStyle &= ~OUTLINE_BSEL;
	}
	else if ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) {
		g_qeglobals.d_savedinfo.iSelectedOutlinesStyle |= OUTLINE_BSEL;
	}
	else{
		g_qeglobals.d_savedinfo.iSelectedOutlinesStyle |= OUTLINE_ZBUF;
	}
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_selection_nooutline" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_CAMERA );
}

void MainFrame::OnSelectionSelectcompletetall(){
	if ( ActiveXY() ) {
		ActiveXY()->UndoCopy();
	}
	Select_CompleteTall();
}

void MainFrame::OnSelectionSelecttouching(){
	Select_Touching();
}

void MainFrame::OnSelectionSelectpartialtall(){
	Select_PartialTall();
}

void MainFrame::OnSelectionSelectinside(){
	Select_Inside();
}

void MainFrame::OnViewClipper(){
	GtkWidget *w = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_view_clipper" ) );
	g_bIgnoreCommands++;

	if ( ActiveXY() ) {
		if ( ActiveXY()->ClipMode() ) {
			ActiveXY()->SetClipMode( false );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( w ), FALSE );
		}
		else
		{
			if ( ActiveXY()->RotateMode() ) {
				OnSelectMouserotate();
			}
			ActiveXY()->SetClipMode( true );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( w ), TRUE );
		}
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnClipSelected(){
	if ( m_pActiveXY && m_pActiveXY->ClipMode() ) {
		Undo_Start( "clip selected" );
		Undo_AddBrushList( &selected_brushes );
		m_pActiveXY->Clip();
		Undo_EndBrushList( &selected_brushes );
		Undo_End();
	}
	else
	{
		if ( g_bPatchBendMode ) {
			Patch_BendHandleENTER();
		}
//    else if (g_bPatchBendMode)
//      Patch_InsDelHandleENTER();
	}
}

void MainFrame::OnToggleDetail(){
	GtkWidget *w = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_toggle_detail" ) );
	g_bIgnoreCommands++;
	
	if ( g_qeglobals.m_bMakeDetail == TRUE ) {
		g_qeglobals.m_bMakeDetail = FALSE;
		Sys_Printf( "Structural Brush mode activated\n" );

		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( w ), FALSE );
		gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( w ), new_image_icon( "toggle_struct.png" ) );
		
	}
	else
	{
		g_qeglobals.m_bMakeDetail = TRUE;
		Sys_Printf( "Detail Brush mode activated\n" );

		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( w ), TRUE );
		gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( w ), new_image_icon( "toggle_detail.png" ) );

	}

	g_bIgnoreCommands--;
}

void MainFrame::OnSplitSelected(){
	if ( m_pActiveXY ) {
		Undo_Start( "split selected" );
		Undo_AddBrushList( &selected_brushes );
		m_pActiveXY->SplitClip();
		Undo_EndBrushList( &selected_brushes );
		Undo_End();
	}
}

void MainFrame::OnFlipClip(){
	if ( m_pActiveXY ) {
		m_pActiveXY->FlipClip();
	}
}

void MainFrame::OnSelectionConnect(){
	Undo_Start( "connect selected entities" );
	Undo_AddBrushList( &selected_brushes );
	ConnectEntities();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionUngroupentity(){
	Undo_Start( "ungroup selected entities" );
	Undo_AddBrushList( &selected_brushes );
	Select_Ungroup();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMergeentity(){
	Undo_Start( "merge entity" );
	Undo_AddBrushList( &selected_brushes );
	Select_MergeEntity();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionGroupworld(){
	Undo_Start( "group world" );
	Undo_AddBrushList( &selected_brushes );
	Select_GroupEntity( world_entity );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMakeDetail(){
	Undo_Start( "make detail" );
	Undo_AddBrushList( &selected_brushes );
	Select_MakeDetail();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMakeStructural(){
	Undo_Start( "make structural" );
	Undo_AddBrushList( &selected_brushes );
	Select_MakeStructural();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBspCommand( unsigned int nID ){
	// make sure we don't attempt to region compile a map with the camera outside the region
	if ( region_active ) {
		vec3_t vOrig;
		VectorSet( vOrig,
				   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[0],
				   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[1],
				   (int)g_pParentWnd->GetCamWnd()->Camera()->origin[2] );

		int i;
		for ( i = 0 ; i < 3 ; i++ )
		{
			if ( vOrig[i] > region_maxs[i] || vOrig[i] < region_mins[i] ) {
				Sys_FPrintf( SYS_ERR, "The camera must be in the region to start a region compile.\n" );
				return;
			}
		}
	}

	// if the map has not been saved yet we need to handle it now before we start processing the BSP commands
	if ( stricmp( currentmap, "unnamed.map" ) == 0 ) {
		OnFileSaveas();
	}

	if ( g_PrefsDlg.m_bSnapShots && ( stricmp( currentmap, "unnamed.map" ) != 0 ) ) {
		Map_Snapshot();
	}

	if ( g_qeglobals.bBSPFrontendPlugin ) {
		char *cmd = (char*)g_slist_nth_data( g_BSPFrontendCommands, nID - CMD_BSPCOMMAND );
		g_BSPFrontendTable.m_pfnDispatchBSPCommand( cmd );
	}
	else
	{
		RunBsp( bsp_commands[nID - CMD_BSPCOMMAND] );
	}
}

void MainFrame::OnGrid( unsigned int nID ){
	if ( nID == ID_GRID_025 ) {
		g_qeglobals.d_gridsize = 0.25f;
		g_qeglobals.d_bSmallGrid = true;
	}
	else if ( nID == ID_GRID_05 ) {
		g_qeglobals.d_gridsize = 0.5f;
		g_qeglobals.d_bSmallGrid = true;
	}
	else
	{
		switch ( nID )
		{
		case ID_GRID_1: g_qeglobals.d_gridsize = 0; break;
		case ID_GRID_2: g_qeglobals.d_gridsize = 1; break;
		case ID_GRID_4: g_qeglobals.d_gridsize = 2; break;
		case ID_GRID_8: g_qeglobals.d_gridsize = 3; break;
		case ID_GRID_16: g_qeglobals.d_gridsize = 4; break;
		case ID_GRID_32: g_qeglobals.d_gridsize = 5; break;
		case ID_GRID_64: g_qeglobals.d_gridsize = 6; break;
		case ID_GRID_128: g_qeglobals.d_gridsize = 7; break;
		case ID_GRID_256: g_qeglobals.d_gridsize = 8; break;
		}
		g_qeglobals.d_gridsize = 1 << (int)g_qeglobals.d_gridsize;
		g_qeglobals.d_bSmallGrid = false;
	}

	SetGridStatus();

	// SnapTToGrid option: need to check everywhere the grid size is changed
	// this is a bit clumsy, have to do in OnGrid OnGridPrev and OnGridNext
	if ( g_PrefsDlg.m_bSnapTToGrid ) {
		DoSnapTToGrid();
	}

	Sys_UpdateWindows( W_XY | W_Z );
}

void MainFrame::OnSnaptogrid(){
	g_PrefsDlg.m_bSnap ^= 1;
	g_PrefsDlg.SavePrefs();

	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_snaptogrid" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bSnap ? TRUE : FALSE );
	g_bIgnoreCommands--;
}

void MainFrame::OnTexturesShowinuse(){
	Sys_BeginWait();
	Texture_ShowInuse();
#ifdef _DEBUG
	if ( !g_bShowAllShaders ) {
		Sys_Printf( "Already showing only in-use textures.\n" );
	}
#endif
	Sys_UpdateWindows( W_TEXTURE );
	Sys_EndWait();
}

void MainFrame::OnTexturesShowall(){
	Texture_ShowAll();
}

// do some triggering on/off, if the inspector is already up then hide it
void MainFrame::OnTexturesInspector(){
	ToggleSurface();
}

void MainFrame::OnViewNearest( unsigned int nID ){
	Texture_SetMode( nID );
}

void MainFrame::OnTextureReplaceall(){
	FindTextureDialog::show();
}

void MainFrame::OnToggleLock(){
	g_PrefsDlg.m_bTextureLock = !g_PrefsDlg.m_bTextureLock;

	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_toggle_lock" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bTextureLock ? TRUE : FALSE );
	g_bIgnoreCommands--;
	g_PrefsDlg.SavePrefs();
	SetGridStatus();
}

void MainFrame::OnToggleRotatelock(){
	g_PrefsDlg.m_bRotateLock ^= 1;

	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_toggle_rotatelock" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bRotateLock ? TRUE : FALSE );
	g_bIgnoreCommands--;
	g_PrefsDlg.SavePrefs();
	SetGridStatus();
}

// use a dialog for direct selection of a texture menu
// the API is a bit crappy, we need to set texture_directory to the directory name in <basepath>/textures/
void MainFrame::OnTexturesLoad(){
	char def_path[NAME_MAX];

	// FIXME
	// check if that works with fs_game (I suspect some more design is needed)
	// see how this is done in 1.2?
	strcpy( def_path, g_pGameDescription->mEnginePath.GetBuffer() );
	strcat( def_path, g_pGameDescription->mBaseGame.GetBuffer() );
	strcat( def_path, "/" );

	char *dir = dir_dialog( m_pWidget, _( "Load textures from path" ), def_path );

	if ( dir != NULL ) {
		// very uncertain task, let's hope the guy pointed to somewhere below the dir we gave him
		Sys_Printf( "user select: '%s'\n", dir );
		// remove a potential trailing slash?
		if ( dir[strlen( dir ) - 1] == '/' || dir[strlen( dir ) - 1] == '\\' ) {
			dir[strlen( dir ) - 1] = '\0';
		}
		char *pouic = MAX( strrchr( dir, '/' ),strrchr( dir, '\\' ) );
		if ( pouic ) {
			strcpy( texture_directory, pouic + 1 );
			Sys_Printf( "Loading '%s'\n", texture_directory );
			Texture_ShowDirectory();
		}
		else{
			Sys_FPrintf( SYS_WRN, "Failed to extract the directory\n" );
		}
		g_free( dir );
	}
	else{
		Sys_FPrintf( SYS_WRN, "texture load dialog cancelled\n" );
	}
}

void MainFrame::OnTexturesReloadshaders(){
	Sys_BeginWait();
	QERApp_ReloadShaders();
	// current shader
	// NOTE: we are kinda making it loop on itself, it will update the pShader and scroll the texture window
	Texture_SetTexture( &g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef, false, NULL, false );
	Sys_UpdateWindows( W_ALL );
	Sys_EndWait();

	GSList *texdirs = NULL;
	FillTextureList( &texdirs );
	FillTextureMenu( texdirs );
	FillTextureDirListWidget( texdirs );
	ClearGSList( texdirs );
}

void MainFrame::OnTexturesEmptyDirsHide(){
	g_PrefsDlg.m_bHideEmptyDirs ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_emptydirs_hide" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bHideEmptyDirs ? TRUE : FALSE );
	g_bIgnoreCommands--;

	GSList *texdirs = NULL;
	FillTextureList( &texdirs );
	FillTextureMenu( texdirs );
	FillTextureDirListWidget( texdirs );
	ClearGSList( texdirs );
}

void MainFrame::OnTexturesShadersShow(){
	g_PrefsDlg.m_bShowShaders ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_shaders_show" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bShowShaders ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_TEXTURE );
}

void MainFrame::SetTextureScale( int id ){
	GtkWidget *item;

	switch ( id )
	{
	case ID_TEXTURES_TEXTUREWINDOWSCALE_10:
		g_PrefsDlg.m_nTextureScale = 10;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_texturewindowscale_10" ) );
		break;
	case ID_TEXTURES_TEXTUREWINDOWSCALE_25:
		g_PrefsDlg.m_nTextureScale = 25;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_texturewindowscale_25" ) );
		break;
	case ID_TEXTURES_TEXTUREWINDOWSCALE_50:
		g_PrefsDlg.m_nTextureScale = 50;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_texturewindowscale_50" ) );
		break;
	case ID_TEXTURES_TEXTUREWINDOWSCALE_200:
		g_PrefsDlg.m_nTextureScale = 200;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_texturewindowscale_200" ) );
		break;
	default:
		g_PrefsDlg.m_nTextureScale = 100;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_textures_texturewindowscale_100" ) );
		break;
	}

	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	g_bIgnoreCommands--;

	Texture_ResetPosition();
}

void MainFrame::OnTexturewindowScaleup(){
	switch ( g_PrefsDlg.m_nTextureScale ) {
	// 200, all the way in, don't do anything
	case 100:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_200 );
		break;
	case 50:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_100 );
		break;
	case 25:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_50 );
		break;
	case 10:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_25 );
		break;
	}
}

void MainFrame::OnTexturewindowScaledown(){
	switch ( g_PrefsDlg.m_nTextureScale ) {
	case 200:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_100 );
		break;
	case 100:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_50 );
		break;
	case 50:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_25 );
		break;
	case 25:
		SetTextureScale( ID_TEXTURES_TEXTUREWINDOWSCALE_10 );
		break;
		// 10, all the way out, don't do anything
	}
}

void MainFrame::OnTexturesLoadlist(){
	DoTextureListDlg();
}

void MainFrame::OnTexturesShaderlistonly(){
	g_PrefsDlg.m_bTexturesShaderlistOnly ^= 1;
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ),"menu_textures_shaderlistonly" ) );
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), g_PrefsDlg.m_bTexturesShaderlistOnly ? TRUE : FALSE );
	g_bIgnoreCommands--;

	GSList *texdirs = NULL;
	FillTextureList( &texdirs );
	FillTextureMenu( texdirs );
	FillTextureDirListWidget( texdirs );
	ClearGSList( texdirs );
}

void MainFrame::OnTextureWad( unsigned int nID ){
	Sys_BeginWait();
	Texture_ShowDirectory( nID );
	Sys_UpdateWindows( W_ALL );
	Sys_EndWait();
}

void MainFrame::OnMiscBenchmark(){
	m_pCamWnd->BenchMark();
}

void MainFrame::OnColorSetoriginal(){
	for ( int i = 0 ; i < 3 ; i++ )
	{
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i] = 0.25f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i] = 1.0f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i] = 0.75f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][i] = 0.5f;
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i] = 0.25f;
	}

	//djbob
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR_ALT][0] = 0.5f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR_ALT][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR_ALT][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR_ALT][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR_ALT][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR_ALT][2] = 0.0f;
	//-djbob

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][0] = 0.5f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][2] = 0.75f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_DETAIL][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_DETAIL][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_DETAIL][2] = 0.0f;

	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorSetqer(){
	for ( int i = 0 ; i < 3 ; i++ )
	{
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i] = 0.25f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i] = 1.0f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i] = 1.0f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][i] = 0.5f;
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i] = 0.25f;
	}

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][0] = 0.5f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][2] = 0.75f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][2] = 0.0f;

	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorSetblack(){
	for ( int i = 0 ; i < 3 ; i++ )
	{
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i] = 0.25f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i] = 0.0f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i] = 0.2f;
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i] = 0.25f;
	}

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][0] = 0.3f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][1] = 0.5f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][2] = 0.5f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][1] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][1] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][0] = 0.7f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][1] = 0.7f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][2] = 0.0f;

	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_ALL );
}

/* ydnar: to emulate maya/max/lightwave color schemes */
void MainFrame::OnColorSetydnar(){
	for ( int i = 0 ; i < 3 ; i++ )
	{
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i] = 0.25f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i] = 0.77f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i] = 0.83f;
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][i] = 0.89f;
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i] = 0.25f;
	}

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][1] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER][2] = 1.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][0] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES][2] = 0.0f;

	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][0] = 0.5f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME][2] = 0.75f;

	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][0] = 1.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][1] = 0.0f;
	g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][2] = 0.0f;

	g_PrefsDlg.SavePrefs();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnTexturebk(){
	DoColor( COLOR_TEXTUREBACK );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsXybk(){
	DoColor( COLOR_GRIDBACK );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsMajor(){
	DoColor( COLOR_GRIDMAJOR );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsMinor(){
	DoColor( COLOR_GRIDMINOR );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsMajor_Alt(){
	DoColor( COLOR_GRIDMAJOR_ALT );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsMinor_Alt(){
	DoColor( COLOR_GRIDMINOR_ALT );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsGridtext(){
	DoColor( COLOR_GRIDTEXT );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsGridblock(){
	DoColor( COLOR_GRIDBLOCK );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsCameraBack(){
	DoColor( COLOR_CAMERABACK );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsBrush(){
	DoColor( COLOR_BRUSHES );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsSelectedbrush(){
	DoColor( COLOR_SELBRUSHES );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsSelectedbrush3D(){
	DoColor( COLOR_SELBRUSHES3D );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsClipper(){
	DoColor( COLOR_CLIPPER );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsViewname(){
	DoColor( COLOR_VIEWNAME );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnColorsDetail(){
	DoColor( COLOR_DETAIL );
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnMiscGamma(){
	float fSave = g_qeglobals.d_savedinfo.fGamma;
	DoGamma();
	if ( fSave != g_qeglobals.d_savedinfo.fGamma ) {
		gtk_MessageBox( m_pWidget, _( "You must restart Radiant for Gamma settings to take effect." ) );
	}
}
void MainFrame::OnMiscFindbrush(){
	DoFind();
}

void MainFrame::OnMiscNextleakspot(){
	Pointfile_Next();
}

void MainFrame::OnMiscPreviousleakspot(){
	Pointfile_Prev();
}

void MainFrame::OnMiscPrintxy(){
	WXY_Print();
}

void MainFrame::OnMiscSelectentitycolor(){
	if ( edit_entity ) {
		CString strColor = ValueForKey( edit_entity, "_color" );
		if ( strColor.GetLength() > 0 ) {
			float fR, fG, fB;
			int n = sscanf( strColor,"%f %f %f", &fR, &fG, &fB );
			if ( n == 3 ) {
				g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][0] = fR;
				g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][1] = fG;
				g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][2] = fB;
			}
		}

		if ( inspector_mode == W_ENTITY && ( DoColor( COLOR_ENTITY ) ) ) {
			char buffer[100];
			sprintf( buffer, "%f %f %f", g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][0],
					 g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][1],
					 g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][2] );

			gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), buffer );
			gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), "_color" );
			AddProp();
			//DK - SOF change to get color to entity quickly
			//--::SetWindowText( hwndEnt[EntValueField], buffer );
			//--::SetWindowText( hwndEnt[EntKeyField], "color" );
			//--AddProp();
		}
		Sys_UpdateWindows( W_ALL );
	}
}

void MainFrame::OnConvertcurves(){
#if 0
	Select_Deselect();
	for ( brush_t* pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		if ( pb->curveBrush ) {
			for ( face_t* f = pb->brush_faces ; f ; f = f->next )
			{
				if ( f->texdef.contents & CONTENTS_LADDER ) {
					f->texdef.contents &= ~CONTENTS_LADDER;
					f->texdef.contents |= CONTENTS_NEGATIVE_CURVE;
				}
			}
		}
	}
	Map_BuildBrushData();
#endif
}

void MainFrame::OnRegionOff(){
	Map_RegionOff();
}

void MainFrame::OnRegionSetxy(){
	Map_RegionXY();
}

void MainFrame::OnRegionSettallbrush(){
	Map_RegionTallBrush();
}

void MainFrame::OnRegionSetbrush(){
	Map_RegionBrush();
}

void MainFrame::OnRegionSetselection(){
	Map_RegionSelectedBrushes();
}

void MainFrame::OnBrush3sided(){
	Undo_Start( "3 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 3 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush4sided(){
	Undo_Start( "4 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 4 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush5sided(){
	Undo_Start( "5 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 5 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush6sided(){
	Undo_Start( "6 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 6 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush7sided(){
	Undo_Start( "7 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 7 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush8sided(){
	Undo_Start( "8 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 8 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrush9sided(){
	Undo_Start( "9 sided" );
	Undo_AddBrushList( &selected_brushes );
	Brush_MakeSided( 9 );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushArbitrarysided(){
	Undo_Start( "arbitrary sided" );
	Undo_AddBrushList( &selected_brushes );
	DoSides();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushMakecone(){
	Undo_Start( "make cone" );
	Undo_AddBrushList( &selected_brushes );
	DoSides( true );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnBrushPrimitivesSphere(){
	Undo_Start( "make sphere" );
	Undo_AddBrushList( &selected_brushes );

	DoSides( false, true );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchtube(){
	Undo_Start( "make curve cylinder" );
	Undo_AddBrushList( &selected_brushes );
	Patch_BrushToMesh( false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchdensetube(){
	Undo_Start( "dense cylinder" );
	Undo_AddBrushList( &selected_brushes );

	Patch_BrushToMesh( false );
	OnCurveInsertAddrow();
	Sys_UpdateWindows( W_ALL );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchverydensetube(){
	Undo_Start( "very dense cylinder" );
	Undo_AddBrushList( &selected_brushes );

	Patch_BrushToMesh( false );
	OnCurveInsertAddrow();
	OnCurveInsertInsertrow();
	OnCurveInsertAddrow();
	Sys_UpdateWindows( W_ALL );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchsquare(){
	Undo_Start( "square cylinder" );
	Undo_AddBrushList( &selected_brushes );

	Patch_BrushToMesh( false, false, false, true );
	Sys_UpdateWindows( W_ALL );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchendcap(){
	Undo_Start( "make end cap" );
	Undo_AddBrushList( &selected_brushes );
	Patch_BrushToMesh( false, false, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchbevel(){
	Undo_Start( "make bevel" );
	Undo_AddBrushList( &selected_brushes );
	Patch_BrushToMesh( false, true, false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveMoreendcapsbevelsSquarebevel(){
	Undo_Start( "square bevel" );
	Undo_AddBrushList( &selected_brushes );

	Patch_BrushToMesh( false, true, false, true );
	Sys_UpdateWindows( W_ALL );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveMoreendcapsbevelsSquareendcap(){
	Undo_Start( "square endcap" );
	Undo_AddBrushList( &selected_brushes );

	Patch_BrushToMesh( false, false, true, true );
	Sys_UpdateWindows( W_ALL );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurvePatchcone(){
	Undo_Start( "make curve cone" );
	Undo_AddBrushList( &selected_brushes );
	Patch_BrushToMesh( true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveSimplepatchmesh(){
	Undo_Start( "make simpe patch mesh" );
	Undo_AddBrushList( &selected_brushes );
	DoNewPatchDlg();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveInsertInsertcolumn(){
	Undo_Start( "insert (2) columns" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( true, true, false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveInsertAddcolumn(){
	Undo_Start( "add (2) columns" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( true, true, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveInsertInsertrow(){
	Undo_Start( "insert (2) rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( true, false, false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveInsertAddrow(){
	Undo_Start( "add (2) rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( true, false, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeleteFirstcolumn(){
	Undo_Start( "delete first (2) columns" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, true, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeleteLastcolumn(){
	Undo_Start( "delete last (2) columns" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, true, false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeleteFirstrow(){
	Undo_Start( "delete first (2) rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, false, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeleteLastrow(){
	Undo_Start( "delete last (2) rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, false, false );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveNegative(){
	Patch_ToggleInverted();
	//Sys_UpdateWindows(W_ALL);
}

void MainFrame::OnCurveRedisperseRows(){
	Undo_Start( "redisperse rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_DisperseRows();
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveRedisperseIntermediateCols(){
	Undo_Start( "redisperse im cols" );
	Undo_AddBrushList( &selected_brushes );
	Patch_DisperseIntermediateColumns();
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveRedisperseIntermediateRows(){
	Undo_Start( "redisperse im rows" );
	Undo_AddBrushList( &selected_brushes );
	Patch_DisperseIntermediateRows();
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveMatrixTranspose(){
	Patch_Transpose();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnCurveCap(){
	Patch_CapCurrent();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnCurveCyclecap(){
	Patch_CycleCapSelected();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnCurveOverlaySet(){
	Patch_SetOverlays();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnCurveOverlayClear(){
	Patch_ClearOverlays();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnCurveThicken(){
	Undo_Start( "curve thicken" );
	Undo_AddBrushList( &selected_brushes );
	DoThickenDlg();
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

/*!
   this can no longer be trigger manually from the menu
   happens only once at startup
 */
void MainFrame::OnPluginsRefresh(){
	CleanPlugInMenu();
	m_PlugInMgr.Init();
}

// open the Q3Rad manual
void MainFrame::OnHelp(){
	OpenURL( m_pWidget, "http://icculus.org/gtkradiant/documentation/q3radiant_manual/index.htm" );
}

// FIXME: we'll go towards a unified help thing soon
void MainFrame::OnHelpLinks(){
	Str link;
	link = g_strAppPath;
	link += "links.htm";
	OpenURL( m_pWidget, link.GetBuffer() );
}

void MainFrame::OnHelpBugreport(){
	OpenURL( m_pWidget, "https://github.com/TTimo/GtkRadiant/issues" );
}

void MainFrame::OnHelpCommandlist(){
	DoCommandListDlg();
}

void MainFrame::OnHelpAbout(){
	DoAbout();
}

void MainFrame::OnPopupSelection(){
	GtkWidget *menu, *item;
	const gchar *labels[] = { _( "Select Complete Tall" ), _( "Select Touching" ), _( "Select Partial Tall" ), _( "Select Inside" )};
	int ids[] = { ID_SELECTION_SELECTCOMPLETETALL, ID_SELECTION_SELECTTOUCHING,
				  ID_SELECTION_SELECTPARTIALTALL, ID_SELECTION_SELECTINSIDE};

	menu = gtk_menu_new();

	for ( int i = 0; i < 4; i++ )
	{
		item = gtk_menu_item_new_with_label( labels[i] );
		g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
							GINT_TO_POINTER( ids[i] ) );
		gtk_widget_show( item );
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
	}

	gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

void MainFrame::OnViewChange(){
	OnViewNextview();
	//HandlePopup(this, IDR_POPUP_VIEW);
}

void MainFrame::OnTexturesPopup(){
	gpointer item = g_object_get_data( G_OBJECT( m_pWidget ), "render_quality_menu" );
	gtk_menu_popup( GTK_MENU( item ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

void MainFrame::ToggleCamera(){
	if ( m_bCamPreview ) {
		m_bCamPreview = false;
	}
	else{
		m_bCamPreview = true;
	}
}

void MainFrame::OnViewCameraupdate(){
	Sys_UpdateWindows( W_CAMERA );
}

void MainFrame::OnSelectMouserotate(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_select_mouserotate" ) );
	g_bIgnoreCommands++;

	if ( ActiveXY() ) {
		if ( ActiveXY()->ClipMode() ) {
			OnViewClipper();
		}
		if ( ActiveXY()->RotateMode() ) {
			// SetRotateMode(false) always works
			ActiveXY()->SetRotateMode( false );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
		}
		else
		{
			// may not work if no brush selected, see return value
			if ( ActiveXY()->SetRotateMode( true ) ) {
				gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), TRUE );
			}
			else{
				// if MFC called, we need to set back to FALSE ourselves
				gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
			}
		}
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnSelectMousescale(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_select_mousescale" ) );
	g_bIgnoreCommands++;

	if ( ActiveXY() ) {
		if ( ActiveXY()->ClipMode() ) {
			OnViewClipper();
		}
		if ( ActiveXY()->RotateMode() ) {
			// SetRotateMode(false) always works
			ActiveXY()->SetRotateMode( false );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
		}
		if ( ActiveXY()->ScaleMode() ) {
			ActiveXY()->SetScaleMode( false );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
		}
		else
		{
			ActiveXY()->SetScaleMode( true );
			gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), TRUE );
		}
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnScalelockx(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_scalelockx" ) );
	g_bIgnoreCommands++;

	if ( g_nScaleHow & SCALE_X ) {
		g_nScaleHow ^= SCALE_X;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
	}
	else
	{
		g_nScaleHow |= SCALE_X;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), TRUE );
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnScalelocky(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_scalelocky" ) );
	g_bIgnoreCommands++;

	if ( g_nScaleHow & SCALE_Y ) {
		g_nScaleHow ^= SCALE_Y;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
	}
	else
	{
		g_nScaleHow |= SCALE_Y;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), TRUE );
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnScalelockz(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_scalelockz" ) );
	g_bIgnoreCommands++;

	if ( g_nScaleHow & SCALE_Z ) {
		g_nScaleHow ^= SCALE_Z;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), FALSE );
	}
	else
	{
		g_nScaleHow |= SCALE_Z;
		gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), TRUE );
	}
	g_bIgnoreCommands--;
}

void MainFrame::OnDontselectcurve(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_dontselectcurve" ) );
	g_bIgnoreCommands++;
	g_PrefsDlg.m_bSelectCurves ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_PrefsDlg.m_bSelectCurves ) ? FALSE : TRUE );
	g_bIgnoreCommands--;
}

void MainFrame::OnPatchToggleBox(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_showboundingbox" ) );
	g_bIgnoreCommands++;
	g_bPatchShowBounds ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchShowBounds ) ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnPatchWireframe(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_wireframe" ) );
	g_bIgnoreCommands++;
	g_bPatchWireFrame ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchWireFrame ) ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnPatchBend(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_bend" ) );
	g_bIgnoreCommands++;
	Patch_BendToggle();
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchBendMode ) ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnPatchWeld(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_weld" ) );
	g_bIgnoreCommands++;
	g_bPatchWeld ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchWeld ) ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnPatchDrilldown(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_patch_drilldown" ) );
	g_bIgnoreCommands++;
	g_bPatchDrillDown ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_bPatchDrillDown ) ? TRUE : FALSE );
	g_bIgnoreCommands--;
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnShowEntities(){
	gpointer item = g_object_get_data( G_OBJECT( m_pWidget ), "view_entitiesas_menu" ); // use pointer to existing menu object
	gtk_menu_popup( GTK_MENU( item ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

void MainFrame::OnDropGroupName(){
	/*
	   char* name = DoNameDlg ("Name Selection");

	   if (name != NULL)
	   {
	   Select_Name (name);
	   Sys_UpdateWindows (W_ALL);
	   free (name);
	   }
	 */
}

void MainFrame::OnDropGroupNewgroup(){

}

void MainFrame::OnDropGroupRemove(){
	/*
	   Select_AddToGroup("World");
	   Sys_UpdateWindows (W_ALL);
	 */
}

// NOTE: it's called OnFaceFit() but we want to process everything here, faces and patches
void MainFrame::OnFaceFit(){
	SurfaceDlgFitAll();
}

void MainFrame::OnDontselectmodel(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "ttb_dontselectmodel" ) );
	g_bIgnoreCommands++;
	g_PrefsDlg.m_bSelectModels ^= 1;
	gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( item ), ( g_PrefsDlg.m_bSelectModels ) ? FALSE : TRUE );
	g_bIgnoreCommands--;
}

void MainFrame::OnViewTexture(){
	if ( FloatingGroupDialog() ) { // QE4 style
		if ( inspector_mode == W_TEXTURE ) {
			if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
				widget_delete_hide( g_qeglobals_gui.d_entity );
			}
			else{
				gtk_widget_show( g_qeglobals_gui.d_entity );
			}
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_TEXTURE );
		}
	}
}

void MainFrame::OnPatchInspector(){
	TogglePatchInspector();
}

void MainFrame::OnCurveNegativeTextureX(){
	Patch_InvertTexture( false );
	//Sys_UpdateWindows(W_ALL);
}

void MainFrame::OnCurveNegativeTextureY(){
	Patch_InvertTexture( true );
	//Sys_UpdateWindows(W_ALL);
}

void MainFrame::OnCurveInsertcolumn(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "insert colum" );
	Undo_AddBrushList( &selected_brushes );
	//Patch_AdjustSelectedRowCols(0, 2);
	Patch_AdjustSelected( true, true, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveInsertrow(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "insert row" );
	Undo_AddBrushList( &selected_brushes );
	//Patch_AdjustSelectedRowCols(2, 0);
	Patch_AdjustSelected( true, false, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeletecolumn(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "delete column" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, true, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnCurveDeleterow(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "delete row" );
	Undo_AddBrushList( &selected_brushes );
	Patch_AdjustSelected( false, false, true );
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnPatchTab(){
	if ( g_bPatchBendMode ) {
		Patch_BendHandleTAB();
	}
//  else if (g_bPatchInsertMode)
//    Patch_InsDelHandleTAB();
	else
	{
		// check to see if the selected brush is part of a func group
		// if it is, deselect everything and reselect the next brush
		// in the group
		brush_t *b2, *b = selected_brushes.next;
		entity_t * e;
		if ( b != &selected_brushes ) {
			if ( strcmpi( b->owner->eclass->name, "worldspawn" ) != 0 ) {
				e = b->owner;
				Select_Deselect();
				for ( b2 = e->brushes.onext ; b2 != &e->brushes ; b2 = b2->onext )
				{
					if ( b == b2 ) {
						b2 = b2->onext;
						break;
					}
				}
				if ( b2 == &e->brushes ) {
					b2 = b2->onext;
				}

				Select_Brush( b2, false );
				Sys_UpdateWindows( W_ALL );
			}
		}
	}
}

void MainFrame::OnSelectFuncGroup(){
	// check to see if the selected brush is part of a func group
	// if it is, deselect everything and reselect the next brush
	// in the group
	brush_t *b = selected_brushes.next;
	entity_t * e;
	if ( b != &selected_brushes ) {
		if ( strcmpi( b->owner->eclass->name, "worldspawn" ) != 0 ) {
			e = b->owner;
			Select_SelectGroup( e );
		}
	}
}

void MainFrame::OnCameraForward( bool keydown ){
	if ( g_PrefsDlg.m_bCamDiscrete && ( m_pCamWnd && !m_pCamWnd->m_bFreeMove ) ) {
		if ( keydown ) {
			VectorMA( m_pCamWnd->Camera()->origin, SPEED_MOVE, m_pCamWnd->Camera()->forward, m_pCamWnd->Camera()->origin );
			int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
			Sys_UpdateWindows( nUpdate );
		}
	}
	else {
		if ( keydown ) {
			m_pCamWnd->Camera()->movementflags |= MOVE_FORWARD;
		}
		else{
			m_pCamWnd->Camera()->movementflags &= ~MOVE_FORWARD;
		}
	}
}

void MainFrame::OnCameraBack( bool keydown ){
	if ( g_PrefsDlg.m_bCamDiscrete && ( m_pCamWnd && !m_pCamWnd->m_bFreeMove ) ) {
		if ( keydown ) {
			VectorMA( m_pCamWnd->Camera()->origin, -SPEED_MOVE, m_pCamWnd->Camera()->forward, m_pCamWnd->Camera()->origin );
			int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
			Sys_UpdateWindows( nUpdate );
		}
	}
	else {
		if ( keydown ) {
			m_pCamWnd->Camera()->movementflags |= MOVE_BACK;
		}
		else{
			m_pCamWnd->Camera()->movementflags &= ~MOVE_BACK;
		}
	}
}

void MainFrame::OnCameraLeft( bool keydown ){
	if ( m_pCamWnd ) {
		if ( m_pCamWnd->m_bFreeMove ) {
			OnCameraStrafeleft( keydown );
			return;
		}
	}

	if ( g_PrefsDlg.m_bCamDiscrete ) {
		if ( keydown ) {
			m_pCamWnd->Camera()->angles[1] += SPEED_TURN;
			int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
			Sys_UpdateWindows( nUpdate );
		}
	}
	else {
		if ( keydown ) {
			m_pCamWnd->Camera()->movementflags |= MOVE_ROTLEFT;
		}
		else{
			m_pCamWnd->Camera()->movementflags &= ~MOVE_ROTLEFT;
		}
	}
}

void MainFrame::OnCameraRight( bool keydown ){
	if ( m_pCamWnd ) {
		if ( m_pCamWnd->m_bFreeMove ) {
			OnCameraStraferight( keydown );
			return;
		}
	}

	if ( g_PrefsDlg.m_bCamDiscrete ) {
		if ( keydown ) {
			m_pCamWnd->Camera()->angles[1] -= SPEED_TURN;
			int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
			Sys_UpdateWindows( nUpdate );
		}
	}
	else {
		if ( keydown ) {
			m_pCamWnd->Camera()->movementflags |= MOVE_ROTRIGHT;
		}
		else{
			m_pCamWnd->Camera()->movementflags &= ~MOVE_ROTRIGHT;
		}
	}
}

void MainFrame::OnCameraUp(){
	m_pCamWnd->Camera()->origin[2] += SPEED_MOVE;
	int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY | W_Z ) : ( W_CAMERA );
	Sys_UpdateWindows( nUpdate );
}

void MainFrame::OnCameraDown(){
	m_pCamWnd->Camera()->origin[2] -= SPEED_MOVE;
	int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY | W_Z ) : ( W_CAMERA );
	Sys_UpdateWindows( nUpdate );
}

void MainFrame::OnCameraAngleup(){
	m_pCamWnd->Camera()->angles[0] += SPEED_TURN;
	if ( m_pCamWnd->Camera()->angles[0] > 85 ) {
		m_pCamWnd->Camera()->angles[0] = 85;
	}
	Sys_UpdateWindows( W_CAMERA | W_XY_OVERLAY );
}

void MainFrame::OnCameraAngledown(){
	m_pCamWnd->Camera()->angles[0] -= SPEED_TURN;
	if ( m_pCamWnd->Camera()->angles[0] < -85 ) {
		m_pCamWnd->Camera()->angles[0] = -85;
	}
	Sys_UpdateWindows( W_CAMERA | W_XY_OVERLAY );
}

void MainFrame::OnCameraStrafeleft( bool keydown ){
	// FIXME: as soon as gtk supports proper keyup/down support, remove this bit
	if ( m_pCamWnd ) {
		if ( !m_pCamWnd->m_bFreeMove ) {
			if ( keydown ) {
				VectorMA( m_pCamWnd->Camera()->origin, -SPEED_MOVE, m_pCamWnd->Camera()->right, m_pCamWnd->Camera()->origin );
				int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
				Sys_UpdateWindows( nUpdate );
			}
			return;
		}
	}

	if ( keydown ) {
		m_pCamWnd->Camera()->movementflags |= MOVE_STRAFELEFT;
	}
	else{
		m_pCamWnd->Camera()->movementflags &= ~MOVE_STRAFELEFT;
	}
}

void MainFrame::OnCameraStraferight( bool keydown ){
	// FIXME: as soon as gtk supports proper keyup/down support, remove this bit
	if ( m_pCamWnd ) {
		if ( !m_pCamWnd->m_bFreeMove ) {
			if ( keydown ) {
				VectorMA( m_pCamWnd->Camera()->origin, SPEED_MOVE, m_pCamWnd->Camera()->right, m_pCamWnd->Camera()->origin );
				int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
				Sys_UpdateWindows( nUpdate );
			}
			return;
		}
	}

	if ( keydown ) {
		m_pCamWnd->Camera()->movementflags |= MOVE_STRAFERIGHT;
	}
	else{
		m_pCamWnd->Camera()->movementflags &= ~MOVE_STRAFERIGHT;
	}
}

void MainFrame::OnGridToggle(){
	g_qeglobals.d_showgrid = !g_qeglobals.d_showgrid;
	Sys_UpdateWindows( W_XY | W_Z );
}

void MainFrame::OnViewCrosshair(){
	g_bCrossHairs ^= 1;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnSelectionTextureRotateclock(){
	Select_RotateTexture( abs( g_PrefsDlg.m_nRotation ) );
}

void MainFrame::OnSelectionTextureRotatecounter(){
	Select_RotateTexture( -abs( g_PrefsDlg.m_nRotation ) );
}

void MainFrame::OnSelectionTextureScaleup(){
	Select_ScaleTexture( 0, g_qeglobals.d_savedinfo.m_SIIncrement.scale[1] );
}

void MainFrame::OnSelectionTextureScaledown(){
	Select_ScaleTexture( 0, -g_qeglobals.d_savedinfo.m_SIIncrement.scale[1] );
}

void MainFrame::OnSelectionTextureScaleLeft(){
	Select_ScaleTexture( -g_qeglobals.d_savedinfo.m_SIIncrement.scale[0],0 );
}

void MainFrame::OnSelectionTextureScaleRight(){
	Select_ScaleTexture( g_qeglobals.d_savedinfo.m_SIIncrement.scale[0],0 );
}

void MainFrame::OnSelectionTextureShiftleft(){
	Select_ShiftTexture( (int)-g_qeglobals.d_savedinfo.m_SIIncrement.shift[0], 0 );
}

void MainFrame::OnSelectionTextureShiftright(){
	Select_ShiftTexture( (int)g_qeglobals.d_savedinfo.m_SIIncrement.shift[0], 0 );
}

void MainFrame::OnSelectionTextureShiftup(){
	Select_ShiftTexture( 0, (int)g_qeglobals.d_savedinfo.m_SIIncrement.shift[1] );
}

void MainFrame::OnSelectionTextureShiftdown(){
	Select_ShiftTexture( 0, (int)-g_qeglobals.d_savedinfo.m_SIIncrement.shift[1] );
}

void MainFrame::OnGridPrev(){
	GtkWidget *item;
	if ( g_qeglobals.d_gridsize == 1 ) {
		g_qeglobals.d_gridsize = 0.5;
		g_qeglobals.d_bSmallGrid = true;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_05" ) );
	}
	else if ( g_qeglobals.d_gridsize == 0.5 ) {
		g_qeglobals.d_gridsize = 0.25;
		g_qeglobals.d_bSmallGrid = true;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_025" ) );
	}
	else if ( g_qeglobals.d_gridsize > 1 ) {
		g_qeglobals.d_gridsize = (int)g_qeglobals.d_gridsize >> 1;
		g_qeglobals.d_bSmallGrid = false;

		switch ( (int)g_qeglobals.d_gridsize )
		{
		case  1: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_1" ) ); break;
		case  2: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_2" ) ); break;
		case  4: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_4" ) ); break;
		case  8: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_8" ) ); break;
		case  16: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_16" ) ); break;
		case  32: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_32" ) ); break;
		case  64: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_64" ) ); break;
		case 128: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_128" ) ); break;
		case 256: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_256" ) ); break;
		default: return;
		}

	}
	else{
		return;
	}

	// SnapTToGrid option: need to check everywhere the grid size is changed
	// this is a bit clumsy, have to do in OnGrid OnGridPrev and OnGridNext
	if ( g_PrefsDlg.m_bSnapTToGrid ) {
		DoSnapTToGrid();
	}

	SetGridStatus();
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	g_bIgnoreCommands--;

	Sys_UpdateWindows( W_XY | W_Z );
}

void MainFrame::OnGridNext(){
	GtkWidget *item;
	if ( g_qeglobals.d_gridsize == 0.25 ) {
		g_qeglobals.d_gridsize = 0.5;
		g_qeglobals.d_bSmallGrid = true;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_05" ) );
	}
	else if ( g_qeglobals.d_gridsize == 0.5 ) {
		g_qeglobals.d_gridsize = 1;
		g_qeglobals.d_bSmallGrid = false;
		item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_1" ) );
	}
	else if ( g_qeglobals.d_gridsize < 256 ) {
		g_qeglobals.d_gridsize = (int)g_qeglobals.d_gridsize << 1;
		g_qeglobals.d_bSmallGrid = false;

		switch ( (int)g_qeglobals.d_gridsize )
		{
		case  1: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_1" ) ); break;
		case  2: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_2" ) ); break;
		case  4: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_4" ) ); break;
		case  8: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_8" ) ); break;
		case  16: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_16" ) ); break;
		case  32: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_32" ) ); break;
		case  64: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_64" ) ); break;
		case 128: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_128" ) ); break;
		case 256: item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_grid_256" ) ); break;
		default:  item = NULL;
		}

	}
	else{
		return;
	}

	// SnapTToGrid option: need to check everywhere the grid size is changed
	// this is a bit clumsy, have to do in OnGrid OnGridPrev and OnGridNext
	if ( g_PrefsDlg.m_bSnapTToGrid ) {
		DoSnapTToGrid();
	}

	SetGridStatus();
	g_bIgnoreCommands++;
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	g_bIgnoreCommands--;

	Sys_UpdateWindows( W_XY | W_Z );
}

void MainFrame::OnSelectionMovedown(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "move down" );
	Undo_AddBrushList( &selected_brushes );

	vec3_t vAmt;
	vAmt[0] = vAmt[1] = 0.0;
	vAmt[2] = -g_qeglobals.d_gridsize;
	Select_Move( vAmt );
	Sys_UpdateWindows( W_CAMERA | W_XY | W_Z );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionMoveup(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "move up" );
	Undo_AddBrushList( &selected_brushes );

	vec3_t vAmt;
	vAmt[0] = vAmt[1] = 0.0;
	vAmt[2] = g_qeglobals.d_gridsize;
	Select_Move( vAmt );
	Sys_UpdateWindows( W_CAMERA | W_XY | W_Z );

	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectionPrint(){
	for ( brush_t* b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
		Brush_Print( b );
}

void MainFrame::OnSelectionTogglesizepaint(){
	g_PrefsDlg.m_bSizePaint = !g_PrefsDlg.m_bSizePaint;
	Sys_UpdateWindows( W_XY );
}

void MainFrame::OnPatchNaturalize(){
	Patch_NaturalizeSelected();
	Sys_UpdateWindows( W_ALL );
}

void MainFrame::OnSnapToGrid(){
	if ( &selected_brushes == selected_brushes.next ) {
		return;
	}
	Undo_Start( "snap selection to grid" );
	Undo_AddBrushList( &selected_brushes );
	//Select_SnapToGrid();
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			Patch_SnapToGrid( pb->pPatch );
		}
		else{
			Brush_SnapToGrid( pb );
		}
	}
	Sys_UpdateWindows( W_ALL );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

void MainFrame::OnSelectAll(){
	Select_AllOfType();
}

void MainFrame::OnSelectionInvert(){
	Select_Invert();
	Sys_UpdateWindows( W_XY | W_Z | W_CAMERA );
}


void PerformFiltering(){
	brush_t *brush;

	FilterUpdateBase();

	for ( brush = active_brushes.next; brush != &active_brushes; brush = brush->next )
		brush->bFiltered = FilterBrush( brush );

	for ( brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next )
		brush->bFiltered = FilterBrush( brush );
}

void MainFrame::OnFilterAreaportals(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_areaportals" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_AREAPORTALS ) & EXCLUDE_AREAPORTALS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterCaulk(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_caulk" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CAULK ) & EXCLUDE_CAULK ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterClips(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_clips" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CLIP ) & EXCLUDE_CLIP ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterBotClips(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_botclips" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_BOTCLIP ) & EXCLUDE_BOTCLIP ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterStructural(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_structural" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_STRUCTURAL ) & EXCLUDE_STRUCTURAL ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterDetails(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_details" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_DETAILS ) & EXCLUDE_DETAILS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterEntities(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_entities" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_ENT ) & EXCLUDE_ENT ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterHintsskips(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_hintsskips" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_HINTSSKIPS ) & EXCLUDE_HINTSSKIPS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterLights(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_lights" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_LIGHTS ) & EXCLUDE_LIGHTS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterLiquids(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_liquids" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_LIQUIDS ) & EXCLUDE_LIQUIDS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterModels(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_models" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_MODELS ) & EXCLUDE_MODELS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterPatches(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_patches" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CURVES ) & EXCLUDE_CURVES ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterPaths(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_paths" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_PATHS ) & EXCLUDE_PATHS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterClusterportals(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_clusterportals" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CLUSTERPORTALS ) & EXCLUDE_CLUSTERPORTALS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterLightgrid(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_lightgrid" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_LIGHTGRID ) & EXCLUDE_LIGHTGRID ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterTranslucent(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_translucent" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_TRANSLUCENT ) & EXCLUDE_TRANSLUCENT ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterTriggers(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_triggers" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_TRIGGERS ) & EXCLUDE_TRIGGERS ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

void MainFrame::OnFilterWorld(){
	GtkWidget *item = GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), "menu_filter_world" ) );
	g_bIgnoreCommands++;
	if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_WORLD ) & EXCLUDE_WORLD ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	else{
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), FALSE );
	}
	g_bIgnoreCommands--;
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}








// =============================================================================
// leo: Unused functions, not called anywhere from the code (need to check)

void MainFrame::OnViewConsole(){
	if ( FloatingGroupDialog() ) { // QE4 style
		if ( inspector_mode == W_CONSOLE && CurrentStyle() != MainFrame::eFloating ) { // are we in console mode already?
			if ( gtk_widget_get_visible( g_qeglobals_gui.d_entity ) ) {
				widget_delete_hide( g_qeglobals_gui.d_entity );
			}
			else{
				gtk_widget_show( g_qeglobals_gui.d_entity );
			}
		}
		else
		{
			gtk_widget_show( g_qeglobals_gui.d_entity );
			SetInspectorMode( W_CONSOLE );
		}
	}
}

void MainFrame::OnCurveFreeze(){
	Patch_Freeze();
}

void MainFrame::OnCurveUnFreeze(){
	Patch_UnFreeze( false );
}

void MainFrame::OnCurveUnFreezeAll(){
	Patch_UnFreeze( true );
}

void MainFrame::OnSelectReselect(){
	Select_Reselect();
}

void MainFrame::OnSelectionTextureFit(){
	// TODO: Add your command handler code here
}

void MainFrame::OnPatchEnter(){

}

void MainFrame::OnDropGroupAddtoWorld(){
	/*
	   Select_AddToGroup("World");
	   Sys_UpdateWindows (W_ALL);
	 */
}
