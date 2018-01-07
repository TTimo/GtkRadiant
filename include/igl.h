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

//-----------------------------------------------------------------------------
//
//
// DESCRIPTION:
// all purpose OpenGL interface for Q3Radiant plugins
//

#ifndef __IGL_H__
#define __IGL_H__

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <GL/glx.h>
#endif

// we use these classes to let plugins draw inside the Radiant windows
// 2D window like YZ XZ XY
class IGL2DWindow
{
public:
virtual ~IGL2DWindow() { }
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
virtual void Draw2D( VIEWTYPE vt ) = 0;
};

// 3D window
class IGL3DWindow
{
public:
virtual ~IGL3DWindow() { }
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
virtual void Draw3D() = 0;
};

#define QGL_MAJOR "qgl"

#include <GL/gl.h>

typedef void ( APIENTRY * PFN_QGLALPHAFUNC )( GLenum func, GLclampf ref );
typedef void ( APIENTRY * PFN_QGLBEGIN )( GLenum );
typedef void ( APIENTRY * PFN_QGLBINDTEXTURE )( GLenum target, GLuint texture );
typedef void ( APIENTRY * PFN_QGLBLENDFUNC )( GLenum sfactor, GLenum dfactor );
typedef void ( APIENTRY * PFN_QGLCALLLIST )( GLuint list );
typedef void ( APIENTRY * PFN_QGLCALLLISTS )( GLsizei n, GLenum type, const GLvoid *lists );
typedef void ( APIENTRY * PFN_QGLCLEAR )( GLbitfield mask );
typedef void ( APIENTRY * PFN_QGLCLEARCOLOR )( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
typedef void ( APIENTRY * PFN_QGLCLEARDEPTH )( GLclampd depth );
typedef void ( APIENTRY * PFN_QGLCOLOR3F )( GLfloat red, GLfloat green, GLfloat blue );
typedef void ( APIENTRY * PFN_QGLCOLOR3FV )( const GLfloat *v );
typedef void ( APIENTRY * PFN_QGLCOLOR4F )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
typedef void ( APIENTRY * PFN_QGLCOLOR4FV )( const GLfloat *v );
typedef void ( APIENTRY * PFN_QGLCOLOR4UBV )( const GLubyte *v );
typedef void ( APIENTRY * PFN_QGLCOLORPOINTER )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( APIENTRY * PFN_QGLCULLFACE )( GLenum mode );
typedef void ( APIENTRY * PFN_QGLDELETELISTS )( GLuint list, GLsizei range );
typedef void ( APIENTRY * PFN_QGLDELETETEXTURES )( GLsizei n, const GLuint *textures );
typedef void ( APIENTRY * PFN_QGLDEPTHFUNC )( GLenum func );
typedef void ( APIENTRY * PFN_QGLDEPTHMASK )( GLboolean flag );
typedef void ( APIENTRY * PFN_QGLDISABLE )( GLenum cap );
typedef void ( APIENTRY * PFN_QGLDISABLECLIENTSTATE )( GLenum array );
typedef void ( APIENTRY * PFN_QGLDRAWELEMENTS )( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
typedef void ( APIENTRY * PFN_QGLENABLE )( GLenum cap );
typedef void ( APIENTRY * PFN_QGLENABLECLIENTSTATE )( GLenum array );
typedef void ( APIENTRY * PFN_QGLEND )();
typedef void ( APIENTRY * PFN_QGLENDLIST )();
typedef void ( APIENTRY * PFN_QGLFOGF )( GLenum pname, GLfloat param );
typedef void ( APIENTRY * PFN_QGLFOGFV )( GLenum pname, const GLfloat *params );
typedef void ( APIENTRY * PFN_QGLFOGFI )( GLenum pname, GLint param );
typedef GLuint ( APIENTRY * PFN_QGLGENLISTS )( GLsizei range );
typedef void ( APIENTRY * PFN_QGLGENTEXTURES )( GLsizei n, GLuint *textures );
typedef void ( APIENTRY * PFN_QGLGETDOUBLEV )( GLenum pname, GLdouble *params );
typedef void ( APIENTRY * PFN_QGLHINT )( GLenum target, GLenum mode );
typedef void ( APIENTRY * PFN_QGLGETINTEGERV )( GLenum pname, GLint *params );
typedef void ( APIENTRY * PFN_QGLLIGHTFV )( GLenum light, GLenum pname, const GLfloat *params );
typedef void ( APIENTRY * PFN_QGLLINEWIDTH )( GLfloat size );
typedef void ( APIENTRY * PFN_QGLLINESTIPPLE )( GLint factor, GLushort pattern );
typedef void ( APIENTRY * PFN_QGLLINEWIDTH )( GLfloat size );
typedef void ( APIENTRY * PFN_QGLLISTBASE )( GLuint base );
typedef void ( APIENTRY * PFN_QGLLOADIDENTITY )();
typedef void ( APIENTRY * PFN_QGLMATERIALF )( GLenum face, GLenum pname, GLfloat param );
typedef void ( APIENTRY * PFN_QGLMATERIALFV )( GLenum face, GLenum pname, const GLfloat *params );
typedef void ( APIENTRY * PFN_QGLMATRIXMODE )( GLenum mode );
typedef void ( APIENTRY * PFN_QGLMULTMATRIXF )( const GLfloat *m );
typedef void ( APIENTRY * PFN_QGLNEWLIST )( GLuint list, GLenum mode );
typedef void ( APIENTRY * PFN_QGLNORMAL3F )( GLfloat nx, GLfloat ny, GLfloat nz );
typedef void ( APIENTRY * PFN_QGLNORMAL3FV )( const GLfloat *n );
typedef void ( APIENTRY * PFN_QGLNORMALPOINTER )( GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( APIENTRY * PFN_QGLORTHO )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
typedef void ( APIENTRY * PFN_QGLPOINTSIZE )( GLfloat size );
typedef void ( APIENTRY * PFN_QGLPOLYGONMODE )( GLenum face, GLenum mode );
typedef void ( APIENTRY * PFN_QGLPOPATTRIB )();
typedef void ( APIENTRY * PFN_QGLPOPMATRIX )();
typedef void ( APIENTRY * PFN_QGLPUSHATTRIB )( GLbitfield mask );
typedef void ( APIENTRY * PFN_QGLPUSHMATRIX )();
typedef void ( APIENTRY * PFN_QGLRASTERPOS3FV )( const GLfloat *v );
typedef void ( APIENTRY * PFN_QGLROTATED )( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );
typedef void ( APIENTRY * PFN_QGLROTATEF )( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );
typedef void ( APIENTRY * PFN_QGLSCALEF )( GLfloat x, GLfloat y, GLfloat z );
typedef void ( APIENTRY * PFN_QGLSCISSOR )( GLint x, GLint y, GLsizei width, GLsizei height );
typedef void ( APIENTRY * PFN_QGLSCALEF )( GLfloat x, GLfloat y, GLfloat z );
typedef void ( APIENTRY * PFN_QGLSHADEMODEL )( GLenum mode );
typedef void ( APIENTRY * PFN_QGLTEXCOORD2F )( GLfloat s, GLfloat t );
typedef void ( APIENTRY * PFN_QGLTEXCOORD2FV )( const GLfloat *v );
typedef void ( APIENTRY * PFN_QGLTEXCOORDPOINTER )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( APIENTRY * PFN_QGLTEXENVF )( GLenum target, GLenum pname, GLfloat param );
typedef void ( APIENTRY * PFN_QGLTEXGENF )( GLenum coord, GLenum pname, GLfloat param );
typedef void ( APIENTRY * PFN_QGLTEXIMAGE1D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( APIENTRY * PFN_QGLTEXIMAGE2D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( APIENTRY * PFN_QGLTEXPARAMETERF )( GLenum target, GLenum pname, GLfloat param );
typedef void ( APIENTRY * PFN_QGLTEXPARAMETERFV )( GLenum target, GLenum pname, const GLfloat *params );
typedef void ( APIENTRY * PFN_QGLTEXPARAMETERI )( GLenum target, GLenum pname, GLint param );
typedef void ( APIENTRY * PFN_QGLTEXPARAMETERIV )( GLenum target, GLenum pname, const GLint *params );
typedef void ( APIENTRY * PFN_QGLTEXSUBIMAGE1D )( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( APIENTRY * PFN_QGLTEXSUBIMAGE2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( APIENTRY * PFN_QGLTRANSLATED )( GLdouble x, GLdouble y, GLdouble z );
typedef void ( APIENTRY * PFN_QGLTRANSLATEF )( GLfloat x, GLfloat y, GLfloat z );
typedef void ( APIENTRY * PFN_QGLVERTEX2F )( GLfloat x, GLfloat y );
typedef void ( APIENTRY * PFN_QGLVERTEX3F )( GLfloat x, GLfloat y, GLfloat z );
typedef void ( APIENTRY * PFN_QGLVERTEX3FV )( const GLfloat *v );
typedef void ( APIENTRY * PFN_QGLVERTEXPOINTER )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( APIENTRY * PFN_QGLVIEWPORT )( GLint x, GLint y, GLsizei width, GLsizei height );

typedef void ( WINAPI * PFN_QE_CHECKOPENGLFORERRORS )();

// glu stuff
// TTimo: NOTE: relying on glu might not be such a good idea. On many systems, the GLU lib is outdated, misversioned etc.
typedef void ( APIENTRY * PFN_QGLUPERSPECTIVE )( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar );
typedef void ( APIENTRY * PFN_QGLULOOKAT )( GLdouble eyex, GLdouble eyey, GLdouble eyez,
											GLdouble centerx, GLdouble centery, GLdouble centerz,
											GLdouble upx, GLdouble upy, GLdouble upz );
//++timo gluErrorString is defined but not exposed in the IGL interface


// plugins drawing inside the GL windows
//++timo TODO: add hooking into other windows (Z and .. texture??)
//+timo NOTE: this could be moved to the messaging system instead of having a dedicated interface <- yet I don't know how
typedef void ( WINAPI * PFN_QERAPP_HOOKGL2DWINDOW )( IGL2DWindow * );
typedef void ( WINAPI * PFN_QERAPP_UNHOOKGL2DWINDOW )( IGL2DWindow * );
typedef void ( WINAPI * PFN_QERAPP_HOOKGL3DWINDOW )( IGL3DWindow * );
typedef void ( WINAPI * PFN_QERAPP_UNHOOKGL3DWINDOW )( IGL3DWindow * );

struct _QERQglTable
{
	//++timo do we really wanna play with versions ?
	//	float m_fVersion;
	int m_nSize;
	PFN_QGLALPHAFUNC m_pfn_qglAlphaFunc;
	PFN_QGLBEGIN m_pfn_qglBegin;
	PFN_QGLBINDTEXTURE m_pfn_qglBindTexture;
	PFN_QGLBLENDFUNC m_pfn_qglBlendFunc;
	PFN_QGLCALLLIST m_pfn_qglCallList;
	PFN_QGLCLEAR m_pfn_qglClear;
	PFN_QGLCLEARCOLOR m_pfn_qglClearColor;
	PFN_QGLCALLLISTS m_pfn_qglCallLists;
	PFN_QGLCLEARDEPTH m_pfn_qglClearDepth;
	PFN_QGLCOLOR3F m_pfn_qglColor3f;
	PFN_QGLCOLOR3FV m_pfn_qglColor3fv;
	PFN_QGLCOLOR4F m_pfn_qglColor4f;
	PFN_QGLCOLOR4FV m_pfn_qglColor4fv;
	PFN_QGLCOLOR4UBV m_pfn_qglColor4ubv;        // ydnar
	PFN_QGLCOLORPOINTER m_pfn_qglColorPointer;
	PFN_QGLCULLFACE m_pfn_qglCullFace;
	PFN_QGLDELETELISTS m_pfn_qglDeleteLists;
	PFN_QGLDELETETEXTURES m_pfn_qglDeleteTextures;
	PFN_QGLDEPTHFUNC m_pfn_qglDepthFunc;
	PFN_QGLDEPTHMASK m_pfn_qglDepthMask;
	PFN_QGLDISABLE m_pfn_qglDisable;
	PFN_QGLDISABLECLIENTSTATE m_pfn_qglDisableClientState;
	PFN_QGLDRAWELEMENTS m_pfn_qglDrawElements;
	PFN_QGLENABLE m_pfn_qglEnable;
	PFN_QGLENABLECLIENTSTATE m_pfn_qglEnableClientState;
	PFN_QGLEND m_pfn_qglEnd;
	PFN_QGLENDLIST m_pfn_qglEndList;
	PFN_QGLFOGF m_pfn_qglFogf;
	PFN_QGLFOGFV m_pfn_qglFogfv;
	PFN_QGLFOGFI m_pfn_qglFogi;
	PFN_QGLGENLISTS m_pfn_qglGenLists;
	PFN_QGLGENTEXTURES m_pfn_qglGenTextures;
	PFN_QGLGETDOUBLEV m_pfn_qglGetDoublev;
	PFN_QGLGETINTEGERV m_pfn_qglGetIntegerv;
	PFN_QGLHINT m_pfn_qglHint;
	PFN_QGLLIGHTFV m_pfn_qglLightfv;
	PFN_QGLLINESTIPPLE m_pfn_qglLineStipple;
	PFN_QGLLINEWIDTH m_pfn_qglLineWidth;
	PFN_QGLLISTBASE m_pfn_qglListBase;
	PFN_QGLLOADIDENTITY m_pfn_qglLoadIdentity;
	PFN_QGLMATERIALF m_pfn_qglMaterialf;
	PFN_QGLMATERIALFV m_pfn_qglMaterialfv;
	PFN_QGLMATRIXMODE m_pfn_qglMatrixMode;
	PFN_QGLMULTMATRIXF m_pfn_qglMultMatrixf;
	PFN_QGLNEWLIST m_pfn_qglNewList;
	PFN_QGLNORMAL3F m_pfn_qglNormal3f;
	PFN_QGLNORMAL3FV m_pfn_qglNormal3fv;
	PFN_QGLNORMALPOINTER m_pfn_qglNormalPointer;
	PFN_QGLORTHO m_pfn_qglOrtho;
	PFN_QGLPOINTSIZE m_pfn_qglPointSize;
	PFN_QGLPOLYGONMODE m_pfn_qglPolygonMode;
	PFN_QGLPOPATTRIB m_pfn_qglPopAttrib;
	PFN_QGLPOPMATRIX m_pfn_qglPopMatrix;
	PFN_QGLPUSHATTRIB m_pfn_qglPushAttrib;
	PFN_QGLPUSHMATRIX m_pfn_qglPushMatrix;
	PFN_QGLRASTERPOS3FV m_pfn_qglRasterPos3fv;
	PFN_QGLROTATED m_pfn_qglRotated;
	PFN_QGLROTATEF m_pfn_qglRotatef;
	PFN_QGLSCALEF m_pfn_qglScalef;
	PFN_QGLSCISSOR m_pfn_qglScissor;
	PFN_QGLSHADEMODEL m_pfn_qglShadeModel;
	PFN_QGLTEXCOORD2F m_pfn_qglTexCoord2f;
	PFN_QGLTEXCOORD2FV m_pfn_qglTexCoord2fv;
	PFN_QGLTEXCOORDPOINTER m_pfn_qglTexCoordPointer;
	PFN_QGLTEXENVF m_pfn_qglTexEnvf;
	PFN_QGLTEXGENF m_pfn_qglTexGenf;
	PFN_QGLTEXIMAGE1D m_pfn_qglTexImage1D;
	PFN_QGLTEXIMAGE2D m_pfn_qglTexImage2D;
	PFN_QGLTEXPARAMETERF m_pfn_qglTexParameterf;
	PFN_QGLTEXPARAMETERFV m_pfn_qglTexParameterfv;
	PFN_QGLTEXPARAMETERI m_pfn_qglTexParameteri;
	PFN_QGLTEXPARAMETERIV m_pfn_qglTexParameteriv;
	PFN_QGLTEXSUBIMAGE1D m_pfn_qglTexSubImage1D;
	PFN_QGLTEXSUBIMAGE2D m_pfn_qglTexSubImage2D;
	PFN_QGLTRANSLATED m_pfn_qglTranslated;
	PFN_QGLTRANSLATEF m_pfn_qglTranslatef;
	PFN_QGLVERTEX2F m_pfn_qglVertex2f;
	PFN_QGLVERTEX3F m_pfn_qglVertex3f;
	PFN_QGLVERTEX3FV m_pfn_qglVertex3fv;
	PFN_QGLVERTEXPOINTER m_pfn_qglVertexPointer;
	PFN_QGLVIEWPORT m_pfn_qglViewport;

	PFN_QE_CHECKOPENGLFORERRORS m_pfn_QE_CheckOpenGLForErrors;

	// glu stuff
	PFN_QGLUPERSPECTIVE m_pfn_qgluPerspective;
	PFN_QGLULOOKAT m_pfn_qgluLookAt;

	// plugin entities drawing inside Radiant windows
	PFN_QERAPP_HOOKGL2DWINDOW m_pfnHookGL2DWindow;
	PFN_QERAPP_UNHOOKGL2DWINDOW m_pfnUnHookGL2DWindow;
	PFN_QERAPP_HOOKGL3DWINDOW m_pfnHookGL3DWindow;
	PFN_QERAPP_UNHOOKGL3DWINDOW m_pfnUnHookGL3DWindow;
};

#endif
