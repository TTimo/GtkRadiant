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

/*!
   This header is used to make sure the STL we are using is what we expect
   this allows to catch some weird errors early at compile time
 */

// TODO: finish complete cleanup and removal of STLPort usage traces
#ifndef Q_NO_STLPORT
#define Q_NO_STLPORT
#endif

#ifdef Q_NO_STLPORT

// not using STLPort (gcc 3.x build)
using namespace std;

#else

#ifndef _STLPORT_VERSION
#error "Can't find _STLPORT_VERSION, check you are compiling against STLPort"
#endif

#if !defined( _STLP_DONT_USE_EXCEPTIONS )
#error exc
#endif

#if !defined( _STLP_NO_NAMESPACES )
#error namespace
#endif

#if !defined( _STLP_NO_IOSTREAMS )
#error io
#endif

// now check a few more things (paranoid)
// if you use our custom STLPort distribution it should be alright though
#if !defined( _STLP_DONT_USE_EXCEPTIONS ) || !defined( _STLP_NO_NAMESPACES ) || !defined( _STLP_NO_IOSTREAMS )
#error "There is something broken in your STLPort config"
#endif

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#endif
