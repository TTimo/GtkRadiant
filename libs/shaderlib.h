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

#if !defined (INCLUDED_SHADERLIB_H)
#define INCLUDED_SHADERLIB_H

#include "string/string.h"
#include "character.h"
#include "ishaders.h"

inline bool shader_equal(const char* shader, const char* other)
{
  return string_equal_nocase(shader, other);
}

inline bool shader_equal_n(const char* shader, const char* other, std::size_t n)
{
  return string_equal_nocase_n(shader, other, n);
}

inline bool shader_less(const char* shader, const char* other)
{
  return string_less_nocase(shader, other);
}

inline bool shader_equal_prefix(const char* string, const char* prefix)
{
  return shader_equal_n(string, prefix, string_length(prefix));
}

class shader_less_t
{
public:
  bool operator()(const CopiedString& shader, const CopiedString& other) const
  {
    return shader_less(shader.c_str(), other.c_str());
  }
};

inline bool shader_valid(const char* shader)
{
  return string_is_ascii(shader)
    && strchr(shader, ' ') == 0
    && strchr(shader, '\n') == 0
    && strchr(shader, '\r') == 0
    && strchr(shader, '\t') == 0
    && strchr(shader, '\v') == 0
    && strchr(shader, '\\') == 0;
}

inline const char* GlobalTexturePrefix_get()
{
  return GlobalShaderSystem().getTexturePrefix();
}

inline bool shader_is_texture(const char* name)
{
  return shader_equal_prefix(name, GlobalTexturePrefix_get());
}

inline const char* shader_get_textureName(const char* name)
{
  return name + string_length(GlobalTexturePrefix_get());
}

inline bool texdef_name_valid(const char* name)
{
  return shader_valid(name) && shader_is_texture(name);
}

inline const char* texdef_name_default()
{
  return GlobalTexturePrefix_get();
}


#endif
