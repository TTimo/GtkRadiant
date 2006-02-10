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

#if !defined(INCLUDED_TARGETABLE_H)
#define INCLUDED_TARGETABLE_H

#include <set>
#include <map>

#include "cullable.h"
#include "renderable.h"

#include "math/line.h"
#include "render.h"
#include "generic/callback.h"
#include "selectionlib.h"
#include "entitylib.h"
#include "eclasslib.h"
#include "stringio.h"

class Targetable
{
public:
  virtual const Vector3& world_position() const = 0;
};

typedef std::set<Targetable*> targetables_t;

extern const char* g_targetable_nameKey;

targetables_t* getTargetables(const char* targetname);

class EntityConnectionLine : public OpenGLRenderable
{
public:
  Vector3 start;
  Vector3 end;

  void render(RenderStateFlags state) const
  {
    float s1[2], s2[2];
    Vector3 dir(vector3_subtracted(end, start));
    double len = vector3_length(dir);
    vector3_scale(dir, 8.0 * (1.0 / len));
    s1[0] = dir[0] - dir[1];
    s1[1] = dir[0] + dir[1];
    s2[0] = dir[0] + dir[1];
    s2[1] = -dir[0] + dir[1];
    
    glBegin(GL_LINES);

    glVertex3fv(vector3_to_array(start));
    glVertex3fv(vector3_to_array(end));

    len*=0.0625; // half / 8

    Vector3 arrow(start);
    for (unsigned int i = 0, count = (len<32)? 1 : static_cast<unsigned int>(len*0.0625); i < count; i++)
    {
      vector3_add(arrow, vector3_scaled(dir, (len<32)?len:32));
      glVertex3fv(vector3_to_array(arrow));
      glVertex3f(arrow[0]+s1[0], arrow[1]+s1[1], arrow[2]+dir[2]);
      glVertex3fv(vector3_to_array(arrow));
      glVertex3f(arrow[0]+s2[0], arrow[1]+s2[1], arrow[2]+dir[2]);
    }
    
    glEnd();
  }
};

class TargetedEntity
{
  Targetable& m_targetable;
  targetables_t* m_targets;

  void construct()
  {
    if(m_targets != 0)
      m_targets->insert(&m_targetable);
  }
  void destroy()
  {
    if(m_targets != 0)
      m_targets->erase(&m_targetable);
  }
public:
  TargetedEntity(Targetable& targetable)
    : m_targetable(targetable), m_targets(getTargetables(""))
  {
    construct();
  }
  ~TargetedEntity()
  {
    destroy();
  }
  void targetnameChanged(const char* name)
  {
    destroy();
    m_targets = getTargetables(name);
    construct();
  }
  typedef MemberCaller1<TargetedEntity, const char*, &TargetedEntity::targetnameChanged> TargetnameChangedCaller;
};


class TargetingEntity
{
  targetables_t* m_targets;
public:
  TargetingEntity() :
    m_targets(getTargetables(""))
  {
  }
  void targetChanged(const char* target)
  {
    m_targets = getTargetables(target);
  }
  typedef MemberCaller1<TargetingEntity, const char*, &TargetingEntity::targetChanged> TargetChangedCaller;

  typedef targetables_t::iterator iterator;

  iterator begin() const
  {
    if(m_targets == 0)
    {
      return iterator();
    }
    return m_targets->begin();
  }
  iterator end() const
  {
    if(m_targets == 0)
    {
      return iterator();
    }
    return m_targets->end();
  }
  size_t size() const
  {
    if(m_targets == 0)
    {
      return 0;
    }
    return m_targets->size();
  }
  bool empty() const
  {
    return m_targets == 0 || m_targets->empty();
  }
};



template<typename Functor>
void TargetingEntity_forEach(const TargetingEntity& targets, const Functor& functor)
{
  for(TargetingEntity::iterator i = targets.begin(); i != targets.end(); ++i)
  {
    functor((*i)->world_position());
  }
}

typedef std::map<std::size_t, TargetingEntity> TargetingEntities;

template<typename Functor>
void TargetingEntities_forEach(const TargetingEntities& targetingEntities, const Functor& functor)
{
  for(TargetingEntities::const_iterator i = targetingEntities.begin(); i != targetingEntities.end(); ++i)
  {
    TargetingEntity_forEach((*i).second, functor);
  }
}

class TargetLinesPushBack
{
  RenderablePointVector& m_targetLines;
  const Vector3& m_worldPosition;
  const VolumeTest& m_volume;
public:
  TargetLinesPushBack(RenderablePointVector& targetLines, const Vector3& worldPosition, const VolumeTest& volume) :
    m_targetLines(targetLines), m_worldPosition(worldPosition), m_volume(volume)
  {
  }
  void operator()(const Vector3& worldPosition) const
  {
    if(m_volume.TestLine(segment_for_startend(m_worldPosition, worldPosition)))
    {
      m_targetLines.push_back(PointVertex(reinterpret_cast<const Vertex3f&>(m_worldPosition)));
      m_targetLines.push_back(PointVertex(reinterpret_cast<const Vertex3f&>(worldPosition)));
    }
  }
};

class TargetKeys : public EntityKeyValues::Observer
{
  TargetingEntities m_targetingEntities;
  Callback m_targetsChanged;

  bool readTargetKey(const char* key, std::size_t& index)
  {
    if(string_equal_n(key, "target", 6))
    {
      index = 0;
      if(string_empty(key + 6) || string_parse_size(key + 6, index))
      {
        return true;
      }
    }
    return false;
  }
public:
  void setTargetsChanged(const Callback& targetsChanged)
  {
    m_targetsChanged = targetsChanged;
  }
  void targetsChanged()
  {
    m_targetsChanged();
  }

  void insert(const char* key, EntityKeyValues::Value& value)
  {
    std::size_t index;
    if(readTargetKey(key, index))
    {
      TargetingEntities::iterator i = m_targetingEntities.insert(TargetingEntities::value_type(index, TargetingEntity())).first;
      value.attach(TargetingEntity::TargetChangedCaller((*i).second));
      targetsChanged();
    }
  }
  void erase(const char* key, EntityKeyValues::Value& value)
  {
    std::size_t index;
    if(readTargetKey(key, index))
    {
      TargetingEntities::iterator i = m_targetingEntities.find(index);
      value.detach(TargetingEntity::TargetChangedCaller((*i).second));
      m_targetingEntities.erase(i);
      targetsChanged();
    }
  }
  const TargetingEntities& get() const
  {
    return m_targetingEntities;
  }
};



class RenderableTargetingEntity
{
  TargetingEntity& m_targets;
  mutable RenderablePointVector m_target_lines;
public:
  static Shader* m_state;

  RenderableTargetingEntity(TargetingEntity& targets)
    : m_targets(targets), m_target_lines(GL_LINES)
  {
  }
  void compile(const VolumeTest& volume, const Vector3& world_position) const
  {
    m_target_lines.clear();
    m_target_lines.reserve(m_targets.size() * 2);
    TargetingEntity_forEach(m_targets, TargetLinesPushBack(m_target_lines, world_position, volume));
  }
  void render(Renderer& renderer, const VolumeTest& volume, const Vector3& world_position) const
  {
    if(!m_targets.empty())
    {
      compile(volume, world_position);
      if(!m_target_lines.empty())
      {
        renderer.addRenderable(m_target_lines, g_matrix4_identity);
      }
    }
  }
};

class RenderableTargetingEntities
{
  const TargetingEntities& m_targets;
  mutable RenderablePointVector m_target_lines;
public:
  static Shader* m_state;

  RenderableTargetingEntities(const TargetingEntities& targets)
    : m_targets(targets), m_target_lines(GL_LINES)
  {
  }
  void compile(const VolumeTest& volume, const Vector3& world_position) const
  {
    m_target_lines.clear();
    TargetingEntities_forEach(m_targets, TargetLinesPushBack(m_target_lines, world_position, volume));
  }
  void render(Renderer& renderer, const VolumeTest& volume, const Vector3& world_position) const
  {
    if(!m_targets.empty())
    {
      compile(volume, world_position);
      if(!m_target_lines.empty())
      {
        renderer.addRenderable(m_target_lines, g_matrix4_identity);
      }
    }
  }
};


class TargetableInstance :
public SelectableInstance,
public Targetable,
public EntityKeyValues::Observer
{
  mutable Vertex3f m_position;
  EntityKeyValues& m_entity;
  TargetKeys m_targeting;
  TargetedEntity m_targeted;
  RenderableTargetingEntities m_renderable;
public:

  TargetableInstance(
    const scene::Path& path,
    scene::Instance* parent,
    void* instance,
    InstanceTypeCastTable& casts,
    EntityKeyValues& entity,
    Targetable& targetable
  ) :
    SelectableInstance(path, parent, instance, casts),
    m_entity(entity),
    m_targeted(targetable),
    m_renderable(m_targeting.get())
  {
    m_entity.attach(*this);
    m_entity.attach(m_targeting);
  }
  ~TargetableInstance()
  {
    m_entity.detach(m_targeting);
    m_entity.detach(*this);
  }

  void setTargetsChanged(const Callback& targetsChanged)
  {
    m_targeting.setTargetsChanged(targetsChanged);
  }
  void targetsChanged()
  {
    m_targeting.targetsChanged();
  }

  void insert(const char* key, EntityKeyValues::Value& value)
  {
    if(string_equal(key, g_targetable_nameKey))
    {
      value.attach(TargetedEntity::TargetnameChangedCaller(m_targeted));
    }
  }
  void erase(const char* key, EntityKeyValues::Value& value)
  {
    if(string_equal(key, g_targetable_nameKey))
    {
      value.detach(TargetedEntity::TargetnameChangedCaller(m_targeted));
    }
  }

  const Vector3& world_position() const
  {
#if 1
    const AABB& bounds = Instance::worldAABB();
    if(aabb_valid(bounds))
    {
      return bounds.origin;
    }
#else
    const AABB& childBounds = Instance::childBounds();
    if(aabb_valid(childBounds))
    {
      return childBounds.origin;
    }
#endif
    return localToWorld().t();
  }

  void render(Renderer& renderer, const VolumeTest& volume) const
  {
    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eWireframeOnly);
    renderer.SetState(m_entity.getEntityClass().m_state_wire, Renderer::eFullMaterials);
    m_renderable.render(renderer, volume, world_position());
  }

  const TargetingEntities& getTargeting() const
  {
    return m_targeting.get();
  }
};


class RenderableConnectionLines : public Renderable
{
  typedef std::set<TargetableInstance*> TargetableInstances;
  TargetableInstances m_instances;
public:
  void attach(TargetableInstance& instance)
  {
    ASSERT_MESSAGE(m_instances.find(&instance) == m_instances.end(), "cannot attach instance");
    m_instances.insert(&instance);
  }
  void detach(TargetableInstance& instance)
  {
    ASSERT_MESSAGE(m_instances.find(&instance) != m_instances.end(), "cannot detach instance");
    m_instances.erase(&instance);
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    for(TargetableInstances::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
    {
      if((*i)->path().top().get().visible())
      {
        (*i)->render(renderer, volume);
      }
    }
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    renderSolid(renderer, volume);
  }
};

typedef Static<RenderableConnectionLines> StaticRenderableConnectionLines;

#endif
