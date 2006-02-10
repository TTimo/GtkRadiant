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

#if !defined(INCLUDED_IARCHIVE_H)
#define INCLUDED_IARCHIVE_H

#include <cstddef>
#include "generic/constant.h"

class InputStream;

/// \brief A file opened in binary mode.
class ArchiveFile
{
public:
  /// \brief Destroys the file object.
  virtual void release() = 0;
  /// \brief Returns the size of the file data in bytes.
  virtual std::size_t size() const = 0;
  /// \brief Returns the path to this file (relative to the filesystem root)
  virtual const char* getName() const = 0;
  /// \brief Returns the stream associated with this file.
  /// Subsequent calls return the same stream.
  /// The stream may be read forwards until it is exhausted.
  /// The stream remains valid for the lifetime of the file.
  virtual InputStream& getInputStream() = 0;
};

class TextInputStream;

/// \brief A file opened in text mode.
class ArchiveTextFile
{
public:
  /// \brief Destroys the file object.
  virtual void release() = 0;
  /// \brief Returns the stream associated with this file.
  /// Subsequent calls return the same stream.
  /// The stream may be read forwards until it is exhausted.
  /// The stream remains valid for the lifetime of the file.
  virtual TextInputStream& getInputStream() = 0;
};

class ScopedArchiveFile
{
  ArchiveFile& m_file;
public:
  ScopedArchiveFile(ArchiveFile& file) : m_file(file)
  {
  }
  ~ScopedArchiveFile()
  {
    m_file.release();
  }
};

class CustomArchiveVisitor;

class Archive
{
public:

  class Visitor
  {
  public:
    virtual void visit(const char* name) = 0;
  };

  typedef CustomArchiveVisitor VisitorFunc;

  enum EMode
  {
    eFiles = 0x01,
    eDirectories = 0x02,
    eFilesAndDirectories = 0x03,
  };

  /// \brief Destroys the archive object.
  /// Any unreleased file object associated with the archive remains valid. */
  virtual void release() = 0;
  /// \brief Returns a new object associated with the file identified by \p name, or 0 if the file cannot be opened.
  /// Name comparisons are case-insensitive.
  virtual ArchiveFile* openFile(const char* name) = 0;
  /// \brief Returns a new object associated with the file identified by \p name, or 0 if the file cannot be opened.
  /// Name comparisons are case-insensitive.
  virtual ArchiveTextFile* openTextFile(const char* name) = 0;
  /// Returns true if the file identified by \p name can be opened.
  /// Name comparisons are case-insensitive.
  virtual bool containsFile(const char* name) = 0;
  /// \brief Performs a depth-first traversal of the archive tree starting at \p root.
  /// Traverses the entire tree if \p root is "".
  /// When a file is encountered, calls \c visitor.file passing the file name.
  /// When a directory is encountered, calls \c visitor.directory passing the directory name.
  /// Skips the directory if \c visitor.directory returned true.
  /// Root comparisons are case-insensitive.
  /// Names are mixed-case.
  virtual void forEachFile(VisitorFunc visitor, const char* root) = 0;
};

class CustomArchiveVisitor
{
  Archive::Visitor* m_visitor;
  Archive::EMode m_mode;
  std::size_t m_depth;
public:
  CustomArchiveVisitor(Archive::Visitor& visitor, Archive::EMode mode, std::size_t depth)
    : m_visitor(&visitor), m_mode(mode), m_depth(depth)
  {
  }
  void file(const char* name)
  {
    if((m_mode & Archive::eFiles) != 0)
      m_visitor->visit(name);
  }
  bool directory(const char* name, std::size_t depth)
  {
    if((m_mode & Archive::eDirectories) != 0)
      m_visitor->visit(name);
    if(depth == m_depth)
      return true;
    return false;
  }
};

typedef Archive* (*PFN_OPENARCHIVE) (const char* name);

class _QERArchiveTable
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "archive");

  PFN_OPENARCHIVE              m_pfnOpenArchive;
};

template<typename Type>
class Modules;
typedef Modules<_QERArchiveTable> ArchiveModules;

template<typename Type>
class ModulesRef;
typedef ModulesRef<_QERArchiveTable> ArchiveModulesRef;

#endif
