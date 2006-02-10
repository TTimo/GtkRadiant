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

#if !defined(INCLUDED_IMAP_H)
#define INCLUDED_IMAP_H

#include "generic/constant.h"

class Tokeniser;
class TokenWriter;

/// \brief A node whose state can be imported from a token stream.
class MapImporter
{
public:
  STRING_CONSTANT(Name, "MapImporter");

  virtual bool importTokens(Tokeniser& tokeniser) = 0;
};

/// \brief A node whose state can be exported to a token stream.
class MapExporter
{
public:
  STRING_CONSTANT(Name, "MapExporter");

  virtual void exportTokens(TokenWriter& writer) const = 0;
};

#include "iscenegraph.h"

class EntityCreator;

class TextInputStream;
class TextOutputStream;


typedef void(*GraphTraversalFunc)(scene::Node& root, const scene::Traversable::Walker& walker);

/// \brief A module that reads and writes a map in a specific format.
class MapFormat
{
public:
  INTEGER_CONSTANT(Version, 2);
  STRING_CONSTANT(Name, "map");

  /// \brief Read a map graph into \p root from \p outputStream, using \p entityTable to create entities.
  virtual void readGraph(scene::Node& root, TextInputStream& inputStream, EntityCreator& entityTable) const = 0;
  /// \brief Write the map graph obtained by applying \p traverse to \p root into \p outputStream.
  virtual void writeGraph(scene::Node& root, GraphTraversalFunc traverse, TextOutputStream& outputStream) const = 0;
};


template<typename Type>
class Modules;
typedef Modules<MapFormat> MapModules;

template<typename Type>
class ModulesRef;
typedef ModulesRef<MapFormat> MapModulesRef;


#endif
