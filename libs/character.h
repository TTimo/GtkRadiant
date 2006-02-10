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

#if !defined(INCLUDED_CHARACTER_H)
#define INCLUDED_CHARACTER_H

/// \file
/// \brief Character encoding.

/// \brief Returns true if \p c is an ASCII character that can be represented with 7 bits.
inline bool char_is_ascii(char c)
{
  return (c & 0x80) == 0;
}

/// \brief Returns true if \p string consists entirely of ASCII characters.
inline bool string_is_ascii(const char* string)
{
  while(*string != '\0')
  {
    if(!char_is_ascii(*string++))
    {
      return false;
    }
  }
  return true;
}

#endif
