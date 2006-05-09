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

#if !defined(INCLUDED_PATCH_H)
#define INCLUDED_PATCH_H

/// \file
/// \brief The patch primitive.
///
/// A 2-dimensional matrix of vertices that define a quadratic bezier surface.
/// The Boundary-Representation of this primitive is a triangle mesh.
/// The surface is recursively tesselated until the angle between each triangle
/// edge is smaller than a specified tolerance.


#include "nameable.h"
#include "ifilter.h"
#include "imap.h"
#include "ipatch.h"
#include "cullable.h"
#include "renderable.h"
#include "editable.h"
#include "selectable.h"

#include "debugging/debugging.h"

#include <set>

#include "math/frustum.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "stream/textstream.h"
#include "xml/xmlelement.h"
#include "scenelib.h"
#include "transformlib.h"
#include "instancelib.h"
#include "selectionlib.h"
#include "traverselib.h"
#include "render.h"
#include "stringio.h"
#include "shaderlib.h"
#include "generic/callback.h"
#include "signal/signalfwd.h"
#include "texturelib.h"
#include "xml/ixml.h"
#include "dragplanes.h"

enum EPatchType
{
  ePatchTypeQuake3,
  ePatchTypeDoom3,
};

extern int g_PatchSubdivideThreshold;


#define MIN_PATCH_WIDTH 3
#define MIN_PATCH_HEIGHT 3

extern std::size_t MAX_PATCH_WIDTH;
extern std::size_t MAX_PATCH_HEIGHT;

#define MAX_PATCH_ROWCTRL (((MAX_PATCH_WIDTH-1)-1)/2)
#define MAX_PATCH_COLCTRL (((MAX_PATCH_HEIGHT-1)-1)/2)

enum EPatchCap
{
  eCapBevel,
  eCapEndCap,
  eCapIBevel,
  eCapIEndCap,
  eCapCylinder,
};

enum EPatchPrefab
{
  ePlane,
  eBevel,
  eEndCap,
  eCylinder,
  eDenseCylinder,
  eVeryDenseCylinder,
  eSqCylinder,
  eCone,
  eSphere,
};

enum EMatrixMajor
{
  ROW, COL,
};

struct BezierCurve
{
  Vector3 crd;
  Vector3 left;
  Vector3 right;
};

const std::size_t BEZIERCURVETREE_MAX_INDEX = 1 << ((sizeof(std::size_t) * 8) - 1);

struct BezierCurveTree
{
  std::size_t index;
  BezierCurveTree* left;
  BezierCurveTree* right;
};

inline bool BezierCurveTree_isLeaf(const BezierCurveTree* node)
{
  return node->left == 0 && node->right == 0;
}

void BezierCurveTree_Delete(BezierCurveTree *pCurve);


inline VertexPointer vertexpointer_arbitrarymeshvertex(const ArbitraryMeshVertex* array)
{
  return VertexPointer(VertexPointer::pointer(&array->vertex), sizeof(ArbitraryMeshVertex));
}

typedef PatchControl* PatchControlIter;
typedef const PatchControl* PatchControlConstIter;

inline void copy_ctrl(PatchControlIter ctrl, PatchControlConstIter begin, PatchControlConstIter end)
{
  std::copy(begin, end, ctrl);
}

const Colour4b colour_corner(0, 255, 0, 255);
const Colour4b colour_inside(255, 0, 255, 255);

class Patch;

class PatchFilter
{
public:
  virtual bool filter(const Patch& patch) const = 0;
};

bool patch_filtered(Patch& patch);
void add_patch_filter(PatchFilter& filter, int mask, bool invert = false);

void Patch_addTextureChangedCallback(const SignalHandler& handler);
void Patch_textureChanged();

inline void BezierCurveTreeArray_deleteAll(Array<BezierCurveTree*>& curveTrees)
{
  for(Array<BezierCurveTree*>::iterator i = curveTrees.begin(); i != curveTrees.end(); ++i)
  {
    BezierCurveTree_Delete(*i);
  }
}

inline void PatchControlArray_invert(Array<PatchControl>& ctrl, std::size_t width, std::size_t height)
{
  Array<PatchControl> tmp(width);

  PatchControlIter from = ctrl.data() + (width * (height - 1));
  PatchControlIter to = ctrl.data();
  for(std::size_t h = 0; h != ((height - 1) >> 1); ++h, to += width, from -= width)
  {
    copy_ctrl(tmp.data(), to, to + width);
    copy_ctrl(to, from, from + width);
    copy_ctrl(from, tmp.data(), tmp.data() + width);
  }
}

class PatchTesselation
{
public:
  PatchTesselation()
    : m_numStrips(0), m_lenStrips(0), m_nArrayWidth(0), m_nArrayHeight(0)
  {
  }
  Array<ArbitraryMeshVertex> m_vertices;
  Array<RenderIndex> m_indices;
  std::size_t m_numStrips;
  std::size_t m_lenStrips;

  Array<std::size_t> m_arrayWidth;
  std::size_t m_nArrayWidth;
  Array<std::size_t> m_arrayHeight;
  std::size_t m_nArrayHeight;

  Array<BezierCurveTree*> m_curveTreeU;
  Array<BezierCurveTree*> m_curveTreeV;
};

class RenderablePatchWireframe : public OpenGLRenderable
{
  PatchTesselation& m_tess;
public:
  RenderablePatchWireframe(PatchTesselation& tess) : m_tess(tess)
  {
  }
  void render(RenderStateFlags state) const
  {
    {
  #if NV_DRIVER_BUG
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 0);
  #endif

      std::size_t n = 0;
      glVertexPointer(3, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->vertex);
      for(std::size_t i = 0; i <= m_tess.m_curveTreeV.size(); ++i)
      {
        glDrawArrays(GL_LINE_STRIP, GLint(n), GLsizei(m_tess.m_nArrayWidth));

        if(i == m_tess.m_curveTreeV.size()) break;

        if(!BezierCurveTree_isLeaf(m_tess.m_curveTreeV[i]))
          glDrawArrays(GL_LINE_STRIP, GLint(m_tess.m_curveTreeV[i]->index), GLsizei(m_tess.m_nArrayWidth));

        n += (m_tess.m_arrayHeight[i]*m_tess.m_nArrayWidth);
      
      }
    }

    {
      const ArbitraryMeshVertex* p = m_tess.m_vertices.data();
      std::size_t n = m_tess.m_nArrayWidth * sizeof(ArbitraryMeshVertex);
      for(std::size_t i = 0; i <= m_tess.m_curveTreeU.size(); ++i)
      {
        glVertexPointer(3, GL_FLOAT, GLsizei(n), &p->vertex);
        glDrawArrays(GL_LINE_STRIP, 0, GLsizei(m_tess.m_nArrayHeight));

        if(i == m_tess.m_curveTreeU.size()) break;

        if(!BezierCurveTree_isLeaf(m_tess.m_curveTreeU[i]))
        {
          glVertexPointer(3, GL_FLOAT, GLsizei(n), &(m_tess.m_vertices.data() + (m_tess.m_curveTreeU[i]->index))->vertex);
          glDrawArrays(GL_LINE_STRIP, 0, GLsizei(m_tess.m_nArrayHeight));
        }

        p += m_tess.m_arrayWidth[i];
      }
    }
  }
};

class RenderablePatchFixedWireframe : public OpenGLRenderable
{
  PatchTesselation& m_tess;
public:
  RenderablePatchFixedWireframe(PatchTesselation& tess) : m_tess(tess)
  {
  }
  void render(RenderStateFlags state) const
  {
    glVertexPointer(3, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->vertex);
    const RenderIndex* strip_indices = m_tess.m_indices.data();
    for(std::size_t i = 0; i<m_tess.m_numStrips; i++, strip_indices += m_tess.m_lenStrips)
    {
      glDrawElements(GL_QUAD_STRIP, GLsizei(m_tess.m_lenStrips), RenderIndexTypeID, strip_indices);
    }
  }
};

class RenderablePatchSolid : public OpenGLRenderable
{
  PatchTesselation& m_tess;
public:
  RenderablePatchSolid(PatchTesselation& tess) : m_tess(tess)
  {
  }
  void RenderNormals() const;
  void render(RenderStateFlags state) const
  {
#if 0
    if((state & RENDER_FILL) == 0)
    {
      RenderablePatchWireframe(m_tess).render(state);
    }
    else
#endif
    {
      if((state & RENDER_BUMP) != 0)
      {
        if(GlobalShaderCache().useShaderLanguage())
        {
          glNormalPointer(GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->normal);
          glVertexAttribPointerARB(c_attr_TexCoord0, 2, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->texcoord);
          glVertexAttribPointerARB(c_attr_Tangent, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->tangent);
          glVertexAttribPointerARB(c_attr_Binormal, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->bitangent);
        }
        else
        {
          glVertexAttribPointerARB(11, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->normal);
          glVertexAttribPointerARB(8, 2, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->texcoord);
          glVertexAttribPointerARB(9, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->tangent);
          glVertexAttribPointerARB(10, 3, GL_FLOAT, 0, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->bitangent);
        }
      }
      else
      {
        glNormalPointer(GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->texcoord);
      }
      glVertexPointer(3, GL_FLOAT, sizeof(ArbitraryMeshVertex), &m_tess.m_vertices.data()->vertex);
      const RenderIndex* strip_indices = m_tess.m_indices.data();
      for(std::size_t i = 0; i<m_tess.m_numStrips; i++, strip_indices += m_tess.m_lenStrips)
      {
        glDrawElements(GL_QUAD_STRIP, GLsizei(m_tess.m_lenStrips), RenderIndexTypeID, strip_indices);
      }
    }
 
#if defined(_DEBUG)
    RenderNormals();
#endif
  }
};

// parametric surface defined by quadratic bezier control curves
class Patch :
  public XMLImporter,
  public XMLExporter,
  public TransformNode,
  public Bounded,
  public Cullable,
  public Snappable,
  public Undoable,
  public Filterable,
  public Nameable
{
  class xml_state_t
  {
  public:
    enum EState
    {
      eDefault,
      ePatch,
      eMatrix,
      eShader,
    };
    xml_state_t(EState state)
      : m_state(state)
    {}
    EState state() const
    {
      return m_state;
    }
    const char* content() const
    {
      return m_content.c_str();
    }
    std::size_t write(const char* buffer, std::size_t length)
    {
      return m_content.write(buffer, length);
    }
  private:
    EState m_state;
    StringOutputStream m_content;
  };

  std::vector<xml_state_t> m_xml_state;

  typedef Array<PatchControl> PatchControlArray;

  class SavedState : public UndoMemento
  {
  public:
    SavedState(
      std::size_t width,
      std::size_t height,
      const PatchControlArray& ctrl,
      const char* shader,
      bool patchDef3,
      std::size_t subdivisions_x,
      std::size_t subdivisions_y
    ) :
      m_width(width),
      m_height(height),
      m_shader(shader),
      m_ctrl(ctrl),
      m_patchDef3(patchDef3),
      m_subdivisions_x(subdivisions_x),
      m_subdivisions_y(subdivisions_y)
    {
    }

    void release()
    {
      delete this;
    }

    std::size_t m_width, m_height;
    CopiedString m_shader;
    PatchControlArray m_ctrl;
    bool m_patchDef3;
    std::size_t m_subdivisions_x;
    std::size_t m_subdivisions_y;
  };

public:
  class Observer
  {
  public:
    virtual void allocate(std::size_t size) = 0;
  };

private:
  typedef UniqueSet<Observer*> Observers;
  Observers m_observers;

  scene::Node* m_node;

  AABB m_aabb_local; // local bbox

  CopiedString m_shader;
  Shader* m_state;

  std::size_t m_width;
  std::size_t m_height;
public:
  bool m_patchDef3;
  std::size_t m_subdivisions_x;
  std::size_t m_subdivisions_y;
private:
  
  UndoObserver* m_undoable_observer;
  MapFile* m_map;

  // dynamically allocated array of control points, size is m_width*m_height
  PatchControlArray m_ctrl;
  PatchControlArray m_ctrlTransformed;

  PatchTesselation m_tess;
  RenderablePatchSolid m_render_solid;
  RenderablePatchWireframe m_render_wireframe;
  RenderablePatchFixedWireframe m_render_wireframe_fixed;

  static Shader* m_state_ctrl;
  static Shader* m_state_lattice;
  VertexBuffer<PointVertex> m_ctrl_vertices;
  RenderableVertexBuffer m_render_ctrl;
  IndexBuffer m_lattice_indices;
  RenderableIndexBuffer m_render_lattice;

  bool m_bOverlay;

  bool m_transformChanged;
  Callback m_evaluateTransform;
  Callback m_boundsChanged;

  void construct()
  {
    m_bOverlay = false;
    m_width = m_height = 0;

    m_patchDef3 = false;
    m_subdivisions_x = 0;
    m_subdivisions_y = 0;

    check_shader();
    captureShader();

    m_xml_state.push_back(xml_state_t::eDefault);
  }

public:
  Callback m_lightsChanged;

  static int m_CycleCapIndex;// = 0;
  static EPatchType m_type;

  STRING_CONSTANT(Name, "Patch");

  Patch(scene::Node& node, const Callback& evaluateTransform, const Callback& boundsChanged) :
    m_node(&node),
    m_shader(texdef_name_default()),
    m_state(0),
    m_undoable_observer(0),
    m_map(0),
    m_render_solid(m_tess),
    m_render_wireframe(m_tess),
    m_render_wireframe_fixed(m_tess),
    m_render_ctrl(GL_POINTS, m_ctrl_vertices),
    m_render_lattice(GL_LINES, m_lattice_indices, m_ctrl_vertices),
    m_transformChanged(false),
    m_evaluateTransform(evaluateTransform),
    m_boundsChanged(boundsChanged)
  {
    construct();
  }
  Patch(const Patch& other, scene::Node& node, const Callback& evaluateTransform, const Callback& boundsChanged) :
    m_node(&node),
    m_shader(texdef_name_default()),
    m_state(0),
    m_undoable_observer(0),
    m_map(0),
    m_render_solid(m_tess),
    m_render_wireframe(m_tess),
    m_render_wireframe_fixed(m_tess),
    m_render_ctrl(GL_POINTS, m_ctrl_vertices),
    m_render_lattice(GL_LINES, m_lattice_indices, m_ctrl_vertices),
    m_transformChanged(false),
    m_evaluateTransform(evaluateTransform),
    m_boundsChanged(boundsChanged)
  {
    construct();

    m_patchDef3 = other.m_patchDef3;
    m_subdivisions_x = other.m_subdivisions_x;
    m_subdivisions_y = other.m_subdivisions_y;
    setDims(other.m_width, other.m_height);
    copy_ctrl(m_ctrl.data(), other.m_ctrl.data(), other.m_ctrl.data()+(m_width*m_height));
    SetShader(other.m_shader.c_str());
    controlPointsChanged();
  }

  Patch(const Patch& other) :
    XMLImporter(other),
    XMLExporter(other),
    TransformNode(other),
    Bounded(other),
    Cullable(other),
    Undoable(other),
    Filterable(other),
    Nameable(other),
    m_state(0),
    m_undoable_observer(0),
    m_map(0),
    m_render_solid(m_tess),
    m_render_wireframe(m_tess),
    m_render_wireframe_fixed(m_tess),
    m_render_ctrl(GL_POINTS, m_ctrl_vertices),
    m_render_lattice(GL_LINES, m_lattice_indices, m_ctrl_vertices),
    m_transformChanged(false),
    m_evaluateTransform(other.m_evaluateTransform),
    m_boundsChanged(other.m_boundsChanged)
  {
    m_bOverlay = false;

    m_patchDef3 = other.m_patchDef3;
    m_subdivisions_x = other.m_subdivisions_x;
    m_subdivisions_y = other.m_subdivisions_y;
    setDims(other.m_width, other.m_height);
    copy_ctrl(m_ctrl.data(), other.m_ctrl.data(), other.m_ctrl.data()+(m_width*m_height));
    SetShader(other.m_shader.c_str());
    controlPointsChanged();
  }

  ~Patch()
  {
    BezierCurveTreeArray_deleteAll(m_tess.m_curveTreeU);
    BezierCurveTreeArray_deleteAll(m_tess.m_curveTreeV);

    releaseShader();

    ASSERT_MESSAGE(m_observers.empty(), "Patch::~Patch: observers still attached");
  }

  InstanceCounter m_instanceCounter;
  void instanceAttach(const scene::Path& path)
  {
    if(++m_instanceCounter.m_count == 1)
    {
      m_state->incrementUsed();
      m_map = path_find_mapfile(path.begin(), path.end());
      m_undoable_observer = GlobalUndoSystem().observer(this);
      GlobalFilterSystem().registerFilterable(*this);
    }
    else
    {
      ASSERT_MESSAGE(path_find_mapfile(path.begin(), path.end()) == m_map, "node is instanced across more than one file");
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      m_map = 0;
      m_undoable_observer = 0;
      GlobalUndoSystem().release(this);
      GlobalFilterSystem().unregisterFilterable(*this);
      m_state->decrementUsed();
    }
  }

  const char* name() const
  {
    return "patch";
  }
  void attach(const NameCallback& callback)
  {
  }
  void detach(const NameCallback& callback)
  {
  }

  void attach(Observer* observer)
  {
    observer->allocate(m_width * m_height);
    
    m_observers.insert(observer);
  }
  void detach(Observer* observer)
  {
    m_observers.erase(observer);
  }

  void updateFiltered()
  {
    if(m_node != 0)
    {
      if(patch_filtered(*this))
      {
          m_node->enable(scene::Node::eFiltered);
      }
      else
      {
        m_node->disable(scene::Node::eFiltered);
      }
    }
  }

  void onAllocate(std::size_t size)
  {
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->allocate(size);
    }
  }
 
  const Matrix4& localToParent() const
  {
    return g_matrix4_identity;
  }
  const AABB& localAABB() const
  {
    return m_aabb_local;
  }
  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return test.TestAABB(m_aabb_local, localToWorld);
  }
  void render_solid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_state, Renderer::eFullMaterials);
    renderer.addRenderable(m_render_solid, localToWorld);
  }
  void render_wireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_state, Renderer::eFullMaterials);
    if(m_patchDef3)
    {
      renderer.addRenderable(m_render_wireframe_fixed, localToWorld);
    }
    else
    {
      renderer.addRenderable(m_render_wireframe, localToWorld);
    }
  }

  void render_component(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_state_lattice, Renderer::eWireframeOnly);
    renderer.SetState(m_state_lattice, Renderer::eFullMaterials);
    renderer.addRenderable(m_render_lattice, localToWorld);

    renderer.SetState(m_state_ctrl, Renderer::eWireframeOnly);
    renderer.SetState(m_state_ctrl, Renderer::eFullMaterials);
    renderer.addRenderable(m_render_ctrl, localToWorld);
  }
  void testSelect(Selector& selector, SelectionTest& test)
  {
    SelectionIntersection best;
    IndexPointer::index_type* pIndex = m_tess.m_indices.data();
    for(std::size_t s=0; s<m_tess.m_numStrips; s++)
    {
      test.TestQuadStrip(vertexpointer_arbitrarymeshvertex(m_tess.m_vertices.data()), IndexPointer(pIndex, m_tess.m_lenStrips), best);
      pIndex += m_tess.m_lenStrips;
    }
    if(best.valid())
    {
      selector.addIntersection(best);
    }
  }
  void transform(const Matrix4& matrix)
  {
    for(PatchControlIter i = m_ctrlTransformed.data(); i != m_ctrlTransformed.data() + m_ctrlTransformed.size(); ++i)
    {
      matrix4_transform_point(matrix, (*i).m_vertex);
    }

    if(matrix4_handedness(matrix) == MATRIX4_LEFTHANDED)
    {
      PatchControlArray_invert(m_ctrlTransformed, m_width, m_height);
    }
    UpdateCachedData();
  }
  void transformChanged()
  {
    m_transformChanged = true;
    m_lightsChanged();
    SceneChangeNotify();
  }
  typedef MemberCaller<Patch, &Patch::transformChanged> TransformChangedCaller;

  void evaluateTransform()
  {
    if(m_transformChanged)
    {
      m_transformChanged = false;
      revertTransform();
      m_evaluateTransform();
    }
  }

  void revertTransform()
  {
    m_ctrlTransformed = m_ctrl;
  }
  void freezeTransform()
  {
    undoSave();
    evaluateTransform();
    ASSERT_MESSAGE(m_ctrlTransformed.size() == m_ctrl.size(), "Patch::freeze: size mismatch");
    std::copy(m_ctrlTransformed.begin(), m_ctrlTransformed.end(), m_ctrl.begin());
  }

  void controlPointsChanged()
  {
    transformChanged();
    evaluateTransform();
    UpdateCachedData();
  }
  bool isValid() const;

  void snapto(float snap)
  {
    undoSave();

    for(PatchControlIter i = m_ctrl.data(); i != m_ctrl.data() + m_ctrl.size(); ++i)
    {
      vector3_snap((*i).m_vertex, snap);
    }

    controlPointsChanged();
  }




  void RenderDebug(RenderStateFlags state) const;
  void RenderNormals(RenderStateFlags state) const;

  void pushElement(const XMLElement& element)
  {
    switch(m_xml_state.back().state())
    {
    case xml_state_t::eDefault:
      ASSERT_MESSAGE(string_equal(element.name(), "patch"), "parse error");
      m_xml_state.push_back(xml_state_t::ePatch);
      break;
    case xml_state_t::ePatch:
      if(string_equal(element.name(), "matrix"))
      {
        setDims(atoi(element.attribute("width")), atoi(element.attribute("height")));
        m_xml_state.push_back(xml_state_t::eMatrix);
      }
      else if(string_equal(element.name(), "shader"))
      {
        m_xml_state.push_back(xml_state_t::eShader);
      }
      break;
    default:
      ERROR_MESSAGE("parse error");
    }

  }
  void popElement(const char* name)
  {
    switch(m_xml_state.back().state())
    {
    case xml_state_t::eDefault:
      ERROR_MESSAGE("parse error");
      break;
    case xml_state_t::ePatch:
      break;
    case xml_state_t::eMatrix:
      {
        StringTokeniser content(m_xml_state.back().content());

        for(PatchControlIter i = m_ctrl.data(), end = m_ctrl.data() + m_ctrl.size(); i != end; ++i)
        {
          (*i).m_vertex[0] = string_read_float(content.getToken());
          (*i).m_vertex[1] = string_read_float(content.getToken());
          (*i).m_vertex[2] = string_read_float(content.getToken());
          (*i).m_texcoord[0] = string_read_float(content.getToken());
          (*i).m_texcoord[1] = string_read_float(content.getToken());
        }
        controlPointsChanged();
      }
      break;
    case xml_state_t::eShader:
      {
        SetShader(m_xml_state.back().content());
      }
      break;
    default:
      ERROR_MESSAGE("parse error");
    }

    ASSERT_MESSAGE(!m_xml_state.empty(), "popping empty stack");
    m_xml_state.pop_back();
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    switch(m_xml_state.back().state())
    {
    case xml_state_t::eDefault:
      break;
    case xml_state_t::ePatch:
      break;
    case xml_state_t::eMatrix:
    case xml_state_t::eShader:
      return m_xml_state.back().write(buffer, length);
      break;
    default:
      ERROR_MESSAGE("parse error");
    }
    return length;
  }

  void exportXML(XMLImporter& importer)
  {
    StaticElement patchElement("patch");
    importer.pushElement(patchElement);

    {
      const StaticElement element("shader");
      importer.pushElement(element);
      importer.write(m_shader.c_str(), strlen(m_shader.c_str()));
      importer.popElement(element.name());
    }

    {
      char width[16], height[16];
      sprintf(width, "%u", Unsigned(m_width));
      sprintf(height, "%u", Unsigned(m_height));
      StaticElement element("matrix");
      element.insertAttribute("width", width);
      element.insertAttribute("height", height);

      importer.pushElement(element);
      {
        for(PatchControlIter i = m_ctrl.data(), end = m_ctrl.data() + m_ctrl.size(); i != end; ++i)
        {
          importer << (*i).m_vertex[0]
            << ' ' << (*i).m_vertex[1]
            << ' ' << (*i).m_vertex[2]
            << ' ' << (*i).m_texcoord[0]
            << ' ' << (*i).m_texcoord[1];
        }
      }
      importer.popElement(element.name());
    }

    importer.popElement(patchElement.name());
  }

  void UpdateCachedData();

  const char *GetShader() const
  {
    return m_shader.c_str();
  }
  void SetShader(const char* name)
  {
    ASSERT_NOTNULL(name);
  
    if(shader_equal(m_shader.c_str(), name))
      return;

    undoSave();

    if(m_instanceCounter.m_count != 0)
    {
      m_state->decrementUsed();
    }
    releaseShader();
    m_shader = name;
    captureShader();
    if(m_instanceCounter.m_count != 0)
    {
      m_state->incrementUsed();
    }

    check_shader();
    Patch_textureChanged();
  }
  int getShaderFlags() const
  {
    if(m_state != 0)
    {
      return m_state->getFlags();
    }
    return 0;
  }

  typedef PatchControl* iterator;
  typedef const PatchControl* const_iterator;

  iterator begin()
  {
    return m_ctrl.data();
  }
  const_iterator begin() const
  {
    return m_ctrl.data();
  }
  iterator end()
  {
    return m_ctrl.data() + m_ctrl.size();
  }
  const_iterator end() const
  {
    return m_ctrl.data() + m_ctrl.size();
  }

  PatchControlArray& getControlPoints()
  {
    return m_ctrl;
  }
  PatchControlArray& getControlPointsTransformed()
  {
    return m_ctrlTransformed;
  }

  void setDims (std::size_t w, std::size_t h);
  std::size_t getWidth() const
  {
    return m_width;
  }
  std::size_t getHeight() const
  {
    return m_height;
  }
  PatchControl& ctrlAt(std::size_t row, std::size_t col)
  {
    return m_ctrl[row*m_width+col];
  } 
  const PatchControl& ctrlAt(std::size_t row, std::size_t col) const
  {
    return m_ctrl[row*m_width+col];
  } 
 
  void ConstructPrefab(const AABB& aabb, EPatchPrefab eType, int axis, std::size_t width = 3, std::size_t height = 3);
  void constructPlane(const AABB& aabb, int axis, std::size_t width, std::size_t height);
  void InvertMatrix();
  void TransposeMatrix();
  void Redisperse(EMatrixMajor mt);
  void InsertRemove(bool bInsert, bool bColumn, bool bFirst);
  Patch* MakeCap(Patch* patch, EPatchCap eType, EMatrixMajor mt, bool bFirst);
  void ConstructSeam(EPatchCap eType, Vector3* p, std::size_t width);
  
  void FlipTexture(int nAxis);
  void TranslateTexture(float s, float t);
  void ScaleTexture(float s, float t);
  void RotateTexture(float angle);
  void SetTextureRepeat(float s, float t); // call with s=1 t=1 for FIT
  void CapTexture();
  void NaturalTexture();
  void ProjectTexture(int nAxis);
 
  void undoSave()
  {
    if(m_map != 0)
    {
      m_map->changed();
    }
    if(m_undoable_observer != 0)
    {
      m_undoable_observer->save(this);
    }
  }

  UndoMemento* exportState() const
  {
    return new SavedState(m_width, m_height, m_ctrl, m_shader.c_str(), m_patchDef3, m_subdivisions_x, m_subdivisions_y);
  }
  void importState(const UndoMemento* state)
  {
    undoSave();

    const SavedState& other = *(static_cast<const SavedState*>(state));

    // begin duplicate of SavedState copy constructor, needs refactoring

    // copy construct
    {
      m_width = other.m_width;
      m_height = other.m_height;
      SetShader(other.m_shader.c_str());
      m_ctrl = other.m_ctrl;
      onAllocate(m_ctrl.size());
      m_patchDef3 = other.m_patchDef3;
      m_subdivisions_x = other.m_subdivisions_x;
      m_subdivisions_y = other.m_subdivisions_y;
    }

    // end duplicate code

    Patch_textureChanged();

    controlPointsChanged();
  }

  static void constructStatic(EPatchType type)
  {
    Patch::m_type = type;
    Patch::m_state_ctrl = GlobalShaderCache().capture("$POINT");
    Patch::m_state_lattice = GlobalShaderCache().capture("$LATTICE");
  }

  static void destroyStatic()
  {
    GlobalShaderCache().release("$LATTICE");
    GlobalShaderCache().release("$POINT");
  }
private:
  void captureShader()
  {
    m_state = GlobalShaderCache().capture(m_shader.c_str());
  }

  void releaseShader()
  {
    GlobalShaderCache().release(m_shader.c_str());
  }

  void check_shader()
  {
    if(!shader_valid(GetShader()))
    {
      globalErrorStream() << "patch has invalid texture name: '" << GetShader() << "'\n";
    }
  }

  void InsertPoints(EMatrixMajor mt, bool bFirst);
  void RemovePoints(EMatrixMajor mt, bool bFirst);
  
  void AccumulateBBox();
  
  void TesselateSubMatrixFixed(ArbitraryMeshVertex* vertices, std::size_t strideX, std::size_t strideY, unsigned int nFlagsX, unsigned int nFlagsY, PatchControl* subMatrix[3][3]);

  // uses binary trees representing bezier curves to recursively tesselate a bezier sub-patch
  void TesselateSubMatrix( const BezierCurveTree *BX, const BezierCurveTree *BY,
                           std::size_t offStartX, std::size_t offStartY,
                           std::size_t offEndX, std::size_t offEndY,
                           std::size_t nFlagsX, std::size_t nFlagsY,
                           Vector3& left, Vector3& mid, Vector3& right,
                           Vector2& texLeft, Vector2& texMid, Vector2& texRight,
                           bool bTranspose );
  
  // tesselates the entire surface
  void BuildTesselationCurves(EMatrixMajor major);
  void accumulateVertexTangentSpace(std::size_t index, Vector3 tangentX[6], Vector3 tangentY[6], Vector2 tangentS[6], Vector2 tangentT[6], std::size_t index0, std::size_t index1);
  void BuildVertexArray();
};

inline bool Patch_importHeader(Patch& patch, Tokeniser& tokeniser)
{
  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "{"));
  return true;
}

inline bool Patch_importShader(Patch& patch, Tokeniser& tokeniser)
{
  // parse shader name
  tokeniser.nextLine();
  const char* texture = tokeniser.getToken();
  if(texture == 0)
  {
    Tokeniser_unexpectedError(tokeniser, texture, "#texture-name");
    return false;
  }
  if(string_equal(texture, "NULL"))
  {
    patch.SetShader(texdef_name_default());
  }
  else
  {
    StringOutputStream shader(string_length(GlobalTexturePrefix_get()) + string_length(texture));
    shader << GlobalTexturePrefix_get() << texture;
    patch.SetShader(shader.c_str());
  }
  return true;
}

inline bool PatchDoom3_importShader(Patch& patch, Tokeniser& tokeniser)
{
  // parse shader name
  tokeniser.nextLine();
  const char *shader = tokeniser.getToken();
  if(shader == 0)
  {
    Tokeniser_unexpectedError(tokeniser, shader, "#shader-name");
    return false;
  }
  if(string_equal(shader, "_emptyname"))
  {
    shader = texdef_name_default();
  }
  patch.SetShader(shader);
  return true;
}

inline bool Patch_importParams(Patch& patch, Tokeniser& tokeniser)
{
  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));

  // parse matrix dimensions
  {
    std::size_t c, r;
    RETURN_FALSE_IF_FAIL(Tokeniser_getSize(tokeniser, c));
    RETURN_FALSE_IF_FAIL(Tokeniser_getSize(tokeniser, r));

    patch.setDims(c, r);
  }

  if(patch.m_patchDef3)
  {
    RETURN_FALSE_IF_FAIL(Tokeniser_getSize(tokeniser, patch.m_subdivisions_x));
    RETURN_FALSE_IF_FAIL(Tokeniser_getSize(tokeniser, patch.m_subdivisions_y));
  }

  // ignore contents/flags/value
  int tmp;
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, tmp));
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, tmp));
  RETURN_FALSE_IF_FAIL(Tokeniser_getInteger(tokeniser, tmp));

  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  return true;
}

inline bool Patch_importMatrix(Patch& patch, Tokeniser& tokeniser)
{
  // parse matrix
  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
  {
    for(std::size_t c=0; c<patch.getWidth(); c++)
    {
      tokeniser.nextLine();
      RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
      for(std::size_t r=0; r<patch.getHeight(); r++)
      {
        RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "("));
    
        RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, patch.ctrlAt(r,c).m_vertex[0]));
        RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, patch.ctrlAt(r,c).m_vertex[1]));
        RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, patch.ctrlAt(r,c).m_vertex[2]));
        RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, patch.ctrlAt(r,c).m_texcoord[0]));
        RETURN_FALSE_IF_FAIL(Tokeniser_getFloat(tokeniser, patch.ctrlAt(r,c).m_texcoord[1]));

        RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
      }
      RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
    }
  }
  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, ")"));
  return true;
}

inline bool Patch_importFooter(Patch& patch, Tokeniser& tokeniser)
{
  patch.controlPointsChanged();

  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "}"));

  tokeniser.nextLine();
  RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "}"));
  return true;
}

class PatchTokenImporter : public MapImporter
{
  Patch& m_patch;
public:
  PatchTokenImporter(Patch& patch) : m_patch(patch)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(Patch_importHeader(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importShader(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importParams(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importMatrix(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importFooter(m_patch, tokeniser));

    return true;
  }
};

class PatchDoom3TokenImporter : public MapImporter
{
  Patch& m_patch;
public:
  PatchDoom3TokenImporter(Patch& patch) : m_patch(patch)
  {
  }
  bool importTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(Patch_importHeader(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(PatchDoom3_importShader(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importParams(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importMatrix(m_patch, tokeniser));
    RETURN_FALSE_IF_FAIL(Patch_importFooter(m_patch, tokeniser));

    return true;
  }
};

inline void Patch_exportHeader(const Patch& patch, TokenWriter& writer)
{
  writer.writeToken("{");
  writer.nextLine();
  writer.writeToken(patch.m_patchDef3 ? "patchDef3" : "patchDef2");
  writer.nextLine();
  writer.writeToken("{");
  writer.nextLine();
}

inline void Patch_exportShader(const Patch& patch, TokenWriter& writer)
{
  // write shader name
  if(*(shader_get_textureName(patch.GetShader())) == '\0')
  {
    writer.writeToken("NULL");
  }
  else
  {
    writer.writeToken(shader_get_textureName(patch.GetShader()));
  }
  writer.nextLine();
}

inline void PatchDoom3_exportShader(const Patch& patch, TokenWriter& writer)
{
  // write shader name
  if(*(shader_get_textureName(patch.GetShader())) == '\0')
  {
    writer.writeString("_emptyname");
  }
  else
  {
    writer.writeString(patch.GetShader());
  }
  writer.nextLine();
}

inline void Patch_exportParams(const Patch& patch, TokenWriter& writer)
{
  // write matrix dimensions
  writer.writeToken("(");
  writer.writeUnsigned(patch.getWidth());
  writer.writeUnsigned(patch.getHeight());
  if(patch.m_patchDef3)
  {
    writer.writeUnsigned(patch.m_subdivisions_x);
    writer.writeUnsigned(patch.m_subdivisions_y);
  }
  writer.writeInteger(0);
  writer.writeInteger(0);
  writer.writeInteger(0);
  writer.writeToken(")");
  writer.nextLine();
}

inline void Patch_exportMatrix(const Patch& patch, TokenWriter& writer)
{
  // write matrix
  writer.writeToken("(");
  writer.nextLine();
  for(std::size_t c=0; c<patch.getWidth(); c++)
  {
    writer.writeToken("(");
    for(std::size_t r=0; r<patch.getHeight(); r++)
    {
      writer.writeToken("(");

      writer.writeFloat(patch.ctrlAt(r,c).m_vertex[0]);
      writer.writeFloat(patch.ctrlAt(r,c).m_vertex[1]);
      writer.writeFloat(patch.ctrlAt(r,c).m_vertex[2]);
      writer.writeFloat(patch.ctrlAt(r,c).m_texcoord[0]);
      writer.writeFloat(patch.ctrlAt(r,c).m_texcoord[1]);

      writer.writeToken(")");
    }
    writer.writeToken(")");
    writer.nextLine();
  }
  writer.writeToken(")");
  writer.nextLine();
}
  
inline void Patch_exportFooter(const Patch& patch, TokenWriter& writer)
{
  writer.writeToken("}");
  writer.nextLine();
  writer.writeToken("}");
  writer.nextLine();
}

class PatchTokenExporter : public MapExporter
{
  const Patch& m_patch;
public:
  PatchTokenExporter(Patch& patch) : m_patch(patch)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    Patch_exportHeader(m_patch, writer);
    Patch_exportShader(m_patch, writer);
    Patch_exportParams(m_patch, writer);
    Patch_exportMatrix(m_patch, writer);
    Patch_exportFooter(m_patch, writer);
  }
};

class PatchDoom3TokenExporter : public MapExporter
{
  const Patch& m_patch;
public:
  PatchDoom3TokenExporter(Patch& patch) : m_patch(patch)
  {
  }
  void exportTokens(TokenWriter& writer) const
  {
    Patch_exportHeader(m_patch, writer);
    PatchDoom3_exportShader(m_patch, writer);
    Patch_exportParams(m_patch, writer);
    Patch_exportMatrix(m_patch, writer);
    Patch_exportFooter(m_patch, writer);
  }
};

class PatchControlInstance
{
public:
  PatchControl* m_ctrl;
  ObservedSelectable m_selectable;

  PatchControlInstance(PatchControl* ctrl, const SelectionChangeCallback& observer)
    : m_ctrl(ctrl), m_selectable(observer)
  {
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    SelectionIntersection best;
    test.TestPoint(m_ctrl->m_vertex, best);
    if(best.valid())
    {
      Selector_add(selector, m_selectable, best);
    }
  }
  void snapto(float snap)
  {
    vector3_snap(m_ctrl->m_vertex, snap);
  }
};


class PatchInstance :
public Patch::Observer,
public scene::Instance,
public Selectable,
public Renderable,
public SelectionTestable,
public ComponentSelectionTestable,
public ComponentEditable,
public ComponentSnappable,
public PlaneSelectable,
public LightCullable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      InstanceStaticCast<PatchInstance, Selectable>::install(m_casts);
      InstanceContainedCast<PatchInstance, Bounded>::install(m_casts);
      InstanceContainedCast<PatchInstance, Cullable>::install(m_casts);
      InstanceStaticCast<PatchInstance, Renderable>::install(m_casts);
      InstanceStaticCast<PatchInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<PatchInstance, ComponentSelectionTestable>::install(m_casts);
      InstanceStaticCast<PatchInstance, ComponentEditable>::install(m_casts);
      InstanceStaticCast<PatchInstance, ComponentSnappable>::install(m_casts);
      InstanceStaticCast<PatchInstance, PlaneSelectable>::install(m_casts);
      InstanceIdentityCast<PatchInstance>::install(m_casts);
      InstanceContainedCast<PatchInstance, Transformable>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };


  Patch& m_patch;
  typedef std::vector<PatchControlInstance> PatchControlInstances;
  PatchControlInstances m_ctrl_instances;

  ObservedSelectable m_selectable;

  DragPlanes m_dragPlanes;

  mutable RenderablePointVector m_render_selected;
  mutable AABB m_aabb_component;

  static Shader* m_state_selpoint;

  const LightList* m_lightList;

  TransformModifier m_transform;
public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  void lightsChanged()
  {
    m_lightList->lightsChanged();
  }
  typedef MemberCaller<PatchInstance, &PatchInstance::lightsChanged> LightsChangedCaller;

  STRING_CONSTANT(Name, "PatchInstance");

  PatchInstance(const scene::Path& path, scene::Instance* parent, Patch& patch) :
    Instance(path, parent, this, StaticTypeCasts::instance().get()),
    m_patch(patch),
    m_selectable(SelectedChangedCaller(*this)),
    m_dragPlanes(SelectedChangedComponentCaller(*this)),
    m_render_selected(GL_POINTS),
    m_transform(Patch::TransformChangedCaller(m_patch), ApplyTransformCaller(*this))
  {
    m_patch.instanceAttach(Instance::path());
    m_patch.attach(this);

    m_lightList = &GlobalShaderCache().attach(*this);
    m_patch.m_lightsChanged = LightsChangedCaller(*this);

    Instance::setTransformChangedCallback(LightsChangedCaller(*this));
  }
  ~PatchInstance()
  {
    Instance::setTransformChangedCallback(Callback());

    m_patch.m_lightsChanged = Callback();
    GlobalShaderCache().detach(*this);

    m_patch.detach(this);
    m_patch.instanceDetach(Instance::path());
  }

  void selectedChanged(const Selectable& selectable)
  {
    GlobalSelectionSystem().getObserver(SelectionSystem::ePrimitive)(selectable);
    GlobalSelectionSystem().onSelectedChanged(*this, selectable);

    Instance::selectedChanged();
  }
  typedef MemberCaller1<PatchInstance, const Selectable&, &PatchInstance::selectedChanged> SelectedChangedCaller;

  void selectedChangedComponent(const Selectable& selectable)
  {
    GlobalSelectionSystem().getObserver(SelectionSystem::eComponent)(selectable);
    GlobalSelectionSystem().onComponentSelection(*this, selectable);
  }
  typedef MemberCaller1<PatchInstance, const Selectable&, &PatchInstance::selectedChangedComponent> SelectedChangedComponentCaller;

  Patch& getPatch()
  {
    return m_patch;
  }
  Bounded& get(NullType<Bounded>)
  {
    return m_patch;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_patch;
  }
  Transformable& get(NullType<Transformable>)
  {
    return m_transform;
  }

  static void constructStatic()
  {
    m_state_selpoint = GlobalShaderCache().capture("$SELPOINT");
  }

  static void destroyStatic()
  {
    GlobalShaderCache().release("$SELPOINT");
  }


  void allocate(std::size_t size)
  {
    m_ctrl_instances.clear();
    m_ctrl_instances.reserve(size);
    for(Patch::iterator i = m_patch.begin(); i != m_patch.end(); ++i)
    {
      m_ctrl_instances.push_back(PatchControlInstance(&(*i), SelectedChangedComponentCaller(*this)));
    }
  }

  void setSelected(bool select)
  {
    m_selectable.setSelected(select);
  }
  bool isSelected() const
  {
    return m_selectable.isSelected();
  }


  void update_selected() const
  {
    m_render_selected.clear();
    Patch::iterator ctrl = m_patch.getControlPointsTransformed().begin();
    for(PatchControlInstances::const_iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i, ++ctrl)
    {
      if((*i).m_selectable.isSelected())
      {
        const Colour4b colour_selected(0, 0, 255, 255);
        m_render_selected.push_back(PointVertex(reinterpret_cast<Vertex3f&>((*ctrl).m_vertex), colour_selected));
      }
    }
  }

#if 0
  void render(Renderer& renderer, const VolumeTest& volume) const
  {
    if(GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
      && m_selectable.isSelected())
    {
      renderer.Highlight(Renderer::eFace, false);

      m_patch.render(renderer, volume, localToWorld());

      if(GlobalSelectionSystem().ComponentMode() == SelectionSystem::eVertex)
      {
        renderer.Highlight(Renderer::ePrimitive, false);

        m_patch.render_component(renderer, volume, localToWorld());

        renderComponentsSelected(renderer, volume);
      }
    }
    else
      m_patch.render(renderer, volume, localToWorld());
  }
#endif

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_patch.evaluateTransform();
    renderer.setLights(*m_lightList);
    m_patch.render_solid(renderer, volume, localToWorld());

    renderComponentsSelected(renderer, volume);
  }

  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_patch.evaluateTransform();
    m_patch.render_wireframe(renderer, volume, localToWorld());

    renderComponentsSelected(renderer, volume);
  }

  void renderComponentsSelected(Renderer& renderer, const VolumeTest& volume) const
  {
    m_patch.evaluateTransform();
    update_selected();
    if(!m_render_selected.empty())
    {
      renderer.Highlight(Renderer::ePrimitive, false);
      renderer.SetState(m_state_selpoint, Renderer::eWireframeOnly);
      renderer.SetState(m_state_selpoint, Renderer::eFullMaterials);
      renderer.addRenderable(m_render_selected, localToWorld());
    }
  }
  void renderComponents(Renderer& renderer, const VolumeTest& volume) const
  {
    m_patch.evaluateTransform();
    if(GlobalSelectionSystem().ComponentMode() == SelectionSystem::eVertex)
    {
      m_patch.render_component(renderer, volume, localToWorld());
    }
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    test.BeginMesh(localToWorld(), true);
    m_patch.testSelect(selector, test);
  }

  void selectCtrl(bool select)
  {
    for(PatchControlInstances::iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
    {
      (*i).m_selectable.setSelected(select);
    }
  }
  bool isSelectedComponents() const
  {
    for(PatchControlInstances::const_iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
    {
      if((*i).m_selectable.isSelected())
      {
        return true;
      }
    }
    return false;
  }
  void setSelectedComponents(bool select, SelectionSystem::EComponentMode mode)
  {
    if(mode == SelectionSystem::eVertex)
    {
      selectCtrl(select);
    }
    else if(mode == SelectionSystem::eFace)
    {
      m_dragPlanes.setSelected(select);
    }
  }
  const AABB& getSelectedComponentsBounds() const
  {
    m_aabb_component = AABB();

    for(PatchControlInstances::const_iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
    {
      if((*i).m_selectable.isSelected())
      {
        aabb_extend_by_point_safe(m_aabb_component, (*i).m_ctrl->m_vertex);
      }
    }

    return m_aabb_component;
  }

  void testSelectComponents(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode)
  {
    test.BeginMesh(localToWorld());

    switch(mode)
    {
    case SelectionSystem::eVertex:
      {
        for(PatchControlInstances::iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
        {
          (*i).testSelect(selector, test);
        }
      }
      break;
    default:
      break;
    }
  }

  bool selectedVertices()
  {
    for(PatchControlInstances::iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
    {
      if((*i).m_selectable.isSelected())
      {
        return true;
      }
    }
    return false;
  }

  void transformComponents(const Matrix4& matrix)
  {
    if(selectedVertices())
    {
      PatchControlIter ctrl = m_patch.getControlPointsTransformed().begin();
      for(PatchControlInstances::iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i, ++ctrl)
      {
        if((*i).m_selectable.isSelected())
        {
          matrix4_transform_point(matrix, (*ctrl).m_vertex);
        }
      }
      m_patch.UpdateCachedData();
    }

    if(m_dragPlanes.isSelected()) // this should only be true when the transform is a pure translation.
    {
      m_patch.transform(m_dragPlanes.evaluateTransform(vector4_to_vector3(matrix.t())));
    }
  }


  void selectPlanes(Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback)
  {
    test.BeginMesh(localToWorld());

    m_dragPlanes.selectPlanes(m_patch.localAABB(), selector, test, selectedPlaneCallback);
  }
  void selectReversedPlanes(Selector& selector, const SelectedPlanes& selectedPlanes)
  {
    m_dragPlanes.selectReversedPlanes(m_patch.localAABB(), selector, selectedPlanes);
  }


  void snapComponents(float snap)
  {
    if(selectedVertices())
    {
      m_patch.undoSave();
      for(PatchControlInstances::iterator i = m_ctrl_instances.begin(); i != m_ctrl_instances.end(); ++i)
      {
        if((*i).m_selectable.isSelected())
        {
          (*i).snapto(snap);
        }
      }
      m_patch.controlPointsChanged();
    }
  }

  void evaluateTransform()
  {
    Matrix4 matrix(m_transform.calculateTransform());

    if(m_transform.getType() == TRANSFORM_PRIMITIVE)
    {
      m_patch.transform(matrix);
    }
    else
    {
      transformComponents(matrix);
    }
  }
  void applyTransform()
  {
    m_patch.revertTransform();
    evaluateTransform();
    m_patch.freezeTransform();
  }
  typedef MemberCaller<PatchInstance, &PatchInstance::applyTransform> ApplyTransformCaller;


  bool testLight(const RendererLight& light) const
  {
    return light.testAABB(worldAABB());
  }
};


template<typename TokenImporter, typename TokenExporter>
class PatchNode :
  public scene::Node::Symbiot,
  public scene::Instantiable,
  public scene::Cloneable
{
  typedef PatchNode<TokenImporter, TokenExporter> Self;

  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<PatchNode, scene::Instantiable>::install(m_casts);
      NodeStaticCast<PatchNode, scene::Cloneable>::install(m_casts);
      NodeContainedCast<PatchNode, Snappable>::install(m_casts);
      NodeContainedCast<PatchNode, TransformNode>::install(m_casts);
      NodeContainedCast<PatchNode, Patch>::install(m_casts);
      NodeContainedCast<PatchNode, XMLImporter>::install(m_casts);
      NodeContainedCast<PatchNode, XMLExporter>::install(m_casts);
      NodeContainedCast<PatchNode, MapImporter>::install(m_casts);
      NodeContainedCast<PatchNode, MapExporter>::install(m_casts);
      NodeContainedCast<PatchNode, Nameable>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };


  scene::Node m_node;
  InstanceSet m_instances;
  Patch m_patch;
  TokenImporter m_importMap;
  TokenExporter m_exportMap;

public:

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  Snappable& get(NullType<Snappable>)
  {
    return m_patch;
  }
  TransformNode& get(NullType<TransformNode>)
  {
    return m_patch;
  }
  Patch& get(NullType<Patch>)
  {
    return m_patch;
  }
  XMLImporter& get(NullType<XMLImporter>)
  {
    return m_patch;
  }
  XMLExporter& get(NullType<XMLExporter>)
  {
    return m_patch;
  }
  MapImporter& get(NullType<MapImporter>)
  {
    return m_importMap;
  }
  MapExporter& get(NullType<MapExporter>)
  {
    return m_exportMap;
  }
  Nameable& get(NullType<Nameable>)
  {
    return m_patch;
  }

  PatchNode(bool patchDef3 = false) :
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_patch(m_node, InstanceSetEvaluateTransform<PatchInstance>::Caller(m_instances), InstanceSet::BoundsChangedCaller(m_instances)),
    m_importMap(m_patch),
    m_exportMap(m_patch)
  {
    m_patch.m_patchDef3 = patchDef3;
  }
  PatchNode(const PatchNode& other) :
    scene::Node::Symbiot(other),
    scene::Instantiable(other),
    scene::Cloneable(other),
    m_node(this, this, StaticTypeCasts::instance().get()),
    m_patch(other.m_patch, m_node, InstanceSetEvaluateTransform<PatchInstance>::Caller(m_instances), InstanceSet::BoundsChangedCaller(m_instances)),
    m_importMap(m_patch),
    m_exportMap(m_patch)
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
  Patch& get()
  {
    return m_patch;
  }
  const Patch& get() const
  {
    return m_patch;
  }
  
  scene::Node& clone() const
  {
    return (new PatchNode(*this))->node();
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new PatchInstance(path, parent, m_patch);
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



typedef PatchNode<PatchTokenImporter, PatchTokenExporter> PatchNodeQuake3;
typedef PatchNode<PatchDoom3TokenImporter, PatchDoom3TokenExporter> PatchNodeDoom3;

inline Patch* Node_getPatch(scene::Node& node)
{
  return NodeTypeCast<Patch>::cast(node);
}

inline PatchInstance* Instance_getPatch(scene::Instance& instance)
{
  return InstanceTypeCast<PatchInstance>::cast(instance);
}

template<typename Functor>
class PatchSelectedVisitor : public SelectionSystem::Visitor
{
  const Functor& m_functor;
public:
  PatchSelectedVisitor(const Functor& functor) : m_functor(functor)
  {
  }
  void visit(scene::Instance& instance) const
  {
    PatchInstance* patch = Instance_getPatch(instance);
    if(patch != 0)
    {
      m_functor(*patch);
    }
  }
};

template<typename Functor>
inline void Scene_forEachSelectedPatch(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(PatchSelectedVisitor<Functor>(functor));
}


template<typename Functor>
class PatchVisibleSelectedVisitor : public SelectionSystem::Visitor
{
  const Functor& m_functor;
public:
  PatchVisibleSelectedVisitor(const Functor& functor) : m_functor(functor)
  {
  }
  void visit(scene::Instance& instance) const
  {
    PatchInstance* patch = Instance_getPatch(instance);
    if(patch != 0
      && instance.path().top().get().visible())
    {
      m_functor(*patch);
    }
  }
};

template<typename Functor>
inline void Scene_forEachVisibleSelectedPatchInstance(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(PatchVisibleSelectedVisitor<Functor>(functor));
}

template<typename Functor>
class PatchForEachWalker : public scene::Graph::Walker
{
  const Functor& m_functor;
public:
  PatchForEachWalker(const Functor& functor) : m_functor(functor)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      Patch* patch = Node_getPatch(path.top());
      if(patch != 0)
      {
        m_functor(*patch);
      }
    }
    return true;
  }
};

template<typename Functor>
inline void Scene_forEachVisiblePatch(const Functor& functor)
{
  GlobalSceneGraph().traverse(PatchForEachWalker<Functor>(functor));
}

template<typename Functor>
class PatchForEachSelectedWalker : public scene::Graph::Walker
{
  const Functor& m_functor;
public:
  PatchForEachSelectedWalker(const Functor& functor) : m_functor(functor)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      Patch* patch = Node_getPatch(path.top());
      if(patch != 0
        && Instance_getSelectable(instance)->isSelected())
      {
        m_functor(*patch);
      }
    }
    return true;
  }
};

template<typename Functor>
inline void Scene_forEachVisibleSelectedPatch(const Functor& functor)
{
  GlobalSceneGraph().traverse(PatchForEachSelectedWalker<Functor>(functor));
}

template<typename Functor>
class PatchForEachInstanceWalker : public scene::Graph::Walker
{
  const Functor& m_functor;
public:
  PatchForEachInstanceWalker(const Functor& functor) : m_functor(functor)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      PatchInstance* patch = Instance_getPatch(instance);
      if(patch != 0)
      {
        m_functor(*patch);
      }
    }
    return true;
  }
};

template<typename Functor>
inline void Scene_forEachVisiblePatchInstance(const Functor& functor)
{
  GlobalSceneGraph().traverse(PatchForEachInstanceWalker<Functor>(functor));
}

#endif
