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

#include "qerplugin.h"
#include "signal/isignal.h"
#include "string/string.h"

#include "DEntity.h"
#include "scriptparser.h"
#include "mathlib.h"
#include "misc.h"

#define MAX_QPATH 64

typedef struct treeModel_s {
	char name[MAX_QPATH];
} treeModel_t;

#define MAX_TP_MODELS 256

class DTreePlanter {
  MouseEventHandlerId m_mouseDown;
public:
	SignalHandlerResult mouseDown(const WindowVector& position, ButtonIdentifier button, ModifierFlags modifiers);
  typedef Member3<DTreePlanter, const WindowVector&, ButtonIdentifier, ModifierFlags, SignalHandlerResult, &DTreePlanter::mouseDown> MouseDownCaller;

  DTreePlanter() {
		m_numModels =	0;
		m_offset =		0;
		m_maxPitch =	0;
		m_minPitch =	0;
		m_maxYaw =		0;
		m_minYaw =		0;
		m_setAngles =	false;
		m_useScale =	false;
		m_autoLink =	false;
		m_linkNum =		0;

		m_world.LoadSelectedBrushes();

		char buffer[256];
		GetFilename( buffer, "bt/tp_ent.txt" );

		FILE* file = fopen( buffer, "rb" );
		if(file) {
			fseek( file, 0, SEEK_END );
			int len = ftell( file );
			fseek( file, 0, SEEK_SET );

			if(len) {
				char* buf = new char[len+1];
				buf[len] = '\0';
				// parser will do the cleanup, dont delete.

				fread( buf, len, 1, file );

				CScriptParser parser;
				parser.SetScript( buf );

				ReadConfig( &parser );
			}

			fclose( file );
		}

    m_mouseDown = GlobalRadiant().XYWindowMouseDown_connect(makeSignalHandler3(MouseDownCaller(), *this));
	}

  virtual ~DTreePlanter()
  {
    GlobalRadiant().XYWindowMouseDown_disconnect(m_mouseDown);
  }

#define MT(t)	string_equal_nocase( pToken, t )
#define GT		pToken = pScriptParser->GetToken( true )
#define CT		if(!*pToken) { return; }

	void ReadConfig( CScriptParser* pScriptParser ) {
		const char* GT;
		CT;

		do {
			GT;
			if(*pToken == '}') {
				break;
			}

			if(MT("model")) {
				if(m_numModels >= MAX_TP_MODELS) {
					return;
				}

				GT; CT;

				strncpy( m_trees[m_numModels++].name, pToken, MAX_QPATH );
			} else if(MT("link")) {
				GT; CT;

				strncpy( m_linkName, pToken, MAX_QPATH );

				m_autoLink = true;
			} else if(MT("entity")) {
				GT; CT;

				strncpy( m_entType, pToken, MAX_QPATH );
			} else if(MT("offset")) {
				GT; CT;

				m_offset = atoi(pToken);
			} else if(MT("pitch")) {
				GT; CT;

				m_minPitch = atoi(pToken);

				GT; CT;

				m_maxPitch = atoi(pToken);

				m_setAngles = true;
			} else if(MT("yaw")) {
				GT; CT;

				m_minYaw = atoi(pToken);

				GT; CT;

				m_maxYaw = atoi(pToken);

				m_setAngles = true;
			} else if(MT("scale")) {
				GT; CT;

				m_minScale = static_cast<float>(atof(pToken));

				GT; CT;

				m_maxScale = static_cast<float>(atof(pToken));

				m_useScale = true;
			} else if(MT("numlinks")) {
				GT; CT;

				m_linkNum = atoi( pToken );
			}
		} while( true );
	}

	bool FindDropPoint(vec3_t in, vec3_t out);
	void DropEntsToGround( void );
	void MakeChain( void );
	void SelectChain( void );

private:
	DEntity			m_world;

	treeModel_t		m_trees[MAX_TP_MODELS];

	int				m_numModels;
	int				m_offset;
	int				m_maxPitch;
	int				m_minPitch;
	int				m_maxYaw;
	int				m_minYaw;

	char			m_entType[MAX_QPATH];
	char			m_linkName[MAX_QPATH];
	int				m_linkNum;

	float			m_minScale;
	float			m_maxScale;

	bool			m_useScale;
	bool			m_setAngles;
	bool			m_autoLink;
};

#endif
