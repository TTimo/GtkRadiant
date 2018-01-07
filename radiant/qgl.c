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
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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
#ifdef _WIN32
	g_hGLDLL = LoadLibrary( dllname );
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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

	qgluPerspective              = &gluPerspective2;

	qgluLookAt                   = &gluLookAt2;
	qgluErrorString              = &gluErrorString;
	qglAccum                     = safe_dlsym( g_hGLDLL, "glAccum" );
	qglAlphaFunc                 = safe_dlsym( g_hGLDLL, "glAlphaFunc" );
	qglAreTexturesResident       = safe_dlsym( g_hGLDLL, "glAreTexturesResident" );
	qglArrayElement              = safe_dlsym( g_hGLDLL, "glArrayElement" );
	qglBegin                     = safe_dlsym( g_hGLDLL, "glBegin" );
	qglBindTexture               = safe_dlsym( g_hGLDLL, "glBindTexture" );
	qglBitmap                    = safe_dlsym( g_hGLDLL, "glBitmap" );
	qglBlendFunc                 = safe_dlsym( g_hGLDLL, "glBlendFunc" );
	qglCallList                  = safe_dlsym( g_hGLDLL, "glCallList" );
	qglCallLists                 = safe_dlsym( g_hGLDLL, "glCallLists" );
	qglClear                     = safe_dlsym( g_hGLDLL, "glClear" );
	qglClearAccum                = safe_dlsym( g_hGLDLL, "glClearAccum" );
	qglClearColor                = safe_dlsym( g_hGLDLL, "glClearColor" );
	qglClearDepth                = safe_dlsym( g_hGLDLL, "glClearDepth" );
	qglClearIndex                = safe_dlsym( g_hGLDLL, "glClearIndex" );
	qglClearStencil              = safe_dlsym( g_hGLDLL, "glClearStencil" );
	qglClipPlane                 = safe_dlsym( g_hGLDLL, "glClipPlane" );
	qglColor3b                   = safe_dlsym( g_hGLDLL, "glColor3b" );
	qglColor3bv                  = safe_dlsym( g_hGLDLL, "glColor3bv" );
	qglColor3d                   = safe_dlsym( g_hGLDLL, "glColor3d" );
	qglColor3dv                  = safe_dlsym( g_hGLDLL, "glColor3dv" );
	qglColor3f                   = safe_dlsym( g_hGLDLL, "glColor3f" );
	qglColor3fv                  = safe_dlsym( g_hGLDLL, "glColor3fv" );
	qglColor3i                   = safe_dlsym( g_hGLDLL, "glColor3i" );
	qglColor3iv                  = safe_dlsym( g_hGLDLL, "glColor3iv" );
	qglColor3s                   = safe_dlsym( g_hGLDLL, "glColor3s" );
	qglColor3sv                  = safe_dlsym( g_hGLDLL, "glColor3sv" );
	qglColor3ub                  = safe_dlsym( g_hGLDLL, "glColor3ub" );
	qglColor3ubv                 = safe_dlsym( g_hGLDLL, "glColor3ubv" );
	qglColor3ui                  = safe_dlsym( g_hGLDLL, "glColor3ui" );
	qglColor3uiv                 = safe_dlsym( g_hGLDLL, "glColor3uiv" );
	qglColor3us                  = safe_dlsym( g_hGLDLL, "glColor3us" );
	qglColor3usv                 = safe_dlsym( g_hGLDLL, "glColor3usv" );
	qglColor4b                   = safe_dlsym( g_hGLDLL, "glColor4b" );
	qglColor4bv                  = safe_dlsym( g_hGLDLL, "glColor4bv" );
	qglColor4d                   = safe_dlsym( g_hGLDLL, "glColor4d" );
	qglColor4dv                  = safe_dlsym( g_hGLDLL, "glColor4dv" );
	qglColor4f                   = safe_dlsym( g_hGLDLL, "glColor4f" );
	qglColor4fv                  = safe_dlsym( g_hGLDLL, "glColor4fv" );
	qglColor4i                   = safe_dlsym( g_hGLDLL, "glColor4i" );
	qglColor4iv                  = safe_dlsym( g_hGLDLL, "glColor4iv" );
	qglColor4s                   = safe_dlsym( g_hGLDLL, "glColor4s" );
	qglColor4sv                  = safe_dlsym( g_hGLDLL, "glColor4sv" );
	qglColor4ub                  = safe_dlsym( g_hGLDLL, "glColor4ub" );
	qglColor4ubv                 = safe_dlsym( g_hGLDLL, "glColor4ubv" );
	qglColor4ui                  = safe_dlsym( g_hGLDLL, "glColor4ui" );
	qglColor4uiv                 = safe_dlsym( g_hGLDLL, "glColor4uiv" );
	qglColor4us                  = safe_dlsym( g_hGLDLL, "glColor4us" );
	qglColor4usv                 = safe_dlsym( g_hGLDLL, "glColor4usv" );
	qglColorMask                 = safe_dlsym( g_hGLDLL, "glColorMask" );
	qglColorMaterial             = safe_dlsym( g_hGLDLL, "glColorMaterial" );
	qglColorPointer              = safe_dlsym( g_hGLDLL, "glColorPointer" );
	qglCopyPixels                = safe_dlsym( g_hGLDLL, "glCopyPixels" );
	qglCopyTexImage1D            = safe_dlsym( g_hGLDLL, "glCopyTexImage1D" );
	qglCopyTexImage2D            = safe_dlsym( g_hGLDLL, "glCopyTexImage2D" );
	qglCopyTexSubImage1D         = safe_dlsym( g_hGLDLL, "glCopyTexSubImage1D" );
	qglCopyTexSubImage2D         = safe_dlsym( g_hGLDLL, "glCopyTexSubImage2D" );
#ifdef ATIHACK_812
	qglCullFace_real              = safe_dlsym( g_hGLDLL, "glCullFace" );
	qglCullFace                  = qglCullFace_real;
#else
	qglCullFace                  = safe_dlsym( g_hGLDLL, "glCullFace" );
#endif
	qglDeleteLists               = safe_dlsym( g_hGLDLL, "glDeleteLists" );
	qglDeleteTextures            = safe_dlsym( g_hGLDLL, "glDeleteTextures" );
	qglDepthFunc                 = safe_dlsym( g_hGLDLL, "glDepthFunc" );
	qglDepthMask                 = safe_dlsym( g_hGLDLL, "glDepthMask" );
	qglDepthRange                = safe_dlsym( g_hGLDLL, "glDepthRange" );
#ifdef ATIHACK_812
	qglDisable_real               = safe_dlsym( g_hGLDLL, "glDisable" );
	qglDisable                   = qglDisable_real;
#else
	qglDisable                   = safe_dlsym( g_hGLDLL, "glDisable" );
#endif
	qglDisableClientState        = safe_dlsym( g_hGLDLL, "glDisableClientState" );
	qglDrawArrays                = safe_dlsym( g_hGLDLL, "glDrawArrays" );
	qglDrawBuffer                = safe_dlsym( g_hGLDLL, "glDrawBuffer" );
	qglDrawElements              = safe_dlsym( g_hGLDLL, "glDrawElements" );
	qglDrawPixels                = safe_dlsym( g_hGLDLL, "glDrawPixels" );
	qglEdgeFlag                  = safe_dlsym( g_hGLDLL, "glEdgeFlag" );
	qglEdgeFlagPointer           = safe_dlsym( g_hGLDLL, "glEdgeFlagPointer" );
	qglEdgeFlagv                 = safe_dlsym( g_hGLDLL, "glEdgeFlagv" );
#ifdef ATIHACK_812
	qglEnable_real                = safe_dlsym( g_hGLDLL, "glEnable" );
	qglEnable                    = qglEnable_real;
#else
	qglEnable                    = safe_dlsym( g_hGLDLL, "glEnable" );
#endif
	qglEnableClientState         = safe_dlsym( g_hGLDLL, "glEnableClientState" );
	qglEnd                       = safe_dlsym( g_hGLDLL, "glEnd" );
	qglEndList                   = safe_dlsym( g_hGLDLL, "glEndList" );
	qglEvalCoord1d               = safe_dlsym( g_hGLDLL, "glEvalCoord1d" );
	qglEvalCoord1dv              = safe_dlsym( g_hGLDLL, "glEvalCoord1dv" );
	qglEvalCoord1f               = safe_dlsym( g_hGLDLL, "glEvalCoord1f" );
	qglEvalCoord1fv              = safe_dlsym( g_hGLDLL, "glEvalCoord1fv" );
	qglEvalCoord2d               = safe_dlsym( g_hGLDLL, "glEvalCoord2d" );
	qglEvalCoord2dv              = safe_dlsym( g_hGLDLL, "glEvalCoord2dv" );
	qglEvalCoord2f               = safe_dlsym( g_hGLDLL, "glEvalCoord2f" );
	qglEvalCoord2fv              = safe_dlsym( g_hGLDLL, "glEvalCoord2fv" );
	qglEvalMesh1                 = safe_dlsym( g_hGLDLL, "glEvalMesh1" );
	qglEvalMesh2                 = safe_dlsym( g_hGLDLL, "glEvalMesh2" );
	qglEvalPoint1                = safe_dlsym( g_hGLDLL, "glEvalPoint1" );
	qglEvalPoint2                = safe_dlsym( g_hGLDLL, "glEvalPoint2" );
	qglFeedbackBuffer            = safe_dlsym( g_hGLDLL, "glFeedbackBuffer" );
	qglFinish                    = safe_dlsym( g_hGLDLL, "glFinish" );
	qglFlush                     = safe_dlsym( g_hGLDLL, "glFlush" );
	qglFogf                      = safe_dlsym( g_hGLDLL, "glFogf" );
	qglFogfv                     = safe_dlsym( g_hGLDLL, "glFogfv" );
	qglFogi                      = safe_dlsym( g_hGLDLL, "glFogi" );
	qglFogiv                     = safe_dlsym( g_hGLDLL, "glFogiv" );
	qglFrontFace                 = safe_dlsym( g_hGLDLL, "glFrontFace" );
	qglFrustum                   = safe_dlsym( g_hGLDLL, "glFrustum" );
	qglGenLists                  = safe_dlsym( g_hGLDLL, "glGenLists" );
	qglGenTextures               = safe_dlsym( g_hGLDLL, "glGenTextures" );
	qglGetBooleanv               = safe_dlsym( g_hGLDLL, "glGetBooleanv" );
	qglGetClipPlane              = safe_dlsym( g_hGLDLL, "glGetClipPlane" );
	qglGetDoublev                = safe_dlsym( g_hGLDLL, "glGetDoublev" );
	qglGetError                  = safe_dlsym( g_hGLDLL, "glGetError" );
	qglGetFloatv                 = safe_dlsym( g_hGLDLL, "glGetFloatv" );
	qglGetIntegerv               = safe_dlsym( g_hGLDLL, "glGetIntegerv" );
	qglGetLightfv                = safe_dlsym( g_hGLDLL, "glGetLightfv" );
	qglGetLightiv                = safe_dlsym( g_hGLDLL, "glGetLightiv" );
	qglGetMapdv                  = safe_dlsym( g_hGLDLL, "glGetMapdv" );
	qglGetMapfv                  = safe_dlsym( g_hGLDLL, "glGetMapfv" );
	qglGetMapiv                  = safe_dlsym( g_hGLDLL, "glGetMapiv" );
	qglGetMaterialfv             = safe_dlsym( g_hGLDLL, "glGetMaterialfv" );
	qglGetMaterialiv             = safe_dlsym( g_hGLDLL, "glGetMaterialiv" );
	qglGetPixelMapfv             = safe_dlsym( g_hGLDLL, "glGetPixelMapfv" );
	qglGetPixelMapuiv            = safe_dlsym( g_hGLDLL, "glGetPixelMapuiv" );
	qglGetPixelMapusv            = safe_dlsym( g_hGLDLL, "glGetPixelMapusv" );
	qglGetPointerv               = safe_dlsym( g_hGLDLL, "glGetPointerv" );
	qglGetPolygonStipple         = safe_dlsym( g_hGLDLL, "glGetPolygonStipple" );
	qglGetString                 = safe_dlsym( g_hGLDLL, "glGetString" );
	qglGetTexEnvfv               = safe_dlsym( g_hGLDLL, "glGetTexEnvfv" );
	qglGetTexEnviv               = safe_dlsym( g_hGLDLL, "glGetTexEnviv" );
	qglGetTexGendv               = safe_dlsym( g_hGLDLL, "glGetTexGendv" );
	qglGetTexGenfv               = safe_dlsym( g_hGLDLL, "glGetTexGenfv" );
	qglGetTexGeniv               = safe_dlsym( g_hGLDLL, "glGetTexGeniv" );
	qglGetTexImage               = safe_dlsym( g_hGLDLL, "glGetTexImage" );
	qglGetTexLevelParameterfv    = safe_dlsym( g_hGLDLL, "glGetTexLevelParameterfv" );
	qglGetTexLevelParameteriv    = safe_dlsym( g_hGLDLL, "glGetTexLevelParameteriv" );
	qglGetTexParameterfv         = safe_dlsym( g_hGLDLL, "glGetTexParameterfv" );
	qglGetTexParameteriv         = safe_dlsym( g_hGLDLL, "glGetTexParameteriv" );
	qglHint                      = safe_dlsym( g_hGLDLL, "glHint" );
	qglIndexMask                 = safe_dlsym( g_hGLDLL, "glIndexMask" );
	qglIndexPointer              = safe_dlsym( g_hGLDLL, "glIndexPointer" );
	qglIndexd                    = safe_dlsym( g_hGLDLL, "glIndexd" );
	qglIndexdv                   = safe_dlsym( g_hGLDLL, "glIndexdv" );
	qglIndexf                    = safe_dlsym( g_hGLDLL, "glIndexf" );
	qglIndexfv                   = safe_dlsym( g_hGLDLL, "glIndexfv" );
	qglIndexi                    = safe_dlsym( g_hGLDLL, "glIndexi" );
	qglIndexiv                   = safe_dlsym( g_hGLDLL, "glIndexiv" );
	qglIndexs                    = safe_dlsym( g_hGLDLL, "glIndexs" );
	qglIndexsv                   = safe_dlsym( g_hGLDLL, "glIndexsv" );
	qglIndexub                   = safe_dlsym( g_hGLDLL, "glIndexub" );
	qglIndexubv                  = safe_dlsym( g_hGLDLL, "glIndexubv" );
	qglInitNames                 = safe_dlsym( g_hGLDLL, "glInitNames" );
	qglInterleavedArrays         = safe_dlsym( g_hGLDLL, "glInterleavedArrays" );
	qglIsEnabled                 = safe_dlsym( g_hGLDLL, "glIsEnabled" );
	qglIsList                    = safe_dlsym( g_hGLDLL, "glIsList" );
	qglIsTexture                 = safe_dlsym( g_hGLDLL, "glIsTexture" );
	qglLightModelf               = safe_dlsym( g_hGLDLL, "glLightModelf" );
	qglLightModelfv              = safe_dlsym( g_hGLDLL, "glLightModelfv" );
	qglLightModeli               = safe_dlsym( g_hGLDLL, "glLightModeli" );
	qglLightModeliv              = safe_dlsym( g_hGLDLL, "glLightModeliv" );
	qglLightf                    = safe_dlsym( g_hGLDLL, "glLightf" );
	qglLightfv                   = safe_dlsym( g_hGLDLL, "glLightfv" );
	qglLighti                    = safe_dlsym( g_hGLDLL, "glLighti" );
	qglLightiv                   = safe_dlsym( g_hGLDLL, "glLightiv" );
	qglLineStipple               = safe_dlsym( g_hGLDLL, "glLineStipple" );
	qglLineWidth                 = safe_dlsym( g_hGLDLL, "glLineWidth" );
	qglListBase                  = safe_dlsym( g_hGLDLL, "glListBase" );
	qglLoadIdentity              = safe_dlsym( g_hGLDLL, "glLoadIdentity" );
	qglLoadMatrixd               = safe_dlsym( g_hGLDLL, "glLoadMatrixd" );
	qglLoadMatrixf               = safe_dlsym( g_hGLDLL, "glLoadMatrixf" );
	qglLoadName                  = safe_dlsym( g_hGLDLL, "glLoadName" );
	qglLogicOp                   = safe_dlsym( g_hGLDLL, "glLogicOp" );
	qglMap1d                     = safe_dlsym( g_hGLDLL, "glMap1d" );
	qglMap1f                     = safe_dlsym( g_hGLDLL, "glMap1f" );
	qglMap2d                     = safe_dlsym( g_hGLDLL, "glMap2d" );
	qglMap2f                     = safe_dlsym( g_hGLDLL, "glMap2f" );
	qglMapGrid1d                 = safe_dlsym( g_hGLDLL, "glMapGrid1d" );
	qglMapGrid1f                 = safe_dlsym( g_hGLDLL, "glMapGrid1f" );
	qglMapGrid2d                 = safe_dlsym( g_hGLDLL, "glMapGrid2d" );
	qglMapGrid2f                 = safe_dlsym( g_hGLDLL, "glMapGrid2f" );
	qglMaterialf                 = safe_dlsym( g_hGLDLL, "glMaterialf" );
	qglMaterialfv                = safe_dlsym( g_hGLDLL, "glMaterialfv" );
	qglMateriali                 = safe_dlsym( g_hGLDLL, "glMateriali" );
	qglMaterialiv                = safe_dlsym( g_hGLDLL, "glMaterialiv" );
	qglMatrixMode                = safe_dlsym( g_hGLDLL, "glMatrixMode" );
	qglMultMatrixd               = safe_dlsym( g_hGLDLL, "glMultMatrixd" );
	qglMultMatrixf               = safe_dlsym( g_hGLDLL, "glMultMatrixf" );
	qglNewList                   = safe_dlsym( g_hGLDLL, "glNewList" );
	qglNormal3b                  = safe_dlsym( g_hGLDLL, "glNormal3b" );
	qglNormal3bv                 = safe_dlsym( g_hGLDLL, "glNormal3bv" );
	qglNormal3d                  = safe_dlsym( g_hGLDLL, "glNormal3d" );
	qglNormal3dv                 = safe_dlsym( g_hGLDLL, "glNormal3dv" );
	qglNormal3f                  = safe_dlsym( g_hGLDLL, "glNormal3f" );
	qglNormal3fv                 = safe_dlsym( g_hGLDLL, "glNormal3fv" );
	qglNormal3i                  = safe_dlsym( g_hGLDLL, "glNormal3i" );
	qglNormal3iv                 = safe_dlsym( g_hGLDLL, "glNormal3iv" );
	qglNormal3s                  = safe_dlsym( g_hGLDLL, "glNormal3s" );
	qglNormal3sv                 = safe_dlsym( g_hGLDLL, "glNormal3sv" );
	qglNormalPointer             = safe_dlsym( g_hGLDLL, "glNormalPointer" );
	qglOrtho                     = safe_dlsym( g_hGLDLL, "glOrtho" );
	qglPassThrough               = safe_dlsym( g_hGLDLL, "glPassThrough" );
	qglPixelMapfv                = safe_dlsym( g_hGLDLL, "glPixelMapfv" );
	qglPixelMapuiv               = safe_dlsym( g_hGLDLL, "glPixelMapuiv" );
	qglPixelMapusv               = safe_dlsym( g_hGLDLL, "glPixelMapusv" );
	qglPixelStoref               = safe_dlsym( g_hGLDLL, "glPixelStoref" );
	qglPixelStorei               = safe_dlsym( g_hGLDLL, "glPixelStorei" );
	qglPixelTransferf            = safe_dlsym( g_hGLDLL, "glPixelTransferf" );
	qglPixelTransferi            = safe_dlsym( g_hGLDLL, "glPixelTransferi" );
	qglPixelZoom                 = safe_dlsym( g_hGLDLL, "glPixelZoom" );
	qglPointSize                 = safe_dlsym( g_hGLDLL, "glPointSize" );
#ifdef ATIHACK_812
	qglPolygonMode_real           = safe_dlsym( g_hGLDLL, "glPolygonMode" );
	qglPolygonMode               = qglPolygonMode_real;
#else
	qglPolygonMode               = safe_dlsym( g_hGLDLL, "glPolygonMode" );
#endif
	qglPolygonOffset             = safe_dlsym( g_hGLDLL, "glPolygonOffset" );
	qglPolygonStipple            = safe_dlsym( g_hGLDLL, "glPolygonStipple" );
	qglPopAttrib                 = safe_dlsym( g_hGLDLL, "glPopAttrib" );
	qglPopClientAttrib           = safe_dlsym( g_hGLDLL, "glPopClientAttrib" );
	qglPopMatrix                 = safe_dlsym( g_hGLDLL, "glPopMatrix" );
	qglPopName                   = safe_dlsym( g_hGLDLL, "glPopName" );
	qglPrioritizeTextures        = safe_dlsym( g_hGLDLL, "glPrioritizeTextures" );
	qglPushAttrib                = safe_dlsym( g_hGLDLL, "glPushAttrib" );
	qglPushClientAttrib          = safe_dlsym( g_hGLDLL, "glPushClientAttrib" );
	qglPushMatrix                = safe_dlsym( g_hGLDLL, "glPushMatrix" );
	qglPushName                  = safe_dlsym( g_hGLDLL, "glPushName" );
	qglRasterPos2d               = safe_dlsym( g_hGLDLL, "glRasterPos2d" );
	qglRasterPos2dv              = safe_dlsym( g_hGLDLL, "glRasterPos2dv" );
	qglRasterPos2f               = safe_dlsym( g_hGLDLL, "glRasterPos2f" );
	qglRasterPos2fv              = safe_dlsym( g_hGLDLL, "glRasterPos2fv" );
	qglRasterPos2i               = safe_dlsym( g_hGLDLL, "glRasterPos2i" );
	qglRasterPos2iv              = safe_dlsym( g_hGLDLL, "glRasterPos2iv" );
	qglRasterPos2s               = safe_dlsym( g_hGLDLL, "glRasterPos2s" );
	qglRasterPos2sv              = safe_dlsym( g_hGLDLL, "glRasterPos2sv" );
	qglRasterPos3d               = safe_dlsym( g_hGLDLL, "glRasterPos3d" );
	qglRasterPos3dv              = safe_dlsym( g_hGLDLL, "glRasterPos3dv" );
	qglRasterPos3f               = safe_dlsym( g_hGLDLL, "glRasterPos3f" );
	qglRasterPos3fv              = safe_dlsym( g_hGLDLL, "glRasterPos3fv" );
	qglRasterPos3i               = safe_dlsym( g_hGLDLL, "glRasterPos3i" );
	qglRasterPos3iv              = safe_dlsym( g_hGLDLL, "glRasterPos3iv" );
	qglRasterPos3s               = safe_dlsym( g_hGLDLL, "glRasterPos3s" );
	qglRasterPos3sv              = safe_dlsym( g_hGLDLL, "glRasterPos3sv" );
	qglRasterPos4d               = safe_dlsym( g_hGLDLL, "glRasterPos4d" );
	qglRasterPos4dv              = safe_dlsym( g_hGLDLL, "glRasterPos4dv" );
	qglRasterPos4f               = safe_dlsym( g_hGLDLL, "glRasterPos4f" );
	qglRasterPos4fv              = safe_dlsym( g_hGLDLL, "glRasterPos4fv" );
	qglRasterPos4i               = safe_dlsym( g_hGLDLL, "glRasterPos4i" );
	qglRasterPos4iv              = safe_dlsym( g_hGLDLL, "glRasterPos4iv" );
	qglRasterPos4s               = safe_dlsym( g_hGLDLL, "glRasterPos4s" );
	qglRasterPos4sv              = safe_dlsym( g_hGLDLL, "glRasterPos4sv" );
	qglReadBuffer                = safe_dlsym( g_hGLDLL, "glReadBuffer" );
	qglReadPixels                = safe_dlsym( g_hGLDLL, "glReadPixels" );
	qglRectd                     = safe_dlsym( g_hGLDLL, "glRectd" );
	qglRectdv                    = safe_dlsym( g_hGLDLL, "glRectdv" );
	qglRectf                     = safe_dlsym( g_hGLDLL, "glRectf" );
	qglRectfv                    = safe_dlsym( g_hGLDLL, "glRectfv" );
	qglRecti                     = safe_dlsym( g_hGLDLL, "glRecti" );
	qglRectiv                    = safe_dlsym( g_hGLDLL, "glRectiv" );
	qglRects                     = safe_dlsym( g_hGLDLL, "glRects" );
	qglRectsv                    = safe_dlsym( g_hGLDLL, "glRectsv" );
	qglRenderMode                = safe_dlsym( g_hGLDLL, "glRenderMode" );
	qglRotated                   = safe_dlsym( g_hGLDLL, "glRotated" );
	qglRotatef                   = safe_dlsym( g_hGLDLL, "glRotatef" );
	qglScaled                    = safe_dlsym( g_hGLDLL, "glScaled" );
	qglScalef                    = safe_dlsym( g_hGLDLL, "glScalef" );
	qglScissor                   = safe_dlsym( g_hGLDLL, "glScissor" );
	qglSelectBuffer              = safe_dlsym( g_hGLDLL, "glSelectBuffer" );
	qglShadeModel                = safe_dlsym( g_hGLDLL, "glShadeModel" );
	qglStencilFunc               = safe_dlsym( g_hGLDLL, "glStencilFunc" );
	qglStencilMask               = safe_dlsym( g_hGLDLL, "glStencilMask" );
	qglStencilOp                 = safe_dlsym( g_hGLDLL, "glStencilOp" );
	qglTexCoord1d                = safe_dlsym( g_hGLDLL, "glTexCoord1d" );
	qglTexCoord1dv               = safe_dlsym( g_hGLDLL, "glTexCoord1dv" );
	qglTexCoord1f                = safe_dlsym( g_hGLDLL, "glTexCoord1f" );
	qglTexCoord1fv               = safe_dlsym( g_hGLDLL, "glTexCoord1fv" );
	qglTexCoord1i                = safe_dlsym( g_hGLDLL, "glTexCoord1i" );
	qglTexCoord1iv               = safe_dlsym( g_hGLDLL, "glTexCoord1iv" );
	qglTexCoord1s                = safe_dlsym( g_hGLDLL, "glTexCoord1s" );
	qglTexCoord1sv               = safe_dlsym( g_hGLDLL, "glTexCoord1sv" );
	qglTexCoord2d                = safe_dlsym( g_hGLDLL, "glTexCoord2d" );
	qglTexCoord2dv               = safe_dlsym( g_hGLDLL, "glTexCoord2dv" );
	qglTexCoord2f                = safe_dlsym( g_hGLDLL, "glTexCoord2f" );
	qglTexCoord2fv               = safe_dlsym( g_hGLDLL, "glTexCoord2fv" );
	qglTexCoord2i                = safe_dlsym( g_hGLDLL, "glTexCoord2i" );
	qglTexCoord2iv               = safe_dlsym( g_hGLDLL, "glTexCoord2iv" );
	qglTexCoord2s                = safe_dlsym( g_hGLDLL, "glTexCoord2s" );
	qglTexCoord2sv               = safe_dlsym( g_hGLDLL, "glTexCoord2sv" );
	qglTexCoord3d                = safe_dlsym( g_hGLDLL, "glTexCoord3d" );
	qglTexCoord3dv               = safe_dlsym( g_hGLDLL, "glTexCoord3dv" );
	qglTexCoord3f                = safe_dlsym( g_hGLDLL, "glTexCoord3f" );
	qglTexCoord3fv               = safe_dlsym( g_hGLDLL, "glTexCoord3fv" );
	qglTexCoord3i                = safe_dlsym( g_hGLDLL, "glTexCoord3i" );
	qglTexCoord3iv               = safe_dlsym( g_hGLDLL, "glTexCoord3iv" );
	qglTexCoord3s                = safe_dlsym( g_hGLDLL, "glTexCoord3s" );
	qglTexCoord3sv               = safe_dlsym( g_hGLDLL, "glTexCoord3sv" );
	qglTexCoord4d                = safe_dlsym( g_hGLDLL, "glTexCoord4d" );
	qglTexCoord4dv               = safe_dlsym( g_hGLDLL, "glTexCoord4dv" );
	qglTexCoord4f                = safe_dlsym( g_hGLDLL, "glTexCoord4f" );
	qglTexCoord4fv               = safe_dlsym( g_hGLDLL, "glTexCoord4fv" );
	qglTexCoord4i                = safe_dlsym( g_hGLDLL, "glTexCoord4i" );
	qglTexCoord4iv               = safe_dlsym( g_hGLDLL, "glTexCoord4iv" );
	qglTexCoord4s                = safe_dlsym( g_hGLDLL, "glTexCoord4s" );
	qglTexCoord4sv               = safe_dlsym( g_hGLDLL, "glTexCoord4sv" );
	qglTexCoordPointer           = safe_dlsym( g_hGLDLL, "glTexCoordPointer" );
	qglTexEnvf                   = safe_dlsym( g_hGLDLL, "glTexEnvf" );
	qglTexEnvfv                  = safe_dlsym( g_hGLDLL, "glTexEnvfv" );
	qglTexEnvi                   = safe_dlsym( g_hGLDLL, "glTexEnvi" );
	qglTexEnviv                  = safe_dlsym( g_hGLDLL, "glTexEnviv" );
	qglTexGend                   = safe_dlsym( g_hGLDLL, "glTexGend" );
	qglTexGendv                  = safe_dlsym( g_hGLDLL, "glTexGendv" );
	qglTexGenf                   = safe_dlsym( g_hGLDLL, "glTexGenf" );
	qglTexGenfv                  = safe_dlsym( g_hGLDLL, "glTexGenfv" );
	qglTexGeni                   = safe_dlsym( g_hGLDLL, "glTexGeni" );
	qglTexGeniv                  = safe_dlsym( g_hGLDLL, "glTexGeniv" );
	qglTexImage1D                = safe_dlsym( g_hGLDLL, "glTexImage1D" );
	qglTexImage2D                = safe_dlsym( g_hGLDLL, "glTexImage2D" );
	qglTexParameterf             = safe_dlsym( g_hGLDLL, "glTexParameterf" );
	qglTexParameterfv            = safe_dlsym( g_hGLDLL, "glTexParameterfv" );
	qglTexParameteri             = safe_dlsym( g_hGLDLL, "glTexParameteri" );
	qglTexParameteriv            = safe_dlsym( g_hGLDLL, "glTexParameteriv" );
	qglTexSubImage1D             = safe_dlsym( g_hGLDLL, "glTexSubImage1D" );
	qglTexSubImage2D             = safe_dlsym( g_hGLDLL, "glTexSubImage2D" );
	qglTranslated                = safe_dlsym( g_hGLDLL, "glTranslated" );
	qglTranslatef                = safe_dlsym( g_hGLDLL, "glTranslatef" );
	qglVertex2d                  = safe_dlsym( g_hGLDLL, "glVertex2d" );
	qglVertex2dv                 = safe_dlsym( g_hGLDLL, "glVertex2dv" );
	qglVertex2f                  = safe_dlsym( g_hGLDLL, "glVertex2f" );
	qglVertex2fv                 = safe_dlsym( g_hGLDLL, "glVertex2fv" );
	qglVertex2i                  = safe_dlsym( g_hGLDLL, "glVertex2i" );
	qglVertex2iv                 = safe_dlsym( g_hGLDLL, "glVertex2iv" );
	qglVertex2s                  = safe_dlsym( g_hGLDLL, "glVertex2s" );
	qglVertex2sv                 = safe_dlsym( g_hGLDLL, "glVertex2sv" );
	qglVertex3d                  = safe_dlsym( g_hGLDLL, "glVertex3d" );
	qglVertex3dv                 = safe_dlsym( g_hGLDLL, "glVertex3dv" );
	qglVertex3f                  = safe_dlsym( g_hGLDLL, "glVertex3f" );
	qglVertex3fv                 = safe_dlsym( g_hGLDLL, "glVertex3fv" );
	qglVertex3i                  = safe_dlsym( g_hGLDLL, "glVertex3i" );
	qglVertex3iv                 = safe_dlsym( g_hGLDLL, "glVertex3iv" );
	qglVertex3s                  = safe_dlsym( g_hGLDLL, "glVertex3s" );
	qglVertex3sv                 = safe_dlsym( g_hGLDLL, "glVertex3sv" );
	qglVertex4d                  = safe_dlsym( g_hGLDLL, "glVertex4d" );
	qglVertex4dv                 = safe_dlsym( g_hGLDLL, "glVertex4dv" );
	qglVertex4f                  = safe_dlsym( g_hGLDLL, "glVertex4f" );
	qglVertex4fv                 = safe_dlsym( g_hGLDLL, "glVertex4fv" );
	qglVertex4i                  = safe_dlsym( g_hGLDLL, "glVertex4i" );
	qglVertex4iv                 = safe_dlsym( g_hGLDLL, "glVertex4iv" );
	qglVertex4s                  = safe_dlsym( g_hGLDLL, "glVertex4s" );
	qglVertex4sv                 = safe_dlsym( g_hGLDLL, "glVertex4sv" );
	qglVertexPointer             = safe_dlsym( g_hGLDLL, "glVertexPointer" );
	qglViewport                  = safe_dlsym( g_hGLDLL, "glViewport" );

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
	qwglCopyContext              = safe_dlsym( g_hGLDLL, "wglCopyContext" );
	qwglCreateContext            = safe_dlsym( g_hGLDLL, "wglCreateContext" );
	qwglCreateLayerContext       = safe_dlsym( g_hGLDLL, "wglCreateLayerContext" );
	qwglDeleteContext            = safe_dlsym( g_hGLDLL, "wglDeleteContext" );
	qwglDescribeLayerPlane       = safe_dlsym( g_hGLDLL, "wglDescribeLayerPlane" );
	qwglGetCurrentContext        = safe_dlsym( g_hGLDLL, "wglGetCurrentContext" );
	qwglGetCurrentDC             = safe_dlsym( g_hGLDLL, "wglGetCurrentDC" );
	qwglGetLayerPaletteEntries   = safe_dlsym( g_hGLDLL, "wglGetLayerPaletteEntries" );
	qwglGetProcAddress           = safe_dlsym( g_hGLDLL, "wglGetProcAddress" );
	qwglMakeCurrent              = safe_dlsym( g_hGLDLL, "wglMakeCurrent" );
	qwglRealizeLayerPalette      = safe_dlsym( g_hGLDLL, "wglRealizeLayerPalette" );
	qwglSetLayerPaletteEntries   = safe_dlsym( g_hGLDLL, "wglSetLayerPaletteEntries" );
	qwglShareLists               = safe_dlsym( g_hGLDLL, "wglShareLists" );
	qwglSwapLayerBuffers         = safe_dlsym( g_hGLDLL, "wglSwapLayerBuffers" );
	qwglUseFontBitmaps           = safe_dlsym( g_hGLDLL, "wglUseFontBitmapsA" );
	qwglUseFontOutlines          = safe_dlsym( g_hGLDLL, "wglUseFontOutlinesA" );

	qwglChoosePixelFormat        = safe_dlsym( g_hGLDLL, "wglChoosePixelFormat" );
	qwglDescribePixelFormat      = safe_dlsym( g_hGLDLL, "wglDescribePixelFormat" );
	qwglGetPixelFormat           = safe_dlsym( g_hGLDLL, "wglGetPixelFormat" );
	qwglSetPixelFormat           = safe_dlsym( g_hGLDLL, "wglSetPixelFormat" );
	qwglSwapBuffers              = safe_dlsym( g_hGLDLL, "wglSwapBuffers" );

	qwglSwapIntervalEXT = 0;
	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	qglXChooseVisual             = safe_dlsym( g_hGLDLL, "glXChooseVisual" );
	qglXCreateContext            = safe_dlsym( g_hGLDLL, "glXCreateContext" );
	qglXDestroyContext           = safe_dlsym( g_hGLDLL, "glXDestroyContext" );
	qglXMakeCurrent              = safe_dlsym( g_hGLDLL, "glXMakeCurrent" );
	qglXCopyContext              = safe_dlsym( g_hGLDLL, "glXCopyContext" );
	qglXSwapBuffers              = safe_dlsym( g_hGLDLL, "glXSwapBuffers" );
	qglXCreateGLXPixmap          = safe_dlsym( g_hGLDLL, "glXCreateGLXPixmap" );
	qglXDestroyGLXPixmap         = safe_dlsym( g_hGLDLL, "glXDestroyGLXPixmap" );
	qglXQueryExtension           = safe_dlsym( g_hGLDLL, "glXQueryExtension" );
	qglXQueryVersion             = safe_dlsym( g_hGLDLL, "glXQueryVersion" );
	qglXIsDirect                 = safe_dlsym( g_hGLDLL, "glXIsDirect" );
	qglXGetConfig                = safe_dlsym( g_hGLDLL, "glXGetConfig" );
	qglXGetCurrentContext        = safe_dlsym( g_hGLDLL, "glXGetCurrentContext" );
	qglXGetCurrentDrawable       = safe_dlsym( g_hGLDLL, "glXGetCurrentDrawable" );
	qglXWaitGL                   = safe_dlsym( g_hGLDLL, "glXWaitGL" );
	qglXWaitX                    = safe_dlsym( g_hGLDLL, "glXWaitX" );
	qglXUseXFont                 = safe_dlsym( g_hGLDLL, "glXUseXFont" );
#endif

	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;

	Sys_Printf( "Done.\n" );

#ifdef ATIHACK_812
	ATIhack_culling = 0;
	ATIhack_cullmode = GL_BACK;
	ATIhack_backmode = GL_FILL;
	ATIhack_frontmode = GL_FILL;
#endif

	if ( init_error == 1 ) {
		return 0;
	}

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
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
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
