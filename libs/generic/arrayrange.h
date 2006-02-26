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

#if !defined(INCLUDED_GENERIC_ARRAYRANGE_H)
#define INCLUDED_GENERIC_ARRAYRANGE_H

/// \file
/// \brief Macros for automatically converting a compile-time-sized array to a range. 

template<typename Element>
struct ArrayRange
{
  typedef Element* Iterator;
  ArrayRange(Iterator first, Iterator last)
    : first(first), last(last)
  {
  }
  Iterator first;
  Iterator last;
};

template<typename Element>
inline ArrayRange<Element> makeArrayRange(Element* first, Element* last)
{
  return ArrayRange<Element>(first, last);
}

template<typename Element>
struct ArrayConstRange
{
  typedef const Element* Iterator;
  ArrayConstRange(Iterator first, Iterator last)
    : first(first), last(last)
  {
  }
  Iterator first;
  Iterator last;
};

template<typename Element>
inline ArrayConstRange<Element> makeArrayRange(const Element* first, const Element* last)
{
  return ArrayConstRange<Element>(first, last);
}

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#define ARRAY_END(array) (array + ARRAY_SIZE(array))
#define ARRAY_RANGE(array) (makeArrayRange(array, ARRAY_END(array)))


typedef ArrayConstRange<const char*> StringArrayRange;
#define STRING_ARRAY_RANGE(array) (StringArrayRange(array, ARRAY_END(array)))

typedef ArrayRange<const char> StringRange;

#endif
