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

#if !defined(INCLUDED_ISCRIPLIB_H)
#define INCLUDED_ISCRIPLIB_H

/// \file iscriplib.h
/// \brief Token input/output stream module.

#include <cstddef>
#include "generic/constant.h"

#define	MAXTOKEN	1024

class Tokeniser
{
public:
  virtual void release() = 0;
  virtual void nextLine() = 0;
  virtual const char* getToken() = 0;
  virtual void ungetToken() = 0;
  virtual std::size_t getLine() const = 0;
  virtual std::size_t getColumn() const = 0;
};

class TextInputStream;

class TokenWriter
{
public:
  virtual void release() = 0;
  virtual void nextLine() = 0;
  virtual void writeToken(const char* token) = 0;
  virtual void writeString(const char* string) = 0;
  virtual void writeInteger(int i) = 0;
  virtual void writeUnsigned(std::size_t i) = 0;
  virtual void writeFloat(double f) = 0;
};

class TextOutputStream;

struct _QERScripLibTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "scriptlib");

  Tokeniser& (* m_pfnNewScriptTokeniser)(TextInputStream& istream);
  Tokeniser& (* m_pfnNewSimpleTokeniser)(TextInputStream& istream);
  TokenWriter& (* m_pfnNewSimpleTokenWriter)(TextOutputStream& ostream);
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<_QERScripLibTable> GlobalScripLibModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<_QERScripLibTable> GlobalScripLibModuleRef;

inline _QERScripLibTable& GlobalScriptLibrary()
{
  return GlobalScripLibModule::getTable();
}

#endif
