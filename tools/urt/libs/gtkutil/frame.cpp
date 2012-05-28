
#include "frame.h"

#include <gtk/gtkframe.h>

GtkFrame* create_framed_widget( GtkWidget* widget ){
	GtkFrame* frame = GTK_FRAME( gtk_frame_new( 0 ) );
	gtk_widget_show( GTK_WIDGET( frame ) );
	gtk_frame_set_shadow_type( frame, GTK_SHADOW_IN );
	gtk_container_add( GTK_CONTAINER( frame ), widget );
	gtk_widget_show( GTK_WIDGET( widget ) );
	return frame;
}
