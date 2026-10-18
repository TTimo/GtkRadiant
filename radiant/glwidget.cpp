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

// OpenGL widget for GTK3
//
// Radiant draws with the fixed-function pipeline and expects every GL view to share textures and
// display lists. GtkGLArea can't provide that (it only creates core profile contexts on macOS, and
// its contexts are per toplevel), so each widget gets its own offscreen compatibility context, all
// sharing objects with one root context, like the gtkglext share list did. A widget renders into
// a framebuffer object, and gtk_glwidget_swap_buffers() reads the frame back into a cairo surface
// that is painted in the widget's "draw" signal. On high-dpi displays the framebuffer has the
// display's resolution; see gtk_glwidget_scale_gl_functions().

#include "stdafx.h"

#include <pango/pangoft2.h>

#include "glwidget.h"
#include "qgl.h"

#if defined( __APPLE__ )
  #include <OpenGL/OpenGL.h>
  #include <OpenGL/glext.h>
#elif defined( __linux__ ) || defined( __BSD__ )
  #include <EGL/egl.h>
  #include <GL/glext.h>
#else
  #error "no offscreen OpenGL context implementation for this platform"
#endif

typedef void ( APIENTRY * PFN_GENFRAMEBUFFERS )( GLsizei n, GLuint *ids );
typedef void ( APIENTRY * PFN_DELETEFRAMEBUFFERS )( GLsizei n, const GLuint *ids );
typedef void ( APIENTRY * PFN_BINDFRAMEBUFFER )( GLenum target, GLuint id );
typedef void ( APIENTRY * PFN_FRAMEBUFFERRENDERBUFFER )( GLenum target, GLenum attachment, GLenum rbtarget, GLuint rb );
typedef GLenum ( APIENTRY * PFN_CHECKFRAMEBUFFERSTATUS )( GLenum target );
typedef void ( APIENTRY * PFN_RENDERBUFFERSTORAGE )( GLenum target, GLenum format, GLsizei width, GLsizei height );

static PFN_GENFRAMEBUFFERS glw_GenFramebuffers;
static PFN_DELETEFRAMEBUFFERS glw_DeleteFramebuffers;
static PFN_BINDFRAMEBUFFER glw_BindFramebuffer;
static PFN_FRAMEBUFFERRENDERBUFFER glw_FramebufferRenderbuffer;
static PFN_CHECKFRAMEBUFFERSTATUS glw_CheckFramebufferStatus;
static PFN_GENFRAMEBUFFERS glw_GenRenderbuffers;
static PFN_DELETEFRAMEBUFFERS glw_DeleteRenderbuffers;
static PFN_BINDFRAMEBUFFER glw_BindRenderbuffer;
static PFN_RENDERBUFFERSTORAGE glw_RenderbufferStorage;

// =============================================================================
// offscreen contexts

#if defined( __APPLE__ )

static CGLPixelFormatObj g_pixel_format = NULL;
static CGLContextObj g_root_context = NULL;

static bool glcontext_init(){
	// the legacy profile is the only one with the fixed-function pipeline
	CGLPixelFormatAttribute attribs[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
		kCGLPFAAccelerated,
		kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
		kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
		kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
		(CGLPixelFormatAttribute)0
	};
	GLint count = 0;
	if ( CGLChoosePixelFormat( attribs, &g_pixel_format, &count ) != kCGLNoError || g_pixel_format == NULL ) {
		Sys_FPrintf( SYS_ERR, "CGLChoosePixelFormat failed\n" );
		return false;
	}
	CGLError err = CGLCreateContext( g_pixel_format, NULL, &g_root_context );
	if ( err != kCGLNoError ) {
		Sys_FPrintf( SYS_ERR, "CGLCreateContext failed: %s\n", CGLErrorString( err ) );
		return false;
	}

	glw_GenFramebuffers = glGenFramebuffersEXT;
	glw_DeleteFramebuffers = glDeleteFramebuffersEXT;
	glw_BindFramebuffer = glBindFramebufferEXT;
	glw_FramebufferRenderbuffer = glFramebufferRenderbufferEXT;
	glw_CheckFramebufferStatus = glCheckFramebufferStatusEXT;
	glw_GenRenderbuffers = glGenRenderbuffersEXT;
	glw_DeleteRenderbuffers = glDeleteRenderbuffersEXT;
	glw_BindRenderbuffer = glBindRenderbufferEXT;
	glw_RenderbufferStorage = glRenderbufferStorageEXT;
	return true;
}

static void* glcontext_new(){
	CGLContextObj context = NULL;
	if ( CGLCreateContext( g_pixel_format, g_root_context, &context ) != kCGLNoError ) {
		return NULL;
	}
	return context;
}

static void glcontext_delete( void* context ){
	if ( CGLGetCurrentContext() == context ) {
		CGLSetCurrentContext( NULL );
	}
	CGLDestroyContext( static_cast<CGLContextObj>( context ) );
}

static bool glcontext_make_current( void* context ){
	return CGLSetCurrentContext( static_cast<CGLContextObj>( context ) ) == kCGLNoError;
}

#else

static EGLDisplay g_egl_display = EGL_NO_DISPLAY;
static EGLConfig g_egl_config;
static EGLSurface g_egl_surface = EGL_NO_SURFACE;
static EGLContext g_root_context = EGL_NO_CONTEXT;

static bool glcontext_init(){
	g_egl_display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( g_egl_display == EGL_NO_DISPLAY || !eglInitialize( g_egl_display, NULL, NULL ) ) {
		Sys_FPrintf( SYS_ERR, "eglInitialize failed\n" );
		return false;
	}
	if ( !eglBindAPI( EGL_OPENGL_API ) ) {
		Sys_FPrintf( SYS_ERR, "eglBindAPI( EGL_OPENGL_API ) failed\n" );
		return false;
	}
	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};
	EGLint count = 0;
	if ( !eglChooseConfig( g_egl_display, config_attribs, &g_egl_config, 1, &count ) || count == 0 ) {
		Sys_FPrintf( SYS_ERR, "eglChooseConfig failed\n" );
		return false;
	}
	// the framebuffer objects are the real render targets, the pbuffer only makes contexts current
	const EGLint pbuffer_attribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
	g_egl_surface = eglCreatePbufferSurface( g_egl_display, g_egl_config, pbuffer_attribs );
	// no version requested: a compatibility profile context
	g_root_context = eglCreateContext( g_egl_display, g_egl_config, EGL_NO_CONTEXT, NULL );
	if ( g_egl_surface == EGL_NO_SURFACE || g_root_context == EGL_NO_CONTEXT ) {
		Sys_FPrintf( SYS_ERR, "eglCreateContext failed\n" );
		return false;
	}

	glw_GenFramebuffers = (PFN_GENFRAMEBUFFERS)eglGetProcAddress( "glGenFramebuffersEXT" );
	glw_DeleteFramebuffers = (PFN_DELETEFRAMEBUFFERS)eglGetProcAddress( "glDeleteFramebuffersEXT" );
	glw_BindFramebuffer = (PFN_BINDFRAMEBUFFER)eglGetProcAddress( "glBindFramebufferEXT" );
	glw_FramebufferRenderbuffer = (PFN_FRAMEBUFFERRENDERBUFFER)eglGetProcAddress( "glFramebufferRenderbufferEXT" );
	glw_CheckFramebufferStatus = (PFN_CHECKFRAMEBUFFERSTATUS)eglGetProcAddress( "glCheckFramebufferStatusEXT" );
	glw_GenRenderbuffers = (PFN_GENFRAMEBUFFERS)eglGetProcAddress( "glGenRenderbuffersEXT" );
	glw_DeleteRenderbuffers = (PFN_DELETEFRAMEBUFFERS)eglGetProcAddress( "glDeleteRenderbuffersEXT" );
	glw_BindRenderbuffer = (PFN_BINDFRAMEBUFFER)eglGetProcAddress( "glBindRenderbufferEXT" );
	glw_RenderbufferStorage = (PFN_RENDERBUFFERSTORAGE)eglGetProcAddress( "glRenderbufferStorageEXT" );
	if ( glw_GenFramebuffers == NULL || glw_RenderbufferStorage == NULL ) {
		Sys_FPrintf( SYS_ERR, "GL_EXT_framebuffer_object is not available\n" );
		return false;
	}
	return true;
}

static void* glcontext_new(){
	EGLContext context = eglCreateContext( g_egl_display, g_egl_config, g_root_context, NULL );
	return context != EGL_NO_CONTEXT ? context : NULL;
}

static void glcontext_delete( void* context ){
	if ( eglGetCurrentContext() == context ) {
		eglMakeCurrent( g_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	}
	eglDestroyContext( g_egl_display, static_cast<EGLContext>( context ) );
}

static bool glcontext_make_current( void* context ){
	return eglMakeCurrent( g_egl_display, g_egl_surface, g_egl_surface, static_cast<EGLContext>( context ) );
}

#endif

static bool glcontext_ready(){
	static int state = 0; // 0 not tried, 1 ready, -1 failed
	if ( state == 0 ) {
		state = glcontext_init() ? 1 : -1;
		if ( state < 0 ) {
			Sys_FPrintf( SYS_ERR, "Failed to create the OpenGL context\n" );
		}
	}
	return state > 0;
}

// =============================================================================
// high-dpi views
//
// Radiant sets viewports, line widths and point sizes in window coordinates, so those calls are
// scaled by the scale factor of the view whose context is current.

#define GLWIDGET_MAX_SCALE 4

static int g_current_scale = 1;

static void ( APIENTRY * real_glViewport )( GLint x, GLint y, GLsizei width, GLsizei height );
static void ( APIENTRY * real_glScissor )( GLint x, GLint y, GLsizei width, GLsizei height );
static void ( APIENTRY * real_glLineWidth )( GLfloat width );
static void ( APIENTRY * real_glPointSize )( GLfloat size );
static void ( APIENTRY * real_glLineStipple )( GLint factor, GLushort pattern );
static void ( APIENTRY * real_glGetIntegerv )( GLenum pname, GLint *params );

static void APIENTRY scaled_glViewport( GLint x, GLint y, GLsizei width, GLsizei height ){
	real_glViewport( x * g_current_scale, y * g_current_scale, width * g_current_scale, height * g_current_scale );
}

static void APIENTRY scaled_glScissor( GLint x, GLint y, GLsizei width, GLsizei height ){
	real_glScissor( x * g_current_scale, y * g_current_scale, width * g_current_scale, height * g_current_scale );
}

static void APIENTRY scaled_glLineWidth( GLfloat width ){
	real_glLineWidth( width * g_current_scale );
}

static void APIENTRY scaled_glPointSize( GLfloat size ){
	real_glPointSize( size * g_current_scale );
}

static void APIENTRY scaled_glLineStipple( GLint factor, GLushort pattern ){
	real_glLineStipple( factor * g_current_scale, pattern );
}

static void APIENTRY scaled_glGetIntegerv( GLenum pname, GLint *params ){
	real_glGetIntegerv( pname, params );
	if ( pname == GL_VIEWPORT || pname == GL_SCISSOR_BOX ) {
		for ( int i = 0; i < 4; i++ )
			params[i] /= g_current_scale;
	}
}

// called once after QGL_Init, before the plugins copy the qgl function pointers
void gtk_glwidget_scale_gl_functions(){
	real_glViewport = qglViewport;
	real_glScissor = qglScissor;
	real_glLineWidth = qglLineWidth;
	real_glPointSize = qglPointSize;
	real_glLineStipple = qglLineStipple;
	real_glGetIntegerv = qglGetIntegerv;
	qglViewport = scaled_glViewport;
	qglScissor = scaled_glScissor;
	qglLineWidth = scaled_glLineWidth;
	qglPointSize = scaled_glPointSize;
	qglLineStipple = scaled_glLineStipple;
	qglGetIntegerv = scaled_glGetIntegerv;
}

// =============================================================================
// per-widget context and framebuffer

typedef struct
{
	void *context;           // GL state is per view, like it was with one gtkglext context per widget
	GLuint fbo;
	GLuint color_rb;
	GLuint depth_rb;
	int width, height;       // in window coordinates
	int scale;               // device pixels per window coordinate
	cairo_surface_t *frame;  // last frame read back from the fbo, bottom-up
	int xor_x, xor_y, xor_w, xor_h;  // rubber band, in GL window coordinates
	cairo_t *cr;             // valid while the widget's "draw" signal is being emitted
	bool frame_pending;      // frame swapped outside of a draw, not painted yet
} glwidget_t;

static const char *glwidget_key = "radiant-glwidget";

static glwidget_t *glwidget_get( GtkWidget *widget ){
	return static_cast<glwidget_t*>( g_object_get_data( G_OBJECT( widget ), glwidget_key ) );
}

static void glwidget_destroy( gpointer data ){
	glwidget_t *gl = static_cast<glwidget_t*>( data );
	if ( gl->context != NULL ) {
		if ( gl->fbo != 0 && glcontext_make_current( gl->context ) ) {
			glw_BindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
			glw_DeleteFramebuffers( 1, &gl->fbo );
			glw_DeleteRenderbuffers( 1, &gl->color_rb );
			glw_DeleteRenderbuffers( 1, &gl->depth_rb );
		}
		glcontext_delete( gl->context );
	}
	if ( gl->frame != NULL ) {
		cairo_surface_destroy( gl->frame );
	}
	g_free( gl );
}

static void glwidget_paint( glwidget_t *gl, cairo_t *cr ){
	if ( gl->frame == NULL ) {
		return;
	}
	// GL rows are bottom-up
	cairo_save( cr );
	cairo_translate( cr, 0, gl->height );
	cairo_scale( cr, 1, -1 );
	cairo_set_source_surface( cr, gl->frame, 0, 0 );
	cairo_set_operator( cr, CAIRO_OPERATOR_SOURCE );
	cairo_paint( cr );
	if ( gl->xor_w != 0 && gl->xor_h != 0 ) {
		// inverting with white stands in for the old GDK_XOR rubber band
		cairo_set_operator( cr, CAIRO_OPERATOR_DIFFERENCE );
		cairo_set_source_rgb( cr, 1, 1, 1 );
		cairo_rectangle( cr, gl->xor_x, gl->xor_y, gl->xor_w, gl->xor_h );
		cairo_fill( cr );
	}
	cairo_restore( cr );
	gl->frame_pending = false;
}

// connected before any other "draw" handler so that swap_buffers can paint into the cairo context
static void glwidget_scale_changed( GObject *object, GParamSpec *pspec, gpointer data ){
	gtk_widget_queue_draw( GTK_WIDGET( object ) );
}

static gboolean glwidget_draw_begin( GtkWidget *widget, cairo_t *cr, gpointer data ){
	glwidget_t *gl = glwidget_get( widget );
	gl->cr = cr;
	if ( gl->frame_pending ) {
		glwidget_paint( gl, cr );
	}
	return FALSE;
}

static bool glwidget_in_draw( GtkWidget *widget ){
	static guint draw_signal = g_signal_lookup( "draw", GTK_TYPE_WIDGET );
	GSignalInvocationHint *hint = g_signal_get_invocation_hint( widget );
	return hint != NULL && hint->signal_id == draw_signal;
}

GtkWidget* WINAPI gtk_glwidget_new( gboolean zbuffer, GtkWidget* share ){
	GtkWidget* drawing_area = gtk_drawing_area_new();
	glwidget_t *gl = g_new0( glwidget_t, 1 );
	g_object_set_data_full( G_OBJECT( drawing_area ), glwidget_key, gl, glwidget_destroy );
	g_signal_connect( G_OBJECT( drawing_area ), "draw", G_CALLBACK( glwidget_draw_begin ), NULL );
	g_signal_connect( G_OBJECT( drawing_area ), "notify::scale-factor", G_CALLBACK( glwidget_scale_changed ), NULL );
	// every context shares objects with the root context, and every fbo has a depth buffer
	return drawing_area;
}

void WINAPI gtk_glwidget_destroy_context( GtkWidget *widget ){
}

void WINAPI gtk_glwidget_create_context( GtkWidget *widget ){
}

void gtk_glwidget_set_xor_rectangle( GtkWidget *widget, int x, int y, int w, int h ){
	glwidget_t *gl = glwidget_get( widget );
	if ( gl == NULL || ( gl->xor_x == x && gl->xor_y == y && gl->xor_w == w && gl->xor_h == h ) ) {
		return;
	}
	gl->xor_x = x;
	gl->xor_y = y;
	gl->xor_w = w;
	gl->xor_h = h;
	gtk_widget_queue_draw( widget );
}

void WINAPI gtk_glwidget_swap_buffers( GtkWidget *widget ){
	glwidget_t *gl = glwidget_get( widget );
	if ( gl == NULL || gl->fbo == 0 || !glcontext_make_current( gl->context ) ) {
		return;
	}
	g_current_scale = gl->scale;

	const int pixel_width = gl->width * gl->scale;
	const int pixel_height = gl->height * gl->scale;
	if ( gl->frame == NULL
		 || cairo_image_surface_get_width( gl->frame ) != pixel_width
		 || cairo_image_surface_get_height( gl->frame ) != pixel_height ) {
		if ( gl->frame != NULL ) {
			cairo_surface_destroy( gl->frame );
		}
		gl->frame = cairo_image_surface_create( CAIRO_FORMAT_RGB24, pixel_width, pixel_height );
		cairo_surface_set_device_scale( gl->frame, gl->scale, gl->scale );
	}

	// BGRA with 8_8_8_8_REV is cairo's native-endian 0xAARRGGBB layout
	cairo_surface_flush( gl->frame );
	glw_BindFramebuffer( GL_FRAMEBUFFER_EXT, gl->fbo );
	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	glPixelStorei( GL_PACK_ROW_LENGTH, cairo_image_surface_get_stride( gl->frame ) / 4 );
	glReadPixels( 0, 0, pixel_width, pixel_height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
				  cairo_image_surface_get_data( gl->frame ) );
	glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
	cairo_surface_mark_dirty( gl->frame );

	if ( glwidget_in_draw( widget ) && gl->cr != NULL ) {
		glwidget_paint( gl, gl->cr );
	}
	else
	{
		gl->frame_pending = true;
		gtk_widget_queue_draw( widget );
	}
}

gboolean WINAPI gtk_glwidget_make_current( GtkWidget *widget ){
	glwidget_t *gl = glwidget_get( widget );
	if ( gl == NULL || !glcontext_ready() ) {
		return FALSE;
	}
	if ( gl->context == NULL ) {
		gl->context = glcontext_new();
		if ( gl->context == NULL ) {
			Sys_FPrintf( SYS_ERR, "Failed to create an OpenGL context for a view\n" );
			return FALSE;
		}
	}
	if ( !glcontext_make_current( gl->context ) ) {
		return FALSE;
	}

	GtkAllocation allocation;
	gtk_widget_get_allocation( widget, &allocation );
	int width = MAX( allocation.width, 1 );
	int height = MAX( allocation.height, 1 );
	int scale = CLAMP( gtk_widget_get_scale_factor( widget ), 1, GLWIDGET_MAX_SCALE );
	g_current_scale = scale;

	if ( gl->fbo == 0 ) {
		glw_GenFramebuffers( 1, &gl->fbo );
		glw_GenRenderbuffers( 1, &gl->color_rb );
		glw_GenRenderbuffers( 1, &gl->depth_rb );
	}
	glw_BindFramebuffer( GL_FRAMEBUFFER_EXT, gl->fbo );
	if ( gl->width != width || gl->height != height || gl->scale != scale ) {
		gl->width = width;
		gl->height = height;
		gl->scale = scale;
		glw_BindRenderbuffer( GL_RENDERBUFFER_EXT, gl->color_rb );
		glw_RenderbufferStorage( GL_RENDERBUFFER_EXT, GL_RGBA8, width * scale, height * scale );
		glw_BindRenderbuffer( GL_RENDERBUFFER_EXT, gl->depth_rb );
		glw_RenderbufferStorage( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width * scale, height * scale );
		glw_BindRenderbuffer( GL_RENDERBUFFER_EXT, 0 );
		glw_FramebufferRenderbuffer( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, gl->color_rb );
		glw_FramebufferRenderbuffer( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gl->depth_rb );
		glw_FramebufferRenderbuffer( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gl->depth_rb );
		GLenum status = glw_CheckFramebufferStatus( GL_FRAMEBUFFER_EXT );
		if ( status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
			Sys_FPrintf( SYS_ERR, "GL framebuffer incomplete: 0x%x\n", status );
			return FALSE;
		}
	}
	return TRUE;
}

// Think about rewriting this font stuff to use OpenGL display lists and glBitmap().
// Bit maps together with display lists may offer a performance increase, but
// they would not allow antialiased fonts.
static const char font_string[] = "Monospace bold";
static const int font_height = 10;

typedef struct
{
	PangoFontMap *font_map;
	PangoContext *ft2_context;
	int ascent;  // pixels at this scale
	int descent;
	int y_offset_bitmap_render_pango_units;
} glwidget_font_t;

// one font per view scale factor, so that text stays sharp on high-dpi views;
// fonts[1] also gives the metrics in window coordinates
static glwidget_font_t fonts[GLWIDGET_MAX_SCALE + 1];
static int _debug_font_created = 0;


// Units are pixels.  Returns a positive value [most likely].
int gtk_glwidget_font_ascent(){
	if ( !_debug_font_created ) {
		Error( "Programming error: gtk_glwidget_font_ascent() called but font does not exist; "
			   "you should have called gtk_glwidget_create_font() first" );
	}

	return fonts[1].ascent;
}

// Units are pixels.  Returns a positive value [most likely].
int gtk_glwidget_font_descent(){
	if ( !_debug_font_created ) {
		Error( "Programming error: gtk_glwidget_font_descent() called but font does not exist; "
			   "you should have called gtk_glwidget_create_font() first" );
	}

	return fonts[1].descent;
}


static void glwidget_font_create( glwidget_font_t *font, int scale ){
	PangoFontDescription *font_desc;
	PangoLayout *layout;
	PangoRectangle log_rect;
	int font_ascent_pango_units;
	int font_descent_pango_units;

	font->font_map = pango_ft2_font_map_new();
	pango_ft2_font_map_set_resolution( PANGO_FT2_FONT_MAP( font->font_map ), 72 * scale, 72 * scale );
	font->ft2_context = pango_font_map_create_context( PANGO_FONT_MAP( font->font_map ));

	font_desc = pango_font_description_from_string( font_string );
	pango_font_description_set_size( font_desc, font_height * PANGO_SCALE );
	pango_context_set_font_description( font->ft2_context, font_desc );
	pango_font_description_free( font_desc );

	layout = pango_layout_new( font->ft2_context );

	// I don't believe that's standard preprocessor syntax?
#if !PANGO_VERSION_CHECK( 1,22,0 )
	PangoLayoutIter *iter;
	iter = pango_layout_get_iter( layout );
	font_ascent_pango_units = pango_layout_iter_get_baseline( iter );
	pango_layout_iter_free( iter );
#else
	font_ascent_pango_units = pango_layout_get_baseline( layout );
#endif

	pango_layout_get_extents( layout, NULL, &log_rect );
	g_object_unref( G_OBJECT( layout ) );
	font_descent_pango_units = log_rect.height - font_ascent_pango_units;

	font->ascent = PANGO_PIXELS_CEIL( font_ascent_pango_units );
	font->descent = PANGO_PIXELS_CEIL( font_descent_pango_units );
	font->y_offset_bitmap_render_pango_units = ( font->ascent * PANGO_SCALE ) - font_ascent_pango_units;
}

void gtk_glwidget_create_font(){
	if ( _debug_font_created ) {
		Error( "Programming error: gtk_glwidget_create_font() was already called; "
			   "you must call gtk_glwidget_destroy_font() before creating font again" );
	}
	_debug_font_created = 1;

	glwidget_font_create( &fonts[1], 1 );
}

void gtk_glwidget_destroy_font(){
	if ( !_debug_font_created ) {
		Error( "Programming error: gtk_glwidget_destroy_font() called when font "
			   "does not exist" );
	}

	for ( int i = 0; i <= GLWIDGET_MAX_SCALE; i++ )
	{
		if ( fonts[i].ft2_context != NULL ) {
			g_object_unref( G_OBJECT( fonts[i].ft2_context ) );
			g_object_unref( G_OBJECT( fonts[i].font_map ) );
		}
		memset( &fonts[i], 0, sizeof( fonts[i] ) );
	}
	_debug_font_created = 0;
}


// Renders the input text at the current location with the current color.
// The X position of the current location is used to place the left edge of the text image,
// where the text image bounds are defined as the logical extents of the line of text.
// The Y position of the current location is used to place the bottom of the text image.
// You should offset the Y position by the amount returned by gtk_glwidget_font_descent()
// if you want to place the baseline of the text image at the current Y position.
// Note: A problem with this function is that if the lower left corner of the text falls
// just a hair outside of the viewport (meaning the current raster position is invalid),
// then no text will be rendered.  The solution to this is a very hacky one.  You can search
// Google for "glDrawPixels clipping".
void gtk_glwidget_print_string( const char *s ){
	// The idea for this code initially came from the font-pangoft2.c example that comes with GtkGLExt.

	PangoLayout *layout;
	PangoRectangle log_rect;
	FT_Bitmap bitmap;
	unsigned char *begin_bitmap_buffer;
	GLfloat color[4];
	GLint previous_unpack_alignment;
	GLboolean previous_blend_enabled;
	GLint previous_blend_func_src;
	GLint previous_blend_func_dst;
	GLfloat previous_red_bias;
	GLfloat previous_green_bias;
	GLfloat previous_blue_bias;
	GLfloat previous_alpha_scale;

	if ( !_debug_font_created ) {
		Error( "Programming error: gtk_glwidget_print_string() called but font does not exist; "
			   "you should have called gtk_glwidget_create_font() first" );
	}

	// glDrawPixels writes device pixels, so render the text at the view's scale
	glwidget_font_t *font = &fonts[g_current_scale];
	if ( font->ft2_context == NULL ) {
		glwidget_font_create( font, g_current_scale );
	}

	layout = pango_layout_new( font->ft2_context );
	pango_layout_set_width( layout, -1 ); // -1 no wrapping.  All text on one line.
	pango_layout_set_text( layout, s, -1 ); // -1 null-terminated string.
	pango_layout_get_extents( layout, NULL, &log_rect );

	if ( log_rect.width > 0 && log_rect.height > 0 ) {
		bitmap.rows = font->ascent + font->descent;
		bitmap.width = PANGO_PIXELS_CEIL( log_rect.width );
		bitmap.pitch = -bitmap.width; // Rendering it "upside down" for OpenGL.
		begin_bitmap_buffer = (unsigned char *) g_malloc( bitmap.rows * bitmap.width );
		memset( begin_bitmap_buffer, 0, bitmap.rows * bitmap.width );
		bitmap.buffer = begin_bitmap_buffer + ( bitmap.rows - 1 ) * bitmap.width; // See pitch above.
		bitmap.num_grays = 0xff;
		bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
		pango_ft2_render_layout_subpixel( &bitmap, layout, -log_rect.x,
										  font->y_offset_bitmap_render_pango_units );
		qglGetFloatv( GL_CURRENT_COLOR, color );

		// Save state.  I didn't see any OpenGL push/pop operations for these.
		// Question: Is saving/restoring this state necessary?  Being safe.
		qglGetIntegerv( GL_UNPACK_ALIGNMENT, &previous_unpack_alignment );
		previous_blend_enabled = qglIsEnabled( GL_BLEND );
		qglGetIntegerv( GL_BLEND_SRC, &previous_blend_func_src );
		qglGetIntegerv( GL_BLEND_DST, &previous_blend_func_dst );
		qglGetFloatv( GL_RED_BIAS, &previous_red_bias );
		qglGetFloatv( GL_GREEN_BIAS, &previous_green_bias );
		qglGetFloatv( GL_BLUE_BIAS, &previous_blue_bias );
		qglGetFloatv( GL_ALPHA_SCALE, &previous_alpha_scale );

		qglPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		qglEnable( GL_BLEND );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglPixelTransferf( GL_RED_BIAS, color[0] );
		qglPixelTransferf( GL_GREEN_BIAS, color[1] );
		qglPixelTransferf( GL_BLUE_BIAS, color[2] );
		qglPixelTransferf( GL_ALPHA_SCALE, color[3] );

		qglDrawPixels( bitmap.width, bitmap.rows,
					   GL_ALPHA, GL_UNSIGNED_BYTE, begin_bitmap_buffer );
		g_free( begin_bitmap_buffer );

		// Restore state in reverse order of how we set it.
		qglPixelTransferf( GL_ALPHA_SCALE, previous_alpha_scale );
		qglPixelTransferf( GL_BLUE_BIAS, previous_blue_bias );
		qglPixelTransferf( GL_GREEN_BIAS, previous_green_bias );
		qglPixelTransferf( GL_RED_BIAS, previous_red_bias );
		qglBlendFunc( previous_blend_func_src, previous_blend_func_dst );
		if ( !previous_blend_enabled ) {
			qglDisable( GL_BLEND );
		}
		qglPixelStorei( GL_UNPACK_ALIGNMENT, previous_unpack_alignment );
	}

	g_object_unref( G_OBJECT( layout ) );
}

void gtk_glwidget_print_char( char s ){
	char str[2];
	str[0] = s;
	str[1] = '\0';
	gtk_glwidget_print_string( str );
}
