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

#if !defined (INCLUDED_TEXTURELIB_H)
#define INCLUDED_TEXTURELIB_H

#include "generic/vector.h"
typedef Vector3 Colour3;
typedef unsigned int GLuint;
class LoadImageCallback;

// describes a GL texture
struct qtexture_t
{
  qtexture_t(const LoadImageCallback& load, const char* name) : load(load), name(name)
  {
  }
  const LoadImageCallback& load;
  const char* name;
  std::size_t width, height;
  GLuint texture_number; // gl bind number
  Colour3 color; // for flat shade mode
  int surfaceFlags, contentFlags, value;
};

#endif
