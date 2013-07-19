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

#ifndef __QEDEFS_H__
#define __QEDEFS_H__

#define _3DFXCAMERA_WINDOW_CLASS    "Q3DFXCamera"
#define CAMERA_WINDOW_CLASS "QCamera"
#define XY_WINDOW_CLASS     "QXY"
#define Z_WINDOW_CLASS      "QZ"
#define ENT_WINDOW_CLASS    "QENT"
#define TEXTURE_WINDOW_CLASS    "QTEX"

#define ZWIN_WIDTH  40
#define CWIN_SIZE   ( 0.4 )

#define MAX_EDGES   512
#define MAX_POINTS  1024

#define CMD_TEXTUREWAD  60000
#define CMD_BSPCOMMAND  62000

#define PITCH   0
#define YAW     1
#define ROLL    2

#define QE_TIMER0   1

#define PLANE_X     0
#define PLANE_Y     1
#define PLANE_Z     2
#define PLANE_ANYX  3
#define PLANE_ANYY  4
#define PLANE_ANYZ  5

#define ON_EPSILON  0.01

#define KEY_FORWARD     1
#define KEY_BACK        2
#define KEY_TURNLEFT    4
#define KEY_TURNRIGHT   8
#define KEY_LEFT        16
#define KEY_RIGHT       32
#define KEY_LOOKUP      64
#define KEY_LOOKDOWN    128
#define KEY_UP          256
#define KEY_DOWN        512

// xy.c
#define EXCLUDE_WORLD           0x00000001
#define EXCLUDE_ENT             0x00000002
#define EXCLUDE_CURVES          0x00000004
#define EXCLUDE_TRANSLUCENT     0x00000008
#define EXCLUDE_LIQUIDS         0x00000010
#define EXCLUDE_CAULK           0x00000020
#define EXCLUDE_CLIP            0x00000040
#define EXCLUDE_PATHS           0x00000080
#define EXCLUDE_LIGHTS          0x00000100
#define EXCLUDE_DETAILS         0x00000200
#define EXCLUDE_HINTSSKIPS      0x00000400
#define EXCLUDE_MODELS          0x00000800
#define EXCLUDE_AREAPORTALS     0x00001000
#define EXCLUDE_TRIGGERS        0x00002000
#define EXCLUDE_CLUSTERPORTALS  0x00004000
#define EXCLUDE_TERRAIN         0x00008000
#define EXCLUDE_LIGHTGRID       0x00010000
#define EXCLUDE_STRUCTURAL      0x00020000
#define EXCLUDE_BOTCLIP           0x00040000

#define INCLUDE_EASY            0x00000001
#define INCLUDE_NORMAL          0x00000002
#define INCLUDE_HARD            0x00000004
#define INCLUDE_DEATHMATCH      0x00000008
#define INCLUDE_NAMES           0x00000010
#define INCLUDE_COORDS          0x00000020
#define INCLUDE_BLOCKS          0x00000040
#define INCLUDE_ANGLES          0x00000080
#define INCLUDE_PATCHBBOXES     0x00000100
#define INCLUDE_PATCHWIREFRAME  0x00000200
#define INCLUDE_CAMERATINT      0x00000400
#define INCLUDE_MODELBOXONLY    0x00000800

//
// menu indexes for modifying menus
//
#define MENU_VIEW       2
#define MENU_BSP        4
#define MENU_TEXTURE    6
#define MENU_PLUGIN 11

// odd things not in windows header...
#define VK_COMMA        188
#define VK_PERIOD       190

// ShowEntitiesAs flags
// used in camera code, not menus
#define ENTITY_WIREFRAME        0x00001
#define ENTITY_SKIN_MODEL       0x00010
#define ENTITY_SELECTED_ONLY    0x00100
#define ENTITY_BOXED            0x01000

// ShowEntitiesAs menu settings .. combinations of the above settings
#define ENTITY_BOX              0x01000
#define ENTITY_WIRE             0x00001
#define ENTITY_SELECTED         0x00101
#define ENTITY_SKINNED          0x00010
#define ENTITY_SKINNED_BOXED    0x01010
#define ENTITY_SELECTED_SKIN    0x00110

#endif
