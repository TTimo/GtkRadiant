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

#include "funchandlers.h"

#include "IntersectDialog.h"
#include "PolygonDialog.h"
#include "StairDialog.h"
#include "DoorDialog.h"
#include "IntersectInfoDialog.h"
#include "BrushCheckDialog.h"
#include "AutoCaulkDialog.h"
#include "AutoCaulkStartDialog.h"
#include "TextureResetDialog.h"
#include "pathplotterdialog.h"

#include "DEntity.h"
#include "shapes.h"
#include "lists.h"
#include "misc.h"
#include "DShape.h"

// for autocaulk
list<Str> exclusionList;        // whole brush exclusion
list<Str> exclusionList_Face;   // single face exclusion

BOOL el1Loaded;
BOOL el2Loaded;

DBobView*   g_PathView = NULL;
// -------------

/************************
    Global Variables
************************/

CPolygonDialog polygonDlg;
CIntersectDialog intrDlg;
CStairDialog stairDlg;
CDoorDialog doorDlg;
CAutoCaulkStartDialog autocaulkDlg;
CTextureResetDialog texRstDlg;
CPathPlotterDialog ppDlg;

/************************
    --Main Functions--
************************/

void LoadLists(){
	char buffer[256];

	if ( !el1Loaded ) {
		el1Loaded = LoadExclusionList( GetFilename( buffer, "bt\\bt-el1.txt" ), &exclusionList );
	}
	if ( !el2Loaded ) {
		el2Loaded = LoadExclusionList( GetFilename( buffer, "bt\\bt-el2.txt" ), &exclusionList );
	}
}

void PolygonBuilder( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		MessageBox( NULL, "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...

	// ask user for type, size, etc....
	if ( polygonDlg.DoModal() == IDOK ) {
		DShape poly;

		g_FuncTable.m_pfnDeleteBrushHandle( brush );

		if ( polygonDlg.m_bInverse ) {
			poly.BuildInversePrism( vMin, vMax, polygonDlg.m_nSideCount, polygonDlg.m_bAlignTop );
		}
		else
		{
			if ( polygonDlg.m_bBorder ) {
				poly.BuildBorderedPrism( vMin, vMax, polygonDlg.m_nSideCount, polygonDlg.m_nBorderSize, polygonDlg.m_bAlignTop );
			}
			else{
				poly.BuildRegularPrism( vMin, vMax, polygonDlg.m_nSideCount, polygonDlg.m_bAlignTop );
			}
		}

		poly.Commit();
	}


	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}


void IntersectionFinder(){
	if ( intrDlg.DoModal() == IDCANCEL ) {
		return;
	}

	if ( intrDlg.m_nBrushOptions == BRUSH_OPT_SELECTED ) {
		// ensure we have enough brushes selected
		if ( g_FuncTable.m_pfnSelectedBrushCount() < 2 ) {
			MessageBox( NULL, "Invalid number of brushes selected, choose at least 2", "Error", MB_OK );
			return;
		}
	}

	CIntersectInfoDialog*   intrInfoDlg = new CIntersectInfoDialog();
	intrInfoDlg->Create( IDD_INTERSECT_INFO_DIALOG );

	DEntity world;

	switch ( intrDlg.m_nBrushOptions )
	{
	case BRUSH_OPT_SELECTED:
	{
		world.LoadSelectedBrushes( &intrInfoDlg->m_prog1 );
		break;
	}
	case BRUSH_OPT_WHOLE_MAP:
	{
		world.LoadFromEntity( 0, &intrInfoDlg->m_prog1 );
		break;
	}
	}

	world.RemoveNonCheckBrushes( &exclusionList, intrDlg.m_bUseDetail );
	BOOL* pbSelectList;
	if ( intrDlg.m_bDuplicateOnly ) {
		pbSelectList = world.BuildDuplicateList();
	}
	else{
		pbSelectList = world.BuildIntersectList();
	}

	world.SelectBrushes( pbSelectList );

	intrInfoDlg->DestroyWindow();
	delete[] pbSelectList;
}

void StairBuilder( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		MessageBox( NULL, "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...


	// ask user for type, size, etc....
	if ( stairDlg.DoModal() == IDOK ) {

		// calc brush size
		vec3_t size;
		_VectorSubtract( vMax, vMin, size );


		if ( ( (int)size[2] % stairDlg.m_nStairHeight ) != 0 ) {
			// stairs must fit evenly into brush
			MessageBox( NULL, "Invalid stair height\nHeight of block must be divisable by stair height", "Error", MB_OK );
		}
		else
		{

			// Remove Size Brush
			g_FuncTable.m_pfnDeleteBrushHandle( brush );


			// Get Step Count, Direction of Stairs, Stair Style
			int numSteps = (int)size[2] / stairDlg.m_nStairHeight;
			int direction = stairDlg.m_StairDir;
			int style = stairDlg.m_StairStyle;

			if ( stairDlg.m_StairStyle == STYLE_CORNER ) {
				BuildCornerStairs( vMin, vMax, numSteps, "textures/common/caulk", (LPCTSTR)stairDlg.m_riserTexture );
			}
			else
			{
				// Get Step Dimensions
				float stairHeight = (float)stairDlg.m_nStairHeight;
				float stairWidth;
				if ( ( direction == MOVE_EAST ) || ( direction == MOVE_WEST ) ) {
					stairWidth = ( size[0] ) / numSteps;
				}
				else{
					stairWidth = ( size[1] ) / numSteps;
				}


				// Build Base For Stair (bob's style)
				if ( style == STYLE_BOB ) {
					Build_Wedge( direction, vMin, vMax, TRUE );
				}


				// Set First Step Starting Position
				vMax[2] = vMin[2] + stairHeight;
				SetInitialStairPos( direction, vMin, vMax, stairWidth );


				// Build The Steps
				for ( int i = 0; i < numSteps; i++ )
				{
					if ( style == STYLE_BOB ) {
						Build_StairStep_Wedge( direction, vMin, vMax, "textures/common/caulk", (LPCTSTR)stairDlg.m_riserTexture, stairDlg.m_bDetail );
					}
					else if ( style == STYLE_ORIGINAL ) {
						Build_StairStep( vMin, vMax, "textures/common/caulk", (LPCTSTR)stairDlg.m_riserTexture, direction );
					}

					// get step into next position
					MoveBlock( direction, vMin, vMax, stairWidth );
					vMax[2] += stairHeight;
					if ( style == STYLE_BOB ) {
						vMin[2] += stairHeight; // wedge bottom must be raised
					}
				}
			}
		}
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DoorBuilder( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		MessageBox( NULL, "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...



	strcpy( doorDlg.m_fbTextureName.GetBuffer( 256 ), g_FuncTable.m_pfnGetCurrentTexture() );

	if ( doorDlg.DoModal() == IDOK ) {
		g_FuncTable.m_pfnDeleteBrushHandle( brush );

		BuildDoorsX2( vMin, vMax,
					  doorDlg.m_bSclMainHor, doorDlg.m_bSclMainVert,
					  doorDlg.m_bSclTrimHor, doorDlg.m_bSclTrimVert,
					  (LPCTSTR)doorDlg.m_fbTextureName,
					  (LPCTSTR)doorDlg.m_trimTextureName,
					  doorDlg.m_doorDirection );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void FixBrushes(){
	DEntity world;

	CIntersectInfoDialog*   intrInfoDlg = new CIntersectInfoDialog();
	intrInfoDlg->Create( IDD_INTERSECT_INFO_DIALOG );

	world.LoadFromEntity( 0, &intrInfoDlg->m_prog1 );

	intrInfoDlg->DestroyWindow();

	CBrushCheckDialog*  chkDlg = new CBrushCheckDialog();
	chkDlg->Create( IDD_BRUSHCHECKER_DIALOG );

	int count = world.FixBrushes( TRUE, &chkDlg->m_prog1 );

	chkDlg->DestroyWindow();

	Sys_Printf( "%i invalid/duplicate planes removed\n", count );
}

void AutoCaulk(){
	if ( !el1Loaded ) {
		autocaulkDlg.m_Warning1 = "WARNING: Brush exclusion list not found\n, ALL BRUSHES WILL BE USED";
	}

	if ( autocaulkDlg.DoModal() == IDCANCEL ) {
		return;
	}

	if ( autocaulkDlg.m_nMode == MODE_AC_BUILD_MINI_PRT ) {
		BuildMiniPrt( &exclusionList );
		return;
	}

	CAutoCaulkDialog* acDlg = new CAutoCaulkDialog;
	acDlg->Create( IDD_AUTOCAULK_DIALOG );

	char filename[1204];

	if ( autocaulkDlg.m_nMode == MODE_AC_NORMAL ) {
		char* rad_filename = g_BSPTable.m_pfnGetMapName();
		if ( !rad_filename ) {
			MessageBox( NULL, "An Error Occurred While Trying To Get The Map Filename", "Error", MB_OK );
			acDlg->DestroyWindow();
			return;
		}

		strcpy( filename, rad_filename );

		char* ext = strrchr( filename, '.' ) + 1;
		strcpy( ext, "prt" ); // rename the extension
	}
	else
	{
		IEpair* pEp = g_EpairTable.m_pfnIEpairForProjectKeys();
		char *pn = pEp->ValueForKey( "mapspath" );
		pEp->DecRef();

		strcpy( filename, pn );
		strcat( filename, "/ac_prt.prt" );
	}

	DEntity portals;
	if ( !portals.LoadFromPrt( filename, &acDlg->m_prog1 ) ) {
		MessageBox( NULL, "Failed To Load Portal File", "Error", MB_OK );
		acDlg->DestroyWindow();
		return;
	}
	// load portal file

	CIntersectInfoDialog*   intrInfoDlg = new CIntersectInfoDialog();
	intrInfoDlg->Create( IDD_INTERSECT_INFO_DIALOG );

	DEntity world;

	world.LoadFromEntity( 0, &intrInfoDlg->m_prog1 );
	intrInfoDlg->DestroyWindow();

	if ( autocaulkDlg.m_nMode == MODE_AC_NORMAL ) {
		world.RemoveNonCheckBrushes( &exclusionList, FALSE );
	}
	else{
		world.RemoveNonCheckBrushes( &exclusionList, TRUE );
	}

	world.ResetChecks( &exclusionList_Face );

	int caulkedCount = 0;
	int killCnt = world.AutoCaulk( &portals, autocaulkDlg.m_bAllowDestruction, &caulkedCount, &acDlg->m_prog2 );

	if ( autocaulkDlg.m_bAllowDestruction ) {
		Sys_Printf( "%i unrequired brush(es) killed\n", killCnt );
	}
	Sys_Printf( "%i face(s) caulked\n", caulkedCount );

	acDlg->DestroyWindow();
}

void ResetTextures(){
	texRstDlg.m_TextureName = GetCurrentTexture();
	texRstDlg.m_NewTextureName = GetCurrentTexture();

	if ( texRstDlg.DoModal() == IDCANCEL ) {
		return;
	}

	float fScale[2];
	float fShift[2];
	fScale[1] = texRstDlg.m_fScaleVertical;
	fScale[0] = texRstDlg.m_fScaleHorizontal;

	fShift[1] = (float)texRstDlg.m_nShiftVertical;
	fShift[0] = (float)texRstDlg.m_nShiftHorizontal;

	DEntity world;
	world.LoadFromEntity( 0, NULL );

	if ( texRstDlg.m_bAllTextures ) {
		world.ResetTextures( NULL, fScale, fShift, texRstDlg.m_nRotation, texRstDlg.m_NewTextureName, texRstDlg.m_bOnlyTexture );
	}
	else{
		world.ResetTextures( texRstDlg.m_TextureName, fScale, fShift, texRstDlg.m_nRotation, texRstDlg.m_NewTextureName, texRstDlg.m_bOnlyTexture );
	}
}

void PathPlotter(){
	int ret = ppDlg.DoModal();
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
		MessageBox( NULL, "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
		return;
	}

	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	// get handle to size definition brush
	brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	// cant release until we delete the brush, if we do...

	DEntity world;
	world.LoadEPairList( *g_FuncTable.m_pfnGetEntityKeyValList( brush->owner ) );

	DEPair* trigger_ep = world.FindEPairByKey( "targetname" );

	if ( trigger_ep ) {
		if ( !strcmp( world.m_Classname, "trigger_push" ) ) {
			DEPair* target_ep = world.FindEPairByKey( "target" );
			if ( target_ep ) {
				entity_s* entTarget = FindEntityFromTargetname( target_ep->value );
				if ( entTarget ) {
					if ( g_PathView ) {
						delete g_PathView;
					}
					g_PathView = new DBobView;

					g_PathView->Begin( trigger_ep->value, target_ep->value, ppDlg.m_fMultiplier, ppDlg.m_nPoints, ppDlg.m_fGravity, ppDlg.m_bNoUpdate, ppDlg.m_bShowExtra );
				}
				else{
					MessageBox( NULL, "trigger_push target could not be found.", "Error", MB_OK );
				}
			}
			else{
				MessageBox( NULL, "trigger_push has no target.", "Error", MB_OK );
			}
		}
		else{
			MessageBox( NULL, "You must select a 'trigger_push' entity.", "Error", MB_OK );
		}
	}
	else{
		MessageBox( NULL, "Entity must have a targetname", "Error", MB_OK );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void PitBuilder( vec3_t vMin, vec3_t vMax ){
	// ensure we have something selected
	if ( g_FuncTable.m_pfnSelectedBrushCount() != 1 ) {
		MessageBox( NULL, "Invalid number of brushes selected, chose 1 only", "Error", MB_OK );
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
		MessageBox( NULL, "Failed To Make Pit\nTry Making The Brush Bigger", "Error", MB_OK );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}
