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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// header for Pointfile stuff (adding a C++ class to wrap the pointfile thing in the SAX parser)
//

#ifndef __POINTS__
#define __POINTS__

void Pointfile_Delete( void );
void WINAPI Pointfile_Check( void );
void Pointfile_Next( void );
void Pointfile_Prev( void );
void Pointfile_Clear( void );
void Pointfile_Draw( void );
void Pointfile_Load( void );

class CPointfile : public ISAXHandler
{
public:
CPointfile() { }
void Init();
void PushPoint( vec3_t v );
void GenerateDisplayList();
// SAX interface
void saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs );
void saxEndElement( message_info_t *ctx, const xmlChar *name );
void saxCharacters( message_info_t *ctx, const xmlChar *ch, int len );
char *getName();

// class is only used for g_pointfile and we should not attempt to free it
bool ShouldDelete() { return false; }
};

// instead of using Pointfile_Load you can do it by hand through g_pointfile
// but the usual pointfile mechanism remains the same, use Pointfile_Draw etc.
extern CPointfile g_pointfile;

#endif
