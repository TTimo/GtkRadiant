/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "brush.h"
#include "signal/signal.h"

Signal0 g_brushTextureChangedCallbacks;

void Brush_addTextureChangedCallback(const SignalHandler& handler)
{
  g_brushTextureChangedCallbacks.connectLast(handler);
}

void Brush_textureChanged()
{
  g_brushTextureChangedCallbacks();
}

QuantiseFunc Face::m_quantise;
EBrushType Face::m_type;
EBrushType FacePlane::m_type;
bool g_brush_texturelock_enabled = false;

EBrushType Brush::m_type;
double Brush::m_maxWorldCoord = 0;
Shader* Brush::m_state_point;
Shader* BrushClipPlane::m_state = 0;
Shader* BrushInstance::m_state_selpoint;
Counter* BrushInstance::m_counter = 0;

FaceInstanceSet g_SelectedFaceInstances;


struct SListNode
{
  SListNode* m_next;
};

class ProximalVertex
{
public:
  const SListNode* m_vertices;

  ProximalVertex(const SListNode* next)
    : m_vertices(next)
  {
  }

  bool operator<(const ProximalVertex& other) const
  {
    if(!(operator==(other)))
    {
      return m_vertices < other.m_vertices;
    }
    return false;
  }
  bool operator==(const ProximalVertex& other) const
  {
    const SListNode* v = m_vertices;
    std::size_t DEBUG_LOOP = 0;
    do
    {
      if(v == other.m_vertices)
        return true;
      v = v->m_next;
      //ASSERT_MESSAGE(DEBUG_LOOP < c_brush_maxFaces, "infinite loop");
      if(!(DEBUG_LOOP < c_brush_maxFaces))
      {
        break;
      }
      ++DEBUG_LOOP;
    }
    while(v != m_vertices);
    return false;
  }
};

typedef Array<SListNode> ProximalVertexArray;
std::size_t ProximalVertexArray_index(const ProximalVertexArray& array, const ProximalVertex& vertex)
{
  return vertex.m_vertices - array.data();
}



inline bool Brush_isBounded(const Brush& brush)
{
  for(Brush::const_iterator i = brush.begin(); i != brush.end(); ++i)
  {
    if(!(*i)->is_bounded())
    {
      return false;
    }
  }
  return true;
}

void Brush::buildBRep()
{
  bool degenerate = buildWindings();

  std::size_t faces_size = 0;
  std::size_t faceVerticesCount = 0;
  for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
  {
    if((*i)->contributes())
    {
      ++faces_size;
    }
    faceVerticesCount += (*i)->getWinding().numpoints;
  }

  if(degenerate || faces_size < 4 || faceVerticesCount != (faceVerticesCount>>1)<<1) // sum of vertices for each face of a valid polyhedron is always even
  {
    m_uniqueVertexPoints.resize(0);

    vertex_clear();
    edge_clear();

    m_edge_indices.resize(0);
    m_edge_faces.resize(0);

    m_faceCentroidPoints.resize(0);
    m_uniqueEdgePoints.resize(0);
    m_uniqueVertexPoints.resize(0);

    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->getWinding().resize(0);
    }
  }
  else
  {
    {
      typedef std::vector<FaceVertexId> FaceVertices;
      FaceVertices faceVertices;
      faceVertices.reserve(faceVerticesCount);

      {
        for(std::size_t i = 0; i != m_faces.size(); ++i)
        {
          for(std::size_t j = 0; j < m_faces[i]->getWinding().numpoints; ++j)
          {
            faceVertices.push_back(FaceVertexId(i, j));
          }
        }
      }

      IndexBuffer uniqueEdgeIndices;
      typedef VertexBuffer<ProximalVertex> UniqueEdges;
      UniqueEdges uniqueEdges;

      uniqueEdgeIndices.reserve(faceVertices.size());
      uniqueEdges.reserve(faceVertices.size());

      {
        ProximalVertexArray edgePairs;
        edgePairs.resize(faceVertices.size());

        {
          for(std::size_t i=0; i<faceVertices.size(); ++i)
          {
            edgePairs[i].m_next = edgePairs.data() + absoluteIndex(next_edge(m_faces, faceVertices[i]));
          }
        }

        {
          UniqueVertexBuffer<ProximalVertex> inserter(uniqueEdges);
          for(ProximalVertexArray::iterator i = edgePairs.begin(); i != edgePairs.end(); ++i)
          {
            uniqueEdgeIndices.insert(inserter.insert(ProximalVertex(&(*i))));
          }
        }

        {
          edge_clear();
          m_select_edges.reserve(uniqueEdges.size());
          for(UniqueEdges::iterator i = uniqueEdges.begin(); i != uniqueEdges.end(); ++i)
          {
            edge_push_back(faceVertices[ProximalVertexArray_index(edgePairs, *i)]);
          }
        }

        {
          m_edge_faces.resize(uniqueEdges.size());
          for(std::size_t i=0; i<uniqueEdges.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(edgePairs, uniqueEdges[i])];
            m_edge_faces[i] = EdgeFaces(faceVertex.getFace(), m_faces[faceVertex.getFace()]->getWinding()[faceVertex.getVertex()].adjacent);
          }
        }

        {
          m_uniqueEdgePoints.resize(uniqueEdges.size());
          for(std::size_t i=0; i<uniqueEdges.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(edgePairs, uniqueEdges[i])];

            const Winding& w = m_faces[faceVertex.getFace()]->getWinding();
            Vector3 edge = vector3_mid(w[faceVertex.getVertex()].vertex, w[Winding_next(w, faceVertex.getVertex())].vertex);
            m_uniqueEdgePoints[i] = pointvertex_for_windingpoint(edge, colour_vertex);
          }
        }

      }
    

      IndexBuffer uniqueVertexIndices;
      typedef VertexBuffer<ProximalVertex> UniqueVertices;
      UniqueVertices uniqueVertices;

      uniqueVertexIndices.reserve(faceVertices.size());
      uniqueVertices.reserve(faceVertices.size());

      {
        ProximalVertexArray vertexRings;
        vertexRings.resize(faceVertices.size());

        {
          for(std::size_t i=0; i<faceVertices.size(); ++i)
          {
            vertexRings[i].m_next = vertexRings.data() + absoluteIndex(next_vertex(m_faces, faceVertices[i]));
          }
        }

        {
          UniqueVertexBuffer<ProximalVertex> inserter(uniqueVertices);
          for(ProximalVertexArray::iterator i = vertexRings.begin(); i != vertexRings.end(); ++i)
          {
            uniqueVertexIndices.insert(inserter.insert(ProximalVertex(&(*i))));
          }
        }

        {
          vertex_clear();
          m_select_vertices.reserve(uniqueVertices.size());  
          for(UniqueVertices::iterator i = uniqueVertices.begin(); i != uniqueVertices.end(); ++i)
          {
            vertex_push_back(faceVertices[ProximalVertexArray_index(vertexRings, (*i))]);
          }
        }

        {
          m_uniqueVertexPoints.resize(uniqueVertices.size());
          for(std::size_t i=0; i<uniqueVertices.size(); ++i)
          {
            FaceVertexId faceVertex = faceVertices[ProximalVertexArray_index(vertexRings, uniqueVertices[i])];

            const Winding& winding = m_faces[faceVertex.getFace()]->getWinding();
            m_uniqueVertexPoints[i] = pointvertex_for_windingpoint(winding[faceVertex.getVertex()].vertex, colour_vertex);
          }
        }
      }

      if((uniqueVertices.size() + faces_size) - uniqueEdges.size() != 2)
      {
        globalErrorStream() << "Final B-Rep: inconsistent vertex count\n";
      }

#if BRUSH_CONNECTIVITY_DEBUG
      if((uniqueVertices.size() + faces_size) - uniqueEdges.size() != 2)
      {
        for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
        {
          std::size_t faceIndex = std::distance(m_faces.begin(), i);

          if(!(*i)->contributes())
          {
            globalOutputStream() << "face: " << Unsigned(faceIndex) << " does not contribute\n";
          }

          Winding_printConnectivity((*i)->getWinding());
        }
      }
#endif

      // edge-index list for wireframe rendering
      {
        m_edge_indices.resize(uniqueEdgeIndices.size());

        for(std::size_t i=0, count=0; i<m_faces.size(); ++i)
        {
          const Winding& winding = m_faces[i]->getWinding();
          for(std::size_t j = 0; j < winding.numpoints; ++j)
          {
            const RenderIndex edge_index = uniqueEdgeIndices[count+j];

            m_edge_indices[edge_index].first = uniqueVertexIndices[count + j];
            m_edge_indices[edge_index].second = uniqueVertexIndices[count + Winding_next(winding, j)];
          }
          count += winding.numpoints;
        }
      }
    }

    {
      m_faceCentroidPoints.resize(m_faces.size());
      for(std::size_t i=0; i<m_faces.size(); ++i)
      {
        m_faces[i]->construct_centroid();
        m_faceCentroidPoints[i] = pointvertex_for_windingpoint(m_faces[i]->centroid(), colour_vertex);
      }
    }
  }
}


class FaceFilterWrapper : public Filter
{
  FaceFilter& m_filter;
  bool m_active;
  bool m_invert;
public:
  FaceFilterWrapper(FaceFilter& filter, bool invert) :
    m_filter(filter),
    m_invert(invert)
  {
  }
  void setActive(bool active)
  {
    m_active = active;
  }
  bool active()
  {
    return m_active;
  }
  bool filter(const Face& face)
  {
    return m_invert ^ m_filter.filter(face);
  }
};


typedef std::list<FaceFilterWrapper> FaceFilters;
FaceFilters g_faceFilters;

void add_face_filter(FaceFilter& filter, int mask, bool invert)
{
  g_faceFilters.push_back(FaceFilterWrapper(filter, invert));
  GlobalFilterSystem().addFilter(g_faceFilters.back(), mask);
}

bool face_filtered(Face& face)
{
  for(FaceFilters::iterator i = g_faceFilters.begin(); i != g_faceFilters.end(); ++i)
  {
    if((*i).active() && (*i).filter(face))
    {
      return true;
    }
  }
  return false;
}


class BrushFilterWrapper : public Filter
{
  bool m_active;
  bool m_invert;
  BrushFilter& m_filter;
public:
  BrushFilterWrapper(BrushFilter& filter, bool invert) : m_invert(invert), m_filter(filter)
  {
  }
  void setActive(bool active)
  {
    m_active = active;
  }
  bool active()
  {
    return m_active;
  }
  bool filter(const Brush& brush)
  {
    return m_invert ^ m_filter.filter(brush);
  }
};


typedef std::list<BrushFilterWrapper> BrushFilters;
BrushFilters g_brushFilters;

void add_brush_filter(BrushFilter& filter, int mask, bool invert)
{
  g_brushFilters.push_back(BrushFilterWrapper(filter, invert));
  GlobalFilterSystem().addFilter(g_brushFilters.back(), mask);
}

bool brush_filtered(Brush& brush)
{
  for(BrushFilters::iterator i = g_brushFilters.begin(); i != g_brushFilters.end(); ++i)
  {
    if((*i).active() && (*i).filter(brush))
    {
      return true;
    }
  }
  return false;
}


