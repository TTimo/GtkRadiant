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

#include "dialog.h"

#include <gtk/gtkmain.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktable.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>

#include "button.h"
#include "window.h"

GtkVBox* create_dialog_vbox(int spacing, int border)
{
  GtkVBox* vbox = GTK_VBOX(gtk_vbox_new(FALSE, spacing));
  gtk_widget_show(GTK_WIDGET(vbox));
  gtk_container_set_border_width(GTK_CONTAINER(vbox), border);
  return vbox;
}

GtkHBox* create_dialog_hbox(int spacing, int border)
{
  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new(FALSE, spacing));
  gtk_widget_show(GTK_WIDGET(hbox));
  gtk_container_set_border_width(GTK_CONTAINER(hbox), border);
  return hbox;
}

GtkFrame* create_dialog_frame(const char* label, GtkShadowType shadow)
{
  GtkFrame* frame = GTK_FRAME(gtk_frame_new(label));
  gtk_widget_show(GTK_WIDGET(frame));
  gtk_frame_set_shadow_type(frame, shadow);
  return frame;
}

GtkTable* create_dialog_table(unsigned int rows, unsigned int columns, unsigned int row_spacing, unsigned int col_spacing, int border)
{
  GtkTable* table = GTK_TABLE(gtk_table_new(rows, columns, FALSE));
  gtk_widget_show(GTK_WIDGET(table));
  gtk_table_set_row_spacings(table, row_spacing);
  gtk_table_set_col_spacings(table, col_spacing);
  gtk_container_set_border_width(GTK_CONTAINER(table), border);
  return table;
}

GtkButton* create_dialog_button(const char* label, GCallback func, gpointer data)
{
  GtkButton* button = GTK_BUTTON(gtk_button_new_with_label(label));
  gtk_widget_set_size_request(GTK_WIDGET(button), 64, -1);
  gtk_widget_show(GTK_WIDGET(button));
  g_signal_connect(G_OBJECT(button), "clicked", func, data);
  return button;
}

GtkWindow* create_dialog_window(GtkWindow* parent, const char* title, GCallback func, gpointer data, int default_w, int default_h)
{
  GtkWindow* window = create_floating_window(title, parent);
  gtk_window_set_default_size(window, default_w, default_h);
  gtk_window_set_position(window, GTK_WIN_POS_CENTER_ON_PARENT);
  g_signal_connect(G_OBJECT(window), "delete_event", func, data);

  return window;
}

gboolean modal_dialog_button_clicked(GtkWidget *widget, ModalDialogButton* button)
{
  button->m_dialog.loop = false;
  button->m_dialog.ret = button->m_value;
  return TRUE;
}

gboolean modal_dialog_delete(GtkWidget *widget, GdkEvent* event, ModalDialog* dialog)
{
  dialog->loop = 0;
  dialog->ret = eIDCANCEL;
  return TRUE;
}

EMessageBoxReturn modal_dialog_show(GtkWindow* window, ModalDialog& dialog)
{
  gtk_grab_add(GTK_WIDGET(window));
  gtk_widget_show(GTK_WIDGET(window));

  dialog.loop = true;
  while(dialog.loop)
  {
    gtk_main_iteration();
  }

  gtk_widget_hide(GTK_WIDGET(window));
  gtk_grab_remove(GTK_WIDGET(window));

  return dialog.ret;
}

GtkButton* create_modal_dialog_button(const char* label, ModalDialogButton& button)
{
  return create_dialog_button(label, G_CALLBACK(modal_dialog_button_clicked), &button);
}

GtkWindow* create_modal_dialog_window(GtkWindow* parent, const char* title, ModalDialog& dialog, int default_w, int default_h)
{
  return create_dialog_window(parent, title, G_CALLBACK(modal_dialog_delete), &dialog, default_w, default_h);
}

GtkWindow* create_fixedsize_modal_dialog_window(GtkWindow* parent, const char* title, ModalDialog& dialog, int width, int height)
{
  GtkWindow* window = create_modal_dialog_window(parent, title, dialog, width, height);

  gtk_window_set_resizable(window, FALSE);
  gtk_window_set_modal(window, TRUE);
  gtk_window_set_position(window, GTK_WIN_POS_CENTER);

  window_remove_minmax(window);

  //gtk_widget_set_size_request(GTK_WIDGET(window), width, height);
  //gtk_window_set_default_size(window, width, height);
  //gtk_window_resize(window, width, height);
  //GdkGeometry geometry = { width, height, -1, -1, width, height, -1, -1, -1, -1, GDK_GRAVITY_STATIC, };
  //gtk_window_set_geometry_hints(window, GTK_WIDGET(window), &geometry, (GdkWindowHints)(GDK_HINT_POS|GDK_HINT_MIN_SIZE|GDK_HINT_BASE_SIZE));

  return window;
}

gboolean dialog_button_ok(GtkWidget *widget, ModalDialog* data)
{
  data->loop = false;
  data->ret = eIDOK;
  return TRUE;
}

gboolean dialog_button_cancel(GtkWidget *widget, ModalDialog* data)
{
  data->loop = false;
  data->ret = eIDCANCEL;
  return TRUE;
}

gboolean dialog_button_yes(GtkWidget *widget, ModalDialog* data)
{
  data->loop = false;
  data->ret = eIDYES;
  return TRUE;
}

gboolean dialog_button_no(GtkWidget *widget, ModalDialog* data)
{
  data->loop = false;
  data->ret = eIDNO;
  return TRUE;
}

gboolean dialog_delete_callback(GtkWidget *widget, GdkEventAny* event, ModalDialog* data)
{
  gtk_widget_hide(widget);
  data->loop = false;
  return TRUE;
}

GtkWindow* create_simple_modal_dialog_window(const char* title, ModalDialog& dialog, GtkWidget* contents)
{
  GtkWindow* window = create_fixedsize_modal_dialog_window(0, title, dialog);

  GtkVBox* vbox1 = create_dialog_vbox(8, 4);
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox1));

  gtk_container_add(GTK_CONTAINER(vbox1), contents);

  GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new(0.5, 0.0, 0.0, 0.0));
  gtk_widget_show(GTK_WIDGET(alignment));
  gtk_box_pack_start(GTK_BOX(vbox1), GTK_WIDGET(alignment), FALSE, FALSE, 0);

  GtkButton* button = create_dialog_button("OK", G_CALLBACK(dialog_button_ok), &dialog);
  gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(button));

  return window;
}

RadioHBox RadioHBox_new(StringArrayRange names)
{
  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new(TRUE, 4));
  gtk_widget_show(GTK_WIDGET(hbox));

  GSList* group = 0;
  GtkRadioButton* radio = 0;
  for(StringArrayRange::Iterator i = names.begin; i != names.end; ++i)
  {
    radio = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(group, *i));
    gtk_widget_show(GTK_WIDGET(radio));
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(radio), FALSE, FALSE, 0);

    group = gtk_radio_button_get_group(radio);
  }

  return RadioHBox(hbox, radio);
}


PathEntry PathEntry_new()
{
  GtkFrame* frame = GTK_FRAME(gtk_frame_new(NULL));
  gtk_widget_show(GTK_WIDGET(frame));
  gtk_frame_set_shadow_type(frame, GTK_SHADOW_IN);

  // path entry
  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new(FALSE, 0));
  gtk_widget_show(GTK_WIDGET(hbox));

  GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_has_frame(entry, FALSE);
  gtk_widget_show(GTK_WIDGET(entry));
  gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry), TRUE, TRUE, 0);

  // browse button
  GtkButton* button = GTK_BUTTON(gtk_button_new());
  button_set_icon(button, "ellipsis.bmp");
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(hbox));

  return PathEntry(frame, entry, button);
}

void PathEntry_setPath(PathEntry& self, const char* path)
{
  gtk_entry_set_text(self.m_entry, path);
}
typedef ReferenceCaller1<PathEntry, const char*, PathEntry_setPath> PathEntrySetPathCaller;

void BrowsedPathEntry_clicked(GtkWidget* widget, BrowsedPathEntry* self)
{
  self->m_browse(PathEntrySetPathCaller(self->m_entry));
}

BrowsedPathEntry::BrowsedPathEntry(const BrowseCallback& browse) :
  m_entry(PathEntry_new()),
  m_browse(browse)
{
  g_signal_connect(G_OBJECT(m_entry.m_button), "clicked", G_CALLBACK(BrowsedPathEntry_clicked), this);
}


GtkLabel* DialogLabel_new(const char* name)
{
  GtkLabel* label = GTK_LABEL(gtk_label_new(name));
  gtk_widget_show(GTK_WIDGET(label));
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
  gtk_label_set_justify(label, GTK_JUSTIFY_LEFT);

  return label;
}

GtkTable* DialogRow_new(const char* name, GtkWidget* widget)
{
  GtkTable* table = GTK_TABLE(gtk_table_new(1, 3, TRUE));
  gtk_widget_show(GTK_WIDGET(table));

  gtk_table_set_col_spacings(table, 4);
  gtk_table_set_row_spacings(table, 0);

  gtk_table_attach(table, GTK_WIDGET(DialogLabel_new(name)), 0, 1, 0, 1,
      (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach(table, widget, 1, 3, 0, 1,
      (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

  return table;
}

void DialogVBox_packRow(GtkVBox* vbox, GtkWidget* row)
{
  gtk_box_pack_start(GTK_BOX(vbox), row, FALSE, FALSE, 0);
}

