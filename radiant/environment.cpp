/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#include "environment.h"

#include "stream/textstream.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "debugging/debugging.h"
#include "os/path.h"
#include "cmdlib.h"


int g_argc;
char** g_argv;

void args_init(int argc, char* argv[])
{
  int i, j, k;

  for (i = 1; i < argc; i++)
  {
    for (k = i; k < argc; k++)
      if (argv[k] != 0)
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
}

namespace
{
  CopiedString home_path;
  CopiedString app_path;
}

const char* environment_get_home_path()
{
  return home_path.c_str();
}

const char* environment_get_app_path()
{
  return app_path.c_str();
}


#if defined (__linux__) || defined (__APPLE__)

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h> 

#include <glib/gutils.h>

const char* LINK_NAME =
#if defined (__linux__)
  "/proc/self/exe"
#else
  "/proc/curproc/file"
#endif
;

/// brief Returns the filename of the executable belonging to the current process, or 0 if not found.
char* getexename(char *buf)
{
  /* Now read the symbolic link */
  int ret = readlink(LINK_NAME, buf, PATH_MAX);

  if(ret == -1)
  {
    globalOutputStream() << "getexename: falling back to argv[0]: " << makeQuoted(g_argv[0]);
    const char* path = realpath(g_argv[0], buf);
    if(path == 0)
    {
      /* In case of an error, leave the handling up to the caller */
      return "";
    }
  }

  /* Ensure proper NUL termination */
  buf[ret] = 0;

  /* delete the program name */
  *(strrchr(buf, '/')) = '\0';

  // NOTE: we build app path with a trailing '/'
  // it's a general convention in Radiant to have the slash at the end of directories
  if (buf[strlen(buf)-1] != '/')
  {
    strcat(buf, "/");
  }

  return buf;
}

void environment_init(int argc, char* argv[])
{
  // Give away unnecessary root privileges.
  // Important: must be done before calling gtk_init().
  char *loginname;
  struct passwd *pw;
  seteuid(getuid());
  if (geteuid() == 0 && (loginname = getlogin()) != 0 &&
      (pw = getpwnam(loginname)) != 0)
    setuid(pw->pw_uid);

  args_init(argc, argv);

  {
    StringOutputStream home(256);
    home << DirectoryCleaned(g_get_home_dir()) << ".radiant/";
    Q_mkdir(home.c_str());
    home_path = home.c_str();
  }
  {
    char real[PATH_MAX];
    app_path = getexename(real);
    ASSERT_MESSAGE(!string_empty(app_path.c_str()), "failed to deduce app path");
  }
}

#endif

#ifdef WIN32

#include <windows.h>
#include <shfolder.h>

void environment_init(int argc, char* argv[])
{
  args_init(argc, argv);

  {
    char appdata[MAX_PATH+1];
    SHGetFolderPath(0, CSIDL_APPDATA, 0, 0, appdata);

    StringOutputStream home(256);
    if(string_empty(appdata))
    {
      ERROR_MESSAGE("Application Data folder not available.\n"
        "Please install shfolder redistributable package.\n"
        "Radiant will use C:\\ for user preferences.\n");
      home << "C:";
    }
    else
    {
      home << PathCleaned(appdata);
    }
    home << "/RadiantSettings/";
    Q_mkdir(home.c_str());
    home_path = home.c_str();
  }
  {
    // get path to the editor
    char filename[MAX_PATH+1];
    GetModuleFileName(0, filename, MAX_PATH);
    char* last_separator = strrchr(filename, '\\');
    if(last_separator != 0)
    {
      *(last_separator+1) = '\0';
    }
    else
    {
      filename[0] = '\0';
    }
    StringOutputStream app(256);
    app << PathCleaned(filename);
    app_path = app.c_str();
  }
}

#endif
