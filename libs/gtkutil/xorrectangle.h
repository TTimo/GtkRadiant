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

#if !defined (INCLUDED_XORRECTANGLE_H)
#define INCLUDED_XORRECTANGLE_H

#include <gtk/gtkwidget.h>
#include "math/vector.h"

class rectangle_t
{
public:
  rectangle_t()
    : x(0), y(0), w(0), h(0)
  {}
  rectangle_t(float _x, float _y, float _w, float _h)
    : x(_x), y(_y), w(_w), h(_h)
  {}
  float x;
  float y;
  float w;
  float h;
};

struct Coord2D
{
  float x, y;
  Coord2D(float _x, float _y)
    : x(_x), y(_y)
  {
  }
};

inline Coord2D coord2d_device2screen(const Coord2D& coord, unsigned int width, unsigned int height)
{
  return Coord2D(((coord.x + 1.0f) * 0.5f) * width, ((coord.y + 1.0f) * 0.5f) * height);
}

inline rectangle_t rectangle_from_area(const float min[2], const float max[2], unsigned int width, unsigned int height)
{
  Coord2D botleft(coord2d_device2screen(Coord2D(min[0], min[1]), width, height));
  Coord2D topright(coord2d_device2screen(Coord2D(max[0], max[1]), width, height));
  return rectangle_t(botleft.x, botleft.y, topright.x - botleft.x, topright.y - botleft.y);
}

class XORRectangle
{

  rectangle_t m_rectangle;

  GtkWidget* m_widget;
  GdkGC* m_gc;

  bool initialised() const
  {
    return m_gc != 0;
  }
  void lazy_init()
  {
    if(!initialised())
    {
      m_gc = gdk_gc_new(m_widget->window);

      GdkColor color = { 0, 0xffff, 0xffff, 0xffff, };
      GdkColormap* colormap = gdk_window_get_colormap(m_widget->window);
      gdk_colormap_alloc_color (colormap, &color, FALSE, TRUE);
      gdk_gc_copy(m_gc, m_widget->style->white_gc);
      gdk_gc_set_foreground(m_gc, &color);
      gdk_gc_set_background(m_gc, &color);

      gdk_gc_set_function(m_gc, GDK_INVERT);
    }
  }
  void draw() const
  {
    const int x = float_to_integer(m_rectangle.x);
    const int y = float_to_integer(m_rectangle.y);
    const int w = float_to_integer(m_rectangle.w);
    const int h = float_to_integer(m_rectangle.h);
    gdk_draw_rectangle(m_widget->window, m_gc, FALSE, x, -(h) - (y - m_widget->allocation.height), w, h);
  }

public:
  XORRectangle(GtkWidget* widget) : m_widget(widget), m_gc(0)
  {
  }
  ~XORRectangle()
  {
    if(initialised())
    {
      gdk_gc_unref(m_gc);
    }
  }
  void set(rectangle_t rectangle)
  {
    if(GTK_WIDGET_REALIZED(m_widget))
    {
      lazy_init();
      draw();
      m_rectangle = rectangle;
      draw();
    }
  }
};


#endif
