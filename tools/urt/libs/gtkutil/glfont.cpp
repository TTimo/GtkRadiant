
#include "glfont.h"

#include "igl.h"
#include <gtk/gtkglwidget.h>

GLFont glfont_create(const char* font_string)
{
  GLuint font_list_base = glGenLists (256);
  gint font_height = 0;

  PangoFontDescription* font_desc = pango_font_description_from_string (font_string);

  PangoFont* font = gdk_gl_font_use_pango_font (font_desc, 0, 256, font_list_base);

  if(font != 0)
  {
    PangoFontMetrics* font_metrics = pango_font_get_metrics (font, 0);

    font_height = pango_font_metrics_get_ascent (font_metrics) +
                  pango_font_metrics_get_descent (font_metrics);
    font_height = PANGO_PIXELS (font_height);

    pango_font_metrics_unref (font_metrics);
  }

  pango_font_description_free (font_desc);

  return GLFont(font_list_base, font_height);
}

void glfont_release(GLFont& font)
{
  glDeleteLists(font.getDisplayList(), 256);
  font = GLFont(0, 0);
}
