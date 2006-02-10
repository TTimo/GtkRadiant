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

#if !defined(INCLUDED_IIMAGE_H)
#define INCLUDED_IIMAGE_H

#include "generic/constant.h"

typedef unsigned char byte;

class Image
{
public:
  virtual void release() = 0;
  virtual byte* getRGBAPixels() const = 0;
  virtual unsigned int getWidth() const = 0;
  virtual unsigned int getHeight() const = 0;

  virtual int getSurfaceFlags() const
  {
    return 0;
  }
  virtual int getContentFlags() const
  {
    return 0;
  }
  virtual int getValue() const
  {
    return 0;
  }
};

class ArchiveFile;

struct _QERPlugImageTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "image");

  /// Read an image from the file.
  /// Returns 0 if the image could not be read.
  Image* (*loadImage)(ArchiveFile& file);
};

template<typename Type>
class ModuleRef;
typedef ModuleRef<_QERPlugImageTable> ImageModuleRef;

template<typename Type>
class Modules;
typedef Modules<_QERPlugImageTable> ImageModules;

template<typename Type>
class ModulesRef;
typedef ModulesRef<_QERPlugImageTable> ImageModulesRef;

#endif // _IIMAGE_H
