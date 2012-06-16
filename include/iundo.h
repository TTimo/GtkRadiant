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

#ifndef _IUNDO_H_
#define _IUNDO_H_

#define UNDO_MAJOR "undo"

//start operation
typedef void ( *PFN_UNDOSTART )( const char *operation );
//end operation
typedef void ( *PFN_UNDOEND )( void );
//add brush to the undo
typedef void ( *PFN_UNDOADDBRUSH )( brush_t *pBrush );
//end a brush after the operation is performed
typedef void ( *PFN_UNDOENDBRUSH )( brush_t *pBrush );
//add a list with brushes to the undo
typedef void ( *PFN_UNDOADDBRUSHLIST )( brush_t *brushlist );
//end a list with brushes after the operation is performed
typedef void ( *PFN_UNDOENDBRUSHLIST )( brush_t *brushlist );
//add entity to undo
typedef void ( *PFN_UNDOADDENTITY )( entity_t *entity );
//end an entity after the operation is performed
typedef void ( *PFN_UNDOENDENTITY )( entity_t *entity );
//undo last operation (bSilent == true -> will not print the "undone blah blah message")
typedef void ( *PFN_UNDO )( qboolean bSilent );
//redo last undone operation
typedef void ( *PFN_REDO )( void );
//get the undo Id of the next undo (0 if none available)
typedef int ( *PFN_GETUNDOID )( void );
//returns true if there is something to be undone available
typedef int ( *PFN_UNDOAVAILABLE )( void );
//returns true if there is something to redo available
typedef int ( *PFN_REDOAVAILABLE )( void );

struct _QERUndoTable
{
	int m_nSize;
	PFN_UNDOSTART m_pfnUndo_Start;
	PFN_UNDOEND m_pfnUndo_End;
	PFN_UNDOADDBRUSH m_pfnUndo_AddBrush;
	PFN_UNDOENDBRUSH m_pfnUndo_EndBrush;
	PFN_UNDOADDBRUSHLIST m_pfnUndo_AddBrushList;
	PFN_UNDOENDBRUSHLIST m_pfnUndo_EndBrushList;
	PFN_UNDOADDENTITY m_pfnUndo_AddEntity;
	PFN_UNDOENDENTITY m_pfnUndo_EndEntity;
	PFN_UNDO m_pfnUndo_Undo;
	PFN_REDO m_pfnUndo_Redo;
	PFN_GETUNDOID m_pfnUndo_GetUndoId;
	PFN_UNDOAVAILABLE m_pfnUndo_UndoAvailable;
	PFN_REDOAVAILABLE m_pfnUndo_RedoAvailable;
};

#ifdef USE_UNDOTABLE_DEFINE
#ifndef __UNDOTABLENAME
#define __UNDOTABLENAME g_UndoTable
#endif
#define Undo_Start __UNDOTABLENAME.m_pfnUndo_Start
#define Undo_End __UNDOTABLENAME.m_pfnUndo_End
#define Undo_AddBrush __UNDOTABLENAME.m_pfnUndo_AddBrush
#define Undo_EndBrush __UNDOTABLENAME.m_pfnUndo_EndBrush
#define Undo_AddBrushList __UNDOTABLENAME.m_pfnUndo_AddBrushList
#define Undo_EndBrushList __UNDOTABLENAME.m_pfnUndo_EndBrushList
#define Undo_AddEntity __UNDOTABLENAME.m_pfnUndo_AddEntity
#define Undo_EndEntity __UNDOTABLENAME.m_pfnUndo_EndEntity
#endif

#endif // _IUNDO_H_
