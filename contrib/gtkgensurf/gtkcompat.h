
#if ! GTK_CHECK_VERSION(3, 0, 0)
#	define gtk_widget_get_allocated_height(widget)	(((GtkWidget *) (widget))->allocation.height)
#	define gtk_widget_get_allocated_width(widget)	(((GtkWidget *) (widget))->allocation.width)
#endif
