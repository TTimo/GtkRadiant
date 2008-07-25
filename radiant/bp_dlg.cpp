/*
Copyright (C) 1999-2007 id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

//-----------------------------------------------------------------------------
//
// DESCRIPTION
//
// custom Gtk dialog for brush primitives load/save

#include "stdafx.h"
#include <glib/gi18n.h>

void BP_dialog_button_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *parent;
  int *loop, *ret;

  parent = gtk_widget_get_toplevel (widget);
  loop = (int*)g_object_get_data (G_OBJECT (parent), "loop");
  ret = (int*)g_object_get_data (G_OBJECT (parent), "ret");

  *loop = 0;
  *ret = GPOINTER_TO_INT (data);
}

gint BP_dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  int *loop;

  gtk_widget_hide (widget);
  loop = (int*)g_object_get_data (G_OBJECT (widget), "loop");
  *loop = 0;

  return TRUE;
}

// ret: 0 = abort, 1 = load and convert, 2 = changed project settings, load and don't convert
// the user might decide to switch the BP mode in project settings
// status: 0 = loading regular, got conflict 1 = loading BP, got conflict
// int WINAPI gtk_MessageBox (GtkWidget *parent, const char* lpText, const char* lpCaption, guint32 uType)
int BP_MessageBox (int status)
{
  GtkWidget *window, *w, *vbox, *hbox;
  GtkAccelGroup *accel;
  int ret, loop = 1;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (BP_dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

  gtk_window_set_title (GTK_WINDOW (window), _("Current map format is incompatible"));

  gtk_container_border_width (GTK_CONTAINER (window), 10);
  g_object_set_data (G_OBJECT (window), "loop", &loop);
  g_object_set_data (G_OBJECT (window), "ret", &ret);
  gtk_widget_realize (window);

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (g_pParentWnd->m_pWidget));

  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  if (status == 0)
  {
    w = gtk_label_new (_("This map was saved using brush primitives format\n"
      "and your project settings use the standard format.\n"
      "Do you want to convert the map, change default format or abort?\n"
      "NOTE: due to limitations of the standard format, "
      "some texture alignments may be lost after conversion."));
  }
  else
  {
    w = gtk_label_new (_("This map was saved using standard format\n"
      "and your project settings use the new \"brush primitives\" format.\n"
      "Do you want to convert the map, change default format or abort?\n"
      "NOTE: Next versions of Radiant will allow mixing the two formats"
      "in the same maps for a smooth transition."));
  }
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
  gtk_widget_show (w);

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  gtk_widget_show (hbox);

  w = gtk_button_new_with_label (_("Convert"));
  gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (w), "clicked",
    GTK_SIGNAL_FUNC (BP_dialog_button_callback), GINT_TO_POINTER (1));
  GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (w);
  gtk_widget_show (w);

  w = gtk_button_new_with_label (_("Change default"));
  gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (w), "clicked",
    GTK_SIGNAL_FUNC (BP_dialog_button_callback), GINT_TO_POINTER (2));
  gtk_widget_show (w);

  w = gtk_button_new_with_label (_("Abort load"));
  gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (w), "clicked",
    GTK_SIGNAL_FUNC (BP_dialog_button_callback), GINT_TO_POINTER (0));
  gtk_widget_show (w);
  ret = 0; // abort

  gtk_widget_show (window);
  gtk_grab_add (window);

  while (loop)
    gtk_main_iteration ();

  if (ret == 2)
  {
    // change project settings
    if (status == 0)
      g_qeglobals.m_bBrushPrimitMode = TRUE;
    else
      g_qeglobals.m_bBrushPrimitMode = FALSE;
    SetKeyValue(g_qeglobals.d_project_entity, "brush_primit", (g_qeglobals.m_bBrushPrimitMode ? "1" : "0" ));
  }

  gtk_grab_remove (window);
  gtk_widget_destroy (window);

  return ret;
}
