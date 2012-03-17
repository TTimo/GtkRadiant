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

//-----------------------------------------------------------------------------
//
// DESCRIPTION
// This contains functions specific to the UI toolkit
//   it is best to avoid using them, but they are provided for backward compatibility with the older interfaces
//   the abstracted UI layer in iUI.h is not sufficient for some tasks .. no other choice than to rely on UI specific code

#ifndef __IGL_GTK_H__
#define __IGL_GTK_H__

#define UIGTK_MAJOR "uigtk"

// All OpenGL stuff is handled by GLWidget to ensure portability
typedef GtkWidget* ( WINAPI * PFN_QERAPP_GETQEGLOBALSGLWIDGET )();
typedef GtkWidget* ( WINAPI * PFN_GLWIDGET_NEW )( gboolean zbufffer, GtkWidget* share );
typedef void ( WINAPI * PFN_GLWIDGET_SWAPBUFFERS )( GtkWidget* widget );
typedef gboolean ( WINAPI * PFN_GLWIDGET_MAKECURRENT )( GtkWidget* widget );
typedef void ( WINAPI * PFN_GLWIDGET_DESTROYCONTEXT )( GtkWidget* widget );
typedef void ( WINAPI * PFN_GLWIDGET_CREATECONTEXT )( GtkWidget* widget );
#if 0
typedef gpointer ( WINAPI * PFN_GLWIDGET_GETCONTEXT )( GtkWidget* widget );
#endif

struct _QERUIGtkTable
{
	int m_nSize;
	PFN_QERAPP_GETQEGLOBALSGLWIDGET m_pfn_GetQeglobalsGLWidget;
	PFN_GLWIDGET_NEW m_pfn_glwidget_new;
	PFN_GLWIDGET_SWAPBUFFERS m_pfn_glwidget_swap_buffers;
	PFN_GLWIDGET_MAKECURRENT m_pfn_glwidget_make_current;
	PFN_GLWIDGET_DESTROYCONTEXT m_pfn_glwidget_destroy_context;
	PFN_GLWIDGET_CREATECONTEXT m_pfn_glwidget_create_context;
#if 0
	PFN_GLWIDGET_GETCONTEXT m_pfn_glwidget_get_context;
#endif
};

#endif
