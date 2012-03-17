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

#include "dialogs/dialogs-gtk.h"

#include "DEntity.h"
#include "DMap.h"

#include "misc.h"
#include "lists.h"
#include "funchandlers.h"

// for ctf texture changer
list<Str> clrList_Blue;
list<Str> clrList_Red;

BOOL clrLst1Loaded = FALSE;
BOOL clrLst2Loaded = FALSE;

// -------------

//========================//
//    Helper Functions    //
//========================//

void LoadLists(){
	char buffer[256];

	if ( !clrLst1Loaded ) {
		clrLst1Loaded = LoadExclusionList( GetFilename( buffer, "plugins/bt/ctf-blue.txt" ), &clrList_Blue );
		LoadExclusionList( GetFilename( buffer, "plugins/bt/blue.txt" ), &clrList_Blue );
	}
	if ( !clrLst2Loaded ) {
		clrLst2Loaded = LoadExclusionList( GetFilename( buffer, "plugins/bt/ctf-red.txt" ), &clrList_Red );
		LoadExclusionList( GetFilename( buffer, "plugins/bt/red.txt" ), &clrList_Red );
	}
}


//========================//
//     Main Functions     //
//========================//

void DoCTFColourChanger(){
	if ( !clrLst1Loaded || !clrLst2Loaded ) {
		DoMessageBox( "CTF texture lists not found, this function will terminate.", "Error", MB_OK );
		return;
	}

	int ret = DoCTFColourChangeBox();
	if ( ret == IDCANCEL ) {
		return;
	}

	int cnt = Min( clrList_Blue.size(), clrList_Red.size() );

	list<Str>::const_iterator Texture_change;
	list<Str>::const_iterator Texture_new;

	float fDummy[2];

	int eCnt = g_FuncTable.m_pfnGetEntityCount();

	DMap world;
	world.LoadAll( TRUE );

	if ( ret == IDYES ) {
		Texture_change =    clrList_Blue.begin();
		Texture_new =       clrList_Red.begin();
	}
	else
	{
		Texture_change =    clrList_Red.begin();
		Texture_new =       clrList_Blue.begin();
	}

	for ( int i = 0; i < cnt; i++ )
	{
		world.ResetTextures( ( *Texture_change ).c_str(), fDummy, fDummy, 0, ( *Texture_new ).c_str(), TRUE );

		Texture_change++;
		Texture_new++;
	}
}

void DoSwapLights(){
/*	DMap world;
    world.LoadAll();

    for(list<DEntity*>::const_iterator loopEnt = world.entityList.begin(); loopEnt != world.entityList.end(); loopEnt++)
    {
        DEntity* e = (*loopEnt);
        DEPair* epLightColour = e->FindEPairByKey("_color");
        if(epLightColour)
        {
            float r, g, b;
            sscanf(epLightColour->value, "%f %f %f", &r, &g, &b);
            sprintf(epLightColour->value, "%f %f %f", b, g, r);
            DMap::RebuildEntity(e);
        }
    }*/

	int cnt = g_FuncTable.m_pfnGetEntityCount();

	for ( int i = 0; i < cnt; i++ )
	{
		void* ent = g_FuncTable.m_pfnGetEntityHandle( i );

		for ( epair_t* epList = *g_FuncTable.m_pfnGetEntityKeyValList( ent ); epList; epList = epList->next )
		{
			if ( !stricmp( "_color", epList->key ) ) {
				float r, g, b;
				sscanf( epList->value, "%f %f %f", &r, &g, &b );
				sprintf( epList->value, "%f %f %f", b, g, r );
			}
		}
	}
}

void DoChangeAngles(){
	int cnt = g_FuncTable.m_pfnGetEntityCount();

	for ( int i = 0; i < cnt; i++ )
	{
		void* ent = g_FuncTable.m_pfnGetEntityHandle( i );

		for ( epair_t* epList = *g_FuncTable.m_pfnGetEntityKeyValList( ent ); epList; epList = epList->next )
		{
			if ( !stricmp( "angle", epList->key ) ) {
				float angle;
				sscanf( epList->value, "%f", &angle );
				angle += 180;
				while ( angle > 360 )
					angle -= 360;

				sprintf( epList->value, "%f", angle );
			}
		}
	}
}

void DoSwapSpawns(){
	int cnt = g_FuncTable.m_pfnGetEntityCount();

	for ( int i = 0; i < cnt; i++ )
	{
		void* ent = g_FuncTable.m_pfnGetEntityHandle( i );

		for ( epair_t* epList = *g_FuncTable.m_pfnGetEntityKeyValList( ent ); epList; epList = epList->next )
		{
			if ( !stricmp( "classname", epList->key ) ) {
				if ( !strcmp( epList->value, "team_CTF_redplayer" ) ) {
					sprintf( epList->value, "team_CTF_blueplayer" );
				}
				else if ( !strcmp( epList->value, "team_CTF_blueplayer" ) ) {
					sprintf( epList->value, "team_CTF_redplayer" );
				}

				if ( !strcmp( epList->value, "team_CTF_redspawn" ) ) {
					sprintf( epList->value, "team_CTF_bluespawn" );
				}
				else if ( !strcmp( epList->value, "team_CTF_bluespawn" ) ) {
					sprintf( epList->value, "team_CTF_redspawn" );
				}

				if ( !strcmp( epList->value, "team_CTF_redflag" ) ) {
					sprintf( epList->value, "team_CTF_blueflag" );
				}
				else if ( !strcmp( epList->value, "team_CTF_blueflag" ) ) {
					sprintf( epList->value, "team_CTF_redflag" )
					;
				}
				if ( !strcmp( epList->value, "team_redobelisk" ) ) {
					sprintf( epList->value, "team_blueobelisk" );
				}
				else if ( !strcmp( epList->value, "team_blueobelisk" ) ) {
					sprintf( epList->value, "team_redobelisk" );
				}
			}
		}
	}
}

/*void test()
   {
    DMap world;
    world.LoadAll();

    for(list<DEntity*>::const_iterator ents = world.entityList.begin(); ents != world.entityList.end(); ents++)
    {
        (*ents)->RemoveFromRadiant();
    }
   }*/
