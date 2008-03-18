// PixMap.cpp: implementation of the CPixMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pixmap.h"

#ifdef __linux__
	#include <gdk/gdkx.h>
#else
	#include <gdk/win32/gdkwin32.h>
	#define NO_UNDERSCORE
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPixMap::CPixMap()
{

}

CPixMap::~CPixMap()
{

}

unsigned char* CPixMap::load_bitmap_file (const char* filename,guint16* width, guint16* height)
{
  gint32 bmWidth, bmHeight;
  guint16 bmPlanes, bmBitsPixel;
  typedef struct {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
  } RGBQUAD;
  unsigned char m1,m2;
  unsigned long sizeimage;
  short res1,res2;
  long filesize, pixoff;
  long bmisize, compression;
  long xscale, yscale;
  long colors, impcol;
  unsigned long m_bytesRead = 0;
  unsigned char *imagebits = NULL;
  FILE *fp;

  *width = *height = 0;

  fp = fopen(filename,"rb");
  if (fp == NULL)
    return NULL;

  long rc;
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

  rc = fread((long*)&(filesize),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((int*)&(res1),2,1,fp); m_bytesRead+=2;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((int*)&(res2),2,1,fp); m_bytesRead+=2;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(pixoff),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(bmisize),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(bmWidth),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(bmHeight),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((int*)&(bmPlanes),2,1,fp); m_bytesRead+=2;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((int*)&(bmBitsPixel),2,1,fp); m_bytesRead+=2;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(compression),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(sizeimage),4,1,fp); m_bytesRead+=4;
  if (rc != 1) {fclose(fp); return NULL; }

  rc = fread((long*)&(xscale),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(yscale),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(colors),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

  rc = fread((long*)&(impcol),4,1,fp); m_bytesRead+=4;
  if (rc != 1) { fclose(fp); return NULL; }

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
  imagebits = (unsigned char*)malloc(w*h*3);
  long row_size = w * 3;

  if (imagebits != NULL) 
    {
      *width = w;
      *height = h;
      unsigned char* outbuf = imagebits;
      long row = 0;
      long rowOffset = 0;

      if (compression == 0) // BI_RGB
	{
	  // read rows in reverse order
	  for (row=bmHeight-1;row>=0;row--)
	    {
	      // which row are we working on?
	      rowOffset = (long unsigned)row*row_size;						      

	      if (bmBitsPixel == 24)
		{
		  for (int col=0;col<w;col++)
		    {
		      long offset = col * 3;
		      char pixel[3];

		      if (fread((void*)(pixel),1,3,fp)==3)
			{
			  // we swap red and blue here
			  *(outbuf + rowOffset + offset + 0)=pixel[2];		// r
			  *(outbuf + rowOffset + offset + 1)=pixel[1];		// g
			  *(outbuf + rowOffset + offset + 2)=pixel[0];		// b
			}
		    }
		  m_bytesRead += row_size;

		  // read DWORD padding
		  while ((m_bytesRead-pixoff)&3)
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

		  for (int col=0;col<w;col++)
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
		  while ((m_bytesRead-pixoff)&3)
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
	  pp = outbuf + (bmHeight-1)*bmWidth*3;

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
			  pp = outbuf + (bmHeight-row-1)*bmWidth*3;
			}
		      else if (c == 0x01)
			break; // end of pic
		      else if (c == 0x02) // delta
			{
			  c = getc(fp);
			  x += c;
			  c = getc(fp);
			  row += c;
			  pp = outbuf + x*3 + (bmHeight-row-1)*bmWidth*3;
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
			  pp = outbuf + (bmHeight-row-1)*bmWidth*3;
			}
		      else if (c == 0x01)
			break; // end of pic
		      else if (c == 0x02) // delta
			{
			  c = getc(fp);
			  x += c;
			  c = getc(fp);
			  row += c;
			  pp = outbuf + x*3 + (bmHeight-row-1)*bmWidth*3;
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

			  if (((c&3) == 1) || ((c&3) == 2))
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

void CPixMap::bmp_to_pixmap (const char* filename, GdkPixmap **pixmap, GdkBitmap **mask)
{
  guint16 width, height;
  unsigned char *buf;
  GdkWindow *window = GDK_ROOT_PARENT();
  GdkColormap *colormap;
  GdkGC* gc = gdk_gc_new (window);
  int i, j;
  GdkColor c;

  *pixmap = *mask = NULL;
  buf = load_bitmap_file (filename, &width, &height);
  if (!buf)
    return;

  colormap = gdk_window_get_colormap (window);
  *pixmap = gdk_pixmap_new (window, width, height, -1);

  typedef struct
  {
    GdkColor c;
    unsigned char *p;
  } PAL;

  GPtrArray* pal = g_ptr_array_new ();
  GdkColor** col = (GdkColor**)malloc (sizeof (GdkColor*) * height * width);
  guint32 k;

  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
    {
      unsigned char* p = &buf[(i*width+j)*3];

      for (k = 0; k < pal->len; k++)
      {
	PAL *pe = (PAL*)pal->pdata[k];
	if ((pe->p[0] == p[0]) &&
	    (pe->p[1] == p[1]) &&
	    (pe->p[2] == p[2]))
	{
	  col[(i*width+j)] = &pe->c;
	  break;
	}
      }

      if (k == pal->len)
      {
	PAL *pe = (PAL*)malloc (sizeof (PAL));

	pe->c.red = (gushort)(p[0]*0xFF);
	pe->c.green = (gushort)(p[1]*0xFF);
	pe->c.blue = (gushort)(p[2]*0xFF);
	gdk_color_alloc (colormap, &pe->c);
	col[(i*width+j)] = &pe->c;
	pe->p = p;
	g_ptr_array_add (pal, pe);
      }
    }

  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
    {
      /*
      c.red = (gushort)(buf[(i*width+j)*3]*0xFF);
      c.green = (gushort)(buf[(i*width+j)*3+1]*0xFF);
      c.blue = (gushort)(buf[(i*width+j)*3+2]*0xFF);
      gdk_color_alloc (colormap, &c);
      gdk_gc_set_foreground(gc, &c);
      gdk_draw_point(*pixmap, gc, j, i);
      */
      gdk_gc_set_foreground(gc, col[(i*width+j)]);
      gdk_draw_point(*pixmap, gc, j, i);
    }

  free (col);
  for (k = 0; k < pal->len; k++)
    free (pal->pdata[k]);
  g_ptr_array_free (pal, TRUE);

  gdk_gc_destroy (gc);
  *mask = gdk_pixmap_new (window, width, height, 1);
  gc = gdk_gc_new (*mask);

  for (i = 0; i < height; i++)
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
//      gdk_draw_point (*mask, gc, j, i);
      gdk_draw_line (*mask, gc, j, i, j + 1, i);
    }

  gdk_gc_destroy(gc);
  free (buf);
}

void CPixMap::load_pixmap (const char* filename, GdkPixmap **gdkpixmap, GdkBitmap **mask)
{
  CString str;

  str = g_strBitmapsPath;
  str += filename;
  bmp_to_pixmap (str.GetBuffer (),gdkpixmap, mask);

  if (*gdkpixmap == NULL)
  {
    char *dummy[] = { "1 1 1 1", "  c None", " " };
    *gdkpixmap = gdk_pixmap_create_from_xpm_d (GDK_ROOT_PARENT(), mask, NULL, dummy);
  }
}

// Load a xpm file and return a pixmap widget.
GtkWidget* CPixMap::new_pixmap (GtkWidget* widget, char* filename)
{
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

  load_pixmap (filename, &gdkpixmap, &mask);
  pixmap = gtk_pixmap_new (gdkpixmap, mask);

  gdk_pixmap_unref (gdkpixmap);
  gdk_pixmap_unref (mask);

 // g_FuncTable.m_pfnLoadBitmap( filename, &pixmap, &mask )

  return pixmap;
} 

GtkWidget* CPixMap::pixmap_from_char(GtkWidget *window, gchar **xpm_data)
{
	GdkPixmap *pixmap;
    GdkBitmap *mask;
	GtkStyle *style;
    GtkWidget *widget;

	style = gtk_widget_get_style( window );
    pixmap = gdk_pixmap_create_from_xpm_d( window->window,  &mask,
                                           &style->bg[GTK_STATE_NORMAL],
                                           (gchar **)xpm_data );

	widget = gtk_pixmap_new( pixmap, mask );

	return widget;
}
