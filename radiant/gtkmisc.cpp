/*
Copyright (c) 2001, Loki software, inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Loki software nor the names of its contributors may be used
to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Small functions to help with GTK
//

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#if defined (__linux__) || defined (__APPLE__)
#include <unistd.h>
#endif

#include <gtk/gtk.h>

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



#ifdef _WIN32
#include <io.h>
#include <direct.h>
#define R_OK 04
#endif
#include "stdafx.h"

// =============================================================================
// Misc stuff

// NOTE TTimo window position saving has always been tricky
//   it doesn't work the same between win32 and linux .. see below that code is fairly different
//   it's also very poorly done, the save calls are a bit randomly disctributed in the OnDestroy

void save_window_pos (GtkWidget *wnd, window_position_t& pos)
{
  if ((wnd == NULL) || (wnd->window == NULL))
    return;

  get_window_pos(wnd, &pos.x, &pos.y);

  pos.w = wnd->allocation.width;
  pos.h = wnd->allocation.height;

#ifdef DBG_WINDOWPOS
  //Sys_Printf("save_window_pos 'Window %s'\n",buf);
#endif
}

#ifdef _WIN32
void win32_get_window_pos(GtkWidget *widget, gint *x, gint *y)
{
  if ( g_PrefsDlg.m_bStartOnPrimMon ) {
    RECT rc;
    POINT point;
    HWND xwnd = (HWND)GDK_WINDOW_HWND (widget->window);
    const GdkRectangle primaryMonitorRect = g_pParentWnd->GetPrimaryMonitorRect();

    GetClientRect(xwnd,&rc);
    point.x=rc.left;
    point.y=rc.top;
    ClientToScreen(xwnd,&point);

    *x=point.x;
    *y=point.y;

    *x=max(*x,-widget->allocation.width+10);
    *x=min(*x,primaryMonitorRect.width-10);
    *y=max(*y,-widget->allocation.height+10);
    *y=min(*y,primaryMonitorRect.height-10);
  } else {
    // this is the same as the unix version of get_window_pos
    gdk_window_get_root_origin (widget->window, x, y);
  }
#ifdef DBG_WINDOWPOS
  Sys_Printf("win32_get_window_pos %p %d,%d\n",widget,*x,*y);
#endif
}
#endif

void load_window_pos (GtkWidget *wnd, window_position_t& pos)
{
#ifdef _WIN32
  const GdkRectangle primaryMonitorRect = g_pParentWnd->GetPrimaryMonitorRect();

  if(pos.x < primaryMonitorRect.x
    || pos.y < primaryMonitorRect.y
    || pos.x > primaryMonitorRect.x + primaryMonitorRect.width
    || pos.y > primaryMonitorRect.y + primaryMonitorRect.height)
    gtk_window_set_position(GTK_WINDOW(wnd), GTK_WIN_POS_CENTER_ON_PARENT);
#else
  // FIXME: not multihead safe
  if(pos.x < 0
    || pos.y < 0
    || pos.x > gdk_screen_width ()
    || pos.y > gdk_screen_height ())
    gtk_window_set_position(GTK_WINDOW(wnd), GTK_WIN_POS_CENTER_ON_PARENT);
#endif
  else
    gtk_window_move(GTK_WINDOW(wnd), pos.x, pos.y);

  gtk_window_set_default_size (GTK_WINDOW (wnd), pos.w, pos.h);
#ifdef DBG_WINDOWPOS
  Sys_Printf("load_window_pos %p 'Window,%s'\n",wnd,windowData);
#endif
}

gint widget_delete_hide (GtkWidget *widget)
{
  gtk_widget_hide (widget);

  return TRUE;
}


// Thanks to Mercury, Fingolfin - ETG
int readLongLE(FILE *file, unsigned long *m_bytesRead, int *value)
{
  byte buf[4];
  int len = fread(buf, 4, 1, file);
  *m_bytesRead += 4;
  if (len != 1)
    return -1;

  *value = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
  return 0;
}

short readShortLE(FILE *file, unsigned long *m_bytesRead, short unsigned *value)
{
  byte buf[2];
  int len = fread(buf, 2, 1, file);
  *m_bytesRead += 2;
  if (len != 1)
    return -1;

  *value = buf[0] | buf[1] << 8;
  return 0;
}

unsigned char *load_bitmap_file (const char* filename, guint16 *width, guint16 *height)
{
  int bmWidth, bmHeight;
  short unsigned bmPlanes, bmBitsPixel;
  typedef struct {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
  } RGBQUAD;
  unsigned char m1,m2;
  int sizeimage;
  short unsigned res1,res2;
  int filesize, pixoff;
  int bmisize, compression;
  int xscale, yscale;
  int colors, impcol;
  unsigned long m_bytesRead = 0;
  unsigned char *imagebits = NULL;
  FILE *fp;

  *width = *height = 0;

  fp = fopen(filename,"rb");
  if (fp == NULL)
  {
	return NULL;
  }

  size_t rc;
  rc = fread(&m1, 1, 1, fp);
  m_bytesRead++;
  if (rc == -1)
  {
      fclose(fp);
      return NULL;
  }

  rc = fread(&m2, 1, 1, fp);
  m_bytesRead++;
  if ((m1!='B') || (m2!='M'))
  {
      fclose(fp);
      return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&filesize)) {
	fclose(fp);
	return NULL;
  }

  if (readShortLE(fp,&m_bytesRead,&res1)) {
	fclose(fp);
	return NULL;
  }

  if (readShortLE(fp,&m_bytesRead,&res2)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&pixoff)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&bmisize)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&bmWidth)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&bmHeight)) {
	fclose(fp);
	return NULL;
  }

  if (readShortLE(fp,&m_bytesRead,&bmPlanes)) {
	fclose(fp);
	return NULL;
  }

  if (readShortLE(fp,&m_bytesRead,&bmBitsPixel)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&compression)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&sizeimage)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&xscale)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&yscale)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&colors)) {
	fclose(fp);
	return NULL;
  }

  if (readLongLE(fp,&m_bytesRead,&impcol)) {
	fclose(fp);
	return NULL;
  }

  if (colors == 0)
    colors = 1 << bmBitsPixel;

  RGBQUAD *colormap = NULL;
  if (bmBitsPixel != 24)
  {
	colormap = new RGBQUAD[colors];
	if (colormap == NULL)
	{
	  fclose(fp);
	  return NULL;
	}

	int i;
	for (i = 0; i < colors; i++)
	{
	  unsigned char r ,g, b, dummy;

	  rc = fread(&b, 1, 1, fp);
	  m_bytesRead++;
	  if (rc!=1)
	  {
		delete [] colormap;
		fclose(fp);
		return NULL;
	  }

	  rc = fread(&g, 1, 1, fp);
	  m_bytesRead++;
	  if (rc!=1)
	  {
		delete [] colormap;
		fclose(fp);
		return NULL;
	  }

	  rc = fread(&r, 1, 1, fp);
	  m_bytesRead++;
	  if (rc != 1)
	  {
		delete [] colormap;
		fclose(fp);
		return NULL;
	  }

	  rc = fread(&dummy, 1, 1, fp);
	  m_bytesRead++;
	  if (rc != 1)
	  {
		delete [] colormap;
		fclose(fp);
		return NULL;
	  }

	  colormap[i].rgbRed=r;
	  colormap[i].rgbGreen=g;
	  colormap[i].rgbBlue=b;
    }
  }

  if ((long)m_bytesRead > pixoff)
  {
	delete [] colormap;
	fclose(fp);
	return NULL;
  }

  while ((long)m_bytesRead < pixoff)
  {
	char dummy;
	fread(&dummy,1,1,fp);
	m_bytesRead++;
  }

  int w = bmWidth;
  int h = bmHeight;

  // set the output params
  imagebits = (unsigned char *)malloc(w * h * 3);
  long row_size = w * 3;

  if (imagebits != NULL)
  {
	  *width = w;
	  *height = h;
	  unsigned char *outbuf = imagebits;
	  long row = 0;
	  long rowOffset = 0;

	  if (compression == 0) // BI_RGB
    {
	    // read rows in reverse order
	    for (row = bmHeight - 1; row >= 0; row--)
	    {
		    // which row are we working on?
		    rowOffset = (long unsigned)row * row_size;

		    if (bmBitsPixel == 24)
		    {
		      for (int col=0;col<w;col++)
		      {
			    long offset = col * 3;
			    char pixel[3];

			    if (fread((void *)(pixel),1,3,fp) == 3)
			    {
			      // we swap red and blue here
			      *(outbuf + rowOffset + offset + 0) = pixel[2];  // r
			      *(outbuf + rowOffset + offset + 1) = pixel[1];  // g
			      *(outbuf + rowOffset + offset + 2) = pixel[0];  // b
			    }
              }
		      m_bytesRead += row_size;

		      // read DWORD padding
		      while ((m_bytesRead - pixoff) & 3)
		      {
			      char dummy;
			      if (fread(&dummy,1,1,fp) != 1)
            {
		          free(imagebits);
		          fclose(fp);
		          return NULL;
            }
			      m_bytesRead++;
          }
        }
        else
        {
		      // pixels are packed as 1 , 4 or 8 bit vals. need to unpack them
		      int bit_count = 0;
		      unsigned long mask = (1 << bmBitsPixel) - 1;
		      unsigned char inbyte = 0;

		      for (int col = 0; col < w; col++)
          {
		        int pix = 0;

		        // if we need another byte
		        if (bit_count <= 0)
			      {
			        bit_count = 8;
			        if (fread(&inbyte,1,1,fp) != 1)
			        {
				        free(imagebits);
				        delete [] colormap;
				        fclose(fp);
				        return NULL;
			        }
			        m_bytesRead++;
			      }

			      // keep track of where we are in the bytes
			      bit_count -= bmBitsPixel;
			      pix = ( inbyte >> bit_count) & mask;

			      // lookup the color from the colormap - stuff it in our buffer
			      // swap red and blue
			      *(outbuf + rowOffset + col * 3 + 2) = colormap[pix].rgbBlue;
			      *(outbuf + rowOffset + col * 3 + 1) = colormap[pix].rgbGreen;
			      *(outbuf + rowOffset + col * 3 + 0) = colormap[pix].rgbRed;
          }

		      // read DWORD padding
		      while ((m_bytesRead - pixoff) & 3)
          {
            char dummy;
            if (fread(&dummy,1,1,fp)!=1)
            {
              free(imagebits);
              if (colormap)
                delete [] colormap;
              fclose(fp);
              return NULL;
            }
            m_bytesRead++;
          }
        }
      }
    }
	  else
	  {
	    int i, x = 0;
	    unsigned char c, c1 = 0, *pp;
	    row = 0;
	    pp = outbuf + (bmHeight - 1) * bmWidth * 3;

	    if (bmBitsPixel == 8)
	    {
		    while (row < bmHeight)
		    {
		      c = getc(fp);

		      if (c)
		      {
			      // encoded mode
			      c1 = getc(fp);
			      for (i = 0; i < c; x++, i++)
			      {
			        *pp = colormap[c1].rgbRed; pp++;
			        *pp = colormap[c1].rgbGreen; pp++;
			        *pp = colormap[c1].rgbBlue; pp++;
			      }
		      }
		      else
		      {
			      // c==0x00,  escape codes
			      c = getc(fp);
			      if (c == 0x00) // end of line
			      {
			        row++;
			        x = 0;
			        pp = outbuf + (bmHeight - row - 1) * bmWidth * 3;
			      }
			      else if (c == 0x01)
			        break; // end of pic
			      else if (c == 0x02) // delta
			      {
			        c = getc(fp);
			        x += c;
			        c = getc(fp);
			        row += c;
			        pp = outbuf + x*3 + (bmHeight - row - 1) * bmWidth * 3;
			      }
			      else // absolute mode
			      {
			        for (i = 0; i < c; x++, i++)
			        {
				        c1 = getc(fp);
				        *pp = colormap[c1].rgbRed; pp++;
				        *pp = colormap[c1].rgbGreen; pp++;
				        *pp = colormap[c1].rgbBlue; pp++;
			        }

			        if (c & 1)
				      getc(fp); // odd length run: read an extra pad byte
			      }
		      }
		    }
	    }
	    else if (bmBitsPixel == 4)
	    {
		    while (row < bmHeight)
		    {
		      c = getc(fp);

		      if (c)
		      {
			      // encoded mode
			      c1 = getc(fp);
			      for (i = 0; i < c; x++, i++)
			      {
			        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbRed; pp++;
			        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbGreen; pp++;
			        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbBlue; pp++;
			      }
		      }
		      else
		      {
			      // c==0x00,  escape codes
			      c = getc(fp);

			      if (c == 0x00) // end of line
			      {
			        row++;
			        x = 0;
			        pp = outbuf + (bmHeight - row - 1) * bmWidth * 3;
			      }
			      else if (c == 0x01)
			        break; // end of pic
			      else if (c == 0x02) // delta
			      {
			        c = getc(fp);
			        x += c;
			        c = getc(fp);
			        row += c;
			        pp = outbuf + x * 3 + (bmHeight - row - 1) * bmWidth * 3;
			      }
			      else // absolute mode
			      {
			        for (i = 0; i < c; x++, i++)
			        {
				        if ((i&1) == 0)
				          c1 = getc(fp);
				        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbRed; pp++;
				        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbGreen; pp++;
				        *pp = colormap[(i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f)].rgbBlue; pp++;
			        }

			        if (((c & 3) == 1) || ((c & 3) == 2))
				      getc(fp); // odd length run: read an extra pad byte
            }
          }
		    }
	    }
	  }
	  if (colormap)
	    delete [] colormap;

	  fclose(fp);
  }
  return imagebits;
}

void bmp_to_pixmap (const char* filename, GdkPixmap **pixmap, GdkBitmap **mask)
{
  guint16 width, height;
  unsigned char *buf;
  GdkWindow *window = gdk_get_default_root_window();
  GdkColormap *colormap;
  GdkGC* gc = gdk_gc_new (window);
  int i, j;
  bool hasMask = false;

  *pixmap = *mask = NULL;
  buf = load_bitmap_file (filename, &width, &height);
  if (!buf)
    return;

  colormap = gdk_drawable_get_colormap (window);
  *pixmap = gdk_pixmap_new (window, width, height, -1);

  typedef struct
  {
    GdkColor c;
    unsigned char *p;
  } PAL;

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      unsigned char *p = &buf[(i * width + j) * 3];
      PAL pe;

      pe.c.red = (gushort)(p[0] * 0xFF);
      pe.c.green = (gushort)(p[1] * 0xFF);
      pe.c.blue = (gushort)(p[2] * 0xFF);
      gdk_colormap_alloc_color(colormap, &pe.c, FALSE, TRUE);
      gdk_gc_set_foreground(gc, &pe.c);
      gdk_draw_point(*pixmap, gc, j, i);

      if (p[0] == 0xFF && p[1] == 0x00 && p[2] == 0xFF)
        hasMask = true;
    }
  }

  gdk_gc_unref (gc);
  *mask = gdk_pixmap_new (window, width, height, 1);
  gc = gdk_gc_new (*mask);
  if (hasMask)
  {
    for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
      {
	      GdkColor mask_pattern;

	      // pink is transparent
	      if ((buf[(i*width+j)*3] == 0xff) &&
		      (buf[(i*width+j)*3+1] == 0x00) &&
		      (buf[(i*width+j)*3+2] == 0xff))
		      mask_pattern.pixel = 0;
        else
		      mask_pattern.pixel = 1;

        gdk_gc_set_foreground (gc, &mask_pattern);
        // possible Win32 Gtk bug here
        //gdk_draw_point (*mask, gc, j, i);
        gdk_draw_line (*mask, gc, j, i, j + 1, i);
      }
    }
  }
  else
  {
    GdkColor mask_pattern;
    mask_pattern.pixel = 1;
    gdk_gc_set_foreground (gc, &mask_pattern);
    gdk_draw_rectangle (*mask, gc, 1, 0, 0, width, height);
  }
  gdk_gc_unref(gc);
  free (buf);
}

void load_pixmap (const char* filename, GtkWidget* widget, GdkPixmap **gdkpixmap, GdkBitmap **mask)
{
  CString str;

  str = g_strBitmapsPath;
  str += filename;

  bmp_to_pixmap (str.GetBuffer (), gdkpixmap, mask);
  if (*gdkpixmap == NULL)
  {
    printf("gdkpixmap was null\n");
    gchar *dummy[] = { "1 1 1 1", "  c None", " " };
    printf("calling gdk_pixmap_create_from_xpm_d\n");
    *gdkpixmap = gdk_pixmap_create_from_xpm_d (gdk_get_default_root_window(), mask, NULL, dummy);
  }
}

// this is the same as above but used by the plugins
// GdkPixmap **gdkpixmap, GdkBitmap **mask
bool WINAPI load_plugin_bitmap (const char* filename, void **gdkpixmap, void **mask)
{
  CString str;

  str = g_strGameToolsPath;
  str += g_strPluginsDir;
  str += "bitmaps/";
  str += filename;
  bmp_to_pixmap (str.GetBuffer (), (GdkPixmap **)gdkpixmap, (GdkBitmap **)mask);

  if (*gdkpixmap == NULL)
  {
    // look in the core plugins
    str = g_strAppPath;
    str += g_strPluginsDir;
    str += "bitmaps/";
    str += filename;
    bmp_to_pixmap (str.GetBuffer (), (GdkPixmap **)gdkpixmap, (GdkBitmap **)mask);

    if (*gdkpixmap == NULL)
    {

      // look in core modules
      str = g_strAppPath;
      str += g_strModulesDir;
      str += "bitmaps/";
      str += filename;
      bmp_to_pixmap (str.GetBuffer (), (GdkPixmap **)gdkpixmap, (GdkBitmap **)mask);

      if (*gdkpixmap == NULL)
      {
        gchar *dummy[] = { "1 1 1 1", "  c None", " " };
        *gdkpixmap = gdk_pixmap_create_from_xpm_d (gdk_get_default_root_window(), (GdkBitmap **)mask, NULL, dummy);
        return false;
      }
    }
  }
  return true;
}

// Load a xpm file and return a pixmap widget.
GtkWidget* new_pixmap (GtkWidget* widget, const char* filename)
{
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

  load_pixmap (filename, widget, &gdkpixmap, &mask);
  pixmap = gtk_pixmap_new (gdkpixmap, mask);

  gdk_drawable_unref (gdkpixmap);
  gdk_drawable_unref (mask);

  return pixmap;
}

// =============================================================================
// Menu stuff

GtkWidget* menu_separator (GtkWidget *menu)
{
  GtkWidget *menu_item = gtk_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);
  gtk_widget_show (menu_item);
  return menu_item;
}

GtkWidget* menu_tearoff (GtkWidget *menu)
{
  GtkWidget *menu_item = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), menu_item);
// gtk_widget_set_sensitive (menu_item, FALSE); -- controls whether menu is detachable
  gtk_widget_show (menu_item);
  return menu_item;
}

GtkWidget* create_sub_menu_with_mnemonic (GtkWidget *bar, const gchar *mnemonic)
{
  GtkWidget *item, *sub_menu;

  item = gtk_menu_item_new_with_mnemonic (mnemonic);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (bar), item);

  sub_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), sub_menu);

  return sub_menu;
}

extern void AddMenuItem (GtkWidget* menu, unsigned int id);

GtkWidget* create_menu_item_with_mnemonic (GtkWidget *menu, const gchar *mnemonic, GtkSignalFunc func, int id)
{
  GtkWidget *item;

  item = gtk_menu_item_new_with_mnemonic (mnemonic);

  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (func), GINT_TO_POINTER (id));

  AddMenuItem (item, id);
  return item;
}

GtkWidget* create_check_menu_item_with_mnemonic (GtkWidget *menu, const gchar *mnemonic, GtkSignalFunc func, int id, gboolean active)
{
  GtkWidget *item;

  item = gtk_check_menu_item_new_with_mnemonic(mnemonic);

  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), active);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (func), GINT_TO_POINTER (id));

  AddMenuItem (item, id);
  return item;
}

GtkWidget* create_radio_menu_item_with_mnemonic (GtkWidget *menu, GtkWidget *last, const gchar *mnemonic, GtkSignalFunc func, int id, gboolean state)
{
  GtkWidget *item;
  GSList *group = (GSList*)NULL;

  if (last != NULL)
    group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (last));
  item = gtk_radio_menu_item_new_with_mnemonic (group, mnemonic);
  gtk_check_menu_item_set_state (GTK_CHECK_MENU_ITEM (item), state);

  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (func), GINT_TO_POINTER (id));

  AddMenuItem (item, id);
  return item;
}

GtkWidget* create_menu_in_menu_with_mnemonic (GtkWidget *menu, const gchar *mnemonic)
{
  GtkWidget *item, *submenu;

  item = gtk_menu_item_new_with_mnemonic(mnemonic);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

  return submenu;
}

// =============================================================================
// Message Boxes

void dialog_button_callback( GtkWidget *widget, gpointer data ) {
  GtkWidget *parent;
  int *loop, *ret;

  parent = gtk_widget_get_toplevel( widget );
  loop = (int*)g_object_get_data( G_OBJECT( parent ), "loop" );
  ret = (int*)g_object_get_data( G_OBJECT( parent ), "ret" );

  *loop = 0;
  *ret = (int)data;
}

gint dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  int *loop;

  gtk_widget_hide (widget);
  loop = (int*)g_object_get_data (G_OBJECT (widget), "loop");
  *loop = 0;

  return TRUE;
}

gint dialog_url_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  OpenURL((const char *)g_object_get_data (G_OBJECT (widget), "URL"));

  return TRUE;
}

int WINAPI gtk_MessageBox (void *parent, const char* lpText, const char* lpCaption, guint32 uType, const char* URL)
{
  GtkWidget *window, *w, *vbox, *hbox;
  GtkAccelGroup *accel;
  int mode = (uType & MB_TYPEMASK), ret, loop = 1;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  gtk_window_set_title (GTK_WINDOW (window), lpCaption);
  gtk_container_border_width (GTK_CONTAINER (window), 10);
  g_object_set_data (G_OBJECT (window), "loop", &loop);
  g_object_set_data (G_OBJECT (window), "ret", &ret);
  gtk_widget_realize (window);

  gtk_window_set_policy(GTK_WINDOW (window),FALSE,FALSE,TRUE);

  if (parent != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));

  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  w = gtk_label_new (lpText);
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
  gtk_widget_show (w);

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  gtk_widget_show (hbox);

  if (mode == MB_OK)
  {
    w = gtk_button_new_with_label ("Ok");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
    gtk_widget_add_accelerator (w, "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
    gtk_widget_add_accelerator (w, "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);
    ret = IDOK;
  }
  else if (mode ==  MB_OKCANCEL)
  {
    w = gtk_button_new_with_label ("Ok");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
    gtk_widget_add_accelerator (w, "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("Cancel");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
    gtk_widget_add_accelerator (w, "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
    gtk_widget_show (w);
    ret = IDCANCEL;
  }
  else if (mode == MB_YESNOCANCEL)
  {
    w = gtk_button_new_with_label ("Yes");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDYES));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("No");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDNO));
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("Cancel");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
    gtk_widget_show (w);
    ret = IDCANCEL;
  }
  else /* if (mode == MB_YESNO) */
  {
    w = gtk_button_new_with_label ("Yes");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDYES));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("No");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDNO));
    gtk_widget_show (w);
    ret = IDNO;
  }

  if (URL)
  {
    w = gtk_button_new_with_label ("Go to URL");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_url_callback), NULL);
    g_object_set_data (G_OBJECT (w), "URL", (void *)URL);
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);
  }


  gtk_widget_show (window);
  gtk_grab_add (window);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (window);
  gtk_widget_destroy (window);

  return ret;
}

// =============================================================================
// File dialog

// fenris #3078 WHENHELLISFROZENOVER

//#define FILEDLG_DBG

static void file_sel_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *parent;
  int *loop;
  bool *success;

  parent = gtk_widget_get_toplevel (widget);
  loop = (int*)g_object_get_data (G_OBJECT (parent), "loop");
  success = (bool*)g_object_get_data (G_OBJECT (parent), "success");

  if ((int)data == IDOK)
    *success = true;

#ifdef FILEDLG_DBG
  else
    Sys_Printf("file_sel_callback != IDOK\n");
#endif

  *loop = 0;
}

#ifdef _WIN32
#include <commdlg.h>
static OPENFILENAME ofn;       /* common dialog box structure   */
static char szDirName[MAX_PATH];    /* directory string              */
static char szFile[MAX_PATH];       /* filename string               */
static char szFileTitle[MAX_PATH];  /* file title string             */
static int i, cbString;        /* integer count variables       */
static HANDLE hf;              /* file handle                   */
#else
static char szFile[QER_MAX_NAMELEN];
#endif

#define FILEDLG_CUSTOM_FILTER_LENGTH 64
// to be used with the advanced file selector

class CFileType : public IFileTypeList
{
  struct filetype_copy_t
  {
    void operator=(const filetype_t& other)
    {
      m_name = other.name;
      m_pattern = other.pattern;
    }
    string_t m_name;
    string_t m_pattern;
  };
public:
  CFileType()
  {
    m_nTypes = 0;
    m_pTypes = NULL;
    m_strWin32Filters = NULL;
    m_pstrGTKMasks = NULL;
  }

  virtual ~CFileType()
  {
    delete[] m_pTypes;
    DestroyWin32Filters();
    DestroyGTKMasks();
  }

  void addType(filetype_t type)
  {
    filetype_copy_t* newTypes = new filetype_copy_t [m_nTypes+1];
    if(m_nTypes > 0)
    {
      for(int i=0; i<m_nTypes; i++)
        newTypes[i] = m_pTypes[i];
      delete[] m_pTypes;
    }
    m_pTypes = newTypes;
    m_pTypes[m_nTypes] = type;
    m_nTypes++;
    ConstructGTKMasks();
    ConstructWin32Filters();
  }

  filetype_t GetTypeForWin32Filter(const char *filter) const
  {
    for(int i=0; i<m_nTypes; i++)
      if(strcmp(m_pTypes[i].m_pattern.c_str(), filter)==0)
        return filetype_t(m_pTypes[i].m_name.c_str(), m_pTypes[i].m_pattern.c_str());
    return filetype_t();
  }

  filetype_t GetTypeForGTKMask(const char *mask) const
  {
    for(int i=0; i<m_nTypes; i++)
      if(strcmp(m_pstrGTKMasks[i],mask)==0)
        return filetype_t(m_pTypes[i].m_name.c_str(), m_pTypes[i].m_pattern.c_str());
    return filetype_t();
  }

  char *m_strWin32Filters;
  char **m_pstrGTKMasks;
private:
  int m_nTypes;
  filetype_copy_t *m_pTypes;

  void DestroyWin32Filters()
  {
    delete[] m_strWin32Filters;
  }

  void ConstructWin32Filters()
  {
    const char *r;
    char *w;
    int i;
    int len = 0;
    DestroyWin32Filters();
    for(i=0; i<m_nTypes; i++)
      len = len + strlen(m_pTypes[i].m_name.c_str()) + strlen(m_pTypes[i].m_pattern.c_str())*2 + 5;
    m_strWin32Filters = new char[len+1]; // length + null char
    for(i=0, w = m_strWin32Filters; i<m_nTypes; i++)
    {
      for(r = m_pTypes[i].m_name.c_str(); *r!='\0'; r++, w++)
        *w = *r;
      *w++ = ' ';
      *w++ = '(';
      for(r = m_pTypes[i].m_pattern.c_str(); *r!='\0'; r++, w++)
        *w = *r;
      *w++ = ')';
      *w++ = '\0';
      for(r = m_pTypes[i].m_pattern.c_str(); *r!='\0'; r++, w++)
        *w = (*r == ',') ? ';' : *r;
      *w++ = '\0';
    }
    m_strWin32Filters[len] = '\0';
  }

  void DestroyGTKMasks()
  {
    if(m_pstrGTKMasks != NULL)
      for(char **p = m_pstrGTKMasks; *p != NULL; p++)
        delete[] *p;
    delete[] m_pstrGTKMasks;
  }

  void ConstructGTKMasks()
  {
    const char *r;
    char *w;
    int i;
    int len = 0;
    DestroyGTKMasks();
    m_pstrGTKMasks = new char*[m_nTypes+1];
    for(i=0; i<m_nTypes; i++)
    {
      len = strlen(m_pTypes[i].m_name.c_str()) + strlen(m_pTypes[i].m_pattern.c_str()) + 3;
      m_pstrGTKMasks[i] = new char[len+1]; // length + null char
      w = m_pstrGTKMasks[i];
      for(r = m_pTypes[i].m_name.c_str(); *r!='\0'; r++, w++)
        *w = *r;
      *w++ = ' ';
      *w++ = '<';
      for(r = m_pTypes[i].m_pattern.c_str(); *r!='\0'; r++, w++)
        *w = *r;
      *w++ = '>';
      *w++ = '\0';
    }
    m_pstrGTKMasks[m_nTypes] = NULL;
  }

};

/**
 * @param[in] baseSubDir should have a trailing slash if not @c NULL
 */
const char* file_dialog (void *parent, gboolean open, const char* title, const char* path, const char* pattern, const char *baseSubDir)
{
  // Gtk dialog
  GtkWidget* file_sel;
  int loop = 1;
  char *new_path = NULL;

  const char* r;
  char* w;
  filetype_t type;
  CFileType typelist;
  if(pattern != NULL)
    GetFileTypeRegistry()->getTypeList(pattern, &typelist);

#ifdef FILEDLG_DBG
  Sys_Printf("file_dialog: open = %d title = %s path = %s\n", open, title, path);
  if (pattern)
  {
    Sys_Printf("Patterns:\n");
    char** p = typelist.m_pstrGTKMasks;
    while(*p!=NULL)
      Sys_Printf("%s\n", *p++);
  }
  else
    Sys_Printf("no patterns\n");
#endif

#ifdef _WIN32
  // win32 dialog stores the selected "save as type" extension in the second null-terminated string
  char customfilter[FILEDLG_CUSTOM_FILTER_LENGTH];

  if (g_PrefsDlg.m_bNativeGUI)
  {
#ifdef FILEDLG_DBG
    Sys_Printf("Doing win32 file dialog...");
#endif
    // do that the native way
    /* Place the terminating null character in the szFile. */
    szFile[0] = '\0';
    customfilter[0] = customfilter[1] = customfilter[2] = '\0';

    /* Set the members of the OPENFILENAME structure. */
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = (HWND)GDK_WINDOW_HWND (g_pParentWnd->m_pWidget->window);
    if (pattern)
    {
      ofn.nFilterIndex = 0;
      ofn.lpstrFilter = typelist.m_strWin32Filters;
    }
    else ofn.nFilterIndex = 1;
    ofn.lpstrCustomFilter = customfilter;
    ofn.nMaxCustFilter = sizeof(customfilter);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL; // we don't need to get the name of the file
    if(path)
    {
      // szDirName: Radiant uses unix convention for paths internally
      //   Win32 (of course) and Gtk (who would have thought) expect the '\\' convention
      // copy path, replacing dir separators as appropriate
      for(r=path, w=szDirName; *r!='\0'; r++)
        *w++ = (*r=='/') ? '\\' : *r;
      // terminate string
      *w = '\0';
      ofn.lpstrInitialDir = szDirName;
    }
    else ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    /* Display the Open dialog box. */
    // it's open or close depending on 'open' parameter
    if (open)
    {
      if (!GetOpenFileName(&ofn))
        return NULL;	// canceled
    }
    else
    {
      if (!GetSaveFileName(&ofn))
        return NULL;	// canceled
    }

    if(pattern != NULL)
      type = typelist.GetTypeForWin32Filter(customfilter+1);

#ifdef FILEDLG_DBG
    Sys_Printf("Done.\n");
#endif
  }
  else
  {
#endif
	char buf[PATH_MAX];
    // do that the Gtk way
    if (title == NULL)
      title = open ? _("Open File") : _("Save File");

#ifdef FILEDLG_DBG
    Sys_Printf("Doing Gtk file dialog:\nBuilding new_path..");
#endif
    // we expect an actual path below, if the path is NULL we might crash
    if (!path || path[0] == '\0')
    {
		strcpy(buf, g_pGameDescription->mEnginePath.GetBuffer());
		strcat(buf, g_pGameDescription->mBaseGame.GetBuffer());
		strcat(buf, "/");
		if (baseSubDir)
			strcat(buf, baseSubDir);
		path = buf;
	}

    // alloc new path with extra char for dir separator
    new_path = new char[strlen(path)+1+1];
    // copy path, replacing dir separators as appropriate
    for(r=path, w=new_path; *r!='\0'; r++)
      *w++ = (*r=='/') ? G_DIR_SEPARATOR : *r;
    // add dir separator to end of path if required
    if(*(w-1) != G_DIR_SEPARATOR) *w++ = G_DIR_SEPARATOR;
    // terminate string
    *w = '\0';

#ifdef FILEDLG_DBG
	Sys_Printf("Done.\n");
	Sys_Printf("Calling gtk_file_selection_new with title: %s...", title);
#endif
    file_sel = gtk_file_selection_new (title);
#ifdef FILEDLG_DBG
	Sys_Printf("Done.\n");
	Sys_Printf("Set the masks...");
#endif

#if 0 //!\todo Add masks to GtkFileSelection in gtk-2.0
    // set the masks
    if (pattern)
    {
      gtk_file_selection_clear_masks (GTK_FILE_SELECTION (file_sel));
      gtk_file_selection_set_masks (GTK_FILE_SELECTION (file_sel), const_cast<const char**>(typelist.m_pstrGTKMasks));
    }
#endif

#ifdef FILEDLG_DBG
    Sys_Printf("Done.\n");
#endif

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->ok_button), "clicked",
      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDOK));
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->cancel_button), "clicked",
      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDCANCEL));
    gtk_signal_connect (GTK_OBJECT (file_sel), "delete_event",
      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
    gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (file_sel));

    if (parent != NULL)
      gtk_window_set_transient_for (GTK_WINDOW (file_sel), GTK_WINDOW (parent));

#ifdef FILEDLG_DBG
    Sys_Printf("set_data...");
#endif
    bool success = false;
    g_object_set_data (G_OBJECT (file_sel), "loop", &loop);
    g_object_set_data (G_OBJECT (file_sel), "success", &success);
#ifdef FILEDLG_DBG
    Sys_Printf("Done.\n");
#endif

    if (!open)
    {
#ifdef FILEDLG_DBG
      Sys_Printf("set_data \"overwrite\" ...");
#endif
      g_object_set_data (G_OBJECT (file_sel), "overwrite", GINT_TO_POINTER (1));
#ifdef FILEDLG_DBG
      Sys_Printf("Done.\n");
#endif
    }

    if (new_path != NULL)
    {
#ifdef FILEDLG_DBG
      Sys_Printf("gtk_file_selection_set_filename... %p (%s)", file_sel, new_path);
#endif
      gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_sel), new_path);
      delete[] new_path;
#ifdef FILEDLG_DBG
      Sys_Printf("Done.\n");
#endif
    }

    gtk_grab_add (file_sel);
#ifdef FILEDLG_DBG
    Sys_Printf("gtk_widget_show... %p", file_sel);
#endif
    gtk_widget_show (file_sel);
#ifdef FILEDLG_DBG
    Sys_Printf("Done.\n");
#endif

#ifdef FILEDLG_DBG
    Sys_Printf("gtk_main_iteration...");
#endif
    while (loop)
      gtk_main_iteration ();
    if(success)
    {
#if 0 //!\todo Add masks to GtkFileSelection in gtk2
      if(pattern!=NULL)
        type = typelist.GetTypeForGTKMask(GTK_FILE_SELECTION (file_sel)->mask);
#endif
      strcpy(szFile, gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_sel)));
    }
#ifdef FILEDLG_DBG
    Sys_Printf("Done.\n");
#endif

    gtk_grab_remove (file_sel);
    gtk_widget_destroy (file_sel);
#ifdef _WIN32
  }
#endif

  // don't return an empty filename
  if(szFile[0] == '\0') return NULL;

  // convert back to unix format
  for(w=szFile; *w!='\0'; w++)
    if(*w=='\\')
      *w = '/';

#if defined(WIN32)
  if (g_PrefsDlg.m_bNativeGUI) // filetype mask not supported in gtk dialog yet
  {
    // when saving, force an extension depending on filetype
    /* \todo SPoG - file_dialog should return filetype information separately.. not force file extension.. */
    if(!open && pattern != NULL)
    {
      // last ext separator
      w = strrchr(szFile, '.');
      // no extension
      w = (w!=NULL) ? w : szFile+strlen(szFile);
      strcpy(w, type.pattern+1);
    }
  }
#endif

  // prompt to overwrite existing files
  if (!open)
    if (access (szFile, R_OK) == 0)
      if (gtk_MessageBox (parent, "File already exists.\nOverwrite?", "GtkRadiant", MB_YESNO) == IDNO)
        return NULL;

#ifdef FILEDLG_DBG
  // ... let's use a static filename
  Sys_Printf("filename: %p\n", szFile);
#endif

  return szFile;
}

char* WINAPI dir_dialog (void *parent, const char* title, const char* path)
{
  GtkWidget* file_sel;
  char* filename = (char*)NULL;
  int loop = 1;
  bool success = false;

  file_sel = gtk_file_selection_new (title);
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->ok_button), "clicked",
                      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDOK));
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_signal_connect (GTK_OBJECT (file_sel), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (file_sel));

  if (parent != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (file_sel), GTK_WINDOW (parent));

  gtk_widget_hide (GTK_FILE_SELECTION (file_sel)->file_list->parent);

  g_object_set_data (G_OBJECT (file_sel), "loop", &loop);
  g_object_set_data (G_OBJECT (file_sel), "success", &success);

  if (path != NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_sel), path);

  gtk_grab_add (file_sel);
  gtk_widget_show (file_sel);

  while (loop)
    gtk_main_iteration ();

  filename = g_strdup(gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_sel)));

  gtk_grab_remove (file_sel);
  gtk_widget_destroy (file_sel);

  return filename;
}

bool WINAPI color_dialog (void *parent, float *color, const char* title)
{
  GtkWidget* dlg;
  double clr[3];
  int loop = 1, ret = IDCANCEL;

  clr[0] = color[0];
  clr[1] = color[1];
  clr[2] = color[2];

  dlg = gtk_color_selection_dialog_new (title);
  gtk_color_selection_set_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), clr);
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG (dlg)->ok_button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG (dlg)->cancel_button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  if (parent != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (parent));

  gtk_widget_show (dlg);
  gtk_grab_add (dlg);

  while (loop)
    gtk_main_iteration ();

  GdkColor gdkcolor;
  gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), &gdkcolor);
  clr[0] = gdkcolor.red / 65535.0;
  clr[1] = gdkcolor.green / 65535.0;
  clr[2] = gdkcolor.blue / 65535.0;

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  if (ret == IDOK)
  {
    color[0] = (float)clr[0];
    color[1] = (float)clr[1];
    color[2] = (float)clr[2];

    return true;
  }

  return false;
}

void OpenURL(const char *url)
{
  // let's put a little comment
  Sys_Printf("OpenURL: %s\n", url);
#ifdef __linux__
  // \todo FIXME: the way we open URLs on *nix should be improved. A script is good (see how I do on RTCW)
  char command[2*PATH_MAX];
  snprintf( command, sizeof(command), "%s/openurl.sh \"%s\" &", g_strAppPath.GetBuffer(), url );
  if (system (command) != 0)
    gtk_MessageBox (g_pParentWnd->m_pWidget, "Failed to launch Netscape!");
#endif
#ifdef __APPLE__
  char command[2*PATH_MAX];
  snprintf (command, sizeof(command),
            "open \"%s\" &", url, url);
  if (system (command) != 0)
    gtk_MessageBox (g_pParentWnd->m_pWidget, "Unable to launch browser!");
#endif
#ifdef _WIN32
  ShellExecute( (HWND)GDK_WINDOW_HWND (g_pParentWnd->m_pWidget->window), "open", url, NULL, NULL, SW_SHOW );
#endif
}

void CheckMenuSplitting (GtkWidget *&menu)
{
  GtkWidget *item,*menu2;

  GtkRequisition requisition;
  gint screen_height;

  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);
  screen_height = gdk_screen_height ();

  if ((screen_height - requisition.height) < 20)
  {
    menu2 = gtk_menu_new ();

    // move the last 2 items to a submenu (3 because of win32)
    for (int i = 0; i < 3; i++)
    {
      item = GTK_WIDGET (g_list_last (gtk_container_children (GTK_CONTAINER (menu)))->data);
      gtk_widget_ref (item);
      gtk_container_remove (GTK_CONTAINER (menu), item);
      gtk_menu_append (GTK_MENU (menu2), item);
      gtk_widget_unref (item);
    }

    item = gtk_menu_item_new_with_label ("--------");
    gtk_widget_show (item);
    gtk_container_add (GTK_CONTAINER (menu), item);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu2);
    menu = menu2;
  }
}
