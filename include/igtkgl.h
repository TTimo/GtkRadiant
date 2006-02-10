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

#if !defined(INCLUDED_IGTKGL_H)
#define INCLUDED_IGTKGL_H

#include "generic/constant.h"

typedef struct _GtkWidget GtkWidget;
typedef int    gint;
typedef gint   gboolean;

struct _QERGtkGLTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "gtkgl");

  GtkWidget* (*glwidget_new)(gboolean zbufffer);
  void (*glwidget_swap_buffers)(GtkWidget* widget);
  gboolean(*glwidget_make_current)(GtkWidget* widget);
  void (*glwidget_destroy_context)(GtkWidget* widget);
  void (*glwidget_create_context)(GtkWidget* widget);
};

#endif
