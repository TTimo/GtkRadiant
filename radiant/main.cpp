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

/*! \mainpage GtkRadiant Documentation Index

\section intro_sec Introduction

This documentation is generated from comments in the source code.

\section links_sec Useful Links

\link include/itextstream.h include/itextstream.h \endlink - Global output and error message streams, similar to std::cout and std::cerr. \n

FileInputStream - similar to std::ifstream (binary mode) \n
FileOutputStream - similar to std::ofstream (binary mode) \n
TextFileInputStream - similar to std::ifstream (text mode) \n
TextFileOutputStream - similar to std::ofstream (text mode) \n
StringOutputStream - similar to std::stringstream \n

\link string/string.h string/string.h \endlink - C-style string comparison and memory management. \n
\link os/path.h os/path.h \endlink - Path manipulation for radiant's standard path format \n
\link os/file.h os/file.h \endlink - OS file-system access. \n

::CopiedString - automatic string memory management \n
Array - automatic array memory management \n
HashTable - generic hashtable, similar to std::hash_map \n

\link math/vector.h math/vector.h \endlink - Vectors \n
\link math/matrix.h math/matrix.h \endlink - Matrices \n
\link math/quaternion.h math/quaternion.h \endlink - Quaternions \n
\link math/plane.h math/plane.h \endlink - Planes \n
\link math/aabb.h math/aabb.h \endlink - AABBs \n

Callback MemberCaller FunctionCaller - callbacks similar to using boost::function with boost::bind \n
SmartPointer SmartReference - smart-pointer and smart-reference similar to Loki's SmartPtr \n

\link generic/bitfield.h generic/bitfield.h \endlink - Type-safe bitfield \n
\link generic/enumeration.h generic/enumeration.h \endlink - Type-safe enumeration \n

DefaultAllocator - Memory allocation using new/delete, compliant with std::allocator interface \n

\link debugging/debugging.h debugging/debugging.h \endlink - Debugging macros \n

*/

#include "main.h"

#include "version.h"

#include "debugging/debugging.h"

#include "iundo.h"

#include <gtk/gtkmain.h>

#include "cmdlib.h"
#include "os/file.h"
#include "os/path.h"
#include "stream/stringstream.h"
#include "stream/textfilestream.h"

#include "gtkutil/messagebox.h"
#include "gtkutil/image.h"
#include "console.h"
#include "texwindow.h"
#include "map.h"
#include "mainframe.h"
#include "commands.h"
#include "preferences.h"
#include "environment.h"
#include "referencecache.h"
#include "stacktrace.h"

void show_splash();
void hide_splash();

void error_redirect (const gchar *domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  gboolean in_recursion;
  gboolean is_fatal;
  char buf[256];

  in_recursion = (log_level & G_LOG_FLAG_RECURSION) != 0;
  is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  log_level = (GLogLevelFlags) (log_level & G_LOG_LEVEL_MASK);

  if (!message)
    message = "(0) message";

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

  ERROR_MESSAGE("GTK+ error: " << buf);
}

#if defined (_DEBUG) && defined (WIN32) && defined (_MSC_VER)
#include "crtdbg.h"
#endif

void crt_init()
{
#if defined (_DEBUG) && defined (WIN32) && defined (_MSC_VER)
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
}

class Lock
{
  bool m_locked;
public:
  Lock() : m_locked(false)
  {
  }
  void lock()
  {
    m_locked = true;
  }
  void unlock()
  {
    m_locked = false;
  }
  bool locked() const
  {
    return m_locked;
  }
};

class ScopedLock
{
  Lock& m_lock;
public:
  ScopedLock(Lock& lock) : m_lock(lock)
  {
    m_lock.lock();
  }
  ~ScopedLock()
  {
    m_lock.unlock();
  }
};

class PopupDebugMessageHandler : public DebugMessageHandler
{
  StringOutputStream m_buffer;
  Lock m_lock;
public:
  TextOutputStream& getOutputStream()
  {
    if(!m_lock.locked())
    {
      return m_buffer;
    }
    return globalErrorStream();
  }
  bool handleMessage()
  {
    getOutputStream() << "----------------\n";
    write_stack_trace(getOutputStream());
    getOutputStream() << "----------------\n";
    if(!m_lock.locked())
    {
      ScopedLock lock(m_lock);
#if defined _DEBUG
      m_buffer << "Break into the debugger?\n";
      globalErrorStream() << m_buffer.c_str();
      bool handled = gtk_MessageBox(0, m_buffer.c_str(), "Radiant - Runtime Error", eMB_YESNO, eMB_ICONERROR) == eIDNO;
      m_buffer.clear();
      return handled;
#else
      m_buffer << "Please report this error to the developers\n";
      globalErrorStream() << m_buffer.c_str();
      gtk_MessageBox(0, m_buffer.c_str(), "Radiant - Runtime Error", eMB_OK, eMB_ICONERROR);
      m_buffer.clear();
      return true;
#endif
    }
    return false;
  }
};

typedef Static<PopupDebugMessageHandler> GlobalPopupDebugMessageHandler;

void streams_init()
{
  GlobalErrorStream::instance().setOutputStream(getSysPrintErrorStream());
  GlobalOutputStream::instance().setOutputStream(getSysPrintOutputStream());
}

void paths_init()
{
  const char* home = environment_get_home_path();
  Q_mkdir(home);

  {
    StringOutputStream path(256);
    path << home << RADIANT_VERSION << '/';
    g_strSettingsPath = path.c_str();
  }

  Q_mkdir(g_strSettingsPath.c_str());

  g_strAppPath = environment_get_app_path();

  // radiant is installed in the parent dir of "tools/"
  // NOTE: this is not very easy for debugging
  // maybe add options to lookup in several places?
  // (for now I had to create symlinks)
  {
    StringOutputStream path(256);
    path << g_strAppPath.c_str() << "bitmaps/";
    BitmapsPath_set(path.c_str());
  }

  // we will set this right after the game selection is done
  g_strGameToolsPath = g_strAppPath;
}

bool check_version_file(const char* filename, const char* version)
{
  TextFileInputStream file(filename);
  if(!file.failed())
  {
    char buf[10];
    buf[file.read(buf, 9)] = '\0';

    // chomp it (the hard way)
    int chomp = 0;
    while(buf[chomp] >= '0' && buf[chomp] <= '9')
      chomp++;
    buf[chomp] = '\0';

    return string_equal(buf, version);
  }
  return false;
}

bool check_version()
{
  // a safe check to avoid people running broken installations
  // (otherwise, they run it, crash it, and blame us for not forcing them hard enough to pay attention while installing)
  // make something idiot proof and someone will make better idiots, this may be overkill
  // let's leave it disabled in debug mode in any case
  // http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=431
#ifndef _DEBUG
#define CHECK_VERSION
#endif
#ifdef CHECK_VERSION  
  // locate and open RADIANT_MAJOR and RADIANT_MINOR
  bool bVerIsGood = true;
  {
    StringOutputStream ver_file_name(256);
    ver_file_name << AppPath_get() << "RADIANT_MAJOR";
    bVerIsGood = check_version_file(ver_file_name.c_str(), RADIANT_MAJOR_VERSION);
  }
  {
    StringOutputStream ver_file_name(256);
    ver_file_name << AppPath_get() << "RADIANT_MINOR";
    bVerIsGood = check_version_file(ver_file_name.c_str(), RADIANT_MINOR_VERSION);
  }

  if (!bVerIsGood)
  {
    StringOutputStream msg(256);
    msg << "This editor binary (" RADIANT_VERSION ") doesn't match what the latest setup has configured in this directory\n"
    "Make sure you run the right/latest editor binary you installed\n"
    << AppPath_get();
    gtk_MessageBox(0, msg.c_str(), "Radiant", eMB_OK, eMB_ICONDEFAULT);
  }
  return bVerIsGood;
#else
  return true;
#endif
}

void create_global_pid()
{
  /*!
  the global prefs loading / game selection dialog might fail for any reason we don't know about
  we need to catch when it happens, to cleanup the stateful prefs which might be killing it
  and to turn on console logging for lookup of the problem
  this is the first part of the two step .pid system
  http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=297
  */
  StringOutputStream g_pidFile(256); ///< the global .pid file (only for global part of the startup)

  g_pidFile << SettingsPath_get() << "radiant.pid";

  FILE *pid;
  pid = fopen (g_pidFile.c_str(), "r");
  if (pid != 0)
  {
    fclose (pid);

    if (remove (g_pidFile.c_str()) == -1)
    {
      StringOutputStream msg(256);
      msg << "WARNING: Could not delete " << g_pidFile.c_str();
      gtk_MessageBox (0, msg.c_str(), "Radiant", eMB_OK, eMB_ICONERROR );
    }

    // in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined(_DEBUG)
    StringOutputStream msg(256);
    msg << "Radiant failed to start properly the last time it was run.\n"
           "The failure may be related to current global preferences.\n"
           "Do you want to reset global preferences to defaults?";

    if (gtk_MessageBox (0, msg.c_str(), "Radiant - Startup Failure", eMB_YESNO, eMB_ICONQUESTION) == eIDYES)
    {
      g_GamesDialog.Reset();
    }

    msg.clear();
    msg << "Logging console output to " << SettingsPath_get() << "radiant.log\nRefer to the log if Radiant fails to start again.";

    gtk_MessageBox (0, msg.c_str(), "Radiant - Console Log", eMB_OK);
#endif

    // set without saving, the class is not in a coherent state yet
    // just do the value change and call to start logging, CGamesDialog will pickup when relevant
    g_GamesDialog.m_bForceLogConsole = true;
    Sys_LogFile(true);
  }

  // create a primary .pid for global init run
  pid = fopen (g_pidFile.c_str(), "w");
  if (pid)
    fclose (pid);
}

void remove_global_pid()
{
  StringOutputStream g_pidFile(256);
  g_pidFile << SettingsPath_get() << "radiant.pid";

  // close the primary
  if (remove (g_pidFile.c_str()) == -1)
  {
    StringOutputStream msg(256);
    msg << "WARNING: Could not delete " << g_pidFile.c_str();
    gtk_MessageBox (0, msg.c_str(), "Radiant", eMB_OK, eMB_ICONERROR );
  }
}

/*!
now the secondary game dependant .pid file
http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=297
*/
void create_local_pid()
{
  StringOutputStream g_pidGameFile(256); ///< the game-specific .pid file
  g_pidGameFile << SettingsPath_get() << g_pGameDescription->mGameFile.c_str() << "/radiant-game.pid";

  FILE *pid = fopen (g_pidGameFile.c_str(), "r");
  if (pid != 0)
  {
    fclose (pid);
    if (remove (g_pidGameFile.c_str()) == -1)
    {
      StringOutputStream msg;
      msg << "WARNING: Could not delete " << g_pidGameFile.c_str();
      gtk_MessageBox (0, msg.c_str(), "Radiant", eMB_OK, eMB_ICONERROR );
    }

    // in debug, never prompt to clean registry, turn console logging auto after a failed start
#if !defined(_DEBUG)
    StringOutputStream msg;
    msg << "Radiant failed to start properly the last time it was run.\n"
           "The failure may be caused by current preferences.\n"
           "Do you want to reset all preferences to defaults?";

    if (gtk_MessageBox (0, msg.c_str(), "Radiant - Startup Failure", eMB_YESNO, eMB_ICONQUESTION) == eIDYES)
    {
      Preferences_Reset();
    }

    msg.clear();
    msg << "Logging console output to " << SettingsPath_get() << "radiant.log\nRefer to the log if Radiant fails to start again.";

    gtk_MessageBox (0, msg.c_str(), "Radiant - Console Log", eMB_OK);
#endif

    // force console logging on! (will go in prefs too)
    g_GamesDialog.m_bForceLogConsole = true;
    Sys_LogFile(true);
  }
  else
  {
    // create one, will remove right after entering message loop
    pid = fopen (g_pidGameFile.c_str(), "w");
    if (pid)
      fclose (pid);
  }
}


/*!
now the secondary game dependant .pid file
http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=297
*/
void remove_local_pid()
{
  StringOutputStream g_pidGameFile(256);
  g_pidGameFile << SettingsPath_get() << g_pGameDescription->mGameFile.c_str() << "/radiant-game.pid";
  remove(g_pidGameFile.c_str());
}

void user_shortcuts_init()
{
  StringOutputStream path(256);
  path << SettingsPath_get() << g_pGameDescription->mGameFile.c_str() << '/';
  LoadCommandMap(path.c_str());
  SaveCommandMap(path.c_str());
}

int main (int argc, char* argv[])
{
  crt_init();

  streams_init();

  gtk_disable_setlocale();
  gtk_init(&argc, &argv);

  // redirect Gtk warnings to the console
  g_log_set_handler ("Gdk", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, 0);
  g_log_set_handler ("Gtk", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, 0);
  g_log_set_handler ("GtkGLExt", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, 0);
  g_log_set_handler ("GLib", (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, 0);
  g_log_set_handler (0, (GLogLevelFlags)(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING|
                                             G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG), error_redirect, 0);

  GlobalDebugMessageHandler::instance().setHandler(GlobalPopupDebugMessageHandler::instance());

  environment_init(argc, argv);

  paths_init();

  if(!check_version())
  {
    return EXIT_FAILURE;
  }

  show_splash();

  create_global_pid();

  GlobalPreferences_Init();

  g_GamesDialog.Init();

  g_strGameToolsPath = g_pGameDescription->mGameToolsPath;
  
  remove_global_pid();

  create_local_pid();

  g_Preferences.Init();

  // in a very particular post-.pid startup
  // we may have the console turned on and want to keep it that way
  // so we use a latching system
  if (g_GamesDialog.m_bForceLogConsole)
  {
    Sys_LogFile(true);
    g_Console_enableLogging = true;
    g_GamesDialog.m_bForceLogConsole = false;
  }


  Radiant_Initialise();

  global_accel_init();

  user_shortcuts_init();

  g_pParentWnd = 0;
  g_pParentWnd = new MainFrame();

  hide_splash();

  if (g_bLoadLastMap && !g_strLastMap.empty())
  {
    Map_LoadFile(g_strLastMap.c_str());
  }
  else
  {
    Map_New();
  }

  // load up shaders now that we have the map loaded
  // eviltypeguy
  TextureBrowser_ShowStartupShaders(GlobalTextureBrowser());


  remove_local_pid();

  gtk_main();

  // avoid saving prefs when the app is minimized
  if (g_pParentWnd->IsSleeping())
  {
    globalOutputStream() << "Shutdown while sleeping, not saving prefs\n";
    g_preferences_globals.disable_ini = true;
  }

  Map_Free();

  if (!Map_Unnamed(g_map))
  {
    g_strLastMap = Map_Name(g_map);
  }

  delete g_pParentWnd;

  global_accel_destroy();

  Radiant_Shutdown();

  // close the log file if any
  Sys_LogFile(false);

  return EXIT_SUCCESS;
}
