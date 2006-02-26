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

#if !defined(INCLUDED_MODEL_H)
#define INCLUDED_MODEL_H

#include "cullable.h"
#include "renderable.h"
#include "selectable.h"
#include "modelskin.h"

#include "math/frustum.h"
#include "string/string.h"
#include "generic/static.h"
#include "stream/stringstream.h"
#include "os/path.h"
#include "scenelib.h"
#include "instancelib.h"
#include "transformlib.h"
#include "traverselib.h"
#include "render.h"

class VectorLightList : public LightList
{
  typedef std::vector<const RendererLight*> Lights;
  Lights m_lights;
public:
  void addLight(const RendererLight& light)
  {
    m_lights.push_back(&light);
  }
  void clear()
  {
    m_lights.clear();
  }
  void evaluateLights() const
  {
  }
  void lightsChanged() const
  {
  }
  void forEachLight(const RendererLightCallback& callback) const
  {
    for(Lights::const_iterator i = m_lights.begin(); i != m_lights.end(); ++i)
    {
      callback(*(*i));
    }
  }
};

inline VertexPointer vertexpointer_arbitrarymeshvertex(const ArbitraryMeshVertex* array)
{
  return VertexPointer(VertexPointer::pointer(&array->vertex), sizeof(ArbitraryMeshVertex));
}

inline void parseTextureName(CopiedString& name, const char* token)
{
  StringOutputStream cleaned(256);
  cleaned << PathCleaned(token);
  name = StringRange(cleaned.c_str(), path_get_filename_base_end(cleaned.c_str())); // remove extension
}

// generic renderable triangle surface
class Surface : 
public OpenGLRenderable
{
public:
  typedef VertexBuffer<ArbitraryMeshVertex> vertices_t;
  typedef IndexBuffer indices_t;
private:

  AABB m_aabb_local;
  CopiedString m_shader;
  Shader* m_state;

  vertices_t m_vertices;
  indices_t m_indices;

  void CaptureShader()
  {
    m_state = GlobalShaderCache().capture(m_shader.c_str());
  }
  void ReleaseShader()
  {
    GlobalShaderCache().release(m_shader.c_str());
  }

public:

  Surface()
    : m_shader(""), m_state(0)
  {
    CaptureShader();
  }
  ~Surface()
  {
    ReleaseShader();
  }

  vertices_t& vertices()
  {
    return m_vertices;
  }
  indices_t& indices()
  {
    return m_indices;
  }

  void setShader(const char* name)
  {
    ReleaseShader();
    parseTextureName(m_shader, name);
    CaptureShader();
  }
  const char* getShader() const
  {
    return m_shader.c_str();
  }
  Shader* getState() const
  {
    return m_state;
  }
  void updateAABB()
  {
    m_aabb_local = AABB();
    for(vertices_t::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
      aabb_extend_by_point_safe(m_aabb_local, reinterpret_cast<const Vector3&>((*i).vertex));



    for(Surface::indices_t::iterator i = m_indices.begin(); i != m_indices.end(); i += 3)
    {
			ArbitraryMeshVertex& a = m_vertices[*(i + 0)];
			ArbitraryMeshVertex& b = m_vertices[*(i + 1)];
			ArbitraryMeshVertex& c = m_vertices[*(i + 2)];

      ArbitraryMeshTriangle_sumTangents(a, b, c);
    }

    for(Surface::vertices_t::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
    {
      vector3_normalise(reinterpret_cast<Vector3&>((*i).tangent));
      vector3_normalise(reinterpret_cast<Vector3&>((*i).bitangent));
    }
  }

  void render(RenderStateFlags state) const
  {
#if 1
    if((state & RENDER_BUMP) != 0)
    {
      if(GlobalShaderCache().useShaderLanguage())
      {
        glNormalPointer(GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_vertices.data()->normal);
        glVertexAttribPointerARB(c_attr_TexCoord0, 2, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->texcoord);
        glVertexAttribPointerARB(c_attr_Tangent, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->tangent);
        glVertexAttribPointerARB(c_attr_Binormal, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->bitangent);
      }
      else
      {
        glVertexAttribPointerARB(11, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->normal);
        glVertexAttribPointerARB(8, 2, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->texcoord);
        glVertexAttribPointerARB(9, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->tangent);
        glVertexAttribPointerARB(10, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_vertices.data()->bitangent);
      }
    }
    else
    {
      glNormalPointer(GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_vertices.data()->normal);
      glTexCoordPointer(2, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_vertices.data()->texcoord);
    }
    glVertexPointer(3, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_vertices.data()->vertex);
    glDrawElements(GL_TRIANGLES, GLsizei(m_indices.size()), RenderIndexTypeID, m_indices.data());
#else
    glBegin(GL_TRIANGLES);
    for(unsigned int i = 0; i < m_indices.size(); ++i)
    {
      glTexCoord2fv(&m_vertices[m_indices[i]].texcoord.s);
      glNormal3fv(&m_vertices[m_indices[i]].normal.x);
      glVertex3fv(&m_vertices[m_indices[i]].vertex.x);
    }
    glEnd();
#endif

#if defined(_DEBUG)
    glBegin(GL_LINES);

    for(VertexBuffer<ArbitraryMeshVertex>::const_iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
    {
      Vector3 normal = vector3_added(vertex3f_to_vector3((*i).vertex), vector3_scaled(normal3f_to_vector3((*i).normal), 8));
      glVertex3fv(vertex3f_to_array((*i).vertex));
      glVertex3fv(vector3_to_array(normal));
    }
    glEnd();
#endif
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return test.TestAABB(m_aabb_local, localToWorld);
  }

  const AABB& localAABB() const
  {
    return m_aabb_local;
  }

  void render(Renderer& renderer, const Matrix4& localToWorld, Shader* state) const
  {
    renderer.SetState(state, Renderer::eFullMaterials);
    renderer.addRenderable(*this, localToWorld);
  }

  void render(Renderer& renderer, const Matrix4& localToWorld) const
  {
    render(renderer, localToWorld, m_state);
  }

  void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld)
  {
    test.BeginMesh(localToWorld);

    SelectionIntersection best;
    test.TestTriangles(
      vertexpointer_arbitrarymeshvertex(m_vertices.data()),
      IndexPointer(m_indices.data(), IndexPointer::index_type(m_indices.size())),
      best
    );
    if(best.valid())
    {
      selector.addIntersection(best);
    }
  }
};

// generic model node
class Model :
public Cullable,
public Bounded
{
  typedef std::vector<Surface*> surfaces_t;
  surfaces_t m_surfaces;

  AABB m_aabb_local;
public:
  Callback m_lightsChanged;

  ~Model()
  {
    for(surfaces_t::iterator i = m_surfaces.begin(); i != m_surfaces.end(); ++i)
    {
      delete *i;
    }
  }

  typedef surfaces_t::const_iterator const_iterator;

  const_iterator begin() const
  {
    return m_surfaces.begin();
  }
  const_iterator end() const
  {
    return m_surfaces.end();
  }
  std::size_t size() const
  {
    return m_surfaces.size();
  }

  Surface& newSurface()
  {
    m_surfaces.push_back(new Surface);
    return *m_surfaces.back();
  }
  void updateAABB()
  {
    m_aabb_local = AABB();
    for(surfaces_t::iterator i = m_surfaces.begin(); i != m_surfaces.end(); ++i)
    {
      aabb_extend_by_aabb_safe(m_aabb_local, (*i)->localAABB());
    }
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return test.TestAABB(m_aabb_local, localToWorld);
  }

  virtual const AABB& localAABB() const
  {
    return m_aabb_local;
  }

  void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld)
  {
    for(surfaces_t::iterator i = m_surfaces.begin(); i != m_surfaces.end(); ++i)
    {
      if((*i)->intersectVolume(test.getVolume(), localToWorld) != c_volumeOutside)
      {
        (*i)->testSelect(selector, test, localToWorld);
      }
    }
  }
};

inline void Surface_addLight(const Surface& surface, VectorLightList& lights, const Matrix4& localToWorld, const RendererLight& light)
{
  if(light.testAABB(aabb_for_oriented_aabb(surface.localAABB(), localToWorld)))
  {
    lights.addLight(light);
  }
}

class ModelInstance :
  public scene::Instance,
  public Renderable,
  public SelectionTestable,
  public LightCullable,
  public SkinnedModel
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      InstanceContainedCast<ModelInstance, Bounded>::install(m_casts);
      InstanceContainedCast<ModelInstance, Cullable>::install(m_casts);
      InstanceStaticCast<ModelInstance, Renderable>::install(m_casts);
      InstanceStaticCast<ModelInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<ModelInstance, SkinnedModel>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  Model& m_model;

  const LightList* m_lightList;
  typedef Array<VectorLightList> SurfaceLightLists;
  SurfaceLightLists m_surfaceLightLists;

  class Remap
  {
  public:
    CopiedString first;
    Shader* second;
    Remap() : second(0)
    {
    }
  };
  typedef Array<Remap> SurfaceRemaps;
  SurfaceRemaps m_skins;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Bounded& get(NullType<Bounded>)
  {
    return m_model;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_model;
  }

  void lightsChanged()
  {
    m_lightList->lightsChanged();
  }
  typedef MemberCaller<ModelInstance, &ModelInstance::lightsChanged> LightsChangedCaller;

  void constructRemaps()
  {
    ModelSkin* skin = NodeTypeCast<ModelSkin>::cast(path().parent());
    if(skin != 0 && skin->realised())
    {
      SurfaceRemaps::iterator j = m_skins.begin();
      for(Model::const_iterator i = m_model.begin(); i != m_model.end(); ++i, ++j)
      {
        const char* remap = skin->getRemap((*i)->getShader());
        if(!string_empty(remap))
        {
          (*j).first = remap;
          (*j).second = GlobalShaderCache().capture(remap);
        }
        else
        {
          (*j).second = 0;
        }
      }
      SceneChangeNotify();
    }
  }
  void destroyRemaps()
  {
    for(SurfaceRemaps::iterator i = m_skins.begin(); i != m_skins.end(); ++i)
    {
      if((*i).second != 0)
      {
        GlobalShaderCache().release((*i).first.c_str());
        (*i).second = 0;
      }
    }
  }
  void skinChanged()
  {
    ASSERT_MESSAGE(m_skins.size() == m_model.size(), "ERROR");
    destroyRemaps();
    constructRemaps();
  }

  ModelInstance(const scene::Path& path, scene::Instance* parent, Model& model) :
    Instance(path, parent, this, StaticTypeCasts::instance().get()), 
    m_model(model),
    m_surfaceLightLists(m_model.size()),
    m_skins(m_model.size())
  {
    m_lightList = &GlobalShaderCache().attach(*this);
    m_model.m_lightsChanged = LightsChangedCaller(*this);

    Instance::setTransformChangedCallback(LightsChangedCaller(*this));

    constructRemaps();
  }
  ~ModelInstance()
  {
    destroyRemaps();

    Instance::setTransformChangedCallback(Callback());

    m_model.m_lightsChanged = Callback();
    GlobalShaderCache().detach(*this);
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    SurfaceLightLists::const_iterator j = m_surfaceLightLists.begin();
    SurfaceRemaps::const_iterator k = m_skins.begin();
    for(Model::const_iterator i = m_model.begin(); i != m_model.end(); ++i, ++j, ++k)
    {
      if((*i)->intersectVolume(volume, localToWorld) != c_volumeOutside)
      {
        renderer.setLights(*j);
        (*i)->render(renderer, localToWorld, (*k).second != 0 ? (*k).second : (*i)->getState());
      }
    }
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_lightList->evaluateLights();

    render(renderer, volume, Instance::localToWorld());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    renderSolid(renderer, volume);
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    m_model.testSelect(selector, test, Instance::localToWorld());
  }

  bool testLight(const RendererLight& light) const
  {
    return light.testAABB(worldAABB());
  }
  void insertLight(const RendererLight& light)
  {
    const Matrix4& localToWorld = Instance::localToWorld();
    SurfaceLightLists::iterator j = m_surfaceLightLists.begin();
    for(Model::const_iterator i = m_model.begin(); i != m_model.end(); ++i)
    {
      Surface_addLight(*(*i), *j++, localToWorld, light);
    }
  }
  void clearLights()
  {
    for(SurfaceLightLists::iterator i = m_surfaceLightLists.begin(); i != m_surfaceLightLists.end(); ++i)
    {
      (*i).clear();
    }
  }
};

class ModelNode : public scene::Node::Symbiot, public scene::Instantiable
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<ModelNode, scene::Instantiable>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  Model m_model;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  ModelNode() : m_node(this, this, StaticTypeCasts::instance().get())
  {
  }

  Model& model()
  {
    return m_model;
  }

  void release()
  {
    delete this;
  }
  scene::Node& node()
  {
    return m_node;
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new ModelInstance(path, parent, m_model);
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


inline void Surface_constructQuad(Surface& surface, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& normal)
{
  surface.vertices().push_back(
    ArbitraryMeshVertex(
      vertex3f_for_vector3(a),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_topleft)
    )
  );
  surface.vertices().push_back(
    ArbitraryMeshVertex(
      vertex3f_for_vector3(b),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_topright)
    )
  );
  surface.vertices().push_back(
    ArbitraryMeshVertex(
      vertex3f_for_vector3(c),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_botright)
    )
  );
  surface.vertices().push_back(
    ArbitraryMeshVertex(
      vertex3f_for_vector3(d),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_botleft)
    )
  );
}

inline void Model_constructNull(Model& model)
{
  Surface& surface = model.newSurface();

  AABB aabb(Vector3(0, 0, 0), Vector3(8, 8, 8));

  Vector3 points[8];
	aabb_corners(aabb, points);

  surface.vertices().reserve(24);

  Surface_constructQuad(surface, points[2], points[1], points[5], points[6], aabb_normals[0]);
  Surface_constructQuad(surface, points[1], points[0], points[4], points[5], aabb_normals[1]);
  Surface_constructQuad(surface, points[0], points[1], points[2], points[3], aabb_normals[2]);
  Surface_constructQuad(surface, points[0], points[3], points[7], points[4], aabb_normals[3]);
  Surface_constructQuad(surface, points[3], points[2], points[6], points[7], aabb_normals[4]);
  Surface_constructQuad(surface, points[7], points[6], points[5], points[4], aabb_normals[5]);

  surface.indices().reserve(36);

  RenderIndex indices[36] = {
     0,  1,  2,  0,  2,  3,
     4,  5,  6,  4,  6,  7,
     8,  9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 10, 22, 23,
  };

  for(RenderIndex* i = indices; i != indices+(sizeof(indices)/sizeof(RenderIndex)); ++i)
  {
    surface.indices().insert(*i);
  }

  surface.setShader("");

  surface.updateAABB();

  model.updateAABB();
}

#endif
