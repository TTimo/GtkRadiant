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

#include "idatastream.h"
#include "cmdlib.h"
#include "bytestreamutils.h"

#include "modulesystem.h"
#include "iarchive.h"

#include <algorithm>
#include "stream/filestream.h"
#include "container/array.h"
#include "archivelib.h"
#include "zlibstream.h"

class DeflatedArchiveFile : public ArchiveFile
{
  CopiedString m_name;
  FileInputStream m_istream;
  SubFileInputStream m_substream;
  DeflatedInputStream m_zipstream;
  FileInputStream::size_type m_size;
public:
  typedef FileInputStream::size_type size_type;
  typedef FileInputStream::position_type position_type;

  DeflatedArchiveFile(const char* name, const char* archiveName, position_type position, size_type stream_size, size_type file_size)
    : m_name(name), m_istream(archiveName), m_substream(m_istream, position, stream_size), m_zipstream(m_substream), m_size(file_size)
  {
  }

  void release()
  {
    delete this;
  }
  size_type size() const
  {
    return m_size;
  }
  const char* getName() const
  {
    return m_name.c_str();
  }
  InputStream& getInputStream()
  {
    return m_zipstream;
  }
};

class DeflatedArchiveTextFile : public ArchiveTextFile
{
  CopiedString m_name;
  FileInputStream m_istream;
  SubFileInputStream m_substream;
  DeflatedInputStream m_zipstream;
  BinaryToTextInputStream<DeflatedInputStream> m_textStream;
public:
  typedef FileInputStream::size_type size_type;
  typedef FileInputStream::position_type position_type;

  DeflatedArchiveTextFile(const char* name, const char* archiveName, position_type position, size_type stream_size)
    : m_name(name), m_istream(archiveName), m_substream(m_istream, position, stream_size), m_zipstream(m_substream), m_textStream(m_zipstream)
  {
  }

  void release()
  {
    delete this;
  }
  TextInputStream& getInputStream()
  {
    return m_textStream;
  }
};

#include "pkzip.h"

#include <map>
#include "string/string.h"
#include "fs_filesystem.h"


class ZipArchive : public Archive
{
  class ZipRecord
  {
  public:
    enum ECompressionMode
    {
      eStored,
      eDeflated,
    };
    ZipRecord(unsigned int position, unsigned int compressed_size, unsigned int uncompressed_size, ECompressionMode mode)
      : m_position(position), m_stream_size(compressed_size), m_file_size(uncompressed_size), m_mode(mode)
    {
    }
    unsigned int m_position;
    unsigned int m_stream_size;
    unsigned int m_file_size;
    ECompressionMode m_mode;
  };

  typedef GenericFileSystem<ZipRecord> ZipFileSystem;
  ZipFileSystem m_filesystem;
  CopiedString m_name;
  FileInputStream m_istream;

  bool read_record()
  {
    zip_magic magic;
    istream_read_zip_magic(m_istream, magic);
    if(!(magic == zip_root_dirent_magic))
    {
      return false;
    }
    zip_version version_encoder;
    istream_read_zip_version(m_istream, version_encoder);
    zip_version version_extract;
    istream_read_zip_version(m_istream, version_extract);
    //unsigned short flags = 
    istream_read_int16_le(m_istream);
    unsigned short compression_mode = istream_read_int16_le(m_istream);
    if(compression_mode != Z_DEFLATED && compression_mode != 0)
    {
      return false;
    }
    zip_dostime dostime;
    istream_read_zip_dostime(m_istream, dostime);
    //unsigned int crc32 = 
    istream_read_int32_le(m_istream);
    unsigned int compressed_size = istream_read_uint32_le(m_istream);
    unsigned int uncompressed_size = istream_read_uint32_le(m_istream);
    unsigned int namelength = istream_read_uint16_le(m_istream);
    unsigned short extras = istream_read_uint16_le(m_istream);
    unsigned short comment = istream_read_uint16_le(m_istream);
    //unsigned short diskstart =
    istream_read_int16_le(m_istream);
    //unsigned short filetype = 
    istream_read_int16_le(m_istream);
    //unsigned int filemode =
    istream_read_int32_le(m_istream);
    unsigned int position = istream_read_int32_le(m_istream);

    Array<char> filename(namelength+1);
    m_istream.read(reinterpret_cast<FileInputStream::byte_type*>(filename.data()), namelength);
    filename[namelength] = '\0';

    m_istream.seek(extras + comment, FileInputStream::cur);

    if(path_is_directory(filename.data()))
    {
      m_filesystem[filename.data()] = 0;
    }
    else
    {
      ZipFileSystem::entry_type& file = m_filesystem[filename.data()];
      if(!file.is_directory())
      {
        globalOutputStream() << "Warning: zip archive " << makeQuoted(m_name.c_str()) << " contains duplicated file: " << makeQuoted(filename.data()) << "\n";
      }
      else
      {
        file = new ZipRecord(position, compressed_size, uncompressed_size, (compression_mode == Z_DEFLATED) ? ZipRecord::eDeflated : ZipRecord::eStored);
      }
    }
    
    return true;
  }

  bool read_pkzip()
  {
    SeekableStream::position_type pos = pkzip_find_disk_trailer(m_istream);
    if(pos != 0)
    {
      zip_disk_trailer disk_trailer;
      m_istream.seek(pos);
      istream_read_zip_disk_trailer(m_istream, disk_trailer);
      if(!(disk_trailer.z_magic == zip_disk_trailer_magic))
      {
        return false;
      }

      m_istream.seek(disk_trailer.z_rootseek);
      for(unsigned int i = 0; i < disk_trailer.z_entries; ++i)
      {
        if(!read_record())
        {
          return false;
        }
      }
      return true;
    }
    return false;
  }
public:
  ZipArchive(const char* name)
    : m_name(name), m_istream(name)
  {
    if(!m_istream.failed())
    {
      if(!read_pkzip())
      {
        globalErrorStream() << "ERROR: invalid zip-file " << makeQuoted(name) << '\n';
      }        
    }
  }
  ~ZipArchive()
  {
    for(ZipFileSystem::iterator i = m_filesystem.begin(); i != m_filesystem.end(); ++i)
    {
      delete i->second.file();
    }
  }

  bool failed()
  {
    return m_istream.failed();
  }

  void release()
  {
    delete this;
  }
  ArchiveFile* openFile(const char* name)
  {
    ZipFileSystem::iterator i = m_filesystem.find(name);
    if(i != m_filesystem.end() && !i->second.is_directory())
    {
      ZipRecord* file = i->second.file();

      m_istream.seek(file->m_position);
      zip_file_header file_header;
      istream_read_zip_file_header(m_istream, file_header);
      if(file_header.z_magic != zip_file_header_magic)
      {
        globalErrorStream() << "error reading zip file " << makeQuoted(m_name.c_str());
        return 0;
      }

      switch(file->m_mode)
      {
      case ZipRecord::eStored:
        return StoredArchiveFile::create(name, m_name.c_str(), m_istream.tell(), file->m_stream_size, file->m_file_size);
      case ZipRecord::eDeflated:
        return new DeflatedArchiveFile(name, m_name.c_str(), m_istream.tell(), file->m_stream_size, file->m_file_size);
      }
    }
    return 0;
  }
  ArchiveTextFile* openTextFile(const char* name)
  {
    ZipFileSystem::iterator i = m_filesystem.find(name);
    if(i != m_filesystem.end() && !i->second.is_directory())
    {
      ZipRecord* file = i->second.file();

      m_istream.seek(file->m_position);
      zip_file_header file_header;
      istream_read_zip_file_header(m_istream, file_header);
      if(file_header.z_magic != zip_file_header_magic)
      {
        globalErrorStream() << "error reading zip file " << makeQuoted(m_name.c_str());
        return 0;
      }

      switch(file->m_mode)
      {
      case ZipRecord::eStored:
        return StoredArchiveTextFile::create(name, m_name.c_str(), m_istream.tell(), file->m_stream_size);
      case ZipRecord::eDeflated:
        return new DeflatedArchiveTextFile(name, m_name.c_str(), m_istream.tell(), file->m_stream_size);
      }
    }
    return 0;
  }
  bool containsFile(const char* name)
  {
    ZipFileSystem::iterator i = m_filesystem.find(name);
    return i != m_filesystem.end() && !i->second.is_directory();
  }
  void forEachFile(VisitorFunc visitor, const char* root)
  {
    m_filesystem.traverse(visitor, root);
  }
};

Archive* OpenArchive(const char* name)
{
  return new ZipArchive(name);
}

#if 0

class TestZip
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
  TestZip()
  {
    testzip("c:/quake3/baseq3/mapmedia.pk3", "textures/radiant/notex.tga");
  }

  void testzip(const char* name, const char* filename)
  {
    Archive* archive = OpenArchive(name);
    ArchiveFile* file = archive->openFile(filename);
    if(file != 0)
    {
      unsigned char buffer[4096];
      std::size_t count = file->getInputStream().read((InputStream::byte_type*)buffer, 4096);
      file->release();
    }
    TestVisitor visitor;
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 0), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFiles, 1), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eDirectories, 1), "");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "textures");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 1), "textures/");
    archive->forEachFile(Archive::VisitorFunc(&visitor, Archive::eFilesAndDirectories, 2), "");
    archive->release();
  }
};

TestZip g_TestZip;

#endif
