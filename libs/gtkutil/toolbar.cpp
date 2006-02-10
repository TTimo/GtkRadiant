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

#include "toolbar.h"

#include <gtk/gtktoolbar.h>
#include <gtk/gtktogglebutton.h>

#include "generic/callback.h"

#include "accelerator.h"
#include "button.h"
#include "closure.h"
#include "pointer.h"


void toolbar_append(GtkToolbar* toolbar, GtkButton* button, const char* description)
{
  gtk_widget_show(GTK_WIDGET(button));
  gtk_button_set_relief(button, GTK_RELIEF_NONE);
  GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(button), GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(button), GTK_CAN_DEFAULT);
  gtk_toolbar_append_element(toolbar, GTK_TOOLBAR_CHILD_WIDGET, GTK_WIDGET(button), "", description, "", 0, 0, 0);
}

GtkButton* toolbar_append_button(GtkToolbar* toolbar, const char* description, const char* icon, const Callback& callback)
{
  GtkButton* button = GTK_BUTTON(gtk_button_new());
  button_set_icon(button, icon);
  button_connect_callback(button, callback);
  toolbar_append(toolbar, button, description);
  return button;
}

GtkToggleButton* toolbar_append_toggle_button(GtkToolbar* toolbar, const char* description, const char* icon, const Callback& callback)
{
  GtkToggleButton* button = GTK_TOGGLE_BUTTON(gtk_toggle_button_new());
  button_set_icon(GTK_BUTTON(button), icon);
  toggle_button_connect_callback(button, callback);
  toolbar_append(toolbar, GTK_BUTTON(button), description);
  return button;
}

GtkButton* toolbar_append_button(GtkToolbar* toolbar, const char* description, const char* icon, const Command& command)
{
  return toolbar_append_button(toolbar, description, icon, command.m_callback);
}

void toggle_button_set_active_callback(GtkToggleButton& button, bool active)
{
  toggle_button_set_active_no_signal(&button, active);
}
typedef ReferenceCaller1<GtkToggleButton, bool, toggle_button_set_active_callback> ToggleButtonSetActiveCaller;

GtkToggleButton* toolbar_append_toggle_button(GtkToolbar* toolbar, const char* description, const char* icon, const Toggle& toggle)
{
  GtkToggleButton* button = toolbar_append_toggle_button(toolbar, description, icon, toggle.m_command.m_callback);
  toggle.m_exportCallback(ToggleButtonSetActiveCaller(*button));
  return button;
}
