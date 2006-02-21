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
// Camera Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "camwindow.h"

#include "debugging/debugging.h"

#include "iscenegraph.h"
#include "irender.h"
#include "igl.h"
#include "icamera.h"
#include "cullable.h"
#include "renderable.h"
#include "preferencesystem.h"

#include "generic/callback.h"
#include "container/array.h"
#include "scenelib.h"
#include "render.h"
#include "cmdlib.h"
#include "math/frustum.h"

#include "gtkutil/widget.h"
#include "gtkutil/button.h"
#include "gtkutil/toolbar.h"
#include "gtkutil/glwidget.h"
#include "gtkutil/xorrectangle.h"
#include "gtkmisc.h"
#include "selection.h"
#include "mainframe.h"
#include "preferences.h"
#include "commands.h"
#include "xywindow.h"
#include "windowobservers.h"
#include "renderstate.h"

#include "timer.h"

std::vector<Callback> g_cameraMoved_callbacks;

void AddCameraMovedCallback(const Callback& callback)
{
  g_cameraMoved_callbacks.push_back(callback);
}

void CameraMovedNotify()
{
  std::for_each(g_cameraMoved_callbacks.begin(), g_cameraMoved_callbacks.end(), CallbackInvoke());
}


struct camwindow_globals_private_t
{
  int m_nMoveSpeed;
  int m_nAngleSpeed;
  bool m_bCamInverseMouse;
  bool m_bCamDiscrete;
  bool m_bCubicClipping;

  camwindow_globals_private_t() :
    m_nMoveSpeed(100),
    m_nAngleSpeed(3),
    m_bCamInverseMouse(false),
    m_bCamDiscrete(true),
    m_bCubicClipping(true)
  {
  }

};

camwindow_globals_private_t g_camwindow_globals_private;


const Matrix4 g_opengl2radiant(
  0, 0,-1, 0,
 -1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 0, 1
);

const Matrix4 g_radiant2opengl(
  0,-1, 0, 0,
  0, 0, 1, 0,
 -1, 0, 0, 0,
  0, 0, 0, 1
);

struct camera_t;
void Camera_mouseMove(camera_t& camera, int x, int y);

enum camera_draw_mode
{
  cd_wire,
  cd_solid,
  cd_texture,
  cd_lighting
};

struct camera_t
{
  int width, height;

  bool timing;

  Vector3 origin;
  Vector3 angles;

  Vector3 color;   // background 

  Vector3  forward, right; // move matrix (TTimo: used to have up but it was not updated)
  Vector3 vup, vpn, vright; // view matrix (taken from the modelview matrix)

  Matrix4 projection;
  Matrix4 modelview;

  unsigned int movementflags;  // movement flags
  Timer m_keycontrol_timer;
  guint m_keymove_handler;


  float fieldOfView;

  DeferredMotionDelta m_mouseMove;

  static void motionDelta(int x, int y, void* data)
  {
    Camera_mouseMove(*reinterpret_cast<camera_t*>(data), x, y);
  }

  View* m_view;
  Callback m_update;

  static camera_draw_mode draw_mode;

  camera_t(View* view, const Callback& update)
    : width(0),
    height(0),
    timing(false),
    origin(0, 0, 0),
    angles(0, 0, 0),
    color(0, 0, 0),
    movementflags(0),
    m_keymove_handler(0),
    fieldOfView(90.0f),
    m_mouseMove(motionDelta, this),
    m_view(view),
    m_update(update)
  {
  }
};

camera_draw_mode camera_t::draw_mode = cd_texture;

inline Matrix4 projection_for_camera(float near_z, float far_z, float fieldOfView, int width, int height)
{
  const float half_width = static_cast<float>(near_z * tan(degrees_to_radians(fieldOfView * 0.5)));
  const float half_height = half_width * (static_cast<float>(height) / static_cast<float>(width));

  return matrix4_frustum(
    -half_width,
    half_width,
    -half_height,
    half_height,
    near_z,
    far_z
  );
}

float Camera_getFarClipPlane(camera_t& camera)
{
  return (g_camwindow_globals_private.m_bCubicClipping)? pow(2.0, (g_camwindow_globals.m_nCubicScale + 7) / 2.0) : 32768.0f;
}

void Camera_updateProjection(camera_t& camera)
{
  float farClip = Camera_getFarClipPlane(camera);
  camera.projection = projection_for_camera(farClip / 4096.0f, farClip, camera.fieldOfView, camera.width, camera.height);

  camera.m_view->Construct(camera.projection, camera.modelview, camera.width, camera.height);
}

void Camera_updateVectors(camera_t& camera)
{
  for (int i=0 ; i<3 ; i++)
  {
    camera.vright[i] = camera.modelview[(i<<2)+0];
    camera.vup[i] = camera.modelview[(i<<2)+1];
    camera.vpn[i] = camera.modelview[(i<<2)+2];
  }
}

void Camera_updateModelview(camera_t& camera)
{
  camera.modelview = g_matrix4_identity;

  // roll, pitch, yaw
  Vector3 radiant_eulerXYZ(0, -camera.angles[CAMERA_PITCH], camera.angles[CAMERA_YAW]);

  matrix4_translate_by_vec3(camera.modelview, camera.origin);
  matrix4_rotate_by_euler_xyz_degrees(camera.modelview, radiant_eulerXYZ);
  matrix4_multiply_by_matrix4(camera.modelview, g_radiant2opengl);
  matrix4_affine_invert(camera.modelview);

  Camera_updateVectors(camera);

  camera.m_view->Construct(camera.projection, camera.modelview, camera.width, camera.height);
}


void Camera_Move_updateAxes(camera_t& camera)
{
  double ya = degrees_to_radians(camera.angles[CAMERA_YAW]);

  // the movement matrix is kept 2d
  camera.forward[0] = static_cast<float>(cos(ya));
  camera.forward[1] = static_cast<float>(sin(ya));
  camera.forward[2] = 0;
  camera.right[0] = camera.forward[1];
  camera.right[1] = -camera.forward[0];
}

void Camera_Freemove_updateAxes(camera_t& camera)
{
  camera.right = camera.vright;
  camera.forward = vector3_negated(camera.vpn);
}

const Vector3& Camera_getOrigin(camera_t& camera)
{
  return camera.origin;
}

void Camera_setOrigin(camera_t& camera, const Vector3& origin)
{
  camera.origin = origin;
  Camera_updateModelview(camera);
  camera.m_update();
  CameraMovedNotify();
}

const Vector3& Camera_getAngles(camera_t& camera)
{
  return camera.angles;
}

void Camera_setAngles(camera_t& camera, const Vector3& angles)
{
  camera.angles = angles;
  Camera_updateModelview(camera);
  camera.m_update();
  CameraMovedNotify();
}


void Camera_FreeMove(camera_t& camera, int dx, int dy)
{
  float dtime = 0.1f;
  if (g_camwindow_globals_private.m_bCamInverseMouse)
    camera.angles[CAMERA_PITCH] -= dy * dtime * g_camwindow_globals_private.m_nAngleSpeed;
  else
    camera.angles[CAMERA_PITCH] += dy * dtime * g_camwindow_globals_private.m_nAngleSpeed;

  camera.angles[CAMERA_YAW] += dx * dtime * g_camwindow_globals_private.m_nAngleSpeed;

  if (camera.angles[CAMERA_PITCH] > 90)
    camera.angles[CAMERA_PITCH] = 90;
  else if (camera.angles[CAMERA_PITCH] < -90)
    camera.angles[CAMERA_PITCH] = -90;

  if (camera.angles[CAMERA_YAW] >= 360)
    camera.angles[CAMERA_YAW] -=360;
  else if (camera.angles[CAMERA_YAW] <= 0)
    camera.angles[CAMERA_YAW] +=360;

  Camera_updateModelview(camera);
  Camera_Freemove_updateAxes(camera);
}

void Cam_MouseControl(camera_t& camera, int x, int y)
{
  int   xl, xh;
  int yl, yh;
  float xf, yf;

  xf = (float)(x - camera.width/2) / (camera.width/2);
  yf = (float)(y - camera.height/2) / (camera.height/2);

  xl = camera.width/3;
  xh = xl*2;
  yl = camera.height/3;
  yh = yl*2;

  xf *= 1.0f - fabsf(yf);
  if (xf < 0)
  {
    xf += 0.1f;
    if (xf > 0)
      xf = 0;
  }
  else
  {
    xf -= 0.1f;
    if (xf < 0)
      xf = 0;
  }

  vector3_add(camera.origin, vector3_scaled(camera.forward, yf * 0.1f* g_camwindow_globals_private.m_nMoveSpeed));
  camera.angles[CAMERA_YAW] += xf * -0.1f * g_camwindow_globals_private.m_nAngleSpeed;

  Camera_updateModelview(camera);
}

void Camera_mouseMove(camera_t& camera, int x, int y)
{
  //globalOutputStream() << "mousemove... ";
  Camera_FreeMove(camera, -x, -y);
  camera.m_update();
  CameraMovedNotify();
}

const unsigned int MOVE_NONE = 0;
const unsigned int MOVE_FORWARD = 1 << 0;
const unsigned int MOVE_BACK = 1 << 1;
const unsigned int MOVE_ROTRIGHT = 1 << 2;
const unsigned int MOVE_ROTLEFT = 1 << 3;
const unsigned int MOVE_STRAFERIGHT = 1 << 4;
const unsigned int MOVE_STRAFELEFT = 1 << 5;
const unsigned int MOVE_UP = 1 << 6;
const unsigned int MOVE_DOWN = 1 << 7;
const unsigned int MOVE_PITCHUP = 1 << 8;
const unsigned int MOVE_PITCHDOWN = 1 << 9;
const unsigned int MOVE_ALL = MOVE_FORWARD|MOVE_BACK|MOVE_ROTRIGHT|MOVE_ROTLEFT|MOVE_STRAFERIGHT|MOVE_STRAFELEFT|MOVE_UP|MOVE_DOWN|MOVE_PITCHUP|MOVE_PITCHDOWN;

void Cam_KeyControl(camera_t& camera, float dtime)
{
  // Update angles
  if (camera.movementflags & MOVE_ROTLEFT)
    camera.angles[CAMERA_YAW] += 15 * dtime* g_camwindow_globals_private.m_nAngleSpeed;
  if (camera.movementflags & MOVE_ROTRIGHT)
    camera.angles[CAMERA_YAW] -= 15 * dtime * g_camwindow_globals_private.m_nAngleSpeed;
  if (camera.movementflags & MOVE_PITCHUP)
  {
    camera.angles[CAMERA_PITCH] += 15 * dtime* g_camwindow_globals_private.m_nAngleSpeed;
    if(camera.angles[CAMERA_PITCH] > 90)
      camera.angles[CAMERA_PITCH] = 90;
  }
  if (camera.movementflags & MOVE_PITCHDOWN)
  {
    camera.angles[CAMERA_PITCH] -= 15 * dtime * g_camwindow_globals_private.m_nAngleSpeed;
    if(camera.angles[CAMERA_PITCH] < -90)
      camera.angles[CAMERA_PITCH] = -90;
  }

  Camera_updateModelview(camera);
  Camera_Freemove_updateAxes(camera);

  // Update position
  if (camera.movementflags & MOVE_FORWARD)
    vector3_add(camera.origin, vector3_scaled(camera.forward, dtime * g_camwindow_globals_private.m_nMoveSpeed));
  if (camera.movementflags & MOVE_BACK)
    vector3_add(camera.origin, vector3_scaled(camera.forward, -dtime * g_camwindow_globals_private.m_nMoveSpeed));
  if (camera.movementflags & MOVE_STRAFELEFT)
    vector3_add(camera.origin, vector3_scaled(camera.right, -dtime * g_camwindow_globals_private.m_nMoveSpeed));
  if (camera.movementflags & MOVE_STRAFERIGHT)
    vector3_add(camera.origin, vector3_scaled(camera.right, dtime * g_camwindow_globals_private.m_nMoveSpeed));
  if (camera.movementflags & MOVE_UP)
    vector3_add(camera.origin, vector3_scaled(g_vector3_axis_z, dtime * g_camwindow_globals_private.m_nMoveSpeed));
  if (camera.movementflags & MOVE_DOWN)
    vector3_add(camera.origin, vector3_scaled(g_vector3_axis_z, -dtime * g_camwindow_globals_private.m_nMoveSpeed));

  Camera_updateModelview(camera);
}

void Camera_keyMove(camera_t& camera)
{
  camera.m_mouseMove.flush();

  //globalOutputStream() << "keymove... ";
  float time_seconds = camera.m_keycontrol_timer.elapsed_msec() / static_cast<float>(msec_per_sec);
  camera.m_keycontrol_timer.start();
  if(time_seconds > 0.05f)
  {
    time_seconds = 0.05f; // 20fps
  }
  Cam_KeyControl(camera, time_seconds * 5.0f);

  camera.m_update();
  CameraMovedNotify();
}

gboolean camera_keymove(gpointer data)
{
  Camera_keyMove(*reinterpret_cast<camera_t*>(data));
  return TRUE;
}

void Camera_setMovementFlags(camera_t& camera, unsigned int mask)
{
  if((~camera.movementflags & mask) != 0 && camera.movementflags == 0)
  {
    camera.m_keymove_handler = g_idle_add(camera_keymove, &camera);
  }
  camera.movementflags |= mask;
}
void Camera_clearMovementFlags(camera_t& camera, unsigned int mask)
{
  if((camera.movementflags & ~mask) == 0 && camera.movementflags != 0)
  {
    g_source_remove(camera.m_keymove_handler);
    camera.m_keymove_handler = 0;
  }
  camera.movementflags &= ~mask;
}

void Camera_MoveForward_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_FORWARD);
}
void Camera_MoveForward_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_FORWARD);
}
void Camera_MoveBack_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_BACK);
}
void Camera_MoveBack_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_BACK);
}

void Camera_MoveLeft_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_STRAFELEFT);
}
void Camera_MoveLeft_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_STRAFELEFT);
}
void Camera_MoveRight_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_STRAFERIGHT);
}
void Camera_MoveRight_KeyUp(camera_t& camera)
{ 
  Camera_clearMovementFlags(camera, MOVE_STRAFERIGHT);
}

void Camera_MoveUp_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_UP);
}
void Camera_MoveUp_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_UP);
}
void Camera_MoveDown_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_DOWN);
}
void Camera_MoveDown_KeyUp(camera_t& camera)
{ 
  Camera_clearMovementFlags(camera, MOVE_DOWN);
}

void Camera_RotateLeft_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_ROTLEFT);
}
void Camera_RotateLeft_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_ROTLEFT);
}
void Camera_RotateRight_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_ROTRIGHT);
}
void Camera_RotateRight_KeyUp(camera_t& camera)
{ 
  Camera_clearMovementFlags(camera, MOVE_ROTRIGHT);
}

void Camera_PitchUp_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_PITCHUP);
}
void Camera_PitchUp_KeyUp(camera_t& camera)
{
  Camera_clearMovementFlags(camera, MOVE_PITCHUP);
}
void Camera_PitchDown_KeyDown(camera_t& camera)
{
  Camera_setMovementFlags(camera, MOVE_PITCHDOWN);
}
void Camera_PitchDown_KeyUp(camera_t& camera)
{ 
  Camera_clearMovementFlags(camera, MOVE_PITCHDOWN);
}


typedef ReferenceCaller<camera_t, &Camera_MoveForward_KeyDown> FreeMoveCameraMoveForwardKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveForward_KeyUp> FreeMoveCameraMoveForwardKeyUpCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveBack_KeyDown> FreeMoveCameraMoveBackKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveBack_KeyUp> FreeMoveCameraMoveBackKeyUpCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveLeft_KeyDown> FreeMoveCameraMoveLeftKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveLeft_KeyUp> FreeMoveCameraMoveLeftKeyUpCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveRight_KeyDown> FreeMoveCameraMoveRightKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveRight_KeyUp> FreeMoveCameraMoveRightKeyUpCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveUp_KeyDown> FreeMoveCameraMoveUpKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveUp_KeyUp> FreeMoveCameraMoveUpKeyUpCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveDown_KeyDown> FreeMoveCameraMoveDownKeyDownCaller;
typedef ReferenceCaller<camera_t, &Camera_MoveDown_KeyUp> FreeMoveCameraMoveDownKeyUpCaller;


#define SPEED_MOVE 32
#define SPEED_TURN 22.5

void Camera_MoveForward_Discrete(camera_t& camera)
{
  Camera_Move_updateAxes(camera);
  Camera_setOrigin(camera, vector3_added(Camera_getOrigin(camera), vector3_scaled(camera.forward, SPEED_MOVE)));
}
void Camera_MoveBack_Discrete(camera_t& camera)
{
  Camera_Move_updateAxes(camera);
  Camera_setOrigin(camera, vector3_added(Camera_getOrigin(camera), vector3_scaled(camera.forward, -SPEED_MOVE)));
}

void Camera_MoveUp_Discrete(camera_t& camera)
{
  Vector3 origin(Camera_getOrigin(camera));
  origin[2] += SPEED_MOVE;
  Camera_setOrigin(camera, origin);
}
void Camera_MoveDown_Discrete(camera_t& camera)
{
  Vector3 origin(Camera_getOrigin(camera));
  origin[2] -= SPEED_MOVE;
  Camera_setOrigin(camera, origin);
}

void Camera_MoveLeft_Discrete(camera_t& camera)
{
  Camera_Move_updateAxes(camera);
  Camera_setOrigin(camera, vector3_added(Camera_getOrigin(camera), vector3_scaled(camera.right, -SPEED_MOVE)));
}
void Camera_MoveRight_Discrete(camera_t& camera)
{
  Camera_Move_updateAxes(camera);
  Camera_setOrigin(camera, vector3_added(Camera_getOrigin(camera), vector3_scaled(camera.right, SPEED_MOVE)));
}

void Camera_RotateLeft_Discrete(camera_t& camera)
{
  Vector3 angles(Camera_getAngles(camera));
  angles[CAMERA_YAW] += SPEED_TURN;
  Camera_setAngles(camera, angles);
}
void Camera_RotateRight_Discrete(camera_t& camera)
{
  Vector3 angles(Camera_getAngles(camera));
  angles[CAMERA_YAW] -= SPEED_TURN;
  Camera_setAngles(camera, angles);
}

void Camera_PitchUp_Discrete(camera_t& camera)
{
  Vector3 angles(Camera_getAngles(camera));
  angles[CAMERA_PITCH] += SPEED_TURN;
  if (angles[CAMERA_PITCH] > 90)
    angles[CAMERA_PITCH] = 90;
  Camera_setAngles(camera, angles);
}
void Camera_PitchDown_Discrete(camera_t& camera)
{
  Vector3 angles(Camera_getAngles(camera));
  angles[CAMERA_PITCH] -= SPEED_TURN;
  if (angles[CAMERA_PITCH] < -90)
    angles[CAMERA_PITCH] = -90;
  Camera_setAngles(camera, angles);
}


class RadiantCameraView : public CameraView
{
  camera_t& m_camera;
  View* m_view;
  Callback m_update;
public:
  RadiantCameraView(camera_t& camera, View* view, const Callback& update) : m_camera(camera), m_view(view), m_update(update)
  {
  }
  void update()
  {
    m_view->Construct(m_camera.projection, m_camera.modelview, m_camera.width, m_camera.height);
    m_update();
  }
  void setModelview(const Matrix4& modelview)
  {
    m_camera.modelview = modelview;
    matrix4_multiply_by_matrix4(m_camera.modelview, g_radiant2opengl);
    matrix4_affine_invert(m_camera.modelview);
    Camera_updateVectors(m_camera);
    update();
  }
  void setFieldOfView(float fieldOfView)
  {
    float farClip = Camera_getFarClipPlane(m_camera);
    m_camera.projection = projection_for_camera(farClip / 4096.0f, farClip, fieldOfView, m_camera.width, m_camera.height);
    update();
  }
};


void Camera_motionDelta(int x, int y, unsigned int state, void* data)
{
  reinterpret_cast<camera_t*>(data)->m_mouseMove.motion_delta(x, y, state);
}

class CamWnd
{
  View m_view;
  camera_t m_Camera;
  RadiantCameraView m_cameraview;
#if 0
  int m_PositionDragCursorX;
  int m_PositionDragCursorY;
#endif

  guint m_freemove_handle_focusout;

  static Shader* m_state_select1;
  static Shader* m_state_select2;

  FreezePointer m_freezePointer;

public:
  GtkWidget* m_gl_widget;
  GtkWindow* m_parent;

  SelectionSystemWindowObserver* m_window_observer;
  XORRectangle m_XORRectangle;

  DeferredDraw m_deferredDraw;
  DeferredMotion m_deferred_motion;

  guint m_selection_button_press_handler;
  guint m_selection_button_release_handler;
  guint m_selection_motion_handler;

  guint m_freelook_button_press_handler;

  guint m_sizeHandler;
  guint m_exposeHandler;

  CamWnd();
  ~CamWnd();

  bool m_drawing;
  void queue_draw()
  {
    //ASSERT_MESSAGE(!m_drawing, "CamWnd::queue_draw(): called while draw is already in progress");
    if(m_drawing)
    {
      return;
    }
    //globalOutputStream() << "queue... ";
    m_deferredDraw.draw();
  }
  void draw();

  static void captureStates()
  {
    m_state_select1 = GlobalShaderCache().capture("$CAM_HIGHLIGHT");
    m_state_select2 = GlobalShaderCache().capture("$CAM_OVERLAY");
  }
  static void releaseStates()
  {
    GlobalShaderCache().release("$CAM_HIGHLIGHT");
    GlobalShaderCache().release("$CAM_OVERLAY");
  }

  camera_t& getCamera()
  {
    return m_Camera;
  };

  void BenchMark();
  void Cam_ChangeFloor(bool up);

  void DisableFreeMove();
  void EnableFreeMove();
  bool m_bFreeMove;

  CameraView& getCameraView()
  {
    return m_cameraview;
  }

private:
  void Cam_Draw();
};

typedef MemberCaller<CamWnd, &CamWnd::queue_draw> CamWndQueueDraw;

Shader* CamWnd::m_state_select1 = 0;
Shader* CamWnd::m_state_select2 = 0;

CamWnd* NewCamWnd()
{
  return new CamWnd;
}
void DeleteCamWnd(CamWnd* camwnd)
{
  delete camwnd;
}

void CamWnd_constructStatic()
{
  CamWnd::captureStates();
}

void CamWnd_destroyStatic()
{
  CamWnd::releaseStates();
}

static CamWnd* g_camwnd = 0;

void GlobalCamera_setCamWnd(CamWnd& camwnd)
{
  g_camwnd = &camwnd;
}


GtkWidget* CamWnd_getWidget(CamWnd& camwnd)
{
  return camwnd.m_gl_widget;
}

GtkWindow* CamWnd_getParent(CamWnd& camwnd)
{
  return camwnd.m_parent;
}

ToggleShown g_camera_shown(true);

void CamWnd_setParent(CamWnd& camwnd, GtkWindow* parent)
{
  camwnd.m_parent = parent;
  g_camera_shown.connect(GTK_WIDGET(camwnd.m_parent));
}

void CamWnd_Update(CamWnd& camwnd)
{
  camwnd.queue_draw();
}



camwindow_globals_t g_camwindow_globals;

const Vector3& Camera_getOrigin(CamWnd& camwnd)
{
  return Camera_getOrigin(camwnd.getCamera());
}

void Camera_setOrigin(CamWnd& camwnd, const Vector3& origin)
{
  Camera_setOrigin(camwnd.getCamera(), origin);
}

const Vector3& Camera_getAngles(CamWnd& camwnd)
{
  return Camera_getAngles(camwnd.getCamera());
}

void Camera_setAngles(CamWnd& camwnd, const Vector3& angles)
{
  Camera_setAngles(camwnd.getCamera(), angles);
}


// =============================================================================
// CamWnd class

gboolean enable_freelook_button_press(GtkWidget* widget, GdkEventButton* event, CamWnd* camwnd)
{
  if(event->type == GDK_BUTTON_PRESS && event->button == 3)
  {
    camwnd->EnableFreeMove();
    return TRUE;
  }
  return FALSE;
}

gboolean disable_freelook_button_press(GtkWidget* widget, GdkEventButton* event, CamWnd* camwnd)
{
  if(event->type == GDK_BUTTON_PRESS && event->button == 3)
  {
    camwnd->DisableFreeMove();
    return TRUE;
  }
  return FALSE;
}

#if 0
gboolean mousecontrol_button_press(GtkWidget* widget, GdkEventButton* event, CamWnd* camwnd)
{
  if(event->type == GDK_BUTTON_PRESS && event->button == 3)
  {
    Cam_MouseControl(camwnd->getCamera(), event->x, widget->allocation.height - 1 - event->y);
  }
  return FALSE;
}
#endif

void camwnd_update_xor_rectangle(CamWnd& self, rect_t area)
{
  if(GTK_WIDGET_VISIBLE(self.m_gl_widget))
  {
    self.m_XORRectangle.set(rectangle_from_area(area.min, area.max, self.getCamera().width, self.getCamera().height));
  }
}


gboolean selection_button_press(GtkWidget* widget, GdkEventButton* event, WindowObserver* observer)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    observer->onMouseDown(WindowVector_forDouble(event->x, event->y), button_for_button(event->button), modifiers_for_state(event->state));
  }
  return FALSE;
}

gboolean selection_button_release(GtkWidget* widget, GdkEventButton* event, WindowObserver* observer)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    observer->onMouseUp(WindowVector_forDouble(event->x, event->y), button_for_button(event->button), modifiers_for_state(event->state));
  }
  return FALSE;
}

void selection_motion(gdouble x, gdouble y, guint state, void* data)
{
  //globalOutputStream() << "motion... ";
  reinterpret_cast<WindowObserver*>(data)->onMouseMotion(WindowVector_forDouble(x, y), modifiers_for_state(state));
}

inline WindowVector windowvector_for_widget_centre(GtkWidget* widget)
{
  return WindowVector(static_cast<float>(widget->allocation.width / 2), static_cast<float>(widget->allocation.height / 2));
}

gboolean selection_button_press_freemove(GtkWidget* widget, GdkEventButton* event, WindowObserver* observer)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    observer->onMouseDown(windowvector_for_widget_centre(widget), button_for_button(event->button), modifiers_for_state(event->state));
  }
  return FALSE;
}

gboolean selection_button_release_freemove(GtkWidget* widget, GdkEventButton* event, WindowObserver* observer)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    observer->onMouseUp(windowvector_for_widget_centre(widget), button_for_button(event->button), modifiers_for_state(event->state));
  }
  return FALSE;
}

gboolean selection_motion_freemove(GtkWidget *widget, GdkEventMotion *event, WindowObserver* observer)
{
  observer->onMouseMotion(windowvector_for_widget_centre(widget), modifiers_for_state(event->state));
  return FALSE;
}

gboolean wheelmove_scroll(GtkWidget* widget, GdkEventScroll* event, CamWnd* camwnd)
{
  if(event->direction == GDK_SCROLL_UP)
  {
    Camera_Freemove_updateAxes(camwnd->getCamera());
    Camera_setOrigin(*camwnd, vector3_added(Camera_getOrigin(*camwnd), vector3_scaled(camwnd->getCamera().forward, static_cast<float>(g_camwindow_globals_private.m_nMoveSpeed))));
  }
  else if(event->direction == GDK_SCROLL_DOWN)
  {
    Camera_Freemove_updateAxes(camwnd->getCamera());
    Camera_setOrigin(*camwnd, vector3_added(Camera_getOrigin(*camwnd), vector3_scaled(camwnd->getCamera().forward, -static_cast<float>(g_camwindow_globals_private.m_nMoveSpeed))));
  }

  return FALSE;
}

gboolean camera_size_allocate(GtkWidget* widget, GtkAllocation* allocation, CamWnd* camwnd)
{
  camwnd->getCamera().width = allocation->width;
  camwnd->getCamera().height = allocation->height;
  Camera_updateProjection(camwnd->getCamera());
  camwnd->m_window_observer->onSizeChanged(camwnd->getCamera().width, camwnd->getCamera().height);
  camwnd->queue_draw();
  return FALSE;
}

gboolean camera_expose(GtkWidget* widget, GdkEventExpose* event, gpointer data)
{
  reinterpret_cast<CamWnd*>(data)->draw();
  return FALSE;
}

void KeyEvent_connect(const char* name)
{
  const KeyEvent& keyEvent = GlobalKeyEvents_find(name);
  keydown_accelerators_add(keyEvent.m_accelerator, keyEvent.m_keyDown);
  keyup_accelerators_add(keyEvent.m_accelerator, keyEvent.m_keyUp);
}

void KeyEvent_disconnect(const char* name)
{
  const KeyEvent& keyEvent = GlobalKeyEvents_find(name);
  keydown_accelerators_remove(keyEvent.m_accelerator);
  keyup_accelerators_remove(keyEvent.m_accelerator);
}

void CamWnd_registerCommands(CamWnd& camwnd)
{
  GlobalKeyEvents_insert("CameraForward", Accelerator(GDK_Up),
    ReferenceCaller<camera_t, Camera_MoveForward_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveForward_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraBack", Accelerator(GDK_Down),
    ReferenceCaller<camera_t, Camera_MoveBack_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveBack_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraLeft", Accelerator(GDK_Left),
    ReferenceCaller<camera_t, Camera_RotateLeft_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_RotateLeft_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraRight", Accelerator(GDK_Right),
    ReferenceCaller<camera_t, Camera_RotateRight_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_RotateRight_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraStrafeRight", Accelerator(GDK_period),
    ReferenceCaller<camera_t, Camera_MoveRight_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveRight_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraStrafeLeft", Accelerator(GDK_comma),
    ReferenceCaller<camera_t, Camera_MoveLeft_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveLeft_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraUp", Accelerator('D'),
    ReferenceCaller<camera_t, Camera_MoveUp_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveUp_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraDown", Accelerator('C'),
    ReferenceCaller<camera_t, Camera_MoveDown_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_MoveDown_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraAngleDown", Accelerator('A'),
    ReferenceCaller<camera_t, Camera_PitchDown_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_PitchDown_KeyUp>(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraAngleUp", Accelerator('Z'),
    ReferenceCaller<camera_t, Camera_PitchUp_KeyDown>(camwnd.getCamera()),
    ReferenceCaller<camera_t, Camera_PitchUp_KeyUp>(camwnd.getCamera())
  );

  GlobalKeyEvents_insert("CameraFreeMoveForward", Accelerator(GDK_Up),
    FreeMoveCameraMoveForwardKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveForwardKeyUpCaller(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraFreeMoveBack", Accelerator(GDK_Down),
    FreeMoveCameraMoveBackKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveBackKeyUpCaller(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraFreeMoveLeft", Accelerator(GDK_Left),
    FreeMoveCameraMoveLeftKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveLeftKeyUpCaller(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraFreeMoveRight", Accelerator(GDK_Right),
    FreeMoveCameraMoveRightKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveRightKeyUpCaller(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraFreeMoveUp", Accelerator('D'),
    FreeMoveCameraMoveUpKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveUpKeyUpCaller(camwnd.getCamera())
  );
  GlobalKeyEvents_insert("CameraFreeMoveDown", Accelerator('C'),
    FreeMoveCameraMoveDownKeyDownCaller(camwnd.getCamera()),
    FreeMoveCameraMoveDownKeyUpCaller(camwnd.getCamera())
  );

  GlobalCommands_insert("CameraForward", ReferenceCaller<camera_t, Camera_MoveForward_Discrete>(camwnd.getCamera()), Accelerator(GDK_Up));
  GlobalCommands_insert("CameraBack", ReferenceCaller<camera_t, Camera_MoveBack_Discrete>(camwnd.getCamera()), Accelerator(GDK_Down));
  GlobalCommands_insert("CameraLeft", ReferenceCaller<camera_t, Camera_RotateLeft_Discrete>(camwnd.getCamera()), Accelerator(GDK_Left));
  GlobalCommands_insert("CameraRight", ReferenceCaller<camera_t, Camera_RotateRight_Discrete>(camwnd.getCamera()), Accelerator(GDK_Right));
  GlobalCommands_insert("CameraStrafeRight", ReferenceCaller<camera_t, Camera_MoveRight_Discrete>(camwnd.getCamera()), Accelerator(GDK_period));
  GlobalCommands_insert("CameraStrafeLeft", ReferenceCaller<camera_t, Camera_MoveLeft_Discrete>(camwnd.getCamera()), Accelerator(GDK_comma));

  GlobalCommands_insert("CameraUp", ReferenceCaller<camera_t, Camera_MoveUp_Discrete>(camwnd.getCamera()), Accelerator('D'));
  GlobalCommands_insert("CameraDown", ReferenceCaller<camera_t, Camera_MoveDown_Discrete>(camwnd.getCamera()), Accelerator('C'));
  GlobalCommands_insert("CameraAngleUp", ReferenceCaller<camera_t, Camera_PitchUp_Discrete>(camwnd.getCamera()), Accelerator('A'));
  GlobalCommands_insert("CameraAngleDown", ReferenceCaller<camera_t, Camera_PitchDown_Discrete>(camwnd.getCamera()), Accelerator('Z'));
}

void CamWnd_Move_Enable(CamWnd& camwnd)
{
  KeyEvent_connect("CameraForward");
  KeyEvent_connect("CameraBack");
  KeyEvent_connect("CameraLeft");
  KeyEvent_connect("CameraRight");
  KeyEvent_connect("CameraStrafeRight");
  KeyEvent_connect("CameraStrafeLeft");
  KeyEvent_connect("CameraUp");
  KeyEvent_connect("CameraDown");
  KeyEvent_connect("CameraAngleUp");
  KeyEvent_connect("CameraAngleDown");
}

void CamWnd_Move_Disable(CamWnd& camwnd)
{
  KeyEvent_disconnect("CameraForward");
  KeyEvent_disconnect("CameraBack");
  KeyEvent_disconnect("CameraLeft");
  KeyEvent_disconnect("CameraRight");
  KeyEvent_disconnect("CameraStrafeRight");
  KeyEvent_disconnect("CameraStrafeLeft");
  KeyEvent_disconnect("CameraUp");
  KeyEvent_disconnect("CameraDown");
  KeyEvent_disconnect("CameraAngleUp");
  KeyEvent_disconnect("CameraAngleDown");
}

void CamWnd_Move_Discrete_Enable(CamWnd& camwnd)
{
  command_connect_accelerator("CameraForward");
  command_connect_accelerator("CameraBack");
  command_connect_accelerator("CameraLeft");
  command_connect_accelerator("CameraRight");
  command_connect_accelerator("CameraStrafeRight");
  command_connect_accelerator("CameraStrafeLeft");
  command_connect_accelerator("CameraUp");
  command_connect_accelerator("CameraDown");
  command_connect_accelerator("CameraAngleUp");
  command_connect_accelerator("CameraAngleDown");
}

void CamWnd_Move_Discrete_Disable(CamWnd& camwnd)
{
  command_disconnect_accelerator("CameraForward");
  command_disconnect_accelerator("CameraBack");
  command_disconnect_accelerator("CameraLeft");
  command_disconnect_accelerator("CameraRight");
  command_disconnect_accelerator("CameraStrafeRight");
  command_disconnect_accelerator("CameraStrafeLeft");
  command_disconnect_accelerator("CameraUp");
  command_disconnect_accelerator("CameraDown");
  command_disconnect_accelerator("CameraAngleUp");
  command_disconnect_accelerator("CameraAngleDown");
}

void CamWnd_Move_Discrete_Import(CamWnd& camwnd, bool value)
{
  if(g_camwindow_globals_private.m_bCamDiscrete)
  {
    CamWnd_Move_Discrete_Disable(camwnd);
  }
  else
  {
    CamWnd_Move_Disable(camwnd);
  }

  g_camwindow_globals_private.m_bCamDiscrete = value;

  if(g_camwindow_globals_private.m_bCamDiscrete)
  {
    CamWnd_Move_Discrete_Enable(camwnd);
  }
  else
  {
    CamWnd_Move_Enable(camwnd);
  }
}

void CamWnd_Move_Discrete_Import(bool value)
{
  if(g_camwnd != 0)
  {
    CamWnd_Move_Discrete_Import(*g_camwnd, value);
  }
  else
  {
    g_camwindow_globals_private.m_bCamDiscrete = value;
  }
}


void CamWnd_Add_Handlers_Move(CamWnd& camwnd)
{
  camwnd.m_selection_button_press_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_press_event", G_CALLBACK(selection_button_press), camwnd.m_window_observer);
  camwnd.m_selection_button_release_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_release_event", G_CALLBACK(selection_button_release), camwnd.m_window_observer);
  camwnd.m_selection_motion_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "motion_notify_event", G_CALLBACK(DeferredMotion::gtk_motion), &camwnd.m_deferred_motion);

  camwnd.m_freelook_button_press_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_press_event", G_CALLBACK(enable_freelook_button_press), &camwnd);

  if(g_camwindow_globals_private.m_bCamDiscrete)
  {
    CamWnd_Move_Discrete_Enable(camwnd);
  }
  else
  {
    CamWnd_Move_Enable(camwnd);
  }
}

void CamWnd_Remove_Handlers_Move(CamWnd& camwnd)
{
  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_button_press_handler);
  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_button_release_handler);
  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_motion_handler);

  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_freelook_button_press_handler);

  if(g_camwindow_globals_private.m_bCamDiscrete)
  {
    CamWnd_Move_Discrete_Disable(camwnd);
  }
  else
  {
    CamWnd_Move_Disable(camwnd);
  }
}

void CamWnd_Add_Handlers_FreeMove(CamWnd& camwnd)
{
  camwnd.m_selection_button_press_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_press_event", G_CALLBACK(selection_button_press_freemove), camwnd.m_window_observer);
  camwnd.m_selection_button_release_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_release_event", G_CALLBACK(selection_button_release_freemove), camwnd.m_window_observer);
  camwnd.m_selection_motion_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "motion_notify_event", G_CALLBACK(selection_motion_freemove), camwnd.m_window_observer);

  camwnd.m_freelook_button_press_handler = g_signal_connect(G_OBJECT(camwnd.m_gl_widget), "button_press_event", G_CALLBACK(disable_freelook_button_press), &camwnd);

  KeyEvent_connect("CameraFreeMoveForward");
  KeyEvent_connect("CameraFreeMoveBack");
  KeyEvent_connect("CameraFreeMoveLeft");
  KeyEvent_connect("CameraFreeMoveRight");
  KeyEvent_connect("CameraFreeMoveUp");
  KeyEvent_connect("CameraFreeMoveDown");
}

void CamWnd_Remove_Handlers_FreeMove(CamWnd& camwnd)
{
  KeyEvent_disconnect("CameraFreeMoveForward");
  KeyEvent_disconnect("CameraFreeMoveBack");
  KeyEvent_disconnect("CameraFreeMoveLeft");
  KeyEvent_disconnect("CameraFreeMoveRight");
  KeyEvent_disconnect("CameraFreeMoveUp");
  KeyEvent_disconnect("CameraFreeMoveDown");

  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_button_press_handler);
  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_button_release_handler);
  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_selection_motion_handler);

  g_signal_handler_disconnect(G_OBJECT(camwnd.m_gl_widget), camwnd.m_freelook_button_press_handler);
}

CamWnd::CamWnd() :
  m_view(true),
  m_Camera(&m_view, CamWndQueueDraw(*this)),
  m_cameraview(m_Camera, &m_view, ReferenceCaller<CamWnd, CamWnd_Update>(*this)),
  m_gl_widget(glwidget_new(TRUE)),
  m_window_observer(NewWindowObserver()),
  m_XORRectangle(m_gl_widget),
  m_deferredDraw(WidgetQueueDrawCaller(*m_gl_widget)),
  m_deferred_motion(selection_motion, m_window_observer),
  m_selection_button_press_handler(0),
  m_selection_button_release_handler(0),
  m_selection_motion_handler(0),
  m_freelook_button_press_handler(0),
  m_drawing(false)
{
  m_bFreeMove = false;

  GlobalWindowObservers_add(m_window_observer);
  GlobalWindowObservers_connectWidget(m_gl_widget);

  m_window_observer->setRectangleDrawCallback(ReferenceCaller1<CamWnd, rect_t, camwnd_update_xor_rectangle>(*this));
  m_window_observer->setView(m_view);

  gtk_widget_ref(m_gl_widget);

  gtk_widget_set_events(m_gl_widget, GDK_DESTROY | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
  GTK_WIDGET_SET_FLAGS (m_gl_widget, GTK_CAN_FOCUS);

  m_sizeHandler = g_signal_connect(G_OBJECT(m_gl_widget), "size_allocate", G_CALLBACK(camera_size_allocate), this);
  m_exposeHandler = g_signal_connect(G_OBJECT(m_gl_widget), "expose_event", G_CALLBACK(camera_expose), this);

  Map_addValidCallback(g_map, DeferredDrawOnMapValidChangedCaller(m_deferredDraw));

  CamWnd_registerCommands(*this);

  CamWnd_Add_Handlers_Move(*this);

  g_signal_connect(G_OBJECT(m_gl_widget), "scroll_event", G_CALLBACK(wheelmove_scroll), this);

  AddSceneChangeCallback(ReferenceCaller<CamWnd, CamWnd_Update>(*this));

  PressedButtons_connect(g_pressedButtons, m_gl_widget);
}

CamWnd::~CamWnd()
{
  if(m_bFreeMove)
  {
    DisableFreeMove();
  }

  CamWnd_Remove_Handlers_Move(*this);

  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_sizeHandler);
  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_exposeHandler);

  gtk_widget_unref(m_gl_widget);

  m_window_observer->release();
}

class FloorHeightWalker : public scene::Graph::Walker
{
  float m_current;
  float& m_bestUp;
  float& m_bestDown;
public:
  FloorHeightWalker(float current, float& bestUp, float& bestDown) :
      m_current(current), m_bestUp(bestUp), m_bestDown(bestDown)
  {
    bestUp = g_MaxWorldCoord;
    bestDown = -g_MaxWorldCoord;
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible()
      && Node_isBrush(path.top())) // this node is a floor
    {
      const AABB& aabb = instance.worldAABB();
      float floorHeight = aabb.origin.z() + aabb.extents.z();
      if(floorHeight > m_current && floorHeight < m_bestUp)
      {
        m_bestUp = floorHeight;
      }
      if(floorHeight < m_current && floorHeight > m_bestDown)
      {
        m_bestDown = floorHeight;
      }
    }
    return true;
  }
};

void CamWnd::Cam_ChangeFloor(bool up)
{
  float current = m_Camera.origin[2] - 48;
  float bestUp;
  float bestDown;
  GlobalSceneGraph().traverse(FloorHeightWalker(current, bestUp, bestDown));

  if(up && bestUp != g_MaxWorldCoord)
  {
    current = bestUp;
  }
  if(!up && bestDown != -g_MaxWorldCoord)
  {
    current = bestDown;
  }

  m_Camera.origin[2] = current + 48;
  Camera_updateModelview(getCamera());
  CamWnd_Update(*this);
  CameraMovedNotify();
}


#if 0

// button_press
  Sys_GetCursorPos(&m_PositionDragCursorX, &m_PositionDragCursorY);

// motion
  if ( (m_bFreeMove && (buttons == (RAD_CONTROL|RAD_SHIFT)))
    || (!m_bFreeMove && (buttons == (RAD_RBUTTON|RAD_CONTROL))) )
  {
    Cam_PositionDrag();
    CamWnd_Update(camwnd);
    CameraMovedNotify();
    return;
  }

void CamWnd::Cam_PositionDrag()
{
  int x, y;

  Sys_GetCursorPos(GTK_WINDOW(m_gl_widget), &x, &y);
  if (x != m_PositionDragCursorX || y != m_PositionDragCursorY)
  {
    x -= m_PositionDragCursorX;
    vector3_add(m_Camera.origin, vector3_scaled(m_Camera.vright, x));
    y -= m_PositionDragCursorY;
    m_Camera.origin[2] -= y;
    Camera_updateModelview();
    CamWnd_Update(camwnd);
    CameraMovedNotify();

    Sys_SetCursorPos(GTK_WINDOW(m_parent), m_PositionDragCursorX, m_PositionDragCursorY);
  }
}
#endif


// NOTE TTimo if there's an OS-level focus out of the application
//   then we can release the camera cursor grab
static gboolean camwindow_freemove_focusout(GtkWidget* widget, GdkEventFocus* event, gpointer data)
{
  reinterpret_cast<CamWnd*>(data)->DisableFreeMove();
  return FALSE;
}

void CamWnd::EnableFreeMove()
{
#if 0
  globalOutputStream() << "EnableFreeMove\n";
#endif
  ASSERT_MESSAGE(!m_bFreeMove, "EnableFreeMove: free-move was already enabled");
  m_bFreeMove = true;
  Camera_clearMovementFlags(getCamera(), MOVE_ALL);

  CamWnd_Remove_Handlers_Move(*this);
  CamWnd_Add_Handlers_FreeMove(*this);

  gtk_window_set_focus(m_parent, m_gl_widget);
  m_freemove_handle_focusout = g_signal_connect(G_OBJECT(m_gl_widget), "focus_out_event", G_CALLBACK(camwindow_freemove_focusout), this);
  m_freezePointer.freeze_pointer(m_parent, Camera_motionDelta, &m_Camera);

  CamWnd_Update(*this);
}

void CamWnd::DisableFreeMove()
{
#if 0
  globalOutputStream() << "DisableFreeMove\n";
#endif
  ASSERT_MESSAGE(m_bFreeMove, "DisableFreeMove: free-move was not enabled");
  m_bFreeMove = false;
  Camera_clearMovementFlags(getCamera(), MOVE_ALL);

  CamWnd_Remove_Handlers_FreeMove(*this);
  CamWnd_Add_Handlers_Move(*this);

  m_freezePointer.unfreeze_pointer(m_parent);
  g_signal_handler_disconnect(G_OBJECT(m_gl_widget), m_freemove_handle_focusout);

  CamWnd_Update(*this);
}


#include "renderer.h"

class CamRenderer: public Renderer
{
  struct state_type
  {
    state_type() : m_highlight(0), m_state(0), m_lights(0)
    {
    }  
    unsigned int m_highlight;
    Shader* m_state;
    const LightList* m_lights;
  };

  std::vector<state_type> m_state_stack;
  RenderStateFlags m_globalstate;
  Shader* m_state_select0;
  Shader* m_state_select1;
  const Vector3& m_viewer;

public:
  CamRenderer(RenderStateFlags globalstate, Shader* select0, Shader* select1, const Vector3& viewer) :
    m_globalstate(globalstate),
    m_state_select0(select0),
    m_state_select1(select1),
    m_viewer(viewer)
  {
    ASSERT_NOTNULL(select0);
    ASSERT_NOTNULL(select1);
    m_state_stack.push_back(state_type());
  }

  void SetState(Shader* state, EStyle style)
  {
    ASSERT_NOTNULL(state);
    if(style == eFullMaterials)
    {
      m_state_stack.back().m_state = state;
    }
  }
  const EStyle getStyle() const
  {
    return eFullMaterials;
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
  void setLights(const LightList& lights)
  {
    m_state_stack.back().m_lights = &lights;
  }
  void addRenderable(const OpenGLRenderable& renderable, const Matrix4& world)
  {
    if(m_state_stack.back().m_highlight & ePrimitive)
    {
      m_state_select0->addRenderable(renderable, world, m_state_stack.back().m_lights);
    }
    if(m_state_stack.back().m_highlight & eFace)
    {
      m_state_select1->addRenderable(renderable, world, m_state_stack.back().m_lights);
    }

    m_state_stack.back().m_state->addRenderable(renderable, world, m_state_stack.back().m_lights);
  }

  void render(const Matrix4& modelview, const Matrix4& projection)
  {
    GlobalShaderCache().render(m_globalstate, modelview, projection, m_viewer);
  }
};

/*
==============
Cam_Draw
==============
*/

void CamWnd::Cam_Draw()
{
  glViewport(0, 0, m_Camera.width, m_Camera.height);
#if 0
  GLint viewprt[4];
  glGetIntegerv (GL_VIEWPORT, viewprt);
#endif

  // enable depth buffer writes
  glDepthMask(GL_TRUE);

  Vector3 clearColour(0, 0, 0);
  if(m_Camera.draw_mode != cd_lighting)
  {
    clearColour = g_camwindow_globals.color_cameraback;
  }

  glClearColor(clearColour[0], clearColour[1], clearColour[2], 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  extern void Renderer_ResetStats();
  Renderer_ResetStats();
  extern void Cull_ResetStats();
  Cull_ResetStats();

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(reinterpret_cast<const float*>(&m_Camera.projection));

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(reinterpret_cast<const float*>(&m_Camera.modelview));


  // one directional light source directly behind the viewer
  {
    GLfloat inverse_cam_dir[4], ambient[4], diffuse[4];//, material[4];

    ambient[0] = ambient[1] = ambient[2] = 0.4f;
    ambient[3] = 1.0f;
    diffuse[0] = diffuse[1] = diffuse[2] = 0.4f;
    diffuse[3] = 1.0f;
    //material[0] = material[1] = material[2] = 0.8f;
    //material[3] = 1.0f;
    
    inverse_cam_dir[0] = m_Camera.vpn[0];
    inverse_cam_dir[1] = m_Camera.vpn[1];
    inverse_cam_dir[2] = m_Camera.vpn[2];
    inverse_cam_dir[3] = 0;

    glLightfv(GL_LIGHT0, GL_POSITION, inverse_cam_dir);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glEnable(GL_LIGHT0);
  }


  unsigned int globalstate = RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_ALPHATEST|RENDER_BLEND|RENDER_CULLFACE|RENDER_COLOUR|RENDER_OFFSETLINE;
  switch (m_Camera.draw_mode)
  {
  case cd_wire:
    break;
  case cd_solid:
    globalstate |= RENDER_FILL
      | RENDER_LIGHTING
      | RENDER_SMOOTH
      | RENDER_SCALED;
    break;
  case cd_texture:
    globalstate |= RENDER_FILL
      | RENDER_LIGHTING
      | RENDER_TEXTURE
      | RENDER_SMOOTH
      | RENDER_SCALED;
    break;
  case cd_lighting:
    globalstate |= RENDER_FILL
      | RENDER_LIGHTING
      | RENDER_TEXTURE
      | RENDER_SMOOTH
      | RENDER_SCALED
      | RENDER_BUMP
      | RENDER_PROGRAM
      | RENDER_SCREEN;
    break;
  default:
    globalstate = 0;
    break;
  }

  if(!g_xywindow_globals.m_bNoStipple)
  {
    globalstate |= RENDER_LINESTIPPLE|RENDER_POLYGONSTIPPLE;
  }

  {
    CamRenderer renderer(globalstate, m_state_select2, m_state_select1, m_view.getViewer());

    Scene_Render(renderer, m_view);

    renderer.render(m_Camera.modelview, m_Camera.projection);
  }

  // prepare for 2d stuff
  glColor4f(1, 1, 1, 1);
  glDisable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, (float)m_Camera.width, 0, (float)m_Camera.height, -100, 100);
  glScalef(1, -1, 1);
  glTranslatef(0, -(float)m_Camera.height, 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(GlobalOpenGL().GL_1_3())
  {
    glClientActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0);
  }

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_DEPTH_TEST);
  glColor3f( 1.f, 1.f, 1.f );
  glLineWidth(1);

  // draw the crosshair
  if (m_bFreeMove)
  {
    glBegin( GL_LINES );
    glVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f + 6 );
    glVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f + 2 );
    glVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f - 6 );
    glVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f - 2 );
    glVertex2f( (float)m_Camera.width / 2.f + 6, (float)m_Camera.height / 2.f );
    glVertex2f( (float)m_Camera.width / 2.f + 2, (float)m_Camera.height / 2.f );
    glVertex2f( (float)m_Camera.width / 2.f - 6, (float)m_Camera.height / 2.f );
    glVertex2f( (float)m_Camera.width / 2.f - 2, (float)m_Camera.height / 2.f );
    glEnd();
  }

  glRasterPos3f(1.0f, static_cast<float>(m_Camera.height) - 1.0f, 0.0f);
  extern const char* Renderer_GetStats();
  GlobalOpenGL().drawString(Renderer_GetStats());

  glRasterPos3f(1.0f, static_cast<float>(m_Camera.height) - 11.0f, 0.0f);
  extern const char* Cull_GetStats();
  GlobalOpenGL().drawString(Cull_GetStats());

  // bind back to the default texture so that we don't have problems
  // elsewhere using/modifying texture maps between contexts
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void CamWnd::draw()
{
  m_drawing = true;

  //globalOutputStream() << "draw...\n";
  if (glwidget_make_current(m_gl_widget) != FALSE)
  {
    if(Map_Valid(g_map) && ScreenUpdates_Enabled())
    {
      GlobalOpenGL_debugAssertNoErrors();
      Cam_Draw();
      GlobalOpenGL_debugAssertNoErrors();
      //qglFinish();

      m_XORRectangle.set(rectangle_t());
    }

    glwidget_swap_buffers(m_gl_widget);
  }

  m_drawing = false;
}

void CamWnd::BenchMark()
{
  double dStart = Sys_DoubleTime();
  for (int i=0 ; i < 100 ; i++)
  {
    Vector3 angles;
    angles[CAMERA_ROLL] = 0;
    angles[CAMERA_PITCH] = 0;
    angles[CAMERA_YAW] = static_cast<float>(i * (360.0 / 100.0));
    Camera_setAngles(*this, angles);
  }
  double dEnd = Sys_DoubleTime();
  globalOutputStream() << FloatFormat(dEnd - dStart, 5, 2), " seconds\n";
}


void fill_view_camera_menu(GtkMenu* menu)
{
  create_check_menu_item_with_mnemonic(menu, "Camera View", "ToggleCamera");
}

void GlobalCamera_ResetAngles()
{
  CamWnd& camwnd = *g_camwnd;
  Vector3 angles;
  angles[CAMERA_ROLL] = angles[CAMERA_PITCH] = 0;
  angles[CAMERA_YAW] = static_cast<float>(22.5 * floor((Camera_getAngles(camwnd)[CAMERA_YAW]+11)/22.5));
  Camera_setAngles(camwnd, angles);
}

void Camera_ChangeFloorUp()
{
  CamWnd& camwnd = *g_camwnd;
  camwnd.Cam_ChangeFloor (true);
}

void Camera_ChangeFloorDown()
{
  CamWnd& camwnd = *g_camwnd;
  camwnd.Cam_ChangeFloor (false);
}

void Camera_CubeIn()
{
  CamWnd& camwnd = *g_camwnd;
  g_camwindow_globals.m_nCubicScale--;
  if (g_camwindow_globals.m_nCubicScale < 1)
    g_camwindow_globals.m_nCubicScale = 1;
  Camera_updateProjection(camwnd.getCamera());
  CamWnd_Update(camwnd);
  g_pParentWnd->SetGridStatus();
}

void Camera_CubeOut()
{
  CamWnd& camwnd = *g_camwnd;
  g_camwindow_globals.m_nCubicScale++;
  if (g_camwindow_globals.m_nCubicScale > 23)
    g_camwindow_globals.m_nCubicScale = 23;
  Camera_updateProjection(camwnd.getCamera());
  CamWnd_Update(camwnd);
  g_pParentWnd->SetGridStatus();
}

bool Camera_GetFarClip()
{
  return g_camwindow_globals_private.m_bCubicClipping;
}

BoolExportCaller g_getfarclip_caller(g_camwindow_globals_private.m_bCubicClipping);
ToggleItem g_getfarclip_item(g_getfarclip_caller);

void Camera_SetFarClip(bool value)
{
  CamWnd& camwnd = *g_camwnd;
  g_camwindow_globals_private.m_bCubicClipping = value;
  g_getfarclip_item.update();
  Camera_updateProjection(camwnd.getCamera());
  CamWnd_Update(camwnd);
}

void Camera_ToggleFarClip()
{
  Camera_SetFarClip(!Camera_GetFarClip());
}


void CamWnd_constructToolbar(GtkToolbar* toolbar)
{
  toolbar_append_toggle_button(toolbar, "Cubic clip the camera view", "view_cubicclipping.bmp", "ToggleCubicClip");
}

void CamWnd_registerShortcuts()
{
  toggle_add_accelerator("ToggleCubicClip");
  
  if(g_pGameDescription->mGameType == "doom3")
  {
    command_connect_accelerator("TogglePreview");
  }
}


void GlobalCamera_Benchmark()
{
  CamWnd& camwnd = *g_camwnd;
  camwnd.BenchMark();
}

void GlobalCamera_Update()
{
  CamWnd& camwnd = *g_camwnd;
  CamWnd_Update(camwnd);
}

camera_draw_mode CamWnd_GetMode()
{
  return camera_t::draw_mode;
}
void CamWnd_SetMode(camera_draw_mode mode)
{
  ShaderCache_setBumpEnabled(mode == cd_lighting);
  camera_t::draw_mode = mode;
  if(g_camwnd != 0)
  {
    CamWnd_Update(*g_camwnd);
  }
}

void CamWnd_TogglePreview(void)
{
  // gametype must be doom3 for this function to work
  // if the gametype is not doom3 something is wrong with the
  // global command list or somebody else calls this function.
  ASSERT_MESSAGE(g_pGameDescription->mGameType == "doom3", "CamWnd_TogglePreview called although mGameType is not doom3 compatible");

  // switch between textured and lighting mode
  CamWnd_SetMode((CamWnd_GetMode() == cd_lighting) ? cd_texture : cd_lighting);
}


CameraModel* g_camera_model = 0;

void CamWnd_LookThroughCamera(CamWnd& camwnd)
{
  if(g_camera_model != 0)
  {
    CamWnd_Add_Handlers_Move(camwnd);
    g_camera_model->setCameraView(0, Callback());
    g_camera_model = 0;
    Camera_updateModelview(camwnd.getCamera());
    Camera_updateProjection(camwnd.getCamera());
    CamWnd_Update(camwnd);
  }
}

inline CameraModel* Instance_getCameraModel(scene::Instance& instance)
{
  return InstanceTypeCast<CameraModel>::cast(instance);
}

void CamWnd_LookThroughSelected(CamWnd& camwnd)
{
  if(g_camera_model != 0)
  {
    CamWnd_LookThroughCamera(camwnd);
  }

  if(GlobalSelectionSystem().countSelected() != 0)
  {
    scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
    CameraModel* cameraModel = Instance_getCameraModel(instance);
    if(cameraModel != 0)
    {
      CamWnd_Remove_Handlers_Move(camwnd);
      g_camera_model = cameraModel;
      g_camera_model->setCameraView(&camwnd.getCameraView(), ReferenceCaller<CamWnd, CamWnd_LookThroughCamera>(camwnd));
    }
  }
}

void GlobalCamera_LookThroughSelected()
{
  CamWnd_LookThroughSelected(*g_camwnd);
}

void GlobalCamera_LookThroughCamera()
{
  CamWnd_LookThroughCamera(*g_camwnd);
}


void RenderModeImport(int value)
{
  switch(value)
  {
  case 0:
    CamWnd_SetMode(cd_wire);
    break;
  case 1:
    CamWnd_SetMode(cd_solid);
    break;
  case 2:
    CamWnd_SetMode(cd_texture);
    break;
  case 3:
    CamWnd_SetMode(cd_lighting);
    break;
  default:
    CamWnd_SetMode(cd_texture);
  }
}
typedef FreeCaller1<int, RenderModeImport> RenderModeImportCaller;

void RenderModeExport(const IntImportCallback& importer)
{
  switch(CamWnd_GetMode())
  {
  case cd_wire:
    importer(0);
    break;
  case cd_solid:
    importer(1);
    break;
  case cd_texture:
    importer(2);
    break;
  case cd_lighting:
    importer(3);
    break;
  }
}
typedef FreeCaller1<const IntImportCallback&, RenderModeExport> RenderModeExportCaller;

void Camera_constructPreferences(PreferencesPage& page)
{
  page.appendSlider("Movement Speed", g_camwindow_globals_private.m_nMoveSpeed, TRUE, 0, 0, 100, 50, 300, 1, 10, 10);
  page.appendSlider("Rotation Speed", g_camwindow_globals_private.m_nAngleSpeed, TRUE, 0, 0, 3, 1, 180, 1, 10, 10);
  page.appendCheckBox("", "Invert mouse vertical axis", g_camwindow_globals_private.m_bCamInverseMouse);
  page.appendCheckBox(
    "", "Discrete movement",
    FreeCaller1<bool, CamWnd_Move_Discrete_Import>(),
    BoolExportCaller(g_camwindow_globals_private.m_bCamDiscrete)
  );
  page.appendCheckBox(
    "", "Enable far-clip plane",
    FreeCaller1<bool, Camera_SetFarClip>(),
    BoolExportCaller(g_camwindow_globals_private.m_bCubicClipping)
  );

  if(g_pGameDescription->mGameType == "doom3")
  {
    const char* render_mode[] = { "Wireframe", "Flatshade", "Textured", "Lighting" };

    page.appendCombo(
      "Render Mode",
      STRING_ARRAY_RANGE(render_mode),
      IntImportCallback(RenderModeImportCaller()),
      IntExportCallback(RenderModeExportCaller())
    );
  }
  else
  {
    const char* render_mode[] = { "Wireframe", "Flatshade", "Textured" };

    page.appendCombo(
      "Render Mode",
      STRING_ARRAY_RANGE(render_mode),
      IntImportCallback(RenderModeImportCaller()),
      IntExportCallback(RenderModeExportCaller())
    );
  }
}
void Camera_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Camera", "Camera View Preferences"));
  Camera_constructPreferences(page);
}
void Camera_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Camera_constructPage>());
}

#include "preferencesystem.h"
#include "stringio.h"
#include "dialog.h"

typedef FreeCaller1<bool, CamWnd_Move_Discrete_Import> CamWndMoveDiscreteImportCaller;

/// \brief Initialisation for things that have the same lifespan as this module.
void CamWnd_Construct()
{
  GlobalCommands_insert("CenterView", FreeCaller<GlobalCamera_ResetAngles>(), Accelerator(GDK_End));

  GlobalToggles_insert("ToggleCubicClip", FreeCaller<Camera_ToggleFarClip>(), ToggleItem::AddCallbackCaller(g_getfarclip_item), Accelerator('\\', (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("CubicClipZoomIn", FreeCaller<Camera_CubeIn>(), Accelerator('[', (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("CubicClipZoomOut", FreeCaller<Camera_CubeOut>(), Accelerator(']', (GdkModifierType)GDK_CONTROL_MASK));

  GlobalCommands_insert("UpFloor", FreeCaller<Camera_ChangeFloorUp>(), Accelerator(GDK_Prior));
  GlobalCommands_insert("DownFloor", FreeCaller<Camera_ChangeFloorDown>(), Accelerator(GDK_Next));

  GlobalToggles_insert("ToggleCamera", ToggleShown::ToggleCaller(g_camera_shown), ToggleItem::AddCallbackCaller(g_camera_shown.m_item), Accelerator('C', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalCommands_insert("LookThroughSelected", FreeCaller<GlobalCamera_LookThroughSelected>());
  GlobalCommands_insert("LookThroughCamera", FreeCaller<GlobalCamera_LookThroughCamera>());

  if(g_pGameDescription->mGameType == "doom3")
  {
    GlobalCommands_insert("TogglePreview", FreeCaller<CamWnd_TogglePreview>(), Accelerator(GDK_F3));
  }

  GlobalShortcuts_insert("CameraForward", Accelerator(GDK_Up));
  GlobalShortcuts_insert("CameraBack", Accelerator(GDK_Down));
  GlobalShortcuts_insert("CameraLeft", Accelerator(GDK_Left));
  GlobalShortcuts_insert("CameraRight", Accelerator(GDK_Right));
  GlobalShortcuts_insert("CameraStrafeRight", Accelerator(GDK_period));
  GlobalShortcuts_insert("CameraStrafeLeft", Accelerator(GDK_comma));

  GlobalShortcuts_insert("CameraUp", Accelerator('D'));
  GlobalShortcuts_insert("CameraDown", Accelerator('C'));
  GlobalShortcuts_insert("CameraAngleUp", Accelerator('A'));
  GlobalShortcuts_insert("CameraAngleDown", Accelerator('Z'));

  GlobalShortcuts_insert("CameraFreeMoveForward", Accelerator(GDK_Up));
  GlobalShortcuts_insert("CameraFreeMoveBack", Accelerator(GDK_Down));
  GlobalShortcuts_insert("CameraFreeMoveLeft", Accelerator(GDK_Left));
  GlobalShortcuts_insert("CameraFreeMoveRight", Accelerator(GDK_Right));

  GlobalPreferenceSystem().registerPreference("MoveSpeed", IntImportStringCaller(g_camwindow_globals_private.m_nMoveSpeed), IntExportStringCaller(g_camwindow_globals_private.m_nMoveSpeed));
  GlobalPreferenceSystem().registerPreference("AngleSpeed", IntImportStringCaller(g_camwindow_globals_private.m_nAngleSpeed), IntExportStringCaller(g_camwindow_globals_private.m_nAngleSpeed));
  GlobalPreferenceSystem().registerPreference("CamInverseMouse", BoolImportStringCaller(g_camwindow_globals_private.m_bCamInverseMouse), BoolExportStringCaller(g_camwindow_globals_private.m_bCamInverseMouse));
  GlobalPreferenceSystem().registerPreference("CamDiscrete", makeBoolStringImportCallback(CamWndMoveDiscreteImportCaller()), BoolExportStringCaller(g_camwindow_globals_private.m_bCamDiscrete));
  GlobalPreferenceSystem().registerPreference("CubicClipping", BoolImportStringCaller(g_camwindow_globals_private.m_bCubicClipping), BoolExportStringCaller(g_camwindow_globals_private.m_bCubicClipping));
  GlobalPreferenceSystem().registerPreference("CubicScale", IntImportStringCaller(g_camwindow_globals.m_nCubicScale), IntExportStringCaller(g_camwindow_globals.m_nCubicScale));
  GlobalPreferenceSystem().registerPreference("SI_Colors4", Vector3ImportStringCaller(g_camwindow_globals.color_cameraback), Vector3ExportStringCaller(g_camwindow_globals.color_cameraback));
  GlobalPreferenceSystem().registerPreference("SI_Colors12", Vector3ImportStringCaller(g_camwindow_globals.color_selbrushes3d), Vector3ExportStringCaller(g_camwindow_globals.color_selbrushes3d));
  GlobalPreferenceSystem().registerPreference("CameraRenderMode", makeIntStringImportCallback(RenderModeImportCaller()), makeIntStringExportCallback(RenderModeExportCaller()));

  CamWnd_constructStatic();

  Camera_registerPreferencesPage();
}
void CamWnd_Destroy()
{
  CamWnd_destroyStatic();
}
