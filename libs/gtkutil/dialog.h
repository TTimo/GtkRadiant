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

#if !defined(INCLUDED_GTKUTIL_DIALOG_H)
#define INCLUDED_GTKUTIL_DIALOG_H

#include "generic/callback.h"
#include "generic/arrayrange.h"
#include "qerplugin.h"
#include <gtk/gtkenums.h>

typedef int gint;
typedef gint gboolean;
typedef struct _GdkEventAny GdkEventAny;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkHBox GtkHBox;
typedef struct _GtkVBox GtkVBox;
typedef struct _GtkRadioButton GtkRadioButton;
typedef struct _GtkFrame GtkFrame;
typedef struct _GtkEntry GtkEntry;
typedef struct _GtkButton GtkButton;
typedef struct _GtkLabel GtkLabel;
typedef struct _GtkTable GtkTable;


struct ModalDialog
{
  ModalDialog()
    : loop(true), ret(eIDCANCEL)
  {
  }
  bool loop;
  EMessageBoxReturn ret;
};

struct ModalDialogButton
{
  ModalDialogButton(ModalDialog& dialog, EMessageBoxReturn value)
    : m_dialog(dialog), m_value(value)
  {
  }
  ModalDialog& m_dialog;
  EMessageBoxReturn m_value;
};

typedef void (*GCallback)(void);
typedef void* gpointer;
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkTable GtkTable;
typedef struct _GtkButton GtkButton;
typedef struct _GtkVBox GtkVBox;
typedef struct _GtkHBox GtkHBox;
typedef struct _GtkFrame GtkFrame;

GtkWindow* create_fixedsize_modal_window(GtkWindow* parent, const char* title, int width, int height);

GtkWindow* create_dialog_window(GtkWindow* parent, const char* title, GCallback func, gpointer data, int default_w = -1, int default_h = -1);
GtkTable* create_dialog_table(unsigned int rows, unsigned int columns, unsigned int row_spacing, unsigned int col_spacing, int border = 0);
GtkButton* create_dialog_button(const char* label, GCallback func, gpointer data);
GtkVBox* create_dialog_vbox(int spacing, int border = 0);
GtkHBox* create_dialog_hbox(int spacing, int border = 0);
GtkFrame* create_dialog_frame(const char* label, GtkShadowType shadow = GTK_SHADOW_ETCHED_IN);

GtkButton* create_modal_dialog_button(const char* label, ModalDialogButton& button);
GtkWindow* create_modal_dialog_window(GtkWindow* parent, const char* title, ModalDialog& dialog, int default_w = -1, int default_h = -1);
GtkWindow* create_fixedsize_modal_dialog_window(GtkWindow* parent, const char* title, ModalDialog& dialog, int width = -1, int height = -1);
EMessageBoxReturn modal_dialog_show(GtkWindow* window, ModalDialog& dialog);


gboolean dialog_button_ok(GtkWidget *widget, ModalDialog* data);
gboolean dialog_button_cancel(GtkWidget *widget, ModalDialog* data);
gboolean dialog_button_yes(GtkWidget *widget, ModalDialog* data);
gboolean dialog_button_no(GtkWidget *widget, ModalDialog* data);
gboolean dialog_delete_callback(GtkWidget *widget, GdkEventAny* event, ModalDialog* data);

GtkWindow* create_simple_modal_dialog_window(const char* title, ModalDialog& dialog, GtkWidget* contents);

class RadioHBox
{
public:
  GtkHBox* m_hbox;
  GtkRadioButton* m_radio;
  RadioHBox(GtkHBox* hbox, GtkRadioButton* radio) :
    m_hbox(hbox),
    m_radio(radio)
  {
  }
};

RadioHBox RadioHBox_new(StringArrayRange names);


class PathEntry
{
public:
  GtkFrame* m_frame;
  GtkEntry* m_entry;
  GtkButton* m_button;
  PathEntry(GtkFrame* frame, GtkEntry* entry, GtkButton* button) :
    m_frame(frame),
    m_entry(entry),
    m_button(button)
  {
  }
};

PathEntry PathEntry_new();

class BrowsedPathEntry
{
public:
  typedef Callback1<const char*> SetPathCallback;
  typedef Callback1<const SetPathCallback&> BrowseCallback;

  PathEntry m_entry;
  BrowseCallback m_browse;

  BrowsedPathEntry(const BrowseCallback& browse);
};

GtkLabel* DialogLabel_new(const char* name);
GtkTable* DialogRow_new(const char* name, GtkWidget* widget);
typedef struct _GtkVBox GtkVBox;
void DialogVBox_packRow(GtkVBox* vbox, GtkWidget* row);


#endif
