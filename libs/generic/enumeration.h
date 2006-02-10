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

#if !defined(INCLUDED_GENERIC_ENUMERATION_H)
#define INCLUDED_GENERIC_ENUMERATION_H

/// \file
/// \brief Type safe enumeration.

/// \brief An enumerated value.
///
/// - Can be forward-declared when the definition of Enumeration is unknown.
/// - Can only be constructed from valid enumerated values.
/// - Can only be compared with others of the same type.
///
/// \param Enumeration A type that contains an enum \c Value of the allowed values of the enumeration.
template<typename Enumeration>
class EnumeratedValue : public Enumeration
{
  typename Enumeration::Value m_value;
public:
  explicit EnumeratedValue(typename Enumeration::Value value) : m_value(value)
  {
  }
  typename Enumeration::Value get() const
  {
    return m_value;
  }
};

template<typename Enumeration>
inline bool operator==(EnumeratedValue<Enumeration> self, EnumeratedValue<Enumeration> other)
{
  return self.get() == other.get();
}
template<typename Enumeration>
inline bool operator!=(EnumeratedValue<Enumeration> self, EnumeratedValue<Enumeration> other)
{
  return !operator==(self, other);
}

#endif
