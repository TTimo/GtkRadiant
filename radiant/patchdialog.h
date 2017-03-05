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

#ifndef _PATCHDIALOG_H_
#define _PATCHDIALOG_H_

#include "dialog.h"

class PatchDialog : public Dialog
{
public:
// overrides from Dialog
void HideDlg();
void ShowDlg();

//  void UpdateInfo();
//  void SetPatchInfo();
void GetPatchInfo();
void UpdateSpinners( bool bUp, int nID );
// read the current patch on map and initialize m_fX m_fY accordingly
void UpdateRowColInfo();
// sync the dialog our internal data structures
// depending on the flag it will read or write
// we use m_nCol m_nRow m_fX m_fY m_fZ m_fS m_fT m_strName
// (NOTE: this doesn't actually commit stuff to the map or read from it)
void UpdateData( bool retrieve );

void InitDefaultIncrement( texdef_t * );

PatchDialog();
patchMesh_t *m_Patch;

Str m_strName;
float m_fS;
float m_fT;
float m_fX;
float m_fY;
float m_fZ;
/*  float	m_fHScale;
   float	m_fHShift;
   float	m_fRotate;
   float	m_fVScale;
   float	m_fVShift; */
int m_nCol;
int m_nRow;
GtkWidget *m_pRowCombo;
GtkWidget *m_pColCombo;

GtkWidget *GetWidget() { return m_pWidget; }

// 0 is invalid, otherwise it's the Id of the last 'do' we are responsible for
//  int m_nUndoId;

// turn on/off processing of the "changed" "value-changed" messages
// (need to turn off when we are feeding data in)
// NOTE: much more simple than blocking signals
bool m_bListenChanged;

protected:
void BuildDialog();
};

#endif // _PATCHDIALOG_H_
