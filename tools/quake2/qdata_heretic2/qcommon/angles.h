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

// Angles in radians

#define ANGLE_0         0.0F
#define ANGLE_1         0.017453292F
#define ANGLE_5         0.087266462F
#define ANGLE_10        0.174532925F
#define ANGLE_15        0.261799387F
#define ANGLE_20        0.392699081F
#define ANGLE_30        0.523598775F
#define ANGLE_45        0.785398163F
#define ANGLE_60        1.047197551F
#define ANGLE_72        1.256637061F
#define ANGLE_90        1.570796327F
#define ANGLE_120       2.094395102F
#define ANGLE_135       2.35619449F
#define ANGLE_144       2.513274123F
#define ANGLE_180       3.141592653F
#define ANGLE_225       3.926990817F
#define ANGLE_270       4.71238898F
#define ANGLE_315       5.497787144F
#define ANGLE_360       6.283185307F

// Angles in degrees

#define DEGREE_0        0.0F
#define DEGREE_180      180.0F
#define DEGREE_45       ( DEGREE_180 / 4.0F )
#define DEGREE_90       ( DEGREE_180 / 2.0F )
#define DEGREE_135      ( DEGREE_90 + DEGREE_45 )
#define DEGREE_270      ( DEGREE_180 + DEGREE_90 )
#define DEGREE_360      ( DEGREE_180 * 2.0F )

#define DEGREE_225      ( DEGREE_180 + DEGREE_45 )
#define DEGREE_315      ( DEGREE_270 + DEGREE_45 )

#define DEGREE_30       ( DEGREE_180 / 6.0F )
#define DEGREE_60       ( DEGREE_180 / 3.0F )
#define DEGREE_120      ( DEGREE_360 / 3.0F )

#define DEGREE_1        ( DEGREE_180 / 180.0F )
#define DEGREE_5        ( DEGREE_180 / 36.0F )
#define DEGREE_10       ( DEGREE_180 / 18.0F )
#define DEGREE_15       ( DEGREE_180 / 12.0F )
#define DEGREE_20       ( DEGREE_180 / 8.0F )

// Conversion routines

#define ANGLE_TO_RAD    ANGLE_1
#define RAD_TO_ANGLE    ( 180.0F / ANGLE_180 )

#define SHORT_TO_ANGLE  ( 360.0 / 65536 )


#pragma warning(disable : 4305)     // 'initializing' : truncation from 'const double ' to 'float '
