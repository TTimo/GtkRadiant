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

#include "wad.h"

class WadArchive : public Archive
{
  class wad_record_t
  {
  public:
    wad_record_t(unsigned int position, unsigned int stream_size, unsigned int file_size)
      : m_position(position), m_stream_size(stream_size), m_file_size(file_size)
    {}
    unsigned int m_position;
    unsigned int m_stream_size;
    unsigned int m_file_size;
  };

  enum EWadVersion
  {
    eNotValid,
    eWAD2,
    eWAD3,
  };

  typedef std::map<CopiedString, wad_record_t, StringLessNoCase> files_t;
  files_t m_files;
  CopiedString m_name;
  FileInputStream m_wadfile;

  EWadVersion wad_version(const char* identification)
  {
    if(strncmp(identification, "WAD2", 4) == 0)
      return eWAD2;
    if(strncmp(identification, "WAD3", 4) == 0)
      return eWAD3;
    return eNotValid;
  }

  const char* type_for_version(EWadVersion version)
  {
    switch(version)
    {
    case eWAD2:
      return ".mip";
    case eWAD3:
      return ".hlw";
    default:
      break;
    }
    return "";
  }

  int miptex_type_for_version(EWadVersion version)
  {
    switch(version)
    {
    case eWAD2:
      return TYP_MIPTEX;
    case eWAD3:
      return 67;
    default:
      break;
    }
    return -1;
  }

public:
  WadArchive(const char* name)
    : m_name(name), m_wadfile(name)
  {
    if(!m_wadfile.failed())
    {
      wadinfo_t wadinfo;
      istream_read_wadinfo(m_wadfile, wadinfo);

      EWadVersion version = wad_version(wadinfo.identification);
      int miptexType = miptex_type_for_version(version);

      if(version != eNotValid)
      {
        m_wadfile.seek(wadinfo.infotableofs);

        for(int i = 0; i < wadinfo.numlumps; ++i)
        {
          char buffer[32];
          lumpinfo_t lumpinfo;
          istream_read_lumpinfo(m_wadfile, lumpinfo);
          if(lumpinfo.type == miptexType)
          {
            strcpy(buffer, "textures/");
            strcat(buffer, lumpinfo.name);
            strcat(buffer, type_for_version(version));
            m_files.insert(files_t::value_type(buffer, wad_record_t(lumpinfo.filepos, lumpinfo.disksize, lumpinfo.size)));
          }
        }
      }
    }
  }

  void release()
  {
    delete this;
  }
  ArchiveFile* openFile(const char* name)
  {
    files_t::iterator i = m_files.find(name);
    if(i != m_files.end())
    {
      return StoredArchiveFile::create(name, m_name.c_str(), i->second.m_position, i->second.m_stream_size, i->second.m_file_size);
    }
    return 0;
  }
  virtual ArchiveTextFile* openTextFile(const char* name)
  {
    files_t::iterator i = m_files.find(name);
    if(i != m_files.end())
    {
      return StoredArchiveTextFile::create(name, m_name.c_str(), i->second.m_position, i->second.m_stream_size);
    }
    return 0;
  }
  bool containsFile(const char* name)
  {
    return m_files.find(name) != m_files.end();
  }
  void forEachFile(VisitorFunc visitor, const char* root)
  {
    if(root[0] == '\0')
    {
      if(visitor.directory("textures/", 1))
        return;
    }
    else if(strcmp(root, "textures/") != 0)
    {
      return;
    }

    for(files_t::iterator i = m_files.begin(); i != m_files.end(); ++i)
      visitor.file(i->first.c_str());
  }
};


Archive* OpenArchive(const char* name)
{
  return new WadArchive(name);
}

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
    Archive* archive = OpenArchive("c:/quake/id1/quake101.wad");
    ArchiveFile* file = archive->openFile("textures/sky1.mip");
    if(file != 0)
    {
      unsigned char* buffer = new unsigned char[file->size()];
      file->getInputStream().read((InputStream::byte_type*)buffer, file->size());
      delete[] buffer;
      file->release();
    }
    TestVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 0), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 0), "textures/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "textures/");
    archive->release();
    }
  }
};

TestArchive g_test;

#endif
