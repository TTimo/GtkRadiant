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

#include "memory/allocator.h"

#include <vector>

template<typename Value>
struct Vector
{ 
	typedef std::vector<Value, DefaultAllocator<Value> > Type; 
};

namespace
{
  class Bleh
  {
    int* m_blah;
  public:
    Bleh(int* blah) : m_blah(blah)
    {
    }
    ~Bleh()
    {
      *m_blah = 15;
    }
  };

  void TestAllocator()
  {
    Vector<Bleh>::Type test;

    int i = 0;
    test.push_back(Bleh(&i));
  }

  void TestNewDelete()
  {
    {
      NamedAllocator<int> allocator("test");
      int* p = NamedNew<int>::type(allocator).scalar();
      //new int();
      NamedDelete<int>::type(allocator).scalar(p);
    }

    {
      int* p = New<int>().scalar(3);
      Delete<int>().scalar(p);
    }

    {
      int* p = New<int>().scalar(int(15.9));
      Delete<int>().scalar(p);
    }

    {
      int* p = New<int>().vector(15);
      // new int[15]
      Delete<int>().vector(p, 15);
    }
  }
}