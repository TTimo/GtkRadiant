/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#include "glfont.h"
#ifdef _WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>
#include "debugging/debugging.h"

// LordHavoc: this is code for direct Xlib bitmap character fetching, as an
// alternative to requiring gtkglarea, it was created due to a lack of this
// package on SuSE 9.x but this package is now commonly shipping in Linux
// distributions so this code may be unnecessary, feel free however to enable
// it when building packages for distros that do not ship with that package,
// or if you just prefer less dependencies...
#if 0

#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <GL/glx.h>

GLFont glfont_create(const char* font_string)
{
  GLuint font_list_base;
  XFontStruct *fontInfo;
  Display *dpy = GDK_DISPLAY ();
  unsigned int i, first, last, firstrow, lastrow;
  int maxchars;
  int firstbitmap;

  fontInfo = XLoadQueryFont (dpy, "-*-fixed-*-*-*-*-8-*-*-*-*-*-*-*");
  if (fontInfo == NULL)
  {
    // try to load other fonts
    fontInfo = XLoadQueryFont (dpy, "-*-fixed-*-*-*-*-*-*-*-*-*-*-*-*");

    // any font will do !
    if (fontInfo == NULL)
      fontInfo = XLoadQueryFont(dpy, "-*-*-*-*-*-*-*-*-*-*-*-*-*-*");

    if (fontInfo == NULL)
      ERROR_MESSAGE("couldn't create font");
  }

  first = (int)fontInfo->min_char_or_byte2;
  last = (int)fontInfo->max_char_or_byte2;
  firstrow = (int)fontInfo->min_byte1;
  lastrow = (int)fontInfo->max_byte1;
  /*
   * How many chars in the charset
   */
  maxchars = 256 * lastrow + last;
  font_list_base = glGenLists(maxchars+1);
  if (font_list_base == 0)
  {
    ERROR_MESSAGE( "couldn't create font" );
  }

  /*
   * Get offset to first char in the charset
   */
  firstbitmap = 256 * firstrow + first;
  /*
   * for each row of chars, call glXUseXFont to build the bitmaps.
   */

  for(i=firstrow; i<=lastrow; i++)
  {
    glXUseXFont(fontInfo->fid, firstbitmap, last-first+1, font_list_base+firstbitmap);
    firstbitmap += 256;
  }

/*    *height = fontInfo->ascent + fontInfo->descent;
    *width = fontInfo->max_bounds.width;  */
  return GLFont(font_list_base, fontInfo->ascent + fontInfo->descent);
}

void glfont_release(GLFont& font)
{
  glDeleteLists(font.getDisplayList(), 256);
  font = GLFont(0, 0);
}

#else

#include <gtk/gtkglwidget.h>

GLFont glfont_create(const char* font_string)
{
  GLuint font_list_base = glGenLists (256);
  gint font_height = 0;

  PangoFontDescription* font_desc = pango_font_description_from_string (font_string);

  PangoFont* font = gdk_gl_font_use_pango_font (font_desc, 0, 256, font_list_base);

  if(font != 0)
  {
    PangoFontMetrics* font_metrics = pango_font_get_metrics (font, 0);

    font_height = pango_font_metrics_get_ascent (font_metrics) +
                  pango_font_metrics_get_descent (font_metrics);
    font_height = PANGO_PIXELS (font_height);

    pango_font_metrics_unref (font_metrics);
  }

  pango_font_description_free (font_desc);

  // fix for pango/gtkglext metrix bug
  if(font_height > 16)
	  font_height = 16;

  return GLFont(font_list_base, font_height);
}

void glfont_release(GLFont& font)
{
  glDeleteLists(font.getDisplayList(), 256);
  font = GLFont(0, 0);
}
#endif
