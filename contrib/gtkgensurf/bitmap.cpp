/*
GenSurf plugin for GtkRadiant
Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gensurf.h"

void GenerateBitmapMapping ()
{
  double              value;
  double              C0, C1;
  double              x, y;
  int                 i, j;
  int                 O00,O01,O10,O11;
  int                 r0, r1, c0, c1;
  int                 color;
  unsigned char       *colors;

  if (!gbmp.colors)
    return;

  colors = gbmp.colors;

  for (j=0; j<=NV; j++)
  {
    y  = (double)(j*(gbmp.height-1))/(double)NV;
    r0 = (int)floor(y);
    r1 = (int)ceil(y);
    for (i=0; i<=NH; i++)
    {
      x = (double)(i*(gbmp.width-1))/(double)NH;
      c0 = (int)floor(x);
      c1 = (int)ceil(x);
      O00 = r0*gbmp.width + c0;
      O01 = r0*gbmp.width + c1;
      O10 = r1*gbmp.width + c0;
      O11 = r1*gbmp.width + c1;
      C0 = (double)colors[O00] + (double)(colors[O01]-colors[O00])*(x-(double)c0);
      C1 = (double)colors[O10] + (double)(colors[O11]-colors[O10])*(x-(double)c0);
      color = (int)(C0 + (C1-C0)*(y-r0));

      value = CalculateSnapValue(gbmp.black_value + color*((gbmp.white_value-gbmp.black_value)/255.));

      switch(Plane)
      {
      case PLANE_XZ0:
      case PLANE_XZ1:
        xyz[i][j].p[1] = value;
        break;
      case PLANE_YZ0:
      case PLANE_YZ1:
        xyz[i][j].p[0] = value;
        break;
      default:
        xyz[i][j].p[2] = value;
      }
    }
  }
}

static unsigned char* OpenBitmapFile ()
{
  int bmWidth;
  int bmHeight;
  unsigned char bmPlanes;
  unsigned char bmBitsPixel;
  unsigned char m1,m2;
  unsigned long sizeimage;
  short res1,res2;
  long filesize, pixoff;
  long bmisize, compression;
  long xscale, yscale;
  long colors, impcol;
  unsigned long m_bytesRead = 0;
  unsigned char *image;
  FILE *fp;

  fp = fopen (gbmp.name, "rb");
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
  if ((m1 != 'B') || (m2 != 'M'))
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

  rc = fread((long  *)&(bmWidth),4,1,fp); m_bytesRead+=4;
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

  if (bmBitsPixel != 8)
  {
    g_FuncTable.m_pfnMessageBox (g_pWnd, "This is not an 8-bit image. GenSurf can't use it.",
                                 "Bitmap", eMB_OK, eMB_ICONWARNING);
    fclose(fp);
    return NULL; 
  }

  if (colors == 0)
    colors = 1 << bmBitsPixel;

  if (bmBitsPixel != 24)
  {
    int i;
    for (i = 0; i < colors; i++)
    {
      unsigned char r ,g, b, dummy;

      rc = fread(&b, 1, 1, fp);
      m_bytesRead++;
      if (rc!=1)
      {
        fclose(fp);
        return NULL;
      }

      rc = fread(&g, 1, 1, fp); 
      m_bytesRead++;
      if (rc!=1)
      {
        fclose(fp);
        return NULL;
      }

      rc = fread(&r, 1, 1, fp); 
      m_bytesRead++;
      if (rc != 1)
      {
        fclose(fp);
        return NULL;
      }

      rc = fread(&dummy, 1, 1, fp); 
      m_bytesRead++;
      if (rc != 1)
      {
        fclose(fp);
        return NULL;
      }
    }
  }

  if ((long)m_bytesRead > pixoff)
  {
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
  image = (unsigned char*)malloc(w*h);

  if (image != NULL) 
  {
    gbmp.width = w;
    gbmp.height = h;
    unsigned char* outbuf = image;
    long row = 0;
    long rowOffset = 0;

    if (compression == 0) // BI_RGB
    {
      for (row = 0; row < bmHeight; row++)
      {
        // which row are we working on?
        rowOffset = (long unsigned)row*w;						      

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
                free(image);
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
            *(outbuf + rowOffset + col) = pix;
          }

          // read DWORD padding
          while ((m_bytesRead-pixoff)&3)
          {
            char dummy;
            if (fread(&dummy,1,1,fp)!=1)
            {
              free(image);
              fclose(fp);
              return NULL;
            }
            m_bytesRead++;
          }
        }
      }
    }
    else    // compression != 0
    {
      int i, x = 0;
      unsigned char c, c1 = 0, *pp;
      row = 0;
      pp = outbuf;

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
              *pp = c1; pp++;
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
              pp = outbuf + row*bmWidth;
            }
            else if (c == 0x01)
              break; // end of pic
            else if (c == 0x02) // delta
            {
              c = getc(fp);
              x += c;
              c = getc(fp);
              row += c;
              pp = outbuf + x + row*bmWidth;
            }
            else // absolute mode
            {
              for (i = 0; i < c; x++, i++)
              {
                c1 = getc(fp);
                *pp = c1; pp++;
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
              *pp = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f); pp++;
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
              pp = outbuf + bmHeight*bmWidth;
            }
            else if (c == 0x01)
              break; // end of pic
            else if (c == 0x02) // delta
            {
              c = getc(fp);
              x += c;
              c = getc(fp);
              row += c;
              pp = outbuf + x + row*bmWidth;
            }
            else // absolute mode
            {
              for (i = 0; i < c; x++, i++)
              {
                if ((i&1) == 0)
                  c1 = getc(fp);
                *pp = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f); pp++;
              }

              if (((c&3) == 1) || ((c&3) == 2))
                getc(fp); // odd length run: read an extra pad byte
            }
          }
        }
      }
    }
  }
  fclose(fp);
  return image;
}

bool OpenBitmap ()
{
  if (gbmp.colors)
    free (gbmp.colors);

  gbmp.colors = OpenBitmapFile ();

  if (!gbmp.colors)
  {
    char Text[256];

    sprintf (Text, "Error opening %s", gbmp.name);
    g_FuncTable.m_pfnMessageBox (g_pWnd, Text, "Bitmap", eMB_OK, eMB_ICONWARNING);
    strcpy (gbmp.name, "");
  }

  if (g_pWnd)
  {
    gtk_entry_set_text (GTK_ENTRY (g_object_get_data (G_OBJECT (g_pWnd), "bmp_file")), gbmp.name);
    gtk_widget_set_sensitive (GTK_WIDGET (g_object_get_data (G_OBJECT (g_pWnd), "bmp_reload")),
                              strlen (gbmp.name) ? TRUE : FALSE);

    UpdatePreview (true);
  }

  return (gbmp.colors != NULL);
}
