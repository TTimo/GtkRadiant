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

#ifndef _GLWINDOW_H_
#define _GLWINDOW_H_

class GLWindow {
public:
  GLWindow( bool zbuffer );
  virtual ~GLWindow ();

  bool MakeCurrent();
  void SwapBuffers();
  void SetTimer( guint millisec );
  void KillTimer();
  bool HasTimer() { return m_nTimer != 0; }
  void DestroyContext();
  void CreateContext();

  virtual void OnCreate() { }
  virtual void OnExpose() { }

  virtual void OnLButtonDown( guint32 flags, int x, int y ) { }
  virtual void OnRButtonDown( guint32 flags, int x, int y ) { }
  virtual void OnMButtonDown( guint32 flags, int x, int y ) { }
  virtual void OnLButtonUp( guint32 flags, int pointx, int pointy ) { }
  virtual void OnRButtonUp( guint32 flags, int pointx, int pointy ) { }
  virtual void OnMButtonUp( guint32 flags, int pointx, int pointy ) { }
  virtual void OnMouseMove( guint32 flags, int pointx, int pointy ) { }

  virtual void OnSize( int cx, int cy ) { }
  virtual void OnTimer() { }

  virtual void OnMouseWheel( bool bUp, int pointx, int pointy ) { }

  void RedrawWindow() {	gtk_widget_queue_draw( m_pWidget ); }

  void SetFocus() { /* gdk_window_raise (m_pWidget->window); */ }

  void SetCapture() { m_bMouseCapture = TRUE; }
  void ReleaseCapture() { m_bMouseCapture = FALSE; }

  bool HasCapture() { return m_bMouseCapture; }

  GtkWidget * GetWidget() { return m_pWidget; }

// member variables
public:
  GtkWidget* m_pParent;   // for floating windows only

protected:
  bool m_bMouseCapture;
  GtkWidget* m_pWidget;

private:
  guint m_nTimer;        // only one timer supported
};

#endif //_GLWINDOW_H_
