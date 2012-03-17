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

#ifndef __IREFCOUNT_H__
#define __IREFCOUNT_H__

/*!
   \class IRefCounted
   \brief reference counted objects

   general hints: how to use ref count properly
   we consider ref counting as an extension of new and delete operators
   the main issue is 'when to incref' 'when to decref'
   in most cases connected to function calls
   the general thinking about that:
    - if you get a pointer to a refcounted object through a call to a function, assume that this object has been 'reserved' for you
    already (i.e. allocated if you think this the new/delete way). so if you keep the object, you don't need to incref it, and if
    you don't keep it you need to decref it.
    - if you are called in a function and a refcounted object passed as parameter, then you should assume that this is an optional
    object given to you FYI, which you don't need to decref if you don't keep / need to incref if you keep

   refcount is initialized to 1 in constructor. that serves for static objects and memory allocator
   when you allocate in memory a ref counted object, it's default ref count will be 1, you should never delete it but just call DecRef on it

   define an interface and an implementation macro to make things easier
   NOTE: we may have to provide a static library to go with that
   in case we would move irecount.h out of here into libs/

   \todo functionality needed:
   mostly enable/disable some features with compile time flags (independently from each other as much as possible)
   - log the destructor calls with != 0 ref count
   - log all incref/decref (with module info, and maybe even file/line number etc.?)
 */
class IRefCounted
{
int refCount;
public:
IRefCounted() { refCount = 1; }
virtual ~IRefCounted() { }
void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
};

#endif // __ISYNAPSE_H__
