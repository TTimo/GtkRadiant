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

#include "mathlib.h"

void line_construct_for_vec3(line_t *line, const vec3_t start, const vec3_t end)
{
  VectorMid(start, end, line->origin);
  VectorSubtract(end, line->origin, line->extents);
}

int line_test_plane(const line_t* line, const vec4_t plane)
{
  float fDist;

  // calc distance of origin from plane
  fDist = DotProduct(plane, line->origin) + plane[3];
  
  // accept if origin is less than or equal to this distance
  if (fabs(fDist) < fabs(DotProduct(plane, line->extents))) return 1; // partially inside
  else if (fDist < 0) return 2; // totally inside
  return 0; // totally outside
}
