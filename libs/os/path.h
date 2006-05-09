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

#if !defined (INCLUDED_OS_PATH_H)
#define INCLUDED_OS_PATH_H

/// \file
/// \brief OS file-system path comparison, decomposition and manipulation.
///
/// - Paths are c-style null-terminated-character-arrays.
/// - Path separators must be forward slashes (unix style).
/// - Directory paths must end in a separator.
/// - Paths must not contain the ascii characters \\ : * ? " < > or |.
/// - Paths may be encoded in UTF-8 or any extended-ascii character set.

#include "string/string.h"

#if defined(WIN32)
#define OS_CASE_INSENSITIVE
#endif

/// \brief Returns true if \p path is lexicographically sorted before \p other.
/// If both \p path and \p other refer to the same file, neither will be sorted before the other.
/// O(n)
inline bool path_less(const char* path, const char* other)
{
#if defined(OS_CASE_INSENSITIVE)
  return string_less_nocase(path, other);
#else
  return string_less(path, other);
#endif
}

/// \brief Returns <0 if \p path is lexicographically less than \p other.
/// Returns >0 if \p path is lexicographically greater than \p other.
/// Returns 0 if both \p path and \p other refer to the same file.
/// O(n)
inline int path_compare(const char* path, const char* other)
{
#if defined(OS_CASE_INSENSITIVE)
  return string_compare_nocase(path, other);
#else
  return string_compare(path, other);
#endif
}

/// \brief Returns true if \p path and \p other refer to the same file or directory.
/// O(n)
inline bool path_equal(const char* path, const char* other)
{
#if defined(OS_CASE_INSENSITIVE)
  return string_equal_nocase(path, other);
#else
  return string_equal(path, other);
#endif
}

/// \brief Returns true if the first \p n bytes of \p path and \p other form paths that refer to the same file or directory.
/// If the paths are UTF-8 encoded, [\p path, \p path + \p n) must be a complete path.
/// O(n)
inline bool path_equal_n(const char* path, const char* other, std::size_t n)
{
#if defined(OS_CASE_INSENSITIVE)
  return string_equal_nocase_n(path, other, n);
#else
  return string_equal_n(path, other, n);
#endif
}


/// \brief Returns true if \p path is a fully qualified file-system path.
/// O(1)
inline bool path_is_absolute(const char* path)
{
#if defined(WIN32)
  return path[0] == '/'
    || (path[0] != '\0' && path[1] == ':'); // local drive
#elif defined(POSIX)
  return path[0] == '/';
#endif
}

/// \brief Returns true if \p path is a directory.
/// O(n)
inline bool path_is_directory(const char* path)
{
  std::size_t length = strlen(path);
  if(length > 0)
  {
    return path[length-1] == '/';
  }
  return false;
}

/// \brief Returns a pointer to the first character of the component of \p path following the first directory component.
/// O(n)
inline const char* path_remove_directory(const char* path)
{
  const char* first_separator = strchr(path, '/');
  if(first_separator != 0)
  {
    return ++first_separator;
  }
  return "";
}

/// \brief Returns a pointer to the first character of the filename component of \p path.
/// O(n)
inline const char* path_get_filename_start(const char* path)
{
  {
    const char* last_forward_slash = strrchr(path, '/');
    if(last_forward_slash != 0)
    {
      return last_forward_slash + 1;
    }
  }

  // not strictly necessary,since paths should not contain '\'
  {
    const char* last_backward_slash = strrchr(path, '\\');
    if(last_backward_slash != 0)
    {
      return last_backward_slash + 1;
    }
  }

  return path;
}

/// \brief Returns a pointer to the character after the end of the filename component of \p path - either the extension separator or the terminating null character.
/// O(n)
inline const char* path_get_filename_base_end(const char* path)
{
  const char* last_period = strrchr(path_get_filename_start(path), '.');
  return (last_period != 0) ? last_period : path + string_length(path);
}

/// \brief Returns the length of the filename component (not including extension) of \p path.
/// O(n)
inline std::size_t path_get_filename_base_length(const char* path)
{
  return path_get_filename_base_end(path) - path;
}

/// \brief If \p path is a child of \p base, returns the subpath relative to \p base, else returns \p path.
/// O(n)
inline const char* path_make_relative(const char* path, const char* base)
{
  const std::size_t length = string_length(base);
  if(path_equal_n(path, base, length))
  {
    return path + length;
  }
  return path;
}

/// \brief Returns a pointer to the first character of the file extension of \p path, or "" if not found.
/// O(n)
inline const char* path_get_extension(const char* path)
{
  const char* last_period = strrchr(path_get_filename_start(path), '.');
  if(last_period != 0)
  {
    return ++last_period;
  }
  return "";
}

/// \brief Returns true if \p extension is of the same type as \p other.
/// O(n)
inline bool extension_equal(const char* extension, const char* other)
{
  return path_equal(extension, other);
}

template<typename Functor>
class MatchFileExtension
{
  const char* m_extension;
  const Functor& m_functor;
public:
  MatchFileExtension(const char* extension, const Functor& functor) : m_extension(extension), m_functor(functor)
  {
  }
  void operator()(const char* name) const
  {
    const char* extension = path_get_extension(name);
    if(extension_equal(extension, m_extension))
    {
      m_functor(name);
    }
  }
};

/// \brief A functor which invokes its contained \p functor if the \p name argument matches its \p extension.
template<typename Functor>
inline MatchFileExtension<Functor> matchFileExtension(const char* extension, const Functor& functor)
{
  return MatchFileExtension<Functor>(extension, functor);
}

class PathCleaned
{
public:
  const char* m_path;
  PathCleaned(const char* path) : m_path(path)
  {
  }
};

/// \brief Writes \p path to \p ostream with dos-style separators replaced by unix-style separators.
template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const PathCleaned& path)
{
  const char* i = path.m_path;
  for(; *i != '\0'; ++i)
  {
    if(*i == '\\')
    {
      ostream << '/';
    }
    else
    {
      ostream << *i;
    }
  }
  return ostream;
}

class DirectoryCleaned
{
public:
  const char* m_path;
  DirectoryCleaned(const char* path) : m_path(path)
  {
  }
};

/// \brief Writes \p path to \p ostream with dos-style separators replaced by unix-style separators, and appends a separator if necessary.
template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const DirectoryCleaned& path)
{
  const char* i = path.m_path;
  for(; *i != '\0'; ++i)
  {
    if(*i == '\\')
    {
      ostream << '/';
    }
    else
    {
      ostream << *i;
    }
  }
  char c = *(i - 1);
  if(c != '/' && c != '\\')
  {
    ostream << '/';
  }
  return ostream;
}


#endif
