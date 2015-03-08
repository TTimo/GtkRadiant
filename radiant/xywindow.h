/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

#ifndef _XYWINDOW_H_
#define _XYWINDOW_H_

#include "qe3.h"
#include "camwindow.h"
#include "glwindow.h"

const int SCALE_X = 0x01;
const int SCALE_Y = 0x02;
const int SCALE_Z = 0x04;

typedef void ( PFNPathCallback )( bool, int );
// as i didn't really encapsulate anything this
// should really be a struct..
class ClipPoint
{
public:
ClipPoint(){ Reset(); };
void Reset(){ m_ptClip[0] = m_ptClip[1] = m_ptClip[2] = 0.0; m_bSet = false; m_pVec3 = NULL; };
bool Set(){ return m_bSet; };
void Set( bool b ) { m_bSet = b; };
void UpdatePointPtr() {
	if ( m_pVec3 ) {
		VectorCopy( m_ptClip, *m_pVec3 );
	}
};
void SetPointPtr( vec3_t* p ) { m_pVec3 = p; };
vec3_t m_ptClip;        // the 3d point
vec3_t* m_pVec3;        // optional ptr for 3rd party updates
int m_ptScreenX, m_ptScreenY;      // the onscreen xy point (for mousability)
bool m_bSet;
operator vec3_t&() {
	return m_ptClip;
};
operator vec3_t*() {
	return &m_ptClip;
};

/*! Draw clip/path point with rasterized number label */
void Draw( float fScale, int num );
/*! Draw clip/path point with rasterized string label */
void Draw( float fScale, const char *label );
};

class XYWnd : public GLWindow
{
public:
XYWnd();
virtual ~XYWnd() { }

public:
bool AreaSelectOK();
vec3_t& RotateOrigin();
vec3_t& Rotation();
void UndoClear();
bool UndoAvailable();
void KillPathMode();
void Undo();
void UndoCopy();
void Copy();
void Paste();
void Redraw( unsigned int nBits );
void VectorCopyXY( vec3_t in, vec3_t out );
void PositionView();
void FlipClip();
void SplitClip();
void Clip();
vec3_t& GetOrigin();
void SetOrigin( vec3_t org );       // PGM
void XY_Init();
void XY_Overlay();
void XY_Draw();
void DrawZIcon();
void DrawRotateIcon();
void DrawCameraIcon();
void XY_DrawBlockGrid();
void XY_DrawGrid();
void XY_MouseMoved( int x, int y, int buttons );
// TTimo: FIXME: was experimental stuff to track possible endless loop issues
//  void XY_MouseMovedRec (int x, int y, int buttons);
void NewBrushDrag( int x, int y );
qboolean DragDelta( int x, int y, vec3_t move );
void XY_MouseUp( int x, int y, int buttons );
void XY_MouseDown( int x, int y, int buttons );
void XY_ToGridPoint( int x, int y, vec3_t point );
void XY_ToPoint( int x, int y, vec3_t point );
void SnapToPoint( int x, int y, vec3_t point );
void SetActive( bool b ) {m_bActive = b; };
bool Active() {return m_bActive; };

void DropClipPoint( guint32 nFlags, int pointx, int pointy );
bool RogueClipMode();
bool ClipMode();
void SetClipMode( bool bMode );
void RetainClipMode( bool bMode );

bool RotateMode();
bool SetRotateMode( bool bMode );
bool ScaleMode();
void SetScaleMode( bool bMode );

bool PathMode();
void DropPathPoint( guint32 nFlags, int pointx, int pointy );
bool PointMode();
//  void AddPointPoint(guint32 nFlags, vec3_t* pVec);
void SetPointMode( bool b );

void SetViewType( int n );
bool m_bActive;

protected:
int m_nUpdateBits;
int m_nWidth;
int m_nHeight;
bool m_bTiming;
float m_fScale;
float m_TopClip;
float m_BottomClip;
bool m_bDirty;
vec3_t m_vOrigin;

int m_ptCursorX, m_ptCursorY;
bool m_bRButtonDown;

int m_nButtonstate;
int m_nPressx;
int m_nPressy;
vec3_t m_vPressdelta;
bool m_bPress_selection;

friend class CamWnd;

private:
// this is unique for all views
static GtkWidget* m_mnuDrop;

int m_nViewType;

int m_nScrollFlags;
int m_ptDragX, m_ptDragY;
int m_ptDragAdjX, m_ptDragAdjY;
int m_ptDragTotalX, m_ptDragTotalY;

void OriginalButtonUp( guint32 nFlags, int point, int pointy );
void OriginalButtonDown( guint32 nFlags, int point, int pointy );
//  void ProduceSplits(brush_t** pFront, brush_t** pBack);
void PlanePointsFromClipPoints( vec3_t planepts[3], brush_t * pBrush );
void ProduceSplitLists();
void HandleDrop();
void PaintSizeInfo( int nDim1, int nDim2, vec3_t vMinBounds, vec3_t vMaxBounds );

int m_ptDownX, m_ptDownY;

public:
void OnEntityCreate( const char* item );
int GetViewType() {return m_nViewType; }
void SetScale( float f ) {m_fScale = f; }
float Scale() {return m_fScale; }
int Width() {return m_nWidth; }
int Height() {return m_nHeight; }

protected:

void OnCreate();
void OnExpose();
void OnLButtonDown( guint32 flags, int pointx, int pointy );
void OnRButtonDown( guint32 flags, int pointx, int pointy );
void OnMButtonDown( guint32 flags, int pointx, int pointy );
void OnLButtonUp( guint32 flags, int pointx, int pointy );
void OnRButtonUp( guint32 flags, int pointx, int pointy );
void OnMButtonUp( guint32 flags, int pointx, int pointy );
void OnMouseMove( guint32 nFlags, int pointx, int pointy );
void OnMouseWheel( bool bUp, int pointx, int pointy );
void OnSize( int cx, int cy );
void OnTimer();

private:
XORRectangle m_XORRectangle;
};

#endif // _XYWINDOW_H_
