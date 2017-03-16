/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

// BrushScript stuff
//

/*!
   \todo is this still used / in working state?
   should we cleanup and remove it for good
 */

#include <glib/gi18n.h>

#include "stdafx.h"
#include "gtkmisc.h"

//
struct SVariableDef
{
	CString m_strName;
	CString m_strInput;
	float m_fValue;
};

struct SVecVariableDef
{
	CString m_strName;
	CString m_strInput;
	vec3_t m_vValue;
};



const int MAX_VARIABLES = 64;

brush_t* g_pHold1 = NULL;
brush_t* g_pHold2 = NULL;
brush_t* g_pHold3 = NULL;
bool g_bRotateAroundSelection;
int g_nVariableCount;
int g_nVecVariableCount;
int g_nLoopCounter;
float g_fDefault = 9999.9f;
vec3_t g_vDefault;
bool g_bStartLoop;
char* g_pLooper;
bool g_bKeepGoing;

SVariableDef g_Variables[MAX_VARIABLES];
SVecVariableDef g_VecVariables[MAX_VARIABLES];

void InitForScriptRun(){
	g_pHold1 = NULL;
	g_pHold2 = NULL;
	g_pHold3 = NULL;
	g_bRotateAroundSelection = true;
	g_nVariableCount = 0;
	g_nVecVariableCount = 0;
	g_nLoopCounter = 0;
	g_bStartLoop = false;
	g_pLooper = NULL;
	g_bKeepGoing = true;
}

void AddVariable( const char* pName, float fValue, const char* pInput = NULL ){
	if ( g_nVariableCount < MAX_VARIABLES ) {
		g_Variables[g_nVariableCount].m_strName = pName;
		g_Variables[g_nVariableCount].m_strName.MakeLower();
		g_Variables[g_nVariableCount].m_fValue = fValue;
		if ( pInput ) {
			g_Variables[g_nVariableCount].m_strInput = pInput;
		}
		g_nVariableCount++;
	}
	else{
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "Maximum script variable limit reached!" ) );
	}
}

float VariableValue( const char* pName ){
	CString strName = pName;
	strName.MakeLower();
	for ( int n = 0; n < g_nVariableCount; n++ )
	{
		if ( strName == g_Variables[n].m_strName ) {
			return g_Variables[n].m_fValue;
		}
	}
	//strName.Format("Reference to non-existant varirable %s", pName);
	//g_pParentWnd->MessageBox(strName);
	return g_fDefault;
}

void SetVariableValue( const char* pName, float fValue ){
	CString strName = pName;
	strName.MakeLower();
	for ( int n = 0; n < g_nVariableCount; n++ )
	{
		if ( strName == g_Variables[n].m_strName ) {
			g_Variables[n].m_fValue = fValue;
		}
	}
}



void AddVectorVariable( const char* pName, const char* pInput = NULL ){
	if ( g_nVecVariableCount < MAX_VARIABLES ) {
		g_VecVariables[g_nVecVariableCount].m_strName = pName;
		g_VecVariables[g_nVecVariableCount].m_strName.MakeLower();
		if ( pInput ) {
			g_VecVariables[g_nVariableCount].m_strInput = pInput;
		}
		g_nVecVariableCount++;
	}
	else{
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "Maximum script variable limit reached!" ) );
	}
}

void VectorVariableValue( const char* pName, vec3_t& v ){
	CString strName = pName;
	strName.MakeLower();
	for ( int n = 0; n < g_nVecVariableCount; n++ )
	{
		if ( strName == g_VecVariables[n].m_strName ) {
			VectorCopy( g_VecVariables[n].m_vValue, v );
			return;
		}
	}
	strName.Format( "Reference to non-existant variable %s", pName );
	gtk_MessageBox( g_pParentWnd->m_pWidget, strName );
}

void SetVectorVariableValue( const char* pName, vec3_t v ){
	CString strName = pName;
	strName.MakeLower();
	for ( int n = 0; n < g_nVecVariableCount; n++ )
	{
		if ( strName == g_VecVariables[n].m_strName ) {
			VectorCopy( v, g_VecVariables[n].m_vValue );
		}
	}
}





// commands
//
// _CopySelected(nHoldPos)
// copies selected brush to hold spot 1, 2 or 3
//
// _MoveSelected(x, y, z)
// moves selected brush by coords provided
//
// _RotateSelected(x, y, z)
// rotates selected brush by coords provided
//
// _MoveHold(nHoldPos, x, y, z)
// moves brush in hold pos by coords provided
//
// _RotateHold(nHoldPos, x, y, z)
// rotates brush in hold pos by coords provided
//
// _CopyToMap(nHoldPos)
// copies hold brush to map
//
// _CopyAndSelect(nHoldPos)
// copies hold brush to map and selects it
//
// _Input(VarName1, ... VarNamennn)
// inputs a list of values from the user
//

typedef void ( PFNScript )( char*& );


struct SBrushScript
{
	const char* m_pName;
	PFNScript* m_pProc;
};


const char* GetParam( char*& pBuffer ){
	static CString strParam;
	bool bStringMode = false;

	while ( *pBuffer != (char)NULL && isspace( *pBuffer ) ) // skip and whitespace
		pBuffer++;

	if ( *pBuffer == '(' ) { // if it's an opening paren, skip it
		pBuffer++;
	}

	if ( *pBuffer == '\"' ) { // string ?
		pBuffer++;
		bStringMode = true;
	}

	strParam = "";

	if ( bStringMode ) {
		while ( *pBuffer != (char)NULL && *pBuffer != '\"' )
			strParam += *pBuffer++;
	}
	else
	{
		while ( *pBuffer != (char)NULL && *pBuffer != ' ' && *pBuffer != ')' && *pBuffer != ',' )
			strParam += *pBuffer++;
	}

	if ( *pBuffer != (char)NULL ) { // skip last char
		pBuffer++;
	}

	if ( strParam.GetLength() > 0 ) {
		if ( strParam.GetAt( 0 ) == '$' ) { // ? variable name
			float f = VariableValue( strParam );
			if ( f != g_fDefault ) {
				strParam.Format( "%f", f );
			}
		}
	}

	return strParam;
}

brush_t* CopyBrush( brush_t* p ){
	brush_t* pCopy = Brush_Clone( p );
	//Brush_AddToList (pCopy, &active_brushes);
	//Entity_LinkBrush (world_entity, pCopy);
	Brush_Build( pCopy, false );

	return pCopy;
}


void CopySelected( char*& pBuffer ){
	// expects one param
	CString strParam = GetParam( pBuffer );
	int n = atoi( strParam );

	brush_t* pCopy = NULL;
	if ( selected_brushes.next != &selected_brushes &&
		 selected_brushes.next->next == &selected_brushes ) {
		pCopy = selected_brushes.next;
	}

	if ( pCopy ) {
		if ( n == 1 ) {
			//if (g_pHold1)
			//Brush_Free(g_pHold1);
			g_pHold1 = CopyBrush( pCopy );
		}
		else if ( n == 2 ) {
			//if (g_pHold2)
			//Brush_Free(g_pHold2);
			g_pHold2 = CopyBrush( pCopy );
		}
		else
		{
			//if (g_pHold3)
			//Brush_Free(g_pHold3);
			g_pHold3 = CopyBrush( pCopy );
		}
	}
}

void MoveSelected( char*& pBuffer ){
	vec3_t v;
	CString strParam = GetParam( pBuffer );
	v[0] = atof( strParam );
	strParam = GetParam( pBuffer );
	v[1] = atof( strParam );
	strParam = GetParam( pBuffer );
	v[2] = atof( strParam );
	Select_Move( v, false );
	Sys_UpdateWindows( W_ALL );
}

void RotateSelected( char*& pBuffer ){
	vec3_t v;

	if ( g_bRotateAroundSelection ) {
		Select_GetTrueMid( v );
		VectorCopy( v, g_pParentWnd->ActiveXY()->RotateOrigin() );
	}

	CString strParam = GetParam( pBuffer );
	v[0] = atof( strParam );
	strParam = GetParam( pBuffer );
	v[1] = atof( strParam );
	strParam = GetParam( pBuffer );
	v[2] = atof( strParam );
	for ( int i = 0; i < 3; i++ )
		if ( v[i] != 0.0 ) {
			Select_RotateAxis( i, v[i], false, true );
		}
	Sys_UpdateWindows( W_ALL );
}

void MoveHold( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	brush_t* pBrush = NULL;
	int nHold = atoi( strParam );
	if ( nHold == 1 ) {
		pBrush = g_pHold1;
	}
	else if ( nHold == 2 ) {
		pBrush = g_pHold2;
	}
	else{
		pBrush = g_pHold3;
	}

	if ( pBrush ) {
		vec3_t v;
		strParam = GetParam( pBuffer );
		v[0] = atof( strParam );
		strParam = GetParam( pBuffer );
		v[1] = atof( strParam );
		strParam = GetParam( pBuffer );
		v[2] = atof( strParam );
		Brush_Move( pBrush, v, false );
	}
}

void RotateHold( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	brush_t* pBrush = NULL;
	int nHold = atoi( strParam );
	if ( nHold == 1 ) {
		pBrush = g_pHold1;
	}
	else if ( nHold == 2 ) {
		pBrush = g_pHold2;
	}
	else{
		pBrush = g_pHold3;
	}

	if ( pBrush ) {
		vec3_t v;
		strParam = GetParam( pBuffer );
		v[0] = atof( strParam );
		strParam = GetParam( pBuffer );
		v[1] = atof( strParam );
		strParam = GetParam( pBuffer );
		v[2] = atof( strParam );
		for ( int i = 0; i < 3; i++ )
			if ( v[i] != 0.0 ) {
				Select_RotateAxis( i, v[i] );
			}
	}
}

void CopyToMap( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	brush_t* pBrush = NULL;
	int nHold = atoi( strParam );
	if ( nHold == 1 ) {
		pBrush = g_pHold1;
	}
	else if ( nHold == 2 ) {
		pBrush = g_pHold2;
	}
	else{
		pBrush = g_pHold3;
	}

	if ( pBrush ) {
		Brush_AddToList( pBrush, &active_brushes );
		Entity_LinkBrush( world_entity, pBrush );
		Brush_Build( pBrush, false );

		Sys_UpdateWindows( W_ALL );
	}
}

void CopyAndSelect( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	brush_t* pBrush = NULL;
	int nHold = atoi( strParam );
	if ( nHold == 1 ) {
		pBrush = g_pHold1;
	}
	else if ( nHold == 2 ) {
		pBrush = g_pHold2;
	}
	else{
		pBrush = g_pHold3;
	}

	if ( pBrush ) {
		Select_Deselect();
		Brush_AddToList( pBrush, &active_brushes );
		Entity_LinkBrush( world_entity, pBrush );
		Brush_Build( pBrush, false );

		Select_Brush( pBrush );
		Sys_UpdateWindows( W_ALL );
	}
}

void Input( char*& pBuffer ){
	bool bGo = false;
	const char *fields[5] = { "", "", "", "", "" };
	float values[5];

	for ( int n = 0; n < 5 && n < g_nVariableCount; n++ )
	{
		if ( g_Variables[n].m_strInput.GetLength() > 0 ) {
			bGo = true;
			fields[n] = g_Variables[n].m_strInput.GetBuffer();
		}
	}

	if ( !bGo ) {
		return;
	}

	if ( DoBSInputDlg( fields, values ) != IDOK ) {
		g_bKeepGoing = false;
		return;
	}

	for ( int n = 0; n < 5 && n < g_nVariableCount; n++ )
	{
		if ( g_Variables[n].m_strInput.GetLength() > 0 ) {
			g_Variables[n].m_fValue = values[n];
		}
	}
}

bool g_bWaiting;
void _3DPointDone( bool b, int n ){
	g_bWaiting = false;
}

void _3DPointInput( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	CString strParam2 = GetParam( pBuffer );
	ShowInfoDialog( strParam2 );
	AddVectorVariable( strParam, strParam2 );
	g_bWaiting = true;
	AcquirePath( 2, &_3DPointDone );
	while ( g_bWaiting )
		gtk_main_iteration();
	HideInfoDialog();
	SetVectorVariableValue( strParam, g_PathPoints[0] );
}

void SetRotateOrigin( char*& pBuffer ){
	vec3_t v;
	CString strParam = GetParam( pBuffer );
	VectorVariableValue( strParam, v );
	VectorCopy( v, g_pParentWnd->ActiveXY()->RotateOrigin() );
	g_bRotateAroundSelection = false;
}

void InputVar( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	CString strParam2 = GetParam( pBuffer );
	AddVariable( strParam, 0.0, strParam2 );
}

void LoopCount( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	g_nLoopCounter = atoi( strParam );
	if ( g_nLoopCounter == 0 ) {
		g_nLoopCounter = (int)VariableValue( strParam );
	}
	if ( g_nLoopCounter > 0 ) {
		g_pLooper = pBuffer;
	}
}

void LoopRun( char*& pBuffer ){
	if ( g_bStartLoop == true ) {
		g_nLoopCounter--;
		if ( g_nLoopCounter == 0 ) {
			g_bStartLoop = false;
			GetParam( pBuffer );
		}
		else{
			pBuffer = g_pLooper;
		}
	}
	else
	{
		if ( g_pLooper && g_nLoopCounter > 0 ) {
			g_bStartLoop = true;
			pBuffer = g_pLooper;
		}
		else
		{
			GetParam( pBuffer );
		}
	}
}


void ConfirmMessage( char*& pBuffer ){
	CString strParam = GetParam( pBuffer );
	if ( gtk_MessageBox( g_pParentWnd->m_pWidget, strParam, _( "Script Info" ), MB_OKCANCEL ) == IDCANCEL ) {
		g_bKeepGoing = false;
	}
}

void Spherize( char*& pBuffer ){
	g_bScreenUpdates = false;
	for ( int n = 0; n < 120; n += 36 )
	{
		for ( int i = 0; i < 360; i += 36 )
		{
			Select_RotateAxis( 0, i, false, true );
			CSG_Subtract();
		}
		Select_RotateAxis( 2, n, false, true );
	}
	g_bScreenUpdates = true;
}

void RunIt( char*& pBuffer );
SBrushScript g_ScriptCmds[] =
{
	{"_CopySelected", &CopySelected},
	{"_MoveSelected", &MoveSelected},
	{"_RotateSelected", &RotateSelected},
	{"_MoveHold", &MoveHold},
	{"_RotateHold", &RotateHold},
	{"_CopyToMap", &CopyToMap},
	{"_CopyAndSelect", &CopyAndSelect},
	{"_Input", &Input},
	{"_3DPointInput", &_3DPointInput},
	{"_SetRotateOrigin", &SetRotateOrigin},
	{"_InputVar", &InputVar},
	{"_LoopCount", &LoopCount},
	{"_LoopRun", &LoopRun},
	{"_ConfirmMessage", &ConfirmMessage},
	{"_Spherize", &Spherize},
	{"_RunScript", RunIt}
};

const int g_nScriptCmdCount = sizeof( g_ScriptCmds ) / sizeof( SBrushScript );

void RunScript( char* pBuffer ){
	g_pHold1 = NULL;
	g_pHold2 = NULL;
	g_pHold3 = NULL;

	while ( g_bKeepGoing && pBuffer && *pBuffer )
	{
		while ( *pBuffer != (char)NULL && *pBuffer != '_' )
			pBuffer++;

		char* pTemp = pBuffer;
		int nLen = 0;
		while ( *pTemp != (char)NULL && *pTemp != '(' )
		{
			pTemp++;
			nLen++;
		}
		if ( *pBuffer != (char)NULL ) {
			bool bFound = false;
			for ( int i = 0; i < g_nScriptCmdCount; i++ )
			{
				//if (strnicmp(g_ScriptCmds[i].m_pName, pBuffer, strlen(g_ScriptCmds[i].m_pName)) ==  0)
				if ( strnicmp( g_ScriptCmds[i].m_pName, pBuffer, nLen ) ==  0 ) {
					pBuffer += strlen( g_ScriptCmds[i].m_pName );
					g_ScriptCmds[i].m_pProc( pBuffer );
					if ( g_bStartLoop ) {
					}
					bFound = true;
					break;
				}
			}
			if ( !bFound ) {
				pBuffer++;
			}
		}
	}
}


void RunScriptByName( char* pBuffer, bool bInit ){
	if ( bInit ) {
		InitForScriptRun();
	}
	char* pScript = new char[4096];
	CString strINI;
	strINI = g_strGameToolsPath;
	strINI += "/scripts.ini";
	CString strScript;
	FILE *f;

	f = fopen( strINI.GetBuffer(), "rt" );
	if ( f != NULL ) {
		char line[1024], *ptr;

		// read section names
		while ( fgets( line, 1024, f ) != 0 )
		{
			if ( line[0] != '[' ) {
				continue;
			}

			ptr = strchr( line, ']' );
			*ptr = '\0';

			if ( strcmp( line, pScript ) == 0 ) {
				while ( fgets( line, 1024, f ) != 0 )
				{
					if ( ( strchr( line, '=' ) == NULL ) ||
						 strlen( line ) == 0 ) {
						break;
					}
					strScript += line;
				}
				break;
			}
		}
		fclose( f );
	}
	RunScript( (char*)strScript.GetBuffer() );
}


void RunIt( char*& pBuffer ){
	brush_t* p1 = g_pHold1;
	brush_t* p2 = g_pHold2;
	brush_t* p3 = g_pHold3;

	CString strParam = GetParam( pBuffer );
	RunScriptByName( (char*)strParam.GetBuffer(), false );

	g_pHold3 = p3;
	g_pHold2 = p2;
	g_pHold1 = p1;
}
