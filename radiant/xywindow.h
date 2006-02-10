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

#if !defined(INCLUDED_XYWINDOW_H)
#define INCLUDED_XYWINDOW_H

#include "math/matrix.h"

#include "gtkutil/cursor.h"
#include "gtkutil/window.h"
#include "gtkutil/xorrectangle.h"
#include "view.h"
#include "map.h"

class Shader;
class SelectionSystemWindowObserver;
namespace scene
{
  class Node;
}
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkMenu GtkMenu;


void FlipClip();
void SplitClip();
void Clip();
void OnClipMode(bool enabled);
bool ClipMode();

enum VIEWTYPE
{
  YZ = 0,
  XZ = 1,
  XY = 2
};

inline const char* ViewType_getTitle(VIEWTYPE viewtype)
{
  if(viewtype == XY)
  {
    return "XY Top";
  }
  if(viewtype == XZ)
  {
    return "XZ Front";
  }
  if(viewtype == YZ)
  {
    return "YZ Side";
  }
  return "";
}

class XYWnd
{
  GtkWidget* m_gl_widget;
  guint m_sizeHandler;
  guint m_exposeHandler;

  DeferredDraw m_deferredDraw;
  DeferredMotion m_deferred_motion;
public:
  GtkWindow* m_parent;
  XYWnd();
  ~XYWnd();

  void queueDraw()
  {
    m_deferredDraw.draw();
  }
  GtkWidget* GetWidget()
  {
    return m_gl_widget;
  }

public:
  SelectionSystemWindowObserver* m_window_observer;
  XORRectangle m_XORRectangle;
  WindowPositionTracker m_positionTracker;

  static void captureStates();
  static void releaseStates();

  void PositionView(const Vector3& position);
  const Vector3& GetOrigin();
  void SetOrigin(const Vector3& origin);
  void Scroll(int x, int y);

  void XY_Draw();
  void DrawCameraIcon(const Vector3& origin, const Vector3& angles);
  void XY_DrawBlockGrid();
  void XY_DrawGrid();

  void XY_MouseUp(int x, int y, unsigned int buttons);
  void XY_MouseDown(int x, int y, unsigned int buttons);
  void XY_MouseMoved(int x, int y, unsigned int buttons);

  void NewBrushDrag_Begin(int x, int y);
  void NewBrushDrag(int x, int y);
  void NewBrushDrag_End(int x, int y);

  void XY_ToPoint(int x, int y, Vector3& point);
  void XY_SnapToGrid(Vector3& point);

  void Move_Begin();
  void Move_End();
  bool m_move_started;
  guint m_move_focusOut;

  void Zoom_Begin();
  void Zoom_End();
  bool m_zoom_started;
  guint m_zoom_focusOut;

  void SetActive(bool b)
  {
    m_bActive = b;
  };
  bool Active()
  {
    return m_bActive;
  };

  void Clipper_OnLButtonDown(int x, int y);
  void Clipper_OnLButtonUp(int x, int y);
  void Clipper_OnMouseMoved(int x, int y);
  void Clipper_Crosshair_OnMouseMoved(int x, int y);
  void DropClipPoint(int pointx, int pointy);

  void SetViewType(VIEWTYPE n);
  bool m_bActive;

  static GtkMenu* m_mnuDrop;

  int m_chasemouse_current_x, m_chasemouse_current_y;
  int m_chasemouse_delta_x, m_chasemouse_delta_y;
  

  guint m_chasemouse_handler;
  void ChaseMouse();
  bool chaseMouseMotion(int pointx, int pointy);

  void updateModelview();
  void updateProjection();
  Matrix4 m_projection;
  Matrix4 m_modelview;

  int m_nWidth;
  int m_nHeight;
private:
  float	m_fScale;
  Vector3 m_vOrigin;


  View m_view;
  static Shader* m_state_selected;

  int m_ptCursorX, m_ptCursorY;

  unsigned int m_buttonstate;
  void ButtonState_onMouseDown(unsigned int buttons)
  {
    m_buttonstate |= buttons;
  }
  void ButtonState_onMouseUp(unsigned int buttons)
  {
    m_buttonstate &= ~buttons;
  }
  unsigned int getButtonState() const
  {
    return m_buttonstate;
  }

  int m_nNewBrushPressx;
  int m_nNewBrushPressy;
  scene::Node* m_NewBrushDrag;
  bool m_bNewBrushDrag;

  Vector3 m_mousePosition;

  VIEWTYPE m_viewType;

  void OriginalButtonUp(guint32 nFlags, int point, int pointy);
  void OriginalButtonDown(guint32 nFlags, int point, int pointy);

  void OnContextMenu();
  void PaintSizeInfo(int nDim1, int nDim2, Vector3& vMinBounds, Vector3& vMaxBounds);

  int m_entityCreate_x, m_entityCreate_y;
  bool m_entityCreate;

public:
  void EntityCreate_MouseDown(int x, int y);
  void EntityCreate_MouseMove(int x, int y);
  void EntityCreate_MouseUp(int x, int y);

  void OnEntityCreate(const char* item);
  VIEWTYPE GetViewType()
  {
    return m_viewType;
  }
  void SetScale(float f);
  float Scale()
  {
    return m_fScale;
  }
  int Width()
  {
    return m_nWidth;
  }
  int Height()
  {
    return m_nHeight;
  }
};

inline void XYWnd_Update(XYWnd& xywnd)
{
  xywnd.queueDraw();
}


struct xywindow_globals_t
{
  Vector3 color_gridback;
  Vector3 color_gridminor;
  Vector3 color_gridmajor;
  Vector3 color_gridblock;
  Vector3 color_gridtext;
  Vector3 color_brushes;
  Vector3 color_selbrushes;
  Vector3 color_clipper;
  Vector3 color_viewname;
  Vector3 color_gridminor_alt;
  Vector3 color_gridmajor_alt;
  Vector3 AxisColorX;
  Vector3 AxisColorY;
  Vector3 AxisColorZ;

  bool m_bRightClick;
  bool m_bNoStipple;

  xywindow_globals_t() :
    color_gridback(1.f, 1.f, 1.f),
    color_gridminor(0.75f, 0.75f, 0.75f),
    color_gridmajor(0.5f, 0.5f, 0.5f),
    color_gridblock(0.f, 0.f, 1.f),
    color_gridtext(0.f, 0.f, 0.f),
    color_brushes(0.f, 0.f, 0.f),
    color_selbrushes(1.f, 0.f, 0.f),
    color_clipper(0.f, 0.f, 1.f),
    color_viewname(0.5f, 0.f, 0.75f),
    color_gridminor_alt(0.f, 0.f, 0.f),
    color_gridmajor_alt(0.f, 0.f, 0.f),

    AxisColorX(1.f, 0.f, 0.f),
    AxisColorY(0.f, 1.f, 0.f),
    AxisColorZ(0.f, 0.f, 1.f),
    m_bRightClick(true),
    m_bNoStipple(false)
  {
  }

};

extern xywindow_globals_t g_xywindow_globals;


VIEWTYPE GlobalXYWnd_getCurrentViewType();

typedef struct _GtkWindow GtkWindow;
void XY_Top_Shown_Construct(GtkWindow* parent);
void YZ_Side_Shown_Construct(GtkWindow* parent);
void XZ_Front_Shown_Construct(GtkWindow* parent);

void XYWindow_Construct();
void XYWindow_Destroy();

void XYShow_registerCommands();
void XYWnd_registerShortcuts();

#endif
