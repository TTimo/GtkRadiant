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

#ifndef _GTK_MISC_H_
#define _GTK_MISC_H_

#ifdef _WIN32

void win32_get_window_pos( GtkWidget *widget, gint *x, gint *y );

inline void get_window_pos( GtkWidget *wnd, int* x, int* y ){
	win32_get_window_pos( wnd, x, y );
}

#else

inline void get_window_pos( GtkWidget *wnd, int* x, int* y ){
	gdk_window_get_root_origin( gtk_widget_get_window( wnd ), x, y );
}

#endif


struct window_position_t
{
	int x, y, w, h;
};

void save_window_pos( GtkWidget *wnd, window_position_t& pos );
void load_window_pos( GtkWidget *wnd, window_position_t& pos );
gint widget_delete_hide( GtkWidget *widget );

bool WINAPI load_plugin_bitmap( const char* filename, void **gdkpixmap, void **mask );
void load_pixmap( const char* filename, GtkWidget **widget, GtkWidget **gdkpixmap );
GtkWidget* new_pixmap( GtkWidget* widget, const char* filename );
GtkWidget* new_image_icon(const char* filename);
GtkWidget* new_plugin_image_icon( const char* filename );

GtkWidget* menu_separator( GtkWidget *menu );
GtkWidget* menu_tearoff( GtkWidget *menu );
GtkWidget* create_sub_menu_with_mnemonic( GtkWidget *bar, const gchar *mnemonic );
GtkWidget* create_menu_item_with_mnemonic( GtkWidget *menu, const gchar *mnemonic, GCallback func, int id );
GtkWidget* create_check_menu_item_with_mnemonic( GtkWidget *menu, const gchar *mnemonic, GCallback func, int id, gboolean active );
GtkWidget* create_radio_menu_item_with_mnemonic( GtkWidget *menu, GtkWidget *last, const gchar *mnemonic, GCallback func, int id, gboolean state );
GtkWidget* create_menu_in_menu_with_mnemonic( GtkWidget *menu, const gchar *mnemonic );


/*!
   \fn gtk_MessageBox
   do various message boxes, IDOK .. IDNO
   URL adds an optional 'go to URL' button
 */
int WINAPI gtk_MessageBoxNew( void *parent, const char *message, 
						     const char *caption = "Radiant", const guint32 flags = MB_OK, 
							 const char *URL = NULL );
int WINAPI gtk_MessageBox( void *parent, const char* lpText, const char* lpCaption = "Radiant", guint32 uType = MB_OK, const char* URL = NULL );
// NOTE: the returned filename is allocated with g_malloc and MUST be freed with g_free (both for win32 and Gtk dialogs)
// GtkWidget *parent
const gchar* file_dialog( void *parent, gboolean open, const char* title, const char* path = (char*)NULL, const char* pattern = NULL, const char *baseSubDir = NULL );

/*!
   \fn dir_dialog, prompts for a directory
 */
char* WINAPI dir_dialog( void *parent, const char* title = "Choose Directory", const char* path = (char*)NULL );
// GtkWidget *parent
bool WINAPI color_dialog( void *parent, float *color, const char* title = "Choose Color" );

void dialog_button_callback( GtkWidget *widget, gpointer data );
gint dialog_delete_callback( GtkWidget *widget, GdkEvent* event, gpointer data );

void OpenURL( GtkWidget *parent, const char *url );

void CheckMenuSplitting( GtkWidget *&menu );

#endif // _GTK_MISC_H_
