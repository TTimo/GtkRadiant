
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
