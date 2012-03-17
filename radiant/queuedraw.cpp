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
// Try to sort the faces by texture and make rendering faster
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"

typedef struct
{
	qtexture_t* texture;
	GPtrArray* faces;
} windingsort_t;

static windingsort_t* sort;
static guint32 alloc, len;
static GPtrArray* notex_faces;

void QueueClear(){
	len = 0;

	if ( notex_faces == NULL ) {
		notex_faces = g_ptr_array_new();
	}
	g_ptr_array_set_size( notex_faces, 0 );
}

void QueueFace( face_t *face ){
	guint32 i;

	if ( face->d_texture->name[0] == '(' ) {
		g_ptr_array_add( notex_faces, face );
		return;
	}

	for ( i = 0; i < len; i++ )
		if ( sort[i].texture == face->d_texture ) {
			g_ptr_array_add( sort[i].faces, face );
			return;
		}

	if ( len == alloc ) {
		alloc += 8;
		sort = (windingsort_t*)realloc( sort, alloc * sizeof( windingsort_t ) );

		for ( i = len; i < alloc; i++ )
			sort[i].faces = g_ptr_array_new();
	}
	g_ptr_array_set_size( sort[len].faces, 0 );
	g_ptr_array_add( sort[len].faces, face );
	sort[len].texture = face->d_texture;
	len++;
}

void QueueDraw(){
	guint32 i, k;
	face_t *face;
	winding_t *w;
	int j, nDrawMode = g_pParentWnd->GetCamera().draw_mode;

	if ( notex_faces->len ) {
		qglDisable( GL_TEXTURE_2D );

		for ( i = 0; i < notex_faces->len; i++ )
		{
			face = (face_t*)notex_faces->pdata[i];
			w = face->face_winding;

			qglBegin( GL_POLYGON );

			/*
			   if (b->patchBrush)
			   //++timo FIXME: find a use case for this??
			   qglColor4f (face->d_color[0], face->d_color[1], face->d_color[2], 0.13);
			   else
			 */
			qglColor4f( face->d_color[0], face->d_color[1], face->d_color[2], face->pShader->getTrans() );

			if ( g_PrefsDlg.m_bGLLighting ) {
				qglNormal3fv( face->plane.normal );
			}

			for ( j = 0; j < w->numpoints; j++ )
			{
				if ( nDrawMode == cd_texture || nDrawMode == cd_light ) {
					qglTexCoord2fv( &w->points[j][3] );
				}
				qglVertex3fv( w->points[j] );
			}

			qglEnd();
		}
	}

	if ( !len ) {
		return;
	}

	if ( nDrawMode == cd_texture || nDrawMode == cd_light ) {
		qglEnable( GL_TEXTURE_2D );
	}

	for ( k = 0; k < len; k++ )
	{
		qglBindTexture( GL_TEXTURE_2D, sort[k].texture->texture_number );

		for ( i = 0; i < sort[k].faces->len; i++ )
		{
			face = (face_t*)sort[k].faces->pdata[i];
			w = face->face_winding;

			qglBegin( GL_POLYGON );
			/*
			   if (b->patchBrush)
			   //++timo FIXME: find a use case for this??
			   qglColor4f (face->d_color[0], face->d_color[1], face->d_color[2], 0.13);
			   else
			 */
			qglColor4f( face->d_color[0], face->d_color[1], face->d_color[2], face->pShader->getTrans() );

			if ( g_PrefsDlg.m_bGLLighting ) {
				qglNormal3fv( face->plane.normal );
			}

			for ( j = 0; j < w->numpoints; j++ )
			{
				if ( nDrawMode == cd_texture || nDrawMode == cd_light ) {
					qglTexCoord2fv( &w->points[j][3] );
				}
				qglVertex3fv( w->points[j] );
			}

			qglEnd();
		}
	}
	qglBindTexture( GL_TEXTURE_2D, 0 );
}
