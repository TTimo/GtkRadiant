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

#include "paned.h"

#include <gtk/gtkhpaned.h>
#include <gtk/gtkvpaned.h>

#include "frame.h"


class PanedState
{
public:
  float position;
  int size;
};

gboolean hpaned_allocate(GtkWidget* widget, GtkAllocation* allocation, PanedState* paned)
{
  if(paned->size != allocation->width)
  {
    paned->size = allocation->width;
    gtk_paned_set_position (GTK_PANED (widget), static_cast<int>(paned->size * paned->position));
  }
  return FALSE;
}

gboolean vpaned_allocate(GtkWidget* widget, GtkAllocation* allocation, PanedState* paned)
{
  if(paned->size != allocation->height)
  {
    paned->size = allocation->height;
    gtk_paned_set_position (GTK_PANED (widget), static_cast<int>(paned->size * paned->position));
  }
  return FALSE;
}

gboolean paned_position(GtkWidget* widget, gpointer dummy, PanedState* paned)
{
  if(paned->size != -1)
    paned->position = gtk_paned_get_position (GTK_PANED (widget)) / static_cast<float>(paned->size);
  return FALSE;
}

PanedState g_hpaned = { 0.5f, -1, };
PanedState g_vpaned1 = { 0.5f, -1, };
PanedState g_vpaned2 = { 0.5f, -1, };

GtkHPaned* create_split_views(GtkWidget* topleft, GtkWidget* topright, GtkWidget* botleft, GtkWidget* botright)
{
  GtkHPaned* hsplit = GTK_HPANED(gtk_hpaned_new());
  gtk_widget_show(GTK_WIDGET(hsplit));

  g_signal_connect(G_OBJECT(hsplit), "size_allocate", G_CALLBACK(hpaned_allocate), &g_hpaned);
  g_signal_connect(G_OBJECT(hsplit), "notify::position", G_CALLBACK(paned_position), &g_hpaned);

  {
    GtkVPaned* vsplit = GTK_VPANED(gtk_vpaned_new());
    gtk_paned_add1(GTK_PANED(hsplit), GTK_WIDGET(vsplit));
    gtk_widget_show(GTK_WIDGET(vsplit));

    g_signal_connect(G_OBJECT(vsplit), "size_allocate", G_CALLBACK(vpaned_allocate), &g_vpaned1);
    g_signal_connect(G_OBJECT(vsplit), "notify::position", G_CALLBACK(paned_position), &g_vpaned1);

    gtk_paned_add1(GTK_PANED(vsplit), GTK_WIDGET(create_framed_widget(topleft)));
    gtk_paned_add2(GTK_PANED(vsplit), GTK_WIDGET(create_framed_widget(topright)));
  }
  {
    GtkVPaned* vsplit = GTK_VPANED(gtk_vpaned_new());
    gtk_paned_add2(GTK_PANED(hsplit), GTK_WIDGET(vsplit));
    gtk_widget_show(GTK_WIDGET(vsplit));

    g_signal_connect(G_OBJECT(vsplit), "size_allocate", G_CALLBACK(vpaned_allocate), &g_vpaned2);
    g_signal_connect(G_OBJECT(vsplit), "notify::position", G_CALLBACK(paned_position), &g_vpaned2);

    gtk_paned_add1(GTK_PANED(vsplit), GTK_WIDGET(create_framed_widget(botleft)));
    gtk_paned_add2(GTK_PANED(vsplit), GTK_WIDGET(create_framed_widget(botright)));
  }
  return hsplit;
}

