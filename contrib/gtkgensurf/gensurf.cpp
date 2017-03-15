/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
/*
   #include <string.h>
   #include <tchar.h>
   #include <math.h>
 */
#include "gensurf.h"

char gszAppDir[NAME_MAX];
char gszCaption[64];
char gszIni[NAME_MAX];
char gszHelpFile[NAME_MAX];
char gszMapFile[NAME_MAX];
char gszVersion[64];
double Amplitude;
double Roughness;
double TexOffset[2];
double TexScale[2];
double WaveLength;
double Hll, Hur, Vll, Vur;
double Z00, Z01, Z10, Z11;
ELEMENT Vertex[( MAX_ROWS + 1 ) * ( MAX_ROWS + 1 )];
int AddHints;
int ArghRad2;
int AutoOverwrite;
int Decimate = 0;
int SnapToGrid = 0;           // 0, or the grid size to snap to. // Hydra : snap to grid
int FileAppend = 0;
int FixBorders;
int HideBackFaces = 0;
int NH, NV;
int NumVerticesSelected;
int Plane;
int Preview;
int RandomSeed = 1;
int Skybox;
int UseDetail;
int UseLadder;
int VertexMode = 0;
int WaveType;
int gNumNodes = 0;
int gNumTris = 0;
int vid_x, vid_y;
int view_x, view_y;
int view_cx, view_cy;
int UsePatches;
int SlantAngle;
int GimpHints;
int Antialiasing;             // ^Fishman - Antializing for the preview window.
int AddTerrainKey;             // ^Fishman - Add terrain key to func_group.
int SP;             // ^Fishman - Snap to grid.

GtkWidget *g_pWnd;        // ghwnd;
GtkWidget *g_pRadiantWnd; // ghwnd_main;
/*HWND      ghwndAngles;
 */GtkWidget *g_pWndPreview;
GtkWidget *g_pPreviewWidget;
MYBITMAP gbmp;
NODE      *gNode = (NODE *)NULL;
TRI       *gTri = (TRI *)NULL;

int Game;
bounding_box PlayerBox[NUMGAMES] = { {{-16., 16.}, {-16., 16.}, {-24., 32.}},    // Quake2
									 {{-16., 16.}, {-16., 16.}, {-36., 36.}},    // Half-Life
									 {{-16., 16.}, {-16., 16.}, {-32., 32.}},    // SiN
									 {{-16., 16.}, {-16., 16.}, {-24., 32.}},    // Heretic2 (guess)
									 {{-16., 16.}, {-16., 16.}, {-24., 32.}},    // KingPin (guess)
									 {{-30., 30.}, {-30., 30.}, {-10.,160.}},    // Genesis3D (no idea)
									 {{-16., 16.}, {-16., 16.}, {-24., 32.}}};   // Quake3 (not sure)
//char      gszOutputDir[NUMGAMES][NAME_MAX];
//char      gszTextureDir[NUMGAMES][NAME_MAX];
char Texture[NUMGAMES][3][64];
//char      pakfile[NUMGAMES][NAME_MAX];
//char      lastpakfile[NUMGAMES][NAME_MAX];
//int       UsePak[NUMGAMES];
//char      GameDir[NUMGAMES][NAME_MAX];

char GameName[NUMGAMES][16] = {"Quake2", "Half-Life", "SiN", "Heretic2", "Kingpin", "Genesis3D", "Quake3" };


bool GenSurfInit(){
	strcpy( gszVersion, "1.05" );
	strcpy( gszCaption, "GtkGenSurf" );
	if ( strlen( gszVersion ) ) {
		strcat( gszCaption, " v" );
		strcat( gszCaption, gszVersion );
	}

	strcpy( gszIni, g_FuncTable.m_pfnProfileGetDirectory() );
	strcat( gszIni, "gensurf.ini" );

/*if (g_FuncTable.m_pfnReadProjectKey != NULL)
   {
    char *basepath;

    basepath = g_FuncTable.m_pfnReadProjectKey("basepath");
    if (basepath)
    {
      strlwr (basepath);
      if (strstr(basepath,"baseq3"))
        Game = QUAKE3;
      else if (strstr (basepath,"baseq2"))
        Game = QUAKE2;
      else // Gotta have a game, might as well be Quake3
        Game = QUAKE3;
    }
    else
      Game = QUAKE3;
   }
   else */
	Game = QUAKE3;


	if ( g_pWnd == NULL ) {
		g_pWnd = create_main_dialog();
	}

	ReadIniFile( gszIni );

	return true;
}

// Reads default values

#define OPTS_SECTION "Options"

void ReadIniFile( const char *file ){
	char *Text;
	float x1,x2,x3,x4;
	int i;

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "Amplitude", "" );
	if ( strlen( Text ) ) {
		Amplitude = atof( Text );
	}
	else{
		Amplitude = 128;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "Roughness", "" );
	if ( strlen( Text ) ) {
		Roughness = atof( Text );
	}
	else{
		Roughness = 16;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "WaveLength", "" );
	if ( strlen( Text ) ) {
		WaveLength = atof( Text );
	}
	else{
		WaveLength = 1024;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "Extents", "" );
	if ( strlen( Text ) ) {
		sscanf( Text,"%f,%f,%f,%f",&x1,&x2,&x3,&x4 );
		Hll = x1;
		Vll = x2;
		Hur = x3;
		Vur = x4;
	}
	else
	{
		Hll = -512;
		Vll = -512;
		Hur =  512;
		Vur =  512;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "CornerValues", "" );
	if ( strlen( Text ) ) {
		sscanf( Text,"%f,%f,%f,%f",&x1,&x2,&x3,&x4 );
		Z00 = x1;
		Z01 = x2;
		Z10 = x3;
		Z11 = x4;
	}
	else
	{
		Z00 = 0.;
		Z01 = 0.;
		Z10 = 0.;
		Z11 = 0.;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION, "TextureOffset", "" );
	if ( strlen( Text ) ) {
		sscanf( Text,"%f,%f",&x1,&x2 );
		TexOffset[0] = x1;
		TexOffset[1] = x2;
	}
	else
	{
		TexOffset[0] = 0.;
		TexOffset[1] = 0.;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, OPTS_SECTION,"TextureScale","" );
	if ( strlen( Text ) ) {
		sscanf( Text,"%f,%f",&x1,&x2 );
		TexScale[0] = x1;
		TexScale[1] = x2;
		if ( TexScale[0] == 0. ) {
			TexScale[0] = 1.0;
		}
		if ( TexScale[1] == 0. ) {
			TexScale[1] = 1.0;
		}
	}
	else
	{
		TexScale[0] = 1.;
		TexScale[1] = 1.;
	}

	NH = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"NH",8 );
	NH = max( 1,min( NH,MAX_ROWS ) );
	NV = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"NV",8 );
	NV = max( 1,min( NV,MAX_ROWS ) );

//	Decimate   = GetPrivateProfileInt(OPTS_SECTION,"Decimate",0,file);
//	Decimate = max(0,min(Decimate,100));

	AddHints          = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"AddHints",0 );
	ArghRad2          = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"ArghRad2",0 );
	AutoOverwrite = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"AutoOverwrite",0 );
	FixBorders        = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"FixBorders",1 );
	HideBackFaces = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"HideBackFaces",0 );
	Plane                 = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"Plane",0 );
	Preview               = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"Preview", 0 );
	Antialiasing    = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"Antialiasing",0 ); // ^Fishman - Antializing for the preview window.
	RandomSeed        = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"RandomSeed",1 );
	Skybox                = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"Skybox",0 );
	UseDetail         = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"UseDetail",0 );
	AddTerrainKey   =   g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"AddTerrainKey",0 ); // ^Fishman - Add terrain key to func_group.
	UseLadder         = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"UseLadder",0 );
	WaveType          = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"WaveType",0 );
	vid_x                 = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"vid_x", 0 );
	vid_y                 = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"vid_y", 0 );
	view_x                = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"view_x",0 );
	view_y                = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"view_y",0 );
	view_cx               = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"view_cx",0 );
	view_cy               = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"view_cy",0 );

	UsePatches        = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"UsePatches",0 );

	SlantAngle = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"SlantAngle",60 );
	GimpHints  = g_FuncTable.m_pfnProfileLoadInt( file, OPTS_SECTION,"GimpHints",0 );

	for ( i = 0; i < NUMGAMES; i++ )
	{
		//    strcpy (gszOutputDir[i], g_FuncTable.m_pfnProfileLoadString (file, GameName[i],"OutputDir",""));
		strcpy( Texture[i][0], g_FuncTable.m_pfnProfileLoadString( file, GameName[i], "Texture", "" ) );
		strcpy( Texture[i][1], g_FuncTable.m_pfnProfileLoadString( file, GameName[i], "Texture2", "" ) );
		strcpy( Texture[i][2], g_FuncTable.m_pfnProfileLoadString( file, GameName[i], "Texture3", "" ) );
		//    strcpy (gszTextureDir[i], g_FuncTable.m_pfnProfileLoadString (file, GameName[i],"TextureDir",""));
		//    UsePak[i] = GetPrivateProfileInt(GameName[i],"UsePak",0);
		//    strcpy (pakfile[i], g_FuncTable.m_pfnProfileLoadString (file, GameName[i],"PakFile",""));
		//    strcpy (lastpakfile[i], g_FuncTable.m_pfnProfileLoadString (file, GameName[i],"LastPakFile",""));
		//    strcpy (GameDir[i], g_FuncTable.m_pfnProfileLoadString (file, GameName[i],"GameDir","\0"));
	}
	/*
	   if(!strlen(gszTextureDir[QUAKE2]))
	      strcpy(gszTextureDir[QUAKE2],"c:\\quake2\\baseq2\\textures\\");
	   if(!strlen(gszTextureDir[KINGPIN]))
	      strcpy(gszTextureDir[KINGPIN],"c:\\kingpin\\main\\textures\\");
	 */
	if ( !strlen( Texture[QUAKE2][0] ) ) {
		strcpy( Texture[QUAKE2][0],   "textures/e1u1/grass1_4" );
	}
	if ( !strlen( Texture[HALFLIFE][0] ) ) {
		strcpy( Texture[HALFLIFE][0], "textures/OUT_GRND1" );
	}
	if ( !strlen( Texture[SIN][0] ) ) {
		strcpy( Texture[SIN][0],      "textures/generic/floor_organic/fl_grass" );
	}
	if ( !strlen( Texture[HERETIC2][0] ) ) {
		strcpy( Texture[HERETIC2][0], "textures/canyon/canyon05" );
	}
	if ( !strlen( Texture[KINGPIN][0] ) ) {
		strcpy( Texture[KINGPIN][0],  "textures/bricks/s_sr_m3" );
	}
	if ( !strlen( Texture[GENESIS3D][0] ) ) {
		strcpy( Texture[GENESIS3D][0],"textures/rock13" );
	}
	if ( !strlen( Texture[QUAKE3][0] ) ) {
		strcpy( Texture[QUAKE3][0],   "textures/organics/grass3" );
	}
	if ( !strlen( Texture[QUAKE3][1] ) ) {
		strcpy( Texture[QUAKE3][1],   "textures/common/caulk" );
	}
	strcpy( gbmp.name, g_FuncTable.m_pfnProfileLoadString( file, "Bitmap","Filename","" ) );


	gbmp.colors = NULL;
	if ( strlen( gbmp.name ) ) {
		OpenBitmap();
	}

	strcpy( gbmp.defpath, g_FuncTable.m_pfnProfileLoadString( file, "Bitmap","DefaultPath","" ) );

	Text = g_FuncTable.m_pfnProfileLoadString( file, "Bitmap","BlackValue","" );
	if ( strlen( Text ) ) {
		gbmp.black_value = atof( Text );
	}
	else{
		gbmp.black_value = 0;
	}

	Text = g_FuncTable.m_pfnProfileLoadString( file, "Bitmap","WhiteValue","" );
	if ( strlen( Text ) ) {
		gbmp.white_value = atof( Text );
	}
	else{
		gbmp.white_value = 256.;
	}
}

/*
   ============
   va

   does a varargs printf into a temp buffer, so I don't need to have
   varargs versions of all text functions.
   FIXME: make this buffer size safe someday
   ============
 */
char *va( const char *format, ... ){
	va_list argptr;
	static char string[1024];

	va_start( argptr, format );
	vsprintf( string, format,argptr );
	va_end( argptr );

	return string;
}


// Writes current values to INI file
void WriteIniFile( const char *file ){
	int i;

	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "Amplitude",    va( "%g",Amplitude ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "Roughness",    va( "%g",Roughness ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "WaveLength",   va( "%g",WaveLength ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "Extents",      va( "%g,%g,%g,%g",Hll,Vll,Hur,Vur ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "CornerValues", va( "%g,%g,%g,%g",Z00,Z01,Z10,Z11 ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "TextureOffset",va( "%g,%g",TexOffset[0],TexOffset[1] ) );
	g_FuncTable.m_pfnProfileSaveString( file, OPTS_SECTION, "TextureScale", va( "%g,%g",TexScale[0],TexScale[1] ) );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "NH", NH );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "NV", NV );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "AddHints", AddHints );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "ArghRad2", ArghRad2 );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "AutoOverwrite", AutoOverwrite );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "FixBorders", FixBorders );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "Plane", Plane );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "Preview", Preview );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "Antialiasing", Antialiasing ); // ^Fishman - Antializing for the preview window.
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "RandomSeed", RandomSeed );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "Skybox", Skybox );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "UseDetail", UseDetail );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "AddTerrainKey", AddTerrainKey ); // ^Fishman - Add terrain key to func_group.
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "UseLadder", UseLadder );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "WaveType", WaveType );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "vid_x", vid_x );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "vid_y", vid_y );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "view_x", view_x );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "view_y", view_y );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "view_cx", view_cx );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "view_cy", view_cy );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "UsePatches", UsePatches );
	g_FuncTable.m_pfnProfileSaveInt( file, OPTS_SECTION, "SlantAngle", SlantAngle );
	for ( i = 0; i < NUMGAMES; i++ )
	{
		g_FuncTable.m_pfnProfileSaveString( file, GameName[i], "Texture",   Texture[i][0] );
		g_FuncTable.m_pfnProfileSaveString( file, GameName[i], "Texture2",  Texture[i][1] );
		g_FuncTable.m_pfnProfileSaveString( file, GameName[i], "Texture3",  Texture[i][2] );
	}

	g_FuncTable.m_pfnProfileSaveString( file, "Bitmap", "Filename", gbmp.name );
	g_FuncTable.m_pfnProfileSaveString( file, "Bitmap", "DefaultPath", gbmp.defpath );
	g_FuncTable.m_pfnProfileSaveString( file, "Bitmap", "BlackValue", va( "%g",gbmp.black_value ) );
	g_FuncTable.m_pfnProfileSaveString( file, "Bitmap", "WhiteValue", va( "%g",gbmp.white_value ) );
//g_FuncTable.m_pfnProfileSaveString (file, "Formula", "Formula", ExcelFunc );
}

void UpdatePreview( bool DataChange ){
	if ( g_pWndPreview && gtk_widget_get_visible( g_pWndPreview ) ) {
		if ( DataChange ) {
			GenerateXYZ();
		}

		gtk_widget_draw( g_pPreviewWidget, NULL );
	}
}

void SaveSetup( GtkWidget *parent ){
	const char *name = g_FuncTable.m_pfnFileDialog( parent, false, "Save GenSurf Settings",
													g_FuncTable.m_pfnProfileGetDirectory(), "gtkgensurf", NULL );

	if ( name != NULL ) {
		char key[32], text[32];
		int i, j;

		WriteIniFile( name );
		g_FuncTable.m_pfnProfileSaveString( name, OPTS_SECTION,"MapFile",gszMapFile );
		sprintf( text,"0x%04x",FileAppend );
		g_FuncTable.m_pfnProfileSaveString( name, OPTS_SECTION,"Append",text );
		sprintf( text,"0x%04x",Decimate );
		g_FuncTable.m_pfnProfileSaveString( name, OPTS_SECTION,"Decimate",text );
		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				if ( xyz[i][j].fixed ) {
					sprintf( key,"I%dJ%d",i,j );
					sprintf( text,"%g %g %g", xyz[i][j].fixed_value, xyz[i][j].range, xyz[i][j].rate );
					g_FuncTable.m_pfnProfileSaveString( name, "FixedPoints",key,text );
				}
			}
		}
	}
}

void OpenSetup( GtkWidget *parent, int UseDefaults ){
	const char *name;
	char key[32], *text;
	float value,range,rate;
	int i, j;

	if ( UseDefaults ) {
		name = g_strdup( "plugins/defaults.srf" ); // dummy string
	}
	else{
		name = g_FuncTable.m_pfnFileDialog( parent, true, "Open GenSurf Settings",
											g_FuncTable.m_pfnProfileGetDirectory(), "gtkgensurf", NULL );
	}

	if ( name != NULL ) {
		ReadIniFile( name );
		Decimate   = g_FuncTable.m_pfnProfileLoadInt( name, OPTS_SECTION,"Decimate",0 );
		Decimate   = max( 0,min( Decimate,100 ) );

		for ( i = 0; i <= NH; i++ )
		{
			for ( j = 0; j <= NV; j++ )
			{
				sprintf( key,"I%dJ%d",i,j );
				text = g_FuncTable.m_pfnProfileLoadString( name, "FixedPoints", key, "" );
				if ( strlen( text ) ) {
					xyz[i][j].fixed = 1;
					xyz[i][j].rate        = 0.;
					sscanf( text,"%g %g %g",&value,&range,&rate );
					xyz[i][j].fixed_value = value;
					xyz[i][j].range       = range;
					xyz[i][j].rate        = rate;
				}
				else{
					xyz[i][j].fixed = 0;
				}
			}
		}
	}
}
