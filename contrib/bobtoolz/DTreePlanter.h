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

#ifndef __DTREE_H__
#define __DTREE_H__

#include "../include/igl.h"
#include "DEntity.h"
#include "misc.h"
#include "ScriptParser.h"

#define MAX_QPATH 64

typedef struct treeModel_s {
	char name[MAX_QPATH];
} treeModel_t;

#define MAX_TP_MODELS 256

class DTreePlanter : public IWindowListener {
public:
virtual bool OnMouseMove( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnLButtonDown( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnMButtonDown( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnRButtonDown( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnLButtonUp( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnMButtonUp( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnRButtonUp( guint32 nFlags, gdouble x, gdouble y );
virtual bool OnKeyPressed( char *s ) { return false; }
virtual bool Paint() { return true; }
virtual void Close() { }

DTreePlanter() {
	m_refCount =    1;
	m_hooked =      false;
	m_XYWrapper =   NULL;
	m_numModels =   0;
	m_offset =      0;
	m_maxPitch =    0;
	m_minPitch =    0;
	m_maxYaw =      0;
	m_minYaw =      0;
	m_setAngles =   false;
	m_useScale =    false;
	m_autoLink =    false;
	m_linkNum =     0;

	Register();

	m_world.LoadSelectedBrushes();

	char buffer[256];
	GetFilename( buffer, "bt/tp_ent.txt" );

	FILE* file = fopen( buffer, "rb" );
	if ( file ) {
		fseek( file, 0, SEEK_END );
		int len = ftell( file );
		fseek( file, 0, SEEK_SET );

		if ( len ) {
			char* buf = new char[len + 1];
			buf[len] = '\0';
			// parser will do the cleanup, dont delete.

			fread( buf, len, 1, file );

			CScriptParser parser;
			parser.SetScript( buf );

			ReadConfig( &parser );
		}

		fclose( file );
	}
}

#define MT( t )   !stricmp( pToken, t )
#define GT      pToken = pScriptParser->GetToken( true )
#define CT      if ( !*pToken ) { return; }

void ReadConfig( CScriptParser* pScriptParser ) {
	const char* GT;
	CT;

	do {
		GT;
		if ( *pToken == '}' ) {
			break;
		}

		if ( MT( "model" ) ) {
			if ( m_numModels >= MAX_TP_MODELS ) {
				return;
			}

			GT; CT;

			strncpy( m_trees[m_numModels++].name, pToken, MAX_QPATH );
		}
		else if ( MT( "link" ) ) {
			GT; CT;

			strncpy( m_linkName, pToken, MAX_QPATH );

			m_autoLink = true;
		}
		else if ( MT( "entity" ) ) {
			GT; CT;

			strncpy( m_entType, pToken, MAX_QPATH );
		}
		else if ( MT( "offset" ) ) {
			GT; CT;

			m_offset = atoi( pToken );
		}
		else if ( MT( "pitch" ) ) {
			GT; CT;

			m_minPitch = atoi( pToken );

			GT; CT;

			m_maxPitch = atoi( pToken );

			m_setAngles = true;
		}
		else if ( MT( "yaw" ) ) {
			GT; CT;

			m_minYaw = atoi( pToken );

			GT; CT;

			m_maxYaw = atoi( pToken );

			m_setAngles = true;
		}
		else if ( MT( "scale" ) ) {
			GT; CT;

			m_minScale = static_cast< float >( atof( pToken ) );

			GT; CT;

			m_maxScale = static_cast< float >( atof( pToken ) );

			m_useScale = true;
		}
		else if ( MT( "numlinks" ) ) {
			GT; CT;

			m_linkNum = atoi( pToken );
		}
	} while ( true );
}

virtual ~DTreePlanter() {
	UnRegister();
}

virtual void IncRef() { m_refCount++; }
virtual void DecRef() {
	m_refCount--; if ( m_refCount <= 0 ) {
		delete this;
	}
}

void Register() {
	if ( !m_hooked ) {
		g_MessageTable.m_pfnHookWindow( this );
		m_XYWrapper = g_MessageTable.m_pfnGetXYWndWrapper();
		m_hooked = true;
	}
}

void UnRegister() {
	if ( m_hooked ) {
		g_MessageTable.m_pfnUnHookWindow( this );
		m_XYWrapper = NULL;
		m_hooked = false;
	}
}

bool FindDropPoint( vec3_t in, vec3_t out );
void DropEntsToGround( void );
void MakeChain( void );
void SelectChain( void );

private:
IXYWndWrapper*  m_XYWrapper;
DEntity m_world;

treeModel_t m_trees[MAX_TP_MODELS];

int m_refCount;
int m_numModels;
int m_offset;
int m_maxPitch;
int m_minPitch;
int m_maxYaw;
int m_minYaw;

char m_entType[MAX_QPATH];
char m_linkName[MAX_QPATH];
int m_linkNum;

float m_minScale;
float m_maxScale;

bool m_hooked;
bool m_useScale;
bool m_setAngles;
bool m_autoLink;
};

#endif
