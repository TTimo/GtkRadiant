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

#if !defined(INCLUDED_VERSIONLIB_H)
#define INCLUDED_VERSIONLIB_H

#include <cstddef>
#include <string.h>
#include <algorithm>

class Version
{
public:
  int major;
  int minor;
};

inline bool operator<(const Version& version, const Version& other)
{
  return version.major < other.major || (!(other.major < version.major) && version.minor < other.minor);
}

template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& outputStream, const Version& version)
{
  return outputStream << version.major << '.' << version.minor;
}

/// \brief Returns true if \p version (code) is compatible with \p other (data).
inline bool version_compatible(const Version& version, const Version& other)
{
  return version.major == other.major  // different major-versions are always incompatible
    && !(version.minor < other.minor); // data minor-version is incompatible if greater than code minor-version
}

inline int string_range_parse_unsigned_decimal_integer(const char* first, const char* last)
{
  int result = 0;
  for(; first != last; ++first)
  {
    result *= 10;
    result += *first - '0';
  }
  return result;
}

inline Version version_parse(const char* versionString)
{
  Version version;
  const char* endVersion = versionString + strlen(versionString);

  const char* endMajor = strchr(versionString, '.');
  if(endMajor == 0)
  {
    endMajor = endVersion;

    version.minor = 0;
  }
  else
  {
    const char* endMinor = strchr(endMajor + 1, '.');
    if(endMinor == 0)
    {
      endMinor = endVersion;
    }
    version.minor = string_range_parse_unsigned_decimal_integer(endMajor + 1, endMinor);
  }
  version.major = string_range_parse_unsigned_decimal_integer(versionString, endMajor);

  return version;
}

#endif
