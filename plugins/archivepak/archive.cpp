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
#include "cmdlib.h"
#include "bytestreamutils.h"
#include <algorithm>
#include "stream/filestream.h"

#include "iarchive.h"

#include "archivelib.h"


#include "plugin.h"

#include <map>
#include "string/string.h"
#include "fs_filesystem.h"

inline void buffer_findreplace(char* buffer, char find, char replace)
{
  while(*buffer != '\0')
  {
    if(*buffer == find)
      *buffer = replace;
    ++buffer;
  }
}

#include "pak.h"

class PakArchive : public Archive
{
  class PakRecord
  {
  public:
    PakRecord(unsigned int position, unsigned int stream_size)
      : m_position(position), m_stream_size(stream_size)
    {
    }
    unsigned int m_position;
    unsigned int m_stream_size;
  };
  typedef GenericFileSystem<PakRecord> PakFileSystem;
  PakFileSystem m_filesystem;
  FileInputStream m_pakfile;
  CopiedString m_name;

public:
  
  PakArchive(const char* name)
    : m_pakfile(name), m_name(name)
  {
    if(!m_pakfile.failed())
    {
      pakheader_t header;

      m_pakfile.read(reinterpret_cast<FileInputStream::byte_type*>(header.magic), 4);
      header.diroffset = istream_read_uint32_le(m_pakfile);
      header.dirsize = istream_read_uint32_le(m_pakfile);

      if(strncmp (header.magic, "PACK", 4) == 0)
      {
        m_pakfile.seek(header.diroffset);

        for(unsigned int i = 0; i < header.dirsize; i += sizeof(pakentry_t))
        {
          pakentry_t entry;

          m_pakfile.read(reinterpret_cast<FileInputStream::byte_type*>(entry.filename), 0x38);
          entry.offset = istream_read_uint32_le(m_pakfile);
          entry.size = istream_read_uint32_le(m_pakfile);

          buffer_findreplace(entry.filename, '\\', '/');

          PakFileSystem::entry_type& file = m_filesystem[entry.filename];
          if(!file.is_directory())
          {
            globalOutputStream() << "Warning: pak archive " << makeQuoted(m_name.c_str()) << " contains duplicated file: " << makeQuoted(entry.filename) << "\n";
          }
          else
          {
            file = new PakRecord(entry.offset, entry.size);
          }
        }
      }
    }
  }

  ~PakArchive()
  {
    for(PakFileSystem::iterator i = m_filesystem.begin(); i != m_filesystem.end(); ++i)
      delete i->second.file();
  }
  
  void release()
  {
    delete this;
  }
  ArchiveFile* openFile(const char* name)
  {
    PakFileSystem::iterator i = m_filesystem.find(name);
    if(i != m_filesystem.end() && !i->second.is_directory())
    {
      PakRecord* file = i->second.file();
      return StoredArchiveFile::create(name, m_name.c_str(), file->m_position, file->m_stream_size, file->m_stream_size);
    }
    return 0;
  }
  virtual ArchiveTextFile* openTextFile(const char* name)
  {
    PakFileSystem::iterator i = m_filesystem.find(name);
    if(i != m_filesystem.end() && !i->second.is_directory())
    {
      PakRecord* file = i->second.file();
      return StoredArchiveTextFile::create(name, m_name.c_str(), file->m_position, file->m_stream_size);
    }
    return 0;
  }
  bool containsFile(const char* name)
  {
    PakFileSystem::iterator i = m_filesystem.find(name);
    return i != m_filesystem.end() && !i->second.is_directory();
  }
  void forEachFile(VisitorFunc visitor, const char* root)
  {
    m_filesystem.traverse(visitor, root);
  }
};


Archive* OpenArchive(const char* name)
{
  return new PakArchive(name);
}

#if 0

class TestArchive
{
public:
  TestArchive()
  {
    Archive* archive = OpenArchive("c:/quake3/baseq3/pak0.pak");
    ArchiveFile* file = archive->openFile("gfx/palette.lmp");
    if(file != 0)
    {
      char buffer[1024];
      file->getInputStream().read((InputStream::byte_type*)buffer, 1024);
      file->release();
    }
    archive->release();
  }
};

TestArchive g_test;

#endif

#if 0

class TestArchive
{
  class TestVisitor : public Archive::IVisitor
  {
  public:
    void visit(const char* name)
    {
      int bleh = 0;
    }
  };
public:
  TestArchive()
  {
    {
    Archive* archive = OpenArchive("");
    archive->release();
    }
    {
    Archive* archive = OpenArchive("NONEXISTANTFILE");
    archive->release();
    }
    {
    Archive* archive = OpenArchive("c:/quake/id1/pak0.pak");
    ArchiveFile* file = archive->openFile("gfx/palette.lmp");
    if(file != 0)
    {
      char buffer[1024];
      file->getInputStream().read((InputStream::byte_type*)buffer, 1024);
      file->release();
    }
    TestVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 0), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFiles, 0), "progs/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFiles, 0), "maps/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFiles, 1), "sound/ambience/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "sound/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eDirectories, 1), "sound/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 2), "sound/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 2), "");
    archive->release();
    }
  }
};

TestArchive g_test;

#endif
