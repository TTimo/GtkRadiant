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

#include "StdAfx.h"

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include "dialogs/dialogs-gtk.h"

#include "gtkr_list.h"
#include "str.h"

#include "DPoint.h"
#include "DPlane.h"
#include "DBrush.h"
#include "DEPair.h"
#include "DPatch.h"
#include "DEntity.h"
#include "DShape.h"
#include "DBobView.h"
#include "DVisDrawer.h"
#include "DTrainDrawer.h"

#include "misc.h"
#include "scriptparser.h"
#include "DTreePlanter.h"

#include "shapes.h"
#include "lists.h"
#include "funchandlers.h"
#include "visfind.h"

#include "iundo.h"

#include "refcounted_ptr.h"

#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "scenelib.h"

// for autocaulk
list<Str> exclusionList;		// whole brush exclusion
list<Str> exclusionList_Face;	// single face exclusion

bool el1Loaded =		FALSE;
bool el2Loaded =		FALSE;
bool clrLst1Loaded =	FALSE;
bool clrLst2Loaded =	FALSE;

DBobView*		g_PathView =		NULL;
DVisDrawer*		g_VisView =			NULL;
DTrainDrawer*	g_TrainView =		NULL;
DTreePlanter*	g_TreePlanter =		NULL;
// -------------

//========================//
//    Helper Functions    //
//========================//

void LoadLists()
{
	char buffer[256];

	if(!el1Loaded)
		el1Loaded = LoadExclusionList(GetFilename(buffer, "bt/bt-el1.txt"), &exclusionList);
	if(!el2Loaded)
		el2Loaded = LoadExclusionList(GetFilename(buffer, "bt/bt-el2.txt"), &exclusionList_Face);
}


//========================//
//     Main Functions     //
//========================//

void DoIntersect()
{
	IntersectRS rs;

	if(DoIntersectBox(&rs) == eIDCANCEL)
		return;

	if(rs.nBrushOptions == BRUSH_OPT_SELECTED)
	{
		if( GlobalSelectionSystem().countSelected() < 2 )
		{
			DoMessageBox("Invalid number of brushes selected, choose at least 2", "Error", eMB_OK);
			return; 
		}
	}

	DEntity world;

	switch(rs.nBrushOptions)
	{
	case BRUSH_OPT_SELECTED:
		{
			world.LoadSelectedBrushes();
			break;
		}
	case BRUSH_OPT_WHOLE_MAP:
		{
			world.LoadFromEntity(0, FALSE);
			break;
		}
	}

	world.RemoveNonCheckBrushes(&exclusionList, rs.bUseDetail);

	bool* pbSelectList;
	if(rs.bDuplicateOnly)
		pbSelectList = world.BuildDuplicateList();
	else
		pbSelectList = world.BuildIntersectList();

	world.SelectBrushes(pbSelectList);

	delete[] pbSelectList;
}

void DoPolygonsTB()
{
  DoPolygons();
}

void DoPolygons()
{
	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of brushes selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

	PolygonRS rs;

	// ask user for type, size, etc....
	if(DoPolygonBox(&rs) == eIDOK)
	{
		DShape poly;

    vec3_t vMin, vMax;

    {
      scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
      VectorSubtract(instance.aabb_world().origin, instance.aabb_world().extents, vMin);
      VectorAdd(instance.aabb_world().origin, instance.aabb_world().extents, vMax);

      instance.path().parent()->m_traverse->erase(instance.path().top());
    }

		if(rs.bInverse)
			poly.BuildInversePrism(vMin, vMax, rs.nSides, rs.bAlignTop);
		else
		{
			if(rs.bUseBorder)
				poly.BuildBorderedPrism(vMin, vMax, rs.nSides, rs.nBorderWidth, rs.bAlignTop);
			else
				poly.BuildRegularPrism(vMin, vMax, rs.nSides, rs.bAlignTop);

		}

		poly.Commit();
	}
}

void DoFixBrushes()
{
	DMap world;
	world.LoadAll();

	int count = world.FixBrushes();
	
	Sys_Printf("%i invalid/duplicate planes removed\n", count);
}

void DoResetTextures()
{
	static ResetTextureRS rs;

  const char* texName;
	if(1/*g_SelectedFaceTable.m_pfnGetSelectedFaceCount() != 1*/)
  {
    texName = NULL;
  }
  else
  {
    texName = GetCurrentTexture();
	  strcpy(rs.textureName, GetCurrentTexture());
  }

  EMessageBoxReturn ret;
	if((ret = DoResetTextureBox(&rs)) == eIDCANCEL)
		return;  

  if(rs.bResetTextureName)
    texName = rs.textureName;

  if(ret == eIDOK)
  {
	  DEntity world;
	  world.LoadSelectedBrushes();
	  world.ResetTextures(texName,              rs.fScale,      rs.fShift,      rs.rotation, rs.newTextureName, 
                        rs.bResetTextureName, rs.bResetScale, rs.bResetShift, rs.bResetRotation, TRUE);
  }
  else
  {
	  DMap world;
    world.LoadAll(TRUE);
    world.ResetTextures(texName,              rs.fScale,      rs.fShift,      rs.rotation, rs.newTextureName, 
                        rs.bResetTextureName, rs.bResetScale, rs.bResetShift, rs.bResetRotation);
  }
}

void DoBuildStairs()
{
	BuildStairsRS rs;

	strcpy(rs.mainTexture, GetCurrentTexture());

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of brushes selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

	// ask user for type, size, etc....
	if(DoBuildStairsBox(&rs) == eIDOK)
	{
    vec3_t vMin, vMax;

    {
      scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
      VectorSubtract(instance.aabb_world().origin, instance.aabb_world().extents, vMin);
      VectorAdd(instance.aabb_world().origin, instance.aabb_world().extents, vMax);
    }

		// calc brush size
		vec3_t size;
		VectorSubtract(vMax, vMin, size);

		if(((int)size[2] % rs.stairHeight) != 0)
		{
			// stairs must fit evenly into brush
			DoMessageBox("Invalid stair height\nHeight of block must be divisable by stair height", "Error", eMB_OK);
		}
		else
		{
      {
        scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
        instance.path().parent()->m_traverse->erase(instance.path().top());
      }
						
			// Get Step Count
			int numSteps = (int)size[2] / rs.stairHeight;
			
			if(rs.style == STYLE_CORNER)
			{
				BuildCornerStairs(vMin, vMax, numSteps, rs.mainTexture, rs.riserTexture);
			}
			else
			{

				// Get Step Dimensions
				float stairHeight = (float)rs.stairHeight;
				float stairWidth;
				if((rs.direction == MOVE_EAST) || (rs.direction == MOVE_WEST))
					stairWidth = (size[0])/numSteps;
				else
					stairWidth = (size[1])/numSteps;


				// Build Base For Stair (bob's style)
				if(rs.style == STYLE_BOB)
					Build_Wedge(rs.direction, vMin, vMax, TRUE);


				// Set First Step Starting Position
				vMax[2] = vMin[2] + stairHeight;
				SetInitialStairPos(rs.direction, vMin, vMax, stairWidth);


				// Build The Steps
				for(int i = 0; i < numSteps; i++)
				{
					if(rs.style == STYLE_BOB)
						Build_StairStep_Wedge(rs.direction, vMin, vMax, rs.mainTexture, rs.riserTexture, rs.bUseDetail);
					else if(rs.style == STYLE_ORIGINAL)
						Build_StairStep(vMin, vMax, rs.mainTexture, rs.riserTexture, rs.direction);

					// get step into next position
					MoveBlock(rs.direction, vMin, vMax, stairWidth);
					vMax[2] += stairHeight;
					if(rs.style == STYLE_BOB)
						vMin[2] += stairHeight;	// wedge bottom must be raised
				}
			}
		}
	}
}

void DoBuildDoors()
{
	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of brushes selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

  DoorRS rs;
	strcpy(rs.mainTexture, GetCurrentTexture());

	if(DoDoorsBox(&rs) == eIDOK)
	{
    vec3_t vMin, vMax;

    {
      scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
      VectorSubtract(instance.aabb_world().origin, instance.aabb_world().extents, vMin);
      VectorAdd(instance.aabb_world().origin, instance.aabb_world().extents, vMax);
      instance.path().parent()->m_traverse->erase(instance.path().top());
    }

		BuildDoorsX2(vMin, vMax, 
			rs.bScaleMainH, rs.bScaleMainV,
			rs.bScaleTrimH, rs.bScaleTrimV,
			rs.mainTexture, rs.trimTexture,
			rs.nOrientation);	// shapes.cpp
	}
}

void DoPathPlotter()
{
	PathPlotterRS rs;
	EMessageBoxReturn ret = DoPathPlotterBox(&rs);
	if(ret == eIDCANCEL)
		return;
	if(ret == eIDNO)
	{
		if(g_PathView)
			delete g_PathView;
		return;
	}

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of brushes selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

  scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();

	DEntity world;
	world.LoadEPairList(instance.path().top()->m_entity);

	DEPair* trigger_ep = world.FindEPairByKey("targetname");

	if(trigger_ep)
	{
		if(!strcmp(world.m_Classname, "trigger_push"))
		{
			DEPair* target_ep = world.FindEPairByKey("target");
			if(target_ep)
			{
        scene::Path* entTarget = FindEntityFromTargetname(target_ep->value, NULL);
				if(entTarget)
				{
					if(g_PathView)
						delete g_PathView;
					g_PathView = new DBobView;

					g_PathView->Begin(trigger_ep->value, target_ep->value, rs.fMultiplier, rs.nPoints, rs.fGravity, rs.bNoUpdate, rs.bShowExtra);
				}
				else
					DoMessageBox("trigger_push target could not be found.", "Error", eMB_OK);
			}
			else
				DoMessageBox("trigger_push has no target.", "Error", eMB_OK);
		}
		else
			DoMessageBox("You must select a 'trigger_push' entity.", "Error", eMB_OK);
	}	
	else
		DoMessageBox("Entity must have a targetname", "Error", eMB_OK);
}

void DoPitBuilder()
{
	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of brushes selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

  vec3_t vMin, vMax;

  scene::Instance& instance = GlobalSelectionSystem().ultimateSelected();
  VectorSubtract(instance.aabb_world().origin, instance.aabb_world().extents, vMin);
  VectorAdd(instance.aabb_world().origin, instance.aabb_world().extents, vMax);

	DShape pit;

	if(pit.BuildPit(vMin, vMax))
	{
		pit.Commit();

    instance.path().parent()->m_traverse->erase(instance.path().top());
	}
	else
		DoMessageBox("Failed To Make Pit\nTry Making The Brush Bigger", "Error", eMB_OK);
}

void DoMergePatches()
{
  patch_merge_t merge_info;
  DPatch mrgPatches[2];
  int i;

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 2 )
	{
		DoMessageBox("Invalid number of patches selected, choose 2 only", "Error", eMB_OK);
		return; 
	}

  scene::Node* patches[2];
  patches[0] = GlobalSelectionSystem().ultimateSelected().path().top();
  patches[1] = GlobalSelectionSystem().penultimateSelected().path().top();

  for (i = 0; i < 2; i++)
  {
    if (!patches[i]->m_patch)
    {
      DoMessageBox("You must select ONLY patches", "Error", eMB_OK);
      return; 
    }

    mrgPatches[0].LoadFromBrush(patches[i]);
  }

  /*  mrgPatches[0].Transpose();
      mrgPatches[0].RemoveFromRadiant();
      mrgPatches[0].BuildInRadiant();*/

  merge_info = mrgPatches[0].IsMergable(&mrgPatches[1]);

  if (merge_info.mergable)
  {
    Sys_Printf("%i %i", merge_info.pos1, merge_info.pos2);

    Sys_Printf("Patches Mergable\n");
    DPatch* newPatch = mrgPatches[0].MergePatches(merge_info, &mrgPatches[0], &mrgPatches[1]);

    /*                mrgPatches[0].RemoveFromRadiant();
    mrgPatches[0].BuildInRadiant();
    
      mrgPatches[1].RemoveFromRadiant();
      mrgPatches[1].BuildInRadiant();
      
        
    delete newPatch;*/

    if (!newPatch)
    {
    } else
    {
      mrgPatches[0].RemoveFromRadiant();
      mrgPatches[1].RemoveFromRadiant();

      newPatch->BuildInRadiant();
      delete newPatch;
    }
  }
}

void DoSplitPatch() {
	DPatch patch;

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of patches selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

  scene::Node* node = GlobalSelectionSystem().ultimateSelected().path().top();

	if( !node->m_patch ) {
		DoMessageBox("You must select ONLY patches", "Error", eMB_OK);
		return; 
	}

	patch.LoadFromBrush(node);

	list<DPatch> patchList = patch.Split( true, true );
	for(list<DPatch>::iterator patches = patchList.begin(); patches != patchList.end(); patches++) {
		(*patches).BuildInRadiant();
	}

	patch.RemoveFromRadiant();
}

void DoVisAnalyse()
{
	char filename[1024];

	if( GlobalSelectionSystem().countSelected() == 0 )
	{
		if(g_VisView) 
		{
			delete g_VisView;
			return;
		}
	}

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 1 )
	{
		DoMessageBox("Invalid number of objects selected, choose 1 only", "Error", eMB_OK);
		return; 
	}

  scene::Node* brush = GlobalSelectionSystem().ultimateSelected().path().top();

	DBrush orgBrush;
	orgBrush.LoadFromBrush(brush, false);

	orgBrush.BuildBounds();
	vec3_t origin;
	origin[0] = (orgBrush.bbox_max[0] + orgBrush.bbox_min[0])/2.f;
	origin[1] = (orgBrush.bbox_max[1] + orgBrush.bbox_min[1])/2.f;
	origin[2] = (orgBrush.bbox_max[2] + orgBrush.bbox_min[2])/2.f;


  const char* rad_filename = g_FuncTable.m_pfnGetMapName();
	if(!rad_filename)
	{
		DoMessageBox("An Error Occurred While Trying\n To Get The Map Filename", "Error", eMB_OK);
		return;
	}

	strcpy(filename, rad_filename);
		
	char* ext = strrchr(filename, '.')+1;
	strcpy(ext, "bsp");// rename the extension

	list<DWinding*> *pointList = BuildTrace(filename, origin);

	if(!g_VisView)
	{
		g_VisView = new DVisDrawer;
		g_VisView->Register();
	}
	
	g_VisView->SetList(pointList);
}

void DoTrainPathPlot() {
	if(g_TrainView) {
		delete g_TrainView;
		g_TrainView = NULL;
	}

	g_TrainView = new DTrainDrawer();
}

void DoCaulkSelection() {
	DEntity world;
	
	float fScale[2] = { 0.5f, 0.5f };
	float fShift[2] = { 0.0f, 0.0f };

	int bResetScale[2] = { false, false };
	int bResetShift[2] = { false, false };

	world.LoadSelectedBrushes();
	world.LoadSelectedPatches();
	world.ResetTextures( NULL, fScale, fShift, 0, "textures/common/caulk", true, bResetScale, bResetShift, false, true );
}

void DoTreePlanter() {
	if(g_TreePlanter) {
		delete g_TreePlanter;
		g_TreePlanter = NULL;
		return;
	}

	g_TreePlanter = new DTreePlanter();
}

void DoDropEnts() {
	if(g_TreePlanter) {
		g_TreePlanter->DropEntsToGround();
	}
}

void DoMakeChain() {
	DTreePlanter pl;
	pl.MakeChain();
}

typedef DPoint* pntTripple[3];

bool bFacesNoTop[6] = {true, true, true, true, true, false};

void DoFlipTerrain() {
	vec3_t vUp = { 0.f, 0.f, 1.f };
  int i;

	// ensure we have something selected
	if( GlobalSelectionSystem().countSelected() != 2 )
	{
		DoMessageBox("Invalid number of objects selected, choose 2 only", "Error", eMB_OK);
		return; 
	}

  scene::Node* brushes[2];
	brushes[0] = GlobalSelectionSystem().ultimateSelected().path().top();
	brushes[1] = GlobalSelectionSystem().penultimateSelected().path().top();

	DBrush Brushes[2];
	DPlane* Planes[2];
	pntTripple Points[2];
	for( i = 0; i < 2; i++ ) {
		Brushes[i].LoadFromBrush( brushes[i], false );
		if(!(Planes[i] = Brushes[i].FindPlaneWithClosestNormal( vUp )) || Brushes[i].FindPointsForPlane( Planes[i], Points[i], 3 ) != 3) {
			DoMessageBox("Error", "Error", eMB_OK);
			return;
		}
	}

	vec3_t mins1, mins2, maxs1, maxs2;
	Brushes[0].GetBounds( mins1, maxs1 );
	Brushes[1].GetBounds( mins2, maxs2 );

  scene::Node* ents[2];
	ents[0] = GlobalSelectionSystem().ultimateSelected().path().parent();
	ents[1] = GlobalSelectionSystem().penultimateSelected().path().parent();

	for( i = 0; i < 2; i++ ) {
		Brushes[i].RemoveFromRadiant();
	}



	int dontmatch[2] = { -1, -1 };
	bool found = false;
	for( i = 0; i < 3; i++ ) {
		for( int j = 0; j < 3 && !found; j++ ) {
			if(VectorCompare( (Points[0])[i]->_pnt, (Points[1])[j]->_pnt )) {
				found = true;
				break;
			}
		}
		if(!found) {
			dontmatch[0] = i;
			break;
		}
		found = false;
	}
	if(dontmatch[0] == -1) {
		DoMessageBox("Error", "Error", eMB_OK);
		return;
	}

	for( i = 0; i < 3; i++ ) {
		for( int j = 0; j < 3 && !found; j++ ) {
			if(VectorCompare( (Points[1])[i]->_pnt, (Points[0])[j]->_pnt )) {
				found = true;
				break;
			}
		}
		if(!found) {
			dontmatch[1] = i;
			break;
		}
		found = false;
	}
	if(dontmatch[1] == -1) {
		DoMessageBox("Error", "Error", eMB_OK);
		return;
	}

	vec3_t plnpnts1[3];
	vec3_t plnpnts2[3];
	vec3_t plnpntsshr[3];

	VectorCopy( (Points[0])[dontmatch[0]]->_pnt, plnpnts1[0] );
	for( i = 0; i < 3; i++ ) {
		if( dontmatch[0] != i ) {
			VectorCopy( (Points[0])[i]->_pnt, plnpnts1[1] );
			break;
		}
	}
	VectorCopy( (Points[1])[dontmatch[1]]->_pnt, plnpnts1[2] );

	VectorCopy( (Points[1])[dontmatch[1]]->_pnt, plnpnts2[0] );
	for( i = 0; i < 3; i++ ) {
		if( dontmatch[1] != i && !VectorCompare( (Points[1])[i]->_pnt, plnpnts1[1] )) {
			VectorCopy( (Points[1])[i]->_pnt, plnpnts2[1] );
			break;
		}
	}
	VectorCopy( (Points[0])[dontmatch[0]]->_pnt, plnpnts2[2] );

	VectorCopy( (Points[0])[dontmatch[0]]->_pnt, plnpntsshr[0] );
	VectorCopy( (Points[1])[dontmatch[1]]->_pnt, plnpntsshr[1] );
	if( (Points[1])[dontmatch[1]]->_pnt[2] < (Points[0])[dontmatch[0]]->_pnt[2] ) {
		VectorCopy( (Points[1])[dontmatch[1]]->_pnt, plnpntsshr[2] );
	} else {
		VectorCopy( (Points[0])[dontmatch[0]]->_pnt, plnpntsshr[2] );
	}
	plnpntsshr[2][2] -= 16;

	for( i = 0; i < 3; i++ ) {
		if( mins2[i] < mins1[i] ) {
			mins1[i] = mins2[i];
		}
		if( maxs2[i] > maxs1[i] ) {
			maxs1[i] = maxs2[i];
		}
	}

	DBrush* newBrushes[2];
	newBrushes[0] = DShape::GetBoundingCube_Ext( mins1, maxs1, "textures/common/caulk", bFacesAll, true);
	newBrushes[1] = DShape::GetBoundingCube_Ext( mins1, maxs1, "textures/common/caulk", bFacesAll, true);

	vec3_t normal;
	MakeNormal( plnpnts1[0], plnpnts1[1], plnpnts1[2], normal );
	if( normal[2] >= 0 ) {
		newBrushes[0]->AddFace( plnpnts1[0], plnpnts1[1], plnpnts1[2], "textures/common/terrain", true );
	} else {
		newBrushes[0]->AddFace( plnpnts1[2], plnpnts1[1], plnpnts1[0], "textures/common/terrain", true );
	}

	MakeNormal( plnpnts2[0], plnpnts2[1], plnpnts2[2], normal );
	if( normal[2] >= 0 ) {
		newBrushes[1]->AddFace( plnpnts2[0], plnpnts2[1], plnpnts2[2], "textures/common/terrain", true );
	} else {
		newBrushes[1]->AddFace( plnpnts2[2], plnpnts2[1], plnpnts2[0], "textures/common/terrain", true );
	}

	vec3_t vec;
	MakeNormal( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], normal );	
	
	VectorSubtract( plnpnts1[2], plnpnts1[1], vec );
	if( DotProduct( vec, normal ) >= 0 ) {
		newBrushes[0]->AddFace( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], "textures/common/caulk", true );
	} else {
		newBrushes[0]->AddFace( plnpntsshr[2], plnpntsshr[1], plnpntsshr[0], "textures/common/caulk", true );
	}

	VectorSubtract( plnpnts2[2], plnpnts2[1], vec );
	if( DotProduct( vec, normal ) >= 0 ) {
		newBrushes[1]->AddFace( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], "textures/common/caulk", true );
	} else {
		newBrushes[1]->AddFace( plnpntsshr[2], plnpntsshr[1], plnpntsshr[0], "textures/common/caulk", true );
	}

	for( i = 0; i < 2; i++ ) {
		newBrushes[i]->RemoveRedundantPlanes();
		newBrushes[i]->BuildInRadiant( false, NULL, ents[i] );
		delete newBrushes[i];
	}

}
