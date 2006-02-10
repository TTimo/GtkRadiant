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

// OpenGL widget based on GtkGLExt

#include "glwidget.h"

#include "debugging/debugging.h"

#include "igl.h"

#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkglwidget.h>

#include "pointer.h"

void (*GLWidget_sharedContextCreated)() = 0;
void (*GLWidget_sharedContextDestroyed)() = 0;


typedef int* attribs_t;
struct config_t
{
  const char* name;
  attribs_t attribs;
};
typedef const config_t* configs_iterator;

int config_rgba32[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 24,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 16,
  GDK_GL_ATTRIB_LIST_NONE,
};

const config_t configs[] = {
  {
    "colour-buffer = 32bpp, depth-buffer = none",
    config_rgba32,
  },
  {
    "colour-buffer = 16bpp, depth-buffer = none",
    config_rgba,
  }
};

GdkGLConfig* glconfig_new()
{
  GdkGLConfig* glconfig = 0;

  for(configs_iterator i = configs, end = configs + 2; i != end; ++i)
  {
    glconfig = gdk_gl_config_new((*i).attribs);
    if(glconfig != 0)
    {
      globalOutputStream() << "OpenGL window configuration: " << (*i).name << "\n";
      return glconfig;
    }
  }

  globalOutputStream() << "OpenGL window configuration: colour-buffer = auto, depth-buffer = none\n";
  return gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE));
}

int config_rgba32_depth32[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 24,
  GDK_GL_DEPTH_SIZE, 32,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth24[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 24,
  GDK_GL_DEPTH_SIZE, 24,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth16[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 24,
  GDK_GL_DEPTH_SIZE, 16,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 24,
  GDK_GL_DEPTH_SIZE, 1,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba_depth16[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 16,
  GDK_GL_DEPTH_SIZE, 16,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba_depth[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_BUFFER_SIZE, 16,
  GDK_GL_DEPTH_SIZE, 1,
  GDK_GL_ATTRIB_LIST_NONE,
};

const config_t configs_with_depth[] = 
{
  {
    "colour-buffer = 32bpp, depth-buffer = 32bpp",
    config_rgba32_depth32,
  },
  {
    "colour-buffer = 32bpp, depth-buffer = 24bpp",
    config_rgba32_depth24,
  },
  {
    "colour-buffer = 32bpp, depth-buffer = 16bpp",
    config_rgba32_depth16,
  },
  {
    "colour-buffer = 32bpp, depth-buffer = auto",
    config_rgba32_depth,
  },
  {
    "colour-buffer = 16bpp, depth-buffer = 16bpp",
    config_rgba_depth16,
  },
  {
    "colour-buffer = auto, depth-buffer = auto",
    config_rgba_depth,
  },
};

GdkGLConfig* glconfig_new_with_depth()
{
  GdkGLConfig* glconfig = 0;

  for(configs_iterator i = configs_with_depth, end = configs_with_depth + 6; i != end; ++i)
  {
    glconfig = gdk_gl_config_new((*i).attribs);
    if(glconfig != 0)
    {
      globalOutputStream() << "OpenGL window configuration: " << (*i).name << "\n";
      return glconfig;
    }
  }

  globalOutputStream() << "OpenGL window configuration: colour-buffer = auto, depth-buffer = auto (fallback)\n";
  return gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH));
}

unsigned int g_context_count = 0;

namespace
{
  GtkWidget* g_shared = 0;
}

gint glwidget_context_created(GtkWidget* widget, gpointer data)
{
  if(++g_context_count == 1)
  {
    g_shared = widget;
    gtk_widget_ref(g_shared);

    glwidget_make_current(g_shared);
    GlobalOpenGL().contextValid = true;

    GLWidget_sharedContextCreated();
  }
  return FALSE;
}

gint glwidget_context_destroyed(GtkWidget* widget, gpointer data)
{
  if(--g_context_count == 0)
  {
    GlobalOpenGL().contextValid = false;

    GLWidget_sharedContextDestroyed();

    gtk_widget_unref(g_shared);
    g_shared = 0;
  }
  return FALSE;
}

gboolean glwidget_enable_gl(GtkWidget* widget, GtkWidget* widget2, gpointer data)
{
  if(widget2 == 0 && !gtk_widget_is_gl_capable(widget))
  {
    GdkGLConfig* glconfig = (g_object_get_data(G_OBJECT(widget), "zbuffer")) ? glconfig_new_with_depth() : glconfig_new();
    ASSERT_MESSAGE(glconfig != 0, "failed to create OpenGL config");

    gtk_widget_set_gl_capability(widget, glconfig, g_shared != 0 ? gtk_widget_get_gl_context(g_shared) : 0,  TRUE, GDK_GL_RGBA_TYPE);

    gtk_widget_realize(widget);
    if(g_shared == 0)
    {
      g_shared = widget;
    }

    // free glconfig?
  }
  return FALSE;
}

GtkWidget* glwidget_new(gboolean zbuffer)
{
  GtkWidget* widget = gtk_drawing_area_new();

  g_object_set_data(G_OBJECT(widget), "zbuffer", gint_to_pointer(zbuffer));

  g_signal_connect(G_OBJECT(widget), "hierarchy-changed", G_CALLBACK(glwidget_enable_gl), 0);

  g_signal_connect(G_OBJECT(widget), "realize", G_CALLBACK(glwidget_context_created), 0);
  g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(glwidget_context_destroyed), 0);

  return widget;
}

void glwidget_destroy_context (GtkWidget *widget)
{
}

void glwidget_create_context (GtkWidget *widget)
{
}

void glwidget_swap_buffers (GtkWidget *widget)
{
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  gdk_gl_drawable_swap_buffers (gldrawable);
}

gboolean glwidget_make_current (GtkWidget *widget)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  return gdk_gl_drawable_gl_begin (gldrawable, glcontext);
}

