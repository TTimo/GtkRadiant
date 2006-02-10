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

#include "view.h"

#if defined(DEBUG_CULLING)

#include <stdio.h>

char g_cull_stats[1024];
int g_count_dots;
int g_count_planes;
int g_count_oriented_planes;
int g_count_bboxs;
int g_count_oriented_bboxs;

#endif

void Cull_ResetStats()
{
#if defined(DEBUG_CULLING)
  g_count_dots = 0;
  g_count_planes = 0;
  g_count_oriented_planes = 0;
  g_count_bboxs = 0;
  g_count_oriented_bboxs = 0;
#endif
}


const char* Cull_GetStats()
{
#if defined(DEBUG_CULLING)
  sprintf(g_cull_stats, "dots: %d | planes %d + %d | bboxs %d + %d", g_count_dots, g_count_planes, g_count_oriented_planes, g_count_bboxs, g_count_oriented_bboxs);
  return g_cull_stats;
#else
  return "";
#endif
}
