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

#include "frame.h"

#include <gtk/gtkframe.h>

GtkFrame* create_framed_widget(GtkWidget* widget)
{
  GtkFrame* frame = GTK_FRAME(gtk_frame_new(0));
  gtk_widget_show(GTK_WIDGET(frame));
  gtk_frame_set_shadow_type(frame, GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER(frame), widget);
  gtk_widget_show(GTK_WIDGET(widget));
  return frame;
}

