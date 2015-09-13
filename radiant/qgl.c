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

/*
** QGL_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/

/*
 * This causes glDisable(), glEnable(), glCullFace() and glPolygonMode() to
 * be wrapped in order to get around a bug in ATI's FireGL drivers.
 */
#include <stdio.h>
#include <float.h>
#include <string.h>
#if defined ( __linux__ ) || defined ( __APPLE__ )
#include <dlfcn.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "qgl.h"
void Sys_Printf( const char *format, ... );

#ifdef _WIN32
HMODULE g_hGLDLL = NULL;

#pragma warning (disable : 4113 4133 4047 4018 )

int ( WINAPI * qwglChoosePixelFormat )( HDC, CONST PIXELFORMATDESCRIPTOR * );
int ( WINAPI * qwglDescribePixelFormat )( HDC, int, UINT, LPPIXELFORMATDESCRIPTOR );
int ( WINAPI * qwglGetPixelFormat )( HDC );
BOOL ( WINAPI * qwglSetPixelFormat )( HDC, int, CONST PIXELFORMATDESCRIPTOR * );
BOOL ( WINAPI * qwglSwapBuffers )( HDC );

BOOL ( WINAPI * qwglCopyContext )( HGLRC, HGLRC, UINT );
HGLRC ( WINAPI * qwglCreateContext )( HDC );
HGLRC ( WINAPI * qwglCreateLayerContext )( HDC, int );
BOOL ( WINAPI * qwglDeleteContext )( HGLRC );
HGLRC ( WINAPI * qwglGetCurrentContext )( VOID );
HDC ( WINAPI * qwglGetCurrentDC )( VOID );
PROC ( WINAPI * qwglGetProcAddress )( LPCSTR );
BOOL ( WINAPI * qwglMakeCurrent )( HDC, HGLRC );
BOOL ( WINAPI * qwglShareLists )( HGLRC, HGLRC );
BOOL ( WINAPI * qwglUseFontBitmaps )( HDC, DWORD, DWORD, DWORD );

BOOL ( WINAPI * qwglUseFontOutlines )( HDC, DWORD, DWORD, DWORD, FLOAT,
									   FLOAT, int, LPGLYPHMETRICSFLOAT );

BOOL ( WINAPI * qwglDescribeLayerPlane )( HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR );
int ( WINAPI * qwglSetLayerPaletteEntries )( HDC, int, int, int, CONST COLORREF * );
int ( WINAPI * qwglGetLayerPaletteEntries )( HDC, int, int, int, COLORREF * );
BOOL ( WINAPI * qwglRealizeLayerPalette )( HDC, int, BOOL );
BOOL ( WINAPI * qwglSwapLayerBuffers )( HDC, UINT );

BOOL ( WINAPI * qwglGetDeviceGammaRampEXT )( unsigned char *, unsigned char *, unsigned char * );
BOOL ( WINAPI * qwglSetDeviceGammaRampEXT )( const unsigned char *, const unsigned char *,
											 const unsigned char * );
BOOL ( WINAPI * qwglSwapIntervalEXT )( int interval );

#else
#define WINAPI
#endif

#if defined ( __linux__ ) || defined ( __APPLE__ )
void* g_hGLDLL;

XVisualInfo* ( *qglXChooseVisual )( Display * dpy, int screen, int *attribList );
GLXContext ( *qglXCreateContext )( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct );
void ( *qglXDestroyContext )( Display *dpy, GLXContext ctx );
Bool ( *qglXMakeCurrent )( Display *dpy, GLXDrawable drawable, GLXContext ctx );
void ( *qglXCopyContext )( Display *dpy, GLXContext src, GLXContext dst, GLuint mask );
void ( *qglXSwapBuffers )( Display *dpy, GLXDrawable drawable );
GLXPixmap ( *qglXCreateGLXPixmap )( Display *dpy, XVisualInfo *visual, Pixmap pixmap );
void ( *qglXDestroyGLXPixmap )( Display *dpy, GLXPixmap pixmap );
Bool ( *qglXQueryExtension )( Display *dpy, int *errorb, int *event );
Bool ( *qglXQueryVersion )( Display *dpy, int *maj, int *min );
Bool ( *qglXIsDirect )( Display *dpy, GLXContext ctx );
int ( *qglXGetConfig )( Display *dpy, XVisualInfo *visual, int attrib, int *value );
GLXContext ( *qglXGetCurrentContext )( void );
GLXDrawable ( *qglXGetCurrentDrawable )( void );
void ( *qglXWaitGL )( void );
void ( *qglXWaitX )( void );
void ( *qglXUseXFont )( Font font, int first, int count, int list );
void*        ( *qglXGetProcAddressARB )( const GLubyte * procName );
#endif

void ( APIENTRY * qglAccum )( GLenum op, GLfloat value );
void ( APIENTRY * qglAlphaFunc )( GLenum func, GLclampf ref );
GLboolean ( APIENTRY * qglAreTexturesResident )( GLsizei n, const GLuint *textures, GLboolean *residences );
void ( APIENTRY * qglArrayElement )( GLint i );
void ( APIENTRY * qglBegin )( GLenum mode );
void ( APIENTRY * qglBindTexture )( GLenum target, GLuint texture );
void ( APIENTRY * qglBitmap )( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
void ( APIENTRY * qglBlendFunc )( GLenum sfactor, GLenum dfactor );
void ( APIENTRY * qglCallList )( GLuint list );
void ( APIENTRY * qglCallLists )( GLsizei n, GLenum type, const GLvoid *lists );
void ( APIENTRY * qglClear )( GLbitfield mask );
void ( APIENTRY * qglClearAccum )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
void ( APIENTRY * qglClearColor )( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
void ( APIENTRY * qglClearDepth )( GLclampd depth );
void ( APIENTRY * qglClearIndex )( GLfloat c );
void ( APIENTRY * qglClearStencil )( GLint s );
void ( APIENTRY * qglClipPlane )( GLenum plane, const GLdouble *equation );
void ( APIENTRY * qglColor3b )( GLbyte red, GLbyte green, GLbyte blue );
void ( APIENTRY * qglColor3bv )( const GLbyte *v );
void ( APIENTRY * qglColor3d )( GLdouble red, GLdouble green, GLdouble blue );
void ( APIENTRY * qglColor3dv )( const GLdouble *v );
void ( APIENTRY * qglColor3f )( GLfloat red, GLfloat green, GLfloat blue );
void ( APIENTRY * qglColor3fv )( const GLfloat *v );
void ( APIENTRY * qglColor3i )( GLint red, GLint green, GLint blue );
void ( APIENTRY * qglColor3iv )( const GLint *v );
void ( APIENTRY * qglColor3s )( GLshort red, GLshort green, GLshort blue );
void ( APIENTRY * qglColor3sv )( const GLshort *v );
void ( APIENTRY * qglColor3ub )( GLubyte red, GLubyte green, GLubyte blue );
void ( APIENTRY * qglColor3ubv )( const GLubyte *v );
void ( APIENTRY * qglColor3ui )( GLuint red, GLuint green, GLuint blue );
void ( APIENTRY * qglColor3uiv )( const GLuint *v );
void ( APIENTRY * qglColor3us )( GLushort red, GLushort green, GLushort blue );
void ( APIENTRY * qglColor3usv )( const GLushort *v );
void ( APIENTRY * qglColor4b )( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha );
void ( APIENTRY * qglColor4bv )( const GLbyte *v );
void ( APIENTRY * qglColor4d )( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha );
void ( APIENTRY * qglColor4dv )( const GLdouble *v );
void ( APIENTRY * qglColor4f )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
void ( APIENTRY * qglColor4fv )( const GLfloat *v );
void ( APIENTRY * qglColor4i )( GLint red, GLint green, GLint blue, GLint alpha );
void ( APIENTRY * qglColor4iv )( const GLint *v );
void ( APIENTRY * qglColor4s )( GLshort red, GLshort green, GLshort blue, GLshort alpha );
void ( APIENTRY * qglColor4sv )( const GLshort *v );
void ( APIENTRY * qglColor4ub )( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
void ( APIENTRY * qglColor4ubv )( const GLubyte *v );
void ( APIENTRY * qglColor4ui )( GLuint red, GLuint green, GLuint blue, GLuint alpha );
void ( APIENTRY * qglColor4uiv )( const GLuint *v );
void ( APIENTRY * qglColor4us )( GLushort red, GLushort green, GLushort blue, GLushort alpha );
void ( APIENTRY * qglColor4usv )( const GLushort *v );
void ( APIENTRY * qglColorMask )( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
void ( APIENTRY * qglColorMaterial )( GLenum face, GLenum mode );
void ( APIENTRY * qglColorPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglCopyPixels )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );
void ( APIENTRY * qglCopyTexImage1D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border );
void ( APIENTRY * qglCopyTexImage2D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border );
void ( APIENTRY * qglCopyTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width );
void ( APIENTRY * qglCopyTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
void ( APIENTRY * qglCullFace )( GLenum mode );
void ( APIENTRY * qglDeleteLists )( GLuint list, GLsizei range );
void ( APIENTRY * qglDeleteTextures )( GLsizei n, const GLuint *textures );
void ( APIENTRY * qglDepthFunc )( GLenum func );
void ( APIENTRY * qglDepthMask )( GLboolean flag );
void ( APIENTRY * qglDepthRange )( GLclampd zNear, GLclampd zFar );
void ( APIENTRY * qglDisable )( GLenum cap );
void ( APIENTRY * qglDisableClientState )( GLenum array );
void ( APIENTRY * qglDrawArrays )( GLenum mode, GLint first, GLsizei count );
void ( APIENTRY * qglDrawBuffer )( GLenum mode );
void ( APIENTRY * qglDrawElements )( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
void ( APIENTRY * qglDrawPixels )( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
void ( APIENTRY * qglEdgeFlag )( GLboolean flag );
void ( APIENTRY * qglEdgeFlagPointer )( GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglEdgeFlagv )( const GLboolean *flag );
void ( APIENTRY * qglEnable )( GLenum cap );
void ( APIENTRY * qglEnableClientState )( GLenum array );
void ( APIENTRY * qglEnd )( void );
void ( APIENTRY * qglEndList )( void );
void ( APIENTRY * qglEvalCoord1d )( GLdouble u );
void ( APIENTRY * qglEvalCoord1dv )( const GLdouble *u );
void ( APIENTRY * qglEvalCoord1f )( GLfloat u );
void ( APIENTRY * qglEvalCoord1fv )( const GLfloat *u );
void ( APIENTRY * qglEvalCoord2d )( GLdouble u, GLdouble v );
void ( APIENTRY * qglEvalCoord2dv )( const GLdouble *u );
void ( APIENTRY * qglEvalCoord2f )( GLfloat u, GLfloat v );
void ( APIENTRY * qglEvalCoord2fv )( const GLfloat *u );
void ( APIENTRY * qglEvalMesh1 )( GLenum mode, GLint i1, GLint i2 );
void ( APIENTRY * qglEvalMesh2 )( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );
void ( APIENTRY * qglEvalPoint1 )( GLint i );
void ( APIENTRY * qglEvalPoint2 )( GLint i, GLint j );
void ( APIENTRY * qglFeedbackBuffer )( GLsizei size, GLenum type, GLfloat *buffer );
void ( APIENTRY * qglFinish )( void );
void ( APIENTRY * qglFlush )( void );
void ( APIENTRY * qglFogf )( GLenum pname, GLfloat param );
void ( APIENTRY * qglFogfv )( GLenum pname, const GLfloat *params );
void ( APIENTRY * qglFogi )( GLenum pname, GLint param );
void ( APIENTRY * qglFogiv )( GLenum pname, const GLint *params );
void ( APIENTRY * qglFrontFace )( GLenum mode );
void ( APIENTRY * qglFrustum )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
GLuint ( APIENTRY * qglGenLists )( GLsizei range );
void ( APIENTRY * qglGenTextures )( GLsizei n, GLuint *textures );
void ( APIENTRY * qglGetBooleanv )( GLenum pname, GLboolean *params );
void ( APIENTRY * qglGetClipPlane )( GLenum plane, GLdouble *equation );
void ( APIENTRY * qglGetDoublev )( GLenum pname, GLdouble *params );
GLenum ( APIENTRY * qglGetError )( void );
void ( APIENTRY * qglGetFloatv )( GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetIntegerv )( GLenum pname, GLint *params );
void ( APIENTRY * qglGetLightfv )( GLenum light, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetLightiv )( GLenum light, GLenum pname, GLint *params );
void ( APIENTRY * qglGetMapdv )( GLenum target, GLenum query, GLdouble *v );
void ( APIENTRY * qglGetMapfv )( GLenum target, GLenum query, GLfloat *v );
void ( APIENTRY * qglGetMapiv )( GLenum target, GLenum query, GLint *v );
void ( APIENTRY * qglGetMaterialfv )( GLenum face, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetMaterialiv )( GLenum face, GLenum pname, GLint *params );
void ( APIENTRY * qglGetPixelMapfv )( GLenum map, GLfloat *values );
void ( APIENTRY * qglGetPixelMapuiv )( GLenum map, GLuint *values );
void ( APIENTRY * qglGetPixelMapusv )( GLenum map, GLushort *values );
void ( APIENTRY * qglGetPointerv )( GLenum pname, GLvoid* *params );
void ( APIENTRY * qglGetPolygonStipple )( GLubyte *mask );
const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglGetTexEnvfv )( GLenum target, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetTexEnviv )( GLenum target, GLenum pname, GLint *params );
void ( APIENTRY * qglGetTexGendv )( GLenum coord, GLenum pname, GLdouble *params );
void ( APIENTRY * qglGetTexGenfv )( GLenum coord, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetTexGeniv )( GLenum coord, GLenum pname, GLint *params );
void ( APIENTRY * qglGetTexImage )( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels );
void ( APIENTRY * qglGetTexLevelParameterfv )( GLenum target, GLint level, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetTexLevelParameteriv )( GLenum target, GLint level, GLenum pname, GLint *params );
void ( APIENTRY * qglGetTexParameterfv )( GLenum target, GLenum pname, GLfloat *params );
void ( APIENTRY * qglGetTexParameteriv )( GLenum target, GLenum pname, GLint *params );
void ( APIENTRY * qglHint )( GLenum target, GLenum mode );
void ( APIENTRY * qglIndexMask )( GLuint mask );
void ( APIENTRY * qglIndexPointer )( GLenum type, GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglIndexd )( GLdouble c );
void ( APIENTRY * qglIndexdv )( const GLdouble *c );
void ( APIENTRY * qglIndexf )( GLfloat c );
void ( APIENTRY * qglIndexfv )( const GLfloat *c );
void ( APIENTRY * qglIndexi )( GLint c );
void ( APIENTRY * qglIndexiv )( const GLint *c );
void ( APIENTRY * qglIndexs )( GLshort c );
void ( APIENTRY * qglIndexsv )( const GLshort *c );
void ( APIENTRY * qglIndexub )( GLubyte c );
void ( APIENTRY * qglIndexubv )( const GLubyte *c );
void ( APIENTRY * qglInitNames )( void );
void ( APIENTRY * qglInterleavedArrays )( GLenum format, GLsizei stride, const GLvoid *pointer );
GLboolean ( APIENTRY * qglIsEnabled )( GLenum cap );
GLboolean ( APIENTRY * qglIsList )( GLuint list );
GLboolean ( APIENTRY * qglIsTexture )( GLuint texture );
void ( APIENTRY * qglLightModelf )( GLenum pname, GLfloat param );
void ( APIENTRY * qglLightModelfv )( GLenum pname, const GLfloat *params );
void ( APIENTRY * qglLightModeli )( GLenum pname, GLint param );
void ( APIENTRY * qglLightModeliv )( GLenum pname, const GLint *params );
void ( APIENTRY * qglLightf )( GLenum light, GLenum pname, GLfloat param );
void ( APIENTRY * qglLightfv )( GLenum light, GLenum pname, const GLfloat *params );
void ( APIENTRY * qglLighti )( GLenum light, GLenum pname, GLint param );
void ( APIENTRY * qglLightiv )( GLenum light, GLenum pname, const GLint *params );
void ( APIENTRY * qglLineStipple )( GLint factor, GLushort pattern );
void ( APIENTRY * qglLineWidth )( GLfloat width );
void ( APIENTRY * qglListBase )( GLuint base );
void ( APIENTRY * qglLoadIdentity )( void );
void ( APIENTRY * qglLoadMatrixd )( const GLdouble *m );
void ( APIENTRY * qglLoadMatrixf )( const GLfloat *m );
void ( APIENTRY * qglLoadName )( GLuint name );
void ( APIENTRY * qglLogicOp )( GLenum opcode );
void ( APIENTRY * qglMap1d )( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points );
void ( APIENTRY * qglMap1f )( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points );
void ( APIENTRY * qglMap2d )( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points );
void ( APIENTRY * qglMap2f )( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points );
void ( APIENTRY * qglMapGrid1d )( GLint un, GLdouble u1, GLdouble u2 );
void ( APIENTRY * qglMapGrid1f )( GLint un, GLfloat u1, GLfloat u2 );
void ( APIENTRY * qglMapGrid2d )( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 );
void ( APIENTRY * qglMapGrid2f )( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 );
void ( APIENTRY * qglMaterialf )( GLenum face, GLenum pname, GLfloat param );
void ( APIENTRY * qglMaterialfv )( GLenum face, GLenum pname, const GLfloat *params );
void ( APIENTRY * qglMateriali )( GLenum face, GLenum pname, GLint param );
void ( APIENTRY * qglMaterialiv )( GLenum face, GLenum pname, const GLint *params );
void ( APIENTRY * qglMatrixMode )( GLenum mode );
void ( APIENTRY * qglMultMatrixd )( const GLdouble *m );
void ( APIENTRY * qglMultMatrixf )( const GLfloat *m );
void ( APIENTRY * qglNewList )( GLuint list, GLenum mode );
void ( APIENTRY * qglNormal3b )( GLbyte nx, GLbyte ny, GLbyte nz );
void ( APIENTRY * qglNormal3bv )( const GLbyte *v );
void ( APIENTRY * qglNormal3d )( GLdouble nx, GLdouble ny, GLdouble nz );
void ( APIENTRY * qglNormal3dv )( const GLdouble *v );
void ( APIENTRY * qglNormal3f )( GLfloat nx, GLfloat ny, GLfloat nz );
void ( APIENTRY * qglNormal3fv )( const GLfloat *v );
void ( APIENTRY * qglNormal3i )( GLint nx, GLint ny, GLint nz );
void ( APIENTRY * qglNormal3iv )( const GLint *v );
void ( APIENTRY * qglNormal3s )( GLshort nx, GLshort ny, GLshort nz );
void ( APIENTRY * qglNormal3sv )( const GLshort *v );
void ( APIENTRY * qglNormalPointer )( GLenum type, GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglOrtho )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
void ( APIENTRY * qglPassThrough )( GLfloat token );
void ( APIENTRY * qglPixelMapfv )( GLenum map, GLsizei mapsize, const GLfloat *values );
void ( APIENTRY * qglPixelMapuiv )( GLenum map, GLsizei mapsize, const GLuint *values );
void ( APIENTRY * qglPixelMapusv )( GLenum map, GLsizei mapsize, const GLushort *values );
void ( APIENTRY * qglPixelStoref )( GLenum pname, GLfloat param );
void ( APIENTRY * qglPixelStorei )( GLenum pname, GLint param );
void ( APIENTRY * qglPixelTransferf )( GLenum pname, GLfloat param );
void ( APIENTRY * qglPixelTransferi )( GLenum pname, GLint param );
void ( APIENTRY * qglPixelZoom )( GLfloat xfactor, GLfloat yfactor );
void ( APIENTRY * qglPointSize )( GLfloat size );
void ( APIENTRY * qglPolygonMode )( GLenum face, GLenum mode );
void ( APIENTRY * qglPolygonOffset )( GLfloat factor, GLfloat units );
void ( APIENTRY * qglPolygonStipple )( const GLubyte *mask );
void ( APIENTRY * qglPopAttrib )( void );
void ( APIENTRY * qglPopClientAttrib )( void );
void ( APIENTRY * qglPopMatrix )( void );
void ( APIENTRY * qglPopName )( void );
void ( APIENTRY * qglPrioritizeTextures )( GLsizei n, const GLuint *textures, const GLclampf *priorities );
void ( APIENTRY * qglPushAttrib )( GLbitfield mask );
void ( APIENTRY * qglPushClientAttrib )( GLbitfield mask );
void ( APIENTRY * qglPushMatrix )( void );
void ( APIENTRY * qglPushName )( GLuint name );
void ( APIENTRY * qglRasterPos2d )( GLdouble x, GLdouble y );
void ( APIENTRY * qglRasterPos2dv )( const GLdouble *v );
void ( APIENTRY * qglRasterPos2f )( GLfloat x, GLfloat y );
void ( APIENTRY * qglRasterPos2fv )( const GLfloat *v );
void ( APIENTRY * qglRasterPos2i )( GLint x, GLint y );
void ( APIENTRY * qglRasterPos2iv )( const GLint *v );
void ( APIENTRY * qglRasterPos2s )( GLshort x, GLshort y );
void ( APIENTRY * qglRasterPos2sv )( const GLshort *v );
void ( APIENTRY * qglRasterPos3d )( GLdouble x, GLdouble y, GLdouble z );
void ( APIENTRY * qglRasterPos3dv )( const GLdouble *v );
void ( APIENTRY * qglRasterPos3f )( GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRY * qglRasterPos3fv )( const GLfloat *v );
void ( APIENTRY * qglRasterPos3i )( GLint x, GLint y, GLint z );
void ( APIENTRY * qglRasterPos3iv )( const GLint *v );
void ( APIENTRY * qglRasterPos3s )( GLshort x, GLshort y, GLshort z );
void ( APIENTRY * qglRasterPos3sv )( const GLshort *v );
void ( APIENTRY * qglRasterPos4d )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
void ( APIENTRY * qglRasterPos4dv )( const GLdouble *v );
void ( APIENTRY * qglRasterPos4f )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
void ( APIENTRY * qglRasterPos4fv )( const GLfloat *v );
void ( APIENTRY * qglRasterPos4i )( GLint x, GLint y, GLint z, GLint w );
void ( APIENTRY * qglRasterPos4iv )( const GLint *v );
void ( APIENTRY * qglRasterPos4s )( GLshort x, GLshort y, GLshort z, GLshort w );
void ( APIENTRY * qglRasterPos4sv )( const GLshort *v );
void ( APIENTRY * qglReadBuffer )( GLenum mode );
void ( APIENTRY * qglReadPixels )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
void ( APIENTRY * qglRectd )( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
void ( APIENTRY * qglRectdv )( const GLdouble *v1, const GLdouble *v2 );
void ( APIENTRY * qglRectf )( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
void ( APIENTRY * qglRectfv )( const GLfloat *v1, const GLfloat *v2 );
void ( APIENTRY * qglRecti )( GLint x1, GLint y1, GLint x2, GLint y2 );
void ( APIENTRY * qglRectiv )( const GLint *v1, const GLint *v2 );
void ( APIENTRY * qglRects )( GLshort x1, GLshort y1, GLshort x2, GLshort y2 );
void ( APIENTRY * qglRectsv )( const GLshort *v1, const GLshort *v2 );
GLint ( APIENTRY * qglRenderMode )( GLenum mode );
void ( APIENTRY * qglRotated )( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );
void ( APIENTRY * qglRotatef )( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRY * qglScaled )( GLdouble x, GLdouble y, GLdouble z );
void ( APIENTRY * qglScalef )( GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRY * qglScissor )( GLint x, GLint y, GLsizei width, GLsizei height );
void ( APIENTRY * qglSelectBuffer )( GLsizei size, GLuint *buffer );
void ( APIENTRY * qglShadeModel )( GLenum mode );
void ( APIENTRY * qglStencilFunc )( GLenum func, GLint ref, GLuint mask );
void ( APIENTRY * qglStencilMask )( GLuint mask );
void ( APIENTRY * qglStencilOp )( GLenum fail, GLenum zfail, GLenum zpass );
void ( APIENTRY * qglTexCoord1d )( GLdouble s );
void ( APIENTRY * qglTexCoord1dv )( const GLdouble *v );
void ( APIENTRY * qglTexCoord1f )( GLfloat s );
void ( APIENTRY * qglTexCoord1fv )( const GLfloat *v );
void ( APIENTRY * qglTexCoord1i )( GLint s );
void ( APIENTRY * qglTexCoord1iv )( const GLint *v );
void ( APIENTRY * qglTexCoord1s )( GLshort s );
void ( APIENTRY * qglTexCoord1sv )( const GLshort *v );
void ( APIENTRY * qglTexCoord2d )( GLdouble s, GLdouble t );
void ( APIENTRY * qglTexCoord2dv )( const GLdouble *v );
void ( APIENTRY * qglTexCoord2f )( GLfloat s, GLfloat t );
void ( APIENTRY * qglTexCoord2fv )( const GLfloat *v );
void ( APIENTRY * qglTexCoord2i )( GLint s, GLint t );
void ( APIENTRY * qglTexCoord2iv )( const GLint *v );
void ( APIENTRY * qglTexCoord2s )( GLshort s, GLshort t );
void ( APIENTRY * qglTexCoord2sv )( const GLshort *v );
void ( APIENTRY * qglTexCoord3d )( GLdouble s, GLdouble t, GLdouble r );
void ( APIENTRY * qglTexCoord3dv )( const GLdouble *v );
void ( APIENTRY * qglTexCoord3f )( GLfloat s, GLfloat t, GLfloat r );
void ( APIENTRY * qglTexCoord3fv )( const GLfloat *v );
void ( APIENTRY * qglTexCoord3i )( GLint s, GLint t, GLint r );
void ( APIENTRY * qglTexCoord3iv )( const GLint *v );
void ( APIENTRY * qglTexCoord3s )( GLshort s, GLshort t, GLshort r );
void ( APIENTRY * qglTexCoord3sv )( const GLshort *v );
void ( APIENTRY * qglTexCoord4d )( GLdouble s, GLdouble t, GLdouble r, GLdouble q );
void ( APIENTRY * qglTexCoord4dv )( const GLdouble *v );
void ( APIENTRY * qglTexCoord4f )( GLfloat s, GLfloat t, GLfloat r, GLfloat q );
void ( APIENTRY * qglTexCoord4fv )( const GLfloat *v );
void ( APIENTRY * qglTexCoord4i )( GLint s, GLint t, GLint r, GLint q );
void ( APIENTRY * qglTexCoord4iv )( const GLint *v );
void ( APIENTRY * qglTexCoord4s )( GLshort s, GLshort t, GLshort r, GLshort q );
void ( APIENTRY * qglTexCoord4sv )( const GLshort *v );
void ( APIENTRY * qglTexCoordPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglTexEnvf )( GLenum target, GLenum pname, GLfloat param );
void ( APIENTRY * qglTexEnvfv )( GLenum target, GLenum pname, const GLfloat *params );
void ( APIENTRY * qglTexEnvi )( GLenum target, GLenum pname, GLint param );
void ( APIENTRY * qglTexEnviv )( GLenum target, GLenum pname, const GLint *params );
void ( APIENTRY * qglTexGend )( GLenum coord, GLenum pname, GLdouble param );
void ( APIENTRY * qglTexGendv )( GLenum coord, GLenum pname, const GLdouble *params );
void ( APIENTRY * qglTexGenf )( GLenum coord, GLenum pname, GLfloat param );
void ( APIENTRY * qglTexGenfv )( GLenum coord, GLenum pname, const GLfloat *params );
void ( APIENTRY * qglTexGeni )( GLenum coord, GLenum pname, GLint param );
void ( APIENTRY * qglTexGeniv )( GLenum coord, GLenum pname, const GLint *params );
void ( APIENTRY * qglTexImage1D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
void ( APIENTRY * qglTexImage2D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
void ( APIENTRY * qglTexParameterf )( GLenum target, GLenum pname, GLfloat param );
void ( APIENTRY * qglTexParameterfv )( GLenum target, GLenum pname, const GLfloat *params );
void ( APIENTRY * qglTexParameteri )( GLenum target, GLenum pname, GLint param );
void ( APIENTRY * qglTexParameteriv )( GLenum target, GLenum pname, const GLint *params );
void ( APIENTRY * qglTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels );
void ( APIENTRY * qglTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
void ( APIENTRY * qglTranslated )( GLdouble x, GLdouble y, GLdouble z );
void ( APIENTRY * qglTranslatef )( GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRY * qglVertex2d )( GLdouble x, GLdouble y );
void ( APIENTRY * qglVertex2dv )( const GLdouble *v );
void ( APIENTRY * qglVertex2f )( GLfloat x, GLfloat y );
void ( APIENTRY * qglVertex2fv )( const GLfloat *v );
void ( APIENTRY * qglVertex2i )( GLint x, GLint y );
void ( APIENTRY * qglVertex2iv )( const GLint *v );
void ( APIENTRY * qglVertex2s )( GLshort x, GLshort y );
void ( APIENTRY * qglVertex2sv )( const GLshort *v );
void ( APIENTRY * qglVertex3d )( GLdouble x, GLdouble y, GLdouble z );
void ( APIENTRY * qglVertex3dv )( const GLdouble *v );
void ( APIENTRY * qglVertex3f )( GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRY * qglVertex3fv )( const GLfloat *v );
void ( APIENTRY * qglVertex3i )( GLint x, GLint y, GLint z );
void ( APIENTRY * qglVertex3iv )( const GLint *v );
void ( APIENTRY * qglVertex3s )( GLshort x, GLshort y, GLshort z );
void ( APIENTRY * qglVertex3sv )( const GLshort *v );
void ( APIENTRY * qglVertex4d )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
void ( APIENTRY * qglVertex4dv )( const GLdouble *v );
void ( APIENTRY * qglVertex4f )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
void ( APIENTRY * qglVertex4fv )( const GLfloat *v );
void ( APIENTRY * qglVertex4i )( GLint x, GLint y, GLint z, GLint w );
void ( APIENTRY * qglVertex4iv )( const GLint *v );
void ( APIENTRY * qglVertex4s )( GLshort x, GLshort y, GLshort z, GLshort w );
void ( APIENTRY * qglVertex4sv )( const GLshort *v );
void ( APIENTRY * qglVertexPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
void ( APIENTRY * qglViewport )( GLint x, GLint y, GLsizei width, GLsizei height );

void ( APIENTRY * qglPointParameterfEXT )( GLenum param, GLfloat value );
void ( APIENTRY * qglPointParameterfvEXT )( GLenum param, const GLfloat *value );
void ( APIENTRY * qglColorTableEXT )( int, int, int, int, int, const void * );
void ( APIENTRY * qglSelectTextureSGIS )( GLenum );
void ( APIENTRY * qglMTexCoord2fSGIS )( GLenum, GLfloat, GLfloat );

void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglMultiTexCoord1dARB )( GLenum target, GLdouble s );
void ( APIENTRY * qglMultiTexCoord1dvARB )( GLenum target, const GLdouble *v );
void ( APIENTRY * qglMultiTexCoord1fARB )( GLenum target, GLfloat s );
void ( APIENTRY * qglMultiTexCoord1fvARB )( GLenum target, const GLfloat *v );
void ( APIENTRY * qglMultiTexCoord1iARB )( GLenum target, GLint s );
void ( APIENTRY * qglMultiTexCoord1ivARB )( GLenum target, const GLint *v );
void ( APIENTRY * qglMultiTexCoord1sARB )( GLenum target, GLshort s );
void ( APIENTRY * qglMultiTexCoord1svARB )( GLenum target, const GLshort *v );
void ( APIENTRY * qglMultiTexCoord2dARB )( GLenum target, GLdouble s );
void ( APIENTRY * qglMultiTexCoord2dvARB )( GLenum target, const GLdouble *v );
void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum target, GLfloat s );
void ( APIENTRY * qglMultiTexCoord2fvARB )( GLenum target, const GLfloat *v );
void ( APIENTRY * qglMultiTexCoord2iARB )( GLenum target, GLint s );
void ( APIENTRY * qglMultiTexCoord2ivARB )( GLenum target, const GLint *v );
void ( APIENTRY * qglMultiTexCoord2sARB )( GLenum target, GLshort s );
void ( APIENTRY * qglMultiTexCoord2svARB )( GLenum target, const GLshort *v );
void ( APIENTRY * qglMultiTexCoord3dARB )( GLenum target, GLdouble s );
void ( APIENTRY * qglMultiTexCoord3dvARB )( GLenum target, const GLdouble *v );
void ( APIENTRY * qglMultiTexCoord3fARB )( GLenum target, GLfloat s );
void ( APIENTRY * qglMultiTexCoord3fvARB )( GLenum target, const GLfloat *v );
void ( APIENTRY * qglMultiTexCoord3iARB )( GLenum target, GLint s );
void ( APIENTRY * qglMultiTexCoord3ivARB )( GLenum target, const GLint *v );
void ( APIENTRY * qglMultiTexCoord3sARB )( GLenum target, GLshort s );
void ( APIENTRY * qglMultiTexCoord3svARB )( GLenum target, const GLshort *v );
void ( APIENTRY * qglMultiTexCoord4dARB )( GLenum target, GLdouble s );
void ( APIENTRY * qglMultiTexCoord4dvARB )( GLenum target, const GLdouble *v );
void ( APIENTRY * qglMultiTexCoord4fARB )( GLenum target, GLfloat s );
void ( APIENTRY * qglMultiTexCoord4fvARB )( GLenum target, const GLfloat *v );
void ( APIENTRY * qglMultiTexCoord4iARB )( GLenum target, GLint s );
void ( APIENTRY * qglMultiTexCoord4ivARB )( GLenum target, const GLint *v );
void ( APIENTRY * qglMultiTexCoord4sARB )( GLenum target, GLshort s );
void ( APIENTRY * qglMultiTexCoord4svARB )( GLenum target, const GLshort *v );

// glu stuff
void ( APIENTRY * qgluPerspective )( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar );

// added for plugins
void ( APIENTRY * qgluLookAt )(
	GLdouble eyex,
	GLdouble eyey,
	GLdouble eyez,
	GLdouble centerx,
	GLdouble centery,
	GLdouble centerz,
	GLdouble upx,
	GLdouble upy,
	GLdouble upz );
const GLubyte* ( APIENTRY * qgluErrorString )(GLenum errCode );

#ifdef ATIHACK_812
void ( APIENTRY * qglCullFace_real )( GLenum mode );
void ( APIENTRY * qglDisable_real )( GLenum cap );
void ( APIENTRY * qglEnable_real )( GLenum cap );
void ( APIENTRY * qglPolygonMode_real )( GLenum face, GLenum mode );
#endif

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void QGL_Shutdown(){
	Sys_Printf( "Shutting down GL ..." );

	if ( g_hGLDLL ) {
#ifdef _WIN32
		FreeLibrary( g_hGLDLL );
#endif

#if defined ( __linux__ ) || defined ( __APPLE__ )
		dlclose( g_hGLDLL );
#endif

		g_hGLDLL = NULL;
	}

	Sys_Printf( "Done.\n" );

	qglAccum                     = NULL;
	qglAlphaFunc                 = NULL;
	qglAreTexturesResident       = NULL;
	qglArrayElement              = NULL;
	qglBegin                     = NULL;
	qglBindTexture               = NULL;
	qglBitmap                    = NULL;
	qglBlendFunc                 = NULL;
	qglCallList                  = NULL;
	qglCallLists                 = NULL;
	qglClear                     = NULL;
	qglClearAccum                = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearIndex                = NULL;
	qglClearStencil              = NULL;
	qglClipPlane                 = NULL;
	qglColor3b                   = NULL;
	qglColor3bv                  = NULL;
	qglColor3d                   = NULL;
	qglColor3dv                  = NULL;
	qglColor3f                   = NULL;
	qglColor3fv                  = NULL;
	qglColor3i                   = NULL;
	qglColor3iv                  = NULL;
	qglColor3s                   = NULL;
	qglColor3sv                  = NULL;
	qglColor3ub                  = NULL;
	qglColor3ubv                 = NULL;
	qglColor3ui                  = NULL;
	qglColor3uiv                 = NULL;
	qglColor3us                  = NULL;
	qglColor3usv                 = NULL;
	qglColor4b                   = NULL;
	qglColor4bv                  = NULL;
	qglColor4d                   = NULL;
	qglColor4dv                  = NULL;
	qglColor4f                   = NULL;
	qglColor4fv                  = NULL;
	qglColor4i                   = NULL;
	qglColor4iv                  = NULL;
	qglColor4s                   = NULL;
	qglColor4sv                  = NULL;
	qglColor4ub                  = NULL;
	qglColor4ubv                 = NULL;
	qglColor4ui                  = NULL;
	qglColor4uiv                 = NULL;
	qglColor4us                  = NULL;
	qglColor4usv                 = NULL;
	qglColorMask                 = NULL;
	qglColorMaterial             = NULL;
	qglColorPointer              = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglCullFace                  = NULL;
	qglDeleteLists               = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDisableClientState        = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglDrawPixels                = NULL;
	qglEdgeFlag                  = NULL;
	qglEdgeFlagPointer           = NULL;
	qglEdgeFlagv                 = NULL;
	qglEnable                    = NULL;
	qglEnableClientState         = NULL;
	qglEnd                       = NULL;
	qglEndList                   = NULL;
	qglEvalCoord1d               = NULL;
	qglEvalCoord1dv              = NULL;
	qglEvalCoord1f               = NULL;
	qglEvalCoord1fv              = NULL;
	qglEvalCoord2d               = NULL;
	qglEvalCoord2dv              = NULL;
	qglEvalCoord2f               = NULL;
	qglEvalCoord2fv              = NULL;
	qglEvalMesh1                 = NULL;
	qglEvalMesh2                 = NULL;
	qglEvalPoint1                = NULL;
	qglEvalPoint2                = NULL;
	qglFeedbackBuffer            = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFogf                      = NULL;
	qglFogfv                     = NULL;
	qglFogi                      = NULL;
	qglFogiv                     = NULL;
	qglFrontFace                 = NULL;
	qglFrustum                   = NULL;
	qglGenLists                  = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetClipPlane              = NULL;
	qglGetDoublev                = NULL;
	qglGetError                  = NULL;
	qglGetFloatv                 = NULL;
	qglGetIntegerv               = NULL;
	qglGetLightfv                = NULL;
	qglGetLightiv                = NULL;
	qglGetMapdv                  = NULL;
	qglGetMapfv                  = NULL;
	qglGetMapiv                  = NULL;
	qglGetMaterialfv             = NULL;
	qglGetMaterialiv             = NULL;
	qglGetPixelMapfv             = NULL;
	qglGetPixelMapuiv            = NULL;
	qglGetPixelMapusv            = NULL;
	qglGetPointerv               = NULL;
	qglGetPolygonStipple         = NULL;
	qglGetString                 = NULL;
	qglGetTexEnvfv               = NULL;
	qglGetTexEnviv               = NULL;
	qglGetTexGendv               = NULL;
	qglGetTexGenfv               = NULL;
	qglGetTexGeniv               = NULL;
	qglGetTexImage               = NULL;
	qglGetTexLevelParameterfv    = NULL;
	qglGetTexLevelParameteriv    = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIndexMask                 = NULL;
	qglIndexPointer              = NULL;
	qglIndexd                    = NULL;
	qglIndexdv                   = NULL;
	qglIndexf                    = NULL;
	qglIndexfv                   = NULL;
	qglIndexi                    = NULL;
	qglIndexiv                   = NULL;
	qglIndexs                    = NULL;
	qglIndexsv                   = NULL;
	qglIndexub                   = NULL;
	qglIndexubv                  = NULL;
	qglInitNames                 = NULL;
	qglInterleavedArrays         = NULL;
	qglIsEnabled                 = NULL;
	qglIsList                    = NULL;
	qglIsTexture                 = NULL;
	qglLightModelf               = NULL;
	qglLightModelfv              = NULL;
	qglLightModeli               = NULL;
	qglLightModeliv              = NULL;
	qglLightf                    = NULL;
	qglLightfv                   = NULL;
	qglLighti                    = NULL;
	qglLightiv                   = NULL;
	qglLineStipple               = NULL;
	qglLineWidth                 = NULL;
	qglListBase                  = NULL;
	qglLoadIdentity              = NULL;
	qglLoadMatrixd               = NULL;
	qglLoadMatrixf               = NULL;
	qglLoadName                  = NULL;
	qglLogicOp                   = NULL;
	qglMap1d                     = NULL;
	qglMap1f                     = NULL;
	qglMap2d                     = NULL;
	qglMap2f                     = NULL;
	qglMapGrid1d                 = NULL;
	qglMapGrid1f                 = NULL;
	qglMapGrid2d                 = NULL;
	qglMapGrid2f                 = NULL;
	qglMaterialf                 = NULL;
	qglMaterialfv                = NULL;
	qglMateriali                 = NULL;
	qglMaterialiv                = NULL;
	qglMatrixMode                = NULL;
	qglMultMatrixd               = NULL;
	qglMultMatrixf               = NULL;
	qglNewList                   = NULL;
	qglNormal3b                  = NULL;
	qglNormal3bv                 = NULL;
	qglNormal3d                  = NULL;
	qglNormal3dv                 = NULL;
	qglNormal3f                  = NULL;
	qglNormal3fv                 = NULL;
	qglNormal3i                  = NULL;
	qglNormal3iv                 = NULL;
	qglNormal3s                  = NULL;
	qglNormal3sv                 = NULL;
	qglNormalPointer             = NULL;
	qglOrtho                     = NULL;
	qglPassThrough               = NULL;
	qglPixelMapfv                = NULL;
	qglPixelMapuiv               = NULL;
	qglPixelMapusv               = NULL;
	qglPixelStoref               = NULL;
	qglPixelStorei               = NULL;
	qglPixelTransferf            = NULL;
	qglPixelTransferi            = NULL;
	qglPixelZoom                 = NULL;
	qglPointSize                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglPolygonStipple            = NULL;
	qglPopAttrib                 = NULL;
	qglPopClientAttrib           = NULL;
	qglPopMatrix                 = NULL;
	qglPopName                   = NULL;
	qglPrioritizeTextures        = NULL;
	qglPushAttrib                = NULL;
	qglPushClientAttrib          = NULL;
	qglPushMatrix                = NULL;
	qglPushName                  = NULL;
	qglRasterPos2d               = NULL;
	qglRasterPos2dv              = NULL;
	qglRasterPos2f               = NULL;
	qglRasterPos2fv              = NULL;
	qglRasterPos2i               = NULL;
	qglRasterPos2iv              = NULL;
	qglRasterPos2s               = NULL;
	qglRasterPos2sv              = NULL;
	qglRasterPos3d               = NULL;
	qglRasterPos3dv              = NULL;
	qglRasterPos3f               = NULL;
	qglRasterPos3fv              = NULL;
	qglRasterPos3i               = NULL;
	qglRasterPos3iv              = NULL;
	qglRasterPos3s               = NULL;
	qglRasterPos3sv              = NULL;
	qglRasterPos4d               = NULL;
	qglRasterPos4dv              = NULL;
	qglRasterPos4f               = NULL;
	qglRasterPos4fv              = NULL;
	qglRasterPos4i               = NULL;
	qglRasterPos4iv              = NULL;
	qglRasterPos4s               = NULL;
	qglRasterPos4sv              = NULL;
	qglReadBuffer                = NULL;
	qglReadPixels                = NULL;
	qglRectd                     = NULL;
	qglRectdv                    = NULL;
	qglRectf                     = NULL;
	qglRectfv                    = NULL;
	qglRecti                     = NULL;
	qglRectiv                    = NULL;
	qglRects                     = NULL;
	qglRectsv                    = NULL;
	qglRenderMode                = NULL;
	qglRotated                   = NULL;
	qglRotatef                   = NULL;
	qglScaled                    = NULL;
	qglScalef                    = NULL;
	qglScissor                   = NULL;
	qglSelectBuffer              = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexCoord1d                = NULL;
	qglTexCoord1dv               = NULL;
	qglTexCoord1f                = NULL;
	qglTexCoord1fv               = NULL;
	qglTexCoord1i                = NULL;
	qglTexCoord1iv               = NULL;
	qglTexCoord1s                = NULL;
	qglTexCoord1sv               = NULL;
	qglTexCoord2d                = NULL;
	qglTexCoord2dv               = NULL;
	qglTexCoord2f                = NULL;
	qglTexCoord2fv               = NULL;
	qglTexCoord2i                = NULL;
	qglTexCoord2iv               = NULL;
	qglTexCoord2s                = NULL;
	qglTexCoord2sv               = NULL;
	qglTexCoord3d                = NULL;
	qglTexCoord3dv               = NULL;
	qglTexCoord3f                = NULL;
	qglTexCoord3fv               = NULL;
	qglTexCoord3i                = NULL;
	qglTexCoord3iv               = NULL;
	qglTexCoord3s                = NULL;
	qglTexCoord3sv               = NULL;
	qglTexCoord4d                = NULL;
	qglTexCoord4dv               = NULL;
	qglTexCoord4f                = NULL;
	qglTexCoord4fv               = NULL;
	qglTexCoord4i                = NULL;
	qglTexCoord4iv               = NULL;
	qglTexCoord4s                = NULL;
	qglTexCoord4sv               = NULL;
	qglTexCoordPointer           = NULL;
	qglTexEnvf                   = NULL;
	qglTexEnvfv                  = NULL;
	qglTexEnvi                   = NULL;
	qglTexEnviv                  = NULL;
	qglTexGend                   = NULL;
	qglTexGendv                  = NULL;
	qglTexGenf                   = NULL;
	qglTexGenfv                  = NULL;
	qglTexGeni                   = NULL;
	qglTexGeniv                  = NULL;
	qglTexImage1D                = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameterfv            = NULL;
	qglTexParameteri             = NULL;
	qglTexParameteriv            = NULL;
	qglTexSubImage1D             = NULL;
	qglTexSubImage2D             = NULL;
	qglTranslated                = NULL;
	qglTranslatef                = NULL;
	qglVertex2d                  = NULL;
	qglVertex2dv                 = NULL;
	qglVertex2f                  = NULL;
	qglVertex2fv                 = NULL;
	qglVertex2i                  = NULL;
	qglVertex2iv                 = NULL;
	qglVertex2s                  = NULL;
	qglVertex2sv                 = NULL;
	qglVertex3d                  = NULL;
	qglVertex3dv                 = NULL;
	qglVertex3f                  = NULL;
	qglVertex3fv                 = NULL;
	qglVertex3i                  = NULL;
	qglVertex3iv                 = NULL;
	qglVertex3s                  = NULL;
	qglVertex3sv                 = NULL;
	qglVertex4d                  = NULL;
	qglVertex4dv                 = NULL;
	qglVertex4f                  = NULL;
	qglVertex4fv                 = NULL;
	qglVertex4i                  = NULL;
	qglVertex4iv                 = NULL;
	qglVertex4s                  = NULL;
	qglVertex4sv                 = NULL;
	qglVertexPointer             = NULL;
	qglViewport                  = NULL;

	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	qglMultiTexCoord1dARB = NULL;
	qglMultiTexCoord1dvARB = NULL;
	qglMultiTexCoord1fARB = NULL;
	qglMultiTexCoord1fvARB = NULL;
	qglMultiTexCoord1iARB = NULL;
	qglMultiTexCoord1ivARB = NULL;
	qglMultiTexCoord1sARB = NULL;
	qglMultiTexCoord1svARB = NULL;
	qglMultiTexCoord2dARB = NULL;
	qglMultiTexCoord2dvARB = NULL;
	qglMultiTexCoord2fARB = NULL;
	qglMultiTexCoord2fvARB = NULL;
	qglMultiTexCoord2iARB = NULL;
	qglMultiTexCoord2ivARB = NULL;
	qglMultiTexCoord2sARB = NULL;
	qglMultiTexCoord2svARB = NULL;
	qglMultiTexCoord3dARB = NULL;
	qglMultiTexCoord3dvARB = NULL;
	qglMultiTexCoord3fARB = NULL;
	qglMultiTexCoord3fvARB = NULL;
	qglMultiTexCoord3iARB = NULL;
	qglMultiTexCoord3ivARB = NULL;
	qglMultiTexCoord3sARB = NULL;
	qglMultiTexCoord3svARB = NULL;
	qglMultiTexCoord4dARB = NULL;
	qglMultiTexCoord4dvARB = NULL;
	qglMultiTexCoord4fARB = NULL;
	qglMultiTexCoord4fvARB = NULL;
	qglMultiTexCoord4iARB = NULL;
	qglMultiTexCoord4ivARB = NULL;
	qglMultiTexCoord4sARB = NULL;
	qglMultiTexCoord4svARB = NULL;

#ifdef _WIN32
	qwglCopyContext              = NULL;
	qwglCreateContext            = NULL;
	qwglCreateLayerContext       = NULL;
	qwglDeleteContext            = NULL;
	qwglDescribeLayerPlane       = NULL;
	qwglGetCurrentContext        = NULL;
	qwglGetCurrentDC             = NULL;
	qwglGetLayerPaletteEntries   = NULL;
	qwglGetProcAddress           = NULL;
	qwglMakeCurrent              = NULL;
	qwglRealizeLayerPalette      = NULL;
	qwglSetLayerPaletteEntries   = NULL;
	qwglShareLists               = NULL;
	qwglSwapLayerBuffers         = NULL;
	qwglUseFontBitmaps           = NULL;
	qwglUseFontOutlines          = NULL;

	qwglChoosePixelFormat        = NULL;
	qwglDescribePixelFormat      = NULL;
	qwglGetPixelFormat           = NULL;
	qwglSetPixelFormat           = NULL;
	qwglSwapBuffers              = NULL;

	qwglSwapIntervalEXT          = NULL;

	qwglGetDeviceGammaRampEXT    = NULL;
	qwglSetDeviceGammaRampEXT    = NULL;
#endif

#if defined ( __linux__ ) || defined ( __APPLE__ )
	qglXChooseVisual             = NULL;
	qglXCreateContext            = NULL;
	qglXDestroyContext           = NULL;
	qglXMakeCurrent              = NULL;
	qglXCopyContext              = NULL;
	qglXSwapBuffers              = NULL;
	qglXCreateGLXPixmap          = NULL;
	qglXDestroyGLXPixmap         = NULL;
	qglXQueryExtension           = NULL;
	qglXQueryVersion             = NULL;
	qglXIsDirect                 = NULL;
	qglXGetConfig                = NULL;
	qglXGetCurrentContext        = NULL;
	qglXGetCurrentDrawable       = NULL;
	qglXWaitGL                   = NULL;
	qglXWaitX                    = NULL;
	qglXUseXFont                 = NULL;
	qglXGetProcAddressARB        = NULL;
#endif

	qgluPerspective = NULL;
	qgluErrorString = NULL;
	qgluLookAt = NULL;

#ifdef ATIHACK_812
	qglCullFace_real              = NULL;
	qglDisable_real               = NULL;
	qglEnable_real                = NULL;
	qglPolygonMode_real           = NULL;
#endif
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to
** the appropriate GL stuff.  In Windows this means doing a
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
**
*/
static int init_error;

static void* safe_dlsym( void *handle, char *symbol ){
#ifdef _WIN32
	return GetProcAddress( handle, symbol );
#endif

#if defined ( __linux__ ) || defined ( __APPLE__ )
	void* ret = dlsym( handle, symbol );
	const char *err = dlerror();
	if ( err ) {
		init_error = 1;
#ifndef __APPLE__
		printf( "Error loading OpenGL libraries: %s\n", err );
#else
		printf( "Error loading OpenGL libraries: %s %s\n", err, symbol );
#endif
	}
	return ret;
#endif
}

#include <math.h>
#include <stdlib.h>
#ifdef _WIN32
#define M_PI 3.14159
#endif

void WINAPI gluLookAt2( GLdouble ex, GLdouble ey, GLdouble ez, GLdouble cx, GLdouble cy, GLdouble cz,
						GLdouble ux, GLdouble uy, GLdouble uz ){
	GLdouble x[3], y[3], z[3] = { ex - cx, ey - cy, ez - cz };
	GLdouble inv;

	inv = sqrt( z[0] * z[0] + z[1] * z[1] + z[2] * z[2] );
	if ( inv ) {
		inv = 1.0 / inv;
		z[0] *= inv;
		z[1] *= inv;
		z[2] *= inv;
	}

	x[0] =  uy * z[2] - uz * z[1];
	x[1] = -ux * z[2] + uz * z[0];
	x[2] =  ux * z[1] - uy * z[0];

	y[0] =  z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] =  z[0] * x[1] - z[1] * x[0];

	inv = sqrt( x[0] * x[0] + x[1] * x[1] + x[2] * x[2] );
	if ( inv ) {
		x[0] *= inv;
		x[1] *= inv;
		x[2] *= inv;
	}

	inv = sqrt( y[0] * y[0] + y[1] * y[1] + y[2] * y[2] );
	if ( inv ) {
		y[0] *= inv;
		y[1] *= inv;
		y[2] *= inv;
	}

	{
		GLdouble m[16] = { x[0], y[0], z[0], 0, x[1], y[1], z[1], 0, x[2], y[2], z[2], 0, 0, 0, 0, 1 };
		qglMultMatrixd( m );
		qglTranslated( -ex, -ey, -ez );
	}
}

void WINAPI gluPerspective2( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar ){
	GLdouble y = zNear * tan( fovy * M_PI / 360.0 );
	qglFrustum( -y * aspect, y * aspect, -y, y, zNear, zFar );
}

#define CEILING( A, B )  ( ( A ) % ( B ) == 0 ? ( A ) / ( B ) : ( A ) / (B)+1 )

typedef struct glu_error_struct
{
	int errnum;
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
	{-1, NULL}
};

const GLubyte* WINAPI gluErrorString( GLenum errCode ){
	int search = 0;
	for ( search = 0; glu_errlist[search].errstr; search++ )
	{
		if ( errCode == glu_errlist[search].errnum ) {
			return (GLubyte *) glu_errlist[search].errstr;
		}
	} //end for
	return (GLubyte *) "Unknown error";
}

#ifdef ATIHACK_812
int ATIhack_culling;
GLenum ATIhack_cullmode;
GLenum ATIhack_backmode;
GLenum ATIhack_frontmode;

static void ATIhack_update( void ){
	if ( !ATIhack_culling || ( GL_FRONT_AND_BACK == ATIhack_cullmode ) ) {
		qglPolygonMode_real( GL_FRONT, ATIhack_frontmode );
		qglPolygonMode_real( GL_BACK, ATIhack_backmode );
	}
	else{
		switch ( ATIhack_cullmode )
		{
		case GL_FRONT:
			qglPolygonMode_real( GL_FRONT_AND_BACK, ATIhack_backmode );
			break;
		case GL_BACK:
			qglPolygonMode_real( GL_FRONT_AND_BACK, ATIhack_frontmode );
		default:
			break;
		}
	}
}

void APIENTRY qglEnable_ATIHack( GLenum cap ){
	qglEnable_real( cap );
	if ( GL_CULL_FACE != cap ) {
		return;
	}
	if ( ATIhack_culling ) {
		return;
	}
	ATIhack_culling = 1;
	ATIhack_update();
}

void APIENTRY qglDisable_ATIHack( GLenum cap ){
	qglDisable_real( cap );
	if ( GL_CULL_FACE != cap ) {
		return;
	}
	if ( !ATIhack_culling ) {
		return;
	}
	ATIhack_culling = 0;
	ATIhack_update();
}

void APIENTRY qglCullFace_ATIHack( GLenum mode ){
	if ( ATIhack_cullmode == mode ) {
		return;
	}
	qglCullFace_real( mode );
	ATIhack_cullmode = mode;
	ATIhack_update();
}

void APIENTRY qglPolygonMode_ATIHack( GLenum face, GLenum mode ){
	switch ( face )
	{
	case GL_FRONT:
		if ( ATIhack_frontmode == mode ) {
			return;
		}
		ATIhack_frontmode = mode;
		break;
	case GL_BACK:
		if ( ATIhack_backmode == mode ) {
			return;
		}
		ATIhack_backmode = mode;
		break;
	case GL_FRONT_AND_BACK:
		if ( ( ATIhack_frontmode == mode ) && ( ATIhack_backmode == mode ) ) {
			return;
		}
		ATIhack_frontmode = ATIhack_backmode = mode;
	default:
		break;
	}
	ATIhack_update();
}
#endif

int QGL_Init( const char *dllname, const char* gluname ){
#ifdef USE_GTKGLAREA
#define GLSYM2FUNC(symbol) #symbol
#else //if USE_GTKGLEXT
#define GLSYM2FUNC(symbol) safe_dlsym( g_hGLDLL, "#symbol" );

#ifdef _WIN32
	g_hGLDLL = LoadLibrary( dllname );
#endif

#if defined ( __linux__ ) || ( __APPLE__ )
	const char *err;

	// NOTE TTimo
	// I don't like RTLD_LAZY|RTLD_GLOBAL too much .. it's dangerous
	// maybe try RTLD_NOW? or would that break compatibility .. you never know when that stuff is going to explode
	g_hGLDLL = dlopen( dllname, RTLD_LAZY | RTLD_GLOBAL );
	err = dlerror();
	if ( err ) {
		printf( "Error loading GL lib:\n%s\n", err );
	}
#endif
	init_error = 0;

	if ( g_hGLDLL == NULL ) {
		return 0;
	}

	Sys_Printf( "Loading GL library: %s ...", dllname );
#endif //USE_GTKGLAREA

	qgluPerspective              = &gluPerspective2;

	qgluLookAt                   = &gluLookAt2;
	qgluErrorString              = &gluErrorString;
	qglAccum                     = GLSYM2FUNC( glAccum );
	qglAlphaFunc                 = GLSYM2FUNC( glAlphaFunc );
	qglAreTexturesResident       = GLSYM2FUNC( glAreTexturesResident );
	qglArrayElement              = GLSYM2FUNC( glArrayElement );
	qglBegin                     = GLSYM2FUNC( glBegin );
	qglBindTexture               = GLSYM2FUNC( glBindTexture );
	qglBitmap                    = GLSYM2FUNC( glBitmap );
	qglBlendFunc                 = GLSYM2FUNC( glBlendFunc );
	qglCallList                  = GLSYM2FUNC( glCallList );
	qglCallLists                 = GLSYM2FUNC( glCallLists );
	qglClear                     = GLSYM2FUNC( glClear );
	qglClearAccum                = GLSYM2FUNC( glClearAccum );
	qglClearColor                = GLSYM2FUNC( glClearColor );
	qglClearDepth                = GLSYM2FUNC( glClearDepth );
	qglClearIndex                = GLSYM2FUNC( glClearIndex );
	qglClearStencil              = GLSYM2FUNC( glClearStencil );
	qglClipPlane                 = GLSYM2FUNC( glClipPlane );
	qglColor3b                   = GLSYM2FUNC( glColor3b );
	qglColor3bv                  = GLSYM2FUNC( glColor3bv );
	qglColor3d                   = GLSYM2FUNC( glColor3d );
	qglColor3dv                  = GLSYM2FUNC( glColor3dv );
	qglColor3f                   = GLSYM2FUNC( glColor3f );
	qglColor3fv                  = GLSYM2FUNC( glColor3fv );
	qglColor3i                   = GLSYM2FUNC( glColor3i );
	qglColor3iv                  = GLSYM2FUNC( glColor3iv );
	qglColor3s                   = GLSYM2FUNC( glColor3s );
	qglColor3sv                  = GLSYM2FUNC( glColor3sv );
	qglColor3ub                  = GLSYM2FUNC( glColor3ub );
	qglColor3ubv                 = GLSYM2FUNC( glColor3ubv );
	qglColor3ui                  = GLSYM2FUNC( glColor3ui );
	qglColor3uiv                 = GLSYM2FUNC( glColor3uiv );
	qglColor3us                  = GLSYM2FUNC( glColor3us );
	qglColor3usv                 = GLSYM2FUNC( glColor3usv );
	qglColor4b                   = GLSYM2FUNC( glColor4b );
	qglColor4bv                  = GLSYM2FUNC( glColor4bv );
	qglColor4d                   = GLSYM2FUNC( glColor4d );
	qglColor4dv                  = GLSYM2FUNC( glColor4dv );
	qglColor4f                   = GLSYM2FUNC( glColor4f );
	qglColor4fv                  = GLSYM2FUNC( glColor4fv );
	qglColor4i                   = GLSYM2FUNC( glColor4i );
	qglColor4iv                  = GLSYM2FUNC( glColor4iv );
	qglColor4s                   = GLSYM2FUNC( glColor4s );
	qglColor4sv                  = GLSYM2FUNC( glColor4sv );
	qglColor4ub                  = GLSYM2FUNC( glColor4ub );
	qglColor4ubv                 = GLSYM2FUNC( glColor4ubv );
	qglColor4ui                  = GLSYM2FUNC( glColor4ui );
	qglColor4uiv                 = GLSYM2FUNC( glColor4uiv );
	qglColor4us                  = GLSYM2FUNC( glColor4us );
	qglColor4usv                 = GLSYM2FUNC( glColor4usv );
	qglColorMask                 = GLSYM2FUNC( glColorMask );
	qglColorMaterial             = GLSYM2FUNC( glColorMaterial );
	qglColorPointer              = GLSYM2FUNC( glColorPointer );
	qglCopyPixels                = GLSYM2FUNC( glCopyPixels );
	qglCopyTexImage1D            = GLSYM2FUNC( glCopyTexImage1D );
	qglCopyTexImage2D            = GLSYM2FUNC( glCopyTexImage2D );
	qglCopyTexSubImage1D         = GLSYM2FUNC( glCopyTexSubImage1D );
	qglCopyTexSubImage2D         = GLSYM2FUNC( glCopyTexSubImage2D );
#ifdef ATIHACK_812
	qglCullFace_real              = GLSYM2FUNC( glCullFace );
	qglCullFace                  = qglCullFace_real;
#else
	qglCullFace                  = GLSYM2FUNC( glCullFace );
#endif
	qglDeleteLists               = GLSYM2FUNC( glDeleteLists );
	qglDeleteTextures            = GLSYM2FUNC( glDeleteTextures );
	qglDepthFunc                 = GLSYM2FUNC( glDepthFunc );
	qglDepthMask                 = GLSYM2FUNC( glDepthMask );
	qglDepthRange                = GLSYM2FUNC( glDepthRange );
#ifdef ATIHACK_812
	qglDisable_real               = GLSYM2FUNC( glDisable );
	qglDisable                   = qglDisable_real;
#else
	qglDisable                   = GLSYM2FUNC( glDisable );
#endif
	qglDisableClientState        = GLSYM2FUNC( glDisableClientState );
	qglDrawArrays                = GLSYM2FUNC( glDrawArrays );
	qglDrawBuffer                = GLSYM2FUNC( glDrawBuffer );
	qglDrawElements              = GLSYM2FUNC( glDrawElements );
	qglDrawPixels                = GLSYM2FUNC( glDrawPixels );
	qglEdgeFlag                  = GLSYM2FUNC( glEdgeFlag );
	qglEdgeFlagPointer           = GLSYM2FUNC( glEdgeFlagPointer );
	qglEdgeFlagv                 = GLSYM2FUNC( glEdgeFlagv );
#ifdef ATIHACK_812
	qglEnable_real                = GLSYM2FUNC( glEnable );
	qglEnable                    = qglEnable_real;
#else
	qglEnable                    = GLSYM2FUNC( glEnable );
#endif
	qglEnableClientState         = GLSYM2FUNC( glEnableClientState );
	qglEnd                       = GLSYM2FUNC( glEnd );
	qglEndList                   = GLSYM2FUNC( glEndList );
	qglEvalCoord1d               = GLSYM2FUNC( glEvalCoord1d );
	qglEvalCoord1dv              = GLSYM2FUNC( glEvalCoord1dv );
	qglEvalCoord1f               = GLSYM2FUNC( glEvalCoord1f );
	qglEvalCoord1fv              = GLSYM2FUNC( glEvalCoord1fv );
	qglEvalCoord2d               = GLSYM2FUNC( glEvalCoord2d );
	qglEvalCoord2dv              = GLSYM2FUNC( glEvalCoord2dv );
	qglEvalCoord2f               = GLSYM2FUNC( glEvalCoord2f );
	qglEvalCoord2fv              = GLSYM2FUNC( glEvalCoord2fv );
	qglEvalMesh1                 = GLSYM2FUNC( glEvalMesh1 );
	qglEvalMesh2                 = GLSYM2FUNC( glEvalMesh2 );
	qglEvalPoint1                = GLSYM2FUNC( glEvalPoint1 );
	qglEvalPoint2                = GLSYM2FUNC( glEvalPoint2 );
	qglFeedbackBuffer            = GLSYM2FUNC( glFeedbackBuffer );
	qglFinish                    = GLSYM2FUNC( glFinish );
	qglFlush                     = GLSYM2FUNC( glFlush );
	qglFogf                      = GLSYM2FUNC( glFogf );
	qglFogfv                     = GLSYM2FUNC( glFogfv );
	qglFogi                      = GLSYM2FUNC( glFogi );
	qglFogiv                     = GLSYM2FUNC( glFogiv );
	qglFrontFace                 = GLSYM2FUNC( glFrontFace );
	qglFrustum                   = GLSYM2FUNC( glFrustum );
	qglGenLists                  = GLSYM2FUNC( glGenLists );
	qglGenTextures               = GLSYM2FUNC( glGenTextures );
	qglGetBooleanv               = GLSYM2FUNC( glGetBooleanv );
	qglGetClipPlane              = GLSYM2FUNC( glGetClipPlane );
	qglGetDoublev                = GLSYM2FUNC( glGetDoublev );
	qglGetError                  = GLSYM2FUNC( glGetError );
	qglGetFloatv                 = GLSYM2FUNC( glGetFloatv );
	qglGetIntegerv               = GLSYM2FUNC( glGetIntegerv );
	qglGetLightfv                = GLSYM2FUNC( glGetLightfv );
	qglGetLightiv                = GLSYM2FUNC( glGetLightiv );
	qglGetMapdv                  = GLSYM2FUNC( glGetMapdv );
	qglGetMapfv                  = GLSYM2FUNC( glGetMapfv );
	qglGetMapiv                  = GLSYM2FUNC( glGetMapiv );
	qglGetMaterialfv             = GLSYM2FUNC( glGetMaterialfv );
	qglGetMaterialiv             = GLSYM2FUNC( glGetMaterialiv );
	qglGetPixelMapfv             = GLSYM2FUNC( glGetPixelMapfv );
	qglGetPixelMapuiv            = GLSYM2FUNC( glGetPixelMapuiv );
	qglGetPixelMapusv            = GLSYM2FUNC( glGetPixelMapusv );
	qglGetPointerv               = GLSYM2FUNC( glGetPointerv );
	qglGetPolygonStipple         = GLSYM2FUNC( glGetPolygonStipple );
	qglGetString                 = GLSYM2FUNC( glGetString );
	qglGetTexEnvfv               = GLSYM2FUNC( glGetTexEnvfv );
	qglGetTexEnviv               = GLSYM2FUNC( glGetTexEnviv );
	qglGetTexGendv               = GLSYM2FUNC( glGetTexGendv );
	qglGetTexGenfv               = GLSYM2FUNC( glGetTexGenfv );
	qglGetTexGeniv               = GLSYM2FUNC( glGetTexGeniv );
	qglGetTexImage               = GLSYM2FUNC( glGetTexImage );
	qglGetTexLevelParameterfv    = GLSYM2FUNC( glGetTexLevelParameterfv );
	qglGetTexLevelParameteriv    = GLSYM2FUNC( glGetTexLevelParameteriv );
	qglGetTexParameterfv         = GLSYM2FUNC( glGetTexParameterfv );
	qglGetTexParameteriv         = GLSYM2FUNC( glGetTexParameteriv );
	qglHint                      = GLSYM2FUNC( glHint );
	qglIndexMask                 = GLSYM2FUNC( glIndexMask );
	qglIndexPointer              = GLSYM2FUNC( glIndexPointer );
	qglIndexd                    = GLSYM2FUNC( glIndexd );
	qglIndexdv                   = GLSYM2FUNC( glIndexdv );
	qglIndexf                    = GLSYM2FUNC( glIndexf );
	qglIndexfv                   = GLSYM2FUNC( glIndexfv );
	qglIndexi                    = GLSYM2FUNC( glIndexi );
	qglIndexiv                   = GLSYM2FUNC( glIndexiv );
	qglIndexs                    = GLSYM2FUNC( glIndexs );
	qglIndexsv                   = GLSYM2FUNC( glIndexsv );
	qglIndexub                   = GLSYM2FUNC( glIndexub );
	qglIndexubv                  = GLSYM2FUNC( glIndexubv );
	qglInitNames                 = GLSYM2FUNC( glInitNames );
	qglInterleavedArrays         = GLSYM2FUNC( glInterleavedArrays );
	qglIsEnabled                 = GLSYM2FUNC( glIsEnabled );
	qglIsList                    = GLSYM2FUNC( glIsList );
	qglIsTexture                 = GLSYM2FUNC( glIsTexture );
	qglLightModelf               = GLSYM2FUNC( glLightModelf );
	qglLightModelfv              = GLSYM2FUNC( glLightModelfv );
	qglLightModeli               = GLSYM2FUNC( glLightModeli );
	qglLightModeliv              = GLSYM2FUNC( glLightModeliv );
	qglLightf                    = GLSYM2FUNC( glLightf );
	qglLightfv                   = GLSYM2FUNC( glLightfv );
	qglLighti                    = GLSYM2FUNC( glLighti );
	qglLightiv                   = GLSYM2FUNC( glLightiv );
	qglLineStipple               = GLSYM2FUNC( glLineStipple );
	qglLineWidth                 = GLSYM2FUNC( glLineWidth );
	qglListBase                  = GLSYM2FUNC( glListBase );
	qglLoadIdentity              = GLSYM2FUNC( glLoadIdentity );
	qglLoadMatrixd               = GLSYM2FUNC( glLoadMatrixd );
	qglLoadMatrixf               = GLSYM2FUNC( glLoadMatrixf );
	qglLoadName                  = GLSYM2FUNC( glLoadName );
	qglLogicOp                   = GLSYM2FUNC( glLogicOp );
	qglMap1d                     = GLSYM2FUNC( glMap1d );
	qglMap1f                     = GLSYM2FUNC( glMap1f );
	qglMap2d                     = GLSYM2FUNC( glMap2d );
	qglMap2f                     = GLSYM2FUNC( glMap2f );
	qglMapGrid1d                 = GLSYM2FUNC( glMapGrid1d );
	qglMapGrid1f                 = GLSYM2FUNC( glMapGrid1f );
	qglMapGrid2d                 = GLSYM2FUNC( glMapGrid2d );
	qglMapGrid2f                 = GLSYM2FUNC( glMapGrid2f );
	qglMaterialf                 = GLSYM2FUNC( glMaterialf );
	qglMaterialfv                = GLSYM2FUNC( glMaterialfv );
	qglMateriali                 = GLSYM2FUNC( glMateriali );
	qglMaterialiv                = GLSYM2FUNC( glMaterialiv );
	qglMatrixMode                = GLSYM2FUNC( glMatrixMode );
	qglMultMatrixd               = GLSYM2FUNC( glMultMatrixd );
	qglMultMatrixf               = GLSYM2FUNC( glMultMatrixf );
	qglNewList                   = GLSYM2FUNC( glNewList );
	qglNormal3b                  = GLSYM2FUNC( glNormal3b );
	qglNormal3bv                 = GLSYM2FUNC( glNormal3bv );
	qglNormal3d                  = GLSYM2FUNC( glNormal3d );
	qglNormal3dv                 = GLSYM2FUNC( glNormal3dv );
	qglNormal3f                  = GLSYM2FUNC( glNormal3f );
	qglNormal3fv                 = GLSYM2FUNC( glNormal3fv );
	qglNormal3i                  = GLSYM2FUNC( glNormal3i );
	qglNormal3iv                 = GLSYM2FUNC( glNormal3iv );
	qglNormal3s                  = GLSYM2FUNC( glNormal3s );
	qglNormal3sv                 = GLSYM2FUNC( glNormal3sv );
	qglNormalPointer             = GLSYM2FUNC( glNormalPointer );
	qglOrtho                     = GLSYM2FUNC( glOrtho );
	qglPassThrough               = GLSYM2FUNC( glPassThrough );
	qglPixelMapfv                = GLSYM2FUNC( glPixelMapfv );
	qglPixelMapuiv               = GLSYM2FUNC( glPixelMapuiv );
	qglPixelMapusv               = GLSYM2FUNC( glPixelMapusv );
	qglPixelStoref               = GLSYM2FUNC( glPixelStoref );
	qglPixelStorei               = GLSYM2FUNC( glPixelStorei );
	qglPixelTransferf            = GLSYM2FUNC( glPixelTransferf );
	qglPixelTransferi            = GLSYM2FUNC( glPixelTransferi );
	qglPixelZoom                 = GLSYM2FUNC( glPixelZoom );
	qglPointSize                 = GLSYM2FUNC( glPointSize );
#ifdef ATIHACK_812
	qglPolygonMode_real           = GLSYM2FUNC( glPolygonMode );
	qglPolygonMode               = qglPolygonMode_real;
#else
	qglPolygonMode               = GLSYM2FUNC( glPolygonMode );
#endif
	qglPolygonOffset             = GLSYM2FUNC( glPolygonOffset );
	qglPolygonStipple            = GLSYM2FUNC( glPolygonStipple );
	qglPopAttrib                 = GLSYM2FUNC( glPopAttrib );
	qglPopClientAttrib           = GLSYM2FUNC( glPopClientAttrib );
	qglPopMatrix                 = GLSYM2FUNC( glPopMatrix );
	qglPopName                   = GLSYM2FUNC( glPopName );
	qglPrioritizeTextures        = GLSYM2FUNC( glPrioritizeTextures );
	qglPushAttrib                = GLSYM2FUNC( glPushAttrib );
	qglPushClientAttrib          = GLSYM2FUNC( glPushClientAttrib );
	qglPushMatrix                = GLSYM2FUNC( glPushMatrix );
	qglPushName                  = GLSYM2FUNC( glPushName );
	qglRasterPos2d               = GLSYM2FUNC( glRasterPos2d );
	qglRasterPos2dv              = GLSYM2FUNC( glRasterPos2dv );
	qglRasterPos2f               = GLSYM2FUNC( glRasterPos2f );
	qglRasterPos2fv              = GLSYM2FUNC( glRasterPos2fv );
	qglRasterPos2i               = GLSYM2FUNC( glRasterPos2i );
	qglRasterPos2iv              = GLSYM2FUNC( glRasterPos2iv );
	qglRasterPos2s               = GLSYM2FUNC( glRasterPos2s );
	qglRasterPos2sv              = GLSYM2FUNC( glRasterPos2sv );
	qglRasterPos3d               = GLSYM2FUNC( glRasterPos3d );
	qglRasterPos3dv              = GLSYM2FUNC( glRasterPos3dv );
	qglRasterPos3f               = GLSYM2FUNC( glRasterPos3f );
	qglRasterPos3fv              = GLSYM2FUNC( glRasterPos3fv );
	qglRasterPos3i               = GLSYM2FUNC( glRasterPos3i );
	qglRasterPos3iv              = GLSYM2FUNC( glRasterPos3iv );
	qglRasterPos3s               = GLSYM2FUNC( glRasterPos3s );
	qglRasterPos3sv              = GLSYM2FUNC( glRasterPos3sv );
	qglRasterPos4d               = GLSYM2FUNC( glRasterPos4d );
	qglRasterPos4dv              = GLSYM2FUNC( glRasterPos4dv );
	qglRasterPos4f               = GLSYM2FUNC( glRasterPos4f );
	qglRasterPos4fv              = GLSYM2FUNC( glRasterPos4fv );
	qglRasterPos4i               = GLSYM2FUNC( glRasterPos4i );
	qglRasterPos4iv              = GLSYM2FUNC( glRasterPos4iv );
	qglRasterPos4s               = GLSYM2FUNC( glRasterPos4s );
	qglRasterPos4sv              = GLSYM2FUNC( glRasterPos4sv );
	qglReadBuffer                = GLSYM2FUNC( glReadBuffer );
	qglReadPixels                = GLSYM2FUNC( glReadPixels );
	qglRectd                     = GLSYM2FUNC( glRectd );
	qglRectdv                    = GLSYM2FUNC( glRectdv );
	qglRectf                     = GLSYM2FUNC( glRectf );
	qglRectfv                    = GLSYM2FUNC( glRectfv );
	qglRecti                     = GLSYM2FUNC( glRecti );
	qglRectiv                    = GLSYM2FUNC( glRectiv );
	qglRects                     = GLSYM2FUNC( glRects );
	qglRectsv                    = GLSYM2FUNC( glRectsv );
	qglRenderMode                = GLSYM2FUNC( glRenderMode );
	qglRotated                   = GLSYM2FUNC( glRotated );
	qglRotatef                   = GLSYM2FUNC( glRotatef );
	qglScaled                    = GLSYM2FUNC( glScaled );
	qglScalef                    = GLSYM2FUNC( glScalef );
	qglScissor                   = GLSYM2FUNC( glScissor );
	qglSelectBuffer              = GLSYM2FUNC( glSelectBuffer );
	qglShadeModel                = GLSYM2FUNC( glShadeModel );
	qglStencilFunc               = GLSYM2FUNC( glStencilFunc );
	qglStencilMask               = GLSYM2FUNC( glStencilMask );
	qglStencilOp                 = GLSYM2FUNC( glStencilOp );
	qglTexCoord1d                = GLSYM2FUNC( glTexCoord1d );
	qglTexCoord1dv               = GLSYM2FUNC( glTexCoord1dv );
	qglTexCoord1f                = GLSYM2FUNC( glTexCoord1f );
	qglTexCoord1fv               = GLSYM2FUNC( glTexCoord1fv );
	qglTexCoord1i                = GLSYM2FUNC( glTexCoord1i );
	qglTexCoord1iv               = GLSYM2FUNC( glTexCoord1iv );
	qglTexCoord1s                = GLSYM2FUNC( glTexCoord1s );
	qglTexCoord1sv               = GLSYM2FUNC( glTexCoord1sv );
	qglTexCoord2d                = GLSYM2FUNC( glTexCoord2d );
	qglTexCoord2dv               = GLSYM2FUNC( glTexCoord2dv );
	qglTexCoord2f                = GLSYM2FUNC( glTexCoord2f );
	qglTexCoord2fv               = GLSYM2FUNC( glTexCoord2fv );
	qglTexCoord2i                = GLSYM2FUNC( glTexCoord2i );
	qglTexCoord2iv               = GLSYM2FUNC( glTexCoord2iv );
	qglTexCoord2s                = GLSYM2FUNC( glTexCoord2s );
	qglTexCoord2sv               = GLSYM2FUNC( glTexCoord2sv );
	qglTexCoord3d                = GLSYM2FUNC( glTexCoord3d );
	qglTexCoord3dv               = GLSYM2FUNC( glTexCoord3dv );
	qglTexCoord3f                = GLSYM2FUNC( glTexCoord3f );
	qglTexCoord3fv               = GLSYM2FUNC( glTexCoord3fv );
	qglTexCoord3i                = GLSYM2FUNC( glTexCoord3i );
	qglTexCoord3iv               = GLSYM2FUNC( glTexCoord3iv );
	qglTexCoord3s                = GLSYM2FUNC( glTexCoord3s );
	qglTexCoord3sv               = GLSYM2FUNC( glTexCoord3sv );
	qglTexCoord4d                = GLSYM2FUNC( glTexCoord4d );
	qglTexCoord4dv               = GLSYM2FUNC( glTexCoord4dv );
	qglTexCoord4f                = GLSYM2FUNC( glTexCoord4f );
	qglTexCoord4fv               = GLSYM2FUNC( glTexCoord4fv );
	qglTexCoord4i                = GLSYM2FUNC( glTexCoord4i );
	qglTexCoord4iv               = GLSYM2FUNC( glTexCoord4iv );
	qglTexCoord4s                = GLSYM2FUNC( glTexCoord4s );
	qglTexCoord4sv               = GLSYM2FUNC( glTexCoord4sv );
	qglTexCoordPointer           = GLSYM2FUNC( glTexCoordPointer );
	qglTexEnvf                   = GLSYM2FUNC( glTexEnvf );
	qglTexEnvfv                  = GLSYM2FUNC( glTexEnvfv );
	qglTexEnvi                   = GLSYM2FUNC( glTexEnvi );
	qglTexEnviv                  = GLSYM2FUNC( glTexEnviv );
	qglTexGend                   = GLSYM2FUNC( glTexGend );
	qglTexGendv                  = GLSYM2FUNC( glTexGendv );
	qglTexGenf                   = GLSYM2FUNC( glTexGenf );
	qglTexGenfv                  = GLSYM2FUNC( glTexGenfv );
	qglTexGeni                   = GLSYM2FUNC( glTexGeni );
	qglTexGeniv                  = GLSYM2FUNC( glTexGeniv );
	qglTexImage1D                = GLSYM2FUNC( glTexImage1D );
	qglTexImage2D                = GLSYM2FUNC( glTexImage2D );
	qglTexParameterf             = GLSYM2FUNC( glTexParameterf );
	qglTexParameterfv            = GLSYM2FUNC( glTexParameterfv );
	qglTexParameteri             = GLSYM2FUNC( glTexParameteri );
	qglTexParameteriv            = GLSYM2FUNC( glTexParameteriv );
	qglTexSubImage1D             = GLSYM2FUNC( glTexSubImage1D );
	qglTexSubImage2D             = GLSYM2FUNC( glTexSubImage2D );
	qglTranslated                = GLSYM2FUNC( glTranslated );
	qglTranslatef                = GLSYM2FUNC( glTranslatef );
	qglVertex2d                  = GLSYM2FUNC( glVertex2d );
	qglVertex2dv                 = GLSYM2FUNC( glVertex2dv );
	qglVertex2f                  = GLSYM2FUNC( glVertex2f );
	qglVertex2fv                 = GLSYM2FUNC( glVertex2fv );
	qglVertex2i                  = GLSYM2FUNC( glVertex2i );
	qglVertex2iv                 = GLSYM2FUNC( glVertex2iv );
	qglVertex2s                  = GLSYM2FUNC( glVertex2s );
	qglVertex2sv                 = GLSYM2FUNC( glVertex2sv );
	qglVertex3d                  = GLSYM2FUNC( glVertex3d );
	qglVertex3dv                 = GLSYM2FUNC( glVertex3dv );
	qglVertex3f                  = GLSYM2FUNC( glVertex3f );
	qglVertex3fv                 = GLSYM2FUNC( glVertex3fv );
	qglVertex3i                  = GLSYM2FUNC( glVertex3i );
	qglVertex3iv                 = GLSYM2FUNC( glVertex3iv );
	qglVertex3s                  = GLSYM2FUNC( glVertex3s );
	qglVertex3sv                 = GLSYM2FUNC( glVertex3sv );
	qglVertex4d                  = GLSYM2FUNC( glVertex4d );
	qglVertex4dv                 = GLSYM2FUNC( glVertex4dv );
	qglVertex4f                  = GLSYM2FUNC( glVertex4f );
	qglVertex4fv                 = GLSYM2FUNC( glVertex4fv );
	qglVertex4i                  = GLSYM2FUNC( glVertex4i );
	qglVertex4iv                 = GLSYM2FUNC( glVertex4iv );
	qglVertex4s                  = GLSYM2FUNC( glVertex4s );
	qglVertex4sv                 = GLSYM2FUNC( glVertex4sv );
	qglVertexPointer             = GLSYM2FUNC( glVertexPointer );
	qglViewport                  = GLSYM2FUNC( glViewport );

	// must be init with an active context
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	qglMultiTexCoord1dARB = NULL;
	qglMultiTexCoord1dvARB = NULL;
	qglMultiTexCoord1fARB = NULL;
	qglMultiTexCoord1fvARB = NULL;
	qglMultiTexCoord1iARB = NULL;
	qglMultiTexCoord1ivARB = NULL;
	qglMultiTexCoord1sARB = NULL;
	qglMultiTexCoord1svARB = NULL;
	qglMultiTexCoord2dARB = NULL;
	qglMultiTexCoord2dvARB = NULL;
	qglMultiTexCoord2fARB = NULL;
	qglMultiTexCoord2fvARB = NULL;
	qglMultiTexCoord2iARB = NULL;
	qglMultiTexCoord2ivARB = NULL;
	qglMultiTexCoord2sARB = NULL;
	qglMultiTexCoord2svARB = NULL;
	qglMultiTexCoord3dARB = NULL;
	qglMultiTexCoord3dvARB = NULL;
	qglMultiTexCoord3fARB = NULL;
	qglMultiTexCoord3fvARB = NULL;
	qglMultiTexCoord3iARB = NULL;
	qglMultiTexCoord3ivARB = NULL;
	qglMultiTexCoord3sARB = NULL;
	qglMultiTexCoord3svARB = NULL;
	qglMultiTexCoord4dARB = NULL;
	qglMultiTexCoord4dvARB = NULL;
	qglMultiTexCoord4fARB = NULL;
	qglMultiTexCoord4fvARB = NULL;
	qglMultiTexCoord4iARB = NULL;
	qglMultiTexCoord4ivARB = NULL;
	qglMultiTexCoord4sARB = NULL;
	qglMultiTexCoord4svARB = NULL;

#ifdef _WIN32
	qwglCopyContext              = GLSYM2FUNC( wglCopyContext );
	qwglCreateContext            = GLSYM2FUNC( wglCreateContext );
	qwglCreateLayerContext       = GLSYM2FUNC( wglCreateLayerContext );
	qwglDeleteContext            = GLSYM2FUNC( wglDeleteContext );
	qwglDescribeLayerPlane       = GLSYM2FUNC( wglDescribeLayerPlane );
	qwglGetCurrentContext        = GLSYM2FUNC( wglGetCurrentContext );
	qwglGetCurrentDC             = GLSYM2FUNC( wglGetCurrentDC );
	qwglGetLayerPaletteEntries   = GLSYM2FUNC( wglGetLayerPaletteEntries );
	qwglGetProcAddress           = GLSYM2FUNC( wglGetProcAddress );
	qwglMakeCurrent              = GLSYM2FUNC( wglMakeCurrent );
	qwglRealizeLayerPalette      = GLSYM2FUNC( wglRealizeLayerPalette );
	qwglSetLayerPaletteEntries   = GLSYM2FUNC( wglSetLayerPaletteEntries );
	qwglShareLists               = GLSYM2FUNC( wglShareLists );
	qwglSwapLayerBuffers         = GLSYM2FUNC( wglSwapLayerBuffers );
	qwglUseFontBitmaps           = GLSYM2FUNC( wglUseFontBitmapsA );
	qwglUseFontOutlines          = GLSYM2FUNC( wglUseFontOutlinesA );

	qwglChoosePixelFormat        = GLSYM2FUNC( wglChoosePixelFormat );
	qwglDescribePixelFormat      = GLSYM2FUNC( wglDescribePixelFormat );
	qwglGetPixelFormat           = GLSYM2FUNC( wglGetPixelFormat );
	qwglSetPixelFormat           = GLSYM2FUNC( wglSetPixelFormat );
	qwglSwapBuffers              = GLSYM2FUNC( wglSwapBuffers );

	qwglSwapIntervalEXT = 0;
	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;
#endif

#if defined ( __linux__ ) || defined ( __APPLE__ )
	qglXChooseVisual             = GLSYM2FUNC( glXChooseVisual );
	qglXCreateContext            = GLSYM2FUNC( glXCreateContext );
	qglXDestroyContext           = GLSYM2FUNC( glXDestroyContext );
	qglXMakeCurrent              = GLSYM2FUNC( glXMakeCurrent );
	qglXCopyContext              = GLSYM2FUNC( glXCopyContext );
	qglXSwapBuffers              = GLSYM2FUNC( glXSwapBuffers );
	qglXCreateGLXPixmap          = GLSYM2FUNC( glXCreateGLXPixmap );
	qglXDestroyGLXPixmap         = GLSYM2FUNC( glXDestroyGLXPixmap );
	qglXQueryExtension           = GLSYM2FUNC( glXQueryExtension );
	qglXQueryVersion             = GLSYM2FUNC( glXQueryVersion );
	qglXIsDirect                 = GLSYM2FUNC( glXIsDirect );
	qglXGetConfig                = GLSYM2FUNC( glXGetConfig );
	qglXGetCurrentContext        = GLSYM2FUNC( glXGetCurrentContext );
	qglXGetCurrentDrawable       = GLSYM2FUNC( glXGetCurrentDrawable );
	qglXWaitGL                   = GLSYM2FUNC( glXWaitGL );
	qglXWaitX                    = GLSYM2FUNC( glXWaitX );
	qglXUseXFont                 = GLSYM2FUNC( glXUseXFont );
#endif

	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;

#ifdef ATIHACK_812
	ATIhack_culling = 0;
	ATIhack_cullmode = GL_BACK;
	ATIhack_backmode = GL_FILL;
	ATIhack_frontmode = GL_FILL;
#endif

#ifdef USE_GTKGLAREA

#else //USE_GTKGLEXT
	Sys_Printf( "Done.\n" );

	if ( init_error == 1 ) {
		return 0;
	}
#endif

	return 1;
}

int GL_ExtensionSupported( const char *extension ){
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	// Extension names should not have spaces.
	where = (GLubyte *) strchr( extension, ' ' );
	if ( where || *extension == '\0' ) {
		return 0;
	}

	extensions = qglGetString( GL_EXTENSIONS );
	if ( !extensions ) {
		return 0;
	}

	// It takes a bit of care to be fool-proof about parsing the
	// OpenGL extensions string. Don't be fooled by sub-strings, etc.
	for ( start = extensions; ; )
	{
		where = (GLubyte *) strstr( (const char *) start, extension );
		if ( !where ) {
			break;
		}

		terminator = where + strlen( extension );
		if ( where == start || *( where - 1 ) == ' ' ) {
			if ( *terminator == ' ' || *terminator == '\0' ) {
				return 1;
			}
		}

		start = terminator;
	}

	return 0;
}

void* Sys_GLGetExtension( const char *symbol ){
#if defined ( __linux__ ) || defined ( __APPLE__ )
	if ( qglXGetProcAddressARB == NULL ) {
		return NULL;
	}
	else{
		return qglXGetProcAddressARB( (GLubyte*)symbol );
	}
#else
	return qwglGetProcAddress( symbol );
#endif
}

void QGL_InitExtensions(){
	if ( GL_ExtensionSupported( "GL_ARB_multitexture" ) ) {
		qglActiveTextureARB = Sys_GLGetExtension( "glActiveTextureARB" );
		qglClientActiveTextureARB = Sys_GLGetExtension( "glClientActiveTextureARB" );
		qglMultiTexCoord1dARB = Sys_GLGetExtension( "glMultiTexCoord1dARB" );
		qglMultiTexCoord1dvARB = Sys_GLGetExtension( "glMultiTexCoord1dvARB" );
		qglMultiTexCoord1fARB = Sys_GLGetExtension( "glMultiTexCoord1fARB" );
		qglMultiTexCoord1fvARB = Sys_GLGetExtension( "glMultiTexCoord1fvARB" );
		qglMultiTexCoord1iARB = Sys_GLGetExtension( "glMultiTexCoord1iARB" );
		qglMultiTexCoord1ivARB = Sys_GLGetExtension( "glMultiTexCoord1ivARB" );
		qglMultiTexCoord1sARB = Sys_GLGetExtension( "glMultiTexCoord1sARB" );
		qglMultiTexCoord1svARB = Sys_GLGetExtension( "glMultiTexCoord1svARB" );
		qglMultiTexCoord2dARB = Sys_GLGetExtension( "glMultiTexCoord2dARB" );
		qglMultiTexCoord2dvARB = Sys_GLGetExtension( "glMultiTexCoord2dvARB" );
		qglMultiTexCoord2fARB = Sys_GLGetExtension( "glMultiTexCoord2fARB" );
		qglMultiTexCoord2fvARB = Sys_GLGetExtension( "glMultiTexCoord2fvARB" );
		qglMultiTexCoord2iARB = Sys_GLGetExtension( "glMultiTexCoord2iARB" );
		qglMultiTexCoord2ivARB = Sys_GLGetExtension( "glMultiTexCoord2ivARB" );
		qglMultiTexCoord2sARB = Sys_GLGetExtension( "glMultiTexCoord2sARB" );
		qglMultiTexCoord2svARB = Sys_GLGetExtension( "glMultiTexCoord2svARB" );
		qglMultiTexCoord3dARB = Sys_GLGetExtension( "glMultiTexCoord3dARB" );
		qglMultiTexCoord3dvARB = Sys_GLGetExtension( "glMultiTexCoord3dvARB" );
		qglMultiTexCoord3fARB = Sys_GLGetExtension( "glMultiTexCoord3fARB" );
		qglMultiTexCoord3fvARB = Sys_GLGetExtension( "glMultiTexCoord3fvARB" );
		qglMultiTexCoord3iARB = Sys_GLGetExtension( "glMultiTexCoord3iARB" );
		qglMultiTexCoord3ivARB = Sys_GLGetExtension( "glMultiTexCoord3ivARB" );
		qglMultiTexCoord3sARB = Sys_GLGetExtension( "glMultiTexCoord3sARB" );
		qglMultiTexCoord3svARB = Sys_GLGetExtension( "glMultiTexCoord3svARB" );
		qglMultiTexCoord4dARB = Sys_GLGetExtension( "glMultiTexCoord4dARB" );
		qglMultiTexCoord4dvARB = Sys_GLGetExtension( "glMultiTexCoord4dvARB" );
		qglMultiTexCoord4fARB = Sys_GLGetExtension( "glMultiTexCoord4fARB" );
		qglMultiTexCoord4fvARB = Sys_GLGetExtension( "glMultiTexCoord4fvARB" );
		qglMultiTexCoord4iARB = Sys_GLGetExtension( "glMultiTexCoord4iARB" );
		qglMultiTexCoord4ivARB = Sys_GLGetExtension( "glMultiTexCoord4ivARB" );
		qglMultiTexCoord4sARB = Sys_GLGetExtension( "glMultiTexCoord4sARB" );
		qglMultiTexCoord4svARB = Sys_GLGetExtension( "glMultiTexCoord4svARB" );
	}
}
