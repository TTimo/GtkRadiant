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

#if !defined(INCLUDED_PARSE_H)
#define INCLUDED_PARSE_H

#include "imap.h"

class BrushCreator;
class PatchCreator;

class PrimitiveParser
{
public:
  virtual scene::Node& parsePrimitive(Tokeniser& tokeniser) const = 0;
};

void Map_Read(scene::Node& root, Tokeniser& tokeniser, EntityCreator& entityTable, const PrimitiveParser& parser);

namespace scene
{
  class Node;
}

#include "generic/referencecounted.h"
typedef SmartReference<scene::Node, IncRefDecRefCounter<scene::Node> > NodeSmartReference;

extern NodeSmartReference g_nullNode;

#endif
