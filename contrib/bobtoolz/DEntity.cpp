/*
BobToolz plugin for GtkRadiant
Copyright (C) 2001 Gordon Biggans

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// DEntity.cpp: implementation of the DEntity class.
//
//////////////////////////////////////////////////////////////////////

#include "DEntity.h"

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <list>
#include "str.h"

#include "DPoint.h"
#include "DPlane.h"
#include "DBrush.h"
#include "DEPair.h"
#include "DPatch.h"

#include "dialogs/dialogs-gtk.h"
#include "misc.h"
#include "CPortals.h"

#include "iundo.h"
#include "ientity.h"
#include "ieclass.h"

#include "generic/referencecounted.h"

#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "scenelib.h"


const char* brushEntityList[] = {
	"worldspawn",
	"trigger_always",
	"trigger_hurt",
	"trigger_multiple",
	"trigger_push",
	"trigger_teleport",
	"func_bobbing",
	"func_button",
	"func_door",
	"func_group",
	"func_pendulum",
	"func_plat",
	"func_rotating",
	"func_static",
	"func_timer",
	"func_train",
	0
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEntity::DEntity(const char *classname, int ID)
{
	SetClassname(classname);
	m_nID = ID;
	QER_Entity = NULL;
}

DEntity::~DEntity()
{
	ClearPatches();
	ClearBrushes();
	ClearEPairs();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void DEntity::ClearBrushes()
{
	for(std::list<DBrush *>::const_iterator deadBrush=brushList.begin(); deadBrush!=brushList.end(); deadBrush++)
	{
		delete *deadBrush;
	}
	brushList.clear();
}

void DEntity::ClearPatches()
{
	for(std::list<DPatch *>::const_iterator deadPatch=patchList.begin(); deadPatch!=patchList.end(); deadPatch++)
	{
		delete *deadPatch;
	}
	patchList.clear();
}

DPatch* DEntity::NewPatch()
{
	DPatch* newPatch = new DPatch;

	patchList.push_back(newPatch);

	return newPatch;
}

DBrush* DEntity::NewBrush(int ID)
{
	DBrush* newBrush = new DBrush(ID);

	brushList.push_back(newBrush);

	return newBrush;
}

char* getNextBracket(char* s)
{
	char* p = s;
	while(*p)
	{
		p++;
		if(*p == '(')
			break;
	}

	return p;
}

bool DEntity::LoadFromPrt(char *filename)
{
	CPortals portals;
	strcpy(portals.fn, filename);
	portals.Load();

	if(portals.node_count == 0)
		return false;

	ClearBrushes();
	ClearEPairs();
	
  bool build = false;
	for(unsigned int i = 0; i < portals.node_count; i++)
	{
    build = false;
		DBrush* brush = NewBrush();

		for(unsigned int j = 0; j < portals.node[i].portal_count; j++)
		{
      for(unsigned int k = 0; k < portals.node[i].portal[j].point_count-2; k++) 
      {
	      vec3_t v1, v2, normal, n;
	      VectorSubtract(portals.node[i].portal[j].point[k+2].p, portals.node[i].portal[j].point[k+1].p, v1);
	      VectorSubtract(portals.node[i].portal[j].point[k].p, portals.node[i].portal[j].point[k+1].p, v2);
	      CrossProduct(v1, v2, n);
        VectorNormalize(n, v2);

        if(k == 0) 
        {
          VectorCopy(v2, normal);
        }
        else
        {
          VectorSubtract(v2, normal, v1);
          if(VectorLength(v1) > 0.01)
          {
            build = true;
            break;
          }
        }
      }

      if(!build)
			  brush->AddFace(portals.node[i].portal[j].point[2].p, portals.node[i].portal[j].point[1].p, portals.node[i].portal[j].point[0].p, "textures/common/caulk", false);
      else
			  brush->AddFace(portals.node[i].portal[j].point[0].p, portals.node[i].portal[j].point[1].p, portals.node[i].portal[j].point[2].p, "textures/common/caulk", false);
		}
    if(build)
      brush->BuildInRadiant(false, NULL);
	}

	return true;
}

DPlane* DEntity::AddFaceToBrush(vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* faceData, int ID)
{
	DBrush* buildBrush = GetBrushForID(ID);
	return buildBrush->AddFace(va, vb, vc, faceData);
	// slow, dont use much
}

DBrush* DEntity::GetBrushForID(int ID)
{
	DBrush* buildBrush = NULL;

	for(std::list<DBrush *>::const_iterator chkBrush=brushList.begin(); chkBrush!=brushList.end(); chkBrush++)
	{
		if((*chkBrush)->m_nBrushID == ID)
		{
			buildBrush = (*chkBrush);
			break;
		}
	}

	if(!buildBrush)
		buildBrush = NewBrush(ID);

	return buildBrush;
}

template<typename Functor>
class BrushSelectedVisitor : public SelectionSystem::Visitor
{
  const Functor& m_functor;
public:
  BrushSelectedVisitor(const Functor& functor) : m_functor(functor)
  {
  }
  void visit(scene::Instance& instance) const
  {
    if(Node_isBrush(instance.path().top()))
    {
      m_functor(instance);
    }
  }
};

template<typename Functor>
inline const Functor& Scene_forEachSelectedBrush(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(BrushSelectedVisitor<Functor>(functor));
  return functor;
}

void DEntity_loadBrush(DEntity& entity, scene::Instance& brush)
{
  DBrush* loadBrush = entity.NewBrush(static_cast<int>(entity.brushList.size()));
	loadBrush->LoadFromBrush(brush, true);
}
typedef ReferenceCaller1<DEntity, scene::Instance&, DEntity_loadBrush> DEntityLoadBrushCaller;

void DEntity::LoadSelectedBrushes()
{
	ClearBrushes();
	ClearEPairs();

  Scene_forEachSelectedBrush(DEntityLoadBrushCaller(*this));
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
    if(Node_isPatch(instance.path().top()))
    {
      m_functor(instance);
    }
  }
};

template<typename Functor>
inline const Functor& Scene_forEachSelectedPatch(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(PatchSelectedVisitor<Functor>(functor));
  return functor;
}

void DEntity_loadPatch(DEntity& entity, scene::Instance& patch)
{
  DPatch* loadPatch = entity.NewPatch();
	loadPatch->LoadFromPatch(patch);
}
typedef ReferenceCaller1<DEntity, scene::Instance&, DEntity_loadPatch> DEntityLoadPatchCaller;

void DEntity::LoadSelectedPatches()
{
	ClearPatches();
	ClearEPairs();

  Scene_forEachSelectedPatch(DEntityLoadPatchCaller(*this));
}

bool* DEntity::BuildIntersectList()
{
	int max = GetIDMax();
	if(max == 0)
		return NULL;

	bool* pbIntList = new bool[max];
	memset(pbIntList, 0, sizeof(bool)*(max));

	for(std::list<DBrush *>::const_iterator pB1=brushList.begin(); pB1!=brushList.end(); pB1++)
	{
		std::list<DBrush *>::const_iterator pB2=pB1;
		for(pB2++; pB2!=brushList.end(); pB2++)
		{
			if((*pB1)->IntersectsWith((*pB2)))
			{
				pbIntList[(*pB1)->m_nBrushID] = true;
				pbIntList[(*pB2)->m_nBrushID] = true;
			}
		}
	}

	return pbIntList;
}

bool* DEntity::BuildDuplicateList()
{
	int max = GetIDMax();
	if(max == 0)
		return NULL;

	bool* pbDupList = new bool[max];
	memset(pbDupList, 0, sizeof(bool)*(max));

	for(std::list<DBrush *>::const_iterator pB1=brushList.begin(); pB1!=brushList.end(); pB1++)
	{
		std::list<DBrush *>::const_iterator pB2=pB1;
		for(pB2++; pB2!=brushList.end(); pB2++)
		{
			if(**pB1 == *pB2)
			{
				pbDupList[(*pB1)->m_nBrushID] = true;
				pbDupList[(*pB2)->m_nBrushID] = true;
			}
		}
	}

	return pbDupList;
}

void DEntity::SelectBrushes(bool *selectList)
{
	if(selectList == NULL)
		return;

  GlobalSelectionSystem().setSelectedAll(false);

  scene::Path path(NodeReference(GlobalSceneGraph().root()));
  path.push(NodeReference(*QER_Entity));

	for(std::list<DBrush *>::const_iterator pBrush=brushList.begin(); pBrush!=brushList.end(); pBrush++)
	{
		if(selectList[(*pBrush)->m_nBrushID])
    {
      path.push(NodeReference(*(*pBrush)->QER_brush));
      Instance_getSelectable(*GlobalSceneGraph().find(path))->setSelected(true);
      path.pop();
    }
	}
}

bool DEntity::LoadFromEntity(scene::Node& ent, bool bLoadPatches) {
	ClearPatches();
	ClearBrushes();
	ClearEPairs();

	QER_Entity = &ent;

	LoadEPairList(Node_getEntity(ent));

	bool keep = false;
	int i;
	for(i = 0; brushEntityList[i]; i++)
	{
		if(string_equal_nocase(brushEntityList[i], m_Classname))
		{
			keep = true;
			break;
		}
	}

	if(!keep)
		return false;

  if(Node_getTraversable(ent))
  {
    class load_brushes_t : public scene::Traversable::Walker
    {
      DEntity* m_entity;
      mutable int m_count;
    public:
      load_brushes_t(DEntity* entity)
        : m_entity(entity), m_count(0)
      {
      }
      bool pre(scene::Node& node) const
      {
        scene::Path path(NodeReference(GlobalSceneGraph().root()));
        path.push(NodeReference(*m_entity->QER_Entity));
        path.push(NodeReference(node));
        scene::Instance* instance = GlobalSceneGraph().find(path);
        ASSERT_MESSAGE(instance != 0, "");

        if(Node_isPatch(node))
        {
          DPatch* loadPatch = m_entity->NewPatch();
				  loadPatch->LoadFromPatch(*instance);
        }
        else if(Node_isBrush(node))
        {
			    DBrush* loadBrush = m_entity->NewBrush(m_count++);
			    loadBrush->LoadFromBrush(*instance, true);
        }
        return false;
      }
    } load_brushes(this);

    Node_getTraversable(ent)->traverse(load_brushes);
  }

	return true;
}

void DEntity::RemoveNonCheckBrushes(std::list<Str>* exclusionList, bool useDetail)
{
	std::list<DBrush *>::iterator chkBrush=brushList.begin();

	while( chkBrush!=brushList.end() )
	{
		if(!useDetail)
		{
			if((*chkBrush)->IsDetail())
			{
				delete *chkBrush;
				chkBrush = brushList.erase(chkBrush);
				continue;
			}
		}

		std::list<Str>::iterator eTexture;

		for( eTexture=exclusionList->begin(); eTexture!=exclusionList->end(); eTexture++ )
		{
			if((*chkBrush)->HasTexture((*eTexture).GetBuffer()))
			{
				delete *chkBrush;
				chkBrush = brushList.erase(chkBrush);
				break;
			}
		}

		if( eTexture == exclusionList->end() )
			chkBrush++;
	}
}

void DEntity::ResetChecks(std::list<Str>* exclusionList)
{
	for(std::list<DBrush *>::const_iterator resetBrush=brushList.begin(); resetBrush!=brushList.end(); resetBrush++)
	{
		(*resetBrush)->ResetChecks(exclusionList);
	}
}

int DEntity::FixBrushes()
{
	int count = 0;

	for(std::list<DBrush *>::const_iterator fixBrush=brushList.begin(); fixBrush!=brushList.end(); fixBrush++)
	{
    count += (*fixBrush)->RemoveRedundantPlanes();
	}

	return count;
}

void DEntity::BuildInRadiant(bool allowDestruction)
{
	bool makeEntity = strcmp(m_Classname, "worldspawn") ? true : false;

	if(makeEntity)
	{
    NodeSmartReference node(GlobalEntityCreator().createEntity(GlobalEntityClassManager().findOrInsert(m_Classname.GetBuffer(), !brushList.empty() || !patchList.empty())));

		for(std::list<DEPair* >::const_iterator buildEPair=epairList.begin(); buildEPair!=epairList.end(); buildEPair++)
		{
      Node_getEntity(node)->setKeyValue((*buildEPair)->key, (*buildEPair)->value);
		}

		Node_getTraversable(GlobalSceneGraph().root())->insert(node);

		for(std::list<DBrush *>::const_iterator buildBrush=brushList.begin(); buildBrush!=brushList.end(); buildBrush++)
			(*buildBrush)->BuildInRadiant(allowDestruction, NULL, node.get_pointer());

		for(std::list<DPatch *>::const_iterator buildPatch=patchList.begin(); buildPatch!=patchList.end(); buildPatch++)
			(*buildPatch)->BuildInRadiant(node.get_pointer());

		QER_Entity = node.get_pointer();
	}
	else
	{
		for(std::list<DBrush *>::const_iterator buildBrush=brushList.begin(); buildBrush!=brushList.end(); buildBrush++)
			(*buildBrush)->BuildInRadiant(allowDestruction, NULL);

		for(std::list<DPatch *>::const_iterator buildPatch=patchList.begin(); buildPatch!=patchList.end(); buildPatch++)
			(*buildPatch)->BuildInRadiant();
	}
}



int DEntity::GetIDMax( void ) {
	int max = -1;
	for(std::list<DBrush *>::const_iterator cntBrush=brushList.begin(); cntBrush!=brushList.end(); cntBrush++) {
		if((*cntBrush)->m_nBrushID > max)
			max = (*cntBrush)->m_nBrushID;
	}
	return max+1;
}

void DEntity::SetClassname( const char *classname ) {
	m_Classname = classname;
}

void DEntity::SaveToFile(FILE *pFile)
{
	fprintf(pFile, "{\n");

	fprintf(pFile, "\"classname\" \"%s\"\n", (const char *)m_Classname);

	for(std::list<DEPair *>::const_iterator ep=epairList.begin(); ep!=epairList.end(); ep++)
	{
		fprintf(pFile, "\"%s\" \"%s\"\n", (const char *)(*ep)->key, (const char *)(*ep)->value);
	}

	for(std::list<DBrush *>::const_iterator bp=brushList.begin(); bp!=brushList.end(); bp++)
	{
		(*bp)->SaveToFile(pFile);
	}

	fprintf(pFile, "}\n");
}

void DEntity::ClearEPairs()
{
	for(std::list<DEPair *>::const_iterator deadEPair=epairList.begin(); deadEPair!=epairList.end(); deadEPair++)
	{
		delete (*deadEPair);
	}
	epairList.clear();
}

void DEntity::AddEPair(const char *key, const char *value) {	
	DEPair* newEPair; 
	newEPair = FindEPairByKey( key );
	if(!newEPair) {
		newEPair = new DEPair;
		newEPair->Build(key, value);
		epairList.push_back(newEPair);
	} else {
		newEPair->Build(key, value);
	}
}

void DEntity::LoadEPairList(Entity *epl)
{
  class load_epairs_t : public Entity::Visitor
  {
    DEntity* m_entity;
  public:
    load_epairs_t(DEntity* entity)
      : m_entity(entity)
    {
    }
    void visit(const char* key, const char* value)
    {
      if(strcmp(key, "classname") == 0)
        m_entity->SetClassname(value);
      else	
			  m_entity->AddEPair(key, value);
    }

  } load_epairs(this);

  epl->forEachKeyValue(load_epairs);
}

bool DEntity::ResetTextures(const char* textureName, float fScale[2],     float fShift[2],    int rotation, const char* newTextureName, 
                            int bResetTextureName,    int bResetScale[2], int bResetShift[2], int bResetRotation, bool rebuild)
{
	bool reset = false;

	for(std::list<DBrush *>::const_iterator resetBrush=brushList.begin(); resetBrush!=brushList.end(); resetBrush++)
	{
		bool tmp = (*resetBrush)->ResetTextures(textureName,        fScale,       fShift,       rotation, newTextureName, 
                                            bResetTextureName,  bResetScale,  bResetShift,  bResetRotation);

		if(tmp)
		{
			reset = true;
			if(rebuild)
			{
        Node_getTraversable(*(*resetBrush)->QER_entity)->erase(*(*resetBrush)->QER_brush);      
        (*resetBrush)->BuildInRadiant(false, NULL, (*resetBrush)->QER_entity);
			}
		}
	}

  if(bResetTextureName)
  {
	  for(std::list<DPatch *>::const_iterator resetPatch=patchList.begin(); resetPatch!=patchList.end(); resetPatch++)
	  {
		  bool tmp = (*resetPatch)->ResetTextures(textureName, newTextureName);

		  if(tmp)
		  {
			  reset = true;
			  if(rebuild)
			  {
          Node_getTraversable(*(*resetPatch)->QER_entity)->erase(*(*resetPatch)->QER_brush);      
          (*resetPatch)->BuildInRadiant((*resetPatch)->QER_entity);
			  }
		  }
	  }
  }

	return reset;
}

DEPair* DEntity::FindEPairByKey(const char* keyname)
{
	for(std::list<DEPair *>::const_iterator ep=epairList.begin(); ep!=epairList.end(); ep++)
	{
		char* c = (*ep)->key;
		if(!strcmp(c, keyname))
			return *ep;
	}
	return NULL;
}

void DEntity::RemoveFromRadiant()
{
	Node_getTraversable(GlobalSceneGraph().root())->erase(*QER_Entity);

	QER_Entity = NULL;
}

void DEntity::SpawnString(const char* key, const char* defaultstring, const char** out)
{
	DEPair* pEP = FindEPairByKey(key);
	if(pEP) {
		*out = pEP->value;
	} else {
		*out = defaultstring;
	}
}

void DEntity::SpawnInt(const char* key, const char* defaultstring, int* out)
{
	DEPair* pEP = FindEPairByKey(key);
	if(pEP) {
		*out = atoi(pEP->value);
	} else {
		*out = atoi(defaultstring);
	}
}

void DEntity::SpawnFloat(const char* key, const char* defaultstring, float* out)
{
	DEPair* pEP = FindEPairByKey(key);
	if(pEP) {
		*out = static_cast<float>(atof(pEP->value));
	} else {
		*out = static_cast<float>(atof(defaultstring));
	}
}

void DEntity::SpawnVector(const char* key, const char* defaultstring, vec_t* out)
{
	DEPair* pEP = FindEPairByKey(key);
	if(pEP) {
		sscanf(pEP->value, "%f %f %f", &out[0], &out[1], &out[2]);
	} else {
		sscanf(defaultstring, "%f %f %f", &out[0], &out[1], &out[2]);
	}
}

int DEntity::GetBrushCount( void ) {
	return static_cast<int>(brushList.size());
}

DBrush* DEntity::FindBrushByPointer( scene::Node& brush ) {
	for(std::list<DBrush *>::const_iterator listBrush = brushList.begin(); listBrush != brushList.end(); listBrush++) {
		DBrush* pBrush = (*listBrush);
		if(pBrush->QER_brush == &brush) {
			return pBrush;
		}
	}
	return NULL;
}
