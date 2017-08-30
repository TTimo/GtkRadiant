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

#define UNICODE
#include "stdafx.h"

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <unistd.h>
#endif

#include <glib/gi18n.h>

/*
   =================
   Error

   For abnormal program terminations
   =================
 */

/*!
   \todo
   FIXME the prompt wether to do prefs dialog, may not even be possible
   if the crash happens before the game is loaded
 */

void Error( const char *error, ... ){
	va_list argptr;
	char text[4096];

	va_start( argptr,error );
	vsprintf( text, error,argptr );
	va_end( argptr );

	strcat( text, "\n" );

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	if ( errno != 0 ) {
		strcat( text, "errno: " );
		strcat( text, strerror( errno ) );
		strcat( text, "\n" );
	}
#endif

#ifdef _WIN32
	if ( GetLastError() != 0 ) {
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			);
		strcat( text, "GetLastError: " );
		/*
		   Gtk will only crunch 0<=char<=127
		   this is a bit hackish, but I didn't find useful functions in win32 API for this
		 */
		TCHAR *scan, *next = (TCHAR*)lpMsgBuf;
		do
		{
			scan = next;
			text[strlen( text ) + 1] = '\0';
			if ( ( scan[0] >= 0 ) && ( scan[0] <= 127 ) ) {
				text[strlen( text )] = scan[0];
			}
			else{
				text[strlen( text )] = '?';
			}
			next = CharNext( scan );
		} while ( next != scan );
		strcat( text, "\n" );
		LocalFree( lpMsgBuf );
	}
#endif

	// we need to have a current context to call glError()
	if ( g_qeglobals_gui.d_glBase != NULL ) {
		// qglGetError .. can record several errors, clears after calling
		//++timo TODO: be able to deal with several errors if necessary, for now I'm just warning about pending error messages
		// NOTE: forget that, most boards don't seem to follow the OpenGL standard
		GLenum iGLError = qglGetError();
		if ( iGLError != GL_NO_ERROR ) {
			// use our own gluErrorString
			strcat( text, "qgluErrorString: " );
			strcat( text, (char*)qgluErrorString( iGLError ) );
			strcat( text, "\n" );
		}
	}

	strcat( text, _( "An unrecoverable error has occured.\n"
				  "Would you like to edit Preferences before exiting Radiant?" ) );

	Sys_Printf( text );

	if ( gtk_MessageBox( NULL, text, _( "Error" ), MB_YESNO ) == IDYES ) {
		Sys_Printf( "Doing prefs..\n" );
		g_PrefsDlg.LoadPrefs();
		g_PrefsDlg.DoModal();
	}

	QGL_Shutdown();

	g_PrefsDlg.Destroy();
	g_dlgSurface.Destroy();
	g_dlgFind.Destroy();

	// force close logging if necessary
	g_PrefsDlg.mGamesDialog.m_bLogConsole = false;
	Sys_LogFile();

	_exit( 1 );
}

void WINAPI Error( char *error, ... ){
	va_list argptr;
	char text[1024];

	va_start( argptr,error );
	vsprintf( text, error,argptr );
	va_end( argptr );

	Error( (const char *)text );
}
