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
// q2map.h

/* platform-specific */
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	#define Q_UNIX
#endif

#ifdef Q_UNIX
	#include <unistd.h>
	#include <pwd.h>
	#include <limits.h>
#endif

#ifdef WIN32
	#include <windows.h>
#endif

#include <stdlib.h>

#include "cmdlib.h"
#include "mathlib.h"
#include "scriplib.h"
#include "polylib.h"
#include "q2_threads.h"
#include "bspfile.h"
#include "inout.h"

int BSP_Main();
int VIS_Main();
int RAD_Main();
