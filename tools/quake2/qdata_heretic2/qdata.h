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

// qdata.h


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "cmdlib.h"
#include "inout.h"
#include "scriplib.h"
#include "mathlib.h"
#include "trilib.h"
#include "lbmlib.h"
#include "her2_threads.h"
#include "l3dslib.h"
#include "bspfile.h"

#ifndef _WIN32
#define stricmp strcasecmp
#define strcmpi strcasecmp
#endif


#define MODEL_AUTO  0
#define MODEL_MD2   1
#define MODEL_FM    2

// Model cover functions (to allow the forcing of a model type)
void MODELCMD_Modelname( int modeltype );
void MODELCMD_Cd( int modeltype );
void MODELCMD_Origin( int modeltype );
void MODELCMD_Jointed( int modeltype );
void MODELCMD_Cluster( int modeltype );
void MODELCMD_Base( int modeltype );
void MODELCMD_BaseST( int modeltype );
void MODELCMD_ScaleUp( int modeltype );
void MODELCMD_Frame( int modeltype );
void MODELCMD_Skin( int modeltype );
void MODELCMD_Skinsize( int modeltype );
void MODELCMD_Skeleton( int modeltype );
void MODELCMD_SkeletalFrame( int modeltype );
void MODELCMD_BeginGroup( int modeltype );
void MODELCMD_EndGroup( int modeltype );
void MODELCMD_Referenced( int modeltype );
void MODELCMD_NodeOrder( int modeltype );

void Cmd_Modelname( void );
void Cmd_Base( void );
void Cmd_Cd( void );
void Cmd_Origin( void );
void Cmd_ScaleUp( void );
void Cmd_Frame( void );
void Cmd_Skin( void );
void Cmd_Skinsize( void );
void FinishModel( void );
void Cmd_Cluster( void );

// Flexible Models
//void Cmd_FMModelname (void);
void Cmd_FMBase( qboolean GetST );
void Cmd_FMCd( void );
//void Cmd_FMOrigin (void);
void Cmd_FMCluster();
void Cmd_FMSkeleton();
//void Cmd_FMScaleUp (void);
void Cmd_FMFrame( void );
void Cmd_FMSkeletalFrame();
void Cmd_FMSkin( void );
//void Cmd_FMSkinsize (void);
void Cmd_FMBeginGroup( void );
void Cmd_FMEndGroup( void );
void Cmd_FMReferenced();
void Cmd_FMNodeOrder( void );
void FMFinishModel( void );
void GenSkin( char *ModelFile, char *OutputName, int Width, int Height );
void NewGen( char *ModelFile, char *OutputName, int width, int height );


void Cmd_Inverse16Table( void );

void Cmd_SpriteName( void );
void Cmd_Load( void );
void Cmd_SpriteFrame( void );
void Cmd_Sprdir( void );
void FinishSprite( void );

void Cmd_Grab( void );
void Cmd_Raw( void );
void Cmd_Mip( void );
void Cmd_Environment( void );
void Cmd_Colormap( void );

void Cmd_File( void );
void Cmd_Dir( void );
void Cmd_StartWad( void );
void Cmd_EndWad( void );
void Cmd_Mippal( void );
void Cmd_Mipdir( void );
void Cmd_Alphalight( void );

void Cmd_Picdir( void );
void Cmd_Pic( void );

void Cmd_Bookdir( void );
void Cmd_Book( void );

void Cmd_TextureMix( void );

void Cmd_Video( void );

//void RemapZero (byte *pixels, byte *palette, int width, int height);

void ReleaseFile( char *filename );

extern byte        *byteimage, *lbmpalette;
extern int byteimagewidth, byteimageheight;
extern qboolean TrueColorImage;
extern unsigned    *longimage;
extern int longimagewidth, longimageheight;

extern qboolean g_release;              // don't grab, copy output data to new tree
extern char g_releasedir[1024];         // c:\quake2\baseq2, etc
extern qboolean g_archive;              // don't grab, copy source data to new tree
extern qboolean do3ds;
extern char g_only[256];                // if set, only grab this cd
extern qboolean g_skipmodel;            // set true when a cd is not g_only
extern qboolean g_no_opimizations;
extern int g_forcemodel;
extern qboolean g_verbose;
extern qboolean g_allow_newskin;
extern qboolean g_ignoreTriUV;      //from qdata.c
extern qboolean g_dokeypress;

extern char        *trifileext;

extern char g_materialFile[256];

extern unsigned total_x;
extern unsigned total_y;
extern unsigned total_textures;

miptex_t *CreateMip( byte *data, unsigned width, unsigned height, byte *palette, int *FinalSize, qboolean mip );
miptex32_t *CreateMip32( unsigned *data, unsigned width, unsigned height, int *FinalSize, qboolean mip );
