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

#ifndef _GROUPDIALOG_H_
#define _GROUPDIALOG_H_

#define DlgXBorder 5
#define DlgYBorder 5


enum
{
	EntList,
	EntComment,
// Spawnflags
	EntCheck1,
	EntCheck2,
	EntCheck3,
	EntCheck4,
	EntCheck5,
	EntCheck6,
	EntCheck7,
	EntCheck8,
// Extra Spawnflags for Halflife Support
	EntCheck9,
	EntCheck10,
	EntCheck11,
	EntCheck12,
	EntCheck13,
	EntCheck14,
	EntCheck15,
	EntCheck16,

	EntCheck17,
	EntCheck18,
	EntCheck19,
	EntCheck20,

	EntProps,
	EntDir0,
	EntDir45,
	EntDir90,
	EntDir135,
	EntDir180,
	EntDir225,
	EntDir270,
	EntDir315,
	EntDirUp,
	EntDirDown,
	EntDelProp,
	EntKeyLabel,
	EntKeyField,
	EntValueLabel,
	EntValueField,
	EntColor,
	EntAssignSounds,
	EntAssignModels,
	EntTab,

	EntLast,
};

extern GtkWidget* EntWidgets[EntLast];

//extern int rgIds[EntLast];


class GroupDlg
{
public:
  GroupDlg();
  void Create();

  void Show() { gtk_widget_show( m_pWidget ); }
  void Hide() { gtk_widget_hide( m_pWidget ); }

public:
  GtkWidget* m_pNotebook;
  GtkWidget* m_pWidget;
  GtkWidget* m_pTree;
};

extern GroupDlg *g_pGroupDlg;

#endif // _GROUPDIALOG_H_
