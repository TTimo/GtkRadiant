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

#ifndef _SURFACEDIALOG_H_
#define _SURFACEDIALOG_H_

#include "surfdlg_plugin.h"

void ShowDlg();
void HideDlg();
void SetTexMods();
void GetTexMods( bool b_SetUndoPoint = FALSE );
void BuildDialog();
void FitAll();
void InitDefaultIncrement( texdef_t * );
void DoSnapTToGrid( float hscale, float vscale );
// called to perform a fitting from the outside (shortcut key)
void SurfaceDialogFitAll();
void UpdateSurfaceDialog();
void DoSurface();
void ToggleSurface();
void SurfaceDlgFitAll();
GtkWidget *Get_SI_Module_Widget();

#endif // _SURFACEDIALOG_H_
