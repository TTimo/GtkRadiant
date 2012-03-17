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

/*
   Camera plugin for GtkRadiant
   Copyright (C) 2002 Splash Damage Ltd.
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#ifdef _WIN32
	#pragma warning(disable : 4267)
#else
typedef unsigned char byte;
#endif

class CCamera;

#include <gtk/gtk.h>

#include "str.h"

#define USE_QERTABLE_DEFINE
#include "qerplugin.h"

#include "igl.h"
#include "iui.h"
#include "icamera.h"

#include "misc.h"
#include "dialogs.h"
#include "funchandlers.h"
#include "renderer.h"
#include "listener.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERQglTable g_QglTable;
extern _QERUITable g_UITable;
extern _QERCameraTable g_CameraTable;

extern CRenderer          *Renderer;
extern CListener          *Listener;

// splinelib
#define CAMERA_PLUGIN
#define DotProduct( a,b )         ( ( a )[0] * ( b )[0] + ( a )[1] * ( b )[1] + ( a )[2] * ( b )[2] )

extern void ( APIENTRY * qglBegin )( GLenum mode );
extern void ( APIENTRY * qglEnd )( void );
extern void ( APIENTRY * qglVertex3fv )( const GLfloat *v );

extern "C" void InitIglToQgl( _QERQglTable *g_QglTable );

#include "splines/splines.h"

// this needs to match splines.cpp
#define MAX_CAMERAS 64
extern idCameraDef camera[MAX_CAMERAS];

extern "C" qboolean loadCamera( int camNum, const char *name );

//
// CCamera
//

class CCamera {
public:
CCamera( int i ) {
	cam = &camera[i];
	camnum = i;
	Init();
}
~CCamera();

void Init() {
	next = prev = NULL;
	fileName[0] = '\0';
	hasbeensaved = 0;
}

idCameraDef *GetCam() {
	return( cam );
}
int GetCamNum() {
	return( camnum );
}

char *GetFileName() {
	return( fileName );
}
void SetFileName( const char *name, bool save ) {
	strcpy( fileName, name );
	if ( save ) {
		hasbeensaved = 1;
	}
}

CCamera *GetNext() {
	return( next );
}

CCamera *GetPrev() {
	return( prev );
}

void SetNext( CCamera *camera ) {
	next = camera;
}
void SetPrev( CCamera *camera ) {
	prev = camera;
}

int HasBeenSaved() {
	return( hasbeensaved );
}
void HasBeenModified() {
	if ( hasbeensaved ) {
		hasbeensaved = 2;
	}
}

protected:
idCameraDef *cam;
int camnum;
CCamera *next, *prev;
char fileName[PATH_MAX];
int hasbeensaved;       // 0:never saved 1:saved 2:saved, but modified
};

CCamera *AllocCam();
void FreeCam( CCamera *cam );
void SetCurrentCam( CCamera *cam );
CCamera *GetCurrentCam();

// globals
extern GtkWidget *g_pRadiantWnd;
extern GtkWidget *g_pCameraInspectorWnd;
extern CCamera *firstCam;
extern bool g_bEditOn;
extern int g_iEditMode;
extern int g_iActiveTarget;
extern int g_iPreviewRunning;
extern CCamera *g_pCurrentEditCam;

#endif // _CAMERA_H_
