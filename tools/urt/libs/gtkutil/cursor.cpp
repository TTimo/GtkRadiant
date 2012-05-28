
#include "cursor.h"


#include <string.h>
#include <gdk/gdkcursor.h>
#include <gdk/gdkpixmap.h>


GdkCursor* create_blank_cursor(){
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	char buffer [( 32 * 32 ) / 8];
	memset( buffer, 0, ( 32 * 32 ) / 8 );
	GdkColor white = {0, 0xffff, 0xffff, 0xffff};
	GdkColor black = {0, 0x0000, 0x0000, 0x0000};
	pixmap = gdk_bitmap_create_from_data( 0, buffer, 32, 32 );
	mask   = gdk_bitmap_create_from_data( 0, buffer, 32, 32 );
	GdkCursor *cursor = gdk_cursor_new_from_pixmap( pixmap, mask, &white, &black, 1, 1 );
	gdk_drawable_unref( pixmap );
	gdk_drawable_unref( mask );

	return cursor;
}

void blank_cursor( GtkWidget* widget ){
	GdkCursor* cursor = create_blank_cursor();
	gdk_window_set_cursor( widget->window, cursor );
	gdk_cursor_unref( cursor );
}

void default_cursor( GtkWidget* widget ){
	gdk_window_set_cursor( widget->window, 0 );
}


#if defined( WIN32 )

#include <gdk/gdkwin32.h>

void Sys_GetCursorPos( int *x, int *y ){
	POINT pos;
	GetCursorPos( &pos );
	*x = pos.x;
	*y = pos.y;
}

void Sys_SetCursorPos( int x, int y ){
	SetCursorPos( x, y );
}

#else

#include <gdk/gdkx.h>

void Sys_GetCursorPos( int *x, int *y ){
	gdk_display_get_pointer( gdk_display_get_default(), 0, x, y, 0 );
}

void Sys_SetCursorPos( int x, int y ){
	XWarpPointer( GDK_DISPLAY(), None, GDK_ROOT_WINDOW(), 0, 0, 0, 0, x, y );
}

#endif
