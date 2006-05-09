/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

#include "error.h"

#include "debugging/debugging.h"
#include "igl.h"

#include "gtkutil/messagebox.h"
#include "console.h"
#include "preferences.h"


#ifdef WIN32
#define UNICODE
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#endif



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

void Error (const char *error, ...)
{
  va_list argptr;
  char	text[4096];

  va_start (argptr,error);
  vsprintf (text, error,argptr);
  va_end (argptr);

  strcat( text, "\n" );

#ifdef WIN32
  if (GetLastError() != 0)
  {
    LPVOID lpMsgBuf;
    FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      0,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0,
      0 
      );
    strcat( text, "GetLastError: " );
    /*
    Gtk will only crunch 0<=char<=127
    this is a bit hackish, but I didn't find useful functions in win32 API for this
    http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=516
    */
    TCHAR *scan, *next = (TCHAR*)lpMsgBuf;
    do
    {
      scan = next;
      text[strlen(text)+1] = '\0';
      if ((scan[0] >= 0) && (scan[0] <= 127))
        text[strlen(text)] = char(scan[0]);
      else
        text[strlen(text)] = '?';
      next = CharNext(scan);
    } while (next != scan);
    strcat( text, "\n");
    LocalFree( lpMsgBuf );
  }
#else
  if (errno != 0)
  {
    strcat( text, "errno: " );
    strcat( text, strerror (errno));
    strcat( text, "\n");
  }
#endif


#if 0
  // we need to have a current context to call glError()
  if (g_glwindow_globals.d_glBase != 0)
  {
    // glGetError .. can record several errors, clears after calling
    //++timo TODO: be able to deal with several errors if necessary, for now I'm just warning about pending error messages
    // NOTE: forget that, most boards don't seem to follow the OpenGL standard
    GLenum iGLError = glGetError();
    if (iGLError != GL_NO_ERROR)
    {
      // use our own gluErrorString
      strcat( text, "gluErrorString: " );
      strcat( text, (char*)gluErrorString( iGLError ) );
      strcat( text, "\n" );
    }
  }
#endif

  strcat (text, "An unrecoverable error has occured.\n");

  ERROR_MESSAGE(text);

  // force close logging if necessary
	Sys_LogFile(false);

  _exit (1);
}
