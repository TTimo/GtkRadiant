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

#include "archive.h"

#include "idatastream.h"
#include "iarchive.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "stream/filestream.h"
#include "stream/textfilestream.h"
#include "string/string.h"
#include "os/path.h"
#include "os/file.h"
#include "os/dir.h"
#include "archivelib.h"
#include "fs_path.h"

#include "vfspk3.h"


class DirectoryArchive : public Archive
{
  CopiedString m_root;
public:
  DirectoryArchive(const char* root) : m_root(root)
  {
  }

  void release()
  {
    delete this;
  }
  virtual ArchiveFile* openFile(const char* name)
  {
    UnixPath path(m_root.c_str());
    path.push_filename(name);
    DirectoryArchiveFile* file = new DirectoryArchiveFile(name, path.c_str());
    if(!file->failed())
    {
      return file;
    }
    file->release();
    return 0;
  }
  virtual ArchiveTextFile* openTextFile(const char* name)
  {
    UnixPath path(m_root.c_str());
    path.push_filename(name);
    DirectoryArchiveTextFile* file = new DirectoryArchiveTextFile(name, path.c_str());
    if(!file->failed())
    {
      return file;
    }
    file->release();
    return 0;
  }
  virtual bool containsFile(const char* name)
  {
    UnixPath path(m_root.c_str());
    path.push_filename(name);
    return file_readable(path.c_str());
  }
  virtual void forEachFile(VisitorFunc visitor, const char* root)
  {
    std::vector<Directory*> dirs;
    UnixPath path(m_root.c_str());
    path.push(root);
    dirs.push_back(directory_open(path.c_str()));

    while(!dirs.empty() && directory_good(dirs.back()))
    {
      const char* name = directory_read_and_increment(dirs.back());

      if(name == 0)
      {
        directory_close(dirs.back());
        dirs.pop_back();
        path.pop();
      }
      else if(!string_equal(name, ".") && !string_equal(name, ".."))
      {
        path.push_filename(name);

        bool is_directory = file_is_directory(path.c_str());

        if(!is_directory)
          visitor.file(path_make_relative(path.c_str(), m_root.c_str()));

        path.pop();

        if(is_directory)
        {
          path.push(name);

          if(!visitor.directory(path_make_relative(path.c_str(), m_root.c_str()), dirs.size()))
            dirs.push_back(directory_open(path.c_str()));
          else
            path.pop();
        }
      }
    }
  }
};

Archive* OpenArchive(const char* name)
{
  return new DirectoryArchive(name);
}

#if 0

class TestArchive
{
  class TestVisitor : public Archive::IVisitor
  {
  public:
    virtual void visit(const char* name)
    {
      int bleh = 0;
    }
  };
public:
  void test1()
  {
    Archive* archive = OpenArchive("d:/quake/id1/");
    ArchiveFile* file = archive->openFile("quake101.wad");
    if(file != 0)
    {
      char buffer[1024];
      file->getInputStream().read(buffer, 1024);
      file->release();
    }
    TestVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 0), "");
    archive->release();
  }
  void test2()
  {
    Archive* archive = OpenArchive("d:/gtkradiant_root/baseq3/");
    TestVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 2), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFiles, 1), "textures");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eDirectories, 1), "textures");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "textures");
    archive->release();
  }
  TestArchive()
  {
    test1();
    test2();
  }
};

TestArchive g_test;

#endif
