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
// bkgrnd2d Plugin
//
// Code by reyalP aka Reed Mideke
//
// Based on various other plugins
//

#include "bkgrnd2d.h"

CBackgroundRender render;

CBackgroundImage backgroundXY( XY ),backgroundXZ( XZ ),backgroundYZ( YZ );

CBackgroundRender::CBackgroundRender(){
	refCount = 1;
}

CBackgroundRender::~CBackgroundRender(){
}

void CBackgroundRender::Register(){
	g_QglTable.m_pfnHookGL2DWindow( this );
}

void CBackgroundRender::Draw2D( VIEWTYPE vt ){
	switch ( vt )
	{
	case XY:
		backgroundXY.Render();
		break;
	case XZ:
		backgroundXZ.Render();
		break;
	case YZ:
		backgroundYZ.Render();
		break;
	}
}


CBackgroundImage::CBackgroundImage( VIEWTYPE vt ){
	m_tex = NULL;
	m_alpha = 0.5;

	// TODO, sensible defaults ? Or not show until we have extents ?
	m_xmin = m_ymin = 0.0f;
	m_xmax = m_ymax = 0.0f;

	m_bActive = false;

	m_vt = vt;

	switch ( m_vt )
	{
	case XY:
		m_ix = 0;
		m_iy = 1;
		break;
	case XZ:
		m_ix = 0;
		m_iy = 2;
		break;
	case YZ:
		m_ix = 1;
		m_iy = 2;
		break;
	}
}

/*
 * should cleanup, but I don't think we can be sure it happens before our
 * interfaces are gone
   CBackgroundImage::~CBackgroundImage()
   {
   }
 */

void CBackgroundImage::Cleanup(){
	if ( m_tex ) {
		g_QglTable.m_pfn_qglDeleteTextures( 1,&m_tex->texture_number );
		g_free( m_tex );
		m_tex = NULL;
	}
}

void CBackgroundImage::Render(){
	if ( !m_bActive || !Valid() ) {
		return;
	}
	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglEnable( GL_TEXTURE_2D );
	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	g_QglTable.m_pfn_qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	g_QglTable.m_pfn_qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	g_QglTable.m_pfn_qglPolygonMode( GL_FRONT,GL_FILL );
	// TODO, just so we can tell if we end up going the wrong way
	// g_QglTable.m_pfn_qglPolygonMode(GL_BACK,GL_LINE);
	// TODO any other state we should not assume ?

	g_QglTable.m_pfn_qglBindTexture( GL_TEXTURE_2D, m_tex->texture_number );
	g_QglTable.m_pfn_qglBegin( GL_QUADS );

	g_QglTable.m_pfn_qglColor4f( 1.0,1.0,1.0,m_alpha );
	g_QglTable.m_pfn_qglTexCoord2f( 0.0,1.0 );
	g_QglTable.m_pfn_qglVertex2f( m_xmin,m_ymin );

	g_QglTable.m_pfn_qglTexCoord2f( 1.0,1.0 );
	g_QglTable.m_pfn_qglVertex2f( m_xmax,m_ymin );

	g_QglTable.m_pfn_qglTexCoord2f( 1.0,0.0 );
	g_QglTable.m_pfn_qglVertex2f( m_xmax,m_ymax );

	g_QglTable.m_pfn_qglTexCoord2f( 0.0,0.0 );
	g_QglTable.m_pfn_qglVertex2f( m_xmin,m_ymax );

	g_QglTable.m_pfn_qglEnd();
	g_QglTable.m_pfn_qglBindTexture( GL_TEXTURE_2D, 0 );

	g_QglTable.m_pfn_qglPopAttrib();
}

bool CBackgroundImage::Load( const char *filename ){
	qtexture_t *newtex;

	unsigned char *image = NULL; // gets allocated with what ? g_malloc
	int width = 0, height = 0;

	g_FuncTable.m_pfnLoadImage( filename,&image,&width,&height );

	if ( !image ) {
		Syn_Printf( MSG_WARN "load %s failed\n",filename );
		return false;
	}

// just in case we want to build for an old version
#ifdef BKGRND2D_JPG_WORKAROUND
	if ( strlen( filename ) > 4 && !strcmp( ".jpg",filename + strlen( filename ) - 4 ) ) {
		Syn_Printf( MSG_PREFIX ".jpg workaround, clearing alpha channel\n" );
		int size = width * height * 4;
		int i;
		for ( i = 3; i < size; i += 4 ) {
			image[i] = 255;
		}
	}
#endif

	//TODO bug for stored texture size
	//TODO whose gl context are we in, anyway ?
	newtex = g_FuncTable.m_pfnLoadTextureRGBA( image,width,height );

	g_free( image );

	if ( !newtex ) {
		Syn_Printf( MSG_WARN "image to texture failed\n" );
		return false;
	}

	Cleanup();
	m_tex = newtex;

	g_FuncTable.m_pfnSysUpdateWindows( W_XY );

	return true;
}

bool CBackgroundImage::SetExtentsMM(){
	entity_s *worldentity;
	const char *val;
	int xmin = 0, ymin = 0, xmax = 0, ymax = 0;

	worldentity = (entity_s *)g_FuncTable.m_pfnGetEntityHandle( 0 );
	if ( !worldentity ) {
		Syn_Printf( MSG_WARN "SetExtentsMM worldspawn not found\n" );
		return false;
	}
	//TODO val is not NULL even if key does not exist
	val = g_EntityTable.m_pfnValueForKey( worldentity,"mapcoordsmins" );
	if ( !val || !val[0] ) {
		Syn_Printf( MSG_WARN "SetExtentsMM mapcoordsmins not found\n" );
		return false;
	}
// we could be more robust
// note contortions due to splashs strange idea of min and max
	if ( sscanf( val, "%d %d",&xmin,&ymax ) != 2 ) {
		Syn_Printf( MSG_WARN "SetExtentsMM mapcoordsmins malformed\n" );
		return false;
	}

	val = g_EntityTable.m_pfnValueForKey( worldentity,"mapcoordsmaxs" );
	if ( !val || !val[0] ) {
		Syn_Printf( MSG_WARN "SetExtentsMM mapcoordsmaxs not found\n" );
		return false;
	}
	if ( sscanf( val, "%d %d",&xmax,&ymin ) != 2 ) {
		Syn_Printf( MSG_WARN "SetExtentsMM mapcoordsmaxs malformed\n" );
		return false;
	}
	//might do sanity check before we commit
	m_xmin = (float)xmin;
	m_ymin = (float)ymin;
	m_xmax = (float)xmax;
	m_ymax = (float)ymax;

	g_FuncTable.m_pfnSysUpdateWindows( W_XY );
	return true;
}

// TODO, this should just be exported from core
// ripped directly from radiant/select.cpp:Select_GetBounds
//
static bool get_selection_bounds( vec3_t mins, vec3_t maxs ){
	brush_t *b;
	int i;
	brush_t *selected_brushes = g_DataTable.m_pfnSelectedBrushes();
	//TODO should never happen
	if ( !selected_brushes ) {
		Sys_Printf( MSG_PREFIX "selected_brushes = NULL\n" );
		return false;
	}
	// this should mean no selection
	if ( selected_brushes == selected_brushes->next ) {
		Sys_Printf( MSG_PREFIX "nothing selected\n" );

		return false;
	}

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = 99999;
		maxs[i] = -99999;
	}

	for ( b = selected_brushes->next ; b != selected_brushes ; b = b->next )
	{
		if ( b->owner->eclass->fixedsize ) {
			for ( i = 0 ; i < 3 ; i++ )
			{
				if ( b->owner->origin[i] < mins[i] ) {
					mins[i] = b->owner->origin[i];
				}
				if ( b->owner->origin[i] > maxs[i] ) {
					maxs[i] = b->owner->origin[i];
				}
			}
		}
		else
		{
			for ( i = 0 ; i < 3 ; i++ )
			{
				if ( b->mins[i] < mins[i] ) {
					mins[i] = b->mins[i];
				}
				if ( b->maxs[i] > maxs[i] ) {
					maxs[i] = b->maxs[i];
				}
			}
		}
	}
	return true;
}

bool CBackgroundImage::SetExtentsSel(){
	vec3_t mins,maxs;

	if ( !get_selection_bounds( mins,maxs ) ) {
		return false;
	}

	if ( ( (int)mins[m_ix] == (int)maxs[m_ix] ) ||
		 ( (int)mins[m_iy] == (int)maxs[m_iy] ) ) {
		Syn_Printf( MSG_PREFIX "tiny selection\n" );
		return false;
	}

	m_xmin = mins[m_ix];
	m_ymin = mins[m_iy];
	m_xmax = maxs[m_ix];
	m_ymax = maxs[m_iy];

	g_FuncTable.m_pfnSysUpdateWindows( W_XY );

	return true;
}
