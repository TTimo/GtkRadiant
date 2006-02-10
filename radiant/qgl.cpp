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


#include "qgl.h"

#include "debugging/debugging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WINGDIAPI __declspec(dllimport)
#define APIENTRY __stdcall
#endif

#include <GL/gl.h>

#if defined(_WIN32)
#undef WINGDIAPI
#undef APIENTRY
#endif

#include "igl.h"




#if defined(_WIN32)

#include <wtypes.h>

int   ( WINAPI * qwglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR *);
int   ( WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
int   ( WINAPI * qwglGetPixelFormat)(HDC);
BOOL  ( WINAPI * qwglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
BOOL  ( WINAPI * qwglSwapBuffers)(HDC);

BOOL  ( WINAPI * qwglCopyContext)(HGLRC, HGLRC, UINT);
HGLRC ( WINAPI * qwglCreateContext)(HDC);
HGLRC ( WINAPI * qwglCreateLayerContext)(HDC, int);
BOOL  ( WINAPI * qwglDeleteContext)(HGLRC);
HGLRC ( WINAPI * qwglGetCurrentContext)(VOID);
HDC   ( WINAPI * qwglGetCurrentDC)(VOID);
PROC  ( WINAPI * qwglGetProcAddress)(LPCSTR);
BOOL  ( WINAPI * qwglMakeCurrent)(HDC, HGLRC);
BOOL  ( WINAPI * qwglShareLists)(HGLRC, HGLRC);
BOOL  ( WINAPI * qwglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

BOOL  ( WINAPI * qwglUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT,
				      FLOAT, int, LPGLYPHMETRICSFLOAT);

BOOL ( WINAPI * qwglDescribeLayerPlane)(HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
int  ( WINAPI * qwglSetLayerPaletteEntries)(HDC, int, int, int, CONST COLORREF *);
int  ( WINAPI * qwglGetLayerPaletteEntries)(HDC, int, int, int, COLORREF *);
BOOL ( WINAPI * qwglRealizeLayerPalette)(HDC, int, BOOL);
BOOL ( WINAPI * qwglSwapLayerBuffers)(HDC, UINT);

#elif defined (__linux__) || defined (__APPLE__)

#include <GL/glx.h>
#include <dlfcn.h>
#include <gdk/gdkx.h>

XVisualInfo* (*qglXChooseVisual)(Display *dpy, int screen, int *attribList);
GLXContext   (*qglXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
void         (*qglXDestroyContext)(Display *dpy, GLXContext ctx);
Bool         (*qglXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
void         (*qglXCopyContext)(Display *dpy, GLXContext src, GLXContext dst, GLuint mask);
void         (*qglXSwapBuffers)( Display *dpy, GLXDrawable drawable );
GLXPixmap    (*qglXCreateGLXPixmap)( Display *dpy, XVisualInfo *visual, Pixmap pixmap );
void         (*qglXDestroyGLXPixmap)( Display *dpy, GLXPixmap pixmap );
Bool         (*qglXQueryExtension)( Display *dpy, int *errorb, int *event );
Bool         (*qglXQueryVersion)( Display *dpy, int *maj, int *min );
Bool         (*qglXIsDirect)( Display *dpy, GLXContext ctx );
int          (*qglXGetConfig)( Display *dpy, XVisualInfo *visual, int attrib, int *value );
GLXContext   (*qglXGetCurrentContext)( void );
GLXDrawable  (*qglXGetCurrentDrawable)( void );
void         (*qglXWaitGL)( void );
void         (*qglXWaitX)( void );
void         (*qglXUseXFont)( Font font, int first, int count, int list );
void*        (*qglXGetProcAddressARB) (const GLubyte *procName);
typedef void* (*glXGetProcAddressARBProc) (const GLubyte *procName);

#endif


void QGL_Shutdown(OpenGLBinding& table)
{
  globalOutputStream() << "Shutting down OpenGL module...";

#ifdef WIN32
  qwglCopyContext              = 0;
  qwglCreateContext            = 0;
  qwglCreateLayerContext       = 0;
  qwglDeleteContext            = 0;
  qwglDescribeLayerPlane       = 0;
  qwglGetCurrentContext        = 0;
  qwglGetCurrentDC             = 0;
  qwglGetLayerPaletteEntries   = 0;
  qwglGetProcAddress           = 0;
  qwglMakeCurrent              = 0;
  qwglRealizeLayerPalette      = 0;
  qwglSetLayerPaletteEntries   = 0;
  qwglShareLists               = 0;
  qwglSwapLayerBuffers         = 0;
  qwglUseFontBitmaps           = 0;
  qwglUseFontOutlines          = 0;

  qwglChoosePixelFormat        = 0;
  qwglDescribePixelFormat      = 0;
  qwglGetPixelFormat           = 0;
  qwglSetPixelFormat           = 0;
  qwglSwapBuffers              = 0;
#endif

#if defined (__linux__) || defined (__APPLE__)
  qglXChooseVisual             = 0;
  qglXCreateContext            = 0;
  qglXDestroyContext           = 0;
  qglXMakeCurrent              = 0;
  qglXCopyContext              = 0;
  qglXSwapBuffers              = 0;
  qglXCreateGLXPixmap          = 0;
  qglXDestroyGLXPixmap         = 0;
  qglXQueryExtension           = 0;
  qglXQueryVersion             = 0;
  qglXIsDirect                 = 0;
  qglXGetConfig                = 0;
  qglXGetCurrentContext        = 0;
  qglXGetCurrentDrawable       = 0;
  qglXWaitGL                   = 0;
  qglXWaitX                    = 0;
  qglXUseXFont                 = 0;
  qglXGetProcAddressARB        = 0;
#endif

  globalOutputStream() << "Done.\n";
}


typedef struct glu_error_struct
{
    GLenum     errnum;
    const char *errstr;
} GLU_ERROR_STRUCT;

GLU_ERROR_STRUCT glu_errlist[] = {
  {GL_NO_ERROR, "GL_NO_ERROR - no error"},
  {GL_INVALID_ENUM, "GL_INVALID_ENUM - An unacceptable value is specified for an enumerated argument."},
  {GL_INVALID_VALUE, "GL_INVALID_VALUE - A numeric argument is out of range."},
  {GL_INVALID_OPERATION, "GL_INVALID_OPERATION - The specified operation is not allowed in the current state."},
  {GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW - Function would cause a stack overflow."},
  {GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW - Function would cause a stack underflow."},
  {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY - There is not enough memory left to execute the function."},
  {0, 0}
};

const GLubyte* qgluErrorString(GLenum errCode )
{
  int search = 0;
  for (search = 0; glu_errlist[search].errstr; search++)
  {
    if (errCode == glu_errlist[search].errnum)
      return (const GLubyte *)glu_errlist[search].errstr;
  } //end for
  return (const GLubyte *)"Unknown error";
}


void glInvalidFunction()
{
  ERROR_MESSAGE("calling an invalid OpenGL function");
}

#define EXTENSIONS_ENABLED 1

bool QGL_ExtensionSupported(const char* extension)
{
#if EXTENSIONS_ENABLED
  const GLubyte *extensions = 0;
  const GLubyte *start;
  GLubyte *where, *terminator;

  // Extension names should not have spaces.
  where = (GLubyte *) strchr (extension, ' ');
  if (where || *extension == '\0')
    return false;

  extensions = GlobalOpenGL().m_glGetString (GL_EXTENSIONS);
#ifndef __APPLE__
  if (!extensions)
    return false;
#endif

  // It takes a bit of care to be fool-proof about parsing the
  // OpenGL extensions string. Don't be fooled by sub-strings, etc.
  for (start = extensions; ;)
  {
    where = (GLubyte *) strstr ((const char *) start, extension);
    if (!where)
      break;

    terminator = where + strlen (extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return true;

    start = terminator;
  }
#endif

  return false;
}

typedef int (QGL_DLLEXPORT *QGLFunctionPointer)();

QGLFunctionPointer QGL_getExtensionFunc(const char* symbol)
{
#if defined (__linux__) || defined (__APPLE__)
  //ASSERT_NOTNULL(qglXGetProcAddressARB);
  if (qglXGetProcAddressARB == 0)
  {
    return reinterpret_cast<QGLFunctionPointer>(glInvalidFunction);
  }
  else
  {
    return (QGLFunctionPointer)qglXGetProcAddressARB(reinterpret_cast<const GLubyte*>(symbol));
  }
#else
  ASSERT_NOTNULL(qwglGetProcAddress);
  return qwglGetProcAddress(symbol);
#endif
}


template<typename Func>
bool QGL_constructExtensionFunc(Func& func, const char* symbol)
{
  func = reinterpret_cast<Func>(QGL_getExtensionFunc(symbol));
  return func != 0;
}

template<typename Func>
void QGL_invalidateExtensionFunc(Func& func)
{
  func = reinterpret_cast<Func>(glInvalidFunction);
}

void QGL_clear(OpenGLBinding& table)
{
  QGL_invalidateExtensionFunc(table.m_glAccum);
  QGL_invalidateExtensionFunc(table.m_glAlphaFunc);
  QGL_invalidateExtensionFunc(table.m_glAreTexturesResident);
  QGL_invalidateExtensionFunc(table.m_glArrayElement);
  QGL_invalidateExtensionFunc(table.m_glBegin);
  QGL_invalidateExtensionFunc(table.m_glBindTexture);
  QGL_invalidateExtensionFunc(table.m_glBitmap);
  QGL_invalidateExtensionFunc(table.m_glBlendFunc);
  QGL_invalidateExtensionFunc(table.m_glCallList);
  QGL_invalidateExtensionFunc(table.m_glCallLists);
  QGL_invalidateExtensionFunc(table.m_glClear);
  QGL_invalidateExtensionFunc(table.m_glClearAccum);
  QGL_invalidateExtensionFunc(table.m_glClearColor);
  QGL_invalidateExtensionFunc(table.m_glClearDepth);
  QGL_invalidateExtensionFunc(table.m_glClearIndex);
  QGL_invalidateExtensionFunc(table.m_glClearStencil);
  QGL_invalidateExtensionFunc(table.m_glClipPlane);
  QGL_invalidateExtensionFunc(table.m_glColor3b);
  QGL_invalidateExtensionFunc(table.m_glColor3bv);
  QGL_invalidateExtensionFunc(table.m_glColor3d);
  QGL_invalidateExtensionFunc(table.m_glColor3dv);
  QGL_invalidateExtensionFunc(table.m_glColor3f);
  QGL_invalidateExtensionFunc(table.m_glColor3fv);
  QGL_invalidateExtensionFunc(table.m_glColor3i);
  QGL_invalidateExtensionFunc(table.m_glColor3iv);
  QGL_invalidateExtensionFunc(table.m_glColor3s);
  QGL_invalidateExtensionFunc(table.m_glColor3sv);
  QGL_invalidateExtensionFunc(table.m_glColor3ub);
  QGL_invalidateExtensionFunc(table.m_glColor3ubv);
  QGL_invalidateExtensionFunc(table.m_glColor3ui);
  QGL_invalidateExtensionFunc(table.m_glColor3uiv);
  QGL_invalidateExtensionFunc(table.m_glColor3us);
  QGL_invalidateExtensionFunc(table.m_glColor3usv);
  QGL_invalidateExtensionFunc(table.m_glColor4b);
  QGL_invalidateExtensionFunc(table.m_glColor4bv);
  QGL_invalidateExtensionFunc(table.m_glColor4d);
  QGL_invalidateExtensionFunc(table.m_glColor4dv);
  QGL_invalidateExtensionFunc(table.m_glColor4f);
  QGL_invalidateExtensionFunc(table.m_glColor4fv);
  QGL_invalidateExtensionFunc(table.m_glColor4i);
  QGL_invalidateExtensionFunc(table.m_glColor4iv);
  QGL_invalidateExtensionFunc(table.m_glColor4s);
  QGL_invalidateExtensionFunc(table.m_glColor4sv);
  QGL_invalidateExtensionFunc(table.m_glColor4ub);
  QGL_invalidateExtensionFunc(table.m_glColor4ubv);
  QGL_invalidateExtensionFunc(table.m_glColor4ui);
  QGL_invalidateExtensionFunc(table.m_glColor4uiv);
  QGL_invalidateExtensionFunc(table.m_glColor4us);
  QGL_invalidateExtensionFunc(table.m_glColor4usv);
  QGL_invalidateExtensionFunc(table.m_glColorMask);
  QGL_invalidateExtensionFunc(table.m_glColorMaterial);
  QGL_invalidateExtensionFunc(table.m_glColorPointer);
  QGL_invalidateExtensionFunc(table.m_glCopyPixels);
  QGL_invalidateExtensionFunc(table.m_glCopyTexImage1D);
  QGL_invalidateExtensionFunc(table.m_glCopyTexImage2D);
  QGL_invalidateExtensionFunc(table.m_glCopyTexSubImage1D);
  QGL_invalidateExtensionFunc(table.m_glCopyTexSubImage2D);
  QGL_invalidateExtensionFunc(table.m_glCullFace);
  QGL_invalidateExtensionFunc(table.m_glDeleteLists);
  QGL_invalidateExtensionFunc(table.m_glDeleteTextures);
  QGL_invalidateExtensionFunc(table.m_glDepthFunc);
  QGL_invalidateExtensionFunc(table.m_glDepthMask);
  QGL_invalidateExtensionFunc(table.m_glDepthRange);
  QGL_invalidateExtensionFunc(table.m_glDisable);
  QGL_invalidateExtensionFunc(table.m_glDisableClientState);
  QGL_invalidateExtensionFunc(table.m_glDrawArrays);
  QGL_invalidateExtensionFunc(table.m_glDrawBuffer);
  QGL_invalidateExtensionFunc(table.m_glDrawElements);
  QGL_invalidateExtensionFunc(table.m_glDrawPixels);
  QGL_invalidateExtensionFunc(table.m_glEdgeFlag);
  QGL_invalidateExtensionFunc(table.m_glEdgeFlagPointer);
  QGL_invalidateExtensionFunc(table.m_glEdgeFlagv);
  QGL_invalidateExtensionFunc(table.m_glEnable);
  QGL_invalidateExtensionFunc(table.m_glEnableClientState);
  QGL_invalidateExtensionFunc(table.m_glEnd);
  QGL_invalidateExtensionFunc(table.m_glEndList);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord1d);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord1dv);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord1f);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord1fv);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord2d);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord2dv);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord2f);
  QGL_invalidateExtensionFunc(table.m_glEvalCoord2fv);
  QGL_invalidateExtensionFunc(table.m_glEvalMesh1);
  QGL_invalidateExtensionFunc(table.m_glEvalMesh2);
  QGL_invalidateExtensionFunc(table.m_glEvalPoint1);
  QGL_invalidateExtensionFunc(table.m_glEvalPoint2);
  QGL_invalidateExtensionFunc(table.m_glFeedbackBuffer);
  QGL_invalidateExtensionFunc(table.m_glFinish);
  QGL_invalidateExtensionFunc(table.m_glFlush);
  QGL_invalidateExtensionFunc(table.m_glFogf);
  QGL_invalidateExtensionFunc(table.m_glFogfv);
  QGL_invalidateExtensionFunc(table.m_glFogi);
  QGL_invalidateExtensionFunc(table.m_glFogiv);
  QGL_invalidateExtensionFunc(table.m_glFrontFace);
  QGL_invalidateExtensionFunc(table.m_glFrustum);
  QGL_invalidateExtensionFunc(table.m_glGenLists);
  QGL_invalidateExtensionFunc(table.m_glGenTextures);
  QGL_invalidateExtensionFunc(table.m_glGetBooleanv);
  QGL_invalidateExtensionFunc(table.m_glGetClipPlane);
  QGL_invalidateExtensionFunc(table.m_glGetDoublev);
  QGL_invalidateExtensionFunc(table.m_glGetError);
  QGL_invalidateExtensionFunc(table.m_glGetFloatv);
  QGL_invalidateExtensionFunc(table.m_glGetIntegerv);
  QGL_invalidateExtensionFunc(table.m_glGetLightfv);
  QGL_invalidateExtensionFunc(table.m_glGetLightiv);
  QGL_invalidateExtensionFunc(table.m_glGetMapdv);
  QGL_invalidateExtensionFunc(table.m_glGetMapfv);
  QGL_invalidateExtensionFunc(table.m_glGetMapiv);
  QGL_invalidateExtensionFunc(table.m_glGetMaterialfv);
  QGL_invalidateExtensionFunc(table.m_glGetMaterialiv);
  QGL_invalidateExtensionFunc(table.m_glGetPixelMapfv);
  QGL_invalidateExtensionFunc(table.m_glGetPixelMapuiv);
  QGL_invalidateExtensionFunc(table.m_glGetPixelMapusv);
  QGL_invalidateExtensionFunc(table.m_glGetPointerv);
  QGL_invalidateExtensionFunc(table.m_glGetPolygonStipple);
  table.m_glGetString = glGetString;
  QGL_invalidateExtensionFunc(table.m_glGetTexEnvfv);
  QGL_invalidateExtensionFunc(table.m_glGetTexEnviv);
  QGL_invalidateExtensionFunc(table.m_glGetTexGendv);
  QGL_invalidateExtensionFunc(table.m_glGetTexGenfv);
  QGL_invalidateExtensionFunc(table.m_glGetTexGeniv);
  QGL_invalidateExtensionFunc(table.m_glGetTexImage);
  QGL_invalidateExtensionFunc(table.m_glGetTexLevelParameterfv);
  QGL_invalidateExtensionFunc(table.m_glGetTexLevelParameteriv);
  QGL_invalidateExtensionFunc(table.m_glGetTexParameterfv);
  QGL_invalidateExtensionFunc(table.m_glGetTexParameteriv);
  QGL_invalidateExtensionFunc(table.m_glHint);
  QGL_invalidateExtensionFunc(table.m_glIndexMask);
  QGL_invalidateExtensionFunc(table.m_glIndexPointer);
  QGL_invalidateExtensionFunc(table.m_glIndexd);
  QGL_invalidateExtensionFunc(table.m_glIndexdv);
  QGL_invalidateExtensionFunc(table.m_glIndexf);
  QGL_invalidateExtensionFunc(table.m_glIndexfv);
  QGL_invalidateExtensionFunc(table.m_glIndexi);
  QGL_invalidateExtensionFunc(table.m_glIndexiv);
  QGL_invalidateExtensionFunc(table.m_glIndexs);
  QGL_invalidateExtensionFunc(table.m_glIndexsv);
  QGL_invalidateExtensionFunc(table.m_glIndexub);
  QGL_invalidateExtensionFunc(table.m_glIndexubv);
  QGL_invalidateExtensionFunc(table.m_glInitNames);
  QGL_invalidateExtensionFunc(table.m_glInterleavedArrays);
  QGL_invalidateExtensionFunc(table.m_glIsEnabled);
  QGL_invalidateExtensionFunc(table.m_glIsList);
  QGL_invalidateExtensionFunc(table.m_glIsTexture);
  QGL_invalidateExtensionFunc(table.m_glLightModelf);
  QGL_invalidateExtensionFunc(table.m_glLightModelfv);
  QGL_invalidateExtensionFunc(table.m_glLightModeli);
  QGL_invalidateExtensionFunc(table.m_glLightModeliv);
  QGL_invalidateExtensionFunc(table.m_glLightf);
  QGL_invalidateExtensionFunc(table.m_glLightfv);
  QGL_invalidateExtensionFunc(table.m_glLighti);
  QGL_invalidateExtensionFunc(table.m_glLightiv);
  QGL_invalidateExtensionFunc(table.m_glLineStipple);
  QGL_invalidateExtensionFunc(table.m_glLineWidth);
  QGL_invalidateExtensionFunc(table.m_glListBase);
  QGL_invalidateExtensionFunc(table.m_glLoadIdentity);
  QGL_invalidateExtensionFunc(table.m_glLoadMatrixd);
  QGL_invalidateExtensionFunc(table.m_glLoadMatrixf);
  QGL_invalidateExtensionFunc(table.m_glLoadName);
  QGL_invalidateExtensionFunc(table.m_glLogicOp);
  QGL_invalidateExtensionFunc(table.m_glMap1d);
  QGL_invalidateExtensionFunc(table.m_glMap1f);
  QGL_invalidateExtensionFunc(table.m_glMap2d);
  QGL_invalidateExtensionFunc(table.m_glMap2f);
  QGL_invalidateExtensionFunc(table.m_glMapGrid1d);
  QGL_invalidateExtensionFunc(table.m_glMapGrid1f);
  QGL_invalidateExtensionFunc(table.m_glMapGrid2d);
  QGL_invalidateExtensionFunc(table.m_glMapGrid2f);
  QGL_invalidateExtensionFunc(table.m_glMaterialf);
  QGL_invalidateExtensionFunc(table.m_glMaterialfv);
  QGL_invalidateExtensionFunc(table.m_glMateriali);
  QGL_invalidateExtensionFunc(table.m_glMaterialiv);
  QGL_invalidateExtensionFunc(table.m_glMatrixMode);
  QGL_invalidateExtensionFunc(table.m_glMultMatrixd);
  QGL_invalidateExtensionFunc(table.m_glMultMatrixf);
  QGL_invalidateExtensionFunc(table.m_glNewList);
  QGL_invalidateExtensionFunc(table.m_glNormal3b);
  QGL_invalidateExtensionFunc(table.m_glNormal3bv);
  QGL_invalidateExtensionFunc(table.m_glNormal3d);
  QGL_invalidateExtensionFunc(table.m_glNormal3dv);
  QGL_invalidateExtensionFunc(table.m_glNormal3f);
  QGL_invalidateExtensionFunc(table.m_glNormal3fv);
  QGL_invalidateExtensionFunc(table.m_glNormal3i);
  QGL_invalidateExtensionFunc(table.m_glNormal3iv);
  QGL_invalidateExtensionFunc(table.m_glNormal3s);
  QGL_invalidateExtensionFunc(table.m_glNormal3sv);
  QGL_invalidateExtensionFunc(table.m_glNormalPointer);
  QGL_invalidateExtensionFunc(table.m_glOrtho);
  QGL_invalidateExtensionFunc(table.m_glPassThrough);
  QGL_invalidateExtensionFunc(table.m_glPixelMapfv);
  QGL_invalidateExtensionFunc(table.m_glPixelMapuiv);
  QGL_invalidateExtensionFunc(table.m_glPixelMapusv);
  QGL_invalidateExtensionFunc(table.m_glPixelStoref);
  QGL_invalidateExtensionFunc(table.m_glPixelStorei);
  QGL_invalidateExtensionFunc(table.m_glPixelTransferf);
  QGL_invalidateExtensionFunc(table.m_glPixelTransferi);
  QGL_invalidateExtensionFunc(table.m_glPixelZoom);
  QGL_invalidateExtensionFunc(table.m_glPointSize);
  QGL_invalidateExtensionFunc(table.m_glPolygonMode);
  QGL_invalidateExtensionFunc(table.m_glPolygonOffset);
  QGL_invalidateExtensionFunc(table.m_glPolygonStipple);
  QGL_invalidateExtensionFunc(table.m_glPopAttrib);
  QGL_invalidateExtensionFunc(table.m_glPopClientAttrib);
  QGL_invalidateExtensionFunc(table.m_glPopMatrix);
  QGL_invalidateExtensionFunc(table.m_glPopName);
  QGL_invalidateExtensionFunc(table.m_glPrioritizeTextures);
  QGL_invalidateExtensionFunc(table.m_glPushAttrib);
  QGL_invalidateExtensionFunc(table.m_glPushClientAttrib);
  QGL_invalidateExtensionFunc(table.m_glPushMatrix);
  QGL_invalidateExtensionFunc(table.m_glPushName);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2d);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2dv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2f);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2fv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2i);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2iv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2s);
  QGL_invalidateExtensionFunc(table.m_glRasterPos2sv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3d);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3dv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3f);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3fv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3i);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3iv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3s);
  QGL_invalidateExtensionFunc(table.m_glRasterPos3sv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4d);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4dv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4f);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4fv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4i);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4iv);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4s);
  QGL_invalidateExtensionFunc(table.m_glRasterPos4sv);
  QGL_invalidateExtensionFunc(table.m_glReadBuffer);
  QGL_invalidateExtensionFunc(table.m_glReadPixels);
  QGL_invalidateExtensionFunc(table.m_glRectd);
  QGL_invalidateExtensionFunc(table.m_glRectdv);
  QGL_invalidateExtensionFunc(table.m_glRectf);
  QGL_invalidateExtensionFunc(table.m_glRectfv);
  QGL_invalidateExtensionFunc(table.m_glRecti);
  QGL_invalidateExtensionFunc(table.m_glRectiv);
  QGL_invalidateExtensionFunc(table.m_glRects);
  QGL_invalidateExtensionFunc(table.m_glRectsv);
  QGL_invalidateExtensionFunc(table.m_glRenderMode);
  QGL_invalidateExtensionFunc(table.m_glRotated);
  QGL_invalidateExtensionFunc(table.m_glRotatef);
  QGL_invalidateExtensionFunc(table.m_glScaled);
  QGL_invalidateExtensionFunc(table.m_glScalef);
  QGL_invalidateExtensionFunc(table.m_glScissor);
  QGL_invalidateExtensionFunc(table.m_glSelectBuffer);
  QGL_invalidateExtensionFunc(table.m_glShadeModel);
  QGL_invalidateExtensionFunc(table.m_glStencilFunc);
  QGL_invalidateExtensionFunc(table.m_glStencilMask);
  QGL_invalidateExtensionFunc(table.m_glStencilOp);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1d);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1dv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1f);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1fv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1i);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1iv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1s);
  QGL_invalidateExtensionFunc(table.m_glTexCoord1sv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2d);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2dv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2f);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2fv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2i);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2iv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2s);
  QGL_invalidateExtensionFunc(table.m_glTexCoord2sv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3d);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3dv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3f);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3fv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3i);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3iv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3s);
  QGL_invalidateExtensionFunc(table.m_glTexCoord3sv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4d);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4dv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4f);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4fv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4i);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4iv);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4s);
  QGL_invalidateExtensionFunc(table.m_glTexCoord4sv);
  QGL_invalidateExtensionFunc(table.m_glTexCoordPointer);
  QGL_invalidateExtensionFunc(table.m_glTexEnvf);
  QGL_invalidateExtensionFunc(table.m_glTexEnvfv);
  QGL_invalidateExtensionFunc(table.m_glTexEnvi);
  QGL_invalidateExtensionFunc(table.m_glTexEnviv);
  QGL_invalidateExtensionFunc(table.m_glTexGend);
  QGL_invalidateExtensionFunc(table.m_glTexGendv);
  QGL_invalidateExtensionFunc(table.m_glTexGenf);
  QGL_invalidateExtensionFunc(table.m_glTexGenfv);
  QGL_invalidateExtensionFunc(table.m_glTexGeni);
  QGL_invalidateExtensionFunc(table.m_glTexGeniv);
  QGL_invalidateExtensionFunc(table.m_glTexImage1D);
  QGL_invalidateExtensionFunc(table.m_glTexImage2D);
  QGL_invalidateExtensionFunc(table.m_glTexParameterf);
  QGL_invalidateExtensionFunc(table.m_glTexParameterfv);
  QGL_invalidateExtensionFunc(table.m_glTexParameteri);
  QGL_invalidateExtensionFunc(table.m_glTexParameteriv);
  QGL_invalidateExtensionFunc(table.m_glTexSubImage1D);
  QGL_invalidateExtensionFunc(table.m_glTexSubImage2D);
  QGL_invalidateExtensionFunc(table.m_glTranslated);
  QGL_invalidateExtensionFunc(table.m_glTranslatef);
  QGL_invalidateExtensionFunc(table.m_glVertex2d);
  QGL_invalidateExtensionFunc(table.m_glVertex2dv);
  QGL_invalidateExtensionFunc(table.m_glVertex2f);
  QGL_invalidateExtensionFunc(table.m_glVertex2fv);
  QGL_invalidateExtensionFunc(table.m_glVertex2i);
  QGL_invalidateExtensionFunc(table.m_glVertex2iv);
  QGL_invalidateExtensionFunc(table.m_glVertex2s);
  QGL_invalidateExtensionFunc(table.m_glVertex2sv);
  QGL_invalidateExtensionFunc(table.m_glVertex3d);
  QGL_invalidateExtensionFunc(table.m_glVertex3dv);
  QGL_invalidateExtensionFunc(table.m_glVertex3f);
  QGL_invalidateExtensionFunc(table.m_glVertex3fv);
  QGL_invalidateExtensionFunc(table.m_glVertex3i);
  QGL_invalidateExtensionFunc(table.m_glVertex3iv);
  QGL_invalidateExtensionFunc(table.m_glVertex3s);
  QGL_invalidateExtensionFunc(table.m_glVertex3sv);
  QGL_invalidateExtensionFunc(table.m_glVertex4d);
  QGL_invalidateExtensionFunc(table.m_glVertex4dv);
  QGL_invalidateExtensionFunc(table.m_glVertex4f);
  QGL_invalidateExtensionFunc(table.m_glVertex4fv);
  QGL_invalidateExtensionFunc(table.m_glVertex4i);
  QGL_invalidateExtensionFunc(table.m_glVertex4iv);
  QGL_invalidateExtensionFunc(table.m_glVertex4s);
  QGL_invalidateExtensionFunc(table.m_glVertex4sv);
  QGL_invalidateExtensionFunc(table.m_glVertexPointer);
  QGL_invalidateExtensionFunc(table.m_glViewport);
}

int QGL_Init(OpenGLBinding& table)
{
  QGL_clear(table);

#ifdef WIN32
  qwglCopyContext              = wglCopyContext;
  qwglCreateContext            = wglCreateContext;
  qwglCreateLayerContext       = wglCreateLayerContext;
  qwglDeleteContext            = wglDeleteContext;
  qwglDescribeLayerPlane       = wglDescribeLayerPlane;
  qwglGetCurrentContext        = wglGetCurrentContext;
  qwglGetCurrentDC             = wglGetCurrentDC;
  qwglGetLayerPaletteEntries   = wglGetLayerPaletteEntries;
  qwglGetProcAddress           = wglGetProcAddress;
  qwglMakeCurrent              = wglMakeCurrent;
  qwglRealizeLayerPalette      = wglRealizeLayerPalette;
  qwglSetLayerPaletteEntries   = wglSetLayerPaletteEntries;
  qwglShareLists               = wglShareLists;
  qwglSwapLayerBuffers         = wglSwapLayerBuffers;
  qwglUseFontBitmaps           = wglUseFontBitmapsA;
  qwglUseFontOutlines          = wglUseFontOutlinesA;

  qwglChoosePixelFormat        = ChoosePixelFormat;
  qwglDescribePixelFormat      = DescribePixelFormat;
  qwglGetPixelFormat           = GetPixelFormat;
  qwglSetPixelFormat           = SetPixelFormat;
  qwglSwapBuffers              = SwapBuffers;
#endif

#if defined (__linux__) || defined (__APPLE__)
  qglXChooseVisual             = glXChooseVisual;
  qglXCreateContext            = glXCreateContext;
  qglXDestroyContext           = glXDestroyContext;
  qglXMakeCurrent              = glXMakeCurrent;
  //qglXCopyContext              = glXCopyContext;
  qglXSwapBuffers              = glXSwapBuffers;
  qglXCreateGLXPixmap          = glXCreateGLXPixmap;
  qglXDestroyGLXPixmap         = glXDestroyGLXPixmap;
  qglXQueryExtension           = glXQueryExtension;
  qglXQueryVersion             = glXQueryVersion;
  qglXIsDirect                 = glXIsDirect;
  qglXGetConfig                = glXGetConfig;
  qglXGetCurrentContext        = glXGetCurrentContext;
  qglXGetCurrentDrawable       = glXGetCurrentDrawable;
  qglXWaitGL                   = glXWaitGL;
  qglXWaitX                    = glXWaitX;
  qglXUseXFont                 = glXUseXFont;
//  qglXGetProcAddressARB        = glXGetProcAddressARB; // Utah-GLX fix
  qglXGetProcAddressARB = (glXGetProcAddressARBProc)dlsym(NULL, "glXGetProcAddressARB"); 
#endif

#if defined (__linux__) || defined (__APPLE__)
  if ((qglXQueryExtension == 0) || (qglXQueryExtension(GDK_DISPLAY(),0,0) != True))
    return 0;
#endif

  return 1;
}

int g_qglMajorVersion = 0;
int g_qglMinorVersion = 0;

// requires a valid gl context
void QGL_InitVersion()
{
#if EXTENSIONS_ENABLED
  const std::size_t versionSize = 256;
  char version[versionSize];
  strncpy(version, reinterpret_cast<const char*>(GlobalOpenGL().m_glGetString(GL_VERSION)), versionSize - 1);
  version[versionSize - 1] = '\0';
  char* firstDot = strchr(version, '.');
  ASSERT_NOTNULL(firstDot);
  *firstDot = '\0';
  g_qglMajorVersion = atoi(version);
  char* secondDot = strchr(firstDot + 1, '.');
  if(secondDot != 0)
  {
    *secondDot = '\0';
  }
  g_qglMinorVersion = atoi(firstDot + 1);
#else
  g_qglMajorVersion = 1;
  g_qglMinorVersion = 1;
#endif
}


inline void extension_not_implemented(const char* extension)
{
  globalErrorStream() << "WARNING: OpenGL driver reports support for " << extension << " but does not implement it\n";
}

void QGL_sharedContextCreated(OpenGLBinding& table)
{
  QGL_InitVersion();

  table.major_version = g_qglMajorVersion;
  table.minor_version = g_qglMinorVersion;

  table.m_glAccum                     = glAccum;
  table.m_glAlphaFunc                 = glAlphaFunc;
  table.m_glAreTexturesResident       = glAreTexturesResident;
  table.m_glArrayElement              = glArrayElement;
  table.m_glBegin                     = glBegin;
  table.m_glBindTexture               = glBindTexture;
  table.m_glBitmap                    = glBitmap;
  table.m_glBlendFunc                 = glBlendFunc;
  table.m_glCallList                  = glCallList;
  table.m_glCallLists                 = glCallLists;
  table.m_glClear                     = glClear;
  table.m_glClearAccum                = glClearAccum;
  table.m_glClearColor                = glClearColor;
  table.m_glClearDepth                = glClearDepth;
  table.m_glClearIndex                = glClearIndex;
  table.m_glClearStencil              = glClearStencil;
  table.m_glClipPlane                 = glClipPlane;
  table.m_glColor3b                   = glColor3b;
  table.m_glColor3bv                  = glColor3bv;
  table.m_glColor3d                   = glColor3d;
  table.m_glColor3dv                  = glColor3dv;
  table.m_glColor3f                   = glColor3f;
  table.m_glColor3fv                  = glColor3fv;
  table.m_glColor3i                   = glColor3i;
  table.m_glColor3iv                  = glColor3iv;
  table.m_glColor3s                   = glColor3s;
  table.m_glColor3sv                  = glColor3sv;
  table.m_glColor3ub                  = glColor3ub;
  table.m_glColor3ubv                 = glColor3ubv;
  table.m_glColor3ui                  = glColor3ui;
  table.m_glColor3uiv                 = glColor3uiv;
  table.m_glColor3us                  = glColor3us;
  table.m_glColor3usv                 = glColor3usv;
  table.m_glColor4b                   = glColor4b;
  table.m_glColor4bv                  = glColor4bv;
  table.m_glColor4d                   = glColor4d;
  table.m_glColor4dv                  = glColor4dv;
  table.m_glColor4f                   = glColor4f;
  table.m_glColor4fv                  = glColor4fv;
  table.m_glColor4i                   = glColor4i;
  table.m_glColor4iv                  = glColor4iv;
  table.m_glColor4s                   = glColor4s;
  table.m_glColor4sv                  = glColor4sv;
  table.m_glColor4ub                  = glColor4ub;
  table.m_glColor4ubv                 = glColor4ubv;
  table.m_glColor4ui                  = glColor4ui;
  table.m_glColor4uiv                 = glColor4uiv;
  table.m_glColor4us                  = glColor4us;
  table.m_glColor4usv                 = glColor4usv;
  table.m_glColorMask                 = glColorMask;
  table.m_glColorMaterial             = glColorMaterial;
  table.m_glColorPointer              = glColorPointer;
  table.m_glCopyPixels                = glCopyPixels;
  table.m_glCopyTexImage1D            = glCopyTexImage1D;
  table.m_glCopyTexImage2D            = glCopyTexImage2D;
  table.m_glCopyTexSubImage1D         = glCopyTexSubImage1D;
  table.m_glCopyTexSubImage2D         = glCopyTexSubImage2D;
  table.m_glCullFace                  = glCullFace;
  table.m_glDeleteLists               = glDeleteLists;
  table.m_glDeleteTextures            = glDeleteTextures;
  table.m_glDepthFunc                 = glDepthFunc;
  table.m_glDepthMask                 = glDepthMask;
  table.m_glDepthRange                = glDepthRange;
  table.m_glDisable                   = glDisable;
  table.m_glDisableClientState        = glDisableClientState;
  table.m_glDrawArrays                = glDrawArrays;
  table.m_glDrawBuffer                = glDrawBuffer;
  table.m_glDrawElements              = glDrawElements;
  table.m_glDrawPixels                = glDrawPixels;
  table.m_glEdgeFlag                  = glEdgeFlag;
  table.m_glEdgeFlagPointer           = glEdgeFlagPointer;
  table.m_glEdgeFlagv                 = glEdgeFlagv;
  table.m_glEnable                    = glEnable;
  table.m_glEnableClientState         = glEnableClientState;
  table.m_glEnd                       = glEnd;
  table.m_glEndList                   = glEndList;
  table.m_glEvalCoord1d               = glEvalCoord1d;
  table.m_glEvalCoord1dv              = glEvalCoord1dv;
  table.m_glEvalCoord1f               = glEvalCoord1f;
  table.m_glEvalCoord1fv              = glEvalCoord1fv;
  table.m_glEvalCoord2d               = glEvalCoord2d;
  table.m_glEvalCoord2dv              = glEvalCoord2dv;
  table.m_glEvalCoord2f               = glEvalCoord2f;
  table.m_glEvalCoord2fv              = glEvalCoord2fv;
  table.m_glEvalMesh1                 = glEvalMesh1;
  table.m_glEvalMesh2                 = glEvalMesh2;
  table.m_glEvalPoint1                = glEvalPoint1;
  table.m_glEvalPoint2                = glEvalPoint2;
  table.m_glFeedbackBuffer            = glFeedbackBuffer;
  table.m_glFinish                    = glFinish;
  table.m_glFlush                     = glFlush;
  table.m_glFogf                      = glFogf;
  table.m_glFogfv                     = glFogfv;
  table.m_glFogi                      = glFogi;
  table.m_glFogiv                     = glFogiv;
  table.m_glFrontFace                 = glFrontFace;
  table.m_glFrustum                   = glFrustum;
  table.m_glGenLists                  = glGenLists;
  table.m_glGenTextures               = glGenTextures;
  table.m_glGetBooleanv               = glGetBooleanv;
  table.m_glGetClipPlane              = glGetClipPlane;
  table.m_glGetDoublev                = glGetDoublev;
  table.m_glGetError                  = glGetError;
  table.m_glGetFloatv                 = glGetFloatv;
  table.m_glGetIntegerv               = glGetIntegerv;
  table.m_glGetLightfv                = glGetLightfv;
  table.m_glGetLightiv                = glGetLightiv;
  table.m_glGetMapdv                  = glGetMapdv;
  table.m_glGetMapfv                  = glGetMapfv;
  table.m_glGetMapiv                  = glGetMapiv;
  table.m_glGetMaterialfv             = glGetMaterialfv;
  table.m_glGetMaterialiv             = glGetMaterialiv;
  table.m_glGetPixelMapfv             = glGetPixelMapfv;
  table.m_glGetPixelMapuiv            = glGetPixelMapuiv;
  table.m_glGetPixelMapusv            = glGetPixelMapusv;
  table.m_glGetPointerv               = glGetPointerv;
  table.m_glGetPolygonStipple         = glGetPolygonStipple;
  table.m_glGetString                 = glGetString;
  table.m_glGetTexEnvfv               = glGetTexEnvfv;
  table.m_glGetTexEnviv               = glGetTexEnviv;
  table.m_glGetTexGendv               = glGetTexGendv;
  table.m_glGetTexGenfv               = glGetTexGenfv;
  table.m_glGetTexGeniv               = glGetTexGeniv;
  table.m_glGetTexImage               = glGetTexImage;
  table.m_glGetTexLevelParameterfv    = glGetTexLevelParameterfv;
  table.m_glGetTexLevelParameteriv    = glGetTexLevelParameteriv;
  table.m_glGetTexParameterfv         = glGetTexParameterfv;
  table.m_glGetTexParameteriv         = glGetTexParameteriv;
  table.m_glHint                      = glHint;
  table.m_glIndexMask                 = glIndexMask;
  table.m_glIndexPointer              = glIndexPointer;
  table.m_glIndexd                    = glIndexd;
  table.m_glIndexdv                   = glIndexdv;
  table.m_glIndexf                    = glIndexf;
  table.m_glIndexfv                   = glIndexfv;
  table.m_glIndexi                    = glIndexi;
  table.m_glIndexiv                   = glIndexiv;
  table.m_glIndexs                    = glIndexs;
  table.m_glIndexsv                   = glIndexsv;
  table.m_glIndexub                   = glIndexub;
  table.m_glIndexubv                  = glIndexubv;
  table.m_glInitNames                 = glInitNames;
  table.m_glInterleavedArrays         = glInterleavedArrays;
  table.m_glIsEnabled                 = glIsEnabled;
  table.m_glIsList                    = glIsList;
  table.m_glIsTexture                 = glIsTexture;
  table.m_glLightModelf               = glLightModelf;
  table.m_glLightModelfv              = glLightModelfv;
  table.m_glLightModeli               = glLightModeli;
  table.m_glLightModeliv              = glLightModeliv;
  table.m_glLightf                    = glLightf;
  table.m_glLightfv                   = glLightfv;
  table.m_glLighti                    = glLighti;
  table.m_glLightiv                   = glLightiv;
  table.m_glLineStipple               = glLineStipple;
  table.m_glLineWidth                 = glLineWidth;
  table.m_glListBase                  = glListBase;
  table.m_glLoadIdentity              = glLoadIdentity;
  table.m_glLoadMatrixd               = glLoadMatrixd;
  table.m_glLoadMatrixf               = glLoadMatrixf;
  table.m_glLoadName                  = glLoadName;
  table.m_glLogicOp                   = glLogicOp;
  table.m_glMap1d                     = glMap1d;
  table.m_glMap1f                     = glMap1f;
  table.m_glMap2d                     = glMap2d;
  table.m_glMap2f                     = glMap2f;
  table.m_glMapGrid1d                 = glMapGrid1d;
  table.m_glMapGrid1f                 = glMapGrid1f;
  table.m_glMapGrid2d                 = glMapGrid2d;
  table.m_glMapGrid2f                 = glMapGrid2f;
  table.m_glMaterialf                 = glMaterialf;
  table.m_glMaterialfv                = glMaterialfv;
  table.m_glMateriali                 = glMateriali;
  table.m_glMaterialiv                = glMaterialiv;
  table.m_glMatrixMode                = glMatrixMode;
  table.m_glMultMatrixd               = glMultMatrixd;
  table.m_glMultMatrixf               = glMultMatrixf;
  table.m_glNewList                   = glNewList;
  table.m_glNormal3b                  = glNormal3b;
  table.m_glNormal3bv                 = glNormal3bv;
  table.m_glNormal3d                  = glNormal3d;
  table.m_glNormal3dv                 = glNormal3dv;
  table.m_glNormal3f                  = glNormal3f;
  table.m_glNormal3fv                 = glNormal3fv;
  table.m_glNormal3i                  = glNormal3i;
  table.m_glNormal3iv                 = glNormal3iv;
  table.m_glNormal3s                  = glNormal3s;
  table.m_glNormal3sv                 = glNormal3sv;
  table.m_glNormalPointer             = glNormalPointer;
  table.m_glOrtho                     = glOrtho;
  table.m_glPassThrough               = glPassThrough;
  table.m_glPixelMapfv                = glPixelMapfv;
  table.m_glPixelMapuiv               = glPixelMapuiv;
  table.m_glPixelMapusv               = glPixelMapusv;
  table.m_glPixelStoref               = glPixelStoref;
  table.m_glPixelStorei               = glPixelStorei;
  table.m_glPixelTransferf            = glPixelTransferf;
  table.m_glPixelTransferi            = glPixelTransferi;
  table.m_glPixelZoom                 = glPixelZoom;
  table.m_glPointSize                 = glPointSize;
  table.m_glPolygonMode               = glPolygonMode;
  table.m_glPolygonOffset             = glPolygonOffset;
  table.m_glPolygonStipple            = glPolygonStipple;
  table.m_glPopAttrib                 = glPopAttrib;
  table.m_glPopClientAttrib           = glPopClientAttrib;
  table.m_glPopMatrix                 = glPopMatrix;
  table.m_glPopName                   = glPopName;
  table.m_glPrioritizeTextures        = glPrioritizeTextures;
  table.m_glPushAttrib                = glPushAttrib;
  table.m_glPushClientAttrib          = glPushClientAttrib;
  table.m_glPushMatrix                = glPushMatrix;
  table.m_glPushName                  = glPushName;
  table.m_glRasterPos2d               = glRasterPos2d;
  table.m_glRasterPos2dv              = glRasterPos2dv;
  table.m_glRasterPos2f               = glRasterPos2f;
  table.m_glRasterPos2fv              = glRasterPos2fv;
  table.m_glRasterPos2i               = glRasterPos2i;
  table.m_glRasterPos2iv              = glRasterPos2iv;
  table.m_glRasterPos2s               = glRasterPos2s;
  table.m_glRasterPos2sv              = glRasterPos2sv;
  table.m_glRasterPos3d               = glRasterPos3d;
  table.m_glRasterPos3dv              = glRasterPos3dv;
  table.m_glRasterPos3f               = glRasterPos3f;
  table.m_glRasterPos3fv              = glRasterPos3fv;
  table.m_glRasterPos3i               = glRasterPos3i;
  table.m_glRasterPos3iv              = glRasterPos3iv;
  table.m_glRasterPos3s               = glRasterPos3s;
  table.m_glRasterPos3sv              = glRasterPos3sv;
  table.m_glRasterPos4d               = glRasterPos4d;
  table.m_glRasterPos4dv              = glRasterPos4dv;
  table.m_glRasterPos4f               = glRasterPos4f;
  table.m_glRasterPos4fv              = glRasterPos4fv;
  table.m_glRasterPos4i               = glRasterPos4i;
  table.m_glRasterPos4iv              = glRasterPos4iv;
  table.m_glRasterPos4s               = glRasterPos4s;
  table.m_glRasterPos4sv              = glRasterPos4sv;
  table.m_glReadBuffer                = glReadBuffer;
  table.m_glReadPixels                = glReadPixels;
  table.m_glRectd                     = glRectd;
  table.m_glRectdv                    = glRectdv;
  table.m_glRectf                     = glRectf;
  table.m_glRectfv                    = glRectfv;
  table.m_glRecti                     = glRecti;
  table.m_glRectiv                    = glRectiv;
  table.m_glRects                     = glRects;
  table.m_glRectsv                    = glRectsv;
  table.m_glRenderMode                = glRenderMode;
  table.m_glRotated                   = glRotated;
  table.m_glRotatef                   = glRotatef;
  table.m_glScaled                    = glScaled;
  table.m_glScalef                    = glScalef;
  table.m_glScissor                   = glScissor;
  table.m_glSelectBuffer              = glSelectBuffer;
  table.m_glShadeModel                = glShadeModel;
  table.m_glStencilFunc               = glStencilFunc;
  table.m_glStencilMask               = glStencilMask;
  table.m_glStencilOp                 = glStencilOp;
  table.m_glTexCoord1d                = glTexCoord1d;
  table.m_glTexCoord1dv               = glTexCoord1dv;
  table.m_glTexCoord1f                = glTexCoord1f;
  table.m_glTexCoord1fv               = glTexCoord1fv;
  table.m_glTexCoord1i                = glTexCoord1i;
  table.m_glTexCoord1iv               = glTexCoord1iv;
  table.m_glTexCoord1s                = glTexCoord1s;
  table.m_glTexCoord1sv               = glTexCoord1sv;
  table.m_glTexCoord2d                = glTexCoord2d;
  table.m_glTexCoord2dv               = glTexCoord2dv;
  table.m_glTexCoord2f                = glTexCoord2f;
  table.m_glTexCoord2fv               = glTexCoord2fv;
  table.m_glTexCoord2i                = glTexCoord2i;
  table.m_glTexCoord2iv               = glTexCoord2iv;
  table.m_glTexCoord2s                = glTexCoord2s;
  table.m_glTexCoord2sv               = glTexCoord2sv;
  table.m_glTexCoord3d                = glTexCoord3d;
  table.m_glTexCoord3dv               = glTexCoord3dv;
  table.m_glTexCoord3f                = glTexCoord3f;
  table.m_glTexCoord3fv               = glTexCoord3fv;
  table.m_glTexCoord3i                = glTexCoord3i;
  table.m_glTexCoord3iv               = glTexCoord3iv;
  table.m_glTexCoord3s                = glTexCoord3s;
  table.m_glTexCoord3sv               = glTexCoord3sv;
  table.m_glTexCoord4d                = glTexCoord4d;
  table.m_glTexCoord4dv               = glTexCoord4dv;
  table.m_glTexCoord4f                = glTexCoord4f;
  table.m_glTexCoord4fv               = glTexCoord4fv;
  table.m_glTexCoord4i                = glTexCoord4i;
  table.m_glTexCoord4iv               = glTexCoord4iv;
  table.m_glTexCoord4s                = glTexCoord4s;
  table.m_glTexCoord4sv               = glTexCoord4sv;
  table.m_glTexCoordPointer           = glTexCoordPointer;
  table.m_glTexEnvf                   = glTexEnvf;
  table.m_glTexEnvfv                  = glTexEnvfv;
  table.m_glTexEnvi                   = glTexEnvi;
  table.m_glTexEnviv                  = glTexEnviv;
  table.m_glTexGend                   = glTexGend;
  table.m_glTexGendv                  = glTexGendv;
  table.m_glTexGenf                   = glTexGenf;
  table.m_glTexGenfv                  = glTexGenfv;
  table.m_glTexGeni                   = glTexGeni;
  table.m_glTexGeniv                  = glTexGeniv;
  table.m_glTexImage1D                = glTexImage1D;
  table.m_glTexImage2D                = glTexImage2D;
  table.m_glTexParameterf             = glTexParameterf;
  table.m_glTexParameterfv            = glTexParameterfv;
  table.m_glTexParameteri             = glTexParameteri;
  table.m_glTexParameteriv            = glTexParameteriv;
  table.m_glTexSubImage1D             = glTexSubImage1D;
  table.m_glTexSubImage2D             = glTexSubImage2D;
  table.m_glTranslated                = glTranslated;
  table.m_glTranslatef                = glTranslatef;
  table.m_glVertex2d                  = glVertex2d;
  table.m_glVertex2dv                 = glVertex2dv;
  table.m_glVertex2f                  = glVertex2f;
  table.m_glVertex2fv                 = glVertex2fv;
  table.m_glVertex2i                  = glVertex2i;
  table.m_glVertex2iv                 = glVertex2iv;
  table.m_glVertex2s                  = glVertex2s;
  table.m_glVertex2sv                 = glVertex2sv;
  table.m_glVertex3d                  = glVertex3d;
  table.m_glVertex3dv                 = glVertex3dv;
  table.m_glVertex3f                  = glVertex3f;
  table.m_glVertex3fv                 = glVertex3fv;
  table.m_glVertex3i                  = glVertex3i;
  table.m_glVertex3iv                 = glVertex3iv;
  table.m_glVertex3s                  = glVertex3s;
  table.m_glVertex3sv                 = glVertex3sv;
  table.m_glVertex4d                  = glVertex4d;
  table.m_glVertex4dv                 = glVertex4dv;
  table.m_glVertex4f                  = glVertex4f;
  table.m_glVertex4fv                 = glVertex4fv;
  table.m_glVertex4i                  = glVertex4i;
  table.m_glVertex4iv                 = glVertex4iv;
  table.m_glVertex4s                  = glVertex4s;
  table.m_glVertex4sv                 = glVertex4sv;
  table.m_glVertexPointer             = glVertexPointer;
  table.m_glViewport                  = glViewport;

  if(QGL_ExtensionSupported("GL_ARB_multitexture"))
  {
    table.support_ARB_multitexture =
    QGL_constructExtensionFunc(table.m_glActiveTextureARB, "glActiveTextureARB")
    && QGL_constructExtensionFunc(table.m_glClientActiveTextureARB, "glClientActiveTextureARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1dARB, "glMultiTexCoord1dARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1dvARB, "glMultiTexCoord1dvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1fARB, "glMultiTexCoord1fARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1fvARB, "glMultiTexCoord1fvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1iARB, "glMultiTexCoord1iARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1ivARB, "glMultiTexCoord1ivARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1sARB, "glMultiTexCoord1sARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1svARB, "glMultiTexCoord1svARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2dARB, "glMultiTexCoord2dARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2dvARB, "glMultiTexCoord2dvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2fARB, "glMultiTexCoord2fARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2fvARB, "glMultiTexCoord2fvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2iARB, "glMultiTexCoord2iARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2ivARB, "glMultiTexCoord2ivARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2sARB, "glMultiTexCoord2sARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2svARB, "glMultiTexCoord2svARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3dARB, "glMultiTexCoord3dARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3dvARB, "glMultiTexCoord3dvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3fARB, "glMultiTexCoord3fARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3fvARB, "glMultiTexCoord3fvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3iARB, "glMultiTexCoord3iARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3ivARB, "glMultiTexCoord3ivARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3sARB, "glMultiTexCoord3sARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3svARB, "glMultiTexCoord3svARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4dARB, "glMultiTexCoord4dARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4dvARB, "glMultiTexCoord4dvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4fARB, "glMultiTexCoord4fARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4fvARB, "glMultiTexCoord4fvARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4iARB, "glMultiTexCoord4iARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4ivARB, "glMultiTexCoord4ivARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4sARB, "glMultiTexCoord4sARB")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4svARB, "glMultiTexCoord4svARB");
    
    if(!table.support_ARB_multitexture)
    {
      extension_not_implemented("GL_ARB_multitexture");
    }
  }
  else
  {
    table.support_ARB_multitexture = false;
  }

  if(QGL_ExtensionSupported("GL_ARB_texture_compression"))
  {
    table.support_ARB_texture_compression =
    QGL_constructExtensionFunc(table.m_glCompressedTexImage3DARB, "glCompressedTexImage3DARB")
    && QGL_constructExtensionFunc(table.m_glCompressedTexImage2DARB, "glCompressedTexImage2DARB")
    && QGL_constructExtensionFunc(table.m_glCompressedTexImage1DARB, "glCompressedTexImage1DARB")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage3DARB, "glCompressedTexSubImage3DARB")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage2DARB, "glCompressedTexSubImage2DARB")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage1DARB, "glCompressedTexSubImage1DARB")
    && QGL_constructExtensionFunc(table.m_glGetCompressedTexImageARB, "glGetCompressedTexImageARB");
    
    if(!table.support_ARB_texture_compression)
    {
      extension_not_implemented("GL_ARB_texture_compression");
    }
  }
  else
  {
    table.support_ARB_texture_compression = false;
  }

  table.support_EXT_texture_compression_s3tc = QGL_ExtensionSupported("GL_EXT_texture_compression_s3tc");

  // GL 1.2
  if(table.major_version > 1 || table.minor_version >= 2)
  {
    table.support_GL_1_2 =
    QGL_constructExtensionFunc(table.m_glCopyTexSubImage3D, "glCopyTexSubImage3D")
    && QGL_constructExtensionFunc(table.m_glDrawRangeElements, "glDrawRangeElements")
    && QGL_constructExtensionFunc(table.m_glTexImage3D, "glTexImage3D")
    && QGL_constructExtensionFunc(table.m_glTexSubImage3D, "glTexSubImage3D");

    if(!table.support_GL_1_2)
    {
      extension_not_implemented("GL_VERSION_1_2");
    }
  }
  else
  {
    table.support_GL_1_2 = false;
  }

  // GL 1.3
  if(table.major_version > 1 || table.minor_version >= 3)
  {
    table.support_GL_1_3 =
    QGL_constructExtensionFunc(table.m_glActiveTexture, "glActiveTexture")
    && QGL_constructExtensionFunc(table.m_glClientActiveTexture, "glClientActiveTexture")
    && QGL_constructExtensionFunc(table.m_glCompressedTexImage1D, "glCompressedTexImage1D")
    && QGL_constructExtensionFunc(table.m_glCompressedTexImage2D, "glCompressedTexImage2D")
    && QGL_constructExtensionFunc(table.m_glCompressedTexImage3D, "glCompressedTexImage3D")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage1D, "glCompressedTexSubImage1D")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage2D, "glCompressedTexSubImage2D")
    && QGL_constructExtensionFunc(table.m_glCompressedTexSubImage3D, "glCompressedTexSubImage3D")
    && QGL_constructExtensionFunc(table.m_glGetCompressedTexImage, "glGetCompressedTexImage")
    && QGL_constructExtensionFunc(table.m_glLoadTransposeMatrixd, "glLoadTransposeMatrixd")
    && QGL_constructExtensionFunc(table.m_glLoadTransposeMatrixf, "glLoadTransposeMatrixf")
    && QGL_constructExtensionFunc(table.m_glMultTransposeMatrixd, "glMultTransposeMatrixd")
    && QGL_constructExtensionFunc(table.m_glMultTransposeMatrixf, "glMultTransposeMatrixf")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1d, "glMultiTexCoord1d")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1dv, "glMultiTexCoord1dv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1f, "glMultiTexCoord1f")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1fv, "glMultiTexCoord1fv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1i, "glMultiTexCoord1i")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1iv, "glMultiTexCoord1iv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1s, "glMultiTexCoord1s")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord1sv, "glMultiTexCoord1sv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2d, "glMultiTexCoord2d")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2dv, "glMultiTexCoord2dv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2f, "glMultiTexCoord2f")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2fv, "glMultiTexCoord2fv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2i, "glMultiTexCoord2i")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2iv, "glMultiTexCoord2iv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2s, "glMultiTexCoord2s")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord2sv, "glMultiTexCoord2sv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3d, "glMultiTexCoord3d")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3dv, "glMultiTexCoord3dv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3f, "glMultiTexCoord3f")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3fv, "glMultiTexCoord3fv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3i, "glMultiTexCoord3i")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3iv, "glMultiTexCoord3iv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3s, "glMultiTexCoord3s")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord3sv, "glMultiTexCoord3sv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4d, "glMultiTexCoord4d")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4dv, "glMultiTexCoord4dv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4f, "glMultiTexCoord4f")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4fv, "glMultiTexCoord4fv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4i, "glMultiTexCoord4i")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4iv, "glMultiTexCoord4iv")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4s, "glMultiTexCoord4s")
    && QGL_constructExtensionFunc(table.m_glMultiTexCoord4sv, "glMultiTexCoord4sv")
    && QGL_constructExtensionFunc(table.m_glSampleCoverage, "glSampleCoverage");

    if(!table.support_GL_1_3)
    {
      extension_not_implemented("GL_VERSION_1_3");
    }
  }
  else
  {
    table.support_GL_1_3 = false;
  }

  // GL 1.4
  if(table.major_version > 1 || table.minor_version >= 4)
  {
    table.support_GL_1_4 =
    QGL_constructExtensionFunc(table.m_glBlendColor, "glBlendColor")
    && QGL_constructExtensionFunc(table.m_glBlendEquation, "glBlendEquation")
    && QGL_constructExtensionFunc(table.m_glBlendFuncSeparate, "glBlendFuncSeparate")
    && QGL_constructExtensionFunc(table.m_glFogCoordPointer, "glFogCoordPointer")
    && QGL_constructExtensionFunc(table.m_glFogCoordd, "glFogCoordd")
    && QGL_constructExtensionFunc(table.m_glFogCoorddv, "glFogCoorddv")
    && QGL_constructExtensionFunc(table.m_glFogCoordf, "glFogCoordf")
    && QGL_constructExtensionFunc(table.m_glFogCoordfv, "glFogCoordfv")
    && QGL_constructExtensionFunc(table.m_glMultiDrawArrays, "glMultiDrawArrays")
    && QGL_constructExtensionFunc(table.m_glMultiDrawElements, "glMultiDrawElements")
    && QGL_constructExtensionFunc(table.m_glPointParameterf, "glPointParameterf")
    && QGL_constructExtensionFunc(table.m_glPointParameterfv, "glPointParameterfv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3b, "glSecondaryColor3b")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3bv, "glSecondaryColor3bv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3d, "glSecondaryColor3d")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3dv, "glSecondaryColor3dv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3f, "glSecondaryColor3f")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3fv, "glSecondaryColor3fv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3i, "glSecondaryColor3i")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3iv, "glSecondaryColor3iv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3s, "glSecondaryColor3s")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3sv, "glSecondaryColor3sv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3ub, "glSecondaryColor3ub")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3ubv, "glSecondaryColor3ubv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3ui, "glSecondaryColor3ui")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3uiv, "glSecondaryColor3uiv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3us, "glSecondaryColor3us")
    && QGL_constructExtensionFunc(table.m_glSecondaryColor3usv, "glSecondaryColor3usv")
    && QGL_constructExtensionFunc(table.m_glSecondaryColorPointer, "glSecondaryColorPointer")
    && QGL_constructExtensionFunc(table.m_glWindowPos2d, "glWindowPos2d")
    && QGL_constructExtensionFunc(table.m_glWindowPos2dv, "glWindowPos2dv")
    && QGL_constructExtensionFunc(table.m_glWindowPos2f, "glWindowPos2f")
    && QGL_constructExtensionFunc(table.m_glWindowPos2fv, "glWindowPos2fv")
    && QGL_constructExtensionFunc(table.m_glWindowPos2i, "glWindowPos2i")
    && QGL_constructExtensionFunc(table.m_glWindowPos2iv, "glWindowPos2iv")
    && QGL_constructExtensionFunc(table.m_glWindowPos2s, "glWindowPos2s")
    && QGL_constructExtensionFunc(table.m_glWindowPos2sv, "glWindowPos2sv")
    && QGL_constructExtensionFunc(table.m_glWindowPos3d, "glWindowPos3d")
    && QGL_constructExtensionFunc(table.m_glWindowPos3dv, "glWindowPos3dv")
    && QGL_constructExtensionFunc(table.m_glWindowPos3f, "glWindowPos3f")
    && QGL_constructExtensionFunc(table.m_glWindowPos3fv, "glWindowPos3fv")
    && QGL_constructExtensionFunc(table.m_glWindowPos3i, "glWindowPos3i")
    && QGL_constructExtensionFunc(table.m_glWindowPos3iv, "glWindowPos3iv")
    && QGL_constructExtensionFunc(table.m_glWindowPos3s, "glWindowPos3s")
    && QGL_constructExtensionFunc(table.m_glWindowPos3sv, "glWindowPos3sv");

    if(!table.support_GL_1_4)
    {
      extension_not_implemented("GL_VERSION_1_4");
    }
  }
  else
  {
    table.support_GL_1_4 = false;
  }
  
  // GL 1.5
  if(table.major_version > 1 || table.minor_version >= 5)
  {
    table.support_GL_1_5 =
    QGL_constructExtensionFunc(table.m_glBeginQuery, "glBeginQuery")
    && QGL_constructExtensionFunc(table.m_glBindBuffer, "glBindBuffer")
    && QGL_constructExtensionFunc(table.m_glBufferData, "glBufferData")
    && QGL_constructExtensionFunc(table.m_glBufferSubData, "glBufferSubData")
    && QGL_constructExtensionFunc(table.m_glDeleteBuffers, "glDeleteBuffers")
    && QGL_constructExtensionFunc(table.m_glDeleteQueries, "glDeleteQueries")
    && QGL_constructExtensionFunc(table.m_glEndQuery, "glEndQuery")
    && QGL_constructExtensionFunc(table.m_glGenBuffers, "glGenBuffers")
    && QGL_constructExtensionFunc(table.m_glGenQueries, "glGenQueries")
    && QGL_constructExtensionFunc(table.m_glGetBufferParameteriv, "glGetBufferParameteriv")
    && QGL_constructExtensionFunc(table.m_glGetBufferPointerv, "glGetBufferPointerv")
    && QGL_constructExtensionFunc(table.m_glGetBufferSubData, "glGetBufferSubData")
    && QGL_constructExtensionFunc(table.m_glGetQueryObjectiv, "glGetQueryObjectiv")
    && QGL_constructExtensionFunc(table.m_glGetQueryObjectuiv, "glGetQueryObjectuiv")
    && QGL_constructExtensionFunc(table.m_glGetQueryiv, "glGetQueryiv")
    && QGL_constructExtensionFunc(table.m_glIsBuffer, "glIsBuffer")
    && QGL_constructExtensionFunc(table.m_glIsQuery, "glIsQuery")
    && QGL_constructExtensionFunc(table.m_glMapBuffer, "glMapBuffer")
    && QGL_constructExtensionFunc(table.m_glUnmapBuffer, "glUnmapBuffer");

    if(!table.support_GL_1_5)
    {
      extension_not_implemented("GL_VERSION_1_5");
    }
  }
  else
  {
    table.support_GL_1_5 = false;
  }


  if(QGL_ExtensionSupported("GL_ARB_vertex_program"))
  {
    table.support_ARB_vertex_program = 
    QGL_constructExtensionFunc(table.m_glVertexAttrib1sARB, "glVertexAttrib1sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1fARB, "glVertexAttrib1fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1dARB, "glVertexAttrib1dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2sARB, "glVertexAttrib2sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fARB, "glVertexAttrib2fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2dARB, "glVertexAttrib2dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3sARB, "glVertexAttrib3sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fARB, "glVertexAttrib3fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3dARB, "glVertexAttrib3dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4sARB, "glVertexAttrib4sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fARB, "glVertexAttrib4fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4dARB, "glVertexAttrib4dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NubARB, "glVertexAttrib4NubARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1svARB, "glVertexAttrib1svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1fvARB, "glVertexAttrib1fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1dvARB, "glVertexAttrib1dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2svARB, "glVertexAttrib2svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fvARB, "glVertexAttrib2fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2dvARB, "glVertexAttrib2dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3svARB, "glVertexAttrib3svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fvARB, "glVertexAttrib3fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3dvARB, "glVertexAttrib3dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4bvARB, "glVertexAttrib4bvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4svARB, "glVertexAttrib4svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4ivARB, "glVertexAttrib4ivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4ubvARB, "glVertexAttrib4ubvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4usvARB, "glVertexAttrib4usvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4uivARB, "glVertexAttrib4uivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fvARB, "glVertexAttrib4fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4dvARB, "glVertexAttrib4dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NbvARB, "glVertexAttrib4NbvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NsvARB, "glVertexAttrib4NsvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NivARB, "glVertexAttrib4NivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NubvARB, "glVertexAttrib4NubvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NusvARB, "glVertexAttrib4NusvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NuivARB, "glVertexAttrib4NuivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttribPointerARB, "glVertexAttribPointerARB")
    && QGL_constructExtensionFunc(table.m_glEnableVertexAttribArrayARB, "glEnableVertexAttribArrayARB")
    && QGL_constructExtensionFunc(table.m_glDisableVertexAttribArrayARB, "glDisableVertexAttribArrayARB")
    && QGL_constructExtensionFunc(table.m_glProgramStringARB, "glProgramStringARB")
    && QGL_constructExtensionFunc(table.m_glBindProgramARB, "glBindProgramARB")
    && QGL_constructExtensionFunc(table.m_glDeleteProgramsARB, "glDeleteProgramsARB")
    && QGL_constructExtensionFunc(table.m_glGenProgramsARB, "glGenProgramsARB")
    && QGL_constructExtensionFunc(table.m_glProgramEnvParameter4dARB, "glProgramEnvParameter4dARB")
    && QGL_constructExtensionFunc(table.m_glProgramEnvParameter4dvARB, "glProgramEnvParameter4dvARB")
    && QGL_constructExtensionFunc(table.m_glProgramEnvParameter4fARB, "glProgramEnvParameter4fARB")
    && QGL_constructExtensionFunc(table.m_glProgramEnvParameter4fvARB, "glProgramEnvParameter4fvARB")
    && QGL_constructExtensionFunc(table.m_glProgramLocalParameter4dARB, "glProgramLocalParameter4dARB")
    && QGL_constructExtensionFunc(table.m_glProgramLocalParameter4dvARB, "glProgramLocalParameter4dvARB")
    && QGL_constructExtensionFunc(table.m_glProgramLocalParameter4fARB, "glProgramLocalParameter4fARB")
    && QGL_constructExtensionFunc(table.m_glProgramLocalParameter4fvARB, "glProgramLocalParameter4fvARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramEnvParameterdvARB, "glGetProgramEnvParameterdvARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramEnvParameterfvARB, "glGetProgramEnvParameterfvARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramLocalParameterdvARB, "glGetProgramLocalParameterdvARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramLocalParameterfvARB, "glGetProgramLocalParameterfvARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramivARB, "glGetProgramivARB")
    && QGL_constructExtensionFunc(table.m_glGetProgramStringARB, "glGetProgramStringARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribdvARB, "glGetVertexAttribdvARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribfvARB, "glGetVertexAttribfvARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribivARB, "glGetVertexAttribivARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribPointervARB, "glGetVertexAttribPointervARB")
    && QGL_constructExtensionFunc(table.m_glIsProgramARB, "glIsProgramARB");

    if(!table.support_ARB_vertex_program)
    {
      extension_not_implemented("GL_ARB_vertex_program");
    }
  }
  else
  {
    table.support_ARB_vertex_program = false;
  }


  table.support_ARB_fragment_program = QGL_ExtensionSupported("GL_ARB_fragment_program");

  if(QGL_ExtensionSupported("GL_ARB_shader_objects"))
  {
    table.support_ARB_shader_objects =
    QGL_constructExtensionFunc(table.m_glDeleteObjectARB, "glDeleteObjectARB")
    && QGL_constructExtensionFunc(table.m_glGetHandleARB, "glGetHandleARB")
    && QGL_constructExtensionFunc(table.m_glDetachObjectARB, "glDetachObjectARB")
    && QGL_constructExtensionFunc(table.m_glCreateShaderObjectARB, "glCreateShaderObjectARB")
    && QGL_constructExtensionFunc(table.m_glShaderSourceARB, "glShaderSourceARB")
    && QGL_constructExtensionFunc(table.m_glCompileShaderARB, "glCompileShaderARB")
    && QGL_constructExtensionFunc(table.m_glCreateProgramObjectARB, "glCreateProgramObjectARB")
    && QGL_constructExtensionFunc(table.m_glAttachObjectARB, "glAttachObjectARB")
    && QGL_constructExtensionFunc(table.m_glLinkProgramARB, "glLinkProgramARB")
    && QGL_constructExtensionFunc(table.m_glUseProgramObjectARB, "glUseProgramObjectARB")
    && QGL_constructExtensionFunc(table.m_glValidateProgramARB, "glValidateProgramARB")
    && QGL_constructExtensionFunc(table.m_glUniform1fARB, "glUniform1fARB")
    && QGL_constructExtensionFunc(table.m_glUniform2fARB, "glUniform2fARB")
    && QGL_constructExtensionFunc(table.m_glUniform3fARB, "glUniform3fARB")
    && QGL_constructExtensionFunc(table.m_glUniform4fARB, "glUniform4fARB")
    && QGL_constructExtensionFunc(table.m_glUniform1iARB, "glUniform1iARB")
    && QGL_constructExtensionFunc(table.m_glUniform2iARB, "glUniform2iARB")
    && QGL_constructExtensionFunc(table.m_glUniform3iARB, "glUniform3iARB")
    && QGL_constructExtensionFunc(table.m_glUniform4iARB, "glUniform4iARB")
    && QGL_constructExtensionFunc(table.m_glUniform1fvARB, "glUniform1fvARB")
    && QGL_constructExtensionFunc(table.m_glUniform2fvARB, "glUniform2fvARB")
    && QGL_constructExtensionFunc(table.m_glUniform3fvARB, "glUniform3fvARB")
    && QGL_constructExtensionFunc(table.m_glUniform4fvARB, "glUniform4fvARB")
    && QGL_constructExtensionFunc(table.m_glUniform1ivARB, "glUniform1ivARB")
    && QGL_constructExtensionFunc(table.m_glUniform2ivARB, "glUniform2ivARB")
    && QGL_constructExtensionFunc(table.m_glUniform3ivARB, "glUniform3ivARB")
    && QGL_constructExtensionFunc(table.m_glUniform4ivARB, "glUniform4ivARB")
    && QGL_constructExtensionFunc(table.m_glUniformMatrix2fvARB, "glUniformMatrix2fvARB")
    && QGL_constructExtensionFunc(table.m_glUniformMatrix3fvARB, "glUniformMatrix3fvARB")
    && QGL_constructExtensionFunc(table.m_glUniformMatrix4fvARB, "glUniformMatrix4fvARB")
    && QGL_constructExtensionFunc(table.m_glGetObjectParameterfvARB, "glGetObjectParameterfvARB")
    && QGL_constructExtensionFunc(table.m_glGetObjectParameterivARB, "glGetObjectParameterivARB")
    && QGL_constructExtensionFunc(table.m_glGetInfoLogARB, "glGetInfoLogARB")
    && QGL_constructExtensionFunc(table.m_glGetAttachedObjectsARB, "glGetAttachedObjectsARB")
    && QGL_constructExtensionFunc(table.m_glGetUniformLocationARB, "glGetUniformLocationARB")
    && QGL_constructExtensionFunc(table.m_glGetActiveUniformARB, "glGetActiveUniformARB")
    && QGL_constructExtensionFunc(table.m_glGetUniformfvARB, "glGetUniformfvARB")
    && QGL_constructExtensionFunc(table.m_glGetUniformivARB, "glGetUniformivARB")
    && QGL_constructExtensionFunc(table.m_glGetShaderSourceARB, "glGetShaderSourceARB");

    if(!table.support_ARB_shader_objects)
    {
      extension_not_implemented("GL_ARB_shader_objects");
    }
  }
  else
  {
    table.support_ARB_shader_objects = false;
  }

  if(QGL_ExtensionSupported("GL_ARB_vertex_shader"))
  {
    table.support_ARB_vertex_shader =
    QGL_constructExtensionFunc(table.m_glVertexAttrib1fARB, "glVertexAttrib1fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1sARB, "glVertexAttrib1sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1dARB, "glVertexAttrib1dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fARB, "glVertexAttrib2fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2sARB, "glVertexAttrib2sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2dARB, "glVertexAttrib2dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fARB, "glVertexAttrib3fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3sARB, "glVertexAttrib3sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3dARB, "glVertexAttrib3dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fARB, "glVertexAttrib4fARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4sARB, "glVertexAttrib4sARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4dARB, "glVertexAttrib4dARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NubARB, "glVertexAttrib4NubARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1fvARB, "glVertexAttrib1fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1svARB, "glVertexAttrib1svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1dvARB, "glVertexAttrib1dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fvARB, "glVertexAttrib2fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2svARB, "glVertexAttrib2svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2dvARB, "glVertexAttrib2dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fvARB, "glVertexAttrib3fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3svARB, "glVertexAttrib3svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3dvARB, "glVertexAttrib3dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fvARB, "glVertexAttrib4fvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4svARB, "glVertexAttrib4svARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4dvARB, "glVertexAttrib4dvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4ivARB, "glVertexAttrib4ivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4bvARB, "glVertexAttrib4bvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4ubvARB, "glVertexAttrib4ubvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4usvARB, "glVertexAttrib4usvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4uivARB, "glVertexAttrib4uivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NbvARB, "glVertexAttrib4NbvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NsvARB, "glVertexAttrib4NsvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NivARB, "glVertexAttrib4NivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NubvARB, "glVertexAttrib4NubvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NusvARB, "glVertexAttrib4NusvARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4NuivARB, "glVertexAttrib4NuivARB")
    && QGL_constructExtensionFunc(table.m_glVertexAttribPointerARB, "glVertexAttribPointerARB")
    && QGL_constructExtensionFunc(table.m_glEnableVertexAttribArrayARB, "glEnableVertexAttribArrayARB")
    && QGL_constructExtensionFunc(table.m_glDisableVertexAttribArrayARB, "glDisableVertexAttribArrayARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribdvARB, "glGetVertexAttribdvARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribfvARB, "glGetVertexAttribfvARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribivARB, "glGetVertexAttribivARB")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribPointervARB, "glGetVertexAttribPointervARB")
    && QGL_constructExtensionFunc(table.m_glBindAttribLocationARB, "glBindAttribLocationARB")
    && QGL_constructExtensionFunc(table.m_glGetActiveAttribARB, "glGetActiveAttribARB")
    && QGL_constructExtensionFunc(table.m_glGetAttribLocationARB, "glGetAttribLocationARB");

    if(!table.support_ARB_vertex_shader)
    {
      extension_not_implemented("GL_ARB_vertex_shader");
    }
  }
  else
  {
    table.support_ARB_vertex_shader = false;
  }

  if(QGL_ExtensionSupported("GL_NV_vertex_program2"))
  {
    table.support_NV_vertex_program2 =
    QGL_constructExtensionFunc(table.m_glAreProgramsResidentNV, "glAreProgramsResidentNV")
    && QGL_constructExtensionFunc(table.m_glBindProgramNV, "glBindProgramNV")
    && QGL_constructExtensionFunc(table.m_glDeleteProgramsNV, "glDeleteProgramsNV")
    && QGL_constructExtensionFunc(table.m_glExecuteProgramNV, "glExecuteProgramNV")
    && QGL_constructExtensionFunc(table.m_glGenProgramsNV, "glGenProgramsNV")
    && QGL_constructExtensionFunc(table.m_glGetProgramParameterdvNV, "glGetProgramParameterdvNV")
    && QGL_constructExtensionFunc(table.m_glGetProgramParameterfvNV, "glGetProgramParameterfvNV")
    && QGL_constructExtensionFunc(table.m_glGetProgramivNV, "glGetProgramivNV")
    && QGL_constructExtensionFunc(table.m_glGetProgramStringNV, "glGetProgramStringNV")
    && QGL_constructExtensionFunc(table.m_glGetTrackMatrixivNV, "glGetTrackMatrixivNV")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribdvNV, "glGetVertexAttribdvNV")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribfvNV, "glGetVertexAttribfvNV")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribivNV, "glGetVertexAttribivNV")
    && QGL_constructExtensionFunc(table.m_glGetVertexAttribPointervNV, "glGetVertexAttribPointervNV")
    && QGL_constructExtensionFunc(table.m_glIsProgramNV, "glIsProgramNV")
    && QGL_constructExtensionFunc(table.m_glLoadProgramNV, "glLoadProgramNV")
    && QGL_constructExtensionFunc(table.m_glProgramParameter4fNV, "glProgramParameter4fNV")
    && QGL_constructExtensionFunc(table.m_glProgramParameter4fvNV, "glProgramParameter4fvNV")
    && QGL_constructExtensionFunc(table.m_glProgramParameters4fvNV, "glProgramParameters4fvNV")
    && QGL_constructExtensionFunc(table.m_glRequestResidentProgramsNV, "glRequestResidentProgramsNV")
    && QGL_constructExtensionFunc(table.m_glTrackMatrixNV, "glTrackMatrixNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttribPointerNV, "glVertexAttribPointerNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1fNV, "glVertexAttrib1fNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib1fvNV, "glVertexAttrib1fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fNV, "glVertexAttrib2fNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib2fvNV, "glVertexAttrib2fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fNV, "glVertexAttrib3fNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib3fvNV, "glVertexAttrib3fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fNV, "glVertexAttrib4fNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttrib4fvNV, "glVertexAttrib4fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttribs1fvNV, "glVertexAttribs1fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttribs2fvNV, "glVertexAttribs2fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttribs3fvNV, "glVertexAttribs3fvNV")
    && QGL_constructExtensionFunc(table.m_glVertexAttribs4fvNV, "glVertexAttribs4fvNV");

    if(!table.support_NV_vertex_program2)
    {
      extension_not_implemented("GL_NV_vertex_program2");
    }
  }
  else
  {
    table.support_NV_vertex_program2 = false;
    QGL_invalidateExtensionFunc(table.m_glAreProgramsResidentNV);
    QGL_invalidateExtensionFunc(table.m_glBindProgramNV);
    QGL_invalidateExtensionFunc(table.m_glDeleteProgramsNV);
    QGL_invalidateExtensionFunc(table.m_glExecuteProgramNV);
    QGL_invalidateExtensionFunc(table.m_glGenProgramsNV);
    QGL_invalidateExtensionFunc(table.m_glGetProgramParameterdvNV);
    QGL_invalidateExtensionFunc(table.m_glGetProgramParameterfvNV);
    QGL_invalidateExtensionFunc(table.m_glGetProgramivNV);
    QGL_invalidateExtensionFunc(table.m_glGetProgramStringNV);
    QGL_invalidateExtensionFunc(table.m_glGetTrackMatrixivNV);
    QGL_invalidateExtensionFunc(table.m_glGetVertexAttribdvNV);
    QGL_invalidateExtensionFunc(table.m_glGetVertexAttribfvNV);
    QGL_invalidateExtensionFunc(table.m_glGetVertexAttribivNV);
    QGL_invalidateExtensionFunc(table.m_glGetVertexAttribPointervNV);
    QGL_invalidateExtensionFunc(table.m_glIsProgramNV);
    QGL_invalidateExtensionFunc(table.m_glLoadProgramNV);
    QGL_invalidateExtensionFunc(table.m_glProgramParameter4fNV);
    QGL_invalidateExtensionFunc(table.m_glProgramParameter4fvNV);
    QGL_invalidateExtensionFunc(table.m_glProgramParameters4fvNV);
    QGL_invalidateExtensionFunc(table.m_glRequestResidentProgramsNV);
    QGL_invalidateExtensionFunc(table.m_glTrackMatrixNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttribPointerNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib1fNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib1fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib2fNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib2fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib3fNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib3fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib4fNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttrib4fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttribs1fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttribs2fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttribs3fvNV);
    QGL_invalidateExtensionFunc(table.m_glVertexAttribs4fvNV);
  }

  if(QGL_ExtensionSupported("GL_NV_fragment_program"))
  {
    table.support_NV_fragment_program = 
    QGL_constructExtensionFunc(table.m_glProgramNamedParameter4fNV, "glProgramNamedParameter4fNV")
    && QGL_constructExtensionFunc(table.m_glProgramNamedParameter4fvNV, "glProgramNamedParameter4fvNV")
    && QGL_constructExtensionFunc(table.m_glGetProgramNamedParameterfvNV, "glGetProgramNamedParameterfvNV");

    if(!table.support_NV_fragment_program)
    {
      extension_not_implemented("GL_NV_fragment_program");
    }
  }
  else
  {
    table.support_NV_fragment_program = false;
  }

  table.support_ARB_fragment_shader = QGL_ExtensionSupported("GL_ARB_fragment_shader");
  table.support_ARB_shading_language_100 = QGL_ExtensionSupported("GL_ARB_shading_language_100");
}

void QGL_sharedContextDestroyed(OpenGLBinding& table)
{
  QGL_clear(table);
}


void QGL_assertNoErrors()
{
  GLenum error = GlobalOpenGL().m_glGetError();
  while (error != GL_NO_ERROR)
  {
    const char* errorString = reinterpret_cast<const char*>(qgluErrorString(error));
    if (error == GL_OUT_OF_MEMORY)
    {
      ERROR_MESSAGE("OpenGL out of memory error: " << errorString);
    }
    else
    {
      ERROR_MESSAGE("OpenGL error: " << errorString);
    }
    error = GlobalOpenGL().m_glGetError();
  }
}


class QglAPI
{
  OpenGLBinding m_qgl;
public:
  typedef OpenGLBinding Type;
  STRING_CONSTANT(Name, "*");

  QglAPI()
  {
    QGL_Init(m_qgl);

    m_qgl.assertNoErrors = &QGL_assertNoErrors;
  }
  ~QglAPI()
  {
    QGL_Shutdown(m_qgl);
  }
  OpenGLBinding* getTable()
  {
    return &m_qgl;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<QglAPI> QglModule;
typedef Static<QglModule> StaticQglModule;
StaticRegisterModule staticRegisterQgl(StaticQglModule::instance());
