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

#include "dialog.h"

#ifdef _DEBUG
//#define DBG_SI 1
#endif

class SurfaceDlg : public Dialog {
  bool m_bPatchMode;
  // brush primitive fake shift scale rot coords
  float m_shift[2];
  float m_rotate;
  float m_scale[2];

public:
  SurfaceDlg();

  virtual void ShowDlg();
  virtual void HideDlg();
  void SetTexMods();
  void GetTexMods();

  void InitDefaultIncrement( texdef_t * );

  // Dialog Data
  int m_nHeight;
  int m_nWidth;

  // 0 is invalid, otherwise it's the Id of the last 'do' we are responsible for
  int m_nUndoId;

  // is the user editing the texture widget (that changes the behaviour of 'Enter' key from OnDone to OnApply
  // reset to false at each SetTexMods or when dealing with Enter key
  bool m_bEditingTextureWidget;

protected:
  void BuildDialog();

public:
  // called to perform a fitting from the outside (shortcut key)
  void FitAll();
  GtkWidget *GetWidget();
};

#endif // _SURFACEDIALOG_H_
