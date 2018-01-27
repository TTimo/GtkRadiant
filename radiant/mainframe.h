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

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include "xywindow.h"
#include "texwindow.h"
#include "zwindow.h"
#include "camwindow.h"
#include "watchbsp.h"

#include "pluginmanager.h"
#include "plugin.h"

#include "gtkr_vector.h"

#ifdef __APPLE__
#define __toascii( c )    ( ( c ) & 0x7f )
#endif

const int RAD_SHIFT =   0x01;
const int RAD_ALT =     0x02;
const int RAD_CONTROL = 0x04;
const int RAD_PRESS   = 0x08;

struct SCommandInfo
{
	const char* m_strCommand;
	unsigned int m_nKey;
	unsigned int m_nModifiers;
	unsigned int m_nCommand;
	const char* m_strMenu;
};

struct SKeyInfo
{
	const char* m_strName;
	unsigned int m_nVKKey;
};

#define ID_FILE_NEW 0xE100
#define ID_FILE_OPEN 0xE101
#define ID_FILE_SAVE 0xE103
#define ID_EDIT_UNDO 0xE12B
#define ID_EDIT_REDO 0xE12C
#define ID_HELP 0xE146
#define ID_FILE_RECENT1 0xE110
#define ID_FILE_RECENT2 0xE111
#define ID_FILE_RECENT3 0xE112
#define ID_FILE_RECENT4 0xE113

#define IDC_BTN_FACEFIT                 1143
#define ID_ENTITY_START                 22800
#define ID_ENTITY_END                   32000 //leo
//#define ID_ENTITY_END                   33500
#define ID_VIEW_XY                      32772
#define ID_VIEW_SIDE                    32773
#define ID_VIEW_FRONT                   32774
#define ID_CAMERATOGGLE                 32775
#define ID_VIEW_CAMERATOGGLE            32776
#define ID_BUTTON32777                  32777
#define ID_BUTTON32778                  32778
#define ID_TEXTURES_POPUP               32780
#define ID_POPUP_SELECTION              32782
#define ID_VIEW_CHANGE                  32783
#define ID_VIEW_CAMERAUPDATE            32784
#define ID_VIEW_CLIPPER                 32785
#define ID_PREFS                        32786
#define ID_TOGGLE_LOCK                  32787
#define ID_EDIT_MAPINFO                 32788
#define ID_EDIT_ENTITYINFO              32789
#define ID_BRUSH_SCRIPTS                32790
#define ID_VIEW_NEXTVIEW                32791
#define ID_HELP_COMMANDLIST             32792
#define ID_FILE_NEWPROJECT              32793
#define ID_SNAPTOGRID                   32795
#define ID_VIEW_CENTERVIEW              32796
#define ID_SPLIT_SELECTED               32823
#define ID_CLIP_SELECTED                32824
#define ID_FLIP_CLIP                    32825
#define ID_TOGGLEVIEW_YZ                32831
#define ID_TOGGLEVIEW_XZ                32832
#define ID_COLORS_GRIDTEXT              32833
#define ID_COLORS_BRUSH                 32834
#define ID_COLORS_SELECTEDBRUSH         32835
#define ID_COLORS_CLIPPER               32836
#define ID_COLORS_GRIDBLOCK             32837
#define ID_COLORS_VIEWNAME              32838
#define ID_COLORS_DETAIL                37002
#define ID_COLOR_SETORIGINAL            32839
#define ID_COLOR_SETQER                 32840
#define ID_COLOR_SETBLACK               32841
#define ID_COLOR_SETYDNAR               37001  /* ydnar */
#define ID_BYEBYE                       32842
#define ID_SELECT_SCALE                 32843
#define ID_SELECT_MOUSEROTATE           32844
#define ID_COLORS_SELECTEDBRUSH3D       32845
#define ID_COLORS_CAMERABACK            32846
#define ID_TEXTURE_REPLACESELECTED      32859
#define ID_TEXTURE_REPLACEALL           32860
#define ID_SELECT_MOUSESCALE            32866
#define ID_SCALELOCKX                   32867
#define ID_SCALELOCKY                   32868
#define ID_SCALELOCKZ                   32869
#define ID_VIEW_CUBICCLIPPING           32870
#define ID_FILE_PROJECTSETTINGS         32875
#define ID_VIEW_CUBEOUT                 32876
#define ID_VIEW_CUBEIN                  32877
#define ID_NODES_LOADNODES              32878
#define ID_NODES_SHOWNODES              32879
#define ID_NODES_SHOWLINKS              32880
#define ID_NODES_REMOVEALLNODES         32881
#define ID_NODES_COUNTNODES             32882
#define ID_NODES_GIVEMONEYTONELNO       32883
#define ID_FILE_SAVEREGION              32887
#define ID_FILE_LOADREGION              32888
#define ID_SELECTION_MOVEDOWN           32890
#define ID_TOOLBAR_MAIN                 32891
#define ID_SELECTION_MOVEUP             32892
//#define ID_TOOLBAR_TEXTURE              32892
#define ID_BRUSH_MAKECONE               32896
#define ID_TEXTURES_LOAD                32897
#define ID_TOGGLE_ROTATELOCK            32898
#define ID_FILE_IMPORTMAP               32911
#define ID_FILE_EXPORTMAP               32912
#define ID_EDIT_LOADPREFAB              32913
#define ID_SELECTION_SELECT_NUDGELEFT   32916
#define ID_SELECTION_SELECT_NUDGERIGHT  32917
#define ID_SELECTION_SELECT_NUDGEUP     32918
#define ID_SELECTION_SELECT_NUDGEDOWN   32919
#define ID_TEXTURES_LOADLIST            32920
#define ID_DONTSELECTCURVE              32923
#define ID_CONVERTCURVES                32924
#define ID_PATCH_SHOWBOUNDINGBOX        32926
#define ID_CURVE_SIMPLEPATCHMESH        32927
#define ID_PATCH_WIREFRAME              32928
#define ID_PATCH_WELD                   32929
#define ID_CURVE_PATCHTUBE              32930
#define ID_CURVE_PATCHCONE              32931
#define ID_CURVE_PATCHENDCAP            32932
#define ID_CURVE_PATCHBEVEL             32933
#define ID_PATCH_DRILLDOWN              32936
#define ID_CURVE_LOADPATCHFILE          32937
#define ID_CURVE_INSERTROW              32938
#define ID_CURVE_INSERTCOLUMN           32939
#define ID_CURVE_DELETEROW              32940
#define ID_CURVE_DELETECOLUMN           32941
#define ID_BUTTON32942                  32942
//#define ID_PATCH_INSDEL                 32942
#define ID_CURVE_INSERT_ADDCOLUMN       32943
#define ID_CURVE_INSERT_INSERTCOLUMN    32944
#define ID_CURVE_INSERT_ADDROW          32945
#define ID_CURVE_INSERT_INSERTROW       32946
#define ID_CURVE_DELETE_FIRSTCOLUMN     32947
#define ID_CURVE_DELETE_LASTCOLUMN      32948
#define ID_CURVE_DELETE_FIRSTROW        32949
#define ID_CURVE_DELETE_LASTROW         32950
#define ID_CURVE_NEGATIVE               32951
#define ID_PATCH_BEND                   32952
#define ID_CURVE_PATCHDENSETUBE         32955
#define ID_CURVE_PATCHVERYDENSETUBE     32956
#define ID_CURVE_CAP                    32957
#define ID_CURVE_REDISPERSE_ROWS        32961
#define ID_PATCH_NATURALIZE             32963
#define ID_CURVE_PATCHSQUARE            32964
#define ID_BRUSH_PRIMITIVES_SPHERE      32965
#define ID_BRUSH_PRIMITIVES_TORUS       32966
#define ID_TEXTURES_TEXTUREWINDOWSCALE_200 32967
#define ID_TEXTURES_TEXTUREWINDOWSCALE_100 32968
#define ID_TEXTURES_TEXTUREWINDOWSCALE_50 32969
#define ID_TEXTURES_TEXTUREWINDOWSCALE_25 32970
#define ID_TEXTURES_TEXTUREWINDOWSCALE_10 32971
#define ID_CURVE_NEGATIVETEXTUREX       32972
#define ID_TEXTURES_FLUSH               32973
#define ID_CURVE_OVERLAY_SET            32974
#define ID_CURVE_OVERLAY_CLEAR          32975
#define ID_CURVE_NEGATIVETEXTUREY       32976
#define ID_CURVE_THICKEN                32977
#define ID_CURVE_CYCLECAP               32978
#define ID_CURVE_MATRIX_TRANSPOSE       32981
#define ID_PLUGINS_REFRESH              32982
#define ID_TEXTURES_RELOADSHADERS       32983
#define ID_VIEW_ENTITIESAS_BOUNDINGBOX  32984
#define ID_VIEW_ENTITIESAS_WRITEFRAME   32985
#define ID_VIEW_ENTITIESAS_SELECTEDWIREFRAME 32986
#define ID_VIEW_ENTITIESAS_SELECTEDSKINNED 32987
#define ID_VIEW_ENTITIESAS_SKINNED      32988
#define ID_VIEW_ENTITIESAS_SKINNEDANDBOXED 32989
#define ID_SHOW_ENTITIES                32990
#define ID_VIEW_ENTITIESAS_WIREFRAME    32991
#define ID_VIEW_OPENGLLIGHTING          32998
#define ID_EDIT_SAVEPREFAB              33001
#define ID_CURVE_MOREENDCAPSBEVELS_SQUAREENDCAP 33002
#define ID_CURVE_MOREENDCAPSBEVELS_SQUAREBEVEL 33003
#define ID_CURVE_PRIMITIVES_SPHERE      33005
#define ID_VIEW_HIDESHOW_HIDESELECTED   33006
#define ID_VIEW_HIDESHOW_SHOWHIDDEN     33007
#define ID_TEXTURES_SHADERS_SHOW        33008
//#define ID_SELECTION_CSGADD             33009
#define ID_TEXTURES_EMPTYDIRS_HIDE      33010
#define ID_SELECTION_CSGMERGE           33011
#define ID_TEXTURES_FLUSH_UNUSED        33014
#define ID_DROP_GROUP_REMOVE            33016
#define ID_DROP_GROUP_ADDTO_WORLD       33017
#define ID_DROP_GROUP_NEWGROUP          33018
#define ID_DROP_GROUP_NAME              33019
#define ID_DROP_GROUP_ADDTO             33020
#define ID_VIEW_SHOWANGLES              33021
#define ID_VIEW_SHOWWORKZONE            33022
#define ID_TEXTURE_FLUSH                33023
#define ID_TEXTURES_SHOWSHADERS         33025
#define ID_DONTSELECTMODEL              33027
#define ID_TEXTURES_SHADERLISTONLY      33030
#define ID_PLUGIN_START                 33800
#define ID_PLUGIN_END                   33999
#define ID_FILE_EXIT                    40002
#define ID_FILE_SAVEAS                  40004
#define ID_VIEW_CENTER                  40005
#define ID_VIEW_UPFLOOR                 40006
#define ID_VIEW_DOWNFLOOR               40007
#define ID_BRUSH_FLIPX                  40008
#define ID_BRUSH_FLIPY                  40009
#define ID_BRUSH_FLIPZ                  40010
#define ID_BRUSH_ROTATEX                40011
#define ID_BRUSH_ROTATEY                40012
#define ID_BRUSH_ROTATEZ                40013
#define ID_BSP_FULLVIS                  40016
#define ID_BSP_FASTVIS                  40017
#define ID_BSP_NOVIS                    40018
#define ID_BSP_RELIGHT                  40019
#define ID_BSP_ENTITIES                 40020
#define ID_FILE_POINTFILE               40021
#define ID_VIEW_100                     40022
#define ID_VIEW_75                      40023
#define ID_VIEW_50                      40024
#define ID_VIEW_25                      40025
#define ID_VIEW_12                      40026
#define ID_TEXTURES_SHOWALL             40033
#define ID_TEXTURES_SHOWINUSE           40034
#define ID_TEXTURES_TOGGLEVIEW          40037
#define ID_SELECTION_CREATEENTITY       40039
#define ID_SELECTION_EDITENTITY         40040
#define ID_MISC_BENCHMARK               40041
#define ID_REGION_OFF                   40043
#define ID_REGION_SETXY                 40044
#define ID_REGION_SETBRUSH              40045
#define ID_SELECTION_MAKEHOLLOW         40046
#define ID_SELECTION_MAKEHOLLOW_TOUCH   40051
#define ID_SELECTION_SELECTPARTIALTALL  40047
#define ID_SELECTION_SELECTCOMPLETETALL 40048
#define ID_SELECTION_CSGSUBTRACT        40049
#define ID_SELECTION_SELECTTOUCHING     40050
#define ID_VIEW_NEAREST                 40052
#define ID_VIEW_NEARESTMIPMAP           40053
#define ID_VIEW_LINEAR                  40054
#define ID_VIEW_BILINEAR                40055
#define ID_VIEW_BILINEARMIPMAP          40056
#define ID_VIEW_TRILINEAR               40057
#define ID_TEXTURES_WIREFRAME           40058
#define ID_TEXTURES_FLATSHADE           40059
#define ID_VIEW_SHOWNAMES               40060
#define ID_VIEW_ZOOMIN                  40061
#define ID_VIEW_ZOOMOUT                 40062
#define ID_VIEW_SHOWCOORDINATES         40063
#define ID_VIEW_Z100                    40064
#define ID_VIEW_ZZOOMIN                 40065
#define ID_VIEW_ZZOOMOUT                40066
#define ID_SELECTION_CLONE              40067
#define ID_SELECTION_DESELECT           40068
#define ID_SELECTION_DELETE             40069
#define ID_BUTTON40068                  40070
#define ID_SELECTION_DRAGVERTECIES      40074
#define ID_SELECTION_DRAGEDGES          40075
#define ID_REGION_SETTALLBRUSH          40076
#define ID_SELECTION_SELECTINSIDE       40092
#define ID_PROJECT_RELEAD               40094
#define ID_PROJECT_CHANGE               40095
#define ID_MISC_GAMMA                   40097
#define ID_MISC_TEXTUREBACKGROUN        40104
#define ID_TEXTUREBK                    40105
#define ID_COLORS_XYBK                  40106
#define ID_FILE_ABOUT                   40107
#define ID_VIEW_CONSOLE                 40108
#define ID_VIEW_ENTITY                  40109
#define ID_VIEW_TEXTURE                 40110
#define ID_COLORS_MAJOR                 40111
#define ID_COLORS_MINOR                 40113
#define ID_SELECTION_CONNECT            40114
#define ID_FILE_LOADPROJECT             40115
#define ID_MISC_FINDBRUSH               40116
#define ID_MISC_NEXTLEAKSPOT            40117
#define ID_MISC_PREVIOUSLEAKSPOT        40118
#define ID_BRUSH_3SIDED                 40119
#define ID_BRUSH_4SIDED                 40120
#define ID_BRUSH_5SIDED                 40121
#define ID_BRUSH_6SIDED                 40122
#define ID_BRUSH_7SIDED                 40123
#define ID_BRUSH_8SIDED                 40124
#define ID_BRUSH_9SIDED                 40125
#define ID_SELECTION_ARBITRARYROTATION  40126
#define ID_BRUSH_ARBITRARYSIDED         40127
#define ID_SELECTION_UNGROUPENTITY      40130
#define ID_MISC_SELECTENTITYCOLOR       40131
#define ID_MISC_PRINTXY                 40132
#define ID_HELP_ABOUT                   40134
#define ID_EDIT_COPYBRUSH               40135
#define ID_EDIT_PASTEBRUSH              40136
#define ID_TEXTURES_INSPECTOR           40137
#define ID_SELECTION_MAKE_DETAIL        40139
#define ID_SELECTION_MAKE_STRUCTURAL    40140
#define ID_REGION_SETSELECTION          40141
#define ID_VIEW_SHOWBLOCKS              40142
#define ID_CAMERA_UP                    40152
#define ID_CAMERA_DOWN                  40153
#define ID_CAMERA_LEFT                  40154
#define ID_CAMERA_RIGHT                 40155
#define ID_CAMERA_FORWARD               40156
#define ID_CAMERA_BACK                  40157
#define ID_CAMERA_ANGLEUP               40158
#define ID_CAMERA_ANGLEDOWN             40159
#define ID_CAMERA_STRAFELEFT            40160
#define ID_CAMERA_STRAFERIGHT           40161
#define ID_GRID_TOGGLE                  40162
#define ID_ENTITYLIST                   40163
#define ID_MAPINFO                      40164
#define ID_TOGGLECONSOLE                40165
#define ID_TOGGLECAMERA                 40166
#define ID_TOGGLEZ                      40167
#define ID_TOGGLEVIEW                   40168
#define ID_SELECTION_TEXTURE_FIT        40171
#define ID_SELECTION_TEXTURE_ROTATECLOCK 40172
#define ID_SELECTION_TEXTURE_ROTATECOUNTER 40173
#define ID_SELECTION_TEXTURE_SCALEUP    40174
#define ID_SELECTION_TEXTURE_SCALEDOWN  40175
#define ID_SELECTION_TEXTURE_SHIFTLEFT  40176
#define ID_SELECTION_TEXTURE_SHIFTRIGHT 40177
#define ID_SELECTION_TEXTURE_SHIFTUP    40178
#define ID_SELECTION_TEXTURE_SHIFTDOWN  40179
#define ID_GRID_NEXT                    40180
#define ID_GRID_PREV                    40181
#define ID_SELECTION_TEXTURE_SCALELEFT  40182
#define ID_SELECTION_TEXTURE_SCALERIGHT 40183
#define ID_SELECTION_PRINT              40184
#define ID_SELECTION_TOGGLESIZEPAINT    40185
#define ID_PATCH_TAB                    40186
#define ID_PATCH_ENTER                  40187
#define ID_SELECT_SNAPTOGRID            40188
#define ID_PATCH_INSPECTOR              40189
#define ID_SELECT_ALL                   40190
#define ID_CURVE_FREEZE                 40191
#define ID_CURVE_UNFREEZE               40192
#define ID_CURVE_UNFREEZEALL            40193
#define ID_SELECT_RESELECT              40194
#define ID_FITFACE                      40196
#define ID_VIEW_CROSSHAIR               40197
#define ID_SELECTION_INVERT             40198
#define ID_VIEW_GROUPS                  40199
#define ID_FILE_SLEEP                   40200
#define ID_HELP_LINKS                   40201
#define ID_VIEW_SHOWOUTLINE             40202 // TTimo: outline as in colored outline around the window to quickly guess the orientation
#define ID_VIEW_SHOWAXES                40203
#define ID_SELECTION_NOOUTLINE          40204 // TTimo: outline as in zbuffered outline toggle on camera view (TA Q3Radiant 200f addition)
#define ID_SELECTION_OUTLINESTYLE       40205 // Arnout: cycles through selection styles (extended 'nooutline')
#define ID_SELECTION_SEPERATE           40206 // TTimo: split brushes out of an entity back into worldspawn
#define ID_SELECTION_MERGE              40207 // TTimo: merge brushes from worldspawn into entity
#define ID_HELP_BUGREPORT               40208

#define ID_FILTER_WORLD                 40209
#define ID_FILTER_PATCHES               40210
#define ID_FILTER_DETAILS               40211
#define ID_FILTER_ENTITIES              40212
#define ID_FILTER_MODELS                40213
#define ID_FILTER_HINTSSKIPS            40214
#define ID_FILTER_CLIPS                 40215
#define ID_FILTER_LIQUIDS               40216
#define ID_FILTER_TRIGGERS              40217
#define ID_FILTER_AREAPORTALS           40218
#define ID_FILTER_TRANSLUCENT           40219
#define ID_FILTER_CAULK                 40220
#define ID_FILTER_LIGHTS                40221
#define ID_FILTER_PATHS                 40223
#define ID_FILTER_CLUSTERPORTALS        40224
#define ID_FILTER_LIGHTGRID             40225
#define ID_FILTER_STRUCTURAL            40226
#define ID_FILTER_BOTCLIPS                          40227

#define ID_CURVE_REDISPERSE_INTERMEDIATE_COLS   40230
#define ID_CURVE_REDISPERSE_INTERMEDIATE_ROWS   40231
#define ID_EDIT_PASTEBRUSHTOCAMERA      40232

#define ID_COLORS_MINOR_ALT             40230
#define ID_COLORS_MAJOR_ALT             40231

#define ID_SELECT_FUNC_GROUP            40233

// those must have their own ID chunk ID_GRID_025 <= ID_GRID <= ID_GRID_256
#define ID_GRID_025                     40300
#define ID_GRID_05                      40301
#define ID_GRID_1                       40302
#define ID_GRID_2                       40303
#define ID_GRID_4                       40304
#define ID_GRID_8                       40305
#define ID_GRID_16                      40306
#define ID_GRID_32                      40307
#define ID_GRID_64                      40308
#define ID_GRID_128                     40309
#define ID_GRID_256                     40310

#define ID_FILE_CHECKUPDATE             40320

#define ID_TEXTUREWINDOW_SCALEUP        40321
#define ID_TEXTUREWINDOW_SCALEDOWN      40322

#define ID_TOGGLE_DETAIL				40323

class CSynapseClientRadiant : public CSynapseClient
{
public:
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
const char* GetName();

void ImportMap( IDataStream *in, CPtrArray *ents, const char *type );
void ExportMap( CPtrArray *ents, IDataStream *out, const char *type );

CSynapseClientRadiant() { }
virtual ~CSynapseClientRadiant() { }
};

class MainFrame
{
public:
enum EViewStyle
{
	eRegular,
	eFloating,
	eSplit,
	eRegularLeft,
};

MainFrame();
GtkWidget *m_pWidget;

/*!
   called to fire up the help links
 */
void handle_help_command( int id );

protected:

/*!
   the urls to fire up in the game packs help menus
 */
vector<Str *> mHelpURLs;

/*!
   scan the .game files for game install packs
   look there for help description nodes
   build the corresponding menus in Radiant
 */
void create_game_help_menu( GtkWidget *menu, GtkAccelGroup *accel );

/*!
   build the menu once the filename is found
 */
void process_xlink( Str &FileName, const char *menu_name, const char *base_url, GtkWidget *menu, GtkAccelGroup *accel );

void Create();
void create_main_menu( GtkWidget *window, GtkWidget *vbox );
void create_main_toolbar( GtkWidget *window, GtkWidget *vbox );
void create_plugin_toolbar( GtkWidget *window, GtkWidget *vbox );
void create_main_statusbar( GtkWidget *window, GtkWidget *vbox );
GtkWidget *m_pStatusLabel[6];
GtkWidget *m_pSplits[5];
XYWnd* m_pXYWnd;
XYWnd* m_pYZWnd;
XYWnd* m_pXZWnd;
CamWnd* m_pCamWnd;
TexWnd* m_pTexWnd;
ZWnd* m_pZWnd;
CWatchBSP* m_pWatchBSP;

XYWnd* m_pActiveXY;
bool m_bCamPreview;
CPlugInManager m_PlugInMgr;
int m_nNextPlugInID;
guint m_nTimer;
bool m_bSleeping;

CString m_strStatus[15];
bool m_bNeedStatusUpdate;

/*!
   synapse server
   deals with dynamically loading the modules, initializing them, requesting the APIs
 */
CSynapseServer m_SynapseServer;
/*!
   we are also a synapse client in that we provide and require some APIs as well
 */
CSynapseClientRadiant m_SynapseClient;

public:

// BSP window
// trigger network listen
void DoWatchBSP();
bool IsSleeping()
{ return m_bSleeping; }

void UpdatePatchToolbarButtons();
// Gef: Changed to float for sub-integer grid size
void NudgeSelection( int nDirection, float nAmount );
void SetButtonMenuStates();
void SetGridStatus();
void RoutineProcessing();
XYWnd* ActiveXY() { return m_pActiveXY; };
void UpdateWindows( int nBits );
void SetStatusText( int nPane, const char* pText );
void UpdateStatusText();
void SetWindowStyle( int nStyle );
virtual ~MainFrame();
XYWnd* GetXYWnd() {return m_pXYWnd; }
XYWnd* GetXZWnd() {return m_pXZWnd; }
XYWnd* GetYZWnd() {return m_pYZWnd; }
ZWnd* GetZWnd() {return m_pZWnd; }
CamWnd* GetCamWnd() {return m_pCamWnd; }
TexWnd* GetTexWnd() {return m_pTexWnd; }
CWatchBSP *GetWatchBSP() { return m_pWatchBSP; }
void ReleaseContexts();
void CreateContexts();

void SetActiveXY( XYWnd* p ){
	if ( m_pActiveXY ) {
		m_pActiveXY->SetActive( false );
	}

	m_pActiveXY = p;

	if ( m_pActiveXY ) {
		m_pActiveXY->SetActive( true );
	}

};

EViewStyle CurrentStyle(){
	return m_nCurrentStyle;
};

bool FloatingGroupDialog(){
	return CurrentStyle() == eFloating || CurrentStyle() == eSplit;
};

#ifdef _WIN32
const GdkRectangle & GetPrimaryMonitorRect( void ) const { return primaryMonitorRect; }
const int GetGDKOffsetX( void ) const { return gdk_offset_x; }
const int GetGDKOffsetY( void ) const { return gdk_offset_y; }
#endif

protected:
bool m_bDoLoop;
bool m_bSplittersOK;
void CreateQEChildren();
void LoadCommandMap();
void ShowMenuItemKeyBindings( GtkWidget* window );

public:
void Copy();
void Paste();
void Nudge( int nDim, float fNudge );
CPlugInManager &GetPlugInMgr() {return m_PlugInMgr; };
CSynapseServer &GetSynapseServer() {return m_SynapseServer; };
CSynapseClientRadiant &GetSynapseClient() {return m_SynapseClient; };
void AddPlugInToolbarButton( const IToolbarButton* button );
void AddPlugInMenuItem( IPlugIn* pPlugIn );
void CleanPlugInMenu();

// these are public so i can easily reflect messages
// from child windows..
void OnTimer();
void OnDelete();
void OnDestroy();
void ToggleCamera();

void OnFileExit();
void OnFileLoadproject();
void OnFileNew();
void OnFileOpen();
void OnFilePointfile();
void OnFileSave();
void OnFileSaveas();
void OnFileCheckUpdate();
void OnView100();
void OnViewCenter();
void OnViewConsole();
void OnViewDownfloor();
void OnViewEntity();
void OnViewFront();
void OnViewShowblocks();
void OnViewShowclip();
void OnViewShowcoordinates();
void OnViewShowOutline();
void OnViewShowAxes();
void OnViewShowdetail();
void OnViewShowent();
void OnViewShowlights();
void OnViewShownames();
void OnViewShowpath();
void OnViewShowwater();
void OnViewShowworld();
void OnViewTexture();
void OnViewUpfloor();
void OnViewXy();
void OnViewZ100();
void OnViewZoomin();
void OnViewZoomout();
void OnViewZzoomin();
void OnViewZzoomout();
void OnViewSide();
void OnTexturesShowinuse();
void OnTexturesInspector();
void OnMiscBenchmark();
void OnMiscFindbrush();
void OnMiscGamma();
void OnMiscNextleakspot();
void OnMiscPreviousleakspot();
void OnMiscPrintxy();
void OnMiscSelectentitycolor();
void OnTexturebk();
void OnColorsMajor();
void OnColorsMinor();
void OnColorsMajor_Alt();
void OnColorsMinor_Alt();
void OnColorsXybk();
void OnBrush3sided();
void OnBrush4sided();
void OnBrush5sided();
void OnBrush6sided();
void OnBrush7sided();
void OnBrush8sided();
void OnBrush9sided();
void OnBrushArbitrarysided();
void OnBrushFlipx();
void OnBrushFlipy();
void OnBrushFlipz();
void OnBrushRotatex();
void OnBrushRotatey();
void OnBrushRotatez();
void OnRegionOff();
void OnRegionSetbrush();
void OnRegionSetselection();
void OnRegionSettallbrush();
void OnRegionSetxy();
void OnSelectionArbitraryrotation();
void OnSelectionClone();
void OnSelectionConnect();
void OnSelectionCsgsubtract();
void OnSelectionCsgmerge();
void OnSelectionNoOutline();
void OnSelectionOutlineStyle();
void OnSelectionDelete();
void OnSelectionDeselect();
void OnSelectionDragedges();
void OnSelectionDragvertecies();
void OnSelectionMakeDetail();
void OnSelectionMakeStructural();
void OnSelectionMakehollow();
void OnSelectionMakehollowTouch();
void OnSelectionSelectcompletetall();
void OnSelectionSelectinside();
void OnSelectionSelectpartialtall();
void OnSelectionSelecttouching();
void OnSelectionUngroupentity();
void OnSelectionMergeentity();
void OnSelectionGroupworld();
void OnTexturesPopup();
void OnPopupSelection();
void OnViewChange();
void OnViewCameraupdate();
void OnHelpAbout();
void OnHelp();
void OnHelpLinks();
void OnHelpBugreport();
void OnViewClipper();
void OnToggleDetail();
void OnCameraAngledown();
void OnCameraAngleup();
void OnCameraBack( bool keydown );
void OnCameraDown();
void OnCameraForward( bool keydown );
void OnCameraLeft( bool keydown );
void OnCameraRight( bool keydown );
void OnCameraStrafeleft( bool keydown );
void OnCameraStraferight( bool keydown );
void OnCameraUp();
void OnGridToggle();
void OnPrefs();
void OnTogglecamera();
void OnToggleconsole();
void OnToggleview();
void OnTogglez();
void OnToggleLock();
void OnEditMapinfo();
void OnEditEntityinfo();
void OnBrushScripts();
void OnViewCenterview();
void OnViewNextview();
void OnHelpCommandlist();
void OnFileNewproject();
void OnFlipClip();
void OnClipSelected();
void OnSplitSelected();
void OnToggleviewXz();
void OnToggleviewYz();
void OnColorsBrush();
void OnColorsClipper();
void OnColorsGridtext();
void OnColorsSelectedbrush();
void OnColorsSelectedbrush3D();
void OnColorsCameraBack();
void OnColorsGridblock();
void OnColorsViewname();
void OnColorsDetail();
void OnColorSetoriginal();
void OnColorSetqer();
void OnColorSetblack();
void OnColorSetydnar();    /* ydnar */
void OnSnaptogrid();
void OnSelectScale();
void OnSelectMouserotate();
void OnEditCopybrush();
void OnEditPastebrush();
void OnEditPastebrushToCamera();
void OnEditUndo();
void OnEditRedo();
void OnSelectionInvert();
//  void OnSelectionTextureDec();
void OnSelectionTextureFit();
//  void OnSelectionTextureInc();
void OnSelectionTextureRotateclock();
void OnSelectionTextureRotatecounter();
void OnSelectionTextureScaledown();
void OnSelectionTextureScaleup();
void OnSelectionTextureShiftdown();
void OnSelectionTextureShiftleft();
void OnSelectionTextureShiftright();
void OnSelectionTextureShiftup();
void OnGridNext();
void OnGridPrev();
void OnSelectionTextureScaleLeft();
void OnSelectionTextureScaleRight();
void OnTextureReplaceall();
void OnScalelockx();
void OnScalelocky();
void OnScalelockz();
void OnSelectMousescale();
void OnViewCubicclipping();
void OnFileProjectsettings();
void OnViewCubein();
void OnViewCubeout();
void OnFileSaveregion();
void OnSelectionMovedown();
void OnSelectionMoveup();
void OnToolbarMain();
void OnToolbarTexture();
void OnSelectionPrint();
void OnSelectionTogglesizepaint();
void OnBrushMakecone();
void OnTexturesLoad();
void OnToggleRotatelock();
void OnFileImportmap();
void OnFileExportmap();
void OnEditLoadprefab();
void OnSelectionSelectNudgedown();
void OnSelectionSelectNudgeleft();
void OnSelectionSelectNudgeright();
void OnSelectionSelectNudgeup();
void OnTexturesLoadlist();
void OnDontselectcurve();
void OnConvertcurves();
void OnCurveSimplepatchmesh();
void OnPatchToggleBox();
void OnPatchWireframe();
void OnCurvePatchcone();
void OnCurvePatchtube();
void OnPatchWeld();
void OnCurvePatchbevel();
void OnCurvePatchendcap();
void OnPatchDrilldown();
void OnCurveInsertcolumn();
void OnCurveInsertrow();
void OnCurveDeletecolumn();
void OnCurveDeleterow();
void OnCurveInsertAddcolumn();
void OnCurveInsertAddrow();
void OnCurveInsertInsertcolumn();
void OnCurveInsertInsertrow();
void OnCurveNegative();
void OnCurveNegativeTextureX();
void OnCurveNegativeTextureY();
void OnCurveDeleteFirstcolumn();
void OnCurveDeleteFirstrow();
void OnCurveDeleteLastcolumn();
void OnCurveDeleteLastrow();
void OnPatchBend();
//  void OnPatchInsdel();
void OnPatchEnter();
void OnPatchTab();
void OnCurvePatchdensetube();
void OnCurvePatchverydensetube();
void OnCurveCap();
void OnCurveCapInvertedbevel();
void OnCurveCapInvertedendcap();
void OnCurveRedisperseRows();
void OnCurveRedisperseIntermediateCols();
void OnCurveRedisperseIntermediateRows();
void OnPatchNaturalize();
void OnSnapToGrid();
void OnCurvePatchsquare();
void OnTexturewindowScaleup();
void OnTexturewindowScaledown();
void OnCurveOverlayClear();
void OnCurveOverlaySet();
void OnCurveThicken();
void OnCurveCyclecap();
void OnCurveMatrixTranspose();
void OnTexturesReloadshaders();
void OnShowEntities();
// will set the view mode right, don't set the value for mode if you only want to update the radio item
void OnEntitiesSetViewAs( int mode = 0 );
void OnPluginsRefresh();
void OnTexturesShowall();
void OnPatchInspector();
void OnViewOpengllighting();
void OnSelectAll();
void OnCurveFreeze();
void OnCurveUnFreeze();
void OnCurveUnFreezeAll();
void OnSelectReselect();
void OnEditSaveprefab();
void OnCurveMoreendcapsbevelsSquarebevel();
void OnCurveMoreendcapsbevelsSquareendcap();
void OnBrushPrimitivesSphere();
void OnViewCrosshair();
void OnViewHideshowHideselected();
void OnViewHideshowShowhidden();
void OnTexturesShadersShow();
void OnTexturesEmptyDirsHide();
void OnViewGroups();
void OnDropGroupAddtoWorld();
void OnDropGroupName();
void OnDropGroupNewgroup();
void OnDropGroupRemove();
void OnViewShowWorkzone();
void OnViewShowAngles();
void OnMru( unsigned int nID );
void OnViewNearest( unsigned int nID );
void OnTextureWad( unsigned int nID );
void OnBspCommand( unsigned int nID );
void OnGrid( unsigned int nID );
void OnPlugIn( unsigned int nID, const char *str );
void OnFaceFit();
void SetTextureScale( int id );
void OnDontselectmodel();
void OnTexturesShaderlistonly();
void OnSleep();
void OnFilterAreaportals();
void OnFilterCaulk();
void OnFilterStructural();
void OnFilterClips();
void OnFilterBotClips();
void OnFilterDetails();
void OnFilterEntities();
void OnFilterHintsskips();
void OnFilterLights();
void OnFilterLiquids();
void OnFilterModels();
void OnFilterPatches();
void OnFilterTranslucent();
void OnFilterTriggers();
void OnFilterWorld();
void OnFilterPaths();
void OnFilterClusterportals();
void OnFilterLightgrid();
void OnSelectFuncGroup();

private:
EViewStyle m_nCurrentStyle;

#ifdef _WIN32
GdkRectangle primaryMonitorRect;
int gdk_offset_x;
int gdk_offset_y;
#endif

};

// some C API to the mainframe functions
void WINAPI QERApp_Sleep();

// Checks whether a given filename ends in .map
const bool IsMap(const char* filename);

#endif // _MAINFRAME_H_
