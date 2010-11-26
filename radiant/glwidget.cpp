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

// OpenGL widget based on GtkGLExt

#include "stdafx.h"

#include <gtk/gtkgl.h>
#include "glwidget.h"
#include "qgl.h"

typedef int* attribs_t;
typedef const attribs_t* configs_iterator;

int config_rgba32[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 8,
  GDK_GL_BLUE_SIZE, 8,
  GDK_GL_GREEN_SIZE, 8,
  GDK_GL_ALPHA_SIZE, 8,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 1,
  GDK_GL_BLUE_SIZE, 1,
  GDK_GL_GREEN_SIZE, 1,
  GDK_GL_ALPHA_SIZE, 1,
  GDK_GL_ATTRIB_LIST_NONE,
};

const attribs_t configs[] = {
  config_rgba32,
  config_rgba,
};

GdkGLConfig* glconfig_new()
{
  GdkGLConfig* glconfig = NULL;

  for(configs_iterator i = configs, end = configs + 2; i != end && glconfig == NULL; ++i)
  {
    glconfig = gdk_gl_config_new(*i);
  }

  if(glconfig == NULL)
  {
    return gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE));
  }

  return glconfig;
}

int config_rgba32_depth32[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 8,
  GDK_GL_BLUE_SIZE, 8,
  GDK_GL_GREEN_SIZE, 8,
  GDK_GL_ALPHA_SIZE, 8,
  GDK_GL_DEPTH_SIZE, 32,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth24[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 8,
  GDK_GL_BLUE_SIZE, 8,
  GDK_GL_GREEN_SIZE, 8,
  GDK_GL_ALPHA_SIZE, 8,
  GDK_GL_DEPTH_SIZE, 24,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth16[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 8,
  GDK_GL_BLUE_SIZE, 8,
  GDK_GL_GREEN_SIZE, 8,
  GDK_GL_ALPHA_SIZE, 8,
  GDK_GL_DEPTH_SIZE, 16,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba32_depth[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 8,
  GDK_GL_BLUE_SIZE, 8,
  GDK_GL_GREEN_SIZE, 8,
  GDK_GL_ALPHA_SIZE, 8,
  GDK_GL_DEPTH_SIZE, 1,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba_depth16[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 1,
  GDK_GL_BLUE_SIZE, 1,
  GDK_GL_GREEN_SIZE, 1,
  GDK_GL_ALPHA_SIZE, 1,
  GDK_GL_DEPTH_SIZE, 16,
  GDK_GL_ATTRIB_LIST_NONE,
};

int config_rgba_depth[] = {
  GDK_GL_RGBA,
  GDK_GL_DOUBLEBUFFER,
  GDK_GL_RED_SIZE, 1,
  GDK_GL_BLUE_SIZE, 1,
  GDK_GL_GREEN_SIZE, 1,
  GDK_GL_ALPHA_SIZE, 1,
  GDK_GL_DEPTH_SIZE, 1,
  GDK_GL_ATTRIB_LIST_NONE,
};

const attribs_t configs_with_depth[] =
{
  config_rgba32_depth32,
  config_rgba32_depth24,
  config_rgba32_depth16,
  config_rgba32_depth,
  config_rgba_depth16,
  config_rgba_depth,
};

GdkGLConfig* glconfig_new_with_depth()
{
  GdkGLConfig* glconfig = NULL;

  for(configs_iterator i = configs_with_depth, end = configs_with_depth + 6; i != end && glconfig == NULL; ++i)
  {
    glconfig = gdk_gl_config_new(*i);
  }

  if(glconfig == NULL)
  {
    return gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH));
  }

  return glconfig;
}

GtkWidget* WINAPI gtk_glwidget_new (gboolean zbuffer, GtkWidget* share)
{
  GtkWidget* drawing_area = gtk_drawing_area_new();
  GdkGLConfig* glconfig = (zbuffer) ? glconfig_new_with_depth() : glconfig_new();
  GdkGLContext* shared_context = (share) ? gtk_widget_get_gl_context(share) : NULL;

  gtk_widget_set_gl_capability (drawing_area, glconfig, shared_context, TRUE, GDK_GL_RGBA_TYPE);

  return drawing_area;
}

void WINAPI gtk_glwidget_destroy_context (GtkWidget *widget)
{
}

void WINAPI gtk_glwidget_create_context (GtkWidget *widget)
{
}

void WINAPI gtk_glwidget_swap_buffers (GtkWidget *widget)
{
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  gdk_gl_drawable_swap_buffers (gldrawable);
}

gboolean WINAPI gtk_glwidget_make_current (GtkWidget *widget)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  return gdk_gl_drawable_gl_begin (gldrawable, glcontext);
}

GLuint font_list_base;
static gchar font_string[] = "courier 8";
static gint font_height;

void gtk_glwidget_create_font (GtkWidget *widget)
{
  PangoFontDescription *font_desc;
  PangoFont *font;
  PangoFontMetrics *font_metrics;

  font_list_base = qglGenLists (256);

  font_desc = pango_font_description_from_string (font_string);

  font = gdk_gl_font_use_pango_font (font_desc, 0, 256, font_list_base);

  if(font != NULL)
  {
    font_metrics = pango_font_get_metrics (font, NULL);

    font_height = pango_font_metrics_get_ascent (font_metrics) +
                  pango_font_metrics_get_descent (font_metrics);
    font_height = PANGO_PIXELS (font_height);

    pango_font_metrics_unref (font_metrics);
  }

  pango_font_description_free (font_desc);
}


void gtk_glwidget_print_string(const char *s)
{
  qglListBase(font_list_base);
  qglCallLists(strlen(s), GL_UNSIGNED_BYTE, (unsigned char *)s);
}

void gtk_glwidget_print_char(char s)
{
  qglListBase(font_list_base);
  qglCallLists(1, GL_UNSIGNED_BYTE, (unsigned char *) &s);
}

