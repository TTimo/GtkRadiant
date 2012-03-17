/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

#ifndef __IPLUGTOOLBAR_H_
#define __IPLUGTOOLBAR_H_

/*
   NOTE: this API requires Gtk
   it's a good practice to avoid putting #include <gtk/gtk.h> here
   in some cases, the compiler will get confused because of 'list' identifiers between Gtk and STL headers
 */

#define TOOLBAR_MAJOR "toolbar"

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

virtual ~IToolbarButton() { }
virtual const char* getImage() const = 0;
virtual const char* getText() const = 0;
virtual const char* getTooltip() const = 0;
virtual EType getType() const = 0;
virtual void activate() const = 0;
};

typedef unsigned int ( *PFN_TOOLBARBUTTONCOUNT )();
typedef const IToolbarButton* ( *PFN_GETTOOLBARBUTTON )( unsigned int index );

struct _QERPlugToolbarTable
{
	int m_nSize;
	PFN_TOOLBARBUTTONCOUNT m_pfnToolbarButtonCount;
	PFN_GETTOOLBARBUTTON m_pfnGetToolbarButton;
};

#endif
