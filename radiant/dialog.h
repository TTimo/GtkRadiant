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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <gtk/gtk.h>
#include "str.h"
#include "gtkmisc.h"

typedef enum
{
	DLG_CHECK_BOOL,
	DLG_RADIO_INT,
	DLG_ENTRY_TEXT,
	DLG_SPIN_FLOAT,
	DLG_SPIN_INT,
	DLG_ADJ_INT,
	DLG_COMBO_BOX_INT,
} DLG_DATA_TYPE;

class Dialog
{
public:
Dialog ();
virtual ~Dialog ();

/*!
   start modal dialog box
   you need to use AddModalButton to select IDOK IDCANCEL buttons
 */
int DoModal();
void EndModal( int code );
virtual void BuildDialog() = 0;
virtual void UpdateData( bool retrieve );
virtual void PreModal() { };
virtual void PostModal( int code ) { };
virtual void ShowDlg();
virtual void HideDlg();
void Create();
void Destroy();
GtkWidget* GetDlgWidget( const char* name ) { return GTK_WIDGET( g_object_get_data( G_OBJECT( m_pWidget ), name ) ); }
GtkWidget* GetWidget() { return m_pWidget; }
bool IsModal() { return m_nLoop; }

protected:
GtkWidget *m_pWidget;
bool m_nLoop;
int m_nReturn;

void AddDialogData( GtkWidget *widget, void *buf, DLG_DATA_TYPE type )
{ AddDialogData( G_OBJECT( widget ), buf, type ); };
void AddDialogData( GObject *object, void *buf, DLG_DATA_TYPE type );
/*!
   used in overloaded BuildDialog implementations to configure modal behaviour easily
 */
void AddModalButton( GtkWidget *widget, int ret );

private:
GSList* m_pDataList;
bool m_bNeedBuild;
};

#endif // _DIALOG_H_
