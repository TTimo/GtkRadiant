
#if !defined( INCLUDED_GTKUTIL_CONTAINER_H )
#define INCLUDED_GTKUTIL_CONTAINER_H

#include <gtk/gtkcontainer.h>

inline GtkWidget* container_add_widget( GtkContainer* container, GtkWidget* widget ){
	gtk_container_add( container, widget );
	return widget;
}

inline void container_remove( GtkWidget* item, gpointer data ){
	gtk_container_remove( GTK_CONTAINER( data ), item );
}

inline void container_remove_all( GtkContainer* container ){
	gtk_container_foreach( container, container_remove, container );
}

#endif
