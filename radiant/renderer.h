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

#if !defined(INCLUDED_RENDERER_H)
#define INCLUDED_RENDERER_H

#include "irender.h"
#include "renderable.h"
#include "iselection.h"
#include "cullable.h"
#include "scenelib.h"
#include "math/frustum.h"
#include <vector>

inline Cullable* Instance_getCullable(scene::Instance& instance)
{
  return InstanceTypeCast<Cullable>::cast(instance);
}

inline Renderable* Instance_getRenderable(scene::Instance& instance)
{
  return InstanceTypeCast<Renderable>::cast(instance);
}

inline VolumeIntersectionValue Cullable_testVisible(scene::Instance& instance, const VolumeTest& volume, VolumeIntersectionValue parentVisible)
{
  if(parentVisible == c_volumePartial)
  {
    Cullable* cullable = Instance_getCullable(instance);
    if(cullable != 0)
    {
      return cullable->intersectVolume(volume, instance.localToWorld());
    }
  }
  return parentVisible;
}

template<typename _Walker>
class CullingWalker
{
  const VolumeTest& m_volume;
  const _Walker& m_walker;
public:
  CullingWalker(const VolumeTest& volume, const _Walker& walker)
    : m_volume(volume), m_walker(walker)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance, VolumeIntersectionValue parentVisible) const
  {
    VolumeIntersectionValue visible = Cullable_testVisible(instance, m_volume, parentVisible);
    if(visible != c_volumeOutside)
    {
      return m_walker.pre(path, instance);
    }
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance, VolumeIntersectionValue parentVisible) const
  {
    return m_walker.post(path, instance);
  }
};

template<typename Walker_>
class ForEachVisible : public scene::Graph::Walker
{
  const VolumeTest& m_volume;
  const Walker_& m_walker;
  mutable std::vector<VolumeIntersectionValue> m_state;
public:
  ForEachVisible(const VolumeTest& volume, const Walker_& walker)
    : m_volume(volume), m_walker(walker)
  {
    m_state.push_back(c_volumePartial);
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    VolumeIntersectionValue visible = (path.top().get().visible()) ? m_state.back() : c_volumeOutside;

    if(visible == c_volumePartial)
    {
      visible = m_volume.TestAABB(instance.worldAABB());
    }

    m_state.push_back(visible);

    if(visible == c_volumeOutside)
    {
      return false;
    }
    else
    {
      return m_walker.pre(path, instance, m_state.back());
    }
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    if(m_state.back() != c_volumeOutside)
    {
      m_walker.post(path, instance, m_state.back());
    }

    m_state.pop_back();
  }
};

template<typename Functor>
inline void Scene_forEachVisible(scene::Graph& graph, const VolumeTest& volume, const Functor& functor)
{
  graph.traverse(ForEachVisible< CullingWalker<Functor> >(volume, CullingWalker<Functor>(volume, functor)));
}

class RenderHighlighted
{
  Renderer& m_renderer;
  const VolumeTest& m_volume;
public:
  RenderHighlighted(Renderer& renderer, const VolumeTest& volume)
    : m_renderer(renderer), m_volume(volume)
  {
  }
  void render(const Renderable& renderable) const
  {
    switch(m_renderer.getStyle())
    {
    case Renderer::eFullMaterials:
      renderable.renderSolid(m_renderer, m_volume);
      break;
    case Renderer::eWireframeOnly:
      renderable.renderWireframe(m_renderer, m_volume);
      break;
    }      
  }
  typedef ConstMemberCaller1<RenderHighlighted, const Renderable&, &RenderHighlighted::render> RenderCaller;

  bool pre(const scene::Path& path, scene::Instance& instance, VolumeIntersectionValue parentVisible) const
  {
    m_renderer.PushState();

    if(Cullable_testVisible(instance, m_volume, parentVisible) != c_volumeOutside)
    {
      Renderable* renderable = Instance_getRenderable(instance);
      if(renderable)
      {
        renderable->viewChanged();
      }

      Selectable* selectable = Instance_getSelectable(instance);
      if(selectable != 0 && selectable->isSelected())
      {
        if(GlobalSelectionSystem().Mode() != SelectionSystem::eComponent)
        {
          m_renderer.Highlight(Renderer::eFace);
        }
        else if(renderable)
        {
          renderable->renderComponents(m_renderer, m_volume);
        }
        m_renderer.Highlight(Renderer::ePrimitive);
      }
        
      if(renderable)
      {
        render(*renderable);    
      }
    }

    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance, VolumeIntersectionValue parentVisible) const
  {
    m_renderer.PopState();
  }
};

inline void Scene_Render(Renderer& renderer, const VolumeTest& volume)
{
  GlobalSceneGraph().traverse(ForEachVisible<RenderHighlighted>(volume, RenderHighlighted(renderer, volume)));
  GlobalShaderCache().forEachRenderable(RenderHighlighted::RenderCaller(RenderHighlighted(renderer, volume)));
}

#endif
