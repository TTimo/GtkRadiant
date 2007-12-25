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

#if defined (__linux__) || defined (__APPLE__)
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
#endif

#include <gtk/gtk.h>
#include "stdafx.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>

#include "watchbsp.h"
#include "filters.h"

bool g_bBuildList = false;
int g_argc;
char** g_argv;

// =============================================================================
// Splash screen

// get rid of it when debugging
#if defined (_DEBUG)
  #define SKIP_SPLASH
#endif

static GtkWidget *splash_screen;

// called based on a timer, or in particular cases when we don't want to keep it around
gint try_destroy_splash (gpointer data)
{
  if (splash_screen)
  {
    gtk_widget_destroy (splash_screen);
    splash_screen = NULL;
  }
  return FALSE;
}

static void create_splash ()
{
  GtkWidget *alert_frame, *alert_frame1, *pixmap;

  splash_screen = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_position (GTK_WINDOW (splash_screen), GTK_WIN_POS_CENTER);
  gtk_widget_realize (splash_screen);

  alert_frame1 = gtk_frame_new (NULL);
  gtk_widget_show (alert_frame1);
  gtk_container_add (GTK_CONTAINER (splash_screen), alert_frame1);
  gtk_frame_set_shadow_type (GTK_FRAME (alert_frame1), GTK_SHADOW_OUT);

  alert_frame = gtk_frame_new (NULL);
  gtk_widget_show (alert_frame);

  gtk_container_add (GTK_CONTAINER (alert_frame1), alert_frame);
  gtk_frame_set_shadow_type (GTK_FRAME (alert_frame), GTK_SHADOW_IN);
  gtk_container_border_width (GTK_CONTAINER (alert_frame), 3);

  pixmap = gtk_preview_new (GTK_PREVIEW_COLOR);
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (alert_frame), pixmap);

  CString str;
  guint16 width, height;
  unsigned char *buf;

  str = g_strGameToolsPath;
  str += "bitmaps/splash.bmp";

  unsigned char* load_bitmap_file (const char* filename, guint16* width, guint16* height);
  buf = load_bitmap_file (str.GetBuffer (), &width, &height);

  if (!buf)
  {
    str = g_strBitmapsPath;
    str += "splash.bmp";

    buf = load_bitmap_file (str.GetBuffer (), &width, &height);
  }

  if (buf)
  {
    GtkPreview *preview = GTK_PREVIEW (pixmap);
    gtk_preview_size (preview, width, height);
    for (int y = 0; y < height; y++)
      gtk_preview_draw_row (preview, buf+y*width*3, 0, y, width);
  }

  gtk_widget_show_all (splash_screen);

  while (gtk_events_pending ())
    gtk_main_iteration ();
}

// =============================================================================
// Loki stuff

#if defined (__linux__) || defined (__APPLE__)

/* A short game name, could be used as argv[0] */
static char game_name[100] = "";

/* The directory where the data files can be found (run directory) */
static char datapath[PATH_MAX];

char *loki_gethomedir(void)
{
  char *home = NULL;

  home = getenv("HOME");
  if ( home == NULL )
  {
    uid_t id = getuid();
    struct passwd *pwd;

    setpwent();
    while ( (pwd = getpwent()) != NULL )
    {
      if ( pwd->pw_uid == id )
      {
        home = pwd->pw_dir;
        break;
      }
    }
    endpwent();
  }
  return home;
}

/* Must be called BEFORE loki_initialize */
void loki_setgamename(const char *n)
{
  strncpy(game_name, n, sizeof(game_name));
}

  #ifdef __linux__
/* Code to determine the mount point of a CD-ROM */
int loki_getmountpoint(const char *device, char *mntpt, int max_size)
{
  char devpath[PATH_MAX], mntdevpath[PATH_MAX];
  FILE * mountfp;
  struct mntent *mntent;
  int mounted;

  /* Nothing to do with no device file */
  if ( device == NULL )
  {
    *mntpt = '\0';
    return -1;
  }

  /* Get the fully qualified path of the CD-ROM device */
  if ( realpath(device, devpath) == NULL )
  {
    perror("realpath() on your CD-ROM failed");
    return(-1);
  }

  /* Get the mount point */
  mounted = -1;
  memset(mntpt, 0, max_size);
  mountfp = setmntent( _PATH_MNTTAB, "r" );
  if ( mountfp != NULL )
  {
    mounted = 0;
    while ( (mntent = getmntent( mountfp )) != NULL )
    {
      char *tmp, mntdev[1024];

      strcpy(mntdev, mntent->mnt_fsname);
      if ( strcmp(mntent->mnt_type, "supermount") == 0 )
      {
        tmp = strstr(mntent->mnt_opts, "dev=");
        if ( tmp )
        {
          strcpy(mntdev, tmp+strlen("dev="));
          tmp = strchr(mntdev, ',');
          if ( tmp )
          {
            *tmp = '\0';
          }
        }
      }
      if ( strncmp(mntdev, "/dev", 4) ||
           realpath(mntdev, mntdevpath) == NULL )
      {
        continue;
      }
      if ( strcmp( mntdevpath, devpath ) == 0 )
      {
        mounted = 1;
        assert((int)strlen( mntent->mnt_dir ) < max_size);
        strncpy( mntpt, mntent->mnt_dir, max_size-1);
        mntpt[max_size-1] = '\0';
        break;
      }
    }
    endmntent( mountfp );
  }
  return(mounted);
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
void loki_initpaths(char *argv0)
{
  char temppath[PATH_MAX]; //, env[100];
  char *home; //, *ptr, *data_env;

  home = loki_gethomedir();
  if ( home == NULL )
  {
    home = ".";
  }

  if (*game_name == 0) /* Game name defaults to argv[0] */
    loki_setgamename(argv0);

  strcpy(temppath, argv0);  /* If this overflows, it's your own fault :) */
  if ( ! strrchr(temppath, '/') )
  {
    char *path;
    char *last;
    int found;

    found = 0;
    path = getenv("PATH");
    do
    {
      /* Initialize our filename variable */
      temppath[0] = '\0';

      /* Get next entry from path variable */
      last = strchr(path, ':');
      if ( ! last )
        last = path+strlen(path);

      /* Perform tilde expansion */
      if ( *path == '~' )
      {
        strcpy(temppath, home);
        ++path;
      }

      /* Fill in the rest of the filename */
      if ( last > (path+1) )
      {
        strncat(temppath, path, (last-path));
        strcat(temppath, "/");
      }
      strcat(temppath, "./");
      strcat(temppath, argv0);

      /* See if it exists, and update path */
      if ( access(temppath, X_OK) == 0 )
      {
        ++found;
      }
      path = last+1;

    } while ( *last && !found );

  } else
  {
    /* Increment argv0 to the basename */
    argv0 = strrchr(argv0, '/')+1;
  }

  /* Now canonicalize it to a full pathname for the data path */
  if ( realpath(temppath, datapath) )
  {
    /* There should always be '/' in the path */
    *(strrchr(datapath, '/')) = '\0';
  }
}

char *loki_getdatapath(void)
{
  return(datapath);
}

#endif

// end of Loki stuff
// =============================================================================

void error_redirect (const gchar *domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  gboolean in_recursion;
  gboolean is_fatal;
  char buf[256];

  in_recursion = (log_level & G_LOG_FLAG_RECURSION) != 0;
  is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  log_level = (GLogLevelFlags) (log_level & G_LOG_LEVEL_MASK);

  if (!message)
    message = "(NULL) message";

  if (domain)
    strcpy (buf, domain);
  else
    strcpy (buf, "**");
  strcat (buf, "-");

  switch (log_level)
  {
  case G_LOG_LEVEL_ERROR:
    if (in_recursion)
      strcat (buf, "ERROR (recursed) **: ");
    else
      strcat (buf, "ERROR **: ");
    break;
  case G_LOG_LEVEL_CRITICAL:
    if (in_recursion)
      strcat (buf, "CRITICAL (recursed) **: ");
    else
      strcat (buf, "CRITICAL **: ");
    break;
  case G_LOG_LEVEL_WARNING:
    if (in_recursion)
      strcat (buf, "WARNING (recursed) **: ");
    else
      strcat (buf, "WARNING **: ");
    break;
  case G_LOG_LEVEL_MESSAGE:
    if (in_recursion)
      strcat (buf, "Message (recursed): ");
    else
      strcat (buf, "Message: ");
    break;
  case G_LOG_LEVEL_INFO:
    if (in_recursion)
      strcat (buf, "INFO (recursed): ");
    else
      strcat (buf, "INFO: ");
    break;
  case G_LOG_LEVEL_DEBUG:
    if (in_recursion)
      strcat (buf, "DEBUG (recursed): ");
    else
      strcat (buf, "DEBUG: ");
    break;
  default:
    /* we are used for a log level that is not defined by GLib itself,
     * try to make the best out of it.
     */
    if (in_recursion)
      strcat (buf, "LOG (recursed:");
    else
      strcat (buf, "LOG (");
    if (log_level)
    {
      gchar string[] = "0x00): ";
      gchar *p = string + 2;
      guint i;

      i = g_bit_nth_msf (log_level, -1);
      *p = i >> 4;
      p++;
      *p = '0' + (i & 0xf);
      if (*p > '9')
        *p += 'A' - '9' - 1;

      strcat (buf, string);
    } else
      strcat (buf, "): ");
  }

  strcat (buf, message);
  if (is_fatal)
    strcat (buf, "\naborting...\n");
  else
    strcat (buf, "\n");

  printf ("%s\n", buf);
  Sys_FPrintf (SYS_WRN, buf);
  // TTimo NOTE: in some cases it may be handy to log only to the file
//  Sys_FPrintf (SYS_NOCON, buf);
}

int main( int argc, char* argv[] ) {
	char *libgl, *ptr;
	int i, j, k;

#ifdef _WIN32
  libgl = "opengl32.dll";
#endif

#if defined (__linux__)
  libgl = "libGL.so.1";
#endif

#ifdef __APPLE__
  libgl = "/usr/X11R6/lib/libGL.1.dylib";
#endif

#if defined (__linux__) || defined (__APPLE__)
  // Give away unnecessary root privileges.
  // Important: must be done before calling gtk_init().
  char *loginname;
  struct passwd *pw;
  seteuid(getuid());
  if ( geteuid() == 0 && ( loginname = getlogin() ) != NULL && ( pw = getpwnam(loginname) ) != NULL ) {
	  setuid(pw->pw_uid);
  }
#endif

  gtk_disable_setlocale();

  gtk_init(&argc, &argv);

  if ((ptr = getenv ("Q3R_LIBGL")) != NULL)
    libgl = ptr;

  for (i = 1; i < argc; i++)
  {
    char* param = argv[i];

    if (param[0] == '-' && param[1] == '-')
    {
      param += 2;

      if ((strcmp (param, "libgl") == 0) && (i != argc))
      {
        libgl = argv[i+1];
        argv[i] = argv[i+1] = NULL;
        i++;
      } else if (strcmp (param, "builddefs") == 0)
      {
        g_bBuildList = true;
        argv[i] = NULL;
      }
    }
  }

  for (i = 1; i < argc; i++)
  {
    for (k = i; k < argc; k++)
      if (argv[k] != NULL)
        break;

    if (k > i)
    {
      k -= i;
      for (j = i + k; j < argc; j++)
        argv[j-k] = argv[j];
      argc -= k;
    }
  }

  g_argc = argc;
  g_argv = argv;

  g_strPluginsDir = "plugins/";
  g_strModulesDir = "modules/";

#ifdef _WIN32
  // get path to the editor
  char* pBuffer = g_strAppPath.GetBufferSetLength(_MAX_PATH + 1);
  GetModuleFileName(NULL, pBuffer, _MAX_PATH);
  pBuffer[g_strAppPath.ReverseFind('\\') + 1] = '\0';
  QE_ConvertDOSToUnixName(pBuffer, pBuffer);
  g_strAppPath.ReleaseBuffer();

  g_strBitmapsPath = g_strAppPath;
  g_strBitmapsPath += "bitmaps/";

  CGameDialog::UpdateNetrun(false); // read the netrun configuration

  if ( CGameDialog::GetNetrun() ) {
    // we have to find a per-user g_strTempPath
    // this behaves the same as on Linux
    g_strTempPath = getenv("USERPROFILE");
    if (!g_strTempPath.GetLength())
    {
      CString msg;
      msg = "Radiant is configured to run from a network installation.\n";
      msg += "I couldn't find the environement variable USERPROFILE\n";
      msg += "I'm going to use C:\\RadiantSettings. Please set USERPROFILE\n";
      gtk_MessageBox (NULL, msg, "Radiant - Network mode", MB_OK);
      g_strTempPath = "C:\\";
    }
    g_strTempPath += "\\RadiantSettings\\";
    Q_mkdir(g_strTempPath.GetBuffer(), 0755);
    g_strTempPath += RADIANT_VERSION;
    g_strTempPath += "\\";
    Q_mkdir(g_strTempPath.GetBuffer(), 0755);
  }
  else
  {
    // use the core path as temp (to save commandlist.txt, and do the .pid files)
    g_strTempPath = g_strAppPath;
  }

#endif

#if defined (__linux__) || defined (__APPLE__)
  Str home;
  home = g_get_home_dir ();
  AddSlash (home);
  home += ".radiant/";
  Q_mkdir (home.GetBuffer (), 0775);
  home += RADIANT_VERSION;
  Q_mkdir (home.GetBuffer (), 0775);
  g_strTempPath = home.GetBuffer ();
  AddSlash (g_strTempPath);

  loki_initpaths(argv[0]);

  // NOTE: we build g_strAppPath with a '/' (or '\' on WIN32)
  // it's a general convention in Radiant to have the slash at the end of directories
  char real[PATH_MAX];
  realpath (loki_getdatapath(), real);
  if (real[strlen(real)-1] != '/')
    strcat(real, "/");

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
  http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=297
  */
  g_pidFile = g_strTempPath.GetBuffer ();
  g_pidFile += "radiant.pid";

  FILE *pid;
  pid = fopen( g_pidFile.GetBuffer(), "r" );
  if ( pid != NULL ) {
    fclose (pid);
    CString msg;

    if (remove (g_pidFile.GetBuffer ()) == -1)
    {
      msg = "WARNING: Could not delete "; msg += g_pidFile;
      gtk_MessageBox (NULL, msg, "Radiant", MB_OK | MB_ICONERROR );
    }

    // in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined(_DEBUG)
    msg = "Found the file ";
    msg += g_pidFile;
    msg += ".\nThis indicates that Radiant failed during the game selection startup last time it was run.\n"
           "Choose YES to clean Radiant's registry settings and shut down Radiant.\n"
           "WARNING: the global prefs will be lost if you choose YES.";

    if (gtk_MessageBox (NULL, msg, "Radiant - Reset global startup?", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
      // remove global prefs and shutdown
      g_PrefsDlg.mGamesDialog.Reset();
      // remove the prefs file (like a full reset of the registry)
      //remove (g_PrefsDlg.m_inipath->str);
      gtk_MessageBox(NULL, "Removed global settings, choose OK to close Radiant.", "Radiant", MB_OK );
      _exit(-1);
    }
    msg = "Logging console output to ";
    msg += g_strTempPath;
    msg += "radiant.log\nRefer to the log if Radiant fails to start again.";

    gtk_MessageBox (NULL, msg, "Radiant - Console Log", MB_OK);
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
  // http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=431
#ifndef _DEBUG
  //#define CHECK_VERSION
#endif
#ifdef CHECK_VERSION  
  // locate and open RADIANT_MAJOR and RADIANT_MINOR
  qboolean bVerIsGood = true;
  Str ver_file_name;
  ver_file_name = g_strAppPath;
  ver_file_name += "RADIANT_MAJOR";
  FILE *ver_file = fopen (ver_file_name.GetBuffer(), "r");
  if (ver_file)
  {
    char buf[10];
    int chomp;
    fread(buf, 1, 10, ver_file);
    // chomp it (the hard way)
    chomp = 0;
    while(buf[chomp] >= '0' && buf[chomp] <= '9')
      chomp++;
    buf[chomp] = '\0';
    if (strcmp(buf, RADIANT_MAJOR_VERSION))
    {
      Sys_Printf("ERROR: file RADIANT_MAJOR doesn't match ('%s')\n", buf);
      bVerIsGood = false;
    }
  }
  else
  {
    Sys_Printf("ERROR: can't find RADIANT_MAJOR in '%s'\n", ver_file_name.GetBuffer());
    bVerIsGood = false;
  }
  ver_file_name = g_strAppPath;
  ver_file_name += "RADIANT_MINOR";
  ver_file = fopen (ver_file_name.GetBuffer(), "r");
  if (ver_file)
  {
    char buf[10];
    int chomp;
    fread(buf, 1, 10, ver_file);
    // chomp it (the hard way)
    chomp = 0;
    while(buf[chomp] >= '0' && buf[chomp] <= '9')
      chomp++;
    buf[chomp] = '\0';
    if (strcmp(buf, RADIANT_MINOR_VERSION))
    {
      Sys_Printf("ERROR: file RADIANT_MINOR doesn't match ('%s')\n", buf);
      bVerIsGood = false;
    }
  }
  else
  {
    Sys_Printf("ERROR: can't find RADIANT_MINOR in '%s'\n", ver_file_name.GetBuffer());
    bVerIsGood = false;
  }
  if (!bVerIsGood)
  {
    CString msg;
    msg = "This editor binary (" RADIANT_VERSION ") doesn't match what the latest setup has configured in this directory\n";
    msg += "Make sure you run the right/latest editor binary you installed\n";
    msg += g_strAppPath; msg += "\n";
    msg += "Check http://www.qeradiant.com/faq/index.cgi?file=219 for more information";
    gtk_MessageBox(NULL, msg.GetBuffer(), "Radiant", MB_OK, "http://www.qeradiant.com/faq/index.cgi?file=219");
    _exit(-1);
  }
#endif
  
  g_qeglobals.disable_ini = false;
  g_PrefsDlg.Init();

  // close the primary
  if ( remove( g_pidFile.GetBuffer () ) == -1 ) {
    CString msg;
    msg = "WARNING: Could not delete "; msg += g_pidGameFile;
    gtk_MessageBox (NULL, msg, "Radiant", MB_OK | MB_ICONERROR );
  }
  
  /*!
  now the secondary game dependant .pid file
  http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=297
  */
  g_pidGameFile = g_PrefsDlg.m_rc_path->str;
  g_pidGameFile += "radiant-game.pid";

  pid = fopen (g_pidGameFile.GetBuffer(), "r");
  if (pid != NULL)
  {
    fclose (pid);
    CString msg;
    if (remove (g_pidGameFile.GetBuffer ()) == -1)
    {
      msg = "WARNING: Could not delete "; msg += g_pidGameFile;
      gtk_MessageBox (NULL, msg, "Radiant", MB_OK | MB_ICONERROR );
    }

    msg = "Found the file ";
    msg += g_pidGameFile;
    msg += ".\nThis indicates that Radiant failed to load the last time it was run.\n"
           "Choose YES to clean Radiant's registry settings and shut down Radiant.\n"
           "WARNING: preferences will be lost if you choose YES.";

    // in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined(_DEBUG)
    //bleh
    if (gtk_MessageBox (NULL, msg, "Radiant - Clean Registry?", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
      // remove the game prefs files
      remove (g_PrefsDlg.m_inipath->str);
      char buf[PATH_MAX];
      sprintf(buf, "%sSavedInfo.bin", g_PrefsDlg.m_rc_path->str);
      remove(buf);
      // remove the global pref too
      g_PrefsDlg.mGamesDialog.Reset();
      gtk_MessageBox(NULL, "Cleaned registry settings, choose OK to close Radiant.\nThe next time Radiant runs it will use default settings.", "Radiant", MB_OK );
      _exit(-1);
    }
    msg = "Logging console output to ";
    msg += g_strTempPath;
    msg += "radiant.log\nRefer to the log if Radiant fails to start again.";

    gtk_MessageBox (NULL, msg, "Radiant - Console Log", MB_OK);
#endif

    // force console logging on! (will go in prefs too)
    g_PrefsDlg.mGamesDialog.m_bLogConsole = true;
    g_PrefsDlg.mGamesDialog.SavePrefs();
    Sys_LogFile();

    g_PrefsDlg.LoadPrefs();

  } else
  {
    // create one, will remove right after entering message loop
    pid = fopen (g_pidGameFile.GetBuffer(), "w");
    if (pid)
      fclose (pid);

    g_PrefsDlg.LoadPrefs();

#ifndef _DEBUG // I can't be arsed about that prompt in debug mode
    // if console logging is on in the prefs, warn about performance hit
    if (g_PrefsDlg.mGamesDialog.m_bLogConsole)
    {
      if (gtk_MessageBox (NULL, "Preferences indicate that console logging is on. This affects performance.\n"
                          "Turn it off?", "Radiant", MB_YESNO | MB_ICONQUESTION) == IDYES)
      {
        g_PrefsDlg.mGamesDialog.m_bLogConsole = false;
        g_PrefsDlg.mGamesDialog.SavePrefs();
      }
    }
#endif
    // toggle console logging if necessary
    Sys_LogFile();
  }

  // FIXME http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=639
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
  for (i=0; i<sRCFile.GetLength(); i++)
  {
    if (pBuffer[i]=='/')
    {
      pBuffer[i] = '\\';
    }
  }
  // check the file exists
  if (access(sRCFile.GetBuffer(), R_OK) != 0)
    Sys_Printf("RC file %s not found\n", sRCFile.GetBuffer());
  else
  {
    Sys_Printf ("Attemping to load RC file %s\n", sRCFile.GetBuffer());
    gtk_rc_parse (sRCFile.GetBuffer());
  }
#endif

#ifndef SKIP_SPLASH
  create_splash();
#endif

  if (!QGL_Init(libgl, ""))
  {
    Sys_FPrintf (SYS_ERR, "Failed to load OpenGL libraries\n");
    _exit (1);
    return 1;
  }

#if defined (__linux__) || defined (__APPLE__)
  if ((qglXQueryExtension == NULL) || (qglXQueryExtension(GDK_DISPLAY(),NULL,NULL) != True))
  {
    Sys_FPrintf (SYS_ERR, "glXQueryExtension failed\n");
    _exit (1);
    return 1;
  }
#endif

  // redirect Gtk warnings to the console
  g_log_set_handler( "Gdk", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, NULL);
  g_log_set_handler( "Gtk", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, NULL);

  // spog - creates new filters list for the first time
  g_qeglobals.d_savedinfo.filters = NULL;
  g_qeglobals.d_savedinfo.filters = FilterUpdate(g_qeglobals.d_savedinfo.filters);

  g_pParentWnd = new MainFrame();

  if ( g_PrefsDlg.m_bLoadLastMap && g_PrefsDlg.m_strLastMap.GetLength() > 0 ) {
	  Map_LoadFile(g_PrefsDlg.m_strLastMap.GetBuffer());
  } else {
	  Map_New();
  }

  // load up shaders now that we have the map loaded
  // eviltypeguy
  Texture_ShowStartupShaders();

#ifndef SKIP_SPLASH
  gdk_window_raise(splash_screen->window);
  gtk_window_set_transient_for( GTK_WINDOW( splash_screen ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
  gtk_timeout_add( 1000, try_destroy_splash, NULL );
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

// ydnar: quick and dirty fix, just make the buffer bigger
#define BIG_PATH_MAX	4096

// TTimo: decompose the BSP command into several steps so we can monitor them eventually
void QE_ExpandBspString (char *bspaction, GPtrArray *out_array, char *mapname)
{
  const char  *in;
  char  *out;
  char  src[BIG_PATH_MAX];
  char  rsh[BIG_PATH_MAX];
  char  base[BIG_PATH_MAX];

  strcpy(src, mapname);
  strlwr(src);
  in = strstr(src, "maps/");
  if (!in)
  {
    in = strstr(src, "maps/");
  }
  if (in)
  {
    in += 5;
    strcpy(base, in);
    out = base;
    while (*out)
    {
      if (*out == '\\')
      {
        *out = '/';
      }
      out++;
    }
  } else
  {
    ExtractFileName (mapname, base);
  }

  // this important step alters the map name to add fs_game
  // NOTE: it used to add fs_basepath too
  // the fs_basepath addition moved to being in the project file during the bug fixing rush
  // but it may not have been the right thing to do

  // HACK: halflife compiler tools don't support -fs_game
  // HACK: neither does JKII/SoF2/ etc..
  // do we use & have fs_game?
  
  if (g_pGameDescription->mGameFile != "hl.game" &&
      *ValueForKey(g_qeglobals.d_project_entity,"gamename") != '\0')
    {
      // set with fs_game
      sprintf(src, "-fs_game %s \"%s\"", ValueForKey(g_qeglobals.d_project_entity,"gamename"), mapname);
    }
    else
    {
      sprintf(src, "\"%s\"", mapname);
  }

  rsh[0] = 0;

  QE_ConvertDOSToUnixName(src, src);

  // initialise the first step
  out = new char[BIG_PATH_MAX];	//% PATH_MAX
  g_ptr_array_add( out_array, out );

  in = ValueForKey( g_qeglobals.d_project_entity, bspaction );
  while (*in)
  {
    if (in[0] == '!')
    {
      strcpy (out, rsh);
      out += strlen(rsh);
      in++;
      continue;
    }
    if (in[0] == '#')
    {
      char tmp[2048];
      // we process these only if monitoring
      if (g_PrefsDlg.m_bWatchBSP)
      {
        // -connect global option (the only global option so far anyway)
        strcpy (tmp, " -connect 127.0.0.1:39000 ");
        strcpy (out, tmp);
        out += strlen(tmp);
      }
      in++;
      continue;
    }
    if (in[0] == '$')
    {
      // $ expansion
      strcpy (out, src);
      out += strlen(src);
      in++;
      continue;
    }
    if (in[0] == '@')
    {
      *out++ = '"';
      in++;
      continue;
    }
    if (in[0] == '&')
      if (in[1] == '&')
      {
        // start a new step
        *out = 0;
        in = in + 2;
        out = new char[BIG_PATH_MAX];	//% PATH_MAX
        g_ptr_array_add( out_array, out );
      }
    *out++ = *in++;
  }
  *out = 0;
}

void FindReplace(CString& strContents, const char* pTag, const char* pValue)
{
  if (strcmp(pTag, pValue) == 0)
    return;
  for (int nPos = strContents.Find(pTag); nPos >= 0; nPos = strContents.Find(pTag))
  {
    int nRightLen = strContents.GetLength() - strlen(pTag) - nPos;
    CString strLeft = strContents.Left(nPos);
    CString strRight = strContents.Right(nRightLen);
    strLeft += pValue;
    strLeft += strRight;
    strContents = strLeft;
  }
}

// save the map, deals with regioning
void SaveWithRegion(char *name)
{
  strcpy (name, currentmap);
  if (region_active)
  {
    // temporary cut the region to save regular map
    region_active = false;
    Map_SaveFile (name, false);
    region_active = true;
    StripExtension (name);
    strcat (name, ".reg");
  }

  Map_SaveFile (name, region_active);
}

void RunBsp (char *command)
{
  GPtrArray *sys;
  char  batpath[BIG_PATH_MAX];	//% PATH_MAX
  char  temppath[BIG_PATH_MAX];	//% PATH_MAX
  char  name[BIG_PATH_MAX];		//% PATH_MAX
  char  cWork[BIG_PATH_MAX];	//% PATH_MAX
  FILE  *hFile;
  unsigned int   i;

  SetInspectorMode(W_CONSOLE);

  strcpy (temppath, g_strTempPath.GetBuffer ());

  SaveWithRegion(name);

  const char *rsh = ValueForKey(g_qeglobals.d_project_entity, "rshcmd");
  if (rsh == NULL)
  {
    CString strPath, strFile;

    ExtractPath_and_Filename(name, strPath, strFile);
    AddSlash(strPath);
    strncpy(cWork, strPath, 1024);
    strcat(cWork, strFile);
  } else
  {
    strcpy(cWork, name);
  }

  // get the array ready
  //++timo TODO: free the array, free the strings ourselves with delete[]
  sys = g_ptr_array_new();

  QE_ExpandBspString (command, sys, cWork);

  if (g_PrefsDlg.m_bWatchBSP)
  {
    // grab the file name for engine running
    char *bspname = new char[1024];
    ExtractFileName( currentmap, bspname );
    StripExtension( bspname );
    g_pParentWnd->GetWatchBSP()->DoMonitoringLoop( sys, bspname );
  } else
  {
    // write all the steps in a single BAT / .sh file and run it, don't bother monitoring it
    CString strSys;
    for (i=0; i < sys->len; i++ )
    {
      strSys += (char *)g_ptr_array_index( sys, i);
#ifdef _WIN32  // write temp\junk.txt in win32... NOTE: stops output to shell prompt window
      if (i==0) 
        strSys += " >";
      else 
        strSys += " >>";
      strSys += "\"";
      strSys += temppath;
      strSys += "junk.txt\"";
#endif
      strSys += "\n";
    };

#if defined (__linux__) || defined (__APPLE__)

    // write qe3bsp.sh
    sprintf (batpath, "%sqe3bsp.sh", temppath);
    Sys_Printf("Writing the compile script to '%s'\n", batpath);
    Sys_Printf("The build output will be saved in '%sjunk.txt'\n", temppath);
    hFile = fopen(batpath, "w");
    if (!hFile)
      Error ("Can't write to %s", batpath);
    fprintf (hFile, "#!/bin/sh \n\n");
    fprintf (hFile, strSys.GetBuffer());
    fclose (hFile);
    chmod (batpath, 0744);
#endif

#ifdef _WIN32
    sprintf (batpath, "%sqe3bsp.bat", temppath);
    Sys_Printf("Writing the compile script to '%s'\n", batpath);
    Sys_Printf("The build output will be saved in '%sjunk.txt'\n", temppath);
    hFile = fopen(batpath, "w");
    if (!hFile)
      Error ("Can't write to %s", batpath);
    fprintf (hFile, strSys.GetBuffer());
    fclose (hFile); 
#endif

    Pointfile_Delete ();

#if defined (__linux__) || defined (__APPLE__)

    pid_t pid;

    pid = fork ();
    switch (pid)
    {
    case -1:
      Error ("CreateProcess failed");
      break;
    case 0:
      execlp (batpath, batpath, NULL);
      printf ("execlp error !");
      _exit (0);
      break;
    default:
      break;
    }
#endif

#ifdef _WIN32
    Sys_Printf ("Running bsp command...\n");
    Sys_Printf ("\n%s\n", strSys.GetBuffer());

    WinExec( batpath, SW_SHOWNORMAL );
#endif
  }
#ifdef _DEBUG
  // yeah, do it .. but not now right before 1.1-TA-beta release
  Sys_Printf("TODO: erase GPtrArray\n");
#endif
}

#if 0

#ifdef _WIN32

int WINAPI QEW_SetupPixelFormat(HDC hDC, qboolean zbuffer )
{
  static PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
    1,                              // version number
    PFD_DRAW_TO_WINDOW |            // support window
    PFD_SUPPORT_OPENGL |            // support OpenGL
    PFD_DOUBLEBUFFER,               // double buffered
    PFD_TYPE_RGBA,                  // RGBA type
    24,                             // 24-bit color depth
    0, 0, 0, 0, 0, 0,               // color bits ignored
    0,                              // no alpha buffer
    0,                              // shift bit ignored
    0,                              // no accumulation buffer
    0, 0, 0, 0,                     // accum bits ignored
    32,                             // depth bits
    0,                              // no stencil buffer
    0,                              // no auxiliary buffer
    PFD_MAIN_PLANE,                 // main layer
    0,                              // reserved
    0, 0, 0                         // layer masks ignored
  };                              //
  int pixelformat = 0;            

  zbuffer = true;
  if ( !zbuffer )
    pfd.cDepthBits = 0;

  if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
  {
    LPVOID lpMsgBuf;
    FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                 FORMAT_MESSAGE_FROM_SYSTEM | 
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR) &lpMsgBuf,
                 0,
                 NULL 
                 );
    Sys_FPrintf (SYS_WRN, "GetLastError: %s", lpMsgBuf);
    Error ("ChoosePixelFormat failed");
  }

  if (!SetPixelFormat(hDC, pixelformat, &pfd))
    Error ("SetPixelFormat failed");

  return pixelformat;
}

#endif

#endif
