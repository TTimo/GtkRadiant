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

#if !defined(INCLUDED_GTKUTIL_IDLEDRAW_H)
#define INCLUDED_GTKUTIL_IDLEDRAW_H

#include <glib/gmain.h>

#include "generic/callback.h"

class IdleDraw
{
  Callback m_draw;
  unsigned int m_handler;
  static gboolean draw(gpointer data)
  {
    reinterpret_cast<IdleDraw*>(data)->m_draw();
    reinterpret_cast<IdleDraw*>(data)->m_handler = 0;
    return FALSE;
  }
public:
  IdleDraw(const Callback& draw) : m_draw(draw), m_handler(0)
  {
  }
  ~IdleDraw()
  {
    if(m_handler != 0)
    {
      g_source_remove(m_handler);
    }
  }
  void queueDraw()
  {
    if(m_handler == 0)
    {
      m_handler = g_idle_add(&draw, this);
    }
  }
  typedef MemberCaller<IdleDraw, &IdleDraw::queueDraw> QueueDrawCaller;

  void flush()
  {
    if(m_handler != 0)
    {
      draw(this);
    }
  }
};


#endif
