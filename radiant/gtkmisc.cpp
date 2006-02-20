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

//
// Small functions to help with GTK
//

#include "gtkmisc.h"

#include <gtk/gtkcolorseldialog.h>
#include <gtk/gtkentry.h>

#include "math/vector.h"
#include "os/path.h"

#include "gtkutil/dialog.h"
#include "gtkutil/filechooser.h"
#include "gtkutil/menu.h"
#include "gtkutil/toolbar.h"
#include "commands.h"


// =============================================================================
// Misc stuff

void command_connect_accelerator(const char* name)
{
  const Command& command = GlobalCommands_find(name);
  GlobalShortcuts_register(name);
  global_accel_group_connect(command.m_accelerator, command.m_callback);
}

void command_disconnect_accelerator(const char* name)
{
  const Command& command = GlobalCommands_find(name);
  global_accel_group_disconnect(command.m_accelerator, command.m_callback);
}

void toggle_add_accelerator(const char* name)
{
  const Toggle& toggle = GlobalToggles_find(name);
  GlobalShortcuts_register(name);
  global_accel_group_connect(toggle.m_command.m_accelerator, toggle.m_command.m_callback);
}

GtkCheckMenuItem* create_check_menu_item_with_mnemonic(GtkMenu* menu, const char* mnemonic, const char* commandName)
{
  GlobalShortcuts_register(commandName);
  const Toggle& toggle = GlobalToggles_find(commandName);
  global_accel_group_connect(toggle.m_command.m_accelerator, toggle.m_command.m_callback);
  return create_check_menu_item_with_mnemonic(menu, mnemonic, toggle);
}

GtkMenuItem* create_menu_item_with_mnemonic(GtkMenu* menu, const char *mnemonic, const char* commandName)
{
  GlobalShortcuts_register(commandName);
  const Command& command = GlobalCommands_find(commandName);
  global_accel_group_connect(command.m_accelerator, command.m_callback);
  return create_menu_item_with_mnemonic(menu, mnemonic, command);
}

GtkButton* toolbar_append_button(GtkToolbar* toolbar, const char* description, const char* icon, const char* commandName)
{
  return toolbar_append_button(toolbar, description, icon, GlobalCommands_find(commandName));
}

GtkToggleButton* toolbar_append_toggle_button(GtkToolbar* toolbar, const char* description, const char* icon, const char* commandName)
{
  return toolbar_append_toggle_button(toolbar, description, icon, GlobalToggles_find(commandName));
}

// =============================================================================
// File dialog

bool color_dialog (GtkWidget *parent, Vector3& color, const char* title)
{
  GtkWidget* dlg;
  double clr[3];
  ModalDialog dialog;

  clr[0] = color[0];
  clr[1] = color[1];
  clr[2] = color[2];

  dlg = gtk_color_selection_dialog_new (title);
  gtk_color_selection_set_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), clr);
  g_signal_connect(G_OBJECT(dlg), "delete_event", G_CALLBACK(dialog_delete_callback), &dialog);
  g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(dlg)->ok_button), "clicked", G_CALLBACK(dialog_button_ok), &dialog);
  g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(dlg)->cancel_button), "clicked", G_CALLBACK(dialog_button_cancel), &dialog);

  if (parent != 0)
    gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (parent));

  bool ok = modal_dialog_show(GTK_WINDOW(dlg), dialog) == eIDOK;
  if(ok)
  {
    GdkColor gdkcolor;
    gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), &gdkcolor);
    clr[0] = gdkcolor.red / 65535.0;
    clr[1] = gdkcolor.green / 65535.0;
    clr[2] = gdkcolor.blue / 65535.0;

    color[0] = (float)clr[0];
    color[1] = (float)clr[1];
    color[2] = (float)clr[2];
  }

  gtk_widget_destroy(dlg);

  return ok;
}

void button_clicked_entry_browse_file(GtkWidget* widget, GtkEntry* entry)
{
  const char *filename = file_dialog(gtk_widget_get_toplevel(widget), TRUE, "Choose File", gtk_entry_get_text(entry));
  
  if(filename != 0)
  {
    gtk_entry_set_text(entry, filename);
  }
}

void button_clicked_entry_browse_directory(GtkWidget* widget, GtkEntry* entry)
{
  const char* text = gtk_entry_get_text(entry);
  char *dir = dir_dialog(gtk_widget_get_toplevel(widget), "Choose Directory", path_is_absolute(text) ? text : "" );
  
  if(dir != 0)
  {
    gchar* converted = g_filename_to_utf8(dir, -1, 0, 0, 0);
    gtk_entry_set_text(entry, converted);
    g_free(dir);
    g_free(converted);
  }
}


