/*
Copyright (C) 2002 Dominic Clifton.

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
// Sprite Model Plugin
//
// Code by Hydra aka Dominic Clifton
//
// Based on MD3Model source code by SPoG
//

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "mathlib.h"
#include <string.h>
#include "qertypes.h"
#include <stdio.h>

#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
extern _QERFuncTable_1 __QERTABLENAME;

#define USE_QGLTABLE_DEFINE
#include "igl.h"
extern OpenGLBinding __QGLTABLENAME;

#include "imodel.h"

#endif // _PLUGIN_H_
