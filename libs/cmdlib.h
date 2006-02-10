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

//
// start of shared cmdlib stuff
// 

#ifndef __CMDLIB__
#define __CMDLIB__

#include <time.h>


// TTimo started adding portability code:
// return true if spawning was successful, false otherwise
// on win32 we have a bCreateConsole flag to create a new console or run inside the current one
//boolean Q_Exec(const char* pCmd, boolean bCreateConsole);
// execute a system command:
//   cmd: the command to run
//   cmdline: the command line
// NOTE TTimo following are win32 specific:
//   execdir: the directory to execute in
//   bCreateConsole: spawn a new console or not
// return values;
//   if the spawn was fine
//   TODO TTimo add functionality to track the process until it dies

bool Q_Exec(const char *cmd, char *cmdline, const char *execdir, bool bCreateConsole);

// some easy portability crap


#define access_owner_read 0400
#define access_owner_write 0200
#define access_owner_execute 0100
#define access_owner_rw_ 0600
#define access_owner_r_x 0500
#define access_owner__wx 0300
#define access_owner_rwx 0700

#define access_group_read 0040
#define access_group_write 0020
#define access_group_execute 0010
#define access_group_rw_ 0060
#define access_group_r_x 0050
#define access_group__wx 0030
#define access_group_rwx 0070

#define access_others_read 0004
#define access_others_write 0002
#define access_others_execute 0001
#define access_others_rw_ 0006
#define access_others_r_x 0005
#define access_others__wx 0003
#define access_others_rwx 0007


#define access_rwxrwxr_x (access_owner_rwx | access_group_rwx | access_others_r_x)
#define access_rwxrwxrwx (access_owner_rwx | access_group_rwx | access_others_rwx)

// Q_mkdir
// returns true if succeeded in creating directory
#ifdef WIN32
#include <direct.h>
inline bool Q_mkdir(const char* name)
{
  return _mkdir(name) != -1; 
}
#else
#include <sys/stat.h>
inline bool Q_mkdir(const char* name)
{
  return mkdir(name, access_rwxrwxr_x) != -1; 
}
#endif


inline double Sys_DoubleTime(void)
{
  return clock()/ 1000.0;
}



#endif
