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

#if !defined(INCLUDED_BRUSHNODE_H)
#define INCLUDED_BRUSHNODE_H

#include "instancelib.h"
#include "brush.h"
#include "brushtokens.h"
#include "brushxml.h"

class BrushNode :
public scene::Node::Symbiot,
public scene::Instantiable,
public scene::Cloneable
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<BrushNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<BrushNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<BrushNode, Snappable>::install(m_casts);
      NodeContainedCast<BrushNode, TransformNode>::install(m_casts);
      NodeContainedCast<BrushNode, Brush>::install(m_casts);
      NodeContainedCast<BrushNode, XMLImporter>::install(m_casts);
      NodeContainedCast<BrushNode, XMLExporter>::install(m_casts);
      NodeContainedCast<BrushNode, MapImporter>::install(m_casts);
      NodeContainedCast<BrushNode, MapExporter>::install(m_casts);
      NodeContainedCast<BrushNode, Nameable>::install(m_casts);
      NodeContainedCast<BrushNode, BrushDoom3>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  Brush m_brush;
  BrushTokenImporter m_mapImporter;
  BrushTokenExporter m_mapExporter;
  BrushXMLImporter m_xmlImporter;
  BrushXMLExporter m_xmlExporter;

public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Snappable& get(NullType<Snappable>)
  {
    return m_brush;
  }
  TransformNode& get(NullType<TransformNode>)
  {
    return m_brush;
  }
  Brush& get(NullType<Brush>)
  {
    return m_brush;
  }
  XMLImporter& get(NullType<XMLImporter>)
  {
    return m_xmlImporter;
  }
  XMLExporter& get(NullType<XMLExporter>)
  {
    return m_xmlExporter;
  }
  MapImporter& get(NullType<MapImporter>)
  {
    return m_mapImporter;
  }
  MapExporter& get(NullType<MapExporter>)
  {
    return m_mapExporter;
  }
  Nameable& get(NullType<Nameable>)
  {
    return m_brush;
  }
  BrushDoom3& get(NullType<BrushDoom3>)
  {
    return m_brush;
  }

  BrushNode() :
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_brush(m_node, InstanceSetEvaluateTransform<BrushInstance>::Caller(m_instances), InstanceSet::BoundsChangedCaller(m_instances)),
    m_mapImporter(m_brush),
    m_mapExporter(m_brush),
    m_xmlImporter(m_brush),
    m_xmlExporter(m_brush)
  {
  }
  BrushNode(const BrushNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_brush(other.m_brush, m_node, InstanceSetEvaluateTransform<BrushInstance>::Caller(m_instances), InstanceSet::BoundsChangedCaller(m_instances)),
    m_mapImporter(m_brush),
    m_mapExporter(m_brush),
    m_xmlImporter(m_brush),
    m_xmlExporter(m_brush)
  {
  }
  void release()
  {
    delete this;
  }
  scene::Node& node()
  {
    return m_node;
  }
  
  scene::Node& clone() const
  {
    return (new BrushNode(*this))->node();
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new BrushInstance(path, parent, m_brush);
  }
  void forEachInstance(const scene::Instantiable::Visitor& visitor)
  {
    m_instances.forEachInstance(visitor);
  }
  void insert(scene::Instantiable::Observer* observer, const scene::Path& path, scene::Instance* instance)
  {
    m_instances.insert(observer, path, instance);
  }
  scene::Instance* erase(scene::Instantiable::Observer* observer, const scene::Path& path)
  {
    return m_instances.erase(observer, path);
  }
};

inline Brush* Node_getBrush(scene::Node& node)
{
  return NodeTypeCast<Brush>::cast(node);
}

#endif
