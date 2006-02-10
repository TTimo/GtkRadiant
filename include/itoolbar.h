/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#if !defined(INCLUDED_IPLUGTOOLBAR_H)
#define INCLUDED_IPLUGTOOLBAR_H

#include <cstddef>
#include "generic/constant.h"

class IToolbarButton
{
public:
  enum EType
  {
    eSpace,
    eButton,
    eToggleButton,
    eRadioButton,
  };

  virtual const char* getImage() const = 0;
  virtual const char* getText() const = 0;
  virtual const char* getTooltip() const = 0;
  virtual EType getType() const = 0;
  virtual void activate() const = 0;
};

typedef std::size_t  (* PFN_TOOLBARBUTTONCOUNT)();
typedef const IToolbarButton* (* PFN_GETTOOLBARBUTTON)(std::size_t index);

struct _QERPlugToolbarTable
{
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "toolbar");

  PFN_TOOLBARBUTTONCOUNT m_pfnToolbarButtonCount;
  PFN_GETTOOLBARBUTTON   m_pfnGetToolbarButton;
};

template<typename Type>
class Modules;
typedef Modules<_QERPlugToolbarTable> ToolbarModules;

template<typename Type>
class ModulesRef;
typedef ModulesRef<_QERPlugToolbarTable> ToolbarModulesRef;

#endif
