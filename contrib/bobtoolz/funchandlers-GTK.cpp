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

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "dialogs/dialogs-gtk.h"

#include "DEntity.h"
#include "DShape.h"
#include "DPatch.h"

#include "misc.h"
#include "shapes.h"
#include "lists.h"
#include "funchandlers.h"
#include "visfind.h"

// for autocaulk
list<Str> exclusionList;        // whole brush exclusion
list<Str> exclusionList_Face;   // single face exclusion

bool el1Loaded =        FALSE;
bool el2Loaded =        FALSE;
bool clrLst1Loaded =    FALSE;
bool clrLst2Loaded =    FALSE;

DBobView*       g_PathView =        NULL;
DVisDrawer*     g_VisView =         NULL;
DTrainDrawer*   g_TrainView =       NULL;
DTreePlanter*   g_TreePlanter =     NULL;
// -------------

//========================//
//    Helper Functions    //
//========================//

void LoadLists(){
	char buffer[256];

	if ( !el1Loaded ) {
		el1Loaded = LoadExclusionList( GetFilename( buffer, "bt/bt-el1.txt" ), &exclusionList );
	}
	if ( !el2Loaded ) {
		el2Loaded = LoadExclusionList( GetFilename( buffer, "bt/bt-el2.txt" ), &exclusionList_Face );
	}
}


//========================//
//     Main Functions     //
//========================//

void DoIntersect(){
	IntersectRS rs;

	if ( DoIntersectBox( &rs ) == IDCANCEL ) {
		return;
	}

	if ( rs.nBrushOptions == BRUSH_OPT_SELECTED ) {
		if ( g_FuncTable.m_pfnSelectedBrushCount() < 2 ) {
			DoMessageBox( "Invalid number of brushes selected, choose at least 2", "Error", MB_OK );
			return;
		}
	}

	DEntity world;

	switch ( rs.nBrushOptions )
	{
	case BRUSH_OPT_SELECTED:
	{
		world.LoadSelectedBrushes();
		break;
	}
	case BRUSH_OPT_WHOLE_MAP:
	{
		world.LoadFromEntity( 0, FALSE );
		break;
	}
	}

	world.RemoveNonCheckBrushes( &exclusionList, rs.bUseDetail );

	bool* pbSelectList;
	if ( rs.bDuplicateOnly ) {
		pbSelectList = world.BuildDuplicateList();
	}
	else{
		pbSelectList = world.BuildIntersectList();
	}

	world.SelectBrushes( pbSelectList );

	delete[] pbSelectList;
}

void DoPolygonsTB(){
	vec3_t vMin, vMax;

	// figure out vMin and vMax
	g_FuncTable.m_pfnGetDispatchParams( vMin, vMax, NULL );

	DoPolygons( vMin, vMax );
}

void DoPolygons( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of brushes selected, choose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...

	PolygonRS rs;

	// ask user for type, size, etc....
	if ( DoPolygonBox( &rs ) == IDOK ) {
		g_FuncTable.m_pfnDeleteBrushHandle( brush );

		DShape poly;

		if ( rs.bInverse ) {
			poly.BuildInversePrism( vMin, vMax, rs.nSides, rs.bAlignTop );
		}
		else
		{
			if ( rs.bUseBorder ) {
				poly.BuildBorderedPrism( vMin, vMax, rs.nSides, rs.nBorderWidth, rs.bAlignTop );
			}
			else{
				poly.BuildRegularPrism( vMin, vMax, rs.nSides, rs.bAlignTop );
			}

		}

		poly.Commit();
	}


	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoFixBrushes(){
	DMap world;
	world.LoadAll( true );

	int count = world.FixBrushes( true );

	Sys_Printf( "%i invalid/duplicate planes removed\n", count );
}

void DoResetTextures(){
	static ResetTextureRS rs;

	const char* texName;
	if ( g_SelectedFaceTable.m_pfnGetSelectedFaceCount() != 1 ) {
		texName = NULL;
	}
	else
	{
		texName = GetCurrentTexture();
		strcpy( rs.textureName, GetCurrentTexture() );
	}

	int ret;
	if ( ( ret = DoResetTextureBox( &rs ) ) == IDCANCEL ) {
		return;
	}

	if ( rs.bResetTextureName ) {
		texName = rs.textureName;
	}

	if ( ret == IDOK ) {
		DEntity world;
		world.LoadSelectedBrushes();
		world.ResetTextures( texName,              rs.fScale,      rs.fShift,      rs.rotation, rs.newTextureName,
							 rs.bResetTextureName, rs.bResetScale, rs.bResetShift, rs.bResetRotation, TRUE );
	}
	else
	{
		DMap world;
		world.LoadAll( TRUE );
		world.ResetTextures( texName,              rs.fScale,      rs.fShift,      rs.rotation, rs.newTextureName,
							 rs.bResetTextureName, rs.bResetScale, rs.bResetShift, rs.bResetRotation );
	}
}

void DoBuildStairs( vec3_t vMin, vec3_t vMax ){
	BuildStairsRS rs;

	strcpy( rs.mainTexture, GetCurrentTexture() );

	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...


	// ask user for type, size, etc....
	if ( DoBuildStairsBox( &rs ) == IDOK ) {
		// calc brush size
		vec3_t size;
		VectorSubtract( vMax, vMin, size );

		if ( ( (int)size[2] % rs.stairHeight ) != 0 ) {
			// stairs must fit evenly into brush
			DoMessageBox( "Invalid stair height\nHeight of block must be divisable by stair height", "Error", MB_OK );
		}
		else
		{

			// Remove Size Brush
			g_FuncTable.m_pfnDeleteBrushHandle( brush );


			// Get Step Count
			int numSteps = (int)size[2] / rs.stairHeight;

			if ( rs.style == STYLE_CORNER ) {
				BuildCornerStairs( vMin, vMax, numSteps, rs.mainTexture, rs.riserTexture );
			}
			else
			{

				// Get Step Dimensions
				float stairHeight = (float)rs.stairHeight;
				float stairWidth;
				if ( ( rs.direction == MOVE_EAST ) || ( rs.direction == MOVE_WEST ) ) {
					stairWidth = ( size[0] ) / numSteps;
				}
				else{
					stairWidth = ( size[1] ) / numSteps;
				}


				// Build Base For Stair (bob's style)
				if ( rs.style == STYLE_BOB ) {
					Build_Wedge( rs.direction, vMin, vMax, TRUE );
				}


				// Set First Step Starting Position
				vMax[2] = vMin[2] + stairHeight;
				SetInitialStairPos( rs.direction, vMin, vMax, stairWidth );


				// Build The Steps
				for ( int i = 0; i < numSteps; i++ )
				{
					if ( rs.style == STYLE_BOB ) {
						Build_StairStep_Wedge( rs.direction, vMin, vMax, rs.mainTexture, rs.riserTexture, rs.bUseDetail );
					}
					else if ( rs.style == STYLE_ORIGINAL ) {
						Build_StairStep( vMin, vMax, rs.mainTexture, rs.riserTexture, rs.direction );
					}

					// get step into next position
					MoveBlock( rs.direction, vMin, vMax, stairWidth );
					vMax[2] += stairHeight;
					if ( rs.style == STYLE_BOB ) {
						vMin[2] += stairHeight; // wedge bottom must be raised
					}
				}
			}
		}
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoBuildDoors( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...

	DoorRS rs;
	strcpy( rs.mainTexture, GetCurrentTexture() );

	if ( DoDoorsBox( &rs ) == IDOK ) {
		g_FuncTable.m_pfnDeleteBrushHandle( brush );

		BuildDoorsX2( vMin, vMax,
					  rs.bScaleMainH, rs.bScaleMainV,
					  rs.bScaleTrimH, rs.bScaleTrimV,
					  rs.mainTexture, rs.trimTexture,
					  rs.nOrientation ); // shapes.cpp
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoPathPlotter(){
	PathPlotterRS rs;
	int ret = DoPathPlotterBox( &rs );
	if ( ret == IDCANCEL ) {
		return;
	}
	if ( ret == IDNO ) {
		if ( g_PathView ) {
			delete g_PathView;
		}
		return;
	}

	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// should be our trigger brush

	DEntity world;
	world.LoadEPairList( *g_EntityTable.m_pfnGetEntityKeyValList( brush->owner ) );

	DEPair* trigger_ep = world.FindEPairByKey( "targetname" );

	if ( trigger_ep ) {
		if ( !strcmp( world.m_Classname, "trigger_push" ) ) {
			DEPair* target_ep = world.FindEPairByKey( "target" );
			if ( target_ep ) {
				entity_s* entTarget = FindEntityFromTargetname( target_ep->value, NULL );
				if ( entTarget ) {
					if ( g_PathView ) {
						delete g_PathView;
					}
					g_PathView = new DBobView;

					g_PathView->Begin( trigger_ep->value, target_ep->value, rs.fMultiplier, rs.nPoints, rs.fGravity, rs.bNoUpdate, rs.bShowExtra );
				}
				else{
					DoMessageBox( "trigger_push target could not be found.", "Error", MB_OK );
				}
			}
			else{
				DoMessageBox( "trigger_push has no target.", "Error", MB_OK );
			}
		}
		else{
			DoMessageBox( "You must select a 'trigger_push' entity.", "Error", MB_OK );
		}
	}
	else{
		DoMessageBox( "Entity must have a targetname", "Error", MB_OK );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoPitBuilder( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...

	DShape pit;

	if ( pit.BuildPit( vMin, vMax ) ) {
		pit.Commit();

		g_FuncTable.m_pfnDeleteBrushHandle( brush );
	}
	else{
		DoMessageBox( "Failed To Make Pit\nTry Making The Brush Bigger", "Error", MB_OK );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoMergePatches(){
	patch_merge_t merge_info;
	DPatch mrgPatches[2];
	int i;

	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 2 ) {
		DoMessageBox( "Invalid number of objects selected, chose 2 only", "Error", MB_OK );
		return;
	}


	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	for ( i = 0; i < 2; i++ )
	{
		brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( i );

		if ( !brush->pPatch ) {
			g_FuncTable.m_pfnReleaseSelectedBrushHandles();
			DoMessageBox( "You must select ONLY patches", "Error", MB_OK );
			return;
		}

		mrgPatches[i].LoadFromBrush_t( brush );
	}

	/*  mrgPatches[0].Transpose();
	    mrgPatches[0].RemoveFromRadiant();
	    mrgPatches[0].BuildInRadiant();*/

	merge_info = mrgPatches[0].IsMergable( &mrgPatches[1] );

	if ( merge_info.mergable ) {
		Sys_Printf( "%i %i", merge_info.pos1, merge_info.pos2 );

		Sys_Printf( "Patches Mergable\n" );
		DPatch* newPatch = mrgPatches[0].MergePatches( merge_info, &mrgPatches[0], &mrgPatches[1] );

		/*                mrgPatches[0].RemoveFromRadiant();
		   mrgPatches[0].BuildInRadiant();

		   mrgPatches[1].RemoveFromRadiant();
		   mrgPatches[1].BuildInRadiant();


		   delete newPatch;*/

		if ( !newPatch ) {
		}
		else
		{
			mrgPatches[0].RemoveFromRadiant();
			mrgPatches[1].RemoveFromRadiant();

			newPatch->BuildInRadiant();
			delete newPatch;
		}
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoSplitPatch() {
	DPatch patch;

	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of objects selected, select 1 patch only", "Error", MB_OK );
		return;
	}

	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );

	if ( !brush->pPatch ) {
		g_FuncTable.m_pfnReleaseSelectedBrushHandles();
		DoMessageBox( "You must select ONLY patches", "Error", MB_OK );
		return;
	}

	patch.LoadFromBrush_t( brush );

	list<DPatch> patchList = patch.Split( true, true );
	for ( list<DPatch>::iterator patches = patchList.begin(); patches != patchList.end(); patches++ ) {
		( *patches ).BuildInRadiant();
	}

	patch.RemoveFromRadiant();

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoVisAnalyse(){
	char filename[1024];

	if ( g_FuncTable.m_pfnSelectedBrushCount() == 0 ) {
		if ( g_VisView ) {
			delete g_VisView;
			return;
		}
	}

	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		DoMessageBox( "Invalid number of objects selected, select 1 only", "Error", MB_OK );
		return;
	}

	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );

	DBrush orgBrush;
	orgBrush.LoadFromBrush_t( brush, false );

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();

	orgBrush.BuildBounds();
	vec3_t origin;
	origin[0] = ( orgBrush.bbox_max[0] + orgBrush.bbox_min[0] ) / 2.f;
	origin[1] = ( orgBrush.bbox_max[1] + orgBrush.bbox_min[1] ) / 2.f;
	origin[2] = ( orgBrush.bbox_max[2] + orgBrush.bbox_min[2] ) / 2.f;


	char* rad_filename = g_FuncTable.m_pfnGetMapName();
	if ( !rad_filename ) {
		DoMessageBox( "An Error Occurred While Trying\n To Get The Map Filename", "Error", MB_OK );
		return;
	}

	strcpy( filename, rad_filename );

	char* ext = strrchr( filename, '.' ) + 1;
	strcpy( ext, "bsp" ); // rename the extension

	list<DWinding*> *pointList = BuildTrace( filename, origin );

	if ( !g_VisView ) {
		g_VisView = new DVisDrawer;
		g_VisView->Register();
	}

	g_VisView->SetList( pointList );
}

void DoTrainPathPlot() {
	if ( g_TrainView ) {
		delete g_TrainView;
		g_TrainView = NULL;
	}

	g_TrainView = new DTrainDrawer();
}

void DoCaulkSelection( void ) {
	DEntity world;

	float fScale[2] = { 0.5f, 0.5f };
	float fShift[2] = { 0.0f, 0.0f };

	int bResetScale[2] = { false, false };
	int bResetShift[2] = { false, false };

	world.LoadSelectedBrushes();
	world.LoadSelectedPatches();
	world.ResetTextures( NULL, fScale, fShift, 0, "textures/common/caulk", true, bResetScale, bResetShift, false, true );
}

void DoTreePlanter( void ) {
	if ( g_TreePlanter ) {
		delete g_TreePlanter;
		g_TreePlanter = NULL;
		return;
	}

	g_TreePlanter = new DTreePlanter();
}

void DoDropEnts( void ) {
	if ( g_TreePlanter ) {
		g_TreePlanter->DropEntsToGround();
	}
}

void DoMakeChain( void ) {
	DTreePlanter pl;
	pl.MakeChain();
}

typedef DPoint* pntTripple[3];

bool bFacesNoTop[6] = {true, true, true, true, true, false};

void DoFlipTerrain( void ) {
	vec3_t vUp = { 0.f, 0.f, 1.f };
	int i;

	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 2 ) {
		DoMessageBox( "Invalid number of objects selected, chose 2 only", "Error", MB_OK );
		return;
	}

	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	brush_t* brushes[2];
	for ( i = 0; i < 2; i++ ) {
		brushes[i] = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( i );
	}

	DBrush Brushes[2];
	DPlane* Planes[2];
	pntTripple Points[2];
	for ( i = 0; i < 2; i++ ) {
		Brushes[i].LoadFromBrush_t( brushes[i], false );
		if ( !( Planes[i] = Brushes[i].FindPlaneWithClosestNormal( vUp ) ) || Brushes[i].FindPointsForPlane( Planes[i], Points[i], 3 ) != 3 ) {
			g_FuncTable.m_pfnReleaseSelectedBrushHandles();
			DoMessageBox( "Error", "Error", MB_OK );
			return;
		}
	}

	vec3_t mins1, mins2, maxs1, maxs2;
	Brushes[0].GetBounds( mins1, maxs1 );
	Brushes[1].GetBounds( mins2, maxs2 );

	entity_t* ents[2];
	for ( i = 0; i < 2; i++ ) {
		ents[i] = brushes[i]->owner;
		Brushes[i].RemoveFromRadiant();
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();




	int dontmatch[2] = { -1, -1 };
	bool found = false;
	for ( i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3 && !found; j++ ) {
			if ( VectorCompare( ( Points[0] )[i]->_pnt, ( Points[1] )[j]->_pnt ) ) {
				found = true;
				break;
			}
		}
		if ( !found ) {
			dontmatch[0] = i;
			break;
		}
		found = false;
	}
	if ( dontmatch[0] == -1 ) {
		DoMessageBox( "Error", "Error", MB_OK );
		return;
	}

	for ( i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3 && !found; j++ ) {
			if ( VectorCompare( ( Points[1] )[i]->_pnt, ( Points[0] )[j]->_pnt ) ) {
				found = true;
				break;
			}
		}
		if ( !found ) {
			dontmatch[1] = i;
			break;
		}
		found = false;
	}
	if ( dontmatch[1] == -1 ) {
		DoMessageBox( "Error", "Error", MB_OK );
		return;
	}

	vec3_t plnpnts1[3];
	vec3_t plnpnts2[3];
	vec3_t plnpntsshr[3];

	VectorCopy( ( Points[0] )[dontmatch[0]]->_pnt, plnpnts1[0] );
	for ( i = 0; i < 3; i++ ) {
		if ( dontmatch[0] != i ) {
			VectorCopy( ( Points[0] )[i]->_pnt, plnpnts1[1] );
			break;
		}
	}
	VectorCopy( ( Points[1] )[dontmatch[1]]->_pnt, plnpnts1[2] );

	VectorCopy( ( Points[1] )[dontmatch[1]]->_pnt, plnpnts2[0] );
	for ( i = 0; i < 3; i++ ) {
		if ( dontmatch[1] != i && !VectorCompare( ( Points[1] )[i]->_pnt, plnpnts1[1] ) ) {
			VectorCopy( ( Points[1] )[i]->_pnt, plnpnts2[1] );
			break;
		}
	}
	VectorCopy( ( Points[0] )[dontmatch[0]]->_pnt, plnpnts2[2] );

	VectorCopy( ( Points[0] )[dontmatch[0]]->_pnt, plnpntsshr[0] );
	VectorCopy( ( Points[1] )[dontmatch[1]]->_pnt, plnpntsshr[1] );
	if ( ( Points[1] )[dontmatch[1]]->_pnt[2] < ( Points[0] )[dontmatch[0]]->_pnt[2] ) {
		VectorCopy( ( Points[1] )[dontmatch[1]]->_pnt, plnpntsshr[2] );
	}
	else {
		VectorCopy( ( Points[0] )[dontmatch[0]]->_pnt, plnpntsshr[2] );
	}
	plnpntsshr[2][2] -= 16;

	for ( i = 0; i < 3; i++ ) {
		if ( mins2[i] < mins1[i] ) {
			mins1[i] = mins2[i];
		}
		if ( maxs2[i] > maxs1[i] ) {
			maxs1[i] = maxs2[i];
		}
	}

	DBrush* newBrushes[2];
	newBrushes[0] = DShape::GetBoundingCube_Ext( mins1, maxs1, "textures/common/caulk", bFacesAll, true );
	newBrushes[1] = DShape::GetBoundingCube_Ext( mins1, maxs1, "textures/common/caulk", bFacesAll, true );

	vec3_t normal;
	MakeNormal( plnpnts1[0], plnpnts1[1], plnpnts1[2], normal );
	if ( normal[2] >= 0 ) {
		newBrushes[0]->AddFace( plnpnts1[0], plnpnts1[1], plnpnts1[2], "textures/common/terrain", true );
	}
	else {
		newBrushes[0]->AddFace( plnpnts1[2], plnpnts1[1], plnpnts1[0], "textures/common/terrain", true );
	}

	MakeNormal( plnpnts2[0], plnpnts2[1], plnpnts2[2], normal );
	if ( normal[2] >= 0 ) {
		newBrushes[1]->AddFace( plnpnts2[0], plnpnts2[1], plnpnts2[2], "textures/common/terrain", true );
	}
	else {
		newBrushes[1]->AddFace( plnpnts2[2], plnpnts2[1], plnpnts2[0], "textures/common/terrain", true );
	}

	vec3_t vec;
	MakeNormal( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], normal );

	VectorSubtract( plnpnts1[2], plnpnts1[1], vec );
	if ( DotProduct( vec, normal ) >= 0 ) {
		newBrushes[0]->AddFace( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], "textures/common/caulk", true );
	}
	else {
		newBrushes[0]->AddFace( plnpntsshr[2], plnpntsshr[1], plnpntsshr[0], "textures/common/caulk", true );
	}

	VectorSubtract( plnpnts2[2], plnpnts2[1], vec );
	if ( DotProduct( vec, normal ) >= 0 ) {
		newBrushes[1]->AddFace( plnpntsshr[0], plnpntsshr[1], plnpntsshr[2], "textures/common/caulk", true );
	}
	else {
		newBrushes[1]->AddFace( plnpntsshr[2], plnpntsshr[1], plnpntsshr[0], "textures/common/caulk", true );
	}

	for ( i = 0; i < 2; i++ ) {
		newBrushes[i]->RemoveRedundantPlanes();
		newBrushes[i]->BuildInRadiant( false, NULL, ents[i] );
		delete newBrushes[i];
	}

}
