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

#if !defined(INCLUDED_OS_FILE_H)
#define INCLUDED_OS_FILE_H

/// \file
/// \brief OS file-system querying and manipulation.

#if defined( WIN32 )
#define S_ISDIR(mode) (mode & _S_IFDIR)
#include <io.h> // _access()
#define F_OK 0x00
#define W_OK 0x02
#define R_OK 0x04
#define access(path, mode) _access(path, mode)
#else
#include <unistd.h> // access()
#endif

#include <stdio.h> // rename(), remove()
#include <sys/stat.h> // stat()
#include <sys/types.h> // this is included by stat.h on win32
#include <cstddef>
#include <ctime>

#include "debugging/debugging.h"

/// \brief Attempts to move the file identified by \p from to \p to and returns true if the operation was successful.
///
/// The operation will fail unless:
/// - The path \p from identifies an existing file which is accessible for writing.
/// - The directory component of \p from identifies an existing directory which is accessible for writing.
/// - The path \p to does not identify an existing file or directory.
/// - The directory component of \p to identifies an existing directory which is accessible for writing.
inline bool file_move(const char* from, const char* to)
{
  ASSERT_MESSAGE(from != 0 && to != 0, "file_move: invalid path");
  return rename(from, to) == 0;
}

/// \brief Attempts to remove the file identified by \p path and returns true if the operation was successful.
///
/// The operation will fail unless:
/// - The \p path identifies an existing file.
/// - The parent-directory component of \p path identifies an existing directory which is accessible for writing.
inline bool file_remove(const char* path)
{
  ASSERT_MESSAGE(path != 0, "file_remove: invalid path");
  return remove(path) == 0;
}

namespace FileAccess
{
  enum Mode
  {
    Read = R_OK,
    Write = W_OK,
    ReadWrite = Read | Write,
    Exists = F_OK
  };
}

/// \brief Returns true if the file or directory identified by \p path exists and/or may be accessed for reading, writing or both, depending on the value of \p mode.
inline bool file_accessible(const char* path, FileAccess::Mode mode)
{
  ASSERT_MESSAGE(path != 0, "file_accessible: invalid path");
  return access(path, static_cast<int>(mode)) == 0;
}

/// \brief Returns true if the file or directory identified by \p path exists and may be opened for reading.
inline bool file_readable(const char* path)
{
  return file_accessible(path, FileAccess::Read);
}

/// \brief Returns true if the file or directory identified by \p path exists and may be opened for writing.
inline bool file_writeable(const char* path)
{
  return file_accessible(path, FileAccess::Write);
}

/// \brief Returns true if the file or directory identified by \p path exists.
inline bool file_exists(const char* path)
{
  return file_accessible(path, FileAccess::Exists);
}

/// \brief Returns true if the file or directory identified by \p path exists and is a directory.
inline bool file_is_directory(const char* path)
{
  ASSERT_MESSAGE(path != 0, "file_is_directory: invalid path");
  struct stat st;
  if(stat(path, &st) == -1)
  {
    return false;
  }
  return S_ISDIR (st.st_mode) != 0;
}

typedef std::size_t FileSize;

/// \brief Returns the size in bytes of the file identified by \p path, or 0 if the file was not found.
inline FileSize file_size(const char* path)
{
  ASSERT_MESSAGE(path != 0, "file_size: invalid path");
  struct stat st;
  if(stat(path, &st) == -1)
  {
    return 0;
  }
  return st.st_size;
}

/// Seconds elapsed since Jan 1, 1970
typedef std::time_t FileTime;
/// No file can have been modified earlier than this time.
const FileTime c_invalidFileTime = -1;

/// \brief Returns the time that the file identified by \p path was last modified, or c_invalidFileTime if the file was not found.
inline FileTime file_modified(const char* path)
{
  ASSERT_MESSAGE(path != 0, "file_modified: invalid path");
  struct stat st;
  if(stat(path, &st) == -1)
  {
    return c_invalidFileTime;
  }
  return st.st_mtime;
}



#endif
