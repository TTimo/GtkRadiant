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

#if !defined(INCLUDED_IFILESYSTEM_H)
#define INCLUDED_IFILESYSTEM_H

#include <cstddef>
#include "generic/constant.h"

template<typename FirstArgument>
class Callback1;
typedef Callback1<const char*> ArchiveNameCallback;
typedef Callback1<const char*> FileNameCallback;

class ArchiveFile;
class ArchiveTextFile;
class Archive;

class ModuleObserver;

typedef struct _GSList GSList;

/// The Virtual File System.
class VirtualFileSystem
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "VFS");

  /// \brief Adds a root search \p path.
  /// Called before \c initialise.
  virtual void initDirectory(const char *path) = 0;
  /// \brief Initialises the filesystem.
  /// Called after all root search paths have been added.
  virtual void initialise() = 0;
  /// \brief Shuts down the filesystem.
  virtual void shutdown() = 0;

  /// \brief Returns the file identified by \p filename opened in binary mode, or 0 if not found.
  /// The caller must \c release() the file returned if it is not 0.
  virtual ArchiveFile* openFile(const char* filename) = 0;
  /// \brief Returns the file identified by \p filename opened in text mode, or 0 if not found.
  /// The caller must \c release() the file returned if it is not 0.
  virtual ArchiveTextFile* openTextFile(const char* filename) = 0;

  /// \brief Opens the file identified by \p filename and reads it into \p buffer, or sets *\p buffer to 0 if not found.
  /// Returns the size of the buffer allocated, or undefined value if *\p buffer is 0;
  /// The caller must free the allocated buffer by calling \c freeFile
  /// \deprecated Deprecated - use \c openFile.
  virtual std::size_t loadFile(const char *filename, void **buffer) = 0;
  /// \brief Frees the buffer returned by \c loadFile.
  /// \deprecated Deprecated.
  virtual void freeFile(void *p) = 0;

  /// \brief Calls \p callback for each directory under \p basedir.
  virtual void forEachDirectory(const char* basedir, const FileNameCallback& callback, std::size_t depth = 1) = 0;
  /// \brief Calls \p callback for each file under \p basedir matching \p extension.
  /// Use "*" as \p extension to match all file extensions.
  virtual void forEachFile(const char* basedir, const char* extension, const FileNameCallback& callback, std::size_t depth = 1) = 0;

  /// \brief Returns a list containing the relative names of all the directories under \p basedir.
  /// The caller must free the returned list by calling \c clearFileDirList;
  /// \deprecated Deprecated - use \c forEachDirectory.
  virtual GSList* getDirList(const char *basedir) = 0;
  /// \brief Returns a list containing the relative names of the files under \p basedir (\p extension can be "*" for all files).
  /// The caller must free the returned list by calling \c clearFileDirList.
  /// \deprecated Deprecated - use \c forEachFile.
  virtual GSList* getFileList(const char *basedir, const char *extension) = 0;
  /// \brief Frees the \p list returned from \c getDirList or \c getFileList.
  /// \deprecated Deprecated.
  virtual void clearFileDirList(GSList **list) = 0;

  /// \brief Returns the absolute filename for a relative \p name, or "" if not found.
  virtual const char* findFile(const char* name) = 0;
  /// \brief Returns the filesystem root for an absolute \p name, or "" if not found.
  /// This can be used to convert an absolute name to a relative name.
  virtual const char* findRoot(const char* name) = 0;

  /// \brief Attach an \p observer whose realise() and unrealise() methods will be called when the filesystem is initialised or shut down.
  virtual void attach(ModuleObserver& observer) = 0;
  /// \brief Detach an \p observer previously-attached by calling \c attach.
  virtual void detach(ModuleObserver& observer) = 0;

  virtual Archive* getArchive(const char* archiveName) = 0;
  virtual void forEachArchive(const ArchiveNameCallback& callback) = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<VirtualFileSystem> GlobalFileSystemModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<VirtualFileSystem> GlobalFileSystemModuleRef;

inline VirtualFileSystem& GlobalFileSystem()
{
  return GlobalFileSystemModule::getTable();
}


/// \deprecated Use \c openFile.
inline int vfsLoadFile(const char* filename, void** buffer, int index = 0)
{
	return static_cast<int>(GlobalFileSystem().loadFile(filename, buffer));
};

/// \deprecated Deprecated.
inline void vfsFreeFile(void* p)
{
	GlobalFileSystem().freeFile(p);
}

#endif
