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

#include "write.h"

#include "ientity.h"
#include "iscriplib.h"
#include "scenelib.h"

inline MapExporter* Node_getMapExporter(scene::Node& node)
{
  return NodeTypeCast<MapExporter>::cast(node);
}


static std::size_t g_count_entities;
static std::size_t g_count_brushes;


void Entity_ExportTokens(const Entity& entity, TokenWriter& writer)
{
  g_count_brushes = 0;

  class WriteKeyValue : public Entity::Visitor
  {
    TokenWriter& m_writer;
  public:
    WriteKeyValue(TokenWriter& writer)
      : m_writer(writer)
    {
    }

    void visit(const char* key, const char* value)
    {
      m_writer.writeString(key);
      m_writer.writeString(value);
      m_writer.nextLine();
    }

  } visitor(writer);

  entity.forEachKeyValue(visitor);
}

class WriteTokensWalker : public scene::Traversable::Walker
{
  mutable Stack<bool> m_stack;
  TokenWriter& m_writer;
  bool m_ignorePatches;
public:
  WriteTokensWalker(TokenWriter& writer, bool ignorePatches)
    : m_writer(writer), m_ignorePatches(ignorePatches)
  {
  }
  bool pre(scene::Node& node) const
  {
    m_stack.push(false);

    Entity* entity = Node_getEntity(node);
    if(entity != 0)
    {
      m_writer.writeToken("//");
      m_writer.writeToken("entity");
      m_writer.writeUnsigned(g_count_entities++);
      m_writer.nextLine();

      m_writer.writeToken("{");
      m_writer.nextLine();
      m_stack.top() = true;

      Entity_ExportTokens(*entity, m_writer);
    }
    else
    {
      MapExporter* exporter = Node_getMapExporter(node);
      if(exporter != 0
      && !(m_ignorePatches && Node_isPatch(node)))
      {
        m_writer.writeToken("//");
        m_writer.writeToken("brush");
        m_writer.writeUnsigned(g_count_brushes++);
        m_writer.nextLine();

        exporter->exportTokens(m_writer);
      }
    }

    return true;
  }
  void post(scene::Node& node) const
  {
    if(m_stack.top())
    {
      m_writer.writeToken("}");
      m_writer.nextLine();
    }
    m_stack.pop();
  }
};

void Map_Write(scene::Node& root, GraphTraversalFunc traverse, TokenWriter& writer, bool ignorePatches)
{
  g_count_entities = 0;
  traverse(root, WriteTokensWalker(writer, ignorePatches));
}

