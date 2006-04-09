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

#if !defined(INCLUDED_GTKUTIL_BUTTON_H)
#define INCLUDED_GTKUTIL_BUTTON_H

#include "generic/callbackfwd.h"

typedef struct _GtkButton GtkButton;
typedef struct _GtkToggleButton GtkToggleButton;
typedef struct _GtkRadioButton GtkRadioButton;
typedef int gint;
typedef gint gboolean;
typedef unsigned int guint;

void button_connect_callback(GtkButton* button, const Callback& callback);
guint toggle_button_connect_callback(GtkToggleButton* button, const Callback& callback);

void button_set_icon(GtkButton* button, const char* icon);
void toggle_button_set_active_no_signal(GtkToggleButton* item, gboolean active);

void radio_button_set_active(GtkRadioButton* radio, int index);
void radio_button_set_active_no_signal(GtkRadioButton* radio, int index);
int radio_button_get_active(GtkRadioButton* radio);

#endif
