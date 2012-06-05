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
#ifndef _SURFACEPLUGIN_H
#define _SURFACEPLUGIN_H

int SI_GetSelectedFaceCountfromBrushes( void );
void SI_GetSelFacesTexdef( texdef_to_face_t *allocd_block_texdef );
void SI_SetTexdef_FaceList( texdef_to_face_t* texdef_face_list, bool b_SetUndoPoint = FALSE, bool bFit_to_Scale = FALSE );
void SI_FaceList_FitTexture( texdef_to_face_t* si_texdef_face_list, int nHeight, int nWidth );
GtkWindow* SI_GetMainWindow( void );
void SI_SetWinPos_from_Prefs( GtkWidget *win );

#endif // _SURFACEPLUGIN_H
