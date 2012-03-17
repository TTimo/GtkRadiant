/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "StdAfx.h" // Added by ClassView

class CBspPoint {
public:
float p[3];
};

class CBspPortal {
public:
CBspPortal();
~CBspPortal();

unsigned point_count;
CBspPoint *point;
bool Build( char *def, unsigned int pointCnt, bool bInverse );
};


class CBspNode {
public:
CBspPortal *portal;
unsigned int portal_count;

bool AddPortal( char* def, unsigned int pointCnt, bool bInverse );
unsigned int portal_next;
CBspNode();
~CBspNode();
};


class CPortals {
public:

CPortals();
~CPortals();

void Load();     // use filename in fn
void Purge();

char fn[PATH_MAX];
CBspNode *node;

unsigned int node_count;
};
