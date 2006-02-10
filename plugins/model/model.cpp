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

#include "model.h"

#include "picomodel.h"

#include "iarchive.h"
#include "idatastream.h"
#include "imodel.h"
#include "modelskin.h"

#include "cullable.h"
#include "renderable.h"
#include "selectable.h"

#include "math/frustum.h"
#include "string/string.h"
#include "generic/static.h"
#include "shaderlib.h"
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

class PicoSurface : 
public OpenGLRenderable
{
  AABB m_aabb_local;
  CopiedString m_shader;
  Shader* m_state;

  Array<ArbitraryMeshVertex> m_vertices;
  Array<RenderIndex> m_indices;

public:

  PicoSurface()
  {
    constructNull();
    CaptureShader();
  }
  PicoSurface(picoSurface_t* surface)
  {
    CopyPicoSurface(surface);
    CaptureShader();
  }
  ~PicoSurface()
  {
    ReleaseShader();
  }

  void render(RenderStateFlags state) const
  {
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
#if defined(_DEBUG)
    glBegin(GL_LINES);

    for(Array<ArbitraryMeshVertex>::const_iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
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
    testSelect(test, best);
    if(best.valid())
    {
      selector.addIntersection(best);
    }
  }

  const char* getShader() const
  {
    return m_shader.c_str();
  }
  Shader* getState() const
  {
    return m_state;
  }

private:

  void CaptureShader()
  {
    m_state = GlobalShaderCache().capture(m_shader.c_str());
  }
  void ReleaseShader()
  {
    GlobalShaderCache().release(m_shader.c_str());
  }

  void UpdateAABB()
  {
    m_aabb_local = AABB();
    for(std::size_t i = 0; i < m_vertices.size(); ++i )
      aabb_extend_by_point_safe(m_aabb_local, reinterpret_cast<const Vector3&>(m_vertices[i].vertex));


    for(Array<RenderIndex>::iterator i = m_indices.begin(); i != m_indices.end(); i += 3)
    {
			ArbitraryMeshVertex& a = m_vertices[*(i + 0)];
			ArbitraryMeshVertex& b = m_vertices[*(i + 1)];
			ArbitraryMeshVertex& c = m_vertices[*(i + 2)];

      ArbitraryMeshTriangle_sumTangents(a, b, c);
    }

    for(Array<ArbitraryMeshVertex>::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
    {
      vector3_normalise(reinterpret_cast<Vector3&>((*i).tangent));
      vector3_normalise(reinterpret_cast<Vector3&>((*i).bitangent));
    }
  }

  void testSelect(SelectionTest& test, SelectionIntersection& best)
  {
    test.TestTriangles(
      VertexPointer(VertexPointer::pointer(&m_vertices.data()->vertex), sizeof(ArbitraryMeshVertex)),
      IndexPointer(m_indices.data(), IndexPointer::index_type(m_indices.size())),
      best
    );
  }

  void CopyPicoSurface(picoSurface_t* surface)
  {
    picoShader_t* shader = PicoGetSurfaceShader( surface );
    if( shader == 0 )
      m_shader = "";
    else
      m_shader = PicoGetShaderName( shader );
    
    m_vertices.resize( PicoGetSurfaceNumVertexes( surface ) );  
    m_indices.resize( PicoGetSurfaceNumIndexes( surface ) );
    
    for(std::size_t i = 0; i < m_vertices.size(); ++i )
    {
      picoVec_t* xyz = PicoGetSurfaceXYZ( surface, int(i) );
      m_vertices[i].vertex = vertex3f_from_array(xyz);
      
      picoVec_t* normal = PicoGetSurfaceNormal( surface, int(i) );
      m_vertices[i].normal = normal3f_from_array(normal);
      
      picoVec_t* st = PicoGetSurfaceST( surface, 0, int(i) );
      m_vertices[i].texcoord = TexCoord2f(st[0], st[1]);
      
#if 0
      picoVec_t* color = PicoGetSurfaceColor( surface, 0, int(i) );
      m_vertices[i].colour = Colour4b(color[0], color[1], color[2], color[3]);
#endif
    }
    
    picoIndex_t* indexes = PicoGetSurfaceIndexes( surface, 0 );
    for(std::size_t j = 0; j < m_indices.size(); ++j )
      m_indices[ j ] = indexes[ j ];

    UpdateAABB();
  }

  void constructQuad(std::size_t index, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& normal)
  {
    m_vertices[index * 4 + 0] = ArbitraryMeshVertex(
      vertex3f_for_vector3(a),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_topleft)
    );
    m_vertices[index * 4 + 1] = ArbitraryMeshVertex(
      vertex3f_for_vector3(b),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_topright)
    );
    m_vertices[index * 4 + 2] = ArbitraryMeshVertex(
      vertex3f_for_vector3(c),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_botright)
    );
    m_vertices[index * 4 + 3] = ArbitraryMeshVertex(
      vertex3f_for_vector3(d),
      normal3f_for_vector3(normal),
      texcoord2f_from_array(aabb_texcoord_botleft)
    );
  }

  void constructNull()
  {
    AABB aabb(Vector3(0, 0, 0), Vector3(8, 8, 8));

    Vector3 points[8];
	  aabb_corners(aabb, points);

    m_vertices.resize(24);

    constructQuad(0, points[2], points[1], points[5], points[6], aabb_normals[0]);
    constructQuad(1, points[1], points[0], points[4], points[5], aabb_normals[1]);
    constructQuad(2, points[0], points[1], points[2], points[3], aabb_normals[2]);
    constructQuad(3, points[0], points[3], points[7], points[4], aabb_normals[3]);
    constructQuad(4, points[3], points[2], points[6], points[7], aabb_normals[4]);
    constructQuad(5, points[7], points[6], points[5], points[4], aabb_normals[5]);

    m_indices.resize(36);

    RenderIndex indices[36] = {
      0,  1,  2,  0,  2,  3,
      4,  5,  6,  4,  6,  7,
      8,  9, 10,  8, 10, 11,
      12, 13, 14, 12, 14, 15,
      16, 17, 18, 16, 18, 19,
      20, 21, 22, 10, 22, 23,
    };


    Array<RenderIndex>::iterator j = m_indices.begin();
    for(RenderIndex* i = indices; i != indices+(sizeof(indices)/sizeof(RenderIndex)); ++i)
    {
      *j++ = *i;
    }

    m_shader = "";

    UpdateAABB();
  }
};


typedef std::pair<CopiedString, int> PicoModelKey;


class PicoModel :
public Cullable,
public Bounded
{
  typedef std::vector<PicoSurface*> surfaces_t;
  surfaces_t m_surfaces;

  AABB m_aabb_local;
public:
  Callback m_lightsChanged;

  PicoModel()
  {
    constructNull();
  }
  PicoModel(picoModel_t* model)
  {
    CopyPicoModel(model);
  }
  ~PicoModel()
  {
    for(surfaces_t::iterator i = m_surfaces.begin(); i != m_surfaces.end(); ++i)
      delete *i;
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

  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return test.TestAABB(m_aabb_local, localToWorld);
  }

  virtual const AABB& localAABB() const
  {
    return m_aabb_local;
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld, std::vector<Shader*> states) const
  {
    for(surfaces_t::const_iterator i = m_surfaces.begin(); i != m_surfaces.end(); ++i)
    {
      if((*i)->intersectVolume(volume, localToWorld) != c_volumeOutside)
      {
        (*i)->render(renderer, localToWorld, states[i - m_surfaces.begin()]);
      }
    }
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

private:
  void CopyPicoModel(picoModel_t* model)
  {
    m_aabb_local = AABB();

    /* each surface on the model will become a new map drawsurface */
    int numSurfaces = PicoGetModelNumSurfaces( model );
    //%  SYs_FPrintf( SYS_VRB, "Model %s has %d surfaces\n", name, numSurfaces );
    for(int s = 0; s < numSurfaces; ++s)
    {
      /* get surface */
      picoSurface_t* surface = PicoGetModelSurface( model, s );
      if( surface == 0 )
        continue;
      
      /* only handle triangle surfaces initially (fixme: support patches) */
      if( PicoGetSurfaceType( surface ) != PICO_TRIANGLES )
        continue;
      
      /* fix the surface's normals */
      PicoFixSurfaceNormals( surface );
      
      PicoSurface* picosurface = new PicoSurface(surface);
      aabb_extend_by_aabb_safe(m_aabb_local, picosurface->localAABB());
      m_surfaces.push_back(picosurface);
    }
  }
  void constructNull()
  {
    PicoSurface* picosurface = new PicoSurface();
    m_aabb_local = picosurface->localAABB();
    m_surfaces.push_back(picosurface);
  }
};

inline void Surface_addLight(PicoSurface& surface, VectorLightList& lights, const Matrix4& localToWorld, const RendererLight& light)
{
  if(light.testAABB(aabb_for_oriented_aabb(surface.localAABB(), localToWorld)))
  {
    lights.addLight(light);
  }
}

class PicoModelInstance :
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
      InstanceContainedCast<PicoModelInstance, Bounded>::install(m_casts);
      InstanceContainedCast<PicoModelInstance, Cullable>::install(m_casts);
      InstanceStaticCast<PicoModelInstance, Renderable>::install(m_casts);
      InstanceStaticCast<PicoModelInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<PicoModelInstance, SkinnedModel>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

  PicoModel& m_picomodel;

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

  PicoModelInstance(const PicoModelInstance&);
  PicoModelInstance operator=(const PicoModelInstance&);
public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  void* m_test;

  Bounded& get(NullType<Bounded>)
  {
    return m_picomodel;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_picomodel;
  }

  void lightsChanged()
  {
    m_lightList->lightsChanged();
  }
  typedef MemberCaller<PicoModelInstance, &PicoModelInstance::lightsChanged> LightsChangedCaller;

  void constructRemaps()
  {
    ASSERT_MESSAGE(m_skins.size() == m_picomodel.size(), "ERROR");
    ModelSkin* skin = NodeTypeCast<ModelSkin>::cast(path().parent());
    if(skin != 0 && skin->realised())
    {
      SurfaceRemaps::iterator j = m_skins.begin();
      for(PicoModel::const_iterator i = m_picomodel.begin(); i != m_picomodel.end(); ++i, ++j)
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
    ASSERT_MESSAGE(m_skins.size() == m_picomodel.size(), "ERROR");
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
    destroyRemaps();
    constructRemaps();
  }

  PicoModelInstance(const scene::Path& path, scene::Instance* parent, PicoModel& picomodel) :
    Instance(path, parent, this, StaticTypeCasts::instance().get()),
    m_picomodel(picomodel),
    m_surfaceLightLists(m_picomodel.size()),
    m_skins(m_picomodel.size())
  {
    m_lightList = &GlobalShaderCache().attach(*this);
    m_picomodel.m_lightsChanged = LightsChangedCaller(*this);

    Instance::setTransformChangedCallback(LightsChangedCaller(*this));

    constructRemaps();
  }
  ~PicoModelInstance()
  {
    destroyRemaps();

    Instance::setTransformChangedCallback(Callback());

    m_picomodel.m_lightsChanged = Callback();
    GlobalShaderCache().detach(*this);
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    SurfaceLightLists::const_iterator j = m_surfaceLightLists.begin();
    SurfaceRemaps::const_iterator k = m_skins.begin();
    for(PicoModel::const_iterator i = m_picomodel.begin(); i != m_picomodel.end(); ++i, ++j, ++k)
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
    m_picomodel.testSelect(selector, test, Instance::localToWorld());
  }

  bool testLight(const RendererLight& light) const
  {
    return light.testAABB(worldAABB());
  }
  void insertLight(const RendererLight& light)
  {
    const Matrix4& localToWorld = Instance::localToWorld();
    SurfaceLightLists::iterator j = m_surfaceLightLists.begin();
    for(PicoModel::const_iterator i = m_picomodel.begin(); i != m_picomodel.end(); ++i)
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

class PicoModelNode : public scene::Node::Symbiot, public scene::Instantiable
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<PicoModelNode, scene::Instantiable>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  PicoModel m_picomodel;

public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  PicoModelNode() : m_node(this, this, StaticTypeCasts::instance().get())
  {
  }
  PicoModelNode(picoModel_t* model) : m_node(this, this, StaticTypeCasts::instance().get()), m_picomodel(model)
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

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new PicoModelInstance(path, parent, m_picomodel);
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


#if 0

template<typename Key, typename Type>
class create_new
{
public:
  static Type* construct(const Key& key)
  {
    return new Type(key);
  }
  static void destroy(Type* value)
  {
    delete value;
  }
};

template<typename Key, typename Type, typename creation_policy = create_new<Key, Type> >
class cache_element : public creation_policy
{
public:
  inline cache_element() : m_count(0), m_value(0) {}
  inline ~cache_element()
  {
    ASSERT_MESSAGE(m_count == 0 , "destroyed a reference before it was released\n");
    if(m_count > 0)
      destroy();
  }
  inline Type* capture(const Key& key)
  {
    if(++m_count == 1)
      construct(key);
    return m_value;
  }
  inline void release()
  {
    ASSERT_MESSAGE(!empty(), "failed to release reference - not found in cache\n");
    if(--m_count == 0)
      destroy();
  }
  inline bool empty()
  {
    return m_count == 0;
  }
  inline void refresh(const Key& key)
  {
    m_value->refresh(key);
  }
private:
  inline void construct(const Key& key)
  {
    m_value = creation_policy::construct(key);
  }
  inline void destroy()
  {
    creation_policy::destroy(m_value);
  }

  std::size_t m_count;
  Type* m_value;
};

class create_picomodel
{
  typedef PicoModelKey key_type;
  typedef PicoModel value_type;
public:
  static value_type* construct(const key_type& key)
  {
    picoModel_t* picomodel = PicoLoadModel(const_cast<char*>(key.first.c_str()), key.second);
    value_type* value = new value_type(picomodel);
    PicoFreeModel(picomodel);
    return value;
  }
  static void destroy(value_type* value)
  {
    delete value;
  }
};

#include <map>

class ModelCache
{
  typedef PicoModel value_type;
  
public:
  typedef PicoModelKey key_type;
  typedef cache_element<key_type, value_type, create_picomodel> elem_type;
  typedef std::map<key_type, elem_type> cache_type;
  
  value_type* capture(const key_type& key)
  {
    return m_cache[key].capture(key);
  }
  void release(const key_type& key)
  {
    m_cache[key].release();
  }

private:
  cache_type m_cache;
};

ModelCache g_model_cache;



typedef struct remap_s {
  char m_remapbuff[64+1024];
  char *original;
  char *remap;
} remap_t;

class RemapWrapper :
public Cullable,
public Bounded
{
public:
  RemapWrapper(const char* name)
  {
    parse_namestr(name);

    m_model = g_model_cache.capture(ModelCache::key_type(m_name, m_frame));

    construct_shaders();
  }
  virtual ~RemapWrapper()
  {
    g_model_cache.release(ModelCache::key_type(m_name, m_frame));

    for(shaders_t::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i)
    {
      GlobalShaderCache().release((*i).c_str());
    }

    for(remaps_t::iterator j = m_remaps.begin(); j != m_remaps.end(); ++j)
    {
      delete (*j);
    }
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return m_model->intersectVolume(test, localToWorld);
  }

  virtual const AABB& localAABB() const
  {
    return m_model->localAABB();
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    m_model->render(renderer, volume, localToWorld, m_states);
  }

  void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld)
  {
    m_model->testSelect(selector, test, localToWorld);
  }

private:
  void add_remap(const char *remap)
  {
    const char *ch;
    remap_t *pRemap;

    ch = remap;

    while( *ch && *ch != ';' )
      ch++;

    if( *ch == '\0' ) {
      // bad remap
      globalErrorStream() << "WARNING: Shader _remap key found in a model entity without a ; character\n";
    } else {
      pRemap = new remap_t;

      strncpy( pRemap->m_remapbuff, remap, sizeof(pRemap->m_remapbuff) );

      pRemap->m_remapbuff[ch - remap] = '\0';

      pRemap->original = pRemap->m_remapbuff;
      pRemap->remap = pRemap->m_remapbuff + ( ch - remap ) + 1;

      m_remaps.push_back( pRemap );
    }
  }

  void parse_namestr(const char *name)
  {
    const char *ptr, *s;
    bool hasName, hasFrame;

    hasName = hasFrame = false;

    m_frame = 0;

    for( s = ptr = name; ; ++ptr )
    {
      if( !hasName && (*ptr == ':' || *ptr == '\0'))
      {
        // model name
        hasName = true;
        m_name = CopiedString(s, ptr);
        s = ptr + 1;
      }
      else if(*ptr == '?' || *ptr == '\0')
      {
        // model frame
        hasFrame = true;
        m_frame = atoi(CopiedString(s, ptr).c_str());
        s = ptr + 1;
      }
      else if(*ptr == '&' || *ptr == '\0') 
      {
        // a remap
        add_remap(CopiedString(s, ptr).c_str());
        s = ptr + 1;
      }

      if(*ptr == '\0')
        break; 
    }
  }

  void construct_shaders()
  {
    const char* global_shader = shader_for_remap("*");

    m_shaders.reserve(m_model->size());
    m_states.reserve(m_model->size());
    for(PicoModel::iterator i = m_model->begin(); i != m_model->end(); ++i)
    {
      const char* shader = shader_for_remap((*i)->getShader());
      m_shaders.push_back(
        (shader[0] != '\0')
        ? shader
        : (global_shader[0] != '\0')
          ? global_shader
          : (*i)->getShader());
      m_states.push_back(GlobalShaderCache().capture(m_shaders.back().c_str()));
    }
  }
  
  inline const char* shader_for_remap(const char* remap)
  {
    for(remaps_t::iterator i = m_remaps.begin(); i != m_remaps.end(); ++i)
    {
      if(shader_equal(remap, (*i)->original))
      {
        return (*i)->remap;
      }
    }
    return "";
  }

  CopiedString m_name;
  int m_frame;
  PicoModel* m_model;

  typedef std::vector<remap_t*> remaps_t;
  remaps_t m_remaps;
  typedef std::vector<CopiedString> shaders_t;
  shaders_t m_shaders;
  typedef std::vector<Shader*> states_t;
  states_t m_states;
};

class RemapWrapperInstance : public scene::Instance, public Renderable, public SelectionTestable
{
  RemapWrapper& m_remapwrapper;
public:
  RemapWrapperInstance(const scene::Path& path, scene::Instance* parent, RemapWrapper& remapwrapper) : Instance(path, parent), m_remapwrapper(remapwrapper)
  {
    scene::Instance::m_cullable = &m_remapwrapper;
    scene::Instance::m_render = this;
    scene::Instance::m_select = this;
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_remapwrapper.render(renderer, volume, Instance::localToWorld());
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    renderSolid(renderer, volume);
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    m_remapwrapper.testSelect(selector, test, Instance::localToWorld());
  }
};

class RemapWrapperNode : public scene::Node::Symbiot, public scene::Instantiable
{
  scene::Node m_node;
  typedef RemapWrapperInstance instance_type;
  InstanceSet m_instances;
  RemapWrapper m_remapwrapper;
public:
  RemapWrapperNode(const char* name) : m_node(this), m_remapwrapper(name)
  {
    m_node.m_instance = this;
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
    return new instance_type(path, parent, m_remapwrapper);
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

scene::Node& LoadRemapModel(const char* name)
{
  return (new RemapWrapperNode(name))->node();
}

#endif


size_t picoInputStreamReam(void* inputStream, unsigned char* buffer, size_t length)
{
  return reinterpret_cast<InputStream*>(inputStream)->read(buffer, length);
}

scene::Node& loadPicoModel(const picoModule_t* module, ArchiveFile& file)
{
  picoModel_t* model = PicoModuleLoadModelStream(module, &file.getInputStream(), picoInputStreamReam, file.size(), 0);
  PicoModelNode* modelNode = new PicoModelNode(model);
  PicoFreeModel(model);
  return modelNode->node();
}
