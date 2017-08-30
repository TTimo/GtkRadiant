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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

void save_window_pos( GtkWidget *wnd, window_position_t& pos ){
	if ( ( wnd == NULL ) || ( gtk_widget_get_window( wnd ) == NULL ) ) {
		return;
	}

	get_window_pos( wnd, &pos.x, &pos.y );

	pos.w = wnd->allocation.width;
	pos.h = wnd->allocation.height;

#ifdef DBG_WINDOWPOS
	//Sys_Printf("save_window_pos 'Window %s'\n",buf);
#endif
}

#ifdef _WIN32
void win32_get_window_pos( GtkWidget *widget, gint *x, gint *y ){
	if ( g_PrefsDlg.m_bStartOnPrimMon ) {
		RECT rc;
		POINT point;
		HWND xwnd = (HWND)GDK_WINDOW_HWND( gtk_widget_get_window( widget ) );
		const GdkRectangle primaryMonitorRect = g_pParentWnd->GetPrimaryMonitorRect();

		GetClientRect( xwnd,&rc );
		point.x = rc.left;
		point.y = rc.top;
		ClientToScreen( xwnd,&point );

		*x = point.x;
		*y = point.y;

		*x = max( *x,-widget->allocation.width + 10 );
		*x = min( *x,primaryMonitorRect.width - 10 );
		*y = max( *y,-widget->allocation.height + 10 );
		*y = min( *y,primaryMonitorRect.height - 10 );
	}
	else {
		// this is the same as the unix version of get_window_pos
		gdk_window_get_root_origin( gtk_widget_get_window( widget ), x, y );
	}
#ifdef DBG_WINDOWPOS
	Sys_Printf( "win32_get_window_pos %p %d,%d\n",widget,*x,*y );
#endif
}
#endif

void load_window_pos( GtkWidget *wnd, window_position_t& pos ){
#ifdef _WIN32
	const GdkRectangle primaryMonitorRect = g_pParentWnd->GetPrimaryMonitorRect();

	if ( pos.x < primaryMonitorRect.x
		 || pos.y < primaryMonitorRect.y
		 || pos.x > primaryMonitorRect.x + primaryMonitorRect.width
		 || pos.y > primaryMonitorRect.y + primaryMonitorRect.height ) {
		gtk_window_set_position( GTK_WINDOW( wnd ), GTK_WIN_POS_CENTER_ON_PARENT );
	}
#else
	// FIXME: not multihead safe
	if ( pos.x < 0
		 || pos.y < 0
		 || pos.x > gdk_screen_width()
		 || pos.y > gdk_screen_height() ) {
		gtk_window_set_position( GTK_WINDOW( wnd ), GTK_WIN_POS_CENTER_ON_PARENT );
	}
#endif
	else{
		gtk_window_move( GTK_WINDOW( wnd ), pos.x, pos.y );
	}

	gtk_window_set_default_size( GTK_WINDOW( wnd ), pos.w, pos.h );
#ifdef DBG_WINDOWPOS
	Sys_Printf( "load_window_pos %p 'Window,%s'\n",wnd,windowData );
#endif
}

gint widget_delete_hide( GtkWidget *widget ) {
  gtk_widget_hide( widget );
  return TRUE;
}


// Thanks to Mercury, Fingolfin - ETG
int readLongLE( FILE *file, unsigned long *m_bytesRead, int *value ){
	byte buf[4];
	int len = fread( buf, 4, 1, file );
	*m_bytesRead += 4;
	if ( len != 1 ) {
		return -1;
	}

	*value = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
	return 0;
}

short readShortLE( FILE *file, unsigned long *m_bytesRead, short unsigned *value ){
	byte buf[2];
	int len = fread( buf, 2, 1, file );
	*m_bytesRead += 2;
	if ( len != 1 ) {
		return -1;
	}

	*value = buf[0] | buf[1] << 8;
	return 0;
}

unsigned char *load_bitmap_file( const char* filename, guint16 *width, guint16 *height ){
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

	fp = fopen( filename,"rb" );
	if ( fp == NULL ) {
		return NULL;
	}

	size_t rc;
	rc = fread( &m1, 1, 1, fp );
	if ( rc != 1 ) {
		fclose( fp );
		return NULL;
	}
	m_bytesRead++;

	rc = fread( &m2, 1, 1, fp );
	if ( rc != 1) {
		fclose( fp );
		return NULL;
	}
	m_bytesRead++;

	if ( ( m1 != 'B' ) || ( m2 != 'M' ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&filesize ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readShortLE( fp,&m_bytesRead,&res1 ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readShortLE( fp,&m_bytesRead,&res2 ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&pixoff ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&bmisize ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&bmWidth ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&bmHeight ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readShortLE( fp,&m_bytesRead,&bmPlanes ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readShortLE( fp,&m_bytesRead,&bmBitsPixel ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&compression ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&sizeimage ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&xscale ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&yscale ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&colors ) ) {
		fclose( fp );
		return NULL;
	}

	if ( readLongLE( fp,&m_bytesRead,&impcol ) ) {
		fclose( fp );
		return NULL;
	}

	if ( colors == 0 ) {
		colors = 1 << bmBitsPixel;
	}

	RGBQUAD *colormap = NULL;
	if ( bmBitsPixel != 24 ) {
		colormap = new RGBQUAD[colors];
		if ( colormap == NULL ) {
			fclose( fp );
			return NULL;
		}

		int i;
		for ( i = 0; i < colors; i++ )
		{
			unsigned char r,g, b, dummy;

			rc = fread( &b, 1, 1, fp );
			m_bytesRead++;
			if ( rc != 1 ) {
				delete [] colormap;
				fclose( fp );
				return NULL;
			}

			rc = fread( &g, 1, 1, fp );
			m_bytesRead++;
			if ( rc != 1 ) {
				delete [] colormap;
				fclose( fp );
				return NULL;
			}

			rc = fread( &r, 1, 1, fp );
			m_bytesRead++;
			if ( rc != 1 ) {
				delete [] colormap;
				fclose( fp );
				return NULL;
			}

			rc = fread( &dummy, 1, 1, fp );
			m_bytesRead++;
			if ( rc != 1 ) {
				delete [] colormap;
				fclose( fp );
				return NULL;
			}

			colormap[i].rgbRed = r;
			colormap[i].rgbGreen = g;
			colormap[i].rgbBlue = b;
		}
	}

	if ( (long)m_bytesRead > pixoff ) {
		delete [] colormap;
		fclose( fp );
		return NULL;
	}

	while ( (long)m_bytesRead < pixoff )
	{
		char dummy;
		fread( &dummy,1,1,fp );
		m_bytesRead++;
	}

	int w = bmWidth;
	int h = bmHeight;

	// set the output params
	imagebits = (unsigned char *)malloc( w * h * 3 );
	long row_size = w * 3;

	if ( imagebits != NULL ) {
		*width = w;
		*height = h;
		unsigned char *outbuf = imagebits;
		long row = 0;
		long rowOffset = 0;

		if ( compression == 0 ) { // BI_RGB
			// read rows in reverse order
			for ( row = bmHeight - 1; row >= 0; row-- )
			{
				// which row are we working on?
				rowOffset = (long unsigned)row * row_size;

				if ( bmBitsPixel == 24 ) {
					for ( int col = 0; col < w; col++ )
					{
						long offset = col * 3;
						char pixel[3];

						if ( fread( (void *)( pixel ),1,3,fp ) == 3 ) {
							// we swap red and blue here
							*( outbuf + rowOffset + offset + 0 ) = pixel[2]; // r
							*( outbuf + rowOffset + offset + 1 ) = pixel[1]; // g
							*( outbuf + rowOffset + offset + 2 ) = pixel[0]; // b
						}
					}
					m_bytesRead += row_size;

					// read DWORD padding
					while ( ( m_bytesRead - pixoff ) & 3 )
					{
						char dummy;
						if ( fread( &dummy,1,1,fp ) != 1 ) {
							free( imagebits );
							fclose( fp );
							return NULL;
						}
						m_bytesRead++;
					}
				}
				else
				{
					// pixels are packed as 1 , 4 or 8 bit vals. need to unpack them
					int bit_count = 0;
					unsigned long mask = ( 1 << bmBitsPixel ) - 1;
					unsigned char inbyte = 0;

					for ( int col = 0; col < w; col++ )
					{
						int pix = 0;

						// if we need another byte
						if ( bit_count <= 0 ) {
							bit_count = 8;
							if ( fread( &inbyte,1,1,fp ) != 1 ) {
								free( imagebits );
								delete [] colormap;
								fclose( fp );
								return NULL;
							}
							m_bytesRead++;
						}

						// keep track of where we are in the bytes
						bit_count -= bmBitsPixel;
						pix = ( inbyte >> bit_count ) & mask;

						// lookup the color from the colormap - stuff it in our buffer
						// swap red and blue
						*( outbuf + rowOffset + col * 3 + 2 ) = colormap[pix].rgbBlue;
						*( outbuf + rowOffset + col * 3 + 1 ) = colormap[pix].rgbGreen;
						*( outbuf + rowOffset + col * 3 + 0 ) = colormap[pix].rgbRed;
					}

					// read DWORD padding
					while ( ( m_bytesRead - pixoff ) & 3 )
					{
						char dummy;
						if ( fread( &dummy,1,1,fp ) != 1 ) {
							free( imagebits );
							if ( colormap ) {
								delete [] colormap;
							}
							fclose( fp );
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
			pp = outbuf + ( bmHeight - 1 ) * bmWidth * 3;

			if ( bmBitsPixel == 8 ) {
				while ( row < bmHeight )
				{
					c = getc( fp );

					if ( c ) {
						// encoded mode
						c1 = getc( fp );
						for ( i = 0; i < c; x++, i++ )
						{
							*pp = colormap[c1].rgbRed; pp++;
							*pp = colormap[c1].rgbGreen; pp++;
							*pp = colormap[c1].rgbBlue; pp++;
						}
					}
					else
					{
						// c==0x00,  escape codes
						c = getc( fp );
						if ( c == 0x00 ) { // end of line
							row++;
							x = 0;
							pp = outbuf + ( bmHeight - row - 1 ) * bmWidth * 3;
						}
						else if ( c == 0x01 ) {
							break; // end of pic
						}
						else if ( c == 0x02 ) { // delta
							c = getc( fp );
							x += c;
							c = getc( fp );
							row += c;
							pp = outbuf + x * 3 + ( bmHeight - row - 1 ) * bmWidth * 3;
						}
						else // absolute mode
						{
							for ( i = 0; i < c; x++, i++ )
							{
								c1 = getc( fp );
								*pp = colormap[c1].rgbRed; pp++;
								*pp = colormap[c1].rgbGreen; pp++;
								*pp = colormap[c1].rgbBlue; pp++;
							}

							if ( c & 1 ) {
								getc( fp ); // odd length run: read an extra pad byte
							}
						}
					}
				}
			}
			else if ( bmBitsPixel == 4 ) {
				while ( row < bmHeight )
				{
					c = getc( fp );

					if ( c ) {
						// encoded mode
						c1 = getc( fp );
						for ( i = 0; i < c; x++, i++ )
						{
							*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbRed; pp++;
							*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbGreen; pp++;
							*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbBlue; pp++;
						}
					}
					else
					{
						// c==0x00,  escape codes
						c = getc( fp );

						if ( c == 0x00 ) { // end of line
							row++;
							x = 0;
							pp = outbuf + ( bmHeight - row - 1 ) * bmWidth * 3;
						}
						else if ( c == 0x01 ) {
							break; // end of pic
						}
						else if ( c == 0x02 ) { // delta
							c = getc( fp );
							x += c;
							c = getc( fp );
							row += c;
							pp = outbuf + x * 3 + ( bmHeight - row - 1 ) * bmWidth * 3;
						}
						else // absolute mode
						{
							for ( i = 0; i < c; x++, i++ )
							{
								if ( ( i & 1 ) == 0 ) {
									c1 = getc( fp );
								}
								*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbRed; pp++;
								*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbGreen; pp++;
								*pp = colormap[( i & 1 ) ? ( c1 & 0x0f ) : ( ( c1 >> 4 ) & 0x0f )].rgbBlue; pp++;
							}

							if ( ( ( c & 3 ) == 1 ) || ( ( c & 3 ) == 2 ) ) {
								getc( fp ); // odd length run: read an extra pad byte
							}
						}
					}
				}
			}
		}
		if ( colormap ) {
			delete [] colormap;
		}

		fclose( fp );
	}
	return imagebits;
}

void bmp_to_pixmap( const char* filename, GdkPixbuf **pixmap ){
	GError *gerror = NULL;

	*pixmap = gdk_pixbuf_new_from_file( filename, &gerror );
	if ( *pixmap == NULL ) {
//		Sys_FPrintf( SYS_ERR, "ERROR: Failed to load bmp %s: %s, creating default pixmap.\n", filename, gerror->message );
		g_error_free( gerror );
	}
}

void load_pixmap( const char* filename, GtkWidget **widget, GdkPixbuf **pixmap ){
	CString str;
	GError *gerror = NULL;

	str = g_strBitmapsPath;
	str += filename;

	*pixmap = gdk_pixbuf_new_from_file( str.GetBuffer(), &gerror );
	if ( *pixmap == NULL ) {
		Sys_FPrintf( SYS_ERR, "ERROR: Failed to load_pixmap %s: %s, creating default pixmap.\n", str.GetBuffer(), gerror->message );
		g_error_free( gerror );
	}
	*widget = gtk_image_new_from_pixbuf( *pixmap );
	gtk_widget_show( *widget );
	if ( *pixmap ) {
		g_object_unref( *pixmap );
	}
}

// this is the same as above but used by the plugins
bool WINAPI load_plugin_bitmap( const char* filename, void **gdkpixmap, void **mask ){
	CString str;

	str = g_strGameToolsPath;
	str += g_strPluginsDir;
	str += "bitmaps/";
	str += filename;
	bmp_to_pixmap( str.GetBuffer(), (GdkPixbuf **)gdkpixmap );

	if ( *gdkpixmap == NULL ) {
		// look in the core plugins
		str = g_strAppPath;
		str += g_strPluginsDir;
		str += "bitmaps/";
		str += filename;
		bmp_to_pixmap( str.GetBuffer(), (GdkPixbuf **)gdkpixmap );

		if ( *gdkpixmap == NULL ) {

			// look in core modules
			str = g_strAppPath;
			str += g_strModulesDir;
			str += "bitmaps/";
			str += filename;
			bmp_to_pixmap( str.GetBuffer(), (GdkPixbuf **)gdkpixmap );

			if ( *gdkpixmap == NULL ) {
				Sys_FPrintf( SYS_ERR, "ERROR: Failed to load bitmap %s, creating default.\n", filename );
				*gdkpixmap = NULL;
				return false;
			}
		}
	}
	return true;
}

GtkWidget* new_pixmap( GtkWidget* widget, const char* filename ){
	return gtk_image_new_from_file( filename );
}

GtkWidget* new_image_icon( const char* filename ) {
	CString str;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;
	GError *gerror = NULL;

	str = g_strBitmapsPath;
	str += filename;

	pixbuf = gdk_pixbuf_new_from_file( str.GetBuffer(), &gerror );
	if( pixbuf != NULL ) {
		icon = gtk_image_new_from_pixbuf( pixbuf );
		g_object_unref( pixbuf );
	} else {
		Sys_FPrintf( SYS_ERR, "ERROR: Failed to load bitmap: %s, %s\n", str.GetBuffer(), gerror->message );
		g_error_free( gerror );
		icon = gtk_image_new_from_file( filename );
	}
	gtk_widget_show( icon );

	return icon;
}

GtkWidget* new_plugin_image_icon( const char* filename ) {
	CString str;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;
	GError *gerror = NULL;

	str = g_strAppPath;
	str += g_strModulesDir;
	str += "bitmaps/";
	str += filename;

	pixbuf = gdk_pixbuf_new_from_file( str.GetBuffer(), &gerror );
	if( pixbuf == NULL ) {
		Sys_FPrintf( SYS_ERR, "ERROR: Failed to load plugin bitmap: %s, %s\n", str.GetBuffer(), gerror->message );
		g_error_free( gerror );
	}
	//manually add transparency to bmp files
	if( strlen( filename ) > 4 && strcmp( filename + strlen( filename ) - 4, ".bmp" ) == 0 && pixbuf && !gdk_pixbuf_get_has_alpha( pixbuf ) ) {
		GdkPixbuf *apixbuf;
		apixbuf = gdk_pixbuf_add_alpha( pixbuf, TRUE, 255, 0, 255 );
		g_object_unref( pixbuf );
		pixbuf = apixbuf;
	}
	icon = gtk_image_new_from_pixbuf( pixbuf );
	gtk_widget_show( icon );
	if( pixbuf ) {
		g_object_unref( pixbuf );
	}
	return icon;
}

// =============================================================================
// Menu stuff

GtkWidget* menu_separator( GtkWidget *menu ){
	GtkWidget *menu_item = gtk_menu_item_new();
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), menu_item );
	gtk_widget_set_sensitive( menu_item, FALSE );
	gtk_widget_show( menu_item );
	return menu_item;
}

GtkWidget* menu_tearoff( GtkWidget *menu ){
	GtkWidget *menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), menu_item );
// gtk_widget_set_sensitive (menu_item, FALSE); -- controls whether menu is detachable
	gtk_widget_show( menu_item );
	return menu_item;
}

GtkWidget* create_sub_menu_with_mnemonic( GtkWidget *bar, const gchar *mnemonic ){
	GtkWidget *item, *sub_menu;

	item = gtk_menu_item_new_with_mnemonic( mnemonic );

	sub_menu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), sub_menu );

	gtk_menu_shell_append( GTK_MENU_SHELL( bar ), item );
//	gtk_container_add( GTK_CONTAINER( bar ), item );

	gtk_widget_show( item );
	return sub_menu;
}

extern void AddMenuItem( GtkWidget* menu, unsigned int id );

GtkWidget* create_menu_item_with_mnemonic( GtkWidget *menu, const gchar *mnemonic, GCallback func, int id ){
	GtkWidget *item;

	item = gtk_menu_item_new_with_mnemonic( mnemonic );

	gtk_widget_show( item );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
//	gtk_container_add( GTK_CONTAINER( menu ), item );
	g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( func ), GINT_TO_POINTER( id ) );

	AddMenuItem( item, id );
	return item;
}

GtkWidget* create_check_menu_item_with_mnemonic( GtkWidget *menu, const gchar *mnemonic, GCallback func, int id, gboolean active ){
	GtkWidget *item;

	item = gtk_check_menu_item_new_with_mnemonic( mnemonic );

	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), active );
	gtk_widget_show( item );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
//	gtk_container_add( GTK_CONTAINER( menu ), item );
	g_signal_connect( G_OBJECT( item ), "activate", func, GINT_TO_POINTER( id ) );

	AddMenuItem( item, id );
	return item;
}

GtkWidget* create_radio_menu_item_with_mnemonic( GtkWidget *menu, GtkWidget *last, const gchar *mnemonic, GCallback func, int id, gboolean state ){
	GtkWidget *item;
	GSList *group = (GSList*)NULL;

	if ( last != NULL ) {
		group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM( last ) );
	}
	item = gtk_radio_menu_item_new_with_mnemonic( group, mnemonic );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), state );

	gtk_widget_show( item );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
//	gtk_container_add( GTK_CONTAINER( menu ), item );
	g_signal_connect( G_OBJECT( item ), "activate", func, GINT_TO_POINTER( id ) );

	AddMenuItem( item, id );
	return item;
}

GtkWidget* create_menu_in_menu_with_mnemonic( GtkWidget *menu, const gchar *mnemonic ){
	GtkWidget *item, *submenu;

	item = gtk_menu_item_new_with_mnemonic( mnemonic );

	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), submenu );
//	gtk_container_add( GTK_CONTAINER( menu ), item );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
	
	gtk_widget_show( item );
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
	*ret = GPOINTER_TO_INT( data );
}

gint dialog_delete_callback( GtkWidget *widget, GdkEvent* event, gpointer data ){
	int *loop;

	gtk_widget_hide( widget );
	loop = (int*)g_object_get_data( G_OBJECT( widget ), "loop" );
	*loop = 0;

	return TRUE;
}

gint dialog_url_callback( GtkWidget *widget, GdkEvent* event, gpointer data ){
	OpenURL( widget, (const char *)g_object_get_data( G_OBJECT( widget ), "URL" ) );

	return TRUE;
}

// helper fcn for gtk_MessageBox
static GtkWidget * gtk_AddDlgButton( GtkWidget *container, const char *label, 
									 const int clickSignal, qboolean setGrabDefault ) {
	GtkWidget *btn = gtk_button_new_with_label( _( label ) );
	gtk_box_pack_start( GTK_BOX( container ), btn, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( btn ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( clickSignal ) );
	gtk_widget_set_can_default( btn, TRUE );
	
	if( setGrabDefault ) 
		gtk_widget_grab_default( btn );
	
	gtk_widget_show( btn );

	return btn;
}

static const int MSGBOX_PAD_MAJOR = 8;
static const int MSGBOX_PAD_MINOR = 2;

//! @note kaz 05/09/2012 I think we could use GtkMessageDialog, but this works too.
//! @todo kaz 05/09/2012 Need to just replace the old fcn and make sure everything 
//! that uses it still works...
int WINAPI gtk_MessageBoxNew( void *parent, const char *message, 
						      const char *caption, const guint32 flags, 
							  const char *URL ) {
	GtkWidget *dialog, *content_area;
	gint response_id;
	int ret;
	GtkDialogFlags dialog_flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( caption, NULL, dialog_flags, NULL );

	gtk_container_set_border_width( GTK_CONTAINER( dialog ), MSGBOX_PAD_MAJOR );

	if( parent ) {
		gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( parent ) );
		gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	}

	GtkAccelGroup *accel = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	// begin layout
	GtkWidget *outer_vbox = gtk_vbox_new( FALSE, MSGBOX_PAD_MAJOR );
	gtk_container_add( GTK_CONTAINER( content_area ), outer_vbox );
	gtk_widget_show( outer_vbox );

	// add icon
	GtkWidget *dlg_icon;

	//! @note kaz 05/09/2012 only support commonly used icons, fill out others as needed
	switch( flags & MB_ICONMASK ) {
	case MB_ICONHAND: {
		dlg_icon = gtk_image_new_from_icon_name( "dialog-error", GTK_ICON_SIZE_DIALOG );
		break;
	}
	case MB_ICONQUESTION: {
		dlg_icon = gtk_image_new_from_icon_name( "dialog-question", GTK_ICON_SIZE_DIALOG );
		break;
	}
	case MB_ICONEXCLAMATION: {
		dlg_icon = gtk_image_new_from_icon_name( "dialog-warning", GTK_ICON_SIZE_DIALOG );
		break;
	}
	case MB_ICONINFORMATION:
	default: {
		dlg_icon = gtk_image_new_from_icon_name( "dialog-information", GTK_ICON_SIZE_DIALOG );
		break;
	}
#if(WINVER >= 0x0400)
	case MB_USERICON: {
		//dlg_icon = gtk_image_new_from_icon_name( ?????, GTK_ICON_SIZE_DIALOG );
		break;
	}
#endif
	}

	GtkWidget *icon_text_hbox = gtk_hbox_new( FALSE, MSGBOX_PAD_MAJOR );
	gtk_box_pack_start( GTK_BOX( outer_vbox ), icon_text_hbox, FALSE, FALSE, MSGBOX_PAD_MINOR );
	gtk_widget_show( icon_text_hbox );

	gtk_box_pack_start( GTK_BOX( icon_text_hbox ), dlg_icon, FALSE, FALSE, MSGBOX_PAD_MINOR );
	gtk_widget_show( dlg_icon );

	// add message
	GtkWidget *dlg_msg = gtk_label_new( message );
	gtk_box_pack_start( GTK_BOX( icon_text_hbox ), dlg_msg, FALSE, FALSE, MSGBOX_PAD_MINOR );
	gtk_misc_set_alignment( GTK_MISC( dlg_msg ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( dlg_msg ), GTK_JUSTIFY_LEFT );
	gtk_label_set_selectable( GTK_LABEL( dlg_msg ), TRUE );
	gtk_widget_show( dlg_msg );

	// add buttons
	GtkWidget *hsep = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( outer_vbox ), hsep, FALSE, FALSE, MSGBOX_PAD_MINOR );
	gtk_widget_show( hsep );

	GtkWidget *buttons_hbox = gtk_hbox_new( FALSE, MSGBOX_PAD_MAJOR ); 
	gtk_box_pack_start( GTK_BOX( outer_vbox ), buttons_hbox, FALSE, FALSE, MSGBOX_PAD_MINOR );
	gtk_widget_show( buttons_hbox );

	//! @note kaz 05/09/2012 only support commonly used types, fill out others as needed
	switch( flags & MB_TYPEMASK ) {
	case MB_OK:
	default: {
		GtkWidget *btn_ok = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		gtk_widget_set_can_default( btn_ok, TRUE );
		gtk_widget_add_accelerator( btn_ok, "clicked", accel, GDK_KEY_Escape, (GdkModifierType)0, (GtkAccelFlags)0 );
		gtk_widget_add_accelerator( btn_ok, "clicked", accel, GDK_KEY_Return, (GdkModifierType)0, (GtkAccelFlags)0 );
		ret = IDOK;
		break;
	}
	case MB_OKCANCEL: {
		GtkWidget *btn_ok = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		gtk_widget_add_accelerator( btn_ok, "clicked", accel, GDK_KEY_Return, (GdkModifierType)0, (GtkAccelFlags)0 );
		GtkWidget *btn_cancel = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
		gtk_widget_add_accelerator( btn_cancel, "clicked", accel, GDK_KEY_Escape, (GdkModifierType)0, (GtkAccelFlags)0 );
		ret = IDCANCEL;
		break;
	}
	case MB_ABORTRETRYIGNORE: {
		//! @todo fill out
		break;
	}
	case MB_YESNOCANCEL: {
		//! @todo accelerators?
		gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );
		gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
		ret = IDCANCEL;
		break;
	}
	case MB_YESNO: {
		//! @todo accelerators?
		gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );
		ret = IDNO;
		break;
	}
	case MB_RETRYCANCEL: {
		//! @todo fill out
		break;
	}
#if(WINVER >= 0x0500)
	case MB_CANCELTRYCONTINUE: {
		//! @todo fill out
		break;
	}
#endif
	}

	// optionally add URL button
	if( URL ) {
		GtkWidget *btn_url = gtk_button_new_with_label( _( "Go to URL" ) );
		gtk_box_pack_start( GTK_BOX( buttons_hbox ), btn_url, TRUE, TRUE, 0 ); 
		g_signal_connect( G_OBJECT( btn_url ), "clicked",
							G_CALLBACK( dialog_url_callback ), NULL );
		g_object_set_data( G_OBJECT( btn_url ), "URL", (void *)URL );
		gtk_widget_show( btn_url );
	}


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	switch( response_id ) {
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	case GTK_RESPONSE_CANCEL:
		ret = IDCANCEL;
		break;
	case GTK_RESPONSE_YES:
		ret = IDYES;
		break;
	case GTK_RESPONSE_NO:
		ret = IDNO;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}


int WINAPI gtk_MessageBox( void *parent, const char* lpText, const char* lpCaption, guint32 uType, const char* URL ){
	GtkWidget *dialog, *w, *vbox, *hbox, *content_area;
	GtkAccelGroup *accel;
	gint response_id;
	int mode, ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( lpCaption, NULL, flags, NULL );

	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 10 );

	if ( parent != NULL ) {
		gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( parent ) );
	}

	accel = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	w = gtk_label_new( lpText );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( w ), GTK_JUSTIFY_LEFT );
	gtk_label_set_selectable( GTK_LABEL( w ), TRUE );
	gtk_widget_show( w );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	mode = ( uType & MB_TYPEMASK );
	if ( mode == MB_OK ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		gtk_widget_add_accelerator( w, "clicked", accel, GDK_KEY_Escape, (GdkModifierType)0, (GtkAccelFlags)0 );
		gtk_widget_add_accelerator( w, "clicked", accel, GDK_KEY_Return, (GdkModifierType)0, (GtkAccelFlags)0 );
		gtk_widget_set_can_default( w, TRUE );
		gtk_widget_grab_default( w );

		ret = IDOK;
	}
	else if ( mode ==  MB_OKCANCEL ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		gtk_widget_add_accelerator( w, "clicked", accel, GDK_KEY_Return, (GdkModifierType)0, (GtkAccelFlags)0 );

		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
		gtk_widget_add_accelerator( w, "clicked", accel, GDK_KEY_Escape, (GdkModifierType)0, (GtkAccelFlags)0 );

		ret = IDCANCEL;
	}
	else if ( mode == MB_YESNOCANCEL ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

		ret = IDCANCEL;
	}
	else /* if (mode == MB_YESNO) */
	{
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );

		ret = IDNO;
	}

	if ( URL ) {
		w = gtk_button_new_with_label( _( "Go to URL" ) );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
		g_signal_connect( G_OBJECT( w ), "clicked",
							G_CALLBACK( dialog_url_callback ), NULL );
		g_object_set_data( G_OBJECT( w ), "URL", (void *)URL );
		gtk_widget_show( w );
	}


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	switch( response_id ) {
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	case GTK_RESPONSE_CANCEL:
		ret = IDCANCEL;
		break;
	case GTK_RESPONSE_YES:
		ret = IDYES;
		break;
	case GTK_RESPONSE_NO:
		ret = IDNO;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// File dialog

// fenris #3078 WHENHELLISFROZENOVER

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
	void operator=( const filetype_t& other ){
		m_name = other.name;
		m_pattern = other.pattern;
	}
	string_t m_name;
	string_t m_pattern;
};
public:
CFileType(){
	m_nTypes = 0;
	m_pTypes = NULL;
	m_strWin32Filters = NULL;
	m_pstrGTKMasks = NULL;
}

virtual ~CFileType(){
	delete[] m_pTypes;
	DestroyWin32Filters();
	DestroyGTKMasks();
}

void addType( filetype_t type ){
	filetype_copy_t* newTypes = new filetype_copy_t [m_nTypes + 1];
	if ( m_nTypes > 0 ) {
		for ( int i = 0; i < m_nTypes; i++ )
			newTypes[i] = m_pTypes[i];
		delete[] m_pTypes;
	}
	m_pTypes = newTypes;
	m_pTypes[m_nTypes] = type;
	m_nTypes++;
	ConstructGTKMasks();
	ConstructWin32Filters();
}

filetype_t GetTypeForWin32Filter( const char *filter ) const {
	for ( int i = 0; i < m_nTypes; i++ )
		if ( strcmp( m_pTypes[i].m_pattern.c_str(), filter ) == 0 ) {
			return filetype_t( m_pTypes[i].m_name.c_str(), m_pTypes[i].m_pattern.c_str() );
		}
	return filetype_t();
}

filetype_t GetTypeForGTKMask( const char *mask ) const {
	for ( int i = 0; i < m_nTypes; i++ )
		if ( strcmp( m_pstrGTKMasks[i],mask ) == 0 ) {
			return filetype_t( m_pTypes[i].m_name.c_str(), m_pTypes[i].m_pattern.c_str() );
		}
	return filetype_t();
}

int GetNumTypes(){
	return m_nTypes;
}

filetype_t GetTypeForIndex( int index ) const // Zero-based index.
{
	if ( index >= 0 && index < m_nTypes ) {
		return filetype_t( m_pTypes[index].m_name.c_str(), m_pTypes[index].m_pattern.c_str() );
	}
	return filetype_t();
}

char *m_strWin32Filters;
char **m_pstrGTKMasks;
private:
int m_nTypes;
filetype_copy_t *m_pTypes;

void DestroyWin32Filters(){
	delete[] m_strWin32Filters;
}

void ConstructWin32Filters(){
	const char *r;
	char *w;
	int i;
	int len = 0;
	DestroyWin32Filters();
	for ( i = 0; i < m_nTypes; i++ )
		len = len + strlen( m_pTypes[i].m_name.c_str() ) + strlen( m_pTypes[i].m_pattern.c_str() ) * 2 + 5;
	m_strWin32Filters = new char[len + 1]; // length + null char
	for ( i = 0, w = m_strWin32Filters; i < m_nTypes; i++ )
	{
		for ( r = m_pTypes[i].m_name.c_str(); *r != '\0'; r++, w++ )
			*w = *r;
		*w++ = ' ';
		*w++ = '(';
		for ( r = m_pTypes[i].m_pattern.c_str(); *r != '\0'; r++, w++ )
			*w = *r;
		*w++ = ')';
		*w++ = '\0';
		for ( r = m_pTypes[i].m_pattern.c_str(); *r != '\0'; r++, w++ )
			*w = ( *r == ',' ) ? ';' : *r;
		*w++ = '\0';
	}
	m_strWin32Filters[len] = '\0';
}

void DestroyGTKMasks(){
	if ( m_pstrGTKMasks != NULL ) {
		for ( char **p = m_pstrGTKMasks; *p != NULL; p++ )
			delete[] *p;
	}
	delete[] m_pstrGTKMasks;
}

void ConstructGTKMasks(){
	const char *r;
	char *w;
	int i;
	int len = 0;
	DestroyGTKMasks();
	m_pstrGTKMasks = new char*[m_nTypes + 1];
	for ( i = 0; i < m_nTypes; i++ )
	{
		len = strlen( m_pTypes[i].m_name.c_str() ) + strlen( m_pTypes[i].m_pattern.c_str() ) + 3;
		m_pstrGTKMasks[i] = new char[len + 1]; // length + null char
		w = m_pstrGTKMasks[i];
		for ( r = m_pTypes[i].m_name.c_str(); *r != '\0'; r++, w++ )
			*w = *r;
		*w++ = ' ';
		*w++ = '(';
		for ( r = m_pTypes[i].m_pattern.c_str(); *r != '\0'; r++, w++ )
			*w = *r;
		*w++ = ')';
		*w++ = '\0';
	}
	m_pstrGTKMasks[m_nTypes] = NULL;
}

};

#ifdef _WIN32

typedef struct {
	gboolean open;
	OPENFILENAME *ofn;
	BOOL dlgRtnVal;
	bool done;
} win32_native_file_dialog_comms_t;

DWORD WINAPI win32_native_file_dialog_thread_func( LPVOID lpParam ){
	win32_native_file_dialog_comms_t *fileDialogComms;
	fileDialogComms = (win32_native_file_dialog_comms_t *) lpParam;
	if ( fileDialogComms->open ) {
		fileDialogComms->dlgRtnVal = GetOpenFileName( fileDialogComms->ofn );
	}
	else {
		fileDialogComms->dlgRtnVal = GetSaveFileName( fileDialogComms->ofn );
	}
	fileDialogComms->done = true; // No need to synchronize around lock, one-way gate.
	return 0;
}

#endif

/**
 * @param[in] baseSubDir should have a trailing slash if not @c NULL
 */
const char* file_dialog( void *parent, gboolean open, const char* title, const char* path, const char* pattern, const char *baseSubDir ){

#ifdef _WIN32
	static bool in_file_dialog = false;
	HANDLE fileDialogThreadHandle;
	win32_native_file_dialog_comms_t fileDialogComms;
	bool dialogDone;
#endif

	// Gtk dialog
	GtkFileChooserAction action;
	GtkWidget* file_sel;
	char *new_path = NULL;

	const char* r;
	char *v, *w;
	filetype_t type;
	CFileType typelist;
	if ( pattern != NULL ) {
		GetFileTypeRegistry()->getTypeList( pattern, &typelist );
	}

#ifdef _WIN32
	if ( g_PrefsDlg.m_bNativeGUI ) {
		// do that the native way

		if ( in_file_dialog ) {
			return NULL;             // Avoid recursive entry.
		}
		in_file_dialog = true;
		/* Set the members of the OPENFILENAME structure. */
		// See http://msdn.microsoft.com/en-us/library/ms646839%28v=vs.85%29.aspx .
		memset( &ofn, 0, sizeof( ofn ) );
		ofn.lStructSize = sizeof( ofn );
		ofn.hwndOwner = (HWND)GDK_WINDOW_HWND( gtk_widget_get_window( g_pParentWnd->m_pWidget ) );
		ofn.nFilterIndex = 1; // The index is 1-based, not 0-based.  This basically says,
		                      // "select the first filter as default".
		if ( pattern ) {
			ofn.lpstrFilter = typelist.m_strWin32Filters;
		}
		else
		{
			// TODO: Would be a bit cleaner if we could extract this string from
			// GetFileTypeRegistry() instead of hardcoding it here.
			ofn.lpstrFilter = "all files\0*.*\0"; // Second '\0' will be added to end of string.
		}
		szFile[0] = '\0';
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		if ( path ) {
			// szDirName: Radiant uses unix convention for paths internally
			//   Win32 (of course) and Gtk (who would have thought) expect the '\\' convention
			// copy path, replacing dir separators as appropriate
			for ( r = path, w = szDirName; *r != '\0'; r++ )
				*w++ = ( *r == '/' ) ? '\\' : *r;
			// terminate string
			*w = '\0';
			ofn.lpstrInitialDir = szDirName;
		}
		ofn.lpstrTitle = title;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		memset( &fileDialogComms, 0, sizeof( fileDialogComms ) );
		fileDialogComms.open = open;
		fileDialogComms.ofn = &ofn;

		fileDialogThreadHandle =
			CreateThread( NULL, // lpThreadAttributes
						  0, // dwStackSize, default stack size
						  win32_native_file_dialog_thread_func, // lpStartAddress, funcion to call
						  &fileDialogComms, // lpParameter, argument to pass to function
						  0, // dwCreationFlags
						  NULL ); // lpThreadId

		dialogDone = false;
		while ( 1 ) {
			// Avoid blocking indefinitely.  Another thread will set fileDialogComms->done to true;
			// we don't want to be in an indefinite blocked state when this happens.  We want to break
			// out of here eventually.
			while ( gtk_events_pending() ) {
				gtk_main_iteration();
			}
			if ( dialogDone ) {
				break;
			}
			if ( fileDialogComms.done ) {
				dialogDone = true;                 // One more loop of gtk_main_iteration() to get things in sync.
			}
			// Avoid tight infinte loop, add a small amount of sleep.
			Sleep( 10 );
		}
		// Make absolutely sure that the thread is finished before we call CloseHandle().
		WaitForSingleObject( fileDialogThreadHandle, INFINITE );
		CloseHandle( fileDialogThreadHandle );

		in_file_dialog = false;

		if ( !fileDialogComms.dlgRtnVal ) {
			return NULL; // Cancelled.
		}

		if ( pattern != NULL ) {
			type = typelist.GetTypeForIndex( ofn.nFilterIndex - 1 );
		}

	}
	else
	{
#endif
	char buf[PATH_MAX];
	// do that the Gtk way
	if ( title == NULL ) {
		title = open ? _( "Open File" ) : _( "Save File" );
	}

	// we expect an actual path below, if the path is NULL we might crash
	if ( !path || path[0] == '\0' ) {
		strcpy( buf, g_pGameDescription->mEnginePath.GetBuffer() );
		strcat( buf, g_pGameDescription->mBaseGame.GetBuffer() );
		strcat( buf, "/" );
		if ( baseSubDir ) {
			strcat( buf, baseSubDir );
		}
		path = buf;
	}

	// alloc new path with extra char for dir separator
	new_path = new char[strlen( path ) + 1 + 1];
	// copy path, replacing dir separators as appropriate
	for ( r = path, w = new_path; *r != '\0'; r++ )
		*w++ = ( *r == '/' ) ? G_DIR_SEPARATOR : *r;
	// add dir separator to end of path if required
	if ( *( w - 1 ) != G_DIR_SEPARATOR ) {
		*w++ = G_DIR_SEPARATOR;
	}
	// terminate string
	*w = '\0';

	action = open ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE;
	file_sel = gtk_file_chooser_dialog_new( title, GTK_WINDOW( parent ), action, NULL, (char*)NULL );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), open ? _( "Open" ) : _("Save" ), GTK_RESPONSE_ACCEPT );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( file_sel ), new_path );
	delete[] new_path;

	// Setting the file chooser dialog to modal and centering it on the parent is done automatically.

	if ( pattern != NULL ) {
		//GtkFileFilter *allTypesFilter = gtk_file_filter_new();
		// http://www.gtkforums.com/viewtopic.php?p=6044
		//gtk_file_filter_set_name( allTypesFilter, "All supported types" );
		for ( int i = 0; i < typelist.GetNumTypes(); i++ ) {
			GtkFileFilter *filter = gtk_file_filter_new();
			type = typelist.GetTypeForIndex( i );
			// We can use type.name here, or m_pstrGTKMasks[i], which includes the actual pattern.
			gtk_file_filter_set_name( filter, typelist.m_pstrGTKMasks[i] );
			gtk_file_filter_add_pattern( filter, type.pattern );
			//gtk_file_filter_add_pattern( allTypesFilter, type.pattern );
			// "Note that the chooser takes ownership of the filter, so
			// you have to ref and sink it if you want to keep a reference."
			// So I guess we won't need to garbage collect this.
			gtk_file_chooser_add_filter( GTK_FILE_CHOOSER( file_sel ), filter );
		}
		//gtk_file_chooser_add_filter( GTK_FILE_CHOOSER( file_sel ), allTypesFilter );
	}

	if ( gtk_dialog_run( GTK_DIALOG( file_sel ) ) == GTK_RESPONSE_ACCEPT ) {
		gchar * filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( file_sel ) );
		if ( filename != NULL ) {
			strncpy( szFile, filename, sizeof( szFile ) );
			szFile[sizeof( szFile ) - 1] = 0;
			g_free( filename );
		} else {
			szFile[0] = '\0';
		}
	}
	else {
		szFile[0] = '\0';
	}

	if ( pattern != NULL ) {
		GtkFileFilter *filter = gtk_file_chooser_get_filter( GTK_FILE_CHOOSER( file_sel ) );
		if ( filter == NULL ) {
			type = filetype_t();
		}
		else {
			type = typelist.GetTypeForGTKMask( gtk_file_filter_get_name( filter ) );
		}
	}
	gtk_widget_destroy( file_sel );

#ifdef _WIN32
}
#endif

	// don't return an empty filename
	if ( szFile[0] == '\0' ) {
		return NULL;
	}

	// convert back to unix format
	for ( w = szFile; *w != '\0'; w++ )
		if ( *w == '\\' ) {
			*w = '/';
		}

	/* \todo SPoG - file_dialog should return filetype information separately.. not force file extension.. */
	if ( !open && pattern != NULL ) {
		v = strrchr( szFile, '/' );
		w = strrchr( szFile, '.' );
		if ( ( v && w && w < v ) || // Last '.' is before the file.
			 w == NULL ) { // Extension missing.
			if ( type.pattern[0] ) {
				w = szFile + strlen( szFile );
				strcpy( w, type.pattern + 1 ); // Add extension of selected filter type.
			}
			else {
				// type will be empty if for example there were no filters for pattern,
				// or if some other UI inconsistencies happen.
				if ( gtk_MessageBox( parent, _( "No file extension specified in file to be saved.\nAttempt to save anyways?" ),
									 _( "GtkRadiant" ), MB_YESNO ) == IDNO ) {
					return NULL;
				}
			}
		}
		else { // An extension was explicitly in the filename.
			bool knownExtension = false;
			for ( int i = typelist.GetNumTypes() - 1; i >= 0; i-- ) {
				type = typelist.GetTypeForIndex( i );
				if ( type.pattern[0] && strcmp( w, type.pattern + 1 ) == 0 ) {
					knownExtension = true;
					break;
				}
			}
			if ( !knownExtension ) {
				if ( gtk_MessageBox( parent, _( "Unknown file extension for this save operation.\nAttempt to save anyways?" ),
									 _( "GtkRadiant" ), MB_YESNO ) == IDNO ) {
					return NULL;
				}
			}
		}
	}

	// prompt to overwrite existing files
	if ( !open ) {
		if ( access( szFile, R_OK ) == 0 ) {
			if ( gtk_MessageBox( parent, _( "File already exists.\nOverwrite?" ), _( "GtkRadiant" ), MB_YESNO ) == IDNO ) {
				return NULL;
			}
		}
	}

	return szFile;
}

char* WINAPI dir_dialog( void *parent, const char* title, const char* path ){
	GtkWidget* file_sel;
	char* filename = (char*)NULL;
	gint response_id;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	file_sel = gtk_file_chooser_dialog_new( title, GTK_WINDOW( parent ), action, NULL, (char*)NULL );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), _( "OK" ), GTK_RESPONSE_ACCEPT );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	if ( path != NULL ) {
		gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( file_sel ), path );
	}

	response_id = gtk_dialog_run( GTK_DIALOG( file_sel ) );

	if ( response_id == GTK_RESPONSE_ACCEPT ) {
		filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( file_sel ) );
	}
	else {
		filename = NULL;
	}

	gtk_widget_destroy( file_sel );

	return filename;
}

bool WINAPI color_dialog( void *parent, float *color, const char* title ){
	GtkWidget* dlg;
	double clr[3];
	int loop = 1, ret = IDCANCEL;

	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];

	dlg = gtk_color_selection_dialog_new( title );
	gtk_color_selection_set_color( GTK_COLOR_SELECTION( GTK_COLOR_SELECTION_DIALOG( dlg )->colorsel ), clr );
	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_signal_connect( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dlg )->ok_button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	g_signal_connect( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dlg )->cancel_button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	if ( parent != NULL ) {
		gtk_window_set_transient_for( GTK_WINDOW( dlg ), GTK_WINDOW( parent ) );
	}

	gtk_widget_show( dlg );
	gtk_grab_add( dlg );

	while ( loop )
		gtk_main_iteration();

	GdkColor gdkcolor;
	gtk_color_selection_get_current_color( GTK_COLOR_SELECTION( GTK_COLOR_SELECTION_DIALOG( dlg )->colorsel ), &gdkcolor );
	clr[0] = gdkcolor.red / 65535.0;
	clr[1] = gdkcolor.green / 65535.0;
	clr[2] = gdkcolor.blue / 65535.0;

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	if ( ret == IDOK ) {
		color[0] = (float)clr[0];
		color[1] = (float)clr[1];
		color[2] = (float)clr[2];

		return true;
	}

	return false;
}

void OpenURL( GtkWidget *parent, const char *url ){
#ifndef _WIN32
    char command[2 * PATH_MAX];
#endif
    GError *gerror = NULL;
    gboolean result;

    Sys_Printf( "OpenURL: %s\n", url );

#if GTK_CHECK_VERSION( 2, 14, 0 )
#   if GTK_CHECK_VERSION( 3, 22, 0 )
    result = gtk_show_uri_on_window( GTK_WINDOW( parent ), url, GDK_CURRENT_TIME, &gerror );
#   else
    result = gtk_show_uri( gtk_window_get_screen( GTK_WINDOW( parent ) ), url, GDK_CURRENT_TIME, &gerror );
#   endif
    if ( result ) {
        return;
    }
    Sys_Printf( "Could not open URL: %s\n", gerror->message );
    g_error_free( gerror );
#endif

#ifdef _WIN32
    ShellExecute( (HWND)GDK_WINDOW_HWND( gtk_widget_get_window( parent ) ), "open", url, NULL, NULL, SW_SHOW );
#else
#   ifdef __APPLE__
        snprintf(command, sizeof(command), "open '%s' &", url);
#   else
        snprintf(command, sizeof(command), "xdg-open '%s' &", url);
#   endif
    if (system(command) != 0) {
         gtk_MessageBox( parent, _( "Failed to launch web browser!" ) );
    }
#endif
}

void CheckMenuSplitting( GtkWidget *&menu ){
	GtkWidget *item,*menu2;

	GtkRequisition requisition;
	gint screen_height;

	gtk_widget_size_request( GTK_WIDGET( menu ), &requisition );
	screen_height = gdk_screen_height();

	if ( ( screen_height - requisition.height ) < 20 ) {
		menu2 = gtk_menu_new();

		// move the last 2 items to a submenu (3 because of win32)
		for ( int i = 0; i < 3; i++ )
		{
			GList *children = gtk_container_get_children( GTK_CONTAINER( menu ) ) ;
			if( !children ) {
				continue;
			}
			item = GTK_WIDGET( g_list_last( children )->data );
			g_object_ref( item );
			gtk_container_remove( GTK_CONTAINER( menu ), item );
			gtk_menu_shell_append( GTK_MENU_SHELL( menu2 ), item );
			g_object_unref( item );

			g_list_free( children );
		}

		item = gtk_menu_item_new_with_label( "--------" );
		gtk_widget_show( item );
		gtk_container_add( GTK_CONTAINER( menu ), item );
		gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), menu2 );
		menu = menu2;
	}
}
