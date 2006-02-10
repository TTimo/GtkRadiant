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

#include "hashtable.h"

#if defined(_DEBUG) || defined(DOXYGEN)

#include "hashfunc.h"

namespace ExampleHashTable
{
  void testStuff()
  {
    // HashTable example
    typedef HashTable<CopiedString, int, HashString> MyHashTable;
    MyHashTable hashtable;
    hashtable["bleh"] = 5;
    hashtable.insert("blah", 17);
    hashtable["foo"] = 99;
    hashtable.insert("bar", 23);

    int bleh = (*hashtable.find("bleh")).value; // 5
    int blah = hashtable["blah"]; // 17
    hashtable.erase("foo");
    MyHashTable::iterator barIter = hashtable.find("bar");
    hashtable.erase(barIter);

    for(MyHashTable::iterator i = hashtable.begin(); i != hashtable.end(); ++i)
    {
      if((*i).key != "bleh")
      {
        ++hashtable["count"]; // insertion does not invalidate iterators
      }
    }
    // end example
  }

  struct Always
  {
    Always()
    {
      testStuff();
    }
  } always;
}

#endif
