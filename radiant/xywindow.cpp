/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

//
// XY Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "xywindow.h"

#include "debugging/debugging.h"

#include "ientity.h"
#include "igl.h"
#include "ibrush.h"
#include "iundo.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkmenuitem.h>

#include "generic/callback.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "scenelib.h"
#include "eclasslib.h"
#include "renderer.h"
#include "moduleobserver.h"

#include "gtkutil/menu.h"
#include "gtkutil/container.h"
#include "gtkutil/widget.h"
#include "gtkutil/glwidget.h"
#include "gtkmisc.h"
#include "select.h"
#include "csg.h"
#include "brushmanip.h"
#include "selection.h"
#include "entity.h"
#include "camwindow.h"
#include "texwindow.h"
#include "mainframe.h"
#include "preferences.h"
#include "commands.h"
#include "feedback.h"
#include "grid.h"
#include "windowobservers.h"




//!\todo Rewrite.
class ClipPoint
{
public:
  Vector3 m_ptClip;      // the 3d point
  bool m_bSet;

  ClipPoint()
  {
    Reset();
  };
  void Reset()
  {
    m_ptClip[0] = m_ptClip[1] = m_ptClip[2] = 0.0;
    m_bSet = false;
  }
  bool Set()
  {
    return m_bSet;
  }
  void Set(bool b)
  {
    m_bSet = b;
  }
  operator Vector3&()
  {
    return m_ptClip;
  };
  
  /*! Draw clip/path point with rasterized number label */
  void Draw(int num, float scale);
  /*! Draw clip/path point with rasterized string label */
  void Draw(const char *label, float scale);
};

VIEWTYPE g_clip_viewtype;
bool g_bSwitch = true;
bool g_clip_useCaulk = false;
ClipPoint g_Clip1;
ClipPoint g_Clip2;
ClipPoint g_Clip3;
ClipPoint* g_pMovingClip = 0;

/* Drawing clip points */
void ClipPoint::Draw(int num, float scale)
{
  StringOutputStream label(4);
  label << num;
  Draw(label.c_str(), scale);
}

void ClipPoint::Draw(const char *label, float scale)
{
  // draw point
  glPointSize (4);
  glColor3fv(vector3_to_array(g_xywindow_globals.color_clipper));
  glBegin (GL_POINTS);
  glVertex3fv(vector3_to_array(m_ptClip));
  glEnd();
  glPointSize (1);

  float offset = 2.0f / scale;

  // draw label
  glRasterPos3f (m_ptClip[0] + offset, m_ptClip[1] + offset, m_ptClip[2] + offset);
  glCallLists (GLsizei(strlen(label)), GL_UNSIGNED_BYTE, label);
}

float fDiff(float f1, float f2)
{
  if (f1 > f2)
    return f1 - f2;
  else
    return f2 - f1;
}

inline double ClipPoint_Intersect(const ClipPoint& clip, const Vector3& point, VIEWTYPE viewtype, float scale)
{
  int nDim1 = (viewtype == YZ) ? 1 : 0;
  int nDim2 = (viewtype == XY) ? 1 : 2;
  double screenDistanceSquared(vector2_length_squared(Vector2(fDiff(clip.m_ptClip[nDim1], point[nDim1]) * scale, fDiff(clip.m_ptClip[nDim2], point[nDim2])  * scale)));
  if(screenDistanceSquared < 8*8)
  {
    return screenDistanceSquared;
  }
  return FLT_MAX;
}

inline void ClipPoint_testSelect(ClipPoint& clip, const Vector3& point, VIEWTYPE viewtype, float scale, double& bestDistance, ClipPoint*& bestClip)
{
  if(clip.Set())
  {
    double distance = ClipPoint_Intersect(clip, point, viewtype, scale);
    if(distance < bestDistance)
    {
      bestDistance = distance;
      bestClip = &clip;
    }
  }
}

inline ClipPoint* GlobalClipPoints_Find(const Vector3& point, VIEWTYPE viewtype, float scale)
{
  double bestDistance = FLT_MAX;
  ClipPoint* bestClip = 0;
  ClipPoint_testSelect(g_Clip1, point, viewtype, scale, bestDistance, bestClip);
  ClipPoint_testSelect(g_Clip2, point, viewtype, scale, bestDistance, bestClip);
  ClipPoint_testSelect(g_Clip3, point, viewtype, scale, bestDistance, bestClip);
  return bestClip;
}

inline void GlobalClipPoints_Draw(float scale)
{
  // Draw clip points
  if (g_Clip1.Set())
    g_Clip1.Draw(1, scale);
  if (g_Clip2.Set())
    g_Clip2.Draw(2, scale);
  if (g_Clip3.Set())
    g_Clip3.Draw(3, scale);
}

inline bool GlobalClipPoints_valid()
{
  return g_Clip1.Set() && g_Clip2.Set();
}

void PlanePointsFromClipPoints(Vector3 planepts[3], const AABB& bounds, int viewtype)
{
  ASSERT_MESSAGE(GlobalClipPoints_valid(), "clipper points not initialised");
  planepts[0] = g_Clip1.m_ptClip;
	planepts[1] = g_Clip2.m_ptClip;
	planepts[2] = g_Clip3.m_ptClip;
  Vector3 maxs(vector3_added(bounds.origin, bounds.extents));
  Vector3 mins(vector3_subtracted(bounds.origin, bounds.extents));
	if(!g_Clip3.Set())
	{
		int n = (viewtype == XY) ? 2 : (viewtype == YZ) ? 0 : 1;
		int x = (n == 0) ? 1 : 0;
		int y = (n == 2) ? 1 : 2;
		
		if (n == 1) // on viewtype XZ, flip clip points
		{
		  planepts[0][n] = maxs[n];
		  planepts[1][n] = maxs[n];
		  planepts[2][x] = g_Clip1.m_ptClip[x];
		  planepts[2][y] = g_Clip1.m_ptClip[y];
		  planepts[2][n] = mins[n];
		}
		else
		{
		  planepts[0][n] = mins[n];
		  planepts[1][n] = mins[n];
		  planepts[2][x] = g_Clip1.m_ptClip[x];
		  planepts[2][y] = g_Clip1.m_ptClip[y];
		  planepts[2][n] = maxs[n];
		}
	}
}

void Clip_Update()
{
  Vector3 planepts[3];
  if(!GlobalClipPoints_valid())
  {
    planepts[0] = Vector3(0, 0, 0);
	  planepts[1] = Vector3(0, 0, 0);
	  planepts[2] = Vector3(0, 0, 0);
    Scene_BrushSetClipPlane(GlobalSceneGraph(), Plane3(0, 0, 0, 0));
  }
  else
  {
    AABB bounds(Vector3(0, 0, 0), Vector3(64, 64, 64));
    PlanePointsFromClipPoints(planepts, bounds, g_clip_viewtype);
    if(g_bSwitch)
    {
      std::swap(planepts[0], planepts[1]);
    }
    Scene_BrushSetClipPlane(GlobalSceneGraph(), plane3_for_points(planepts[0], planepts[1], planepts[2]));
  }
  ClipperChangeNotify();
}

const char* Clip_getShader()
{
  return g_clip_useCaulk ? "textures/common/caulk" : TextureBrowser_GetSelectedShader(GlobalTextureBrowser());
}

void Clip()
{
  if (ClipMode() && GlobalClipPoints_valid())
  {
    Vector3 planepts[3];
    AABB bounds(Vector3(0, 0, 0), Vector3(64, 64, 64));
    PlanePointsFromClipPoints(planepts, bounds, g_clip_viewtype);
    Scene_BrushSplitByPlane(GlobalSceneGraph(), planepts[0], planepts[1], planepts[2], Clip_getShader(), (!g_bSwitch) ? eFront : eBack);
    g_Clip1.Reset();
    g_Clip2.Reset();
    g_Clip3.Reset();
    Clip_Update();
    ClipperChangeNotify();
  }
}

void SplitClip()
{
  if (ClipMode() && GlobalClipPoints_valid())
  {
    Vector3 planepts[3];
    AABB bounds(Vector3(0, 0, 0), Vector3(64, 64, 64));
    PlanePointsFromClipPoints(planepts, bounds, g_clip_viewtype);
    Scene_BrushSplitByPlane(GlobalSceneGraph(), planepts[0], planepts[1], planepts[2], Clip_getShader(), eFrontAndBack);
    g_Clip1.Reset();
    g_Clip2.Reset();
    g_Clip3.Reset();
    Clip_Update();
    ClipperChangeNotify();
  }
}

void FlipClip()
{
  g_bSwitch = !g_bSwitch;
  Clip_Update();
  ClipperChangeNotify();
}

void OnClipMode(bool enabled)
{
  g_Clip1.Reset();
  g_Clip2.Reset();
  g_Clip3.Reset();

  if(!enabled && g_pMovingClip)
  {
    g_pMovingClip = 0;
  }

  Clip_Update();
  ClipperChangeNotify();
}

bool ClipMode()
{
  return GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eClip;
}

void NewClipPoint(const Vector3& point)
{
  if (g_Clip1.Set() == false)
  {
    g_Clip1.m_ptClip = point;
    g_Clip1.Set(true);
  }
  else if (g_Clip2.Set() == false)
  {
    g_Clip2.m_ptClip = point;
    g_Clip2.Set(true);
  }
  else if (g_Clip3.Set() == false)
  {
    g_Clip3.m_ptClip = point;
    g_Clip3.Set(true);
  }
  else 
  {
    g_Clip1.Reset();
    g_Clip2.Reset();
    g_Clip3.Reset();
    g_Clip1.m_ptClip = point;
    g_Clip1.Set(true);
  }

  Clip_Update();
  ClipperChangeNotify();
}



struct xywindow_globals_private_t
{
  bool  d_showgrid;

  // these are in the View > Show menu with Show coordinates
  bool  show_names;
  bool  show_coordinates;
  bool  show_angles;
  bool  show_outline;
  bool  show_axis;

  bool d_show_work;

  bool     show_blocks;
  int		       blockSize;

  bool m_bCamXYUpdate;
  bool m_bChaseMouse;
  bool m_bSizePaint;

  xywindow_globals_private_t() :
    d_showgrid(true),

    show_names(false),
    show_coordinates(true),
    show_angles(true),
    show_outline(false),
    show_axis(true),

    d_show_work(false),

    show_blocks(false),

    m_bCamXYUpdate(true),
    m_bChaseMouse(true),
    m_bSizePaint(false)
  {
  }

};

xywindow_globals_t g_xywindow_globals;
xywindow_globals_private_t g_xywindow_globals_private;

const unsigned int RAD_NONE =    0x00;
const unsigned int RAD_SHIFT =   0x01;
const unsigned int RAD_ALT =     0x02;
const unsigned int RAD_CONTROL = 0x04;
const unsigned int RAD_PRESS   = 0x08;
const unsigned int RAD_LBUTTON = 0x10;
const unsigned int RAD_MBUTTON = 0x20;
const unsigned int RAD_RBUTTON = 0x40;

inline ButtonIdentifier button_for_flags(unsigned int flags)
{
  if(flags & RAD_LBUTTON)
    return c_buttonLeft;
  if(flags & RAD_RBUTTON)
    return c_buttonRight;
  if(flags & RAD_MBUTTON)
    return c_buttonMiddle;
  return c_buttonInvalid;
}

inline ModifierFlags modifiers_for_flags(unsigned int flags)
{
  ModifierFlags modifiers = c_modifierNone;
  if(flags & RAD_SHIFT)
    modifiers |= c_modifierShift;
  if(flags & RAD_CONTROL)
    modifiers |= c_modifierControl;
  if(flags & RAD_ALT)
    modifiers |= c_modifierAlt;
  return modifiers;
}

inline unsigned int buttons_for_event_button(GdkEventButton* event)
{
  unsigned int flags = 0;

  switch (event->button)
  {
  case 1: flags |= RAD_LBUTTON; break;
  case 2: flags |= RAD_MBUTTON; break;
  case 3: flags |= RAD_RBUTTON; break;
  }

  if ((event->state & GDK_CONTROL_MASK) != 0)
    flags |= RAD_CONTROL;

  if ((event->state & GDK_SHIFT_MASK) != 0)
    flags |= RAD_SHIFT;

  if((event->state & GDK_MOD1_MASK) != 0)
    flags |= RAD_ALT;

  return flags;
}

inline unsigned int buttons_for_state(guint state)
{
  unsigned int flags = 0;

  if ((state & GDK_BUTTON1_MASK) != 0)
    flags |= RAD_LBUTTON;

  if ((state & GDK_BUTTON2_MASK) != 0)
    flags |= RAD_MBUTTON;

  if ((state & GDK_BUTTON3_MASK) != 0)
    flags |= RAD_RBUTTON;

  if ((state & GDK_CONTROL_MASK) != 0)
    flags |= RAD_CONTROL;

  if ((state & GDK_SHIFT_MASK) != 0)
    flags |= RAD_SHIFT;

  if ((state & GDK_MOD1_MASK) != 0)
    flags |= RAD_ALT;

  return flags;
}


void XYWnd::SetScale(float f)
{
  m_fScale = f;
  updateModelview();
  XYWnd_Update(*this);
}

void XYWnd_ZoomIn(XYWnd* xy)
{
  float max_scale = 64;
  float scale = xy->Scale() * 5.0f / 4.0f;
  if(scale > max_scale)
  {
    if(xy->Scale() != max_scale)
    {
      xy->SetScale (max_scale);
    }
  }
  else
  {
    xy->SetScale(scale);
  }
}


// NOTE: the zoom out factor is 4/5, we could think about customizing it
//  we don't go below a zoom factor corresponding to 10% of the max world size
//  (this has to be computed against the window size)
void XYWnd_ZoomOut(XYWnd* xy)
{
  float min_scale = MIN(xy->Width(),xy->Height()) / ( 1.1f * (g_MaxWorldCoord-g_MinWorldCoord));
  float scale = xy->Scale() * 4.0f / 5.0f;
  if(scale < min_scale)
  {
    if(xy->Scale() != min_scale)
    {
      xy->SetScale (min_scale);
    }
  }
  else
  {
    xy->SetScale(scale);
  }
}

VIEWTYPE GlobalXYWnd_getCurrentViewType()
{
  ASSERT_NOTNULL(g_pParentWnd);
  ASSERT_NOTNULL(g_pParentWnd->ActiveXY());
  return g_pParentWnd->ActiveXY()->GetViewType();
}

// =============================================================================
// variables

bool g_bCrossHairs = false;

GtkMenu* XYWnd::m_mnuDrop = 0;

// this is disabled, and broken
// http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=394
#if 0
void WXY_Print()
{
  long width, height;
  width = g_pParentWnd->ActiveXY()->Width();
  height = g_pParentWnd->ActiveXY()->Height();
  unsigned char* img;
  const char* filename;

  filename = file_dialog(GTK_WIDGET(MainFrame_getWindow()), FALSE, "Save Image", 0, FILTER_BMP);
  if (!filename)
    return;

  g_pParentWnd->ActiveXY()->MakeCurrent();
  img = (unsigned char*)malloc (width*height*3);
  glReadPixels (0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,img);

  FILE *fp; 
  fp = fopen(filename, "wb");
  if (fp)
  {
    unsigned short bits;
    unsigned long cmap, bfSize;

    bits = 24;
    cmap = 0;
    bfSize = 54 + width*height*3;

    long byteswritten = 0;
    long pixoff = 54 + cmap*4;
    short res = 0;
    char m1 ='B', m2 ='M';
    fwrite(&m1, 1, 1, fp);      byteswritten++; // B
    fwrite(&m2, 1, 1, fp);      byteswritten++; // M
    fwrite(&bfSize, 4, 1, fp);  byteswritten+=4;// bfSize
    fwrite(&res, 2, 1, fp);     byteswritten+=2;// bfReserved1
    fwrite(&res, 2, 1, fp);     byteswritten+=2;// bfReserved2
    fwrite(&pixoff, 4, 1, fp);  byteswritten+=4;// bfOffBits

    unsigned long biSize = 40, compress = 0, size = 0;
    long pixels = 0;
    unsigned short planes = 1;
    fwrite(&biSize, 4, 1, fp);  byteswritten+=4;// biSize
    fwrite(&width, 4, 1, fp);   byteswritten+=4;// biWidth
    fwrite(&height, 4, 1, fp);  byteswritten+=4;// biHeight
    fwrite(&planes, 2, 1, fp);  byteswritten+=2;// biPlanes
    fwrite(&bits, 2, 1, fp);    byteswritten+=2;// biBitCount
    fwrite(&compress, 4, 1, fp);byteswritten+=4;// biCompression
    fwrite(&size, 4, 1, fp);    byteswritten+=4;// biSizeImage
    fwrite(&pixels, 4, 1, fp);  byteswritten+=4;// biXPelsPerMeter
    fwrite(&pixels, 4, 1, fp);  byteswritten+=4;// biYPelsPerMeter
    fwrite(&cmap, 4, 1, fp);    byteswritten+=4;// biClrUsed
    fwrite(&cmap, 4, 1, fp);    byteswritten+=4;// biClrImportant

    unsigned long widthDW = (((width*24) + 31) / 32 * 4);
    long row, row_size = width*3;
    for (row = 0; row < height; row++)
    {
        unsigned char* buf = img+row*row_size;

      // write a row
      int col;
      for (col = 0; col < row_size; col += 3)
        {
          putc(buf[col+2], fp);
          putc(buf[col+1], fp);
          putc(buf[col], fp);
        }
      byteswritten += row_size; 

      unsigned long count;
      for (count = row_size; count < widthDW; count++)
        {
        putc(0, fp);    // dummy
          byteswritten++;
        }
    }

    fclose(fp);
  }

  free (img);
}
#endif


#include "timer.h"

Timer g_chasemouse_timer;

void XYWnd::ChaseMouse()
{
  float multiplier = g_chasemouse_timer.elapsed_msec() / 10.0f;
  Scroll(float_to_integer(multiplier * m_chasemouse_delta_x), float_to_integer(multiplier * -m_chasemouse_delta_y));

  //globalOutputStream() << "chasemouse: multiplier=" << multiplier << " x=" << m_chasemouse_delta_x << " y=" << m_chasemouse_delta_y << '\n';

  XY_MouseMoved(m_chasemouse_current_x, m_chasemouse_current_y , getButtonState());
  g_chasemouse_timer.start();
}

gboolean xywnd_chasemouse(gpointer data)
{
  reinterpret_cast<XYWnd*>(data)->ChaseMouse();
  return TRUE;
}

inline const int& min_int(const int& left, const int& right)
{
  return std::min(left, right);
}

bool XYWnd::chaseMouseMotion(int pointx, int pointy)
{
  m_chasemouse_delta_x = 0;
  m_chasemouse_delta_y = 0;

  if (g_xywindow_globals_private.m_bChaseMouse && getButtonState() == RAD_LBUTTON)
  {
    const int epsilon = 16;

    if (pointx < epsilon)
    {
      m_chasemouse_delta_x = std::max(pointx, 0) - epsilon;
    }
    else if ((pointx - m_nWidth) > -epsilon)
    {
      m_chasemouse_delta_x = min_int((pointx - m_nWidth), 0) + epsilon;
    }

    if (pointy < epsilon)
    {
      m_chasemouse_delta_y = std::max(pointy, 0) - epsilon;
    }
    else if ((pointy - m_nHeight) > -epsilon)
    {
      m_chasemouse_delta_y = min_int((pointy - m_nHeight), 0) + epsilon;
    }

    if(m_chasemouse_delta_y != 0 || m_chasemouse_delta_x != 0)
    {
      //globalOutputStream() << "chasemouse motion: x=" << pointx << " y=" << pointy << "... ";
      m_chasemouse_current_x = pointx;
      m_chasemouse_current_y = pointy;
      if(m_chasemouse_handler == 0)
      {
        //globalOutputStream() << "chasemouse timer start... ";
        g_chasemouse_timer.start();
        m_chasemouse_handler = g_idle_add(xywnd_chasemouse, this);
      }
      return true;
    }
    else
    {
      if(m_chasemouse_handler != 0)
      {
        //globalOutputStream() << "chasemouse cancel\n";
        g_source_remove(m_chasemouse_handler);
        m_chasemouse_handler = 0;
      }
    }
  }
  else
  {
    if(m_chasemouse_handler != 0)
    {
      //globalOutputStream() << "chasemouse cancel\n";
      g_source_remove(m_chasemouse_handler);
      m_chasemouse_handler = 0;
    }
  }
  return false;
}

// =============================================================================
// XYWnd class
Shader* XYWnd::m_state_selected = 0;

void xy_update_xor_rectangle(XYWnd& self, rect_t area)
{
  if(GTK_WIDGET_VISIBLE(self.GetWidget()))
  {
    self.m_XORRectangle.set(rectangle_from_area(area.min, area.max, self.Width(), self.Height()));
  }
}

gboolean xywnd_button_press(GtkWidget* widget, GdkEventButton* event, XYWnd* xywnd)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    xywnd->XY_MouseDown(static_cast<int>(event->x), static_cast<int>(event->y), buttons_for_event_button(event));
  }
  return FALSE;
}

gboolean xywnd_button_release(GtkWidget* widget, GdkEventButton* event, XYWnd* xywnd)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    xywnd->XY_MouseUp(static_cast<int>(event->x), static_cast<int>(event->y), buttons_for_event_button(event));
  }
  return FALSE;
}

void xywnd_motion(gdouble x, gdouble y, guint state, void* data)
{
  if(reinterpret_cast<XYWnd*>(data)->chaseMouseMotion(static_cast<int>(x), static_cast<int>(y)))
  {
    return;
  }
  reinterpret_cast<XYWnd*>(data)->XY_MouseMoved(static_cast<int>(x), static_cast<int>(y), buttons_for_state(state));
}

gboolean xywnd_wheel_scroll(GtkWidget* widget, GdkEventScroll* event, XYWnd* xywnd)
{
  if(event->direction == GDK_SCROLL_UP)
  {
    XYWnd_ZoomIn(xywnd);
  }
  else if(event->direction == GDK_SCROLL_DOWN)
  {
    XYWnd_ZoomOut(xywnd);
  }
  return FALSE;
}

gboolean xywnd_size_allocate(GtkWidget* widget, GtkAllocation* allocation, XYWnd* xywnd)
{
  xywnd->m_nWidth = allocation->width;
  xywnd->m_nHeight = allocation->height;
  xywnd->updateProjection();
  xywnd->m_window_observer->onSizeChanged(xywnd->Width(), xywnd->Height());
  return FALSE;
}

gboolean xywnd_expose(GtkWidget* widget, GdkEventExpose* event, XYWnd* xywnd)
{
  if(glwidget_make_current(xywnd->GetWidget()) != FALSE)
  {
    if(Map_Valid(g_map) && ScreenUpdates_Enabled())
    {
      GlobalOpenGL_debugAssertNoErrors();
      xywnd->XY_Draw();
      GlobalOpenGL_debugAssertNoErrors();

      xywnd->m_XORRectangle.set(rectangle_t());
    }
    glwidget_swap_buffers(xywnd->GetWidget());
  }
  return FALSE;
}


void XYWnd_CameraMoved(XYWnd& xywnd)
{
  if(g_xywindow_globals_private.m_bCamXYUpdate)
  {
    XYWnd_Update(xywnd);
  }
}

XYWnd::XYWnd() :
  m_gl_widget(glwidget_new(FALSE)),
  m_deferredDraw(WidgetQueueDrawCaller(*m_gl_widget)),
  m_deferred_motion(xywnd_motion, this),
  m_parent(0),
  m_window_observer(NewWindowObserver()),
  m_XORRectangle(m_gl_widget),
  m_chasemouse_handler(0)
{
  m_bActive = false;
  m_buttonstate = 0;

  m_bNewBrushDrag = false;
  m_move_started = false;
  m_zoom_started = false;

  m_nWidth = 0;
  m_nHeight = 0;

  m_vOrigin[0] = 0;
  m_vOrigin[1] = 20;
  m_vOrigin[2] = 46;
  m_fScale = 1;
  m_viewType = XY;

  m_entityCreate = false;

  m_mnuDrop = 0;

  GlobalWindowObservers_add(m_window_observer);
  GlobalWindowObservers_connectWidget(m_gl_widget);

  m_window_observer->setRectangleDrawCallback(ReferenceCaller1<XYWnd, rect_t, xy_update_xor_rectangle>(*this));
  m_window_observer->setView(m_view);

  gtk_widget_ref(m_gl_widget);

  gtk_widget_set_events(m_gl_widget, GDK_DESTROY | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
  GTK_WIDGET_SET_FLAGS(m_gl_widget, GTK_CAN_FOCUS);

  m_sizeHandler = g_signal_connect(G_OBJECT(m_gl_widget), "size_allocate", G_CALLBACK(xywnd_size_allocate), this);
  m_exposeHandler = g_signal_connect(G_OBJECT(m_gl_widget), "expose_event", G_CALLBACK(xywnd_expose), this);

  g_signal_connect(G_OBJECT(m_gl_widget), "button_press_event", G_CALLBACK(xywnd_button_press), this);
  g_signal_connect(G_OBJECT(m_gl_widget), "button_release_event", G_CALLBACK(xywnd_button_release), this);
  g_signal_connect(G_OBJECT(m_gl_widget), "motion_notify_event", G_CALLBACK(DeferredMotion::gtk_motion), &m_deferred_motion);

  g_signal_connect(G_OBJECT(m_gl_widget), "scroll_event", G_CALLBACK(xywnd_wheel_scroll), this);

  Map_addValidCallback(g_map, DeferredDrawOnMapValidChangedCaller(m_deferredDraw));

  updateModelview();

  AddSceneChangeCallback(ReferenceCaller<XYWnd, &XYWnd_Update>(*this));
  AddCameraMovedCallback(ReferenceCaller<XYWnd, &XYWnd_CameraMoved>(*this));

  PressedButtons_connect(g_pressedButtons, m_gl_widget);
}

XYWnd::~XYWnd()
{
  if(m_mnuDrop != 0)
  {
    gtk_widget_destroy(GTK_WIDGET(m_mnuDrop));
    m_mnuDrop = 0;
  }

  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_sizeHandler);
  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_exposeHandler);

  gtk_widget_unref(m_gl_widget);

  m_window_observer->release();
}

void XYWnd::captureStates()
{
  m_state_selected = GlobalShaderCache().capture("$XY_OVERLAY");
}

void XYWnd::releaseStates()
{
  GlobalShaderCache().release("$XY_OVERLAY");
}

const Vector3& XYWnd::GetOrigin()
{
  return m_vOrigin;
}

void XYWnd::SetOrigin(const Vector3& origin)
{
  m_vOrigin = origin;
  updateModelview();
}

void XYWnd::Scroll(int x, int y)
{
  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;
  m_vOrigin[nDim1] += x / m_fScale;
  m_vOrigin[nDim2] += y / m_fScale;
  updateModelview();
  queueDraw();
}

unsigned int Clipper_buttons()
{
  return RAD_LBUTTON;
}

void XYWnd::DropClipPoint(int pointx, int pointy)
{
  Vector3 point;

  XY_ToPoint(pointx, pointy, point);

  Vector3 mid;
  Select_GetMid(mid);
  g_clip_viewtype = static_cast<VIEWTYPE>(GetViewType());
  int nDim = (g_clip_viewtype == YZ ) ? nDim = 0 : ( (g_clip_viewtype == XZ) ? nDim = 1 : nDim = 2 );
  point[nDim] = mid[nDim];
  vector3_snap(point, GetGridSize());
  NewClipPoint(point);
}

void XYWnd::Clipper_OnLButtonDown(int x, int y)
{
  Vector3 mousePosition;
  XY_ToPoint(x, y , mousePosition);
  g_pMovingClip = GlobalClipPoints_Find(mousePosition, (VIEWTYPE)m_viewType, m_fScale);
  if(!g_pMovingClip)
  {
    DropClipPoint(x, y);
  }
}

void XYWnd::Clipper_OnLButtonUp(int x, int y)
{
  if (g_pMovingClip)
  {
    g_pMovingClip = 0;
  }
}

void XYWnd::Clipper_OnMouseMoved(int x, int y)
{
  if (g_pMovingClip)
  {
    XY_ToPoint(x, y , g_pMovingClip->m_ptClip);
    XY_SnapToGrid(g_pMovingClip->m_ptClip);
    Clip_Update();
    ClipperChangeNotify();
  }
}

void XYWnd::Clipper_Crosshair_OnMouseMoved(int x, int y)
{
  Vector3 mousePosition;
  XY_ToPoint(x, y , mousePosition);
  if(ClipMode() && GlobalClipPoints_Find(mousePosition, (VIEWTYPE)m_viewType, m_fScale) != 0)
  {
    GdkCursor *cursor;
    cursor = gdk_cursor_new (GDK_CROSSHAIR);
    gdk_window_set_cursor (m_gl_widget->window, cursor);
    gdk_cursor_unref (cursor);
  }
  else
  {
    gdk_window_set_cursor (m_gl_widget->window, 0);
  }
}

unsigned int MoveCamera_buttons()
{
  return RAD_CONTROL | (g_glwindow_globals.m_nMouseType == ETwoButton ? RAD_RBUTTON : RAD_MBUTTON);
}

void XYWnd_PositionCamera(XYWnd* xywnd, int x, int y, CamWnd& camwnd)
{
  Vector3 origin(Camera_getOrigin(camwnd));
  xywnd->XY_ToPoint(x, y, origin);
  xywnd->XY_SnapToGrid(origin);
  Camera_setOrigin(camwnd, origin);
}

unsigned int OrientCamera_buttons()
{
  if(g_glwindow_globals.m_nMouseType == ETwoButton)
    return RAD_RBUTTON | RAD_SHIFT | RAD_CONTROL;
  return RAD_MBUTTON;
}

void XYWnd_OrientCamera(XYWnd* xywnd, int x, int y, CamWnd& camwnd)
{
  Vector3	point = g_vector3_identity;
  xywnd->XY_ToPoint(x, y, point);
  xywnd->XY_SnapToGrid(point);
  vector3_subtract(point, Camera_getOrigin(camwnd));

  int n1 = (xywnd->GetViewType() == XY) ? 1 : 2;
  int n2 = (xywnd->GetViewType() == YZ) ? 1 : 0;
  int nAngle = (xywnd->GetViewType() == XY) ? CAMERA_YAW : CAMERA_PITCH;
  if (point[n1] || point[n2])
  {
    Vector3 angles(Camera_getAngles(camwnd));
    angles[nAngle] = static_cast<float>(radians_to_degrees(atan2 (point[n1], point[n2])));
    Camera_setAngles(camwnd, angles);
  }
}

/*
==============
NewBrushDrag
==============
*/
unsigned int NewBrushDrag_buttons()
{
  return RAD_LBUTTON;
}

void XYWnd::NewBrushDrag_Begin(int x, int y)
{
  m_NewBrushDrag = 0;
  m_nNewBrushPressx = x;
  m_nNewBrushPressy = y;

  m_bNewBrushDrag = true;
  GlobalUndoSystem().start();
}

void XYWnd::NewBrushDrag_End(int x, int y)
{
  if(m_NewBrushDrag != 0)
  {
    GlobalUndoSystem().finish("brushDragNew");
  }
}

void XYWnd::NewBrushDrag(int x, int y)
{
  Vector3	mins, maxs;
  XY_ToPoint(m_nNewBrushPressx, m_nNewBrushPressy, mins);
  XY_SnapToGrid(mins);
	XY_ToPoint(x, y, maxs);
  XY_SnapToGrid(maxs);

  int nDim = (m_viewType == XY) ? 2 : (m_viewType == YZ) ? 0 : 1;

  mins[nDim] = float_snapped(Select_getWorkZone().d_work_min[nDim], GetGridSize());
  maxs[nDim] = float_snapped(Select_getWorkZone().d_work_max[nDim], GetGridSize());

  if (maxs[nDim] <= mins[nDim])
    maxs[nDim] = mins[nDim] + GetGridSize();

  for(int i=0 ; i<3 ; i++)
  {
    if (mins[i] == maxs[i])
      return;	// don't create a degenerate brush
    if (mins[i] > maxs[i])
    {
      float	temp = mins[i];
      mins[i] = maxs[i];
      maxs[i] = temp;
    }
  }

  if(m_NewBrushDrag == 0)
  {
    NodeSmartReference node(GlobalBrushCreator().createBrush());
    Node_getTraversable(Map_FindOrInsertWorldspawn(g_map))->insert(node);

    scene::Path brushpath(makeReference(GlobalSceneGraph().root()));
    brushpath.push(makeReference(*Map_GetWorldspawn(g_map)));
    brushpath.push(makeReference(node.get()));
    selectPath(brushpath, true);

    m_NewBrushDrag = node.get_pointer();
  }

  Scene_BrushResize_Selected(GlobalSceneGraph(), aabb_for_minmax(mins, maxs), TextureBrowser_GetSelectedShader(GlobalTextureBrowser()));
}

void entitycreate_activated(GtkWidget* item)
{
  g_pParentWnd->ActiveXY()->OnEntityCreate(gtk_label_get_text(GTK_LABEL(GTK_BIN(item)->child)));
}

void EntityClassMenu_addItem(GtkMenu* menu, const char* name)
{
  GtkMenuItem* item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(name));
  g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(entitycreate_activated), item);
  gtk_widget_show(GTK_WIDGET(item));
  menu_add_item(menu, item);
}

class EntityClassMenuInserter : public EntityClassVisitor
{
  typedef std::pair<GtkMenu*, CopiedString> MenuPair;
  typedef std::vector<MenuPair> MenuStack;
  MenuStack m_stack;
  CopiedString m_previous;
public:
  EntityClassMenuInserter(GtkMenu* menu)
  {
    m_stack.reserve(2);
    m_stack.push_back(MenuPair(menu, ""));
  }
  ~EntityClassMenuInserter()
  {
    if(!string_empty(m_previous.c_str()))
    {
      addItem(m_previous.c_str(), "");
    }
  }
  void visit(EntityClass* e)
  {
    ASSERT_MESSAGE(!string_empty(e->name()), "entity-class has no name");
    if(!string_empty(m_previous.c_str()))
    {
      addItem(m_previous.c_str(), e->name());
    }
    m_previous = e->name();
  }
  void pushMenu(const CopiedString& name)
  {
    GtkMenuItem* item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(name.c_str()));
    gtk_widget_show(GTK_WIDGET(item));
    container_add_widget(GTK_CONTAINER(m_stack.back().first), GTK_WIDGET(item));

    GtkMenu* submenu = GTK_MENU(gtk_menu_new());
    gtk_menu_item_set_submenu(item, GTK_WIDGET(submenu));

    m_stack.push_back(MenuPair(submenu, name));
  }
  void popMenu()
  {
    m_stack.pop_back();
  }
  void addItem(const char* name, const char* next)
  {
    const char* underscore = strchr(name, '_');

    if(underscore != 0 && underscore != name)
    {
      bool nextEqual = string_equal_n(name, next, (underscore + 1) - name);
      const char* parent = m_stack.back().second.c_str();

      if(!string_empty(parent)
        && string_length(parent) == std::size_t(underscore - name)
        && string_equal_n(name, parent, underscore - name)) // this is a child
      {
      }
      else if(nextEqual)
      {
        if(m_stack.size() == 2)
        {
          popMenu();
        }
        pushMenu(CopiedString(name, underscore));
      }
      else if(m_stack.size() == 2)
      {
        popMenu();
      }
    }
    else if(m_stack.size() == 2)
    {
      popMenu();
    }

    EntityClassMenu_addItem(m_stack.back().first, name);
  }
};

void XYWnd::OnContextMenu()
{
  if (g_xywindow_globals.m_bRightClick == false)
    return;

  if (m_mnuDrop == 0) // first time, load it up
  {
    GtkMenu* menu = m_mnuDrop = GTK_MENU(gtk_menu_new());

    EntityClassMenuInserter inserter(menu);
    GlobalEntityClassManager().forEach(inserter);
  }

  gtk_menu_popup(m_mnuDrop, 0, 0, 0, 0, 1, GDK_CURRENT_TIME);
}

FreezePointer g_xywnd_freezePointer;

unsigned int Move_buttons()
{
  return RAD_RBUTTON;
}

void XYWnd_moveDelta(int x, int y, unsigned int state, void* data)
{
  reinterpret_cast<XYWnd*>(data)->EntityCreate_MouseMove(x, y);
  reinterpret_cast<XYWnd*>(data)->Scroll(-x, y);
}

gboolean XYWnd_Move_focusOut(GtkWidget* widget, GdkEventFocus* event, XYWnd* xywnd)
{
  xywnd->Move_End();
  return FALSE;
}

void XYWnd::Move_Begin()
{
  if(m_move_started)
  {
    Move_End();
  }
  m_move_started = true;
  g_xywnd_freezePointer.freeze_pointer(m_parent != 0 ? m_parent : MainFrame_getWindow(), XYWnd_moveDelta, this);
  m_move_focusOut = g_signal_connect(G_OBJECT(m_gl_widget), "focus_out_event", G_CALLBACK(XYWnd_Move_focusOut), this);
}

void XYWnd::Move_End()
{
  m_move_started = false;
  g_xywnd_freezePointer.unfreeze_pointer(m_parent != 0 ? m_parent : MainFrame_getWindow());
  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_move_focusOut);
}

unsigned int Zoom_buttons()
{
  return RAD_RBUTTON | RAD_SHIFT;
}

int g_dragZoom = 0;

void XYWnd_zoomDelta(int x, int y, unsigned int state, void* data)
{
  if(y != 0)
  {
    g_dragZoom += y;

    while(abs(g_dragZoom) > 8)
    {
      if(g_dragZoom > 0)
      {
        XYWnd_ZoomOut(reinterpret_cast<XYWnd*>(data));
        g_dragZoom -= 8;
      }
      else
      {
        XYWnd_ZoomIn(reinterpret_cast<XYWnd*>(data));
        g_dragZoom += 8;
      }
    }
  }
}

gboolean XYWnd_Zoom_focusOut(GtkWidget* widget, GdkEventFocus* event, XYWnd* xywnd)
{
  xywnd->Zoom_End();
  return FALSE;
}

void XYWnd::Zoom_Begin()
{
  if(m_zoom_started)
  {
    Zoom_End();
  }
  m_zoom_started = true;
  g_dragZoom = 0;
  g_xywnd_freezePointer.freeze_pointer(m_parent != 0 ? m_parent : MainFrame_getWindow(), XYWnd_zoomDelta, this);
  m_zoom_focusOut = g_signal_connect(G_OBJECT(m_gl_widget), "focus_out_event", G_CALLBACK(XYWnd_Zoom_focusOut), this);
}

void XYWnd::Zoom_End()
{
  m_zoom_started = false;
  g_xywnd_freezePointer.unfreeze_pointer(m_parent != 0 ? m_parent : MainFrame_getWindow());
  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_zoom_focusOut);
}

// makes sure the selected brush or camera is in view
void XYWnd::PositionView(const Vector3& position)
{
  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;

  m_vOrigin[nDim1] = position[nDim1];
  m_vOrigin[nDim2] = position[nDim2];

  updateModelview();

  XYWnd_Update(*this);
}

void XYWnd::SetViewType(VIEWTYPE viewType)
{
  m_viewType = viewType; 
  updateModelview();

  if(m_parent != 0)
  {
    gtk_window_set_title(m_parent, ViewType_getTitle(m_viewType));
  }
}


inline WindowVector WindowVector_forInteger(int x, int y)
{
  return WindowVector(static_cast<float>(x), static_cast<float>(y));
}

void XYWnd::XY_MouseDown (int x, int y, unsigned int buttons)
{
  g_pParentWnd->SetActiveXY(this);

  ButtonState_onMouseDown(buttons);

  if(buttons == Move_buttons())
  {
    Move_Begin();
    EntityCreate_MouseDown(x, y);
  }
  else if(buttons == Zoom_buttons())
  {
    Zoom_Begin();
  }
  else if(ClipMode() && buttons == Clipper_buttons())
  {
    Clipper_OnLButtonDown(x, y);
  }
  else if(buttons == NewBrushDrag_buttons() && GlobalSelectionSystem().countSelected() == 0)
  {
    NewBrushDrag_Begin(x, y);
  }
  // control mbutton = move camera
  else if (buttons == MoveCamera_buttons())
  {
    XYWnd_PositionCamera(this, x, y, *g_pParentWnd->GetCamWnd());
  }
  // mbutton = angle camera
  else if(buttons == OrientCamera_buttons())
  {	
    XYWnd_OrientCamera(this, x, y, *g_pParentWnd->GetCamWnd());
  }
  else
  {
    m_window_observer->onMouseDown(WindowVector_forInteger(x, y), button_for_flags(buttons), modifiers_for_flags(buttons));
  }
}

void XYWnd::XY_MouseUp(int x, int y, unsigned int buttons)
{
  if(m_move_started)
  {
    Move_End();
    EntityCreate_MouseUp(x, y);
  }
  else if(m_zoom_started)
  {
    Zoom_End();
  }
  else if (ClipMode() && buttons == Clipper_buttons())
  {
    Clipper_OnLButtonUp(x, y);
  }
  else if (m_bNewBrushDrag)
  {
    m_bNewBrushDrag = false;
    NewBrushDrag_End(x, y);
  }
  else
  {
    m_window_observer->onMouseUp(WindowVector_forInteger(x, y), button_for_flags(buttons), modifiers_for_flags(buttons));
  }

  ButtonState_onMouseUp(buttons);
}

void XYWnd::XY_MouseMoved (int x, int y, unsigned int buttons)
{
  // rbutton = drag xy origin
  if(m_move_started)
  {
  }
  // zoom in/out
  else if(m_zoom_started)
  {
  }

  else if (ClipMode() && g_pMovingClip != 0)
  {
    Clipper_OnMouseMoved(x, y);
  }
  // lbutton without selection = drag new brush
  else if (m_bNewBrushDrag)
  {
    NewBrushDrag(x, y);
  }

  // control mbutton = move camera
  else if (getButtonState() == MoveCamera_buttons())
  {
    XYWnd_PositionCamera(this, x, y, *g_pParentWnd->GetCamWnd());
  }

  // mbutton = angle camera
  else if (getButtonState() == OrientCamera_buttons())
  {	
    XYWnd_OrientCamera(this, x, y, *g_pParentWnd->GetCamWnd());
  }

  else
  {
    m_window_observer->onMouseMotion(WindowVector_forInteger(x, y), modifiers_for_flags(buttons));

    m_mousePosition[0] = m_mousePosition[1] = m_mousePosition[2] = 0.0;
    XY_ToPoint(x, y , m_mousePosition);
    XY_SnapToGrid(m_mousePosition);

    StringOutputStream status(64);
    status << "x:: " << FloatFormat(m_mousePosition[0], 6, 1)
      << "  y:: " << FloatFormat(m_mousePosition[1], 6, 1)
      << "  z:: " << FloatFormat(m_mousePosition[2], 6, 1);
    g_pParentWnd->SetStatusText(g_pParentWnd->m_position_status, status.c_str());

    if (g_bCrossHairs)
    {
      XYWnd_Update(*this);
    }

    Clipper_Crosshair_OnMouseMoved(x, y);
  }
}

void XYWnd::EntityCreate_MouseDown(int x, int y)
{
  m_entityCreate = true;
  m_entityCreate_x = x;
  m_entityCreate_y = y;
}

void XYWnd::EntityCreate_MouseMove(int x, int y)
{
  if(m_entityCreate && (m_entityCreate_x != x || m_entityCreate_y != y))
  {
    m_entityCreate = false;
  }
}

void XYWnd::EntityCreate_MouseUp(int x, int y)
{
  if(m_entityCreate)
  {
    m_entityCreate = false;
    OnContextMenu();
  }
}

inline float screen_normalised(int pos, unsigned int size)
{
  return ((2.0f * pos) / size) - 1.0f;
}

inline float normalised_to_world(float normalised, float world_origin, float normalised2world_scale)
{
  return world_origin + normalised * normalised2world_scale;
}


// TTimo: watch it, this doesn't init one of the 3 coords
void XYWnd::XY_ToPoint (int x, int y, Vector3& point)
{
  float normalised2world_scale_x = m_nWidth / 2 / m_fScale;
  float normalised2world_scale_y = m_nHeight / 2 / m_fScale;
  if (m_viewType == XY)
  {
    point[0] = normalised_to_world(screen_normalised(x, m_nWidth), m_vOrigin[0], normalised2world_scale_x);
    point[1] = normalised_to_world(-screen_normalised(y, m_nHeight), m_vOrigin[1], normalised2world_scale_y);
  }
  else if (m_viewType == YZ)
  {
    point[1] = normalised_to_world(screen_normalised(x, m_nWidth), m_vOrigin[1], normalised2world_scale_x);
    point[2] = normalised_to_world(-screen_normalised(y, m_nHeight), m_vOrigin[2], normalised2world_scale_y);
  }
  else
  {
    point[0] = normalised_to_world(screen_normalised(x, m_nWidth), m_vOrigin[0], normalised2world_scale_x);
    point[2] = normalised_to_world(-screen_normalised(y, m_nHeight), m_vOrigin[2], normalised2world_scale_y);
  }
}

void XYWnd::XY_SnapToGrid(Vector3& point)
{
  if (m_viewType == XY)
  {
    point[0] = float_snapped(point[0], GetGridSize());
    point[1] = float_snapped(point[1], GetGridSize());
  }
  else if (m_viewType == YZ)
  {
    point[1] = float_snapped(point[1], GetGridSize());
    point[2] = float_snapped(point[2], GetGridSize());
  }
  else
  {
    point[0] = float_snapped(point[0], GetGridSize());
    point[2] = float_snapped(point[2], GetGridSize());
  }
}

/*
============================================================================

DRAWING

============================================================================
*/

/*
==============
XY_DrawGrid
==============
*/

double two_to_the_power(int power)
{
  return pow(2.0f, power);
}

void XYWnd::XY_DrawGrid()
{
  float	x, y, xb, xe, yb, ye;
  float		w, h;
  char	text[32];
  float step, minor_step, stepx, stepy;
  step = minor_step = stepx = stepy = GetGridSize();
  
  int minor_power = Grid_getPower();
  int mask;

  while((minor_step * m_fScale) <= 4.0f) // make sure minor grid spacing is at least 4 pixels on the screen
  {
    ++minor_power;
    minor_step *= 2;
  }
  int power = minor_power;
  while((power % 3) != 0 || (step * m_fScale) <= 32.0f) // make sure major grid spacing is at least 32 pixels on the screen
  {
    ++power;
    step = float(two_to_the_power(power));
  }
  mask = (1 << (power - minor_power)) - 1;
  while ((stepx * m_fScale) <= 32.0f) // text step x must be at least 32
    stepx *= 2;
  while ((stepy * m_fScale) <= 32.0f) // text step y must be at least 32
    stepy *= 2;
  

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glLineWidth(1);

  w = (m_nWidth / 2 / m_fScale);
  h = (m_nHeight / 2 / m_fScale);

  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;

  xb = m_vOrigin[nDim1] - w;
  if (xb < region_mins[nDim1])
    xb = region_mins[nDim1];
  xb = step * floor (xb/step);

  xe = m_vOrigin[nDim1] + w;
  if (xe > region_maxs[nDim1])
    xe = region_maxs[nDim1];
  xe = step * ceil (xe/step);

  yb = m_vOrigin[nDim2] - h;
  if (yb < region_mins[nDim2])
    yb = region_mins[nDim2];
  yb = step * floor (yb/step);

  ye = m_vOrigin[nDim2] + h;
  if (ye > region_maxs[nDim2])
    ye = region_maxs[nDim2];
  ye = step * ceil (ye/step);

#define COLORS_DIFFER(a,b) \
  ((a)[0] != (b)[0] || \
   (a)[1] != (b)[1] || \
   (a)[2] != (b)[2])

  // djbob
  // draw minor blocks
  if (g_xywindow_globals_private.d_showgrid)
  {
    if (COLORS_DIFFER(g_xywindow_globals.color_gridminor, g_xywindow_globals.color_gridback))
    {
      glColor3fv(vector3_to_array(g_xywindow_globals.color_gridminor));

      glBegin (GL_LINES);
      int i = 0;
      for (x = xb ; x < xe ; x += minor_step, ++i)
      {
        if((i & mask) != 0)
        {
          glVertex2f (x, yb);
          glVertex2f (x, ye);
        }
      }
      i = 0;
      for (y = yb ; y < ye ; y += minor_step, ++i)
      {
        if((i & mask) != 0)
        {
          glVertex2f (xb, y);
          glVertex2f (xe, y);
        }
      }
      glEnd();
    }

    // draw major blocks
    if (COLORS_DIFFER(g_xywindow_globals.color_gridmajor, g_xywindow_globals.color_gridback))
    {
      glColor3fv(vector3_to_array(g_xywindow_globals.color_gridmajor));

      glBegin (GL_LINES);
      for (x=xb ; x<=xe ; x+=step)
      {
        glVertex2f (x, yb);
        glVertex2f (x, ye);
      }
      for (y=yb ; y<=ye ; y+=step)
      {
        glVertex2f (xb, y);
        glVertex2f (xe, y);
      }
      glEnd();
    }
  }

  // draw coordinate text if needed
  if ( g_xywindow_globals_private.show_coordinates)
  {
    glColor3fv(vector3_to_array(g_xywindow_globals.color_gridtext));
		float offx = m_vOrigin[nDim2] + h - 6 / m_fScale, offy = m_vOrigin[nDim1] - w + 1 / m_fScale;
		for (x = xb - fmod(xb, stepx); x <= xe ; x += stepx)
		{
		  glRasterPos2f (x, offx);
			sprintf (text, "%g", x);
			GlobalOpenGL().drawString(text);
		}
		for (y = yb - fmod(yb, stepy); y <= ye ; y += stepy)
		{
		  glRasterPos2f (offy, y);
			sprintf (text, "%g", y);
			GlobalOpenGL().drawString(text);
		}

    if (Active())
      glColor3fv(vector3_to_array(g_xywindow_globals.color_viewname));

    // we do this part (the old way) only if show_axis is disabled
    if (!g_xywindow_globals_private.show_axis)
    {
      glRasterPos2f ( m_vOrigin[nDim1] - w + 35 / m_fScale, m_vOrigin[nDim2] + h - 20 / m_fScale );

      GlobalOpenGL().drawString(ViewType_getTitle(m_viewType));
    }
  }

  if ( g_xywindow_globals_private.show_axis)
  {
    const char g_AxisName[3] = { 'X', 'Y', 'Z' };

    const Vector3& colourX = (m_viewType == YZ) ? g_xywindow_globals.AxisColorY : g_xywindow_globals.AxisColorX;
    const Vector3& colourY = (m_viewType == XY) ? g_xywindow_globals.AxisColorY : g_xywindow_globals.AxisColorZ;

    // draw two lines with corresponding axis colors to highlight current view
    // horizontal line: nDim1 color
    glLineWidth(2);
    glBegin( GL_LINES );
    glColor3fv (vector3_to_array(colourX));
    glVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
    glVertex2f( m_vOrigin[nDim1] - w + 65 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
    glVertex2f( 0, 0 );
    glVertex2f( 32 / m_fScale, 0 );
    glColor3fv (vector3_to_array(colourY));
    glVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 45 / m_fScale );
    glVertex2f( m_vOrigin[nDim1] - w + 40 / m_fScale, m_vOrigin[nDim2] + h - 20 / m_fScale );
    glVertex2f( 0, 0 );
    glVertex2f( 0, 32 / m_fScale );
    glEnd();
    glLineWidth(1);
    // now print axis symbols
    glColor3fv (vector3_to_array(colourX));
    glRasterPos2f ( m_vOrigin[nDim1] - w + 55 / m_fScale, m_vOrigin[nDim2] + h - 55 / m_fScale );
    GlobalOpenGL().drawChar(g_AxisName[nDim1]);
    glRasterPos2f (28 / m_fScale, -10 / m_fScale );
    GlobalOpenGL().drawChar(g_AxisName[nDim1]);
    glColor3fv (vector3_to_array(colourY));
    glRasterPos2f ( m_vOrigin[nDim1] - w + 25 / m_fScale, m_vOrigin[nDim2] + h - 30 / m_fScale );
    GlobalOpenGL().drawChar(g_AxisName[nDim2]);
    glRasterPos2f ( -10 / m_fScale, 28 / m_fScale );
    GlobalOpenGL().drawChar(g_AxisName[nDim2]);

  }

  // show current work zone?
  // the work zone is used to place dropped points and brushes
  if (g_xywindow_globals_private.d_show_work)
  {
    glColor3f( 1.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
    glVertex2f( xb, Select_getWorkZone().d_work_min[nDim2] );
    glVertex2f( xe, Select_getWorkZone().d_work_min[nDim2] );
    glVertex2f( xb, Select_getWorkZone().d_work_max[nDim2] );
    glVertex2f( xe, Select_getWorkZone().d_work_max[nDim2] );
    glVertex2f( Select_getWorkZone().d_work_min[nDim1], yb );
    glVertex2f( Select_getWorkZone().d_work_min[nDim1], ye );
    glVertex2f( Select_getWorkZone().d_work_max[nDim1], yb );
    glVertex2f( Select_getWorkZone().d_work_max[nDim1], ye );
    glEnd();
  }
}

/*
==============
XY_DrawBlockGrid
==============
*/
void XYWnd::XY_DrawBlockGrid()
{
  if(Map_FindWorldspawn(g_map) == 0)
  {
    return;
  }
  const char *value = Node_getEntity(*Map_GetWorldspawn(g_map))->getKeyValue("_blocksize" );
  if (strlen(value))
	sscanf( value, "%i", &g_xywindow_globals_private.blockSize );

  if (!g_xywindow_globals_private.blockSize || g_xywindow_globals_private.blockSize > 65536 || g_xywindow_globals_private.blockSize < 1024)
	  // don't use custom blocksize if it is less than the default, or greater than the maximum world coordinate
	g_xywindow_globals_private.blockSize = 1024;

  float	x, y, xb, xe, yb, ye;
  float		w, h;
  char	text[32];

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  w = (m_nWidth / 2 / m_fScale);
  h = (m_nHeight / 2 / m_fScale);

  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;

  xb = m_vOrigin[nDim1] - w;
  if (xb < region_mins[nDim1])
    xb = region_mins[nDim1];
  xb = static_cast<float>(g_xywindow_globals_private.blockSize * floor (xb/g_xywindow_globals_private.blockSize));

  xe = m_vOrigin[nDim1] + w;
  if (xe > region_maxs[nDim1])
    xe = region_maxs[nDim1];
  xe = static_cast<float>(g_xywindow_globals_private.blockSize * ceil (xe/g_xywindow_globals_private.blockSize));

  yb = m_vOrigin[nDim2] - h;
  if (yb < region_mins[nDim2])
    yb = region_mins[nDim2];
  yb = static_cast<float>(g_xywindow_globals_private.blockSize * floor (yb/g_xywindow_globals_private.blockSize));

  ye = m_vOrigin[nDim2] + h;
  if (ye > region_maxs[nDim2])
    ye = region_maxs[nDim2];
  ye = static_cast<float>(g_xywindow_globals_private.blockSize * ceil (ye/g_xywindow_globals_private.blockSize));

  // draw major blocks

  glColor3fv(vector3_to_array(g_xywindow_globals.color_gridblock));
  glLineWidth (2);

  glBegin (GL_LINES);
	
  for (x=xb ; x<=xe ; x+=g_xywindow_globals_private.blockSize)
  {
    glVertex2f (x, yb);
    glVertex2f (x, ye);
  }

  if (m_viewType == XY)
  {
	for (y=yb ; y<=ye ; y+=g_xywindow_globals_private.blockSize)
	{
	  glVertex2f (xb, y);
	  glVertex2f (xe, y);
	}
  }
	
  glEnd();
  glLineWidth (1);

  // draw coordinate text if needed

  if (m_viewType == XY && m_fScale > .1)
  {
	for (x=xb ; x<xe ; x+=g_xywindow_globals_private.blockSize)
		for (y=yb ; y<ye ; y+=g_xywindow_globals_private.blockSize)
		{
		  glRasterPos2f (x+(g_xywindow_globals_private.blockSize/2), y+(g_xywindow_globals_private.blockSize/2));
			sprintf (text, "%i,%i",(int)floor(x/g_xywindow_globals_private.blockSize), (int)floor(y/g_xywindow_globals_private.blockSize) );
			GlobalOpenGL().drawString(text);
		}
  }

  glColor4f(0, 0, 0, 0);
}

void XYWnd::DrawCameraIcon(const Vector3& origin, const Vector3& angles)
{
  float	x, y, fov, box;
  double a;

  fov = 48 / m_fScale;
  box = 16 / m_fScale;

  if (m_viewType == XY)
  {
    x = origin[0];
    y = origin[1];
    a = degrees_to_radians(angles[CAMERA_YAW]);
  }
  else if (m_viewType == YZ)
  {
    x = origin[1];
    y = origin[2];
    a = degrees_to_radians(angles[CAMERA_PITCH]);
  }
  else
  {
    x = origin[0];
    y = origin[2];
    a = degrees_to_radians(angles[CAMERA_PITCH]);
  }

  glColor3f (0.0, 0.0, 1.0);
  glBegin(GL_LINE_STRIP);
  glVertex3f (x-box,y,0);
  glVertex3f (x,y+(box/2),0);
  glVertex3f (x+box,y,0);
  glVertex3f (x,y-(box/2),0);
  glVertex3f (x-box,y,0);
  glVertex3f (x+box,y,0);
  glEnd();
	
  glBegin(GL_LINE_STRIP);
  glVertex3f (x + static_cast<float>(fov*cos(a+c_pi/4)), y + static_cast<float>(fov*sin(a+c_pi/4)), 0);
  glVertex3f (x, y, 0);
  glVertex3f (x + static_cast<float>(fov*cos(a-c_pi/4)), y + static_cast<float>(fov*sin(a-c_pi/4)), 0);
  glEnd();

}


float Betwixt(float f1, float f2)
{
  if (f1 > f2)
    return f2 + ((f1 - f2) / 2);
  else
    return f1 + ((f2 - f1) / 2);
}


// can be greatly simplified but per usual i am in a hurry 
// which is not an excuse, just a fact
void XYWnd::PaintSizeInfo(int nDim1, int nDim2, Vector3& vMinBounds, Vector3& vMaxBounds)
{
  if(vector3_equal(vMinBounds, vMaxBounds))
  {
    return;
  }
  const char* g_pDimStrings[] = {"x:", "y:", "z:"};
  typedef const char* OrgStrings[2];
  const OrgStrings g_pOrgStrings[] = { { "x:", "y:", }, { "x:", "z:", }, { "y:", "z:", } };

  Vector3 vSize(vector3_subtracted(vMaxBounds, vMinBounds));

  glColor3f(g_xywindow_globals.color_selbrushes[0] * .65f, 
	     g_xywindow_globals.color_selbrushes[1] * .65f,
	     g_xywindow_globals.color_selbrushes[2] * .65f);

  StringOutputStream dimensions(16);

  if (m_viewType == XY)
  {
    glBegin (GL_LINES);

    glVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale, 0.0f);
    glVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);

    glVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale, 0.0f);
    glVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale, 0.0f);

    glVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale, 0.0f);
    glVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);
  

    glVertex3f(vMaxBounds[nDim1] + 6.0f  / m_fScale, vMinBounds[nDim2], 0.0f);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2], 0.0f);

    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2], 0.0f);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2], 0.0f);
  
    glVertex3f(vMaxBounds[nDim1] + 6.0f  / m_fScale, vMaxBounds[nDim2], 0.0f);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2], 0.0f);

    glEnd();

    glRasterPos3f (Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]),  vMinBounds[nDim2] - 20.0f  / m_fScale, 0.0f);
    dimensions << g_pDimStrings[nDim1] << vSize[nDim1];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();
    
    glRasterPos3f (vMaxBounds[nDim1] + 16.0f  / m_fScale, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]), 0.0f);
    dimensions << g_pDimStrings[nDim2] << vSize[nDim2];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();

    glRasterPos3f (vMinBounds[nDim1] + 4, vMaxBounds[nDim2] + 8 / m_fScale, 0.0f);
    dimensions << "(" << g_pOrgStrings[0][0] << vMinBounds[nDim1] << "  " << g_pOrgStrings[0][1] << vMaxBounds[nDim2] << ")";
    GlobalOpenGL().drawString(dimensions.c_str());
  }
  else if (m_viewType == XZ)
  {
    glBegin (GL_LINES);

    glVertex3f(vMinBounds[nDim1], 0, vMinBounds[nDim2] - 6.0f  / m_fScale);
    glVertex3f(vMinBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale);

    glVertex3f(vMinBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f  / m_fScale);
    glVertex3f(vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f  / m_fScale);

    glVertex3f(vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 6.0f  / m_fScale);
    glVertex3f(vMaxBounds[nDim1], 0,vMinBounds[nDim2] - 10.0f / m_fScale);
  

    glVertex3f(vMaxBounds[nDim1] + 6.0f  / m_fScale, 0,vMinBounds[nDim2]);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMinBounds[nDim2]);

    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMinBounds[nDim2]);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMaxBounds[nDim2]);
  
    glVertex3f(vMaxBounds[nDim1] + 6.0f  / m_fScale, 0,vMaxBounds[nDim2]);
    glVertex3f(vMaxBounds[nDim1] + 10.0f  / m_fScale, 0,vMaxBounds[nDim2]);

    glEnd();

    glRasterPos3f (Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]), 0, vMinBounds[nDim2] - 20.0f  / m_fScale);
    dimensions << g_pDimStrings[nDim1] << vSize[nDim1];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();
    
    glRasterPos3f (vMaxBounds[nDim1] + 16.0f  / m_fScale, 0, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]));
    dimensions << g_pDimStrings[nDim2] << vSize[nDim2];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();

    glRasterPos3f (vMinBounds[nDim1] + 4, 0, vMaxBounds[nDim2] + 8 / m_fScale);
    dimensions << "(" << g_pOrgStrings[1][0] << vMinBounds[nDim1] << "  " << g_pOrgStrings[1][1] << vMaxBounds[nDim2] << ")";
    GlobalOpenGL().drawString(dimensions.c_str());
  }
  else
  {
    glBegin (GL_LINES);

    glVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale);
    glVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);

    glVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale);
    glVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f  / m_fScale);

    glVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f  / m_fScale);
    glVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);
  

    glVertex3f(0, vMaxBounds[nDim1] + 6.0f  / m_fScale, vMinBounds[nDim2]);
    glVertex3f(0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2]);

    glVertex3f(0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMinBounds[nDim2]);
    glVertex3f(0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2]);
  
    glVertex3f(0, vMaxBounds[nDim1] + 6.0f  / m_fScale, vMaxBounds[nDim2]);
    glVertex3f(0, vMaxBounds[nDim1] + 10.0f  / m_fScale, vMaxBounds[nDim2]);

    glEnd();

    glRasterPos3f (0, Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]),  vMinBounds[nDim2] - 20.0f  / m_fScale);
    dimensions << g_pDimStrings[nDim1] << vSize[nDim1];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();
    
    glRasterPos3f (0, vMaxBounds[nDim1] + 16.0f  / m_fScale, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]));
    dimensions << g_pDimStrings[nDim2] << vSize[nDim2];
    GlobalOpenGL().drawString(dimensions.c_str());
    dimensions.clear();

    glRasterPos3f (0, vMinBounds[nDim1] + 4.0f, vMaxBounds[nDim2] + 8 / m_fScale);
    dimensions << "(" << g_pOrgStrings[2][0] << vMinBounds[nDim1] << "  " << g_pOrgStrings[2][1] << vMaxBounds[nDim2] << ")";
    GlobalOpenGL().drawString(dimensions.c_str());
  }
}

class XYRenderer: public Renderer
{
  struct state_type
  {
    state_type() :
    m_highlight(0),
    m_state(0)
    {
    }  
    unsigned int m_highlight;
    Shader* m_state;
  };
public:
  XYRenderer(RenderStateFlags globalstate, Shader* selected) :
    m_globalstate(globalstate),
    m_state_selected(selected)
  {
    ASSERT_NOTNULL(selected);
    m_state_stack.push_back(state_type());
  }

  void SetState(Shader* state, EStyle style)
  {
    ASSERT_NOTNULL(state);
    if(style == eWireframeOnly)
      m_state_stack.back().m_state = state;
  }
  const EStyle getStyle() const
  {
    return eWireframeOnly;
  }
  void PushState()
  {
    m_state_stack.push_back(m_state_stack.back());
  }
  void PopState()
  {
    ASSERT_MESSAGE(!m_state_stack.empty(), "popping empty stack");
    m_state_stack.pop_back();
  }
  void Highlight(EHighlightMode mode, bool bEnable = true)
  {
    (bEnable)
      ? m_state_stack.back().m_highlight |= mode
      : m_state_stack.back().m_highlight &= ~mode;
  }
  void addRenderable(const OpenGLRenderable& renderable, const Matrix4& localToWorld)
  {
    if(m_state_stack.back().m_highlight & ePrimitive)
    {
      m_state_selected->addRenderable(renderable, localToWorld);
    }
    else
    {
      m_state_stack.back().m_state->addRenderable(renderable, localToWorld);
    }
  }

  void render(const Matrix4& modelview, const Matrix4& projection)
  {
    GlobalShaderCache().render(m_globalstate, modelview, projection);
  }
private:
  std::vector<state_type> m_state_stack;
  RenderStateFlags m_globalstate;
  Shader* m_state_selected;
};

void XYWnd::updateProjection()
{
  m_projection[0] = 1.0f / static_cast<float>(m_nWidth / 2);
  m_projection[5] = 1.0f / static_cast<float>(m_nHeight / 2);
  m_projection[10] = 1.0f / g_MaxWorldCoord;

  m_projection[12] = 0.0f;
  m_projection[13] = 0.0f;
  m_projection[14] = -1.0f;

  m_projection[1] =
  m_projection[2] =
  m_projection[3] =

  m_projection[4] =
  m_projection[6] =
  m_projection[7] =

  m_projection[8] =
  m_projection[9] =
  m_projection[11] = 0.0f;

  m_projection[15] = 1.0f;

  m_view.Construct(m_projection, m_modelview, m_nWidth, m_nHeight);
}

void XYWnd::updateModelview()
{
  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;

  // translation
  m_modelview[12] = -m_vOrigin[nDim1] * m_fScale;
  m_modelview[13] = -m_vOrigin[nDim2] * m_fScale;
  m_modelview[14] = static_cast<float>(g_MaxWorldCoord);

  // axis base
  switch(m_viewType)
  {
  case XY:
    m_modelview[0]  =  m_fScale;
    m_modelview[1]  =  0;
    m_modelview[2]  =  0;

    m_modelview[4]  =  0;
    m_modelview[5]  =  m_fScale;
    m_modelview[6]  =  0;

    m_modelview[8]  =  0;
    m_modelview[9]  =  0;
    m_modelview[10] = -m_fScale;
    break;
  case XZ:
    m_modelview[0]  =  m_fScale;
    m_modelview[1]  =  0;
    m_modelview[2]  =  0;

    m_modelview[4]  =  0;
    m_modelview[5]  =  0;
    m_modelview[6]  =  m_fScale;

    m_modelview[8]  =  0;
    m_modelview[9]  =  m_fScale;
    m_modelview[10] =  0;
    break;
  case YZ:
    m_modelview[0]  =  0;
    m_modelview[1]  =  0;
    m_modelview[2]  = -m_fScale;

    m_modelview[4]  =  m_fScale;
    m_modelview[5]  =  0;
    m_modelview[6]  =  0;

    m_modelview[8]  =  0;
    m_modelview[9]  =  m_fScale;
    m_modelview[10] =  0;
    break;
  }

  m_modelview[3] = m_modelview[7] = m_modelview[11] = 0;
  m_modelview[15] = 1;

  m_view.Construct(m_projection, m_modelview, m_nWidth, m_nHeight);
}

/*
==============
XY_Draw
==============
*/

//#define DBG_SCENEDUMP

void XYWnd::XY_Draw()
{
  //
  // clear
  //
  glViewport(0, 0, m_nWidth, m_nHeight);
  glClearColor (g_xywindow_globals.color_gridback[0],
		 g_xywindow_globals.color_gridback[1],
		 g_xywindow_globals.color_gridback[2],0);

  glClear(GL_COLOR_BUFFER_BIT);

  //
  // set up viewpoint
  //

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(reinterpret_cast<const float*>(&m_projection));

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(m_fScale, m_fScale, 1);
  int nDim1 = (m_viewType == YZ) ? 1 : 0;
  int nDim2 = (m_viewType == XY) ? 1 : 2;
  glTranslatef(-m_vOrigin[nDim1], -m_vOrigin[nDim2], 0);

  glDisable (GL_LINE_STIPPLE);
  glLineWidth(1);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_DEPTH_TEST);

  XY_DrawGrid();
  if ( g_xywindow_globals_private.show_blocks)
    XY_DrawBlockGrid();

  glLoadMatrixf(reinterpret_cast<const float*>(&m_modelview));

  unsigned int globalstate = RENDER_COLOUR | RENDER_COLOURWRITE;
  if(!g_xywindow_globals.m_bNoStipple)
  {
    globalstate |= RENDER_LINESTIPPLE;
  }

  {
    XYRenderer renderer(globalstate, m_state_selected);

    Scene_Render(renderer, m_view);

    GlobalOpenGL_debugAssertNoErrors();
    renderer.render(m_modelview, m_projection);
    GlobalOpenGL_debugAssertNoErrors();
  }

  glDepthMask(GL_FALSE);

  GlobalOpenGL_debugAssertNoErrors();

  glLoadMatrixf(reinterpret_cast<const float*>(&m_modelview));
  
  GlobalOpenGL_debugAssertNoErrors();
  glDisable(GL_LINE_STIPPLE);
  GlobalOpenGL_debugAssertNoErrors();
  glLineWidth(1);
  GlobalOpenGL_debugAssertNoErrors();
  if(GlobalOpenGL().GL_1_3())
  {
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
  }
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  GlobalOpenGL_debugAssertNoErrors();
  glDisableClientState(GL_NORMAL_ARRAY);
  GlobalOpenGL_debugAssertNoErrors();
  glDisableClientState(GL_COLOR_ARRAY);
  GlobalOpenGL_debugAssertNoErrors();
  glDisable(GL_TEXTURE_2D);
  GlobalOpenGL_debugAssertNoErrors();
  glDisable(GL_LIGHTING);
  GlobalOpenGL_debugAssertNoErrors();
  glDisable(GL_COLOR_MATERIAL);
  GlobalOpenGL_debugAssertNoErrors();

  GlobalOpenGL_debugAssertNoErrors();


  // size info
  if(g_xywindow_globals_private.m_bSizePaint && GlobalSelectionSystem().countSelected() != 0)
  {
    Vector3 min, max;
    Select_GetBounds(min, max);
    PaintSizeInfo(nDim1, nDim2, min, max);
  }

  if (g_bCrossHairs)
  {
    glColor4f(0.2f, 0.9f, 0.2f, 0.8f);
    glBegin (GL_LINES);
    if (m_viewType == XY)
    {
      glVertex2f(2.0f * g_MinWorldCoord, m_mousePosition[1]);
      glVertex2f(2.0f * g_MaxWorldCoord, m_mousePosition[1]);
      glVertex2f(m_mousePosition[0], 2.0f * g_MinWorldCoord);
      glVertex2f(m_mousePosition[0], 2.0f * g_MaxWorldCoord);
    }
    else if (m_viewType == YZ)
    {
      glVertex3f(m_mousePosition[0], 2.0f * g_MinWorldCoord, m_mousePosition[2]);
      glVertex3f(m_mousePosition[0], 2.0f * g_MaxWorldCoord, m_mousePosition[2]);
      glVertex3f(m_mousePosition[0], m_mousePosition[1], 2.0f * g_MinWorldCoord);
      glVertex3f(m_mousePosition[0], m_mousePosition[1], 2.0f * g_MaxWorldCoord);
    }
    else
    {
      glVertex3f (2.0f * g_MinWorldCoord, m_mousePosition[1], m_mousePosition[2]);
      glVertex3f (2.0f * g_MaxWorldCoord, m_mousePosition[1], m_mousePosition[2]);
      glVertex3f(m_mousePosition[0], m_mousePosition[1], 2.0f * g_MinWorldCoord);
      glVertex3f(m_mousePosition[0], m_mousePosition[1], 2.0f * g_MaxWorldCoord);
    }
    glEnd();
  }

  if (ClipMode())
  {
    GlobalClipPoints_Draw(m_fScale);
  }

  GlobalOpenGL_debugAssertNoErrors();
    
    // reset modelview
  glLoadIdentity();
  glScalef(m_fScale, m_fScale, 1);
  glTranslatef(-m_vOrigin[nDim1], -m_vOrigin[nDim2], 0);

  DrawCameraIcon (Camera_getOrigin(*g_pParentWnd->GetCamWnd()), Camera_getAngles(*g_pParentWnd->GetCamWnd()));

  Feedback_draw2D( m_viewType );

  if (g_xywindow_globals_private.show_outline)
  {
    if (Active())
    {
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity();
      glOrtho (0, m_nWidth, 0, m_nHeight, 0, 1);

      glMatrixMode (GL_MODELVIEW);
      glLoadIdentity();

      // four view mode doesn't colorize
      if (g_pParentWnd->CurrentStyle() == MainFrame::eSplit)
        glColor3fv(vector3_to_array(g_xywindow_globals.color_viewname));
      else
      {
        switch(m_viewType)
        {
        case YZ:
          glColor3fv(vector3_to_array(g_xywindow_globals.AxisColorX));
          break;
        case XZ:
          glColor3fv(vector3_to_array(g_xywindow_globals.AxisColorY));
          break;
        case XY:
          glColor3fv(vector3_to_array(g_xywindow_globals.AxisColorZ));
          break;
        }
      }
      glBegin (GL_LINE_LOOP);
      glVertex2i (0, 0);
      glVertex2i (m_nWidth-1, 0);
      glVertex2i (m_nWidth-1, m_nHeight-1);
      glVertex2i (0, m_nHeight-1);
      glEnd();
    }
  }

  GlobalOpenGL_debugAssertNoErrors();

  glFinish();
}

void XYWnd_MouseToPoint(XYWnd* xywnd, int x, int y, Vector3& point)
{
  xywnd->XY_ToPoint(x, y, point);
  xywnd->XY_SnapToGrid(point);

  int nDim = (xywnd->GetViewType() == XY) ? 2 : (xywnd->GetViewType() == YZ) ? 0 : 1;
  float fWorkMid = float_mid(Select_getWorkZone().d_work_min[nDim], Select_getWorkZone().d_work_max[nDim]);
  point[nDim] = float_snapped(fWorkMid, GetGridSize());
}

void XYWnd::OnEntityCreate (const char* item)
{
  StringOutputStream command;
  command << "entityCreate -class " << item;
  UndoableCommand undo(command.c_str());
  Vector3 point;
  XYWnd_MouseToPoint(this, m_entityCreate_x, m_entityCreate_y, point);
  Entity_createFromSelection(item, point);
}



void GetFocusPosition(Vector3& position)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    Select_GetMid(position);
  }
  else
  {
    position = Camera_getOrigin(*g_pParentWnd->GetCamWnd());
  }
}

void XYWnd_Focus(XYWnd* xywnd)
{
  Vector3 position;
  GetFocusPosition(position);
  xywnd->PositionView(position);
}

void XY_Split_Focus()
{
  Vector3 position;
  GetFocusPosition(position);
  g_pParentWnd->GetXYWnd()->PositionView(position);
  g_pParentWnd->GetXZWnd()->PositionView(position);
  g_pParentWnd->GetYZWnd()->PositionView(position);
}

void XY_Focus()
{
  XYWnd* xywnd = g_pParentWnd->GetXYWnd();
  XYWnd_Focus(xywnd);
}

void XY_Top()
{
  XYWnd* xywnd = g_pParentWnd->GetXYWnd();
  xywnd->SetViewType(XY);
  XYWnd_Focus(xywnd);
}

void XY_Side()
{
  XYWnd* xywnd = g_pParentWnd->GetXYWnd();
  xywnd->SetViewType(XZ);
  XYWnd_Focus(xywnd);
}

void XY_Front()
{
  g_pParentWnd->GetXYWnd()->SetViewType(YZ);
  XYWnd_Focus(g_pParentWnd->GetXYWnd());
}

void XY_Next()
{
  XYWnd* xywnd = g_pParentWnd->GetXYWnd();
  if (xywnd->GetViewType() == XY)
    xywnd->SetViewType(XZ);
  else if (xywnd->GetViewType() ==  XZ)
    xywnd->SetViewType(YZ);
  else
    xywnd->SetViewType(XY);
  XYWnd_Focus(xywnd);
}

void XY_Zoom100()
{
  if (g_pParentWnd->GetXYWnd())
    g_pParentWnd->GetXYWnd()->SetScale(1);
  if (g_pParentWnd->GetXZWnd())
    g_pParentWnd->GetXZWnd()->SetScale(1);
  if (g_pParentWnd->GetYZWnd())
    g_pParentWnd->GetYZWnd()->SetScale(1);
}

void XY_ZoomIn()
{
  XYWnd_ZoomIn(g_pParentWnd->ActiveXY());
}

// NOTE: the zoom out factor is 4/5, we could think about customizing it
//  we don't go below a zoom factor corresponding to 10% of the max world size
//  (this has to be computed against the window size)
void XY_ZoomOut()
{
  XYWnd_ZoomOut(g_pParentWnd->ActiveXY());
}



void ToggleShowCrosshair()
{
  g_bCrossHairs ^= 1; 
  XY_UpdateAllWindows();
}

void ToggleShowSizeInfo()
{
  g_xywindow_globals_private.m_bSizePaint = !g_xywindow_globals_private.m_bSizePaint;
  XY_UpdateAllWindows();
}

void ToggleShowGrid()
{
  g_xywindow_globals_private.d_showgrid = !g_xywindow_globals_private.d_showgrid;
  XY_UpdateAllWindows();
}

ToggleShown g_xy_top_shown(true);

void XY_Top_Shown_Construct(GtkWindow* parent)
{
  g_xy_top_shown.connect(GTK_WIDGET(parent));
}

ToggleShown g_yz_side_shown(false);

void YZ_Side_Shown_Construct(GtkWindow* parent)
{
  g_yz_side_shown.connect(GTK_WIDGET(parent));
}

ToggleShown g_xz_front_shown(false);

void XZ_Front_Shown_Construct(GtkWindow* parent)
{
  g_xz_front_shown.connect(GTK_WIDGET(parent));
}


class EntityClassMenu : public ModuleObserver
{
  std::size_t m_unrealised;
public:
  EntityClassMenu() : m_unrealised(1)
  {
  }
  void realise()
  {
    if(--m_unrealised == 0)
    {
    }
  }
  void unrealise()
  {
    if(++m_unrealised == 1)
    {
      if(XYWnd::m_mnuDrop != 0)
      {
        gtk_widget_destroy(GTK_WIDGET(XYWnd::m_mnuDrop));
        XYWnd::m_mnuDrop = 0;
      }
    }
  }
};

EntityClassMenu g_EntityClassMenu;




void ShowNamesToggle()
{
  GlobalEntityCreator().setShowNames(!GlobalEntityCreator().getShowNames());
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowNamesToggle> ShowNamesToggleCaller;
void ShowNamesExport(const BoolImportCallback& importer)
{
  importer(GlobalEntityCreator().getShowNames());
}
typedef FreeCaller1<const BoolImportCallback&, ShowNamesExport> ShowNamesExportCaller;

void ShowAnglesToggle()
{
  GlobalEntityCreator().setShowAngles(!GlobalEntityCreator().getShowAngles());
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowAnglesToggle> ShowAnglesToggleCaller;
void ShowAnglesExport(const BoolImportCallback& importer)
{
  importer(GlobalEntityCreator().getShowAngles());
}
typedef FreeCaller1<const BoolImportCallback&, ShowAnglesExport> ShowAnglesExportCaller;

void ShowBlocksToggle()
{
  g_xywindow_globals_private.show_blocks ^= 1;
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowBlocksToggle> ShowBlocksToggleCaller;
void ShowBlocksExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_blocks);
}
typedef FreeCaller1<const BoolImportCallback&, ShowBlocksExport> ShowBlocksExportCaller;

void ShowCoordinatesToggle()
{
  g_xywindow_globals_private.show_coordinates ^= 1;
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowCoordinatesToggle> ShowCoordinatesToggleCaller;
void ShowCoordinatesExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_coordinates);
}
typedef FreeCaller1<const BoolImportCallback&, ShowCoordinatesExport> ShowCoordinatesExportCaller;

void ShowOutlineToggle()
{
  g_xywindow_globals_private.show_outline ^= 1;
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowOutlineToggle> ShowOutlineToggleCaller;
void ShowOutlineExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_outline);
}
typedef FreeCaller1<const BoolImportCallback&, ShowOutlineExport> ShowOutlineExportCaller;

void ShowAxesToggle()
{
  g_xywindow_globals_private.show_axis ^= 1;
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowAxesToggle> ShowAxesToggleCaller;
void ShowAxesExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_axis);
}
typedef FreeCaller1<const BoolImportCallback&, ShowAxesExport> ShowAxesExportCaller;

void ShowWorkzoneToggle()
{
  g_xywindow_globals_private.d_show_work ^= 1;
  XY_UpdateAllWindows();
}
typedef FreeCaller<ShowWorkzoneToggle> ShowWorkzoneToggleCaller;
void ShowWorkzoneExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.d_show_work);
}
typedef FreeCaller1<const BoolImportCallback&, ShowWorkzoneExport> ShowWorkzoneExportCaller;

ShowNamesExportCaller g_show_names_caller;
BoolExportCallback g_show_names_callback(g_show_names_caller);
ToggleItem g_show_names(g_show_names_callback);

ShowAnglesExportCaller g_show_angles_caller;
BoolExportCallback g_show_angles_callback(g_show_angles_caller);
ToggleItem g_show_angles(g_show_angles_callback);

ShowBlocksExportCaller g_show_blocks_caller;
BoolExportCallback g_show_blocks_callback(g_show_blocks_caller);
ToggleItem g_show_blocks(g_show_blocks_callback);

ShowCoordinatesExportCaller g_show_coordinates_caller;
BoolExportCallback g_show_coordinates_callback(g_show_coordinates_caller);
ToggleItem g_show_coordinates(g_show_coordinates_callback);

ShowOutlineExportCaller g_show_outline_caller;
BoolExportCallback g_show_outline_callback(g_show_outline_caller);
ToggleItem g_show_outline(g_show_outline_callback);

ShowAxesExportCaller g_show_axes_caller;
BoolExportCallback g_show_axes_callback(g_show_axes_caller);
ToggleItem g_show_axes(g_show_axes_callback);

ShowWorkzoneExportCaller g_show_workzone_caller;
BoolExportCallback g_show_workzone_callback(g_show_workzone_caller);
ToggleItem g_show_workzone(g_show_workzone_callback);

void XYShow_registerCommands()
{
  GlobalToggles_insert("ShowAngles", ShowAnglesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_angles));
  GlobalToggles_insert("ShowNames", ShowNamesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_names));
  GlobalToggles_insert("ShowBlocks", ShowBlocksToggleCaller(), ToggleItem::AddCallbackCaller(g_show_blocks));
  GlobalToggles_insert("ShowCoordinates", ShowCoordinatesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_coordinates));
  GlobalToggles_insert("ShowWindowOutline", ShowOutlineToggleCaller(), ToggleItem::AddCallbackCaller(g_show_outline));
  GlobalToggles_insert("ShowAxes", ShowAxesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_axes));
  GlobalToggles_insert("ShowWorkzone", ShowWorkzoneToggleCaller(), ToggleItem::AddCallbackCaller(g_show_workzone));
}

void XYWnd_registerShortcuts()
{
  command_connect_accelerator("ToggleCrosshairs");
  command_connect_accelerator("ToggleSizePaint");
}



void Orthographic_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox("", "Solid selection boxes", g_xywindow_globals.m_bNoStipple);
  page.appendCheckBox("", "Display size info", g_xywindow_globals_private.m_bSizePaint);
  page.appendCheckBox("", "Chase mouse during drags", g_xywindow_globals_private.m_bChaseMouse);
  page.appendCheckBox("", "Update views on camera move", g_xywindow_globals_private.m_bCamXYUpdate);
}
void Orthographic_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Orthographic", "Orthographic View Preferences"));
  Orthographic_constructPreferences(page);
}
void Orthographic_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Orthographic_constructPage>());
}

void Clipper_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox("", "Clipper tool uses caulk", g_clip_useCaulk);
}
void Clipper_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Clipper", "Clipper Tool Settings"));
  Clipper_constructPreferences(page);
}
void Clipper_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Clipper_constructPage>());
}


#include "preferencesystem.h"
#include "stringio.h"




void ToggleShown_importBool(ToggleShown& self, bool value)
{
  self.set(value);
}
typedef ReferenceCaller1<ToggleShown, bool, ToggleShown_importBool> ToggleShownImportBoolCaller;
void ToggleShown_exportBool(const ToggleShown& self, const BoolImportCallback& importer)
{
  importer(self.active());
}
typedef ConstReferenceCaller1<ToggleShown, const BoolImportCallback&, ToggleShown_exportBool> ToggleShownExportBoolCaller;


void XYWindow_Construct()
{
  GlobalCommands_insert("ToggleCrosshairs", FreeCaller<ToggleShowCrosshair>(), Accelerator('X', (GdkModifierType)GDK_SHIFT_MASK));
  GlobalCommands_insert("ToggleSizePaint", FreeCaller<ToggleShowSizeInfo>(), Accelerator('J'));
  GlobalCommands_insert("ToggleGrid", FreeCaller<ToggleShowGrid>(), Accelerator('0'));

  GlobalToggles_insert("ToggleView", ToggleShown::ToggleCaller(g_xy_top_shown), ToggleItem::AddCallbackCaller(g_xy_top_shown.m_item), Accelerator('V', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalToggles_insert("ToggleSideView", ToggleShown::ToggleCaller(g_yz_side_shown), ToggleItem::AddCallbackCaller(g_yz_side_shown.m_item));
  GlobalToggles_insert("ToggleFrontView", ToggleShown::ToggleCaller(g_xz_front_shown), ToggleItem::AddCallbackCaller(g_xz_front_shown.m_item));
  GlobalCommands_insert("NextView", FreeCaller<XY_Next>(), Accelerator(GDK_Tab, (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("ZoomIn", FreeCaller<XY_ZoomIn>(), Accelerator(GDK_Delete));
  GlobalCommands_insert("ZoomOut", FreeCaller<XY_ZoomOut>(), Accelerator(GDK_Insert));
  GlobalCommands_insert("ViewTop", FreeCaller<XY_Top>());
  GlobalCommands_insert("ViewSide", FreeCaller<XY_Side>());
  GlobalCommands_insert("ViewFront", FreeCaller<XY_Front>());
  GlobalCommands_insert("Zoom100", FreeCaller<XY_Zoom100>());
  GlobalCommands_insert("CenterXYViews", FreeCaller<XY_Split_Focus>(), Accelerator(GDK_Tab, (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalCommands_insert("CenterXYView", FreeCaller<XY_Focus>(), Accelerator(GDK_Tab, (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));

  GlobalPreferenceSystem().registerPreference("ClipCaulk", BoolImportStringCaller(g_clip_useCaulk), BoolExportStringCaller(g_clip_useCaulk));

  GlobalPreferenceSystem().registerPreference("NewRightClick", BoolImportStringCaller(g_xywindow_globals.m_bRightClick), BoolExportStringCaller(g_xywindow_globals.m_bRightClick));
  GlobalPreferenceSystem().registerPreference("ChaseMouse", BoolImportStringCaller(g_xywindow_globals_private.m_bChaseMouse), BoolExportStringCaller(g_xywindow_globals_private.m_bChaseMouse));
  GlobalPreferenceSystem().registerPreference("SizePainting", BoolImportStringCaller(g_xywindow_globals_private.m_bSizePaint), BoolExportStringCaller(g_xywindow_globals_private.m_bSizePaint));
  GlobalPreferenceSystem().registerPreference("NoStipple", BoolImportStringCaller(g_xywindow_globals.m_bNoStipple), BoolExportStringCaller(g_xywindow_globals.m_bNoStipple));
  GlobalPreferenceSystem().registerPreference("SI_ShowCoords", BoolImportStringCaller(g_xywindow_globals_private.show_coordinates), BoolExportStringCaller(g_xywindow_globals_private.show_coordinates));
  GlobalPreferenceSystem().registerPreference("SI_ShowOutlines", BoolImportStringCaller(g_xywindow_globals_private.show_outline), BoolExportStringCaller(g_xywindow_globals_private.show_outline));
  GlobalPreferenceSystem().registerPreference("SI_ShowAxis", BoolImportStringCaller(g_xywindow_globals_private.show_axis), BoolExportStringCaller(g_xywindow_globals_private.show_axis));
  GlobalPreferenceSystem().registerPreference("CamXYUpdate", BoolImportStringCaller(g_xywindow_globals_private.m_bCamXYUpdate), BoolExportStringCaller(g_xywindow_globals_private.m_bCamXYUpdate));

  GlobalPreferenceSystem().registerPreference("SI_AxisColors0", Vector3ImportStringCaller(g_xywindow_globals.AxisColorX), Vector3ExportStringCaller(g_xywindow_globals.AxisColorX));
  GlobalPreferenceSystem().registerPreference("SI_AxisColors1", Vector3ImportStringCaller(g_xywindow_globals.AxisColorY), Vector3ExportStringCaller(g_xywindow_globals.AxisColorY));
  GlobalPreferenceSystem().registerPreference("SI_AxisColors2", Vector3ImportStringCaller(g_xywindow_globals.AxisColorZ), Vector3ExportStringCaller(g_xywindow_globals.AxisColorZ));
  GlobalPreferenceSystem().registerPreference("SI_Colors1", Vector3ImportStringCaller(g_xywindow_globals.color_gridback), Vector3ExportStringCaller(g_xywindow_globals.color_gridback));
  GlobalPreferenceSystem().registerPreference("SI_Colors2", Vector3ImportStringCaller(g_xywindow_globals.color_gridminor), Vector3ExportStringCaller(g_xywindow_globals.color_gridminor));
  GlobalPreferenceSystem().registerPreference("SI_Colors3", Vector3ImportStringCaller(g_xywindow_globals.color_gridmajor), Vector3ExportStringCaller(g_xywindow_globals.color_gridmajor));
  GlobalPreferenceSystem().registerPreference("SI_Colors6", Vector3ImportStringCaller(g_xywindow_globals.color_gridblock), Vector3ExportStringCaller(g_xywindow_globals.color_gridblock));
  GlobalPreferenceSystem().registerPreference("SI_Colors7", Vector3ImportStringCaller(g_xywindow_globals.color_gridtext), Vector3ExportStringCaller(g_xywindow_globals.color_gridtext));
  GlobalPreferenceSystem().registerPreference("SI_Colors8", Vector3ImportStringCaller(g_xywindow_globals.color_brushes), Vector3ExportStringCaller(g_xywindow_globals.color_brushes));
  GlobalPreferenceSystem().registerPreference("SI_Colors9", Vector3ImportStringCaller(g_xywindow_globals.color_selbrushes), Vector3ExportStringCaller(g_xywindow_globals.color_selbrushes));
  GlobalPreferenceSystem().registerPreference("SI_Colors10", Vector3ImportStringCaller(g_xywindow_globals.color_clipper), Vector3ExportStringCaller(g_xywindow_globals.color_clipper));
  GlobalPreferenceSystem().registerPreference("SI_Colors11", Vector3ImportStringCaller(g_xywindow_globals.color_viewname), Vector3ExportStringCaller(g_xywindow_globals.color_viewname));
  GlobalPreferenceSystem().registerPreference("SI_Colors13", Vector3ImportStringCaller(g_xywindow_globals.color_gridminor_alt), Vector3ExportStringCaller(g_xywindow_globals.color_gridminor_alt));
  GlobalPreferenceSystem().registerPreference("SI_Colors14", Vector3ImportStringCaller(g_xywindow_globals.color_gridmajor_alt), Vector3ExportStringCaller(g_xywindow_globals.color_gridmajor_alt));


  GlobalPreferenceSystem().registerPreference("XZVIS", makeBoolStringImportCallback(ToggleShownImportBoolCaller(g_xz_front_shown)), makeBoolStringExportCallback(ToggleShownExportBoolCaller(g_xz_front_shown)));
  GlobalPreferenceSystem().registerPreference("YZVIS", makeBoolStringImportCallback(ToggleShownImportBoolCaller(g_yz_side_shown)), makeBoolStringExportCallback(ToggleShownExportBoolCaller(g_yz_side_shown)));

  Orthographic_registerPreferencesPage();
  Clipper_registerPreferencesPage();

  XYWnd::captureStates();
  GlobalEntityClassManager().attach(g_EntityClassMenu);
}

void XYWindow_Destroy()
{
  GlobalEntityClassManager().detach(g_EntityClassMenu);
  XYWnd::releaseStates();
}
