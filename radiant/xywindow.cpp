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
// XY Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <assert.h>
#include <GL/gl.h>

// =============================================================================
// variables

#define PAGEFLIPS   2

CString g_strStatus;
bool g_bCrossHairs = false;
bool g_bScaleMode;
int g_nScaleHow;
bool g_bRotateMode;
bool g_bClipMode;
bool g_bRogueClipMode;
bool g_bSwitch;
ClipPoint g_Clip1;
ClipPoint g_Clip2;
ClipPoint g_Clip3;
ClipPoint* g_pMovingClip;
brush_t g_brFrontSplits;
brush_t g_brBackSplits;

brush_t g_brClipboard;
brush_t g_brUndo;
entity_t g_enClipboard;

vec3_t g_vRotateOrigin;
vec3_t g_vRotation;

bool g_bPathMode;
ClipPoint g_PathPoints[256]; // this limit isn't enforced?
ClipPoint* g_pMovingPath;
int g_nPathCount;
int g_nPathLimit;

bool g_bSmartGo;

bool g_bPointMode;
ClipPoint g_PointPoints[512];
ClipPoint* g_pMovingPoint;
int g_nPointCount;
int g_nPointLimit;

const int XY_LEFT = 0x01;
const int XY_RIGHT = 0x02;
const int XY_UP = 0x04;
const int XY_DOWN = 0x08;

PFNPathCallback* g_pPathFunc = NULL;

static unsigned s_stipple[32] =
{
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
};

void AcquirePath( int nCount, PFNPathCallback* pFunc ){
	g_nPathCount = 0;
	g_nPathLimit = nCount;
	g_pPathFunc = pFunc;
	g_bPathMode = true;
}


CPtrArray g_ptrMenus;

MemStream g_Clipboard( 4096 );
MemStream g_PatchClipboard( 4096 );

extern int pressx;
extern int pressy;
extern bool g_bWaitCursor;

vec3_t tdp;

GtkWidget* XYWnd::m_mnuDrop = NULL;

extern int g_nPatchClickedView;

// =============================================================================
// global functions


void WXY_Print(){
	const long width = g_pParentWnd->ActiveXY()->Width();
	const long height = g_pParentWnd->ActiveXY()->Height();
	unsigned char* img;
	const char* filename;

	filename = file_dialog( g_pParentWnd->m_pWidget, FALSE, _( "Save BMP Image" ), NULL, "bmp" );
	if ( !filename ) {
		return;
	}

	g_pParentWnd->ActiveXY()->MakeCurrent();
	img = (unsigned char*)malloc( width * height * 3 );
	qglReadPixels( 0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,img );

	FILE *fp;
	fp = fopen( filename, "wb" );
	if ( fp ) {
		unsigned short bits;
		unsigned long cmap, bfSize;

		bits = 24;
		cmap = 0;
		bfSize = 54 + width * height * 3;

		long byteswritten = 0;
		long pixoff = 54 + cmap * 4;
		short res = 0;
		char m1 = 'B', m2 = 'M';
		fwrite( &m1, 1, 1, fp );      byteswritten++; // B
		fwrite( &m2, 1, 1, fp );      byteswritten++; // M
		fwrite( &bfSize, 4, 1, fp );  byteswritten += 4; // bfSize
		fwrite( &res, 2, 1, fp );     byteswritten += 2; // bfReserved1
		fwrite( &res, 2, 1, fp );     byteswritten += 2; // bfReserved2
		fwrite( &pixoff, 4, 1, fp );  byteswritten += 4; // bfOffBits

		unsigned long biSize = 40, compress = 0, size = 0;
		long pixels = 0;
		unsigned short planes = 1;
		fwrite( &biSize, 4, 1, fp );  byteswritten += 4; // biSize
		fwrite( &width, 4, 1, fp );   byteswritten += 4; // biWidth
		fwrite( &height, 4, 1, fp );  byteswritten += 4; // biHeight
		fwrite( &planes, 2, 1, fp );  byteswritten += 2; // biPlanes
		fwrite( &bits, 2, 1, fp );    byteswritten += 2; // biBitCount
		fwrite( &compress, 4, 1, fp ); byteswritten += 4; // biCompression
		fwrite( &size, 4, 1, fp );    byteswritten += 4; // biSizeImage
		fwrite( &pixels, 4, 1, fp );  byteswritten += 4; // biXPelsPerMeter
		fwrite( &pixels, 4, 1, fp );  byteswritten += 4; // biYPelsPerMeter
		fwrite( &cmap, 4, 1, fp );    byteswritten += 4; // biClrUsed
		fwrite( &cmap, 4, 1, fp );    byteswritten += 4; // biClrImportant

		unsigned long widthDW = ( ( ( width * 24 ) + 31 ) / 32 * 4 );
		long row, row_size = width * 3;
		for ( row = 0; row < height; row++ )
		{
			unsigned char* buf = img + row * row_size;

			// write a row
			int col;
			for ( col = 0; col < row_size; col += 3 )
			{
				putc( buf[col + 2], fp );
				putc( buf[col + 1], fp );
				putc( buf[col], fp );
			}
			byteswritten += row_size;

			unsigned long count;
			for ( count = row_size; count < widthDW; count++ )
			{
				putc( 0, fp ); // dummy
				byteswritten++;
			}
		}

		fclose( fp );
	}

	free( img );
}

float ptSum( vec3_t pt ){
	return pt[0] + pt[1] + pt[2];
}

float Betwixt( float f1, float f2 ){
	if ( f1 > f2 ) {
		return f2 + ( ( f1 - f2 ) / 2 );
	}
	else{
		return f1 + ( ( f2 - f1 ) / 2 );
	}
}

void CleanList( brush_t* pList ){
	brush_t* pBrush = pList->next;
	while ( pBrush != NULL && pBrush != pList )
	{
		brush_t* pNext = pBrush->next;
		Brush_Free( pBrush );
		pBrush = pNext;
	}
}

void Brush_CopyList( brush_t* pFrom, brush_t* pTo ){
	brush_t* pBrush = pFrom->next;
	while ( pBrush != NULL && pBrush != pFrom )
	{
		brush_t* pNext = pBrush->next;
		Brush_RemoveFromList( pBrush );
		Brush_AddToList( pBrush, pTo );
		pBrush = pNext;
	}
}

float fDiff( float f1, float f2 ){
	if ( f1 > f2 ) {
		return f1 - f2;
	}
	else{
		return f2 - f1;
	}
}

/*
   =============================================================

   PATH LINES

   =============================================================
 */

/*
   ==================
   DrawPathLines

   Draws connections between entities.
   Needs to consider all entities, not just ones on screen,
   because the lines can be visible when neither end is.
   Called for both camera view and xy view.
   ==================
 */
void DrawPathLines( void ){
	int i, j, k;
	vec3_t mid, mid1;
	entity_t *se, *te;
	brush_t   *sb, *tb;
	const char    *psz;
	vec3_t dir, s1, s2;
	vec_t len, f;
	int arrows;
	int num_entities;
	const char        *ent_target[MAX_MAP_ENTITIES];
	entity_t  *ent_entity[MAX_MAP_ENTITIES];

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS ) {
		return;
	}

	num_entities = 0;
	for ( te = entities.next ; te != &entities && num_entities != MAX_MAP_ENTITIES ; te = te->next )
	{
		ent_target[num_entities] = ValueForKey( te, "target" );
		if ( ent_target[num_entities][0] ) {
			ent_entity[num_entities] = te;
			num_entities++;
		}
	}

	for ( se = entities.next ; se != &entities ; se = se->next )
	{
		psz = ValueForKey( se, "targetname" );

		if ( psz == NULL || psz[0] == '\0' ) {
			continue;
		}

		sb = se->brushes.onext;
		if ( sb == &se->brushes ) {
			continue;
		}

		for ( k = 0 ; k < num_entities ; k++ )
		{
			if ( strcmp( ent_target[k], psz ) ) {
				continue;
			}

			te = ent_entity[k];
			tb = te->brushes.onext;
			if ( tb == &te->brushes ) {
				continue;
			}

			for ( i = 0 ; i < 3 ; i++ )
				mid[i] = ( sb->mins[i] + sb->maxs[i] ) * 0.5;

			for ( i = 0 ; i < 3 ; i++ )
				mid1[i] = ( tb->mins[i] + tb->maxs[i] ) * 0.5;

			VectorSubtract( mid1, mid, dir );
			len = VectorNormalize( dir, dir );
			s1[0] = -dir[1] * 8 + dir[0] * 8;
			s2[0] = dir[1] * 8 + dir[0] * 8;
			s1[1] = dir[0] * 8 + dir[1] * 8;
			s2[1] = -dir[0] * 8 + dir[1] * 8;

			qglColor3f( se->eclass->color[0], se->eclass->color[1], se->eclass->color[2] );

			qglBegin( GL_LINES );
			qglVertex3fv( mid );
			qglVertex3fv( mid1 );

			arrows = (int)( len / 256 ) + 1;

			for ( i = 0 ; i < arrows ; i++ )
			{
				f = len * ( i + 0.5 ) / arrows;

				for ( j = 0 ; j < 3 ; j++ )
					mid1[j] = mid[j] + f * dir[j];
				qglVertex3fv( mid1 );
				qglVertex3f( mid1[0] + s1[0], mid1[1] + s1[1], mid1[2] );
				qglVertex3fv( mid1 );
				qglVertex3f( mid1[0] + s2[0], mid1[1] + s2[1], mid1[2] );
			}

			qglEnd();
		}
	}
}

extern void AssignModel();

static const char *model_classnames[] =
{
	"misc_model",
	"misc_model_static",
	"misc_model_breakable",
	"misc_gamemodel",
	"model_static",
};

static const size_t model_classnames_count = sizeof( model_classnames ) / sizeof( *model_classnames );

qboolean IsModelEntity( const char *name )
{
	for ( size_t i = 0; i < model_classnames_count; i++ )
	{
		if ( stricmp( name, model_classnames[i] ) == 0 )
		{
			return qtrue;
		}
	}
	return qfalse;
}

void CreateEntityFromName( const char* name, const vec3_t origin ){
	entity_t *e;
	brush_t* b;
	if ( stricmp( name, "worldspawn" ) == 0 ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "Can't create an entity with worldspawn." ), "info", 0 );
		return;
	}

	e = Entity_Alloc();
	SetKeyValue( e, "classname", name );

	if ( e->eclass->fixedsize ) {
		Select_Delete();
		b = Brush_Create( e->eclass->mins, e->eclass->maxs, &e->eclass->texdef );
		Entity_LinkBrush( e, b );
		Brush_AddToList( b, &active_brushes );
		Select_Brush( b );
		Brush_Move( b, origin, true );
	}
	else
	{
		Select_GroupEntity( e );
		if ( e->brushes.onext == &e->brushes ) {
			Sys_FPrintf( SYS_ERR, "CreateEntityFromName: selection could not be grouped\n" );
			Entity_Free( e );
			return;
		}
	}

	Entity_AddToList( e, &entities );
	Undo_EndEntity( e );

	Select_Deselect();

	// tweaking: when right clic dropping a light entity, ask for light value in a custom dialog box
	// see SF bug 105383

	if ( g_pGameDescription->mGameFile == "hl.game" ) {
		// FIXME - Hydra: really we need a combined light AND color dialog for halflife.
		if ( ( stricmp( name, "light" ) == 0 )  ||
			 ( stricmp( name, "light_environment" ) == 0 ) ||
			 ( stricmp( name, "light_spot" ) == 0 ) ) {
			int intensity = g_PrefsDlg.m_iLastLightIntensity;

			// Create and show the dialog box
			//    CWnd *pWnd;
			//    pWnd = prompt.GetDlgItem( IDC_EDIT1 );
			//    prompt.GotoDlgCtrl( pWnd );
			if ( DoLightIntensityDlg( &intensity ) == IDOK ) {
				g_PrefsDlg.m_iLastLightIntensity = intensity;
				char buf[30];
				sprintf( buf, "255 255 255 %d", intensity );
				SetKeyValue( e, "_light", buf );
			}
		}
	}
	else
	{
		if ( stricmp( name, "light" ) == 0 ) {
			int intensity = g_PrefsDlg.m_iLastLightIntensity;

			// Create and show the dialog box
			//    CWnd *pWnd;
			//    pWnd = prompt.GetDlgItem( IDC_EDIT1 );
			//    prompt.GotoDlgCtrl( pWnd );
			if ( DoLightIntensityDlg( &intensity ) == IDOK ) {
				g_PrefsDlg.m_iLastLightIntensity = intensity;
				char buf[10];
				sprintf( buf, "%d", intensity );
				SetKeyValue( e, "light", buf );
			}
		}
	}
	Select_Brush( e->brushes.onext );

	if ( IsModelEntity( name ) == qtrue ) {
		SetInspectorMode( W_ENTITY );
		AssignModel();
	}
}

void CreateRightClickEntity( XYWnd* pWnd, int x, int y, const char* pName ){
	int height = pWnd->GetWidget()->allocation.height;
	vec3_t point;
	pWnd->SnapToPoint( x, height - 1 - y, point );

	int nDim = ( pWnd->GetViewType() == XY ) ? 2 : ( pWnd->GetViewType() == YZ ) ? 0 : 1;
	float fWorkMid = ( g_qeglobals.d_work_min[nDim] + g_qeglobals.d_work_max[nDim] ) * 0.5;
	point[nDim] = g_qeglobals.d_gridsize * ( (int)( fWorkMid / g_qeglobals.d_gridsize ) );

	CreateEntityFromName( pName, point );
}


brush_t* CreateSmartBrush( vec3_t v ){
	vec3_t mins, maxs;
	int i;
	brush_t   *n;

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = v[i] - 16;
		maxs[i] = v[i] + 16;
	}

	n = Brush_Create( mins, maxs, &g_qeglobals.d_texturewin.texdef );
	if ( !n ) {
		return NULL;
	}

	Brush_AddToList( n, &selected_brushes );
	//Entity_LinkBrush(world_entity, n);
	Brush_Build( n );
	return n;
}

CString g_strSmartEntity;
int g_nSmartX;
int g_nSmartY;
bool g_bSmartWaiting;
void _SmartPointDone( bool b, int n ){
	g_bSmartWaiting = false;
}

void CreateSmartEntity( XYWnd* pWnd, int x, int y, const char* pName ){
	g_nSmartX = x;
	g_nSmartY = y;
	g_strSmartEntity = pName;
	if ( g_strSmartEntity.Find( "Smart_Train" ) >= 0 ) {
		ShowInfoDialog( "Select the path of the train by left clicking in XY, YZ and/or XZ views. You can move an already dropped point by grabbing and moving it. When you are finished, press ENTER to accept and create the entity and path(s), press ESC to abandon the creation" );
		g_bPathMode = true;
		g_nPathLimit = 0;
		g_nPathCount = 0;
		g_bSmartGo = true;
	}
	else
	if ( g_strSmartEntity.Find( "Smart_Monster..." ) >= 0 ) {
		g_bPathMode = true;
		g_nPathLimit = 0;
		g_nPathCount = 0;
	}
	else
	if ( g_strSmartEntity.Find( "Smart_Rotating" ) >= 0 ) {
		g_bSmartWaiting = true;
		ShowInfoDialog( "Left click to specify the rotation origin" );
		AcquirePath( 1, &_SmartPointDone );
		while ( g_bSmartWaiting )
			gtk_main_iteration();
		HideInfoDialog();
		CPtrArray array;
		g_bScreenUpdates = false;
		CreateRightClickEntity( g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_rotating" );
		array.Add( reinterpret_cast<void*>( selected_brushes.next ) );
		Select_Deselect();
		brush_t* pBrush = CreateSmartBrush( g_PathPoints[0] );
		array.Add( pBrush );
		Select_Deselect();
		Select_Brush( reinterpret_cast<brush_t*>( array.GetAt( 0 ) ) );
		Select_Brush( reinterpret_cast<brush_t*>( array.GetAt( 1 ) ) );
		ConnectEntities();
		g_bScreenUpdates = true;
	}
}

void FinishSmartCreation(){
	CPtrArray array;
	HideInfoDialog();
	//  brush_t* pEntities = NULL;
	int n;

	if ( g_strSmartEntity.Find( "Smart_Train" ) >= 0 ) {
		g_bScreenUpdates = false;
		CreateRightClickEntity( g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_train" );
		array.Add( reinterpret_cast<void*>( selected_brushes.next ) );
		for ( n = 0; n < g_nPathCount; n++ )
		{
			Select_Deselect();
			CreateRightClickEntity( g_pParentWnd->ActiveXY(), g_PathPoints[n].m_ptScreenX,
									g_PathPoints[n].m_ptScreenY, "path_corner" );
			array.Add( reinterpret_cast<void*>( selected_brushes.next ) );
		}

		for ( n = 0; n < g_nPathCount; n++ )
		{
			Select_Deselect();
			Select_Brush( reinterpret_cast<brush_t*>( array.GetAt( n ) ) );
			Select_Brush( reinterpret_cast<brush_t*>( array.GetAt( n + 1 ) ) );
			ConnectEntities();
		}
		g_bScreenUpdates = true;

	}
	g_nPathCount = 0;
	g_bPathMode = false;
	Sys_UpdateWindows( W_ALL );
}

void CleanCopyEntities(){
	entity_t* pe = g_enClipboard.next;
	while ( pe != NULL && pe != &g_enClipboard )
	{
		entity_t* next = pe->next;
		epair_t* enext = NULL;
		for ( epair_t* ep = pe->epairs ; ep ; ep = enext )
		{
			enext = ep->next;
			free( ep->key );
			free( ep->value );
			free( ep );
		}
		free( pe );
		pe = next;
	}
	g_enClipboard.next = g_enClipboard.prev = &g_enClipboard;
}

entity_t *Entity_CopyClone( entity_t *e ){
	entity_t  *n;
	epair_t       *ep, *np;

	n = (entity_t*)qmalloc( sizeof( *n ) );
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;

	// add the entity to the entity list
	n->next = g_enClipboard.next;
	g_enClipboard.next = n;
	n->next->prev = n;
	n->prev = &g_enClipboard;

	for ( ep = e->epairs ; ep ; ep = ep->next )
	{
		np = (epair_t*)qmalloc( sizeof( *np ) );
		np->key = copystring( ep->key );
		np->value = copystring( ep->value );
		np->next = n->epairs;
		n->epairs = np;
	}
	return n;
}

bool OnList( entity_t* pFind, CPtrArray* pList ){
	int nSize = pList->GetSize();
	while ( nSize-- > 0 )
	{
		entity_t* pEntity = reinterpret_cast<entity_t*>( pList->GetAt( nSize ) );
		if ( pEntity == pFind ) {
			return true;
		}
	}
	return false;
}

// =============================================================================
// XYWnd class

XYWnd::XYWnd ()
	: GLWindow( FALSE ), m_XORRectangle( m_pWidget ){
	g_brClipboard.next = &g_brClipboard;
	g_brUndo.next = &g_brUndo;
	g_nScaleHow = 0;
	g_bRotateMode = false;
	g_bClipMode = false;
	g_bRogueClipMode = false;
	g_bSwitch = true;
	g_pMovingClip = (ClipPoint*)NULL;
	g_pMovingPath = (ClipPoint*)NULL;
	g_brFrontSplits.next = &g_brFrontSplits;
	g_brBackSplits.next = &g_brBackSplits;
	m_bActive = false;
	//m_bTiming = true;
	m_bTiming = false;
	m_bRButtonDown = false;
	m_nUpdateBits = W_XY;
	g_bPathMode = false;
	g_nPathCount = 0;
	g_nPathLimit = 0;
	m_nButtonstate = 0;
//  m_mnuDrop = (GtkWidget*)NULL;
	XY_Init();
}

vec3_t& XYWnd::Rotation(){
	return g_vRotation;
}

vec3_t& XYWnd::RotateOrigin(){
	return g_vRotateOrigin;
}

/*
   ==============
   XY_Overlay
   ==============
 */
void XYWnd::XY_Overlay(){
	int w, h;
	int r[4];
	static vec3_t lastz;
	static vec3_t lastcamera;

	qglViewport( 0, 0, m_nWidth, m_nHeight );

	//
	// set up viewpoint
	//
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	w = (int)( m_nWidth / 2 / m_fScale );
	h = (int)( m_nHeight / 2 / m_fScale );

	qglOrtho( m_vOrigin[0] - w, m_vOrigin[0] + w, m_vOrigin[1] - h, m_vOrigin[1] + h, g_MinWorldCoord, g_MaxWorldCoord );
	//
	// erase the old camera and z checker positions
	// if the entire xy hasn't been redrawn
	//
	if ( m_bDirty ) {
		qglReadBuffer( GL_BACK );
		qglDrawBuffer( GL_FRONT );

		qglRasterPos2f( lastz[0] - 9, lastz[1] - 9 );
		qglGetIntegerv( GL_CURRENT_RASTER_POSITION,r );
		qglCopyPixels( r[0], r[1], 18,18, GL_COLOR );

		qglRasterPos2f( lastcamera[0] - 50, lastcamera[1] - 50 );
		qglGetIntegerv( GL_CURRENT_RASTER_POSITION,r );
		qglCopyPixels( r[0], r[1], 100,100, GL_COLOR );
	}
	m_bDirty = true;

	//
	// save off underneath where we are about to draw
	//
	VectorCopy( z.origin, lastz );
	VectorCopy( g_pParentWnd->GetCamWnd()->Camera()->origin, lastcamera );

	qglReadBuffer( GL_FRONT );
	qglDrawBuffer( GL_BACK );

	qglRasterPos2f( lastz[0] - 9, lastz[1] - 9 );
	qglGetIntegerv( GL_CURRENT_RASTER_POSITION,r );
	qglCopyPixels( r[0], r[1], 18,18, GL_COLOR );

	qglRasterPos2f( lastcamera[0] - 50, lastcamera[1] - 50 );
	qglGetIntegerv( GL_CURRENT_RASTER_POSITION,r );
	qglCopyPixels( r[0], r[1], 100,100, GL_COLOR );

	//
	// draw the new icons
	//
	qglDrawBuffer( GL_FRONT );

	qglShadeModel( GL_FLAT );
	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_TEXTURE_1D );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );
	qglColor3f( 0, 0, 0 );

	DrawCameraIcon();
	DrawZIcon();

	qglDrawBuffer( GL_BACK );
	qglFinish();
}

vec3_t& XYWnd::GetOrigin(){
	return m_vOrigin;
}

void XYWnd::SetOrigin( vec3_t org ){
	m_vOrigin[0] = org[0];
	m_vOrigin[1] = org[1];
	m_vOrigin[2] = org[2];
}

void XYWnd::OnSize( int cx, int cy ){
	m_nWidth = cx;
	m_nHeight = cy;
}

brush_t hold_brushes;

void XYWnd::Clip(){
	if ( ClipMode() ) {
		hold_brushes.next = &hold_brushes;
		ProduceSplitLists();
		brush_t* pList;
		if ( g_PrefsDlg.m_bSwitchClip ) {
			pList = ( !g_bSwitch ) ? &g_brFrontSplits : &g_brBackSplits;
		}
		else{
			pList = ( g_bSwitch ) ? &g_brFrontSplits : &g_brBackSplits;
		}

		if ( pList->next != pList ) {
			Brush_CopyList( pList, &hold_brushes );
			CleanList( &g_brFrontSplits );
			CleanList( &g_brBackSplits );
			Select_Delete();
			Brush_CopyList( &hold_brushes, &selected_brushes );
			if ( RogueClipMode() ) {
				RetainClipMode( false );
			}
			else{
				RetainClipMode( true );
			}
			Sys_UpdateWindows( W_ALL );
		}
	}
	else if ( PathMode() ) {
		FinishSmartCreation();
		if ( g_pPathFunc ) {
			g_pPathFunc( true, g_nPathCount );
		}
		g_pPathFunc = NULL;
		g_nPathCount = 0;
		g_bPathMode = false;
	}
}

void XYWnd::SplitClip(){
	ProduceSplitLists();
	if ( ( g_brFrontSplits.next != &g_brFrontSplits ) &&
		 ( g_brBackSplits.next != &g_brBackSplits ) ) {
		Select_Delete();
		Brush_CopyList( &g_brFrontSplits, &selected_brushes );
		Brush_CopyList( &g_brBackSplits, &selected_brushes );
		CleanList( &g_brFrontSplits );
		CleanList( &g_brBackSplits );
		if ( RogueClipMode() ) {
			RetainClipMode( false );
		}
		else{
			RetainClipMode( true );
		}
	}
}

void XYWnd::FlipClip(){
	g_bSwitch = !g_bSwitch;
	Sys_UpdateWindows( XY | W_CAMERA_IFON );
}

// makes sure the selected brush or camera is in view
void XYWnd::PositionView(){
	int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
	brush_t* b = selected_brushes.next;
	if ( b && b->next != b ) {
		Select_GetMid( m_vOrigin );
	}
	else
	{
		m_vOrigin[nDim1] = g_pParentWnd->GetCamWnd()->Camera()->origin[nDim1];
		m_vOrigin[nDim2] = g_pParentWnd->GetCamWnd()->Camera()->origin[nDim2];
	}
}

void XYWnd::VectorCopyXY( vec3_t in, vec3_t out ){
	if ( m_nViewType == XY ) {
		out[0] = in[0];
		out[1] = in[1];
	}
	else if ( m_nViewType == XZ ) {
		out[0] = in[0];
		out[2] = in[2];
	}
	else
	{
		out[1] = in[1];
		out[2] = in[2];
	}
}

void XYWnd::RetainClipMode( bool bMode ){
	bool bSave = g_bRogueClipMode;
	SetClipMode( bMode );
	if ( bMode == true ) {
		g_bRogueClipMode = bSave;
	}
	else{
		g_bRogueClipMode = false;
	}
}

void XYWnd::SetClipMode( bool bMode ){
	g_bClipMode = bMode;
	g_bRogueClipMode = false;
	if ( bMode ) {
		g_Clip1.Reset();
		g_Clip2.Reset();
		g_Clip3.Reset();
		CleanList( &g_brFrontSplits );
		CleanList( &g_brBackSplits );
		g_brFrontSplits.next = &g_brFrontSplits;
		g_brBackSplits.next = &g_brBackSplits;

		// ydnar: set clipper points based on first selected patch mesh
		if ( selected_brushes.next != &selected_brushes ) {
			bool found = false;
			for ( brush_t *pb = selected_brushes.next; pb != &selected_brushes; pb = pb->next )
			{
				if ( pb->patchBrush ) {
					found = true;
					VectorCopy( pb->pPatch->ctrl[ 0 ][ 0 ].xyz, g_Clip1.m_ptClip );
					VectorCopy( pb->pPatch->ctrl[ pb->pPatch->width - 1 ][ pb->pPatch->height - 1 ].xyz, g_Clip2.m_ptClip );
					VectorCopy( pb->pPatch->ctrl[ pb->pPatch->width - 1 ][ 0 ].xyz, g_Clip3.m_ptClip );
					g_Clip1.Set( true );
					g_Clip2.Set( true );
					g_Clip3.Set( true );
					break;
				}
			}

			if ( found ) {
				// SetClipMode( true );
				Sys_UpdateWindows( XY | W_CAMERA_IFON );
			}
		}
	}
	else
	{
		if ( g_pMovingClip ) {
			ReleaseCapture();
			g_pMovingClip = NULL;
		}
		CleanList( &g_brFrontSplits );
		CleanList( &g_brBackSplits );
		g_brFrontSplits.next = &g_brFrontSplits;
		g_brBackSplits.next = &g_brBackSplits;
		Sys_UpdateWindows( XY | W_CAMERA_IFON );
	}
}

bool XYWnd::ClipMode(){
	return g_bClipMode;
}

bool XYWnd::RogueClipMode(){
	return g_bRogueClipMode;
}

bool XYWnd::PathMode(){
	return g_bPathMode;
}

bool XYWnd::PointMode(){
	return g_bPointMode;
}

void XYWnd::SetPointMode( bool b ){
	g_bPointMode = b;
	if ( !b ) {
		g_nPointCount = 0;
	}
}

void XYWnd::SetViewType( int n ){
	m_nViewType = n;
	if ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) {
		const char* str = "YZ Side";
		if ( m_nViewType == XY ) {
			str = "XY Top";
		}
		else if ( m_nViewType == XZ ) {
			str = "XZ Front";
		}

		if ( m_pParent != NULL ) {
			gtk_window_set_title( GTK_WINDOW( m_pParent ), str );
		}
	}
}

void XYWnd::Redraw( unsigned int nBits ){
	m_nUpdateBits = nBits;
	gtk_widget_queue_draw( m_pWidget );
	m_nUpdateBits = W_XY;
}

bool XYWnd::RotateMode(){
	return g_bRotateMode;
}

bool XYWnd::ScaleMode(){
	return g_bScaleMode;
}

bool XYWnd::SetRotateMode( bool bMode ){
	if ( bMode && selected_brushes.next != &selected_brushes ) {
		g_bRotateMode = true;
		Select_GetTrueMid( g_vRotateOrigin );
		g_vRotation[0] = g_vRotation[1] = g_vRotation[2] = 0.0;
	}
	else
	{
		if ( bMode ) {
			Sys_Printf( "Need a brush selected to turn on Mouse Rotation mode\n" );
		}
		g_bRotateMode = false;
	}
	RedrawWindow();
	return g_bRotateMode;
}

void XYWnd::SetScaleMode( bool bMode ){
	g_bScaleMode = bMode;
	RedrawWindow();
}

rectangle_t rectangle_from_area_xy(){
	XYWnd* xy = g_pParentWnd->ActiveXY();
	int nDim1 = ( xy->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( xy->GetViewType() == XY ) ? 1 : 2;
	float origin_left = xy->GetOrigin()[nDim1] - ( xy->Width() / 2 ) / xy->Scale();
	float origin_bottom = xy->GetOrigin()[nDim2] - ( xy->Height() / 2 ) / xy->Scale();
	float left = MIN( g_qeglobals.d_vAreaTL[nDim1], g_qeglobals.d_vAreaBR[nDim1] ) - origin_left;
	float top = MAX( g_qeglobals.d_vAreaTL[nDim2], g_qeglobals.d_vAreaBR[nDim2] ) - origin_bottom;
	float right = MAX( g_qeglobals.d_vAreaTL[nDim1], g_qeglobals.d_vAreaBR[nDim1] ) - origin_left;
	float bottom = MIN( g_qeglobals.d_vAreaTL[nDim2], g_qeglobals.d_vAreaBR[nDim2] ) - origin_bottom;
	left *= xy->Scale();
	top *= xy->Scale();
	right *= xy->Scale();
	bottom *= xy->Scale();
	return rectangle_t( left, bottom, right - left, top - bottom );
}

void update_xor_rectangle_xy( XORRectangle& xor_rectangle ){
	rectangle_t rectangle;
	if ( g_qeglobals.d_select_mode == sel_area ) {
		rectangle = rectangle_from_area_xy();
	}
	xor_rectangle.set( rectangle );
}

void XYWnd::OnMouseMove( guint32 nFlags, int pointx, int pointy ){
	// plugin entities
	// TODO TTimo handle return code
	DispatchOnMouseMove( nFlags, pointx, pointy );

	m_ptDownX = 0;
	m_ptDownY = 0;

	if ( g_PrefsDlg.m_bChaseMouse == TRUE &&
		 ( pointx < 0 || pointy < 0 || pointx > m_nWidth || pointy > m_nHeight ) &&
		 HasCapture() ) {
		float fAdjustment = ( g_qeglobals.d_gridsize / 8 * 64 ) / m_fScale;
		//m_ptDrag = point;
		m_ptDragAdjX = 0;
		m_ptDragAdjY = 0;

		if ( pointx < 0 ) {
			m_ptDragAdjX = (int)( -fAdjustment );
		}
		else if ( pointx > m_nWidth ) {
			m_ptDragAdjX = (int)( fAdjustment );
		}

		if ( pointy < 0 ) {
			m_ptDragAdjY = (int)( -fAdjustment );
		}
		else if ( pointy > m_nHeight ) {
			m_ptDragAdjY = (int)( fAdjustment );
		}

		if ( !HasTimer() ) {
			SetTimer( 50 );
			m_ptDragX = pointx;
			m_ptDragY = pointy;
			m_ptDragTotalX = 0;
			m_ptDragTotalY = 0;
		}
		return;
	}

	if ( HasTimer() ) {
		KillTimer();
		pressx -= m_ptDragTotalX;
		pressy += m_ptDragTotalY;
	}

	bool bCrossHair = false;
	if ( !m_bRButtonDown ) {
		tdp[0] = tdp[1] = tdp[2] = 0.0;
		SnapToPoint( pointx, m_nHeight - 1 - pointy, tdp );

		g_strStatus.Format( "x:: %.1f  y:: %.1f  z:: %.1f", tdp[0], tdp[1], tdp[2] );
		g_pParentWnd->SetStatusText( 1, g_strStatus );

		// i need to generalize the point code.. having 3 flavors pretty much sucks..
		// once the new curve stuff looks like it is going to stick i will
		// rationalize this down to a single interface..
		if ( PointMode() ) {
			if ( g_pMovingPoint && HasCapture() ) {
				bCrossHair = true;
				SnapToPoint( pointx, m_nHeight - 1 - pointy, g_pMovingPoint->m_ptClip );
				g_pMovingPoint->UpdatePointPtr();
				Sys_UpdateWindows( XY | W_CAMERA_IFON );
			}
			else
			{
				g_pMovingPoint = NULL;
				int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
				int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
				for ( int n = 0; n < g_nPointCount; n++ )
				{
					if ( fDiff( g_PointPoints[n].m_ptClip[nDim1], tdp[nDim1] ) < 3 &&
						 fDiff( g_PointPoints[n].m_ptClip[nDim2], tdp[nDim2] ) < 3 ) {
						bCrossHair = true;
						g_pMovingPoint = &g_PointPoints[n];
					}
				}
			}
		}
		else if ( ClipMode() ) {
			if ( g_pMovingClip && HasCapture() ) {
				bCrossHair = true;
				SnapToPoint( pointx, m_nHeight - 1 - pointy, g_pMovingClip->m_ptClip );
				Sys_UpdateWindows( XY | W_CAMERA_IFON );
			}
			else
			{
				g_pMovingClip = NULL;
				int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
				int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
				if ( g_Clip1.Set() ) {
					if ( fDiff( g_Clip1.m_ptClip[nDim1], tdp[nDim1] ) < 3 &&
						 fDiff( g_Clip1.m_ptClip[nDim2], tdp[nDim2] ) < 3 ) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip1;
					}
				}
				if ( g_Clip2.Set() ) {
					if ( fDiff( g_Clip2.m_ptClip[nDim1], tdp[nDim1] ) < 3 &&
						 fDiff( g_Clip2.m_ptClip[nDim2], tdp[nDim2] ) < 3 ) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip2;
					}
				}
				if ( g_Clip3.Set() ) {
					if ( fDiff( g_Clip3.m_ptClip[nDim1], tdp[nDim1] ) < 3 &&
						 fDiff( g_Clip3.m_ptClip[nDim2], tdp[nDim2] ) < 3 ) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip3;
					}
				}
			}
			if ( bCrossHair == false ) {
				XY_MouseMoved( pointx, m_nHeight - 1 - pointy, nFlags );
			}
		}
		else if ( PathMode() ) {
			if ( g_pMovingPath && HasCapture() ) {
				bCrossHair = true;
				SnapToPoint( pointx, m_nHeight - 1 - pointy, g_pMovingPath->m_ptClip );
				Sys_UpdateWindows( XY | W_CAMERA_IFON );
			}
			else
			{
				g_pMovingPath = NULL;
				int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
				int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
				for ( int n = 0; n < g_nPathCount; n++ )
				{
					if ( fDiff( g_PathPoints[n].m_ptClip[nDim1], tdp[nDim1] ) < 3 &&
						 fDiff( g_PathPoints[n].m_ptClip[nDim2], tdp[nDim2] ) < 3 ) {
						bCrossHair = true;
						g_pMovingPath = &g_PathPoints[n];
					}
				}
			}
		}
		else
		{
			XY_MouseMoved( pointx, m_nHeight - 1 - pointy, nFlags );
		}
	}
	else
	{
		XY_MouseMoved( pointx, m_nHeight - 1 - pointy, nFlags );
	}

	if ( ( nFlags & MK_RBUTTON ) == 0 ) {
		if ( bCrossHair && !g_bWaitCursor ) {
			GdkWindow *window;
			GdkDisplay *display;
			GdkCursor *cursor;

			window = gtk_widget_get_window( m_pWidget );
			display = gdk_window_get_display( window );
			cursor = gdk_cursor_new_for_display( display, GDK_CROSSHAIR );
			gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
			g_object_unref( cursor );
#else
			gdk_cursor_unref( cursor );
#endif
		}
		else
		{
			gdk_window_set_cursor( gtk_widget_get_window( m_pWidget ), NULL );
		}
	}

	update_xor_rectangle_xy( m_XORRectangle );
}

void XYWnd::OnMouseWheel( bool bUp, int pointx, int pointy ) {
	if ( bUp ) {
		if ( g_PrefsDlg.m_bMousewheelZoom == TRUE ) {
			// improved zoom-in
			// frame coverges to part of window where the cursor currently resides
			float old_scale = m_fScale;
			g_pParentWnd->OnViewZoomin();
			float scale_diff = 1.0 / old_scale - 1.0 / m_fScale;
			int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
			int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
			m_vOrigin[nDim1] += scale_diff * (pointx - 0.5 * m_nWidth);
			m_vOrigin[nDim2] -= scale_diff * (pointy - 0.5 * m_nHeight);
		}
		else {
				g_pParentWnd->OnViewZoomin();
		}
	}
	else {
		g_pParentWnd->OnViewZoomout();
	}

	int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
	Sys_UpdateWindows( nUpdate );
	g_pParentWnd->OnTimer();
}

void XYWnd::OnTimer(){
	int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
	m_vOrigin[nDim1] += m_ptDragAdjX / m_fScale;
	m_vOrigin[nDim2] -= m_ptDragAdjY / m_fScale;
	Sys_UpdateWindows( W_XY | W_CAMERA );
	m_ptDragX += m_ptDragAdjX;
	m_ptDragY += m_ptDragAdjY;
	m_ptDragTotalX += m_ptDragAdjX;
	m_ptDragTotalY += m_ptDragAdjY;
	XY_MouseMoved( m_ptDragX, m_nHeight - 1 - m_ptDragY, m_nScrollFlags );
}

void XYWnd::OnLButtonDown( guint32 flags, int pointx, int pointy ){
	g_pParentWnd->SetActiveXY( this );
	UndoCopy();

	// plugin entities
	if ( DispatchOnLButtonDown( flags, pointx, pointy ) ) {
		return;
	}

	if ( ClipMode() && !RogueClipMode() ) {
		DropClipPoint( flags, pointx, pointy );
	}
	else if ( PathMode() ) {
		DropPathPoint( flags, pointx, pointy );
	}
	else{ OriginalButtonDown( flags, pointx, pointy ); }
}

void XYWnd::OnMButtonDown( guint32 flags, int pointx, int pointy ){
	OriginalButtonDown( flags, pointx, pointy );
}

void XYWnd::OnRButtonDown( guint32 flags, int pointx, int pointy ){
	g_pParentWnd->SetActiveXY( this );
	m_ptDownX = pointx;
	m_ptDownY = pointy;
	m_bRButtonDown = true;

	if ( g_PrefsDlg.m_nMouseButtons == 3 ) { // 3 button mouse
		if ( flags & MK_CONTROL ) {
			if ( ClipMode() ) { // already there?
				DropClipPoint( flags, pointx, pointy );
			}
			else
			{
				SetClipMode( true );
				g_bRogueClipMode = true;
				DropClipPoint( flags, pointx, pointy );
			}
			return;
		}
	}
	OriginalButtonDown( flags, pointx, pointy );
}

void XYWnd::OnLButtonUp( guint32 flags, int pointx, int pointy ){
	// plugin entities
	if ( DispatchOnLButtonUp( flags, pointx, pointy ) ) {
		return;
	}

	if ( ClipMode() ) {
		if ( g_pMovingClip ) {
			ReleaseCapture();
			g_pMovingClip = NULL;
		}
	}
	OriginalButtonUp( flags, pointx, pointy );
}

void XYWnd::OnMButtonUp( guint32 flags, int pointx, int pointy ){
	OriginalButtonUp( flags, pointx, pointy );
}

void XYWnd::OnRButtonUp( guint32 flags, int pointx, int pointy ){
	m_bRButtonDown = false;
	if ( ( pointx == m_ptDownX ) && ( pointy == m_ptDownY ) ) { // mouse didn't move
		bool bGo = true;
		if ( Sys_AltDown() ) {
			bGo = false;
		}
		if ( flags & MK_CONTROL ) {
			bGo = false;
		}
		if ( flags & MK_SHIFT ) {
			bGo = false;
		}
		if ( bGo ) {
			HandleDrop();
		}
	}
	OriginalButtonUp( flags, pointx, pointy );
}

void XYWnd::XY_MouseDown( int x, int y, int buttons ){
	vec3_t point;
	vec3_t origin, dir, right, up;

	m_nButtonstate = buttons;
	m_nPressx = x;
	m_nPressy = y;
	VectorCopy( vec3_origin, m_vPressdelta );

	VectorClear( point );
	XY_ToPoint( x, y, point );

	VectorCopy( point, origin );

	VectorClear( dir );
	if ( m_nViewType == XY ) { // view facing dir = negative Z
		origin[2] = g_MaxWorldCoord;
		dir[2] = -1;
		right[0] = 1 / m_fScale;
		right[1] = 0;
		right[2] = 0;
		up[0] = 0;
		up[1] = 1 / m_fScale;
		up[2] = 0;
	}
	else if ( m_nViewType == XZ ) {
		origin[1] = g_MinWorldCoord; // view facing dir = positive Y
		dir[1] = 1;
		right[0] = 1 / m_fScale;
		right[1] = 0;
		right[2] = 0;
		up[0] = 0;
		up[1] = 0;
		up[2] = 1 / m_fScale;
	}
	else  // if (m_nViewType == YZ)  // view facing dir = negative X
	{
		origin[0] = g_MaxWorldCoord;
		dir[0] = -1;
		right[0] = 0;
		right[1] = 1 / m_fScale;
		right[2] = 0;
		up[0] = 0;
		up[1] = 0;
		up[2] = 1 / m_fScale;
	}

	m_bPress_selection = ( selected_brushes.next != &selected_brushes );

	Sys_GetCursorPos( &m_ptCursorX, &m_ptCursorY );

	// lbutton = manipulate selection
	// shift-LBUTTON = select
	if ( ( buttons == MK_LBUTTON )
		 || ( buttons == ( MK_LBUTTON | MK_SHIFT ) )
		 || ( buttons == ( MK_LBUTTON | MK_CONTROL ) )
		 || ( buttons == ( MK_LBUTTON | MK_CONTROL | MK_SHIFT ) ) ) {
		Patch_SetView( ( m_nViewType == XY ) ? W_XY : ( m_nViewType == YZ ) ? W_YZ : W_XZ );
		Drag_Begin( x, y, buttons, right, up,   origin, dir );
		return;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;

	// control mbutton = move camera
	if ( m_nButtonstate == ( MK_CONTROL | nMouseButton ) ) {
		VectorCopyXY( point, g_pParentWnd->GetCamWnd()->Camera()->origin );
		Sys_UpdateWindows( W_CAMERA | W_XY_OVERLAY );
	}

	// mbutton = angle camera
	if ( ( g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON ) ||
		 ( g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) ) {
		VectorSubtract( point, g_pParentWnd->GetCamWnd()->Camera()->origin, point );

		int n1 = ( m_nViewType == XY ) ? 1 : 2;
		int n2 = ( m_nViewType == YZ ) ? 1 : 0;
		int nAngle = ( m_nViewType == XY ) ? YAW : PITCH;
		if ( point[n1] || point[n2] ) {
			g_pParentWnd->GetCamWnd()->Camera()->angles[nAngle] = 180 / Q_PI*atan2( point[n1], point[n2] );
			Sys_UpdateWindows( W_CAMERA_IFON | W_XY_OVERLAY );
		}
	}

	// shift mbutton = move z checker
	if ( m_nButtonstate == ( MK_SHIFT | nMouseButton ) ) {
		if ( RotateMode() || g_bPatchBendMode ) {
			SnapToPoint( x, y, point );
			VectorCopyXY( point, g_vRotateOrigin );
			if ( g_bPatchBendMode ) {
				VectorCopy( point, g_vBendOrigin );
			}
			Sys_UpdateWindows( W_XY );
			return;
		}
		else
		{
			SnapToPoint( x, y, point );
			if ( m_nViewType == XY ) {
				z.origin[0] = point[0];
				z.origin[1] = point[1];
			}
			else if ( m_nViewType == YZ ) {
				z.origin[0] = point[1];
				z.origin[1] = point[2];
			}
			else
			{
				z.origin[0] = point[0];
				z.origin[1] = point[2];
			}
			Sys_UpdateWindows( W_XY_OVERLAY | W_Z );
			return;
		}
	}

	update_xor_rectangle_xy( m_XORRectangle );
}

void XYWnd::XY_MouseUp( int x, int y, int buttons ){
	Drag_MouseUp( buttons );
	if ( !m_bPress_selection ) {
		Sys_UpdateWindows( W_ALL );
	}
	m_nButtonstate = 0;

	gdk_window_set_cursor( gtk_widget_get_window( m_pWidget ), NULL );

	update_xor_rectangle_xy( m_XORRectangle );
}

qboolean XYWnd::DragDelta( int x, int y, vec3_t move ){
	vec3_t xvec, yvec, delta;
	int i;

	xvec[0] = 1 / m_fScale;
	xvec[1] = xvec[2] = 0;
	yvec[1] = 1 / m_fScale;
	yvec[0] = yvec[2] = 0;

	for ( i = 0 ; i < 3 ; i++ )
	{
		delta[i] = xvec[i] * ( x - m_nPressx ) + yvec[i] * ( y - m_nPressy );
		if ( g_PrefsDlg.m_bSnap ) {
			delta[i] = floor( delta[i] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		}
	}
	VectorSubtract( delta, m_vPressdelta, move );
	VectorCopy( delta, m_vPressdelta );

	if ( move[0] || move[1] || move[2] ) {
		return true;
	}
	return false;
}

static GtkWidget * create_entity_menu_item( GtkWidget *submenu, CString strName )
{
	GtkWidget *item, *box, *icon, *label;
	CString filepath;
	GdkPixbuf *pixbuf;

	filepath = g_pGameDescription->mEnginePath;
	filepath += g_pGameDescription->mBaseGame;
	filepath += "/icons/entity_icons/";
	filepath += strName;
	filepath += ".png";

	item = gtk_menu_item_new();

	box = gtk_hbox_new( FALSE, 6 );
	
	gtk_container_add( GTK_CONTAINER( item ), box );
	gtk_widget_show( box );

	pixbuf = gdk_pixbuf_new_from_file( filepath, NULL );
	if( !pixbuf ) {
		filepath = g_strGameToolsPath;
		filepath += "entity_icons/";
		filepath += g_pGameDescription->mBaseGame;
		filepath += "/";
		filepath += strName;
		filepath += ".png";
		pixbuf = gdk_pixbuf_new_from_file( filepath, NULL );
		if( !pixbuf ) {
			filepath = g_strGameToolsPath;
			filepath += "entity_icons/";
			filepath += strName;
			filepath += ".png";
			pixbuf = gdk_pixbuf_new_from_file( filepath, NULL );
		}
	}
	if( pixbuf ) {
		icon = gtk_image_new_from_pixbuf( pixbuf );
		gtk_box_pack_start( GTK_BOX( box ), icon, FALSE, FALSE, 0 );
		gtk_widget_show( icon );

		g_object_unref( pixbuf );
	}
	label = gtk_label_new( strName );
	gtk_box_pack_start( GTK_BOX( box ), label, TRUE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0f, 0.5f );
	gtk_widget_show( label );

	g_object_set_data( G_OBJECT( item ), "classname-label", label );

	return item;
}
void XYWnd::HandleDrop(){
	if ( g_PrefsDlg.m_bRightClick == false ) {
		return;
	}

	if ( m_mnuDrop == NULL ) { // first time, load it up
		int nID = ID_ENTITY_START;
		GtkWidget *menu, *menu_in_menu, *item, *submenu, *submenu_root;

		menu = m_mnuDrop = gtk_menu_new();

		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, "Select" );
		create_menu_item_with_mnemonic( menu_in_menu, "Select Complete Tall",
										G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTCOMPLETETALL );
		create_menu_item_with_mnemonic( menu_in_menu, "Select Touching",
										G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTTOUCHING );
		create_menu_item_with_mnemonic( menu_in_menu, "Select Partial Tall",
										G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTPARTIALTALL );
		create_menu_item_with_mnemonic( menu_in_menu, "Select Inside",
										G_CALLBACK( HandleCommand ), ID_SELECTION_SELECTINSIDE );
		menu_separator( menu ); nID++;
		// NOTE: temporary commented out until we put it back in for good (that is with actual features)
		/*
		menu_in_menu = create_menu_in_menu_with_mnemonic (menu, "Group",);
		create_menu_item_with_mnemonic (menu_in_menu, "Add to...",
			G_CALLBACK (HandleCommand), ID_DROP_GROUP_ADDTO);
		create_menu_item_with_mnemonic (menu_in_menu, "Remove",
			G_CALLBACK (HandleCommand), ID_DROP_GROUP_REMOVE);
		create_menu_item_with_mnemonic (menu_in_menu, "Name...",
			G_CALLBACK (HandleCommand), ID_DROP_GROUP_NAME);
		menu_separator (menu_in_menu); nID++;
		create_menu_item_with_mnemonic (menu_in_menu, "New Group...",
			G_CALLBACK (HandleCommand), ID_DROP_GROUP_NEWGROUP);
		 */
		create_menu_item_with_mnemonic( menu, "Ungroup Entity",
										G_CALLBACK( HandleCommand ), ID_SELECTION_UNGROUPENTITY );

		create_menu_item_with_mnemonic( menu, "Move into entity",
										G_CALLBACK( HandleCommand ), ID_SELECTION_MERGE );
		create_menu_item_with_mnemonic( menu, "Move into worldspawn",
										G_CALLBACK( HandleCommand ), ID_SELECTION_SEPERATE );

		create_menu_item_with_mnemonic( menu, "Make Detail",
										G_CALLBACK( HandleCommand ), ID_SELECTION_MAKE_DETAIL );
		create_menu_item_with_mnemonic( menu, "Make Structural",
										G_CALLBACK( HandleCommand ), ID_SELECTION_MAKE_STRUCTURAL );
		menu_separator( menu ); nID++;

		menu_in_menu = create_menu_in_menu_with_mnemonic( menu, "Smart Entities" );
		create_menu_item_with_mnemonic( menu_in_menu, "Smart__Train",
										G_CALLBACK( HandleCommand ), nID++ );
		menu_separator( menu ); nID++;

		submenu = NULL;
		submenu_root = NULL;
		eclass_t    *e;
		CString strActive;
		CString strLast;
		CString strName;
		for ( e = eclass ; e ; e = e->next )
		{
			strLast = strName;
			strName = e->name;
			int n_ = strName.Find( "_" );
			if ( n_ > 0 ) {
				CString strLeft = strName.Left( n_ );
				CString strRight = strName.Right( strName.GetLength() - n_ - 1 );
				if ( strLeft == strActive ) { // this is a child
					assert( submenu );
					CheckMenuSplitting( submenu );
					item = create_entity_menu_item( submenu, strName );

					g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( nID++ ) );

					gtk_widget_show( item );
					gtk_menu_shell_append( GTK_MENU_SHELL( submenu ), item );
				}
				else
				{
					if ( submenu ) {
						// this is submenu from previous main_item, hook it back
						// we use submenu_root cause we may have been cascading submenu
						item = gtk_menu_item_new_with_label( strActive );
						gtk_widget_show( item );
						gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
						gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), submenu_root );
						g_ptrMenus.Add( submenu_root );
						submenu = NULL;
						submenu_root = NULL;
					}
					strActive = strLeft;

					submenu = gtk_menu_new();
					submenu_root = submenu;
					item = create_entity_menu_item( submenu, strName );
					g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( nID++ ) );
					gtk_widget_show( item );
					gtk_menu_shell_append( GTK_MENU_SHELL( submenu ), item );
				}
			}
			else
			{
				if ( submenu ) {
					// this is submenu from previous main_item, hook it back
					// we use submenu_root cause we may have been cascading submenu
					item = gtk_menu_item_new_with_label( strActive );
					gtk_widget_show( item );
					gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
					gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), submenu_root );
					g_ptrMenus.Add( submenu_root );
					submenu = NULL;
					submenu_root = NULL;
				}
				strActive = "";

				item = create_entity_menu_item( menu, strName );
				g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
									GINT_TO_POINTER( nID++ ) );
				gtk_widget_show( item );
				gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
			}
		}
	}

	gtk_menu_popup( GTK_MENU( m_mnuDrop ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

/*
   ==============
   NewBrushDrag
   ==============
 */
void XYWnd::NewBrushDrag( int x, int y ){
	vec3_t mins, maxs, junk;
	int i;
	float temp;
	brush_t   *n;

	if ( !DragDelta( x,y, junk ) ) {
		return;
	}

	// delete the current selection
	if ( selected_brushes.next != &selected_brushes ) {
		Brush_Free( selected_brushes.next );
	}

	SnapToPoint( m_nPressx, m_nPressy, mins );

	int nDim = ( m_nViewType == XY ) ? 2 : ( m_nViewType == YZ ) ? 0 : 1;

	//++timo clean
//  mins[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom_z / g_qeglobals.d_gridsize));
	mins[nDim] = g_qeglobals.d_gridsize * ( (int)( g_qeglobals.d_work_min[nDim] / g_qeglobals.d_gridsize ) );

	SnapToPoint( x, y, maxs );
//  maxs[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top_z / g_qeglobals.d_gridsize));
	maxs[nDim] = g_qeglobals.d_gridsize * ( (int)( g_qeglobals.d_work_max[nDim] / g_qeglobals.d_gridsize ) );
	if ( maxs[nDim] <= mins[nDim] ) {
		maxs[nDim] = mins[nDim] + g_qeglobals.d_gridsize;
	}

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( mins[i] == maxs[i] ) {
			return; // don't create a degenerate brush
		}
		if ( mins[i] > maxs[i] ) {
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}
	
	// Caulk the new brush
	if ( g_PrefsDlg.m_bCaulkNewBrushes == TRUE ) {
		texdef_t tex;
		IShader *shad = QERApp_Shader_ForName( "textures/common/caulk" );
		
		tex.SetName( shad->getName() );
		tex.scale[0] = g_PrefsDlg.m_fDefTextureScale;
		tex.scale[1] = g_PrefsDlg.m_fDefTextureScale;
		tex.flags = shad->getFlags();
		
		n = Brush_Create( mins, maxs, &tex );
	} else {
		n = Brush_Create( mins, maxs, &g_qeglobals.d_texturewin.texdef );
	}
	
	if ( !n ) {
		return;
	}

	// structural or detail?
	face_t *f;
		
	for ( f = n->brush_faces ; f ; f = f->next ) {
		if ( g_qeglobals.m_bMakeDetail == TRUE ) {
			f->texdef.contents |= CONTENTS_DETAIL;
		} else {
			f->texdef.contents &= ~CONTENTS_DETAIL;
		}
	}
	//

	vec3_t vSize;
	VectorSubtract( maxs, mins, vSize );
	g_strStatus.Format( "Size X:: %.1f  Y:: %.1f  Z:: %.1f", vSize[0], vSize[1], vSize[2] );
	g_pParentWnd->SetStatusText( 2, g_strStatus );

	Brush_AddToList( n, &selected_brushes );

	Entity_LinkBrush( world_entity, n );

	Brush_Build( n );

	//    Sys_UpdateWindows (W_ALL);
	Sys_UpdateWindows( W_XY | W_CAMERA );

}

/*
   ==============
   XY_MouseMoved
   ==============
 */
void XYWnd::XY_MouseMoved( int x, int y, int buttons ){
	vec3_t point;

	if ( !m_nButtonstate ) {
		if ( g_bCrossHairs ) {
			Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
		}
		return;
	}

	// lbutton without selection = drag new brush
	if ( m_nButtonstate == MK_LBUTTON && !m_bPress_selection && g_qeglobals.d_select_mode != sel_curvepoint && g_qeglobals.d_select_mode != sel_areatall ) {
		NewBrushDrag( x, y );
		return;
	}

	// lbutton (possibly with control and or shift)
	// with selection = drag selection
	if ( m_nButtonstate & MK_LBUTTON ) {
		Drag_MouseMoved( x, y, buttons );
		if ( g_qeglobals.d_select_mode != sel_area ) {
			Sys_UpdateWindows( W_XY_OVERLAY | W_CAMERA_IFON | W_Z );
		}
		return;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	// control mbutton = move camera
	if ( m_nButtonstate == ( MK_CONTROL | nMouseButton ) ) {
		SnapToPoint( x, y, point );
		VectorCopyXY( point, g_pParentWnd->GetCamWnd()->Camera()->origin );
		Sys_UpdateWindows( W_XY_OVERLAY | W_CAMERA );
		return;
	}

	// shift mbutton = move z checker
	if ( m_nButtonstate == ( MK_SHIFT | nMouseButton ) ) {
		if ( RotateMode() || g_bPatchBendMode ) {
			SnapToPoint( x, y, point );
			VectorCopyXY( point, g_vRotateOrigin );
			if ( g_bPatchBendMode ) {
				VectorCopy( point, g_vBendOrigin );
			}
			Sys_UpdateWindows( W_XY );
			return;
		}
		else
		{
			SnapToPoint( x, y, point );
			if ( m_nViewType == XY ) {
				z.origin[0] = point[0];
				z.origin[1] = point[1];
			}
			else if ( m_nViewType == YZ ) {
				z.origin[0] = point[1];
				z.origin[1] = point[2];
			}
			else
			{
				z.origin[0] = point[0];
				z.origin[1] = point[2];
			}
		}
		Sys_UpdateWindows( W_XY_OVERLAY | W_Z );
		return;
	}

	// mbutton = angle camera
	if ( ( g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON ) ||
		 ( g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) ) {
		SnapToPoint( x, y, point );
		VectorSubtract( point, g_pParentWnd->GetCamWnd()->Camera()->origin, point );

		int n1 = ( m_nViewType == XY ) ? 1 : 2;
		int n2 = ( m_nViewType == YZ ) ? 1 : 0;
		int nAngle = ( m_nViewType == XY ) ? YAW : PITCH;
		if ( point[n1] || point[n2] ) {
			g_pParentWnd->GetCamWnd()->Camera()->angles[nAngle] = 180 / Q_PI*atan2( point[n1], point[n2] );
			Sys_UpdateWindows( W_CAMERA_IFON | W_XY_OVERLAY );
		}
		return;
	}

	// rbutton = drag xy origin
	if ( m_nButtonstate == MK_RBUTTON ) {
		Sys_GetCursorPos( &x, &y );
		if ( x != m_ptCursorX || y != m_ptCursorY ) {
			int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
			int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
			m_vOrigin[nDim1] -= ( x - m_ptCursorX ) / m_fScale;
			m_vOrigin[nDim2] += ( y - m_ptCursorY ) / m_fScale;
			Sys_SetCursorPos( m_ptCursorX, m_ptCursorY );

			// create an empty cursor
			if ( !g_bWaitCursor ) {
				GdkWindow *window;
				GdkDisplay *display;
				GdkCursor *cursor;

				window = gtk_widget_get_window( m_pWidget );
				display = gdk_window_get_display( window );
				cursor = gdk_cursor_new_for_display( display, GDK_BLANK_CURSOR );

				gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
				g_object_unref( cursor );
#else
				gdk_cursor_unref( cursor );
#endif
			}

			Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
		}
		return;
	}

	// zoom in/out
	if ( m_nButtonstate == ( MK_SHIFT | MK_RBUTTON ) ) {
		Sys_GetCursorPos( &x, &y );
		if ( y != m_ptCursorY ) {
			if ( abs( m_ptCursorY - y ) > 10 ) {
				if ( m_ptCursorY < y ) {
					g_pParentWnd->OnViewZoomout();
				}
				else{
					g_pParentWnd->OnViewZoomin();
				}

				Sys_SetCursorPos( m_ptCursorX, m_ptCursorY );
			}
		}
		return;
	}
}

void XYWnd::OriginalButtonDown( guint32 nFlags, int pointx, int pointy ){
	SetFocus();
	SetCapture();
	XY_MouseDown( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
	m_nScrollFlags = nFlags;
}

void XYWnd::OriginalButtonUp( guint32 nFlags, int pointx, int pointy ){
	XY_MouseUp( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
	ReleaseCapture();
}

void XYWnd::DropClipPoint( guint32 nFlags, int pointx, int pointy ){
	if ( g_pMovingClip ) {
		SetCapture();
		SnapToPoint( pointx, m_pWidget->allocation.height - 1 - pointy, *g_pMovingClip );
	}
	else
	{
		vec3_t* pPt = NULL;
		if ( g_Clip1.Set() == false ) {
			pPt = g_Clip1;
			g_Clip1.Set( true );
			g_Clip1.m_ptScreenX = pointx;
			g_Clip1.m_ptScreenY = pointy;
		}
		else
		if ( g_Clip2.Set() == false ) {
			pPt = g_Clip2;
			g_Clip2.Set( true );
			g_Clip2.m_ptScreenX = pointx;
			g_Clip2.m_ptScreenY = pointy;
		}
		else
		if ( g_Clip3.Set() == false ) {
			pPt = g_Clip3;
			g_Clip3.Set( true );
			g_Clip3.m_ptScreenX = pointx;
			g_Clip3.m_ptScreenY = pointy;
		}
		else
		{
			RetainClipMode( true );
			pPt = g_Clip1;
			g_Clip1.Set( true );
			g_Clip1.m_ptScreenX = pointx;
			g_Clip1.m_ptScreenY = pointy;
		}
		SnapToPoint( pointx, m_pWidget->allocation.height - 1 - pointy, *pPt );
		// third coordinates for clip point: use d_work_max
		// Arnout: changed to use center of selection for clipping, saves level designers moving points all over the map
		// g_pParentWnd->ActiveXY()->GetViewType()
		// cf VIEWTYPE defintion: enum VIEWTYPE {YZ, XZ, XY};
		int nViewType = g_pParentWnd->ActiveXY()->GetViewType();
		int nDim = ( nViewType == YZ ) ? 0 : ( ( nViewType == XZ ) ?  1 : 2 );
		//(*pPt)[nDim] = g_qeglobals.d_work_max[nDim];
		vec3_t mid;
		Select_GetMid( mid );
		( *pPt )[nDim] = mid[nDim];
	}
	Sys_UpdateWindows( XY | W_CAMERA_IFON );
}

void XYWnd::DropPathPoint( guint32 nFlags, int pointx, int pointy ){
	if ( g_pMovingPath ) {
		SetCapture();
		SnapToPoint( pointx, m_pWidget->allocation.height - 1 - pointy, *g_pMovingPath );
	}
	else
	{
		g_PathPoints[g_nPathCount].Set( true );
		g_PathPoints[g_nPathCount].m_ptScreenX = pointx;
		g_PathPoints[g_nPathCount].m_ptScreenY = pointy;
		SnapToPoint( pointx, m_pWidget->allocation.height - 1 - pointy, g_PathPoints[g_nPathCount] );
		// third coordinates for dropped point: use d_work_max
		// g_pParentWnd->ActiveXY()->GetViewType()
		// cf VIEWTYPE definition: enum VIEWTYPE {YZ, XZ, XY};
		int nViewType = g_pParentWnd->ActiveXY()->GetViewType();
		int nDim = ( nViewType == YZ ) ? 0 : ( ( nViewType == XZ ) ? 1 : 2 );
		g_PathPoints[g_nPathCount].m_ptClip[nDim] = g_qeglobals.d_work_max[nDim];

		g_nPathCount++;
		if ( g_nPathCount == g_nPathLimit ) {
			if ( g_pPathFunc ) {
				g_pPathFunc( true, g_nPathCount );
			}
			g_nPathCount = 0;
			g_bPathMode = false;
			g_pPathFunc = NULL;
		}
	}
	Sys_UpdateWindows( XY | W_CAMERA_IFON );
}

// FIXME: AddPointPoint() redundant function never called
#if 0
void XYWnd::AddPointPoint( guint32 nFlags, vec3_t* pVec ){
	g_PointPoints[g_nPointCount].Set( true );
	//g_PointPoints[g_nPointCount].m_ptScreen = point;
	_VectorCopy( *pVec, g_PointPoints[g_nPointCount] );
	g_PointPoints[g_nPointCount].SetPointPtr( pVec );
	g_nPointCount++;
	Sys_UpdateWindows( XY | W_CAMERA_IFON );
}

// FIXME: ProduceSplits() redundant function never called
void XYWnd::ProduceSplits( brush_t** pFront, brush_t** pBack ){
	*pFront = NULL;
	*pBack = NULL;
	if ( ClipMode() ) {
		if ( g_Clip1.Set() && g_Clip2.Set() ) {
			face_t face;
			VectorCopy( g_Clip1.m_ptClip,face.planepts[0] );
			VectorCopy( g_Clip2.m_ptClip,face.planepts[1] );
			VectorCopy( g_Clip3.m_ptClip,face.planepts[2] );
			if ( selected_brushes.next && ( selected_brushes.next->next == &selected_brushes ) ) {
				if ( g_Clip3.Set() == false ) {
					if ( m_nViewType == XY ) {
						face.planepts[0][2] = selected_brushes.next->mins[2];
						face.planepts[1][2] = selected_brushes.next->mins[2];
						face.planepts[2][0] = Betwixt( g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0] );
						face.planepts[2][1] = Betwixt( g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1] );
						face.planepts[2][2] = selected_brushes.next->maxs[2];
					}
					else if ( m_nViewType == YZ ) {
						face.planepts[0][0] = selected_brushes.next->mins[0];
						face.planepts[1][0] = selected_brushes.next->mins[0];
						face.planepts[2][1] = Betwixt( g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1] );
						face.planepts[2][2] = Betwixt( g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2] );
						face.planepts[2][0] = selected_brushes.next->maxs[0];
					}
					else
					{
						face.planepts[0][1] = selected_brushes.next->mins[1];
						face.planepts[1][1] = selected_brushes.next->mins[1];
						face.planepts[2][0] = Betwixt( g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0] );
						face.planepts[2][2] = Betwixt( g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2] );
						face.planepts[2][1] = selected_brushes.next->maxs[1];
					}
				}

				Brush_SplitBrushByFace( selected_brushes.next, &face, pFront, pBack );
			}

		}
	}
}
#endif

void XYWnd::PlanePointsFromClipPoints( vec3_t planepts[3], brush_t *pBrush ){
	VectorCopy( g_Clip1.m_ptClip,planepts[0] );
	VectorCopy( g_Clip2.m_ptClip,planepts[1] );
	VectorCopy( g_Clip3.m_ptClip,planepts[2] );
	if ( g_Clip3.Set() == false ) {
		int n = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 2 : ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 0 : 1;
		int x = ( n == 0 ) ? 1 : 0;
		int y = ( n == 2 ) ? 1 : 2;

		if ( n == 1 ) { // on viewtype XZ, flip clip points
			planepts[0][n] = pBrush->maxs[n];
			planepts[1][n] = pBrush->maxs[n];
			planepts[2][x] = g_Clip1.m_ptClip[x];
			planepts[2][y] = g_Clip1.m_ptClip[y];
			planepts[2][n] = pBrush->mins[n];
		}
		else
		{
			planepts[0][n] = pBrush->mins[n];
			planepts[1][n] = pBrush->mins[n];
			planepts[2][x] = g_Clip1.m_ptClip[x];
			planepts[2][y] = g_Clip1.m_ptClip[y];
			planepts[2][n] = pBrush->maxs[n];
		}
	}
}

void XYWnd::ProduceSplitLists(){
	bool bCaulk = false;
	int nFlags;

	if ( AnyPatchesSelected() ) {
		Sys_Printf( "Deselecting patches for clip operation.\n" );
		brush_t *next;
		for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = next )
		{
			next = pb->next;
			if ( pb->patchBrush ) {
				Brush_RemoveFromList( pb );
				Brush_AddToList( pb, &active_brushes );
				UpdatePatchInspector();
			}
		}
		// ydnar: update the window if any patches are selected
		Sys_UpdateWindows( XY | W_CAMERA_IFON );
	}

	CleanList( &g_brFrontSplits );
	CleanList( &g_brBackSplits );
	g_brFrontSplits.next = &g_brFrontSplits;
	g_brBackSplits.next = &g_brBackSplits;
	if ( ClipMode() && ( g_Clip1.Set() && g_Clip2.Set() ) ) {
		brush_t* pBrush;
		for ( pBrush = selected_brushes.next ; pBrush != NULL && pBrush != &selected_brushes ; pBrush = pBrush->next )
		{
			brush_t* pFront = NULL;
			brush_t* pBack = NULL;

			face_t face;
			memset( &face,0,sizeof( face_t ) );
			PlanePointsFromClipPoints( face.planepts, pBrush );

			// decide whether caulking should be applied on the splits
			// FIXME: hack
			// this should take the first brush face, check shader for NODRAW, if it isn't nodraw then find the appropriate
			// common/ shader to use, out of solid+nodraw, nonsolid+nodraw, water+nodraw, lava+nodraw, nonsolid+nodraw+trans, water+nodraw+trans, lava+nodraw+trans.. and fog.. etc
			// or if none of those apply (unlikely), construct a new shader (shadername_nodraw) based on the shader of the first face, but with surfaceparm nodraw
			if ( g_PrefsDlg.m_bClipCaulk ) {
				nFlags = pBrush->brush_faces->pShader->getFlags();
				if ( ( nFlags & QER_NODRAW ) || ( nFlags & QER_NONSOLID ) || ( nFlags & QER_WATER ) || ( nFlags & QER_LAVA ) || ( nFlags & QER_FOG ) ) { // first face shader is anything other than solid AND opaque like caulk
					bCaulk = false; // use first face's shader for the splitting face
				}
				else{
					bCaulk = true; // use caulk
				}
			}

			Brush_SplitBrushByFace( pBrush, &face, &pFront, &pBack, bCaulk );
			if ( pBack ) {
				Brush_AddToList( pBack, &g_brBackSplits );
			}
			if ( pFront ) {
				Brush_AddToList( pFront, &g_brFrontSplits );
			}

		}
	}
}

void XYWnd::XY_Init(){
	m_vOrigin[0] = 0;
	m_vOrigin[1] = 20;
	m_vOrigin[2] = 46;
	m_fScale = 1;
}

void XYWnd::SnapToPoint( int x, int y, vec3_t point ){
	if ( g_PrefsDlg.m_bSnap ) {
		XY_ToGridPoint( x, y, point );
	}
	else
	{
		XY_ToPoint( x, y, point );
	}
}

// TTimo: watch it, this doesn't init one of the 3 coords
void XYWnd::XY_ToPoint( int x, int y, vec3_t point ){
	float fx = x;
	float fy = y;
	float fw = m_nWidth;
	float fh = m_nHeight;
	if ( m_nViewType == XY ) {
		point[0] = m_vOrigin[0] + ( fx - fw / 2 ) / m_fScale;
		point[1] = m_vOrigin[1] + ( fy - fh / 2 ) / m_fScale;
	}
	else if ( m_nViewType == YZ ) {
		point[1] = m_vOrigin[1] + ( fx - fw / 2 ) / m_fScale;
		point[2] = m_vOrigin[2] + ( fy - fh / 2 ) / m_fScale;
	}
	else
	{
		point[0] = m_vOrigin[0] + ( fx - fw / 2 ) / m_fScale;
		point[2] = m_vOrigin[2] + ( fy - fh / 2 ) / m_fScale;
	}
}

void XYWnd::XY_ToGridPoint( int x, int y, vec3_t point ){
	if ( m_nViewType == XY ) {
		point[0] = m_vOrigin[0] + ( x - m_nWidth / 2 ) / m_fScale;
		point[1] = m_vOrigin[1] + ( y - m_nHeight / 2 ) / m_fScale;
		point[0] = floor( point[0] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		point[1] = floor( point[1] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}
	else if ( m_nViewType == YZ ) {
		point[1] = m_vOrigin[1] + ( x - m_nWidth / 2 ) / m_fScale;
		point[2] = m_vOrigin[2] + ( y - m_nHeight / 2 ) / m_fScale;
		point[1] = floor( point[1] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		point[2] = floor( point[2] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}
	else
	{
		point[0] = m_vOrigin[0] + ( x - m_nWidth / 2 ) / m_fScale;
		point[2] = m_vOrigin[2] + ( y - m_nHeight / 2 ) / m_fScale;
		point[0] = floor( point[0] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
		point[2] = floor( point[2] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}
}

/*
   ============================================================================

   DRAWING

   ============================================================================
 */

/*
   ==============
   XY_DrawGrid
   ==============
 */
void XYWnd::XY_DrawGrid(){
	float x, y, xb, xe, yb, ye;
	float w, h;
	char text[32];
	int step, stepx, stepy, colour;
	step = stepx = stepy = MAX( 64, (int)g_qeglobals.d_gridsize );

/*
	int stepSize = (int)(8 / m_fScale);
	if (stepSize > step)
	{
		int i;
		for (i = 1; i < stepSize; i <<= 1);
		step = i;
	}
*/

	//Sys_Printf("scale: %f\n", m_fScale);
	//Sys_Printf("step before: %i\n", step);
	//Sys_Printf("scaled step: %f\n", step * m_fScale);
	while ( ( step * m_fScale ) < 4.0f ) // make sure major grid spacing is at least 4 pixels on the screen
		step *= 8;
	//Sys_Printf("step after: %i\n", step);
	while ( ( stepx * m_fScale ) < 40.0f ) // text step x must be at least 40 pixels
		stepx *= 2;
	while ( ( stepy * m_fScale ) < 40.0f ) // text step y must be at least 40 pixels
		stepy *= 2;

	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_TEXTURE_1D );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );

	w = ( m_nWidth / 2 / m_fScale );
	h = ( m_nHeight / 2 / m_fScale );

	int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( m_nViewType == XY ) ? 1 : 2;

	xb = m_vOrigin[nDim1] - w;
	if ( xb < region_mins[nDim1] ) {
		xb = region_mins[nDim1];
	}
	xb = step * floor( xb / step );

	xe = m_vOrigin[nDim1] + w;
	if ( xe > region_maxs[nDim1] ) {
		xe = region_maxs[nDim1];
	}
	xe = step * ceil( xe / step );

	yb = m_vOrigin[nDim2] - h;
	if ( yb < region_mins[nDim2] ) {
		yb = region_mins[nDim2];
	}
	yb = step * floor( yb / step );

	ye = m_vOrigin[nDim2] + h;
	if ( ye > region_maxs[nDim2] ) {
		ye = region_maxs[nDim2];
	}
	ye = step * ceil( ye / step );

#define COLORS_DIFFER( a,b ) \
	( g_qeglobals.d_savedinfo.colors[a][0] != g_qeglobals.d_savedinfo.colors[b][0] || \
	  g_qeglobals.d_savedinfo.colors[a][1] != g_qeglobals.d_savedinfo.colors[b][1] || \
	  g_qeglobals.d_savedinfo.colors[a][2] != g_qeglobals.d_savedinfo.colors[b][2] )

	// djbob
	// draw minor blocks
	if ( m_fScale > .1 && g_qeglobals.d_showgrid && g_qeglobals.d_gridsize * m_fScale >= 4 ) {
		if ( g_qeglobals.d_gridsize < 1 ) {
			colour = COLOR_GRIDMINOR_ALT;
		}
		else{
			colour = COLOR_GRIDMINOR;
		}

		if ( COLORS_DIFFER( colour, COLOR_GRIDBACK ) ) {
			qglColor3fv( g_qeglobals.d_savedinfo.colors[colour] );

			qglBegin( GL_LINES );
			for ( x = xb ; x < xe ; x += g_qeglobals.d_gridsize )
			{
				if ( !( (int)x & ( step - 1 ) ) && !( (int)x - x ) ) {
					continue;
				}
				qglVertex2f( x, yb );
				qglVertex2f( x, ye );
			}
			for ( y = yb ; y < ye ; y += g_qeglobals.d_gridsize )
			{
				if ( !( (int)y & ( step - 1 ) )  && !( (int)y - y ) ) {
					continue;
				}
				qglVertex2f( xb, y );
				qglVertex2f( xe, y );
			}
			qglEnd();
		}
	}

	if ( g_qeglobals.d_gridsize < 1 ) {
		colour = COLOR_GRIDMAJOR_ALT;
	}
	else{
		colour = COLOR_GRIDMAJOR;
	}

	// draw major blocks
	if ( COLORS_DIFFER( colour, COLOR_GRIDBACK ) ) {
		qglColor3fv( g_qeglobals.d_savedinfo.colors[colour] );
	}

	if ( g_qeglobals.d_showgrid ) {
		qglBegin( GL_LINES );
		for ( x = xb ; x <= xe ; x += step )
		{
			qglVertex2f( x, yb );
			qglVertex2f( x, ye );
		}
		for ( y = yb ; y <= ye ; y += step )
		{
			qglVertex2f( xb, y );
			qglVertex2f( xe, y );
		}
		qglEnd();
	}

	// draw coordinate text if needed
	if ( g_qeglobals.d_savedinfo.show_coordinates ) {
		qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT] );

		// Pixels between top of label for vertical grid line and top of grid view window.
		// Note: There is currently a bug where the top few pixels of the grid view are hidden
		// under the border.  So you should add about 5 to the desired value here.  However,
		// the font ascent reaches higher than all digits, so you can subtract a few from the final
		// number.
		const int pixelsTopCushion = 4;

		// Pixels between left of label and
		//   - left of grid view window (for horizontal grid line label) or
		//   - drawn vertical grid line (for vertical grid line label).
		// IMPORTANT! Must be at least 1 otherwise labels might not be drawn,
		// because the origin of the text might be off screen due to rounding
		const int pixelsLeftCushion = 2;										 

		// Pixels between baseline of horizontal grid line label and drawn horizontal grid line.
		const int pixelsButtomCushion = 2;

		float yPosLabelsTop = m_vOrigin[nDim2] + h - ( gtk_glwidget_font_ascent() + pixelsTopCushion ) / m_fScale;
		float xPosLabelsLeft = m_vOrigin[nDim1] - w + pixelsLeftCushion / m_fScale;
		float leftCushion = pixelsLeftCushion / m_fScale;
		float bottomOffset = ( pixelsButtomCushion - gtk_glwidget_font_descent() ) / m_fScale;

		// This renders the numbers along varying X on top of the grid view (labels vertical grid lines).
		for ( x = xb - ( (int) xb ) % stepx; x <= xe; x += stepx ) {
			qglRasterPos2f( x + leftCushion, yPosLabelsTop );
			sprintf( text, "%i", (int) x );
			gtk_glwidget_print_string( text );
		}

		// This renders the numbers along varying Y on the left of the grid view (labels horizontal grid lines).
		for ( y = yb - ( (int) yb ) % stepy; y <= ye; y += stepy ) {
			qglRasterPos2f( xPosLabelsLeft, y + bottomOffset );
			sprintf( text, "%i", (int) y );
			gtk_glwidget_print_string( text );
		}

		if ( Active() ) {
			qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME] );
		}

		// We do this part (the old way) only if show_axis is disabled
		if ( !g_qeglobals.d_savedinfo.show_axis ) {
			qglRasterPos2f( m_vOrigin[nDim1] - w + 35 / m_fScale, m_vOrigin[nDim2] + h - 30 / m_fScale );

			char cView[20];
			if ( m_nViewType == XY ) {
				strcpy( cView, "XY Top" );
			}
			else
			if ( m_nViewType == XZ ) {
				strcpy( cView, "XZ Front" );
			}
			else {
				strcpy( cView, "YZ Side" );
			}

			gtk_glwidget_print_string( cView );
		}
	}

	if ( g_qeglobals.d_savedinfo.show_axis ) {
		// Draw two lines with corresponding axis colors to highlight current view
		// Horizontal line: nDim1 color
		qglLineWidth( 2 );
		
		qglBegin( GL_LINES );
		qglColor3fv( g_qeglobals.d_savedinfo.AxisColors[nDim1] );
		qglVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
		qglVertex2f( m_vOrigin[nDim1] - w + 65 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
		qglVertex2f( 0, 0 );
		qglVertex2f( 32 / m_fScale, 0 );
		qglColor3fv( g_qeglobals.d_savedinfo.AxisColors[nDim2] );
		qglVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
		qglVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 20 / m_fScale );
		qglVertex2f( 0, 0 );
		qglVertex2f( 0, 32 / m_fScale );
		qglEnd();
		
		qglLineWidth( 1 );

		// Now print axis symbols
		qglColor3fv( g_qeglobals.d_savedinfo.AxisColors[nDim1] );
		qglRasterPos2f( m_vOrigin[nDim1] - w + 57 / m_fScale, m_vOrigin[nDim2] + h - 60 / m_fScale );
		gtk_glwidget_print_char( g_AxisName[nDim1] );
		qglRasterPos2f( 25 / m_fScale, -15 / m_fScale );
		gtk_glwidget_print_char( g_AxisName[nDim1] );

		qglColor3fv( g_qeglobals.d_savedinfo.AxisColors[nDim2] );
		qglRasterPos2f( m_vOrigin[nDim1] - w + 30 / m_fScale, m_vOrigin[nDim2] + h - 30 / m_fScale );
		gtk_glwidget_print_char( g_AxisName[nDim2] );
		qglRasterPos2f( -10 / m_fScale, 20 / m_fScale );
		gtk_glwidget_print_char( g_AxisName[nDim2] );

	}

	// show current work zone?
	// the work zone is used to place dropped points and brushes
	if ( g_qeglobals.d_show_work ) {
		qglColor3f( 1.0f, 0.0f, 0.0f );
		qglBegin( GL_LINES );
		qglVertex2f( xb, g_qeglobals.d_work_min[nDim2] );
		qglVertex2f( xe, g_qeglobals.d_work_min[nDim2] );
		qglVertex2f( xb, g_qeglobals.d_work_max[nDim2] );
		qglVertex2f( xe, g_qeglobals.d_work_max[nDim2] );
		qglVertex2f( g_qeglobals.d_work_min[nDim1], yb );
		qglVertex2f( g_qeglobals.d_work_min[nDim1], ye );
		qglVertex2f( g_qeglobals.d_work_max[nDim1], yb );
		qglVertex2f( g_qeglobals.d_work_max[nDim1], ye );
		qglEnd();
	}
}

/*
   ==============
   XY_DrawBlockGrid
   ==============
 */
void XYWnd::XY_DrawBlockGrid(){
	const char *value = ValueForKey( world_entity, "_blocksize" );
	if ( strlen( value ) ) {
		sscanf( value, "%i", &g_qeglobals.blockSize );
	}

	if ( !g_qeglobals.blockSize || g_qeglobals.blockSize > 65536 || g_qeglobals.blockSize < 1024 ) {
		// don't use custom blocksize if it is less than the default, or greater than the maximum world coordinate
		g_qeglobals.blockSize = 1024;
	}

	float x, y, xb, xe, yb, ye;
	float w, h;
	char text[32];

	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_TEXTURE_1D );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );

	w = ( m_nWidth / 2 / m_fScale );
	h = ( m_nHeight / 2 / m_fScale );

	int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( m_nViewType == XY ) ? 1 : 2;

	xb = m_vOrigin[nDim1] - w;
	if ( xb < region_mins[nDim1] ) {
		xb = region_mins[nDim1];
	}
	xb = g_qeglobals.blockSize * floor( xb / g_qeglobals.blockSize );

	xe = m_vOrigin[nDim1] + w;
	if ( xe > region_maxs[nDim1] ) {
		xe = region_maxs[nDim1];
	}
	xe = g_qeglobals.blockSize * ceil( xe / g_qeglobals.blockSize );

	yb = m_vOrigin[nDim2] - h;
	if ( yb < region_mins[nDim2] ) {
		yb = region_mins[nDim2];
	}
	yb = g_qeglobals.blockSize * floor( yb / g_qeglobals.blockSize );

	ye = m_vOrigin[nDim2] + h;
	if ( ye > region_maxs[nDim2] ) {
		ye = region_maxs[nDim2];
	}
	ye = g_qeglobals.blockSize * ceil( ye / g_qeglobals.blockSize );

	// draw major blocks

	qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK] );
	qglLineWidth( 2 );

	qglBegin( GL_LINES );

	for ( x = xb ; x <= xe ; x += g_qeglobals.blockSize )
	{
		qglVertex2f( x, yb );
		qglVertex2f( x, ye );
	}

	if ( m_nViewType == XY ) {
		for ( y = yb ; y <= ye ; y += g_qeglobals.blockSize )
		{
			qglVertex2f( xb, y );
			qglVertex2f( xe, y );
		}
	}

	qglEnd();
	qglLineWidth( 1 );

	// draw coordinate text if needed

	if ( m_nViewType == XY && m_fScale > .1 ) {
		for ( x = xb ; x < xe ; x += g_qeglobals.blockSize )
			for ( y = yb ; y < ye ; y += g_qeglobals.blockSize )
			{
				qglRasterPos2f( x + ( g_qeglobals.blockSize / 2 ), y + ( g_qeglobals.blockSize / 2 ) );
				sprintf( text, "%i,%i",(int)floor( x / g_qeglobals.blockSize ), (int)floor( y / g_qeglobals.blockSize ) );
				gtk_glwidget_print_string( text );
			}
	}

	qglColor4f( 0, 0, 0, 0 );
}

void XYWnd::DrawRotateIcon(){
	float x, y, a, b;

	a = 4.0 / m_fScale; // compensate for zoom level
	b = 6.0 / m_fScale;

	if ( m_nViewType == XY ) {
		x = g_vRotateOrigin[0];
		y = g_vRotateOrigin[1];
	}
	else if ( m_nViewType == YZ ) {
		x = g_vRotateOrigin[1];
		y = g_vRotateOrigin[2];
	}
	else
	{
		x = g_vRotateOrigin[0];
		y = g_vRotateOrigin[2];
	}

	qglEnable( GL_BLEND );
	qglDisable( GL_TEXTURE_2D );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglDisable( GL_CULL_FACE );
	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	qglColor4f( 0.8f, 0.1f, 0.9f, 0.25f );

	qglBegin( GL_QUADS );
	qglVertex3f( x - a,y - a,0 );
	qglVertex3f( x + a,y - a,0 );
	qglVertex3f( x + a,y + a,0 );
	qglVertex3f( x - a,y + a,0 );
	qglEnd();
	qglDisable( GL_BLEND );

	qglColor4f( 1.0f, 0.2f, 1.0f, 1.f );
	qglBegin( GL_POINTS );
	qglVertex3f( x,y,0 );
	qglEnd();

#if 0
	qglBegin( GL_LINES );
	qglVertex3f( x - b,y + b,0 );
	qglVertex3f( x + b,y + b,0 );
	qglVertex3f( x - b,y - b,0 );
	qglVertex3f( x + b,y - b,0 );
	qglEnd();
#endif

}

void XYWnd::DrawCameraIcon(){
	float x, y, a, fov, box;

	fov = 48 / m_fScale;
	box = 16 / m_fScale;

	if ( m_nViewType == XY ) {
		x = g_pParentWnd->GetCamWnd()->Camera()->origin[0];
		y = g_pParentWnd->GetCamWnd()->Camera()->origin[1];
		a = g_pParentWnd->GetCamWnd()->Camera()->angles[YAW] / 180 * Q_PI;
	}
	else if ( m_nViewType == YZ ) {
		x = g_pParentWnd->GetCamWnd()->Camera()->origin[1];
		y = g_pParentWnd->GetCamWnd()->Camera()->origin[2];
		a = g_pParentWnd->GetCamWnd()->Camera()->angles[PITCH] / 180 * Q_PI;
	}
	else
	{
		x = g_pParentWnd->GetCamWnd()->Camera()->origin[0];
		y = g_pParentWnd->GetCamWnd()->Camera()->origin[2];
		a = g_pParentWnd->GetCamWnd()->Camera()->angles[PITCH] / 180 * Q_PI;
	}

	qglColor3f( 0.0, 0.0, 1.0 );
	qglBegin( GL_LINE_STRIP );
	qglVertex3f( x - box,y,0 );
	qglVertex3f( x,y + ( box / 2 ),0 );
	qglVertex3f( x + box,y,0 );
	qglVertex3f( x,y - ( box / 2 ),0 );
	qglVertex3f( x - box,y,0 );
	qglVertex3f( x + box,y,0 );
	qglEnd();

	qglBegin( GL_LINE_STRIP );
	qglVertex3f( x + fov * cos( a + Q_PI / 4 ), y + fov * sin( a + Q_PI / 4 ), 0 );
	qglVertex3f( x, y, 0 );
	qglVertex3f( x + fov * cos( a - Q_PI / 4 ), y + fov * sin( a - Q_PI / 4 ), 0 );
	qglEnd();

}

void XYWnd::DrawZIcon( void ){
	if ( m_nViewType == XY ) {
		float x = z.origin[0];
		float y = z.origin[1];
		float zdim = 8 / m_fScale;
		qglEnable( GL_BLEND );
		qglDisable( GL_TEXTURE_2D );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		qglDisable( GL_CULL_FACE );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglColor4f( 0.0, 0.0, 1.0, 0.25 );
		qglBegin( GL_QUADS );
		qglVertex3f( x - zdim,y - zdim,0 );
		qglVertex3f( x + zdim,y - zdim,0 );
		qglVertex3f( x + zdim,y + zdim,0 );
		qglVertex3f( x - zdim,y + zdim,0 );
		qglEnd();
		qglDisable( GL_BLEND );

		qglColor4f( 0.0, 0.0, 1.0, 1 );

		qglBegin( GL_LINE_LOOP );
		qglVertex3f( x - zdim,y - zdim,0 );
		qglVertex3f( x + zdim,y - zdim,0 );
		qglVertex3f( x + zdim,y + zdim,0 );
		qglVertex3f( x - zdim,y + zdim,0 );
		qglEnd();

		qglBegin( GL_LINE_STRIP );
		qglVertex3f( x - ( zdim / 2 ),y + ( zdim / 2 ),0 );
		qglVertex3f( x + ( zdim / 2 ),y + ( zdim / 2 ),0 );
		qglVertex3f( x - ( zdim / 2 ),y - ( zdim / 2 ),0 );
		qglVertex3f( x + ( zdim / 2 ),y - ( zdim / 2 ),0 );
		qglEnd();
	}
}

// can be greatly simplified but per usual i am in a hurry
// which is not an excuse, just a fact
void XYWnd::PaintSizeInfo( int nDim1, int nDim2, vec3_t vMinBounds, vec3_t vMaxBounds ){
	const char* g_pDimStrings[] = {"x: %.f", "y: %.f", "z: %.f"};
	const char* g_pOrgStrings[] = {"(x: %.f,  y: %.f)", "(x: %.f,  z: %.f)", "(y: %.f,  z: %.f)"};

	CString g_strDim;

	vec3_t vSize;
	VectorSubtract( vMaxBounds, vMinBounds, vSize );

	qglColor3f( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] * .65,
				g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] * .65,
				g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] * .65 );

	if ( m_nViewType == XY ) {
		qglBegin( GL_LINES );

		qglVertex3f( vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale, 0.0f );
		qglVertex3f( vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f );

		qglVertex3f( vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale, 0.0f );
		qglVertex3f( vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale, 0.0f );

		qglVertex3f( vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale, 0.0f );
		qglVertex3f( vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f );


		qglVertex3f( vMaxBounds[nDim1] + 6.0f  / m_fScale, vMinBounds[nDim2], 0.0f );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2], 0.0f );

		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2], 0.0f );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2], 0.0f );

		qglVertex3f( vMaxBounds[nDim1] + 6.0f  / m_fScale, vMaxBounds[nDim2], 0.0f );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2], 0.0f );

		qglEnd();

		qglRasterPos3f( Betwixt( vMinBounds[nDim1], vMaxBounds[nDim1] ),  vMinBounds[nDim2] - 25.0  / m_fScale, 0.0f );
		g_strDim.Format( g_pDimStrings[nDim1], vSize[nDim1] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( vMaxBounds[nDim1] + 16.0  / m_fScale, Betwixt( vMinBounds[nDim2], vMaxBounds[nDim2] ), 0.0f );
		g_strDim.Format( g_pDimStrings[nDim2], vSize[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( vMinBounds[nDim1] + 4, vMaxBounds[nDim2] + 8 / m_fScale, 0.0f );
		g_strDim.Format( g_pOrgStrings[0], vMinBounds[nDim1], vMaxBounds[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );
	}
	else if ( m_nViewType == XZ ) {
		qglBegin( GL_LINES );

		qglVertex3f( vMinBounds[nDim1], 0, vMinBounds[nDim2] - 6.0f  / m_fScale );
		qglVertex3f( vMinBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale );

		qglVertex3f( vMinBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f  / m_fScale );
		qglVertex3f( vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f  / m_fScale );

		qglVertex3f( vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 6.0f  / m_fScale );
		qglVertex3f( vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f / m_fScale );


		qglVertex3f( vMaxBounds[nDim1] + 6.0f  / m_fScale, 0,vMinBounds[nDim2] );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMinBounds[nDim2] );

		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMinBounds[nDim2] );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMaxBounds[nDim2] );

		qglVertex3f( vMaxBounds[nDim1] + 6.0f  / m_fScale, 0,vMaxBounds[nDim2] );
		qglVertex3f( vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMaxBounds[nDim2] );

		qglEnd();

		qglRasterPos3f( Betwixt( vMinBounds[nDim1], vMaxBounds[nDim1] ), 0, vMinBounds[nDim2] - 20.0  / m_fScale );
		g_strDim.Format( g_pDimStrings[nDim1], vSize[nDim1] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( vMaxBounds[nDim1] + 16.0  / m_fScale, 0, Betwixt( vMinBounds[nDim2], vMaxBounds[nDim2] ) );
		g_strDim.Format( g_pDimStrings[nDim2], vSize[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( vMinBounds[nDim1] + 4, 0, vMaxBounds[nDim2] + 8 / m_fScale );
		g_strDim.Format( g_pOrgStrings[1], vMinBounds[nDim1], vMaxBounds[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );
	}
	else
	{
		qglBegin( GL_LINES );

		qglVertex3f( 0, vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale );
		qglVertex3f( 0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale );

		qglVertex3f( 0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale );
		qglVertex3f( 0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale );

		qglVertex3f( 0, vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale );
		qglVertex3f( 0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale );


		qglVertex3f( 0, vMaxBounds[nDim1] + 6.0f  / m_fScale, vMinBounds[nDim2] );
		qglVertex3f( 0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2] );

		qglVertex3f( 0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2] );
		qglVertex3f( 0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2] );

		qglVertex3f( 0, vMaxBounds[nDim1] + 6.0f  / m_fScale, vMaxBounds[nDim2] );
		qglVertex3f( 0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2] );

		qglEnd();

		qglRasterPos3f( 0, Betwixt( vMinBounds[nDim1], vMaxBounds[nDim1] ),  vMinBounds[nDim2] - 20.0  / m_fScale );
		g_strDim.Format( g_pDimStrings[nDim1], vSize[nDim1] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( 0, vMaxBounds[nDim1] + 16.0  / m_fScale, Betwixt( vMinBounds[nDim2], vMaxBounds[nDim2] ) );
		g_strDim.Format( g_pDimStrings[nDim2], vSize[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );

		qglRasterPos3f( 0, vMinBounds[nDim1] + 4.0, vMaxBounds[nDim2] + 8 / m_fScale );
		g_strDim.Format( g_pOrgStrings[2], vMinBounds[nDim1], vMaxBounds[nDim2] );
		gtk_glwidget_print_string( (char *) g_strDim.GetBuffer() );
	}
}

/*
   ==============
   XY_Draw
   ==============
 */

long g_lCount = 0;
long g_lTotal = 0;
extern void DrawBrushEntityName( brush_t *b );

//#define DBG_SCENEDUMP

void XYWnd::XY_Draw(){
#ifdef DBG_SCENEDUMP
	static time_t s_start = 0; // we use that to dump the selected stuff every 2 seconds
	time_t now;
	time( &now );
	bool bDump;

	if ( ( now - s_start ) > 3 ) {
		bDump = true;
		s_start = now;
		Sys_FPrintf( SYS_WRN, "Starting scene dump\n" );
	}
	else{ bDump = false; }
#endif

	brush_t   *brush;
	float w, h;
	entity_t  *e;
	double start, end;
	double start2, end2;
	vec3_t mins, maxs;
	int drawn, culled;
	int i;

	if ( !active_brushes.next ) {
		return; // not valid yet

	}
	Patch_LODMatchAll(); // spog

	if ( m_bTiming ) {
		start = Sys_DoubleTime();
	}
	//
	// clear
	//
	m_bDirty = false;

	qglViewport( 0, 0, m_nWidth, m_nHeight );
	qglClearColor( g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][0],
				   g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][1],
				   g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][2],0 );

	qglClear( GL_COLOR_BUFFER_BIT );

	//
	// set up viewpoint
	//
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

	// fix GL_INVALID_VALUE error on first time the window is updated (win32)
	if ( w == 0 ) {
		w = 1;
	}

	int nDim1 = ( m_nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( m_nViewType == XY ) ? 1 : 2;
	mins[0] = m_vOrigin[nDim1] - w;
	maxs[0] = m_vOrigin[nDim1] + w;
	mins[1] = m_vOrigin[nDim2] - h;
	maxs[1] = m_vOrigin[nDim2] + h;

	qglOrtho( mins[0], maxs[0], mins[1], maxs[1], g_MinWorldCoord, g_MaxWorldCoord );

	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	//
	// now draw the grid
	//
	XY_DrawGrid();

	//
	// draw block grid
	//
	if ( g_qeglobals.show_blocks ) {
		XY_DrawBlockGrid();
	}

	if ( m_nViewType != XY ) {
		qglPushMatrix();
		if ( m_nViewType == YZ ) {
			qglRotatef( -90,  0, 1, 0 ); // put Z going up
		}
		qglRotatef( -90,  1, 0, 0 ); // put Z going up
	}

	//
	// draw stuff
	//
	qglShadeModel( GL_FLAT );
	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_TEXTURE_1D );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );
	qglDisable( GL_CULL_FACE );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	qglColor3f( 0, 0, 0 );
	qglEnableClientState( GL_VERTEX_ARRAY );

	// Fishman - Add antialiazed points and lines support. 09/15/00
	if ( g_PrefsDlg.m_bAntialiasedPointsAndLines ) {
		qglEnable( GL_BLEND );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglEnable( GL_POINT_SMOOTH );
		qglEnable( GL_LINE_SMOOTH );
	}

	drawn = culled = 0;

	e = world_entity;

	if ( m_bTiming ) {
		start2 = Sys_DoubleTime();
	}

	for ( brush = active_brushes.next ; brush != &active_brushes ; brush = brush->next )
	{
		if ( brush->bFiltered ) {
			continue;
		}

		if ( brush->mins[nDim1] > maxs[0] ||
			 brush->mins[nDim2] > maxs[1] ||
			 brush->maxs[nDim1] < mins[0] ||
			 brush->maxs[nDim2] < mins[1] ) {
			culled++;
			continue; // off screen
		}

		drawn++;

		if ( brush->owner != e && brush->owner ) {
			qglColor3fv( brush->owner->eclass->color );
		}
		else if ( brush->brush_faces->texdef.contents & CONTENTS_DETAIL )
		{
			qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_DETAIL] );
		}
		else
		{
			qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES] );
		}

#ifdef DBG_SCENEDUMP
		if ( bDump ) {
			Sys_FPrintf( SYS_WRN, "Active brush: %p ", brush );
			Sys_FPrintf( SYS_WRN, "owner->eclass: %s\n", brush->owner->eclass->name );
		}
#endif

		Brush_DrawXY( brush, m_nViewType );
	}

	if ( m_bTiming ) {
		end2 = Sys_DoubleTime();
	}

	DrawPathLines();

	//
	// draw pointfile
	//
	//++timo why is the display list broken?
	if ( g_qeglobals.d_pointfile_display_list ) {
		Pointfile_Draw();
	}

	//
	// now draw selected brushes
	//

	if ( RotateMode() ) {
		qglColor3f( 0.8f, 0.1f, 0.9f );
	}
	else
	if ( ScaleMode() ) {
		qglColor3f( 0.1f, 0.8f, 0.1f );
	}
	else{
		qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES] );
	}


	if ( g_PrefsDlg.m_bNoStipple == FALSE ) {
		qglEnable( GL_LINE_STIPPLE );
		qglLineStipple( 3, 0xaaaa );
	}
	qglLineWidth( 2 );

	vec3_t vMinBounds;
	vec3_t vMaxBounds;
	vMinBounds[0] = vMinBounds[1] = vMinBounds[2] = g_MaxWorldCoord;
	vMaxBounds[0] = vMaxBounds[1] = vMaxBounds[2] = g_MinWorldCoord;

	int nSaveDrawn = drawn;
	bool bFixedSize = false;
	for ( brush = selected_brushes.next ; brush != &selected_brushes ; brush = brush->next )
	{
		// spog - added culling of selected brushes in XY window
		if ( brush->mins[nDim1] > maxs[0] ||
			 brush->mins[nDim2] > maxs[1] ||
			 brush->maxs[nDim1] < mins[0] ||
			 brush->maxs[nDim2] < mins[1] ) {
			culled++;
			continue; // off screen
		}
		drawn++;
#ifdef DBG_SCENEDUMP
		if ( bDump ) {
			Sys_FPrintf( SYS_WRN, "Selected brush: %p ", brush );
			Sys_FPrintf( SYS_WRN, "owner->eclass: %s\n", brush->owner->eclass->name );
		}
#endif
		Brush_DrawXY( brush, m_nViewType );

		if ( !bFixedSize ) {
			if ( brush->owner->eclass->fixedsize ) {
				bFixedSize = true;
			}
			if ( g_PrefsDlg.m_bSizePaint ) {
				for ( i = 0; i < 3; i++ )
				{
					if ( brush->mins[i] < vMinBounds[i] ) {
						vMinBounds[i] = brush->mins[i];
					}
					if ( brush->maxs[i] > vMaxBounds[i] ) {
						vMaxBounds[i] = brush->maxs[i];
					}
				}
			}
		}
	}

	if ( g_PrefsDlg.m_bNoStipple == FALSE ) {
		qglDisable( GL_LINE_STIPPLE );
	}
	qglLineWidth( 1 );

	if ( !bFixedSize && !RotateMode() && !ScaleMode() && drawn - nSaveDrawn > 0 && g_PrefsDlg.m_bSizePaint ) {
		PaintSizeInfo( nDim1, nDim2, vMinBounds, vMaxBounds );
	}

	// edge / vertex flags
	if ( g_qeglobals.d_select_mode == sel_vertex ) {
		// brush verts
		qglPointSize( 4 );
		qglColor3f( 0,1,0 );
		qglBegin( GL_POINTS );
		for ( i = 0 ; i < g_qeglobals.d_numpoints ; i++ )
			qglVertex3fv( g_qeglobals.d_points[i] );
		qglEnd();

		if ( g_qeglobals.d_num_move_points ) {
			// selected brush verts
			qglPointSize( 5 );
			qglColor3f( 0,0,1 );
			qglBegin( GL_POINTS );
			for ( i = 0; i < g_qeglobals.d_num_move_points; i++ )
				qglVertex3fv( g_qeglobals.d_move_points[i] );
			qglEnd();
		}
		qglPointSize( 1 );
	}
	else if ( g_qeglobals.d_select_mode == sel_edge ) {
		float   *v1, *v2;
		qglPointSize( 4 );
		qglColor3f( 0,0,1 );
		qglBegin( GL_POINTS );
		for ( i = 0 ; i < g_qeglobals.d_numedges ; i++ )
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];
			qglVertex3f( ( v1[0] + v2[0] ) * 0.5,( v1[1] + v2[1] ) * 0.5,( v1[2] + v2[2] ) * 0.5 );
		}
		qglEnd();
		qglPointSize( 1 );
	}

	if ( !( m_nViewType == XY ) ) {
		qglPopMatrix();
	}
#if 0
	// area selection hack
	if ( ( g_qeglobals.d_select_mode == sel_area || g_qeglobals.d_select_mode == sel_areatall ) && ( g_nPatchClickedView == ( ( m_nViewType == XY ) ? W_XY : ( m_nViewType == YZ ) ? W_YZ : W_XZ ) ) ) {
		qglEnable( GL_BLEND );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglColor4f( 0.0, 0.0, 1.0, 0.25 );
		qglRectf( g_qeglobals.d_vAreaTL[nDim1], g_qeglobals.d_vAreaTL[nDim2], g_qeglobals.d_vAreaBR[nDim1], g_qeglobals.d_vAreaBR[nDim2] );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglDisable( GL_BLEND );
	}
#endif


	//
	// now draw camera point
	//
	DrawCameraIcon();
	DrawZIcon();

	if ( RotateMode() ) {
		DrawRotateIcon();
	}

	// plugin entities
	//++timo TODO: use an object for the 2D view
	Draw2DPluginEntities( (VIEWTYPE)m_nViewType );

	if ( g_qeglobals.d_savedinfo.show_outline ) {
		if ( Active() ) {
			qglMatrixMode( GL_PROJECTION );
			qglPushMatrix();
			qglLoadIdentity();
			qglOrtho( 0, m_nWidth, 0, m_nHeight, 0, 1 );
			qglMatrixMode( GL_MODELVIEW );
			qglPushMatrix();
			qglLoadIdentity();

			// four view mode doesn't colorize
			if ( g_pParentWnd->CurrentStyle() == MainFrame::eSplit ) {
				qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME] );
			}
			else{
				qglColor3fv( g_qeglobals.d_savedinfo.AxisColors[m_nViewType] );
			}
			qglBegin( GL_LINE_LOOP );
			qglVertex2i( 0, 0 );
			qglVertex2i( m_nWidth - 1, 0 );
			qglVertex2i( m_nWidth - 1, m_nHeight - 1 );
			qglVertex2i( 0, m_nHeight - 1 );
			qglEnd();

			qglMatrixMode( GL_PROJECTION );
			qglPopMatrix();
			qglMatrixMode( GL_MODELVIEW );
			qglPopMatrix();
		}
	}

	qglFinish();

	if ( m_bTiming ) {
		end = Sys_DoubleTime();
		i = (int)( 1000 * ( end - start ) );
		int i3 = (int)( 1000 * ( end2 - start2 ) );
		g_lCount++;
		g_lTotal += i;
		int i2 = g_lTotal / g_lCount;
		Sys_Printf( "xy: %i ab: %i  avg: %i\n", i, i3, i2 );
	}

	// Fishman - Add antialiazed points and lines support. 09/03/00
	if ( g_PrefsDlg.m_bAntialiasedPointsAndLines ) {
		qglDisable( GL_POINT_SMOOTH );
		qglDisable( GL_LINE_SMOOTH );
		qglDisable( GL_BLEND );
	}
}

void XYWnd::Copy(){
}

void XYWnd::Undo(){
}

void XYWnd::UndoClear(){
}

void XYWnd::UndoCopy(){
}

bool XYWnd::UndoAvailable(){
	return ( g_brUndo.next != &g_brUndo );
}

void XYWnd::Paste(){
}

// should be static as should be the rotate scale stuff
bool XYWnd::AreaSelectOK(){
	return RotateMode() ? false : ScaleMode() ? false : true;
}

void XYWnd::OnCreate(){
	if ( !MakeCurrent() ) {
		Error( "xywindow: glXMakeCurrent failed" );
	}

	qglPolygonStipple( (unsigned char *)s_stipple );
	qglLineStipple( 3, 0xaaaa );
}

void XYWnd::OnExpose(){
	bool bPaint = true;
	if ( !MakeCurrent() ) {
		Sys_FPrintf( SYS_ERR, "ERROR: glXMakeCurrent failed.. Error:%i\n",qglGetError() );
		Sys_Printf( "Please restart Radiant if the Map view is not working\n" );
		bPaint = false;
	}
	if ( bPaint ) {
		QE_CheckOpenGLForErrors();
		XY_Draw();
		QE_CheckOpenGLForErrors();

		if ( m_nViewType != XY ) {
			qglPushMatrix();
			if ( m_nViewType == YZ ) {
				qglRotatef( -90,  0, 1, 0 ); // put Z going up
			}
			qglRotatef( -90,  1, 0, 0 ); // put Z going up
		}

		if ( g_bCrossHairs ) {
			qglColor4f( 0.2f, 0.9f, 0.2f, 0.8f );
			qglBegin( GL_LINES );
			if ( m_nViewType == XY ) {
				qglVertex2f( 2 * g_MinWorldCoord, tdp[1] );
				qglVertex2f( 2 * g_MaxWorldCoord, tdp[1] );
				qglVertex2f( tdp[0], 2 * g_MinWorldCoord );
				qglVertex2f( tdp[0], 2 * g_MaxWorldCoord );
			}
			else if ( m_nViewType == YZ ) {
				qglVertex3f( tdp[0], 2 * g_MinWorldCoord, tdp[2] );
				qglVertex3f( tdp[0], 2 * g_MaxWorldCoord, tdp[2] );
				qglVertex3f( tdp[0], tdp[1], 2 * g_MinWorldCoord );
				qglVertex3f( tdp[0], tdp[1], 2 * g_MaxWorldCoord );
			}
			else
			{
				qglVertex3f( 2 * g_MinWorldCoord, tdp[1], tdp[2] );
				qglVertex3f( 2 * g_MaxWorldCoord, tdp[1], tdp[2] );
				qglVertex3f( tdp[0], tdp[1], 2 * g_MinWorldCoord );
				qglVertex3f( tdp[0], tdp[1], 2 * g_MaxWorldCoord );
			}
			qglEnd();
		}

		if ( ClipMode() ) {
			// Draw clip points
			if ( g_Clip1.Set() ) {
				g_Clip1.Draw( m_fScale, 1 ); // qglVertex3fv (g_Clip1);
			}
			if ( g_Clip2.Set() ) {
				g_Clip2.Draw( m_fScale, 2 ); // qglVertex3fv (g_Clip2);
			}
			if ( g_Clip3.Set() ) {
				g_Clip3.Draw( m_fScale, 3 ); // qglVertex3fv (g_Clip3);
			}
			if ( g_Clip1.Set() && g_Clip2.Set() ) {
				ProduceSplitLists();
				brush_t* pBrush;
				brush_t* pList = ( g_bSwitch ) ? &g_brBackSplits : &g_brFrontSplits;
				for ( pBrush = pList->next ; pBrush != NULL && pBrush != pList ; pBrush = pBrush->next )
				{
					qglColor3f( 1,1,0 );
					face_t *face;
					int order;
					for ( face = pBrush->brush_faces,order = 0 ; face ; face = face->next, order++ )
					{
						winding_t* w = face->face_winding;
						if ( !w ) {
							continue;
						}
						// draw the polygon
						qglBegin( GL_LINE_LOOP );
						for ( int i = 0 ; i < w->numpoints ; i++ )
							qglVertex3fv( w->points[i] );
						qglEnd();
					}
				}
			}
		}

		if ( PathMode() ) {
			int n;
			for ( n = 0; n < g_nPathCount; n++ )
				g_PathPoints[n].Draw( m_fScale, n + 1 );  // qglVertex3fv(g_PathPoints[n]);
		}
		if ( m_nViewType != XY ) {
			qglPopMatrix();
		}

		m_XORRectangle.set( rectangle_t() );
		SwapBuffers();
	}
}

void XYWnd::KillPathMode(){
	g_bSmartGo = false;
	g_bPathMode = false;
	if ( g_pPathFunc ) {
		g_pPathFunc( false, g_nPathCount );
	}
	g_nPathCount = 0;
	g_pPathFunc = NULL;
	Sys_UpdateWindows( W_ALL );
}

// gets called for drop down menu messages
// TIP: it's not always about EntityCreate
void XYWnd::OnEntityCreate( const char* item ){
	Undo_Start( "create entity" );
	Undo_AddBrushList( &selected_brushes );

	if ( m_mnuDrop != NULL ) {
		CString strItem;
		strItem = item;

		if ( strItem.CompareNoCase( "Add to..." ) == 0 ) {
			//++timo TODO: fill the menu with current groups?
			// this one is for adding to existing groups only
			Sys_Printf( "TODO: Add to... in XYWnd::OnEntityCreate\n" );
		}
		else if ( strItem.CompareNoCase( "Remove" ) == 0 ) {
			// remove selected brushes from their current group
			brush_t *b;
			for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
			{

			}
		}

		//++timo FIXME: remove when all hooks are in
		if ( strItem.CompareNoCase( "Add to..." ) == 0
			 || strItem.CompareNoCase( "Remove" ) == 0
			 || strItem.CompareNoCase( "Name..." ) == 0
			 || strItem.CompareNoCase( "New group..." ) == 0 ) {
			Sys_Printf( "TODO: hook drop down group menu\n" );
			return;
		}

		if ( strItem.Find( "Smart_" ) >= 0 ) {
			CreateSmartEntity( this, m_ptDownX, m_ptDownY, strItem );
		}
		else
		{
			CreateRightClickEntity( this, m_ptDownX, m_ptDownY, (char*)strItem.GetBuffer() );
		}

		Sys_UpdateWindows( W_ALL );
		//OnLButtonDown((MK_LBUTTON | MK_SHIFT), CPoint(m_ptDown.x+2, m_ptDown.y+2));
	}
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
}

/* Drawing clip points */
void ClipPoint::Draw( float fScale, int num ){
	CString strLabel;
	strLabel.Format( "%d", num );
	Draw( fScale, strLabel.GetBuffer() );
}

#define ALT_POINT_VERTS 6

void ClipPoint::Draw( float fScale, const char *label ){
	// draw point
	qglPointSize( 4 );
	qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER] );
	qglBegin( GL_POINTS );
	qglVertex3fv( m_ptClip );
	qglEnd();
	qglPointSize( 1 );

	// draw label
	qglRasterPos3f( m_ptClip[0] + 2, m_ptClip[1] + 2, m_ptClip[2] + 2 );
	qglCallLists( strlen( label ), GL_UNSIGNED_BYTE, label );
}
