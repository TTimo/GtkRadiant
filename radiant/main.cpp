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

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <gdk/gdkx.h>
  #include <pwd.h>
  #include <unistd.h>
  #ifdef __linux__
	#include <mntent.h>
  #endif
  #include <dirent.h>
  #include <pthread.h>
  #include <sys/wait.h>
  #include <signal.h>
  #include <sys/stat.h>
  #include <fcntl.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <glib/gi18n.h>
#include "stdafx.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>

#include "watchbsp.h"
#include "filters.h"
#include "glwidget.h"

bool g_bBuildList = false;
int g_argc;
char** g_argv;

// =============================================================================
// Windows WinMain() wrapper for main()
// used in Release Builds to suppress the Console
#if defined(_WIN32)

#include <winbase.h>

int main(int argc, char* argv[]);

int CALLBACK WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
){
	return main(__argc, __argv);
}

#endif

//============================================================================
// Splash Screen
//============================================================================

// get rid of it when debugging
#if defined ( _DEBUG )
  #define SKIP_SPLASH
#endif

static GtkWidget *splash_screen;

// called based on a timer, or in particular cases when we don't want to 
// keep it around
gint try_destroy_splash( gpointer data ){
	if ( splash_screen ) {
		gtk_widget_destroy( splash_screen );
		splash_screen = NULL;
	}
	return FALSE;
}

#ifndef SKIP_SPLASH
static void create_splash() {
    splash_screen = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(splash_screen), _("Splash Screen"));
    gtk_container_set_border_width(GTK_CONTAINER(splash_screen), 0);
    gtk_widget_set_size_request(splash_screen, 640, 384);
    gtk_window_set_keep_above(GTK_WINDOW(splash_screen), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(splash_screen), FALSE);
    gtk_window_set_position(GTK_WINDOW(splash_screen), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(splash_screen), FALSE);

    CString str = g_strBitmapsPath;
    str += "splash.png";
    GtkWidget *image = gtk_image_new_from_file(str.GetBuffer());
    gtk_container_add(GTK_CONTAINER(splash_screen), image);
    gtk_widget_show_all(splash_screen);

	while(gtk_events_pending())
		gtk_main_iteration();
}
#endif

// =============================================================================
// Loki stuff

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

/* A short game name, could be used as argv[0] */
static char game_name[100] = "";

/* The directory where the data files can be found (run directory) */
static char datapath[PATH_MAX];

char *loki_gethomedir( void ){
	char *home = NULL;

	home = getenv( "HOME" );
	if ( home == NULL ) {
		uid_t id = getuid();
		struct passwd *pwd;

		setpwent();
		while ( ( pwd = getpwent() ) != NULL )
		{
			if ( pwd->pw_uid == id ) {
				home = pwd->pw_dir;
				break;
			}
		}
		endpwent();
	}
	return home;
}

/* Must be called BEFORE loki_initialize */
void loki_setgamename( const char *n ){
	strncpy( game_name, n, sizeof( game_name ) );
}

  #ifdef __linux__
/* Code to determine the mount point of a CD-ROM */
int loki_getmountpoint( const char *device, char *mntpt, int max_size ){
	char devpath[PATH_MAX], mntdevpath[PATH_MAX];
	FILE * mountfp;
	struct mntent *mntent;
	int mounted;

	/* Nothing to do with no device file */
	if ( device == NULL ) {
		*mntpt = '\0';
		return -1;
	}

	/* Get the fully qualified path of the CD-ROM device */
	if ( realpath( device, devpath ) == NULL ) {
		perror( "realpath() on your CD-ROM failed" );
		return( -1 );
	}

	/* Get the mount point */
	mounted = -1;
	memset( mntpt, 0, max_size );
	mountfp = setmntent( _PATH_MNTTAB, "r" );
	if ( mountfp != NULL ) {
		mounted = 0;
		while ( ( mntent = getmntent( mountfp ) ) != NULL )
		{
			char *tmp, mntdev[1024];

			strcpy( mntdev, mntent->mnt_fsname );
			if ( strcmp( mntent->mnt_type, "supermount" ) == 0 ) {
				tmp = strstr( mntent->mnt_opts, "dev=" );
				if ( tmp ) {
					strcpy( mntdev, tmp + strlen( "dev=" ) );
					tmp = strchr( mntdev, ',' );
					if ( tmp ) {
						*tmp = '\0';
					}
				}
			}
			if ( strncmp( mntdev, "/dev", 4 ) ||
				 realpath( mntdev, mntdevpath ) == NULL ) {
				continue;
			}
			if ( strcmp( mntdevpath, devpath ) == 0 ) {
				mounted = 1;
				assert( (int)strlen( mntent->mnt_dir ) < max_size );
				strncpy( mntpt, mntent->mnt_dir, max_size - 1 );
				mntpt[max_size - 1] = '\0';
				break;
			}
		}
		endmntent( mountfp );
	}
	return( mounted );
}
  #endif

/*
    This function gets the directory containing the running program.
    argv0 - the 0'th argument to the program
 */
// FIXME TTimo
// I don't understand this function. It looks like something cut from another piece of software
// we somehow get the g_strAppPath from it, but it's done through a weird scan across $PATH env. var.
// even worse, it doesn't behave the same in all cases .. works well when ran through gdb and borks when ran from a shell
void loki_initpaths( char *argv0 ){
	char temppath[PATH_MAX]; //, env[100];
	char *home; //, *ptr, *data_env;

	home = loki_gethomedir();
	if ( home == NULL ) {
		home = const_cast<char*>(".");
	}

	if ( *game_name == 0 ) { /* Game name defaults to argv[0] */
		loki_setgamename( argv0 );
	}

	strcpy( temppath, argv0 ); /* If this overflows, it's your own fault :) */
	if ( !strrchr( temppath, '/' ) ) {
		char *path;
		char *last;
		int found;

		found = 0;
		path = getenv( "PATH" );
		do
		{
			/* Initialize our filename variable */
			temppath[0] = '\0';

			/* Get next entry from path variable */
			last = strchr( path, ':' );
			if ( !last ) {
				last = path + strlen( path );
			}

			/* Perform tilde expansion */
			if ( *path == '~' ) {
				strcpy( temppath, home );
				++path;
			}

			/* Fill in the rest of the filename */
			if ( last > ( path + 1 ) ) {
				strncat( temppath, path, ( last - path ) );
				strcat( temppath, "/" );
			}
			strcat( temppath, "./" );
			strcat( temppath, argv0 );

			/* See if it exists, and update path */
			if ( access( temppath, X_OK ) == 0 ) {
				++found;
			}
			path = last + 1;

		} while ( *last && !found );

	}
	else
	{
		/* Increment argv0 to the basename */
		argv0 = strrchr( argv0, '/' ) + 1;
	}

	/* Now canonicalize it to a full pathname for the data path */
	if ( realpath( temppath, datapath ) ) {
		/* There should always be '/' in the path */
		*( strrchr( datapath, '/' ) ) = '\0';
	}
}

char *loki_getdatapath( void ){
	return( datapath );
}

#endif

// end of Loki stuff
// =============================================================================

void error_redirect( const gchar *domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data ){
	gboolean in_recursion;
	gboolean is_fatal;
	char buf[256];

	in_recursion = ( log_level & G_LOG_FLAG_RECURSION ) != 0;
	is_fatal = ( log_level & G_LOG_FLAG_FATAL ) != 0;
	log_level = (GLogLevelFlags) ( log_level & G_LOG_LEVEL_MASK );

	if ( !message ) {
		message = "(NULL) message";
	}

	if ( domain ) {
		strcpy( buf, domain );
	}
	else{
		strcpy( buf, "**" );
	}
	strcat( buf, "-" );

	switch ( log_level )
	{
	case G_LOG_LEVEL_ERROR:
		if ( in_recursion ) {
			strcat( buf, "ERROR (recursed) **: " );
		}
		else{
			strcat( buf, "ERROR **: " );
		}
		break;
	case G_LOG_LEVEL_CRITICAL:
		if ( in_recursion ) {
			strcat( buf, "CRITICAL (recursed) **: " );
		}
		else{
			strcat( buf, "CRITICAL **: " );
		}
		break;
	case G_LOG_LEVEL_WARNING:
		if ( in_recursion ) {
			strcat( buf, "WARNING (recursed) **: " );
		}
		else{
			strcat( buf, "WARNING **: " );
		}
		break;
	case G_LOG_LEVEL_MESSAGE:
		if ( in_recursion ) {
			strcat( buf, "Message (recursed): " );
		}
		else{
			strcat( buf, "Message: " );
		}
		break;
	case G_LOG_LEVEL_INFO:
		if ( in_recursion ) {
			strcat( buf, "INFO (recursed): " );
		}
		else{
			strcat( buf, "INFO: " );
		}
		break;
	case G_LOG_LEVEL_DEBUG:
		if ( in_recursion ) {
			strcat( buf, "DEBUG (recursed): " );
		}
		else{
			strcat( buf, "DEBUG: " );
		}
		break;
	default:
		/* we are used for a log level that is not defined by GLib itself,
		 * try to make the best out of it.
		 */
		if ( in_recursion ) {
			strcat( buf, "LOG (recursed:" );
		}
		else{
			strcat( buf, "LOG (" );
		}
		if ( log_level ) {
			gchar string[] = "0x00): ";
			gchar *p = string + 2;
			guint i;

			i = g_bit_nth_msf( log_level, -1 );
			*p = i >> 4;
			p++;
			*p = '0' + ( i & 0xf );
			if ( *p > '9' ) {
				*p += 'A' - '9' - 1;
			}

			strcat( buf, string );
		}
		else{
			strcat( buf, "): " );
		}
	}

	strcat( buf, message );
	if ( is_fatal ) {
		strcat( buf, "\naborting...\n" );
	}
	else{
		strcat( buf, "\n" );
	}

	printf( "%s\n", buf );
	Sys_FPrintf( SYS_WRN, buf );
	// TTimo NOTE: in some cases it may be handy to log only to the file
//  Sys_FPrintf (SYS_NOCON, buf);
}

#define GETTEXT_PACKAGE "radiant"
#define LOCALEDIR "lang"

int mainRadiant( int argc, char* argv[] ) {
	const char *libgl;
	int i, j, k;

#if defined( _WIN32 ) && defined( _MSC_VER )
	//increase the max open files to its maximum for the C run-time of MSVC
	_setmaxstdio( 2048 );
#endif
	/*
	   Rambetter on Sat Nov 13, 2010:

	   The following line fixes parsing and writing of floating point numbers in locales such as
	   Italy, Germany, and others outside of en_US.  In particular, in such problem locales, users
	   are not able to use certain map entities such as "light" because the definitions of these entities
	   in the entity definition files contain floating point values written in the standard "C" format
	   (containing a dot instead of, for example, a comma).  The call sscanf() is all over the code,
	   including parsing entity definition files and reading Radiant preferences.  sscanf() is sensitive
	   to locale (in particular when reading floating point numbers).

	   The line below is the minimalistic way to address only this particular problem - the parsing
	   and writing of floating point values.  There may be other yet-undiscovered bugs related to
	   locale still lingering in the code.  When such bugs are discovered, they should be addressed by
	   setting more than just "LC_NUMERIC=C" (for example LC_CTYPE for regular expression matching)
	   or by fixing the problem in the actual code instead of fiddling with LC_* variables.

	   Another way to fix the floating point format problem is to locate all calls such as *scanf()
	   and *printf() in the code and replace them with other functions.  However, we're also using
	   external libraries such as libxml and [maybe?] they use locale to parse their numeric values.
	   I'm just saying, it may get ugly if we try to fix the problem without setting LC_NUMERIC.

	   Usage of sscanf() throughout the code looks like so:
	    sscanf(str, "%f %f %f", &val1, &val2, &val3);
	   Code like this exists in many files, here are 4 examples:
	    tools/quake3/q3map2/light.c
	    tools/quake3/q3map2/model.c
	    radiant/preferences.cpp
	    plugins/entity/miscmodel.cpp

	   Also affected are printf() calls when using formats that contain "%f".

	   I did some research and putenv() seems to be the best choice for being cross-platform.  It
	   used to be a function in Windows (now deprecated):
	    http://msdn.microsoft.com/en-us/library/ms235321(VS.80).aspx
	   And of course it's defined in UNIX.

	   One more thing.  the gtk_init() call below modifies all of the locale settings.  In fact if it
	   weren't for gtk_init(), we wouldn't have to set LC_NUMERIC (parsing of floating points with
	   a dot works just fine before the gtk_init() call on a sample Linux system).  If we were to
	   just setlocale() here, it would get clobbered by gtk_init().  So instead of using setlocale()
	   _after_ gtk_init(), I chose to fix this problem via environment variable.  I think it's cleaner
	   that way.
	 */
	putenv( (char *)"LC_NUMERIC=C" );

	// Use the same environment variable for resolving libGL as libgtkglext does.
	libgl = getenv("GDK_GL_LIBGL_PATH");
	if ( libgl == NULL ) {
		#if defined ( _WIN32 )
			libgl = "opengl32.dll";
		#elif defined ( __linux__ ) || defined ( __FreeBSD__ )
			libgl = "libGL.so.1";
		#elif defined ( __APPLE__ )
			libgl = "/opt/local/lib/libGL.dylib";
		#else
			#error unknown architecture
		#endif
	}

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	// Give away unnecessary root privileges.
	// Important: must be done before calling gtk_init().
	char *loginname;
	struct passwd *pw;
	seteuid( getuid() );
	if ( geteuid() == 0 && ( loginname = getlogin() ) != NULL && ( pw = getpwnam( loginname ) ) != NULL ) {
		setuid( pw->pw_uid );
	}
#endif


	bindtextdomain( GETTEXT_PACKAGE, LOCALEDIR );
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
	textdomain( GETTEXT_PACKAGE );
//  gtk_disable_setlocale();

	gtk_init( &argc, &argv );
	gtk_gl_init( &argc, &argv );
	gdk_gl_init( &argc, &argv );

	// TODO: Find a better place to call this.
	gtk_glwidget_create_font();

	for ( i = 1; i < argc; i++ )
	{
		char* param = argv[i];

		if ( param[0] == '-' && param[1] == '-' ) {
			param += 2;

			if ( strcmp( param, "builddefs" ) == 0 ) {
				g_bBuildList = true;
				argv[i] = NULL;
			}
		}
	}

	for ( i = 1; i < argc; i++ )
	{
		for ( k = i; k < argc; k++ )
			if ( argv[k] != NULL ) {
				break;
			}

		if ( k > i ) {
			k -= i;
			for ( j = i + k; j < argc; j++ )
				argv[j - k] = argv[j];
			argc -= k;
		}
	}

	g_argc = argc;
	g_argv = argv;

	g_strPluginsDir = "plugins/";
	g_strModulesDir = "modules/";

#ifdef _WIN32
	// get path to the editor
	char* pBuffer = g_strAppPath.GetBufferSetLength( _MAX_PATH + 1 );
	GetModuleFileName( NULL, pBuffer, _MAX_PATH );
	pBuffer[g_strAppPath.ReverseFind( '\\' ) + 1] = '\0';
	QE_ConvertDOSToUnixName( pBuffer, pBuffer );
	g_strAppPath.ReleaseBuffer();

	g_strBitmapsPath = g_strAppPath;
	g_strBitmapsPath += "bitmaps/";

	CGameDialog::UpdateNetrun( false ); // read the netrun configuration

	if ( CGameDialog::GetNetrun() ) {
		// we have to find a per-user g_strTempPath
		// this behaves the same as on Linux
		g_strTempPath = getenv( "USERPROFILE" );
		if ( !g_strTempPath.GetLength() ) {
			CString msg;
			msg = _( "Radiant is configured to run from a network installation.\n" );
			msg += _( "I couldn't find the environement variable USERPROFILE\n" );
			msg += _( "I'm going to use C:\\RadiantSettings. Please set USERPROFILE\n" );
			gtk_MessageBox( NULL, msg, _( "Radiant - Network mode" ), MB_OK );
			g_strTempPath = "C:\\";
		}
		g_strTempPath += "\\RadiantSettings\\";
		Q_mkdir( g_strTempPath.GetBuffer(), 0755 );
		g_strTempPath += RADIANT_VERSION;
		g_strTempPath += "\\";
		Q_mkdir( g_strTempPath.GetBuffer(), 0755 );
	}
	else
	{
		// use the core path as temp (to save commandlist.txt, and do the .pid files)
		g_strTempPath = g_strAppPath;
	}

#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	Str home;
	home = g_get_home_dir();
	AddSlash( home );
	home += ".radiant/";
	Q_mkdir( home.GetBuffer(), 0775 );
	home += RADIANT_VERSION;
	Q_mkdir( home.GetBuffer(), 0775 );
	g_strTempPath = home.GetBuffer();
	AddSlash( g_strTempPath );

	loki_initpaths( argv[0] );

	// NOTE: we build g_strAppPath with a '/' (or '\' on WIN32)
	// it's a general convention in Radiant to have the slash at the end of directories
	char real[PATH_MAX];
	realpath( loki_getdatapath(), real );
	if ( real[strlen( real ) - 1] != '/' ) {
		strcat( real, "/" );
	}

	g_strAppPath = real;

	// radiant is installed in the parent dir of "tools/"
	// NOTE: this is not very easy for debugging
	// maybe add options to lookup in several places?
	// (for now I had to create symlinks)
	g_strBitmapsPath = g_strAppPath;
	g_strBitmapsPath += "bitmaps/";

	// we will set this right after the game selection is done
	g_strGameToolsPath = g_strAppPath;

#endif

	// init the DTD path
	g_strDTDPath = g_strAppPath;
	g_strDTDPath += "dtds/";

	/*!
	   the global prefs loading / game selection dialog might fail for any reason we don't know about
	   we need to catch when it happens, to cleanup the stateful prefs which might be killing it
	   and to turn on console logging for lookup of the problem
	   this is the first part of the two step .pid system
	 */
	g_pidFile = g_strTempPath.GetBuffer();
	g_pidFile += "radiant.pid";

	FILE *pid;
	pid = fopen( g_pidFile.GetBuffer(), "r" );
	if ( pid != NULL ) {
		fclose( pid );
		CString msg;

		if ( remove( g_pidFile.GetBuffer() ) == -1 ) {
			msg = _( "WARNING: Could not delete " ); msg += g_pidFile;
			gtk_MessageBox( NULL, msg, _( "Radiant" ), MB_OK | MB_ICONERROR );
		}

		// in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined( _DEBUG )
		msg = _( "Found the file " );
		msg += g_pidFile;
		msg += _( ".\nThis indicates that Radiant failed during the game selection startup last time it was run.\n"
			   "Choose YES to clean Radiant's registry settings and shut down Radiant.\n"
			   "WARNING: the global prefs will be lost if you choose YES." );

		if ( gtk_MessageBox( NULL, msg, _( "Radiant - Reset global startup?" ), MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			// remove global prefs and shutdown
			g_PrefsDlg.mGamesDialog.Reset();
			// remove the prefs file (like a full reset of the registry)
			//remove (g_PrefsDlg.m_inipath->str);
			gtk_MessageBox( NULL, _( "Removed global settings, choose OK to close Radiant." ), _( "Radiant" ), MB_OK );
			_exit( -1 );
		}
		msg = _( "Logging console output to " );
		msg += g_strTempPath;
		msg += _( "radiant.log\nRefer to the log if Radiant fails to start again." );

		gtk_MessageBox( NULL, msg, _( "Radiant - Console Log" ), MB_OK );
#endif

		// set without saving, the class is not in a coherent state yet
		// just do the value change and call to start logging, CGamesDialog will pickup when relevant
		g_PrefsDlg.mGamesDialog.m_bLogConsole = true;
		g_PrefsDlg.mGamesDialog.m_bForceLogConsole = true;
		Sys_LogFile();
	}

	// create a primary .pid for global init run
	pid = fopen( g_pidFile.GetBuffer(), "w" );
	if ( pid ) {
		fclose( pid );
	}

	// a safe check to avoid people running broken installations
	// (otherwise, they run it, crash it, and blame us for not forcing them hard enough to pay attention while installing)
	// make something idiot proof and someone will make better idiots, this may be overkill
	// let's leave it disabled in debug mode in any case
#ifndef _DEBUG
	//#define CHECK_VERSION
#endif
#ifdef CHECK_VERSION
	// locate and open RADIANT_MAJOR and RADIANT_MINOR
	qboolean bVerIsGood = true;
	Str ver_file_name;
	ver_file_name = g_strAppPath;
	ver_file_name += "RADIANT_MAJOR";
	FILE *ver_file = fopen( ver_file_name.GetBuffer(), "r" );
	if ( ver_file ) {
		char buf[10];
		int chomp;
		fread( buf, 1, 10, ver_file );
		// chomp it (the hard way)
		chomp = 0;
		while ( buf[chomp] >= '0' && buf[chomp] <= '9' )
			chomp++;
		buf[chomp] = '\0';
		if ( strcmp( buf, RADIANT_MAJOR_VERSION ) ) {
			Sys_FPrintf( SYS_ERR, "ERROR: file RADIANT_MAJOR doesn't match ('%s')\n", buf );
			bVerIsGood = false;
		}
	}
	else
	{
		Sys_FPrintf( SYS_ERR, "ERROR: can't find RADIANT_MAJOR in '%s'\n", ver_file_name.GetBuffer() );
		bVerIsGood = false;
	}
	ver_file_name = g_strAppPath;
	ver_file_name += "RADIANT_MINOR";
	ver_file = fopen( ver_file_name.GetBuffer(), "r" );
	if ( ver_file ) {
		char buf[10];
		int chomp;
		fread( buf, 1, 10, ver_file );
		// chomp it (the hard way)
		chomp = 0;
		while ( buf[chomp] >= '0' && buf[chomp] <= '9' )
			chomp++;
		buf[chomp] = '\0';
		if ( strcmp( buf, RADIANT_MINOR_VERSION ) ) {
			Sys_FPrintf( SYS_ERR, "ERROR: file RADIANT_MINOR doesn't match ('%s')\n", buf );
			bVerIsGood = false;
		}
	}
	else
	{
		Sys_FPrintf( SYS_ERR, "ERROR: can't find RADIANT_MINOR in '%s'\n", ver_file_name.GetBuffer() );
		bVerIsGood = false;
	}
	if ( !bVerIsGood ) {
		CString msg;
		msg = "This editor binary (" RADIANT_VERSION ") doesn't match what the latest setup has configured in this directory\n";
		msg += "Make sure you run the right/latest editor binary you installed\n";
		msg += g_strAppPath; msg += "\n";
		msg += "Check http://www.qeradiant.com/faq/index.cgi?file=219 for more information";
		gtk_MessageBox( NULL, msg.GetBuffer(), _( "Radiant" ), MB_OK, "http://www.qeradiant.com/faq/index.cgi?file=219" );
		_exit( -1 );
	}
#endif

	g_qeglobals.disable_ini = false;
	g_PrefsDlg.Init();

	// close the primary
	if ( remove( g_pidFile.GetBuffer() ) == -1 ) {
		CString msg;
		msg = _( "WARNING: Could not delete " ); msg += g_pidGameFile;
		gtk_MessageBox( NULL, msg, _( "Radiant" ), MB_OK | MB_ICONERROR );
	}

	/*!
	   now the secondary game dependant .pid file
	 */
	g_pidGameFile = g_PrefsDlg.m_rc_path->str;
	g_pidGameFile += "radiant-game.pid";

	pid = fopen( g_pidGameFile.GetBuffer(), "r" );
	if ( pid != NULL ) {
		fclose( pid );
		CString msg;
		if ( remove( g_pidGameFile.GetBuffer() ) == -1 ) {
			msg = _( "WARNING: Could not delete " ); msg += g_pidGameFile;
			gtk_MessageBox( NULL, msg, _( "Radiant" ), MB_OK | MB_ICONERROR );
		}

		msg = _( "Found the file " );
		msg += g_pidGameFile;
		msg += _( ".\nThis indicates that Radiant failed to load the last time it was run.\n"
			   "Choose YES to clean Radiant's registry settings and shut down Radiant.\n"
			   "WARNING: preferences will be lost if you choose YES." );

		// in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined( _DEBUG )
		//bleh
		if ( gtk_MessageBox( NULL, msg, _( "Radiant - Clean Registry?" ), MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			// remove the game prefs files
			remove( g_PrefsDlg.m_inipath->str );
			char buf[PATH_MAX];
			sprintf( buf, "%sSavedInfo.bin", g_PrefsDlg.m_rc_path->str );
			remove( buf );
			// remove the global pref too
			g_PrefsDlg.mGamesDialog.Reset();
			gtk_MessageBox( NULL, _( "Cleaned registry settings, choose OK to close Radiant.\nThe next time Radiant runs it will use default settings." ), _( "Radiant" ), MB_OK );
			_exit( -1 );
		}
		msg = _( "Logging console output to " );
		msg += g_strTempPath;
		msg += _( "radiant.log\nRefer to the log if Radiant fails to start again." );

		gtk_MessageBox( NULL, msg, _( "Radiant - Console Log" ), MB_OK );
#endif

		// force console logging on! (will go in prefs too)
		g_PrefsDlg.mGamesDialog.m_bLogConsole = true;
		g_PrefsDlg.mGamesDialog.SavePrefs();
		Sys_LogFile();

		g_PrefsDlg.LoadPrefs();

	}
	else
	{
		// create one, will remove right after entering message loop
		pid = fopen( g_pidGameFile.GetBuffer(), "w" );
		if ( pid ) {
			fclose( pid );
		}

		g_PrefsDlg.LoadPrefs();

#ifndef _DEBUG // I can't be arsed about that prompt in debug mode
		// if console logging is on in the prefs, warn about performance hit
		if ( g_PrefsDlg.mGamesDialog.m_bLogConsole ) {
			if ( gtk_MessageBox( NULL, _( "Preferences indicate that console logging is on. This affects performance.\n"
									   "Turn it off?" ), _( "Radiant" ), MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
				g_PrefsDlg.mGamesDialog.m_bLogConsole = false;
				g_PrefsDlg.mGamesDialog.SavePrefs();
			}
		}
#endif
		// toggle console logging if necessary
		Sys_LogFile();
	}

	// we should search in g_strTempPath, then move over to look at g_strAppPath?
#ifdef _WIN32
	// fine tune the look of the app using a gtk rc file
	// we try to load an RC file placed in the application directory
	// build the full path
	Str sRCFile;
	sRCFile = g_strAppPath;
	sRCFile += "radiantgtkrc";
	// we load that file with g_new in gtk_rc_parse_file (gtkrc.c), change the '/' into '\\'
	pBuffer = (char *)sRCFile.GetBuffer();
	for ( i = 0; i < sRCFile.GetLength(); i++ )
	{
		if ( pBuffer[i] == '/' ) {
			pBuffer[i] = '\\';
		}
	}
	// check the file exists
	if ( access( sRCFile.GetBuffer(), R_OK ) != 0 ) {
		Sys_Printf( "RC file %s not found\n", sRCFile.GetBuffer() );
	}
	else
	{
		Sys_Printf( "Attemping to load RC file %s\n", sRCFile.GetBuffer() );
		gtk_rc_parse( sRCFile.GetBuffer() );
	}
#endif

#ifndef SKIP_SPLASH
	create_splash();
#endif

	if ( !QGL_Init( libgl, "" ) ) {
		Sys_FPrintf( SYS_ERR, "Failed to load OpenGL libraries\n" );
		_exit( 1 );
		return 1;
	}

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	if ( ( qglXQueryExtension == NULL ) || ( qglXQueryExtension( gdk_x11_get_default_xdisplay(), NULL, NULL ) != True ) ) {
		Sys_FPrintf( SYS_ERR, "glXQueryExtension failed\n" );
		_exit( 1 );
		return 1;
	}
#endif

	// redirect Gtk warnings to the console
	g_log_set_handler( "Gdk", (GLogLevelFlags)( G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING |
												G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG ), error_redirect, NULL );
	g_log_set_handler( "Gtk", (GLogLevelFlags)( G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING |
												G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG ), error_redirect, NULL );

	// spog - creates new filters list for the first time
	g_qeglobals.d_savedinfo.filters = NULL;
	g_qeglobals.d_savedinfo.filters = FilterAddBase( g_qeglobals.d_savedinfo.filters );

	g_pParentWnd = new MainFrame();

	// If the first parameter is a .map, load that.
	if( g_argc > 1 && IsMap( g_argv[1] ) ){
		Map_LoadFile( g_argv[1] );
	}
	else if ( g_PrefsDlg.m_bLoadLastMap && g_PrefsDlg.m_strLastMap.GetLength() > 0 ) {
		Map_LoadFile( g_PrefsDlg.m_strLastMap.GetBuffer() );
	}
	else {
		Map_New();
	}

	// load up shaders now that we have the map loaded
	// eviltypeguy
	Texture_ShowStartupShaders();

#ifndef SKIP_SPLASH
	gdk_window_raise( gtk_widget_get_window( splash_screen ) );
	gtk_window_set_transient_for( GTK_WINDOW( splash_screen ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	g_timeout_add( 1000, try_destroy_splash, NULL );
#endif

	g_pParentWnd->GetSynapseServer().DumpActiveClients();

	//++timo: temporary debug
	g_pParentWnd->DoWatchBSP();

	gtk_main();

	// close the log file if any
	// NOTE: don't save prefs past this point!
	g_PrefsDlg.mGamesDialog.m_bLogConsole = false;
	// set the console window to NULL to avoid Sys_Printf crashing
	g_qeglobals_gui.d_edit = NULL;
	Sys_LogFile();

	// NOTE TTimo not sure what this _exit(0) call is worth
	//   restricting it to linux build
#ifdef __linux__
	_exit( 0 );
#endif
	return 0;
}


#if defined( _WIN32 ) && defined( _MSC_VER ) && !defined( _DEBUG )
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h> //StringCchPrintf

int GenerateDump( EXCEPTION_POINTERS* pExceptionPointers ) {
	BOOL bMiniDumpSuccessful;
    char szPath[MAX_PATH]; 
    char szFileName[MAX_PATH]; 
    char szAppName[] = "GTKRadiant";
    char* szVersion = RADIANT_VERSION;
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime( &stLocalTime );
    GetTempPath( dwBufferSize, szPath );

    StringCchPrintf( szFileName, MAX_PATH, "%s%s", szPath, szAppName );
    CreateDirectory( szFileName, NULL );

    StringCchPrintf( szFileName, MAX_PATH, "%s%s\\%s-%s-%04d%02d%02d-%02d%02d%02d.dmp", 
               szPath, szAppName, szAppName, szVersion, 
               stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
               stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond );
    hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 
                FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    ExpParam.ThreadId = GetCurrentThreadId();
    ExpParam.ExceptionPointers = pExceptionPointers;
    ExpParam.ClientPointers = TRUE;

    bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
                    hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main( int argc, char* argv[] ) {

#if defined( _WIN32 ) && defined( _MSC_VER ) && !defined( _DEBUG )
	__try {
		return mainRadiant( argc, argv );
	} __except( GenerateDump( GetExceptionInformation() ) ) {

		char szPath[MAX_PATH]; 
		char szText[MAX_PATH]; 
		char szFileName[MAX_PATH]; 
		char szAppName[] = "GTKRadiant";
		SYSTEMTIME stLocalTime;
		DWORD dwBufferSize = MAX_PATH;

	    GetLocalTime( &stLocalTime );
	    GetTempPath( dwBufferSize, szPath );

	    StringCchPrintf( szFileName, MAX_PATH, "%s%s", szPath, szAppName );

		StringCchPrintf( szText, MAX_PATH, _("Application crashed!\nCreated a dump file in: \n%s"), szFileName );

		MessageBox( NULL, szText, NULL, MB_ICONERROR );
	}
#else
	return mainRadiant( argc, argv );
#endif
}

// ydnar: quick and dirty fix, just make the buffer bigger
#define BIG_PATH_MAX    4096

// TTimo: decompose the BSP command into several steps so we can monitor them eventually
void QE_ExpandBspString( char *bspaction, GPtrArray *out_array, char *mapname ){
	const char  *in;
	char  *out;
	char src[BIG_PATH_MAX];
	char rsh[BIG_PATH_MAX];
	char base[BIG_PATH_MAX];

	strcpy( src, mapname );
	strlwr( src );
	in = strstr( src, "maps/" );
	if ( in ) {
		in += 5;
		strcpy( base, in );
		out = base;
		while ( *out )
		{
			if ( *out == '\\' ) {
				*out = '/';
			}
			out++;
		}
	}
	else
	{
		ExtractFileName( mapname, base );
	}

	// this important step alters the map name to add fs_game
	// NOTE: it used to add fs_basepath too
	// the fs_basepath addition moved to being in the project file during the bug fixing rush
	// but it may not have been the right thing to do

	// HACK: halflife compiler tools don't support -fs_game
	// HACK: neither does JKII/SoF2/ etc..
	// do we use & have fs_game?

	if ( g_pGameDescription->mGameFile != "hl.game" &&
		 *ValueForKey( g_qeglobals.d_project_entity,"gamename" ) != '\0' ) {
		// set with fs_game
		sprintf( src, "-fs_game %s \"%s\"", ValueForKey( g_qeglobals.d_project_entity,"gamename" ), mapname );
	}
	else
	{
		sprintf( src, "\"%s\"", mapname );
	}

	rsh[0] = 0;

	QE_ConvertDOSToUnixName( src, src );

	// initialise the first step
	out = new char[BIG_PATH_MAX];
	g_ptr_array_add( out_array, out );

	in = ValueForKey( g_qeglobals.d_project_entity, bspaction );
	while ( *in )
	{
		if ( in[0] == '!' ) {
			strcpy( out, rsh );
			out += strlen( rsh );
			in++;
			continue;
		}
		if ( in[0] == '#' ) {
			char tmp[2048];
			// we process these only if monitoring
			if ( g_PrefsDlg.m_bWatchBSP ) {
				// -connect global option (the only global option so far anyway)
				strcpy( tmp, " -connect 127.0.0.1:39000 " );
				strcpy( out, tmp );
				out += strlen( tmp );
			}
			in++;
			continue;
		}
		if ( in[0] == '$' ) {
			// $ expansion
			strcpy( out, src );
			out += strlen( src );
			in++;
			continue;
		}
		if ( in[0] == '@' ) {
			*out++ = '"';
			in++;
			continue;
		}
		if ( in[0] == '&' ) {
			if ( in[1] == '&' ) {
				// start a new step
				*out = 0;
				in = in + 2;
				out = new char[BIG_PATH_MAX];
				g_ptr_array_add( out_array, out );
			}
		}
		*out++ = *in++;
	}
	*out = 0;
}

void FindReplace( CString& strContents, const char* pTag, const char* pValue ){
	if ( strcmp( pTag, pValue ) == 0 ) {
		return;
	}
	for ( int nPos = strContents.Find( pTag ); nPos >= 0; nPos = strContents.Find( pTag ) )
	{
		int nRightLen = strContents.GetLength() - strlen( pTag ) - nPos;
		CString strLeft = strContents.Left( nPos );
		CString strRight = strContents.Right( nRightLen );
		strLeft += pValue;
		strLeft += strRight;
		strContents = strLeft;
	}
}

// save the map, deals with regioning
void SaveWithRegion( char *name ){
	strcpy( name, currentmap );
	if ( region_active ) {
		// temporary cut the region to save regular map
		region_active = false;
		Map_SaveFile( name, false );
		region_active = true;
		StripExtension( name );
		strcat( name, ".reg" );
	}

	Map_SaveFile( name, region_active );
}

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
typedef struct {
	pid_t pid;
	int status;
	int pipes[2];
} bsp_child_process_t;

/*
 * @brief A gtk_idle monitor for redirecting stdout and stderr of the BSP
 * compiler process to the Radiant console. This is used on UNIX platforms for
 * older BSP tools that do not support the XML-based network monitoring found
 * in watchbsp.cpp and feedback.cpp.
 */
static gboolean RunBsp_CaptureOutput(void *data) {
	bsp_child_process_t *process = (bsp_child_process_t *) data;
	pid_t pid;

	// if waitpid returns 0, the child process is alive
	if ( ( pid = waitpid( process->pid, &process->status, WNOHANG ) ) == 0 ) {
		char text[1024];
		ssize_t len;

		if ( (len = read( process->pipes[0], text, sizeof( text ) ) ) > 0 ) {
			GtkTextView *view = GTK_TEXT_VIEW( g_qeglobals_gui.d_edit );
			GtkTextBuffer *buffer = gtk_text_view_get_buffer( view );

			if ( buffer ) {
				GtkTextIter iter;
				gtk_text_buffer_get_end_iter( buffer, &iter );
				gtk_text_buffer_insert( buffer, &iter, text, len );

				gtk_text_buffer_get_end_iter( buffer, &iter );
				gtk_text_view_scroll_to_iter( view, &iter , 0.0, false, 0.0, 0.0 );
			}
		}

		return true; // retain the gtk_idle monitor
	}

	if ( pid == -1 ) {
		Sys_FPrintf( SYS_ERR, "ERROR: Failed to wait for %d: %s\n", process->pid, strerror( errno ) );
	} else {
		Sys_Printf( "Process %d terminated with status %d\n", process->pid, process->status );
	}

	close( process->pipes[0] );
	close( process->pipes[1] );

	free( process);

	return false; // cancel the gtk_idle monitor
}
#endif

void RunBsp( char *command ){
	GPtrArray *sys;
	char batpath[BIG_PATH_MAX];
	char temppath[BIG_PATH_MAX];
	char name[BIG_PATH_MAX];
	char cWork[BIG_PATH_MAX];
	FILE  *hFile;
	unsigned int i;

	SetInspectorMode( W_CONSOLE );

	strcpy( temppath, g_strTempPath.GetBuffer() );

	SaveWithRegion( name );

	const char *rsh = ValueForKey( g_qeglobals.d_project_entity, "rshcmd" );
	if ( rsh == NULL ) {
		CString strPath, strFile;

		ExtractPath_and_Filename( name, strPath, strFile );
		AddSlash( strPath );
		strncpy( cWork, strPath, 1024 );
		strcat( cWork, strFile );
	} else {
		strcpy( cWork, name );
	}

	// get the array ready
	sys = g_ptr_array_new();

	QE_ExpandBspString( command, sys, cWork );

	if ( g_PrefsDlg.m_bWatchBSP ) {
		// grab the file name for engine running
		char *bspname = new char[1024];
		ExtractFileName( currentmap, bspname );
		StripExtension( bspname );
		g_pParentWnd->GetWatchBSP()->DoMonitoringLoop( sys, bspname );
	} else {
		// write all the steps in a single BAT / .sh file and run it, don't bother monitoring it
		CString strSys;
		for ( i = 0; i < sys->len; i++ )
		{
			strSys += (char *)g_ptr_array_index( sys, i );
#ifdef _WIN32  // write temp\junk.txt in win32... NOTE: stops output to shell prompt window
			if ( i == 0 ) {
				strSys += " >";
			}
			else{
				strSys += " >>";
			}
			strSys += "\"";
			strSys += temppath;
			strSys += "junk.txt\"";
#endif
			strSys += "\n";
		};

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

		// write qe3bsp.sh
		sprintf( batpath, "%sqe3bsp.sh", temppath );
		Sys_Printf( "Writing the compile script to '%s'\n", batpath );
		hFile = fopen( batpath, "w" );
		if ( !hFile ) {
			Error( "Can't write to %s", batpath );
		}
		fprintf( hFile, "#!/bin/sh \n\n" );
		fprintf( hFile, "%s", strSys.GetBuffer() );
		fclose( hFile );
		chmod( batpath, 0744 );
#endif

#ifdef _WIN32
		sprintf( batpath, "%sqe3bsp.bat", temppath );
		Sys_Printf( "Writing the compile script to '%s'\n", batpath );
		Sys_Printf( "The build output will be saved in '%sjunk.txt'\n", temppath );
		hFile = fopen( batpath, "w" );
		if ( !hFile ) {
			Error( "Can't write to %s", batpath );
		}
		fprintf( hFile, "%s", strSys.GetBuffer() );
		fclose( hFile );
#endif

		Pointfile_Delete();

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
		bsp_child_process_t *process = ( bsp_child_process_t *) malloc( sizeof( bsp_child_process_t ) );
		memset( process, 0, sizeof( *process ) );

		pipe( process->pipes );

		fcntl( process->pipes[0], F_SETFL, O_NONBLOCK );
		fcntl( process->pipes[1], F_SETFL, O_NONBLOCK );

		process->pid = fork();
		switch ( process->pid )
		{
		case -1:
			Error( "CreateProcess failed" );
			break;
		case 0:
			close( process->pipes[0] ); // close reading end in the child

			dup2( process->pipes[1], 1 ); // send stdout to the pipe
			dup2( process->pipes[1], 2 ); // send stderr to the pipe

			execlp( batpath, batpath, (char *) NULL ); // execute the script

			fprintf( stderr, "Failed to execute %s: %s", batpath, strerror( errno ) );
			_exit( 1 );
			break;
		default:
			g_idle_add( RunBsp_CaptureOutput, (void *) process );
			break;
		}
#endif

#ifdef _WIN32
		Sys_Printf( "Running bsp command...\n" );
		Sys_Printf( "\n%s\n", strSys.GetBuffer() );

		WinExec( batpath, SW_SHOWNORMAL );
#endif
	}
        // free the strings and the array
        for ( i = 0; i < sys->len; i++ ) {
          delete[] (char *)g_ptr_array_index( sys, i );
        }
        g_ptr_array_free( sys, TRUE );
        sys = NULL;
}
