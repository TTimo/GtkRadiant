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

#if !defined(INCLUDED_MATH_PI_H)
#define INCLUDED_MATH_PI_H

/// \file
/// \brief Pi constants and degrees/radians conversion.

const double c_pi = 3.1415926535897932384626433832795;
const double c_half_pi = c_pi / 2;
const double c_2pi = 2 * c_pi;
const double c_inv_2pi = 1 / c_2pi;

const double c_DEG2RADMULT = c_pi / 180.0;
const double c_RAD2DEGMULT = 180.0 / c_pi;

inline double radians_to_degrees(double radians)
{
  return radians * c_RAD2DEGMULT;
}
inline double degrees_to_radians(double degrees)
{
  return degrees * c_DEG2RADMULT;
}

#endif
