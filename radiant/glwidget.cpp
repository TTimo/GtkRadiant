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
#include <pango/pangoft2.h>
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


// Think about rewriting this font stuff to use OpenGL display lists and bit maps.
// Bit maps together with display lists may offer a performance increase, but
// they would not allow antialiased fonts.
static const char font_string[] = "Monospace";
static const int font_height = 10;
static int font_ascent = -1;
static int font_descent = -1;
static int y_offset_bitmap_render_pango_units = -1;
static PangoContext *ft2_context = NULL;
static int _debug_font_created = 0;


// Units are pixels.  Returns a positive value [most likely].
int gtk_glwidget_font_ascent()
{
  if (!_debug_font_created) {
    Error("Programming error: gtk_glwidget_font_ascent() called but font does not exist; "
          "you should have called gtk_glwidget_create_font() first");
  }

  return font_ascent;
}

// Units are pixels.  Returns a positive value [most likely].
int gtk_glwidget_font_descent()
{
  if (!_debug_font_created) {
    Error("Programming error: gtk_glwidget_font_descent() called but font does not exist; "
          "you should have called gtk_glwidget_create_font() first");
  }

  return font_descent;
}

void gtk_glwidget_create_font()
{
  PangoFontDescription *font_desc;
  PangoLayout *layout;
  PangoRectangle log_rect;
  int font_ascent_pango_units;
  int font_descent_pango_units;

  if (_debug_font_created) {
    Error("Programming error: gtk_glwidget_create_font() was already called; "
          "you must call gtk_glwidget_destroy_font() before creating font again");
  }
  _debug_font_created = 1;

  // This call is deprecated so we'll have to fix it sometime.
  ft2_context = pango_ft2_get_context(72, 72);

  font_desc = pango_font_description_from_string(font_string);
  pango_font_description_set_size(font_desc, font_height * PANGO_SCALE);
  pango_context_set_font_description(ft2_context, font_desc);
  pango_font_description_free(font_desc);

  layout = pango_layout_new(ft2_context);
#if !PANGO_VERSION_CHECK(1,22,0)
  PangoLayoutIter *iter;  
  iter = pango_layout_get_iter(layout);
  font_ascent_pango_units = pango_layout_iter_get_baseline(iter);
  pango_layout_iter_free(iter);
#else
  font_ascent_pango_units = pango_layout_get_baseline(layout);
#endif
  pango_layout_get_extents(layout, NULL, &log_rect);
  g_object_unref(G_OBJECT(layout));
  font_descent_pango_units = log_rect.height - font_ascent_pango_units;

  font_ascent = PANGO_PIXELS_CEIL(font_ascent_pango_units);
  font_descent = PANGO_PIXELS_CEIL(font_descent_pango_units);
  y_offset_bitmap_render_pango_units = (font_ascent * PANGO_SCALE) - font_ascent_pango_units;
}

void gtk_glwidget_destroy_font()
{
  if (!_debug_font_created) {
    Error("Programming error: gtk_glwidget_destroy_font() called when font "
          "does not exist");
  }

  font_ascent = -1;
  font_descent = -1;
  y_offset_bitmap_render_pango_units = -1;
  g_object_unref(G_OBJECT(ft2_context));
  _debug_font_created = 0;
}


// Renders the input text at the current location with the current color.
// The X position of the current location is used to place the left edge of the text image,
// where the text image bounds are defined as the logical extents of the line of text.
// The Y position of the current location is used to place the bottom of the text image.
// You should offset the Y position by the amount returned by gtk_glwidget_font_descent()
// if you want to place the baseline of the text image at the current Y position.
// Note: A problem with this function is that if the lower left corner of the text falls
// just a hair outside of the viewport (meaning the current raster position is invalid),
// then no text will be rendered.  The solution to this is a very hacky one.  You can search
// Google for "glDrawPixels clipping".
void gtk_glwidget_print_string(const char *s)
{
  // Much of this code is copied from the font-pangoft2.c example that comes with GtkGLExt.

  PangoLayout *layout;
  PangoRectangle ink_rect;
  PangoRectangle log_rect;
  FT_Bitmap bitmap;
  GLvoid *pixels;
  guint32 *p;
  GLfloat color[4];
  guint32 rgb;
  GLfloat alpha;
  guint8 *row, *row_end;
  int i;
  GLint previous_unpack_alignment;
  GLboolean previous_blend_enabled;
  GLint previous_blend_src;
  GLint previous_blend_dst;


  if (!_debug_font_created) {
    Error("Programming error: gtk_glwidget_print_string() called but font does not exist; "
          "you should have called gtk_glwidget_create_font() first");
  }

  layout = pango_layout_new(ft2_context);
  pango_layout_set_width(layout, -1); // -1 no wrapping.  All text on one line.
  pango_layout_set_text(layout, s, -1); // -1 null-terminated string.
  pango_layout_get_extents(layout, &ink_rect, &log_rect);

  if (log_rect.width > 0 && log_rect.height > 0) {
    bitmap.rows = font_ascent + font_descent;
    bitmap.width = PANGO_PIXELS_CEIL(log_rect.width);
    bitmap.pitch = bitmap.width;
    bitmap.buffer = g_malloc(bitmap.rows * bitmap.width);
    bitmap.num_grays = 0xff;
    bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
    memset(bitmap.buffer, 0, bitmap.rows * bitmap.width);
    pango_ft2_render_layout_subpixel(&bitmap, layout, -log_rect.x,
                                     y_offset_bitmap_render_pango_units);

    pixels = g_malloc(bitmap.rows * bitmap.width * 4);
    p = (guint32 *) pixels;
    qglGetFloatv(GL_CURRENT_COLOR, color);
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
    rgb =
      (((guint32) (color[0] * 255.0)) << 0) |
      (((guint32) (color[1] * 255.0)) << 8) |
      (((guint32) (color[2] * 255.0)) << 16);
#else
    rgb =
      (((guint32) (color[0] * 255.0)) << 24) |
      (((guint32) (color[1] * 255.0)) << 16) |
      (((guint32) (color[2] * 255.0)) << 8);
#endif
    alpha = color[3];

    row = bitmap.buffer + bitmap.rows * bitmap.width; // Past the end.
    row_end = bitmap.buffer; // Beginning.

    if (alpha == 1.0) {
      do {
        row -= bitmap.width;
        for (i = 0; i < bitmap.width; i++) {
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
          *p++ = rgb | (((guint32) row[i]) << 24);
#else
          *p++ = rgb | ((guint32) row[i]);
#endif
        }
      } while (row != row_end);
    }

    else { // Translucent.  Much less efficient, so try to avoid.
      do {
        row -= bitmap.width;
        for (i = 0; i < bitmap.width; i++) {
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
          *p++ = rgb | (((guint32) (alpha * row[i])) << 24);
#else
          *p++ = rgb | ((guint32) (alpha * row[i]));
#endif
        }
      } while (row != row_end);
    }

    // Save state.  I didn't see any OpenGL push/pop operations for these.
    // Question: Is saving/restoring this state necessary?
    qglGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment);
    previous_blend_enabled = qglIsEnabled(GL_BLEND);
    qglGetIntegerv(GL_BLEND_SRC, &previous_blend_src);
    qglGetIntegerv(GL_BLEND_DST, &previous_blend_dst);

    qglPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    qglEnable(GL_BLEND);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if !defined(GL_VERSION_1_2)
    qglDrawPixels(bitmap.width, bitmap.rows,
                  GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
    qglDrawPixels(bitmap.width, bitmap.rows,
                  GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
#endif

    // Restore state in reverse order of how we set it.
    qglBlendFunc(previous_blend_src, previous_blend_dst);
    if (!previous_blend_enabled) { qglDisable(GL_BLEND); }
    qglPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);

    g_free(bitmap.buffer);
    g_free(pixels);
  }

  g_object_unref(G_OBJECT(layout));
}

void gtk_glwidget_print_char(char s)
{
  char str[2];
  str[0] = s;
  str[1] = '\0';
  gtk_glwidget_print_string(str);
}
