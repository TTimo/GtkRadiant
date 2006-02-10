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

#include "messagebox.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>

#include "dialog.h"
#include "widget.h"

GtkWidget* create_padding(int width, int height)
{
  GtkWidget* widget = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
  gtk_widget_show(widget);
  gtk_widget_set_size_request(widget, width, height);
  return widget;
}

const char* messagebox_stock_icon(EMessageBoxIcon type)
{
  switch(type)
  {
  default:
  case eMB_ICONDEFAULT:
    return GTK_STOCK_DIALOG_INFO;
  case eMB_ICONERROR:
    return GTK_STOCK_DIALOG_ERROR;
  case eMB_ICONWARNING:
    return GTK_STOCK_DIALOG_WARNING;
  case eMB_ICONQUESTION:
    return GTK_STOCK_DIALOG_QUESTION;
  case eMB_ICONASTERISK:
    return GTK_STOCK_DIALOG_INFO;
  }
}

EMessageBoxReturn gtk_MessageBox (GtkWidget *parent, const char* text, const char* title, EMessageBoxType type, EMessageBoxIcon icon)
{
  ModalDialog dialog;
  ModalDialogButton ok_button(dialog, eIDOK);
  ModalDialogButton cancel_button(dialog, eIDCANCEL);
  ModalDialogButton yes_button(dialog, eIDYES);
  ModalDialogButton no_button(dialog, eIDNO);

  GtkWindow* parentWindow = parent != 0 ? GTK_WINDOW(parent) : 0;

  GtkWindow* window = create_fixedsize_modal_dialog_window(parentWindow, title, dialog, 400, 100);

  if(parentWindow != 0)
  {
    //g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(floating_window_delete_present), parent);
    gtk_window_deiconify(parentWindow);
  }  

  GtkAccelGroup* accel = gtk_accel_group_new();
  gtk_window_add_accel_group(window, accel);

  GtkVBox* vbox = create_dialog_vbox(8, 8);
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));


  GtkHBox* hboxDummy = create_dialog_hbox(0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hboxDummy), FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(hboxDummy), create_padding(0, 50), FALSE, FALSE, 0); // HACK to force minimum height

  GtkHBox* iconBox = create_dialog_hbox(16, 0);
  gtk_box_pack_start(GTK_BOX(hboxDummy), GTK_WIDGET(iconBox), FALSE, FALSE, 0);

  GtkImage* image = GTK_IMAGE(gtk_image_new_from_stock(messagebox_stock_icon(icon), GTK_ICON_SIZE_DIALOG));
  gtk_widget_show(GTK_WIDGET(image));
  gtk_box_pack_start(GTK_BOX(iconBox), GTK_WIDGET(image), FALSE, FALSE, 0);

  GtkLabel* label = GTK_LABEL(gtk_label_new(text));
  gtk_widget_show(GTK_WIDGET(label));
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
  gtk_label_set_justify(label, GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap(label, TRUE);
  gtk_box_pack_start(GTK_BOX(iconBox), GTK_WIDGET(label), TRUE, TRUE, 0);


  GtkVBox* vboxDummy = create_dialog_vbox(0, 0);
  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(vboxDummy), FALSE, FALSE, 0);

  GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new(0.5, 0.0, 0.0, 0.0));
  gtk_widget_show(GTK_WIDGET(alignment));
  gtk_box_pack_start(GTK_BOX(vboxDummy), GTK_WIDGET(alignment), FALSE, FALSE, 0);

  GtkHBox* hbox = create_dialog_hbox(8, 0);
  gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(hbox));

  gtk_box_pack_start(GTK_BOX(vboxDummy), create_padding(400, 0), FALSE, FALSE, 0); // HACK to force minimum width


  if (type == eMB_OK)
  {
    GtkButton* button = create_modal_dialog_button("OK", ok_button);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
    gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
    gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
    widget_make_default(GTK_WIDGET(button));
    gtk_widget_show(GTK_WIDGET(button));

    dialog.ret = eIDOK;
  }
  else if (type ==  eMB_OKCANCEL)
  {
    {
      GtkButton* button = create_modal_dialog_button("OK", ok_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
      widget_make_default(GTK_WIDGET(button));
      gtk_widget_show(GTK_WIDGET(button));
    }

    {
      GtkButton* button = create_modal_dialog_button("OK", cancel_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
      gtk_widget_show(GTK_WIDGET(button));
    }

    dialog.ret = eIDCANCEL;
  }
  else if (type == eMB_YESNOCANCEL)
  {
    {
      GtkButton* button = create_modal_dialog_button("Yes", yes_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      widget_make_default(GTK_WIDGET(button));
      gtk_widget_show(GTK_WIDGET(button));
    }

    {
      GtkButton* button = create_modal_dialog_button("No", no_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(button));
    }
    {
      GtkButton* button = create_modal_dialog_button("Cancel", cancel_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(button));
    }

    dialog.ret = eIDCANCEL;
  }
  else if (type == eMB_NOYES)
  {
    {
      GtkButton* button = create_modal_dialog_button("No", no_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      widget_make_default(GTK_WIDGET(button));
      gtk_widget_show(GTK_WIDGET(button));
    }
    {
      GtkButton* button = create_modal_dialog_button("Yes", yes_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(button));
    }

    dialog.ret = eIDNO;
  }
  else /* if (type == eMB_YESNO) */
  {
    {
      GtkButton* button = create_modal_dialog_button("Yes", yes_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      widget_make_default(GTK_WIDGET(button));
      gtk_widget_show(GTK_WIDGET(button));
    }
 
    {
      GtkButton* button = create_modal_dialog_button("No", no_button);
      gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), TRUE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(button));
    }
    dialog.ret = eIDNO;
  }

  modal_dialog_show(window, dialog);

  gtk_widget_destroy(GTK_WIDGET(window));

  return dialog.ret;
}

