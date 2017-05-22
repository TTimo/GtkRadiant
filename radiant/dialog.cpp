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

//
// Base dialog class, provides a way to run modal dialogs and
// set/get the widget values in member variables.
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	GObject     *object;
	void          *buffer;
	DLG_DATA_TYPE type;
} DLG_DATA;

// =============================================================================
// Dialog class

Dialog::Dialog (){
	m_pDataList = (GSList*)NULL;
	m_nReturn = IDCANCEL;
	m_bNeedBuild = true;
	m_nLoop = false;
}

Dialog::~Dialog (){
	while ( m_pDataList )
	{
		free( m_pDataList->data );
		m_pDataList = g_slist_remove( m_pDataList, m_pDataList->data );
	}

	if ( m_pWidget != NULL ) {
		gtk_widget_destroy( m_pWidget );
	}
}

// i suspect that this is redundant - gtk manages to remember the data stored in its widgets across a hide/show
void Dialog::ShowDlg(){
	Create();
	UpdateData( FALSE );
	gtk_widget_show( m_pWidget );
}

void Dialog::HideDlg(){
	UpdateData( TRUE );
	gtk_widget_hide( m_pWidget );
}

static gint delete_event_callback( GtkWidget *widget, GdkEvent* event, gpointer data ){
	reinterpret_cast<Dialog*>( data )->HideDlg();
	reinterpret_cast<Dialog*>( data )->EndModal( IDCANCEL );
	return TRUE;
}

void Dialog::Create(){
	if ( m_bNeedBuild ) {
		m_pWidget = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		g_signal_connect( G_OBJECT( m_pWidget ), "delete-event",
							G_CALLBACK( delete_event_callback ), this );
		g_signal_connect( G_OBJECT( m_pWidget ), "destroy",
							G_CALLBACK( gtk_widget_destroy ), NULL );
		g_object_set_data( G_OBJECT( m_pWidget ), "loop", &m_nLoop );
		g_object_set_data( G_OBJECT( m_pWidget ), "ret", &m_nReturn );

		BuildDialog();
		m_bNeedBuild = false;
	}
}

void Dialog::Destroy(){
	if ( m_pWidget != NULL ) {
		gtk_widget_destroy( m_pWidget );
		m_pWidget = NULL;
	}
}

void Dialog::AddDialogData( GObject *object, void *buf, DLG_DATA_TYPE type ){
	DLG_DATA *data;

	data = (DLG_DATA*)qmalloc( sizeof( DLG_DATA ) );
	data->object = object;
	data->buffer = buf;
	data->type = type;

	m_pDataList = g_slist_append( m_pDataList, data );
}

void Dialog::AddModalButton( GtkWidget *widget, int ret ) {
	g_signal_connect( G_OBJECT( widget ), "clicked", 
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( ret ) );
}

void Dialog::UpdateData( bool retrieve ){
	DLG_DATA *data;
	GSList *lst;

	if ( retrieve ) {
		for ( lst = m_pDataList; lst != NULL; lst = g_slist_next( lst ) )
		{
			data = (DLG_DATA*)lst->data;

			switch ( data->type )
			{
			case DLG_CHECK_BOOL:
				*(bool*)data->buffer = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( data->object ) );
				break;
			case DLG_RADIO_INT:
			{
				GSList *radio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( data->object ) );
				*(int*)data->buffer = g_slist_length( radio ) - 1;
				for (; radio; radio = g_slist_next( radio ) )
					if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio->data ) ) ) {
						break;
					}
					else{
						( *(int*)data->buffer )--;
					}
			} break;
			case DLG_ENTRY_TEXT:
			{
				const char *txt;
				Str* str;
				str = (Str*)data->buffer;
				txt = gtk_entry_get_text( GTK_ENTRY( data->object ) );
				*str = txt;
			} break;
			case DLG_SPIN_FLOAT:
				*(float*)data->buffer = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data->object ) );
				break;
			case DLG_SPIN_INT:
				*(int*)data->buffer = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( data->object ) );
				break;
			case DLG_ADJ_INT:
				*(int*)data->buffer = (int) gtk_adjustment_get_value( GTK_ADJUSTMENT( data->object ) );
				break;
			case DLG_COMBO_BOX_INT: {
				*(int*)data->buffer = gtk_combo_box_get_active( GTK_COMBO_BOX( data->object ) );
			}
			break;

			}
		}
	}
	else
	{
		for ( lst = m_pDataList; lst != NULL; lst = g_slist_next( lst ) )
		{
			data = (DLG_DATA*)lst->data;

			switch ( data->type )
			{
			case DLG_CHECK_BOOL:
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( data->object ), *(bool*)data->buffer );
				break;
			case DLG_RADIO_INT:
			{
				GSList *radio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( data->object ) );
				gpointer btn =  g_slist_nth_data( radio, g_slist_length( radio ) - ( *(int*)data->buffer ) - 1 );
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( btn ), TRUE );
			} break;
			case DLG_ENTRY_TEXT:
			{
				Str* str;
				str = (Str*)data->buffer;
				const char *txt = str->GetBuffer();
				gtk_entry_set_text( GTK_ENTRY( data->object ), txt );
			} break;
			case DLG_SPIN_FLOAT:
				gtk_spin_button_set_value( GTK_SPIN_BUTTON( data->object ), ( *(float*)data->buffer ) );
				break;
			case DLG_SPIN_INT:
				gtk_spin_button_set_value( GTK_SPIN_BUTTON( data->object ), ( *(int*)data->buffer ) );
				break;
			case DLG_ADJ_INT:
				gtk_adjustment_set_value( GTK_ADJUSTMENT( data->object ), ( *(int*)data->buffer ) );
				break;
			case DLG_COMBO_BOX_INT: {
				gtk_combo_box_set_active( GTK_COMBO_BOX( data->object ), *(int*)data->buffer );
			}
			break;
			}
		}
	}
}

void Dialog::EndModal( int code ) {
	m_nLoop = false;
	m_nReturn = code;
}

int Dialog::DoModal(){
	Create();
	UpdateData( FALSE );

	PreModal();

	gtk_grab_add( m_pWidget );
	gtk_widget_show( m_pWidget );

	m_nLoop = true;
	while ( m_nLoop ) {
		gtk_main_iteration();
	}

	if ( m_pWidget != NULL ) {
		UpdateData( TRUE );

		gtk_grab_remove( m_pWidget );
		gtk_widget_hide( m_pWidget );
	}
	PostModal( m_nReturn );

	return m_nReturn;
}
