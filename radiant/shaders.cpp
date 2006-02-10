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

#include "shaders.h"

#include "ifilesystem.h"

#include "stream/stringstream.h"

#include "gtkdlgs.h"

void ViewShader(const char *pFile, const char *pName)
{
  char* pBuff = 0;
  //int nSize = 
  vfsLoadFile(pFile, reinterpret_cast<void**>(&pBuff));
  if (pBuff == 0)
  {
    globalErrorStream() << "Failed to load shader file " << pFile << "\n";
    return;
  }
  // look for the shader declaration
  StringOutputStream strFind(string_length(pName));
  strFind << LowerCase(pName);
  StringOutputStream strLook(string_length(pBuff));
  strFind << LowerCase(pBuff);
  // offset used when jumping over commented out definitions
  std::size_t nOffset = 0;
  while (true)
  {
    const char* substr = strstr(strFind.c_str() + nOffset, strFind.c_str());
    if (substr == 0)
      break;
    std::size_t nStart = substr - strLook.c_str();
    // we have found something, maybe it's a commented out shader name?
    char *strCheck = new char[string_length(strLook.c_str())+1];
    strcpy( strCheck, strLook.c_str() );
    strCheck[nStart] = 0;
    char *pCheck = strrchr( strCheck, '\n' );
    // if there's a commentary sign in-between we'll continue
    if (pCheck && strstr( pCheck, "//" ))
    {
      delete[] strCheck;
      nOffset = nStart + 1;
      continue;
    }
    delete[] strCheck;
    nOffset = nStart;
    break;
  }
  // now close the file
  vfsFreeFile(pBuff);

  DoTextEditor (pFile, static_cast<int>(nOffset));
}
