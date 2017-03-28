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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include "surfdlg_plugin.h"
#include "surfacedialog.h"
#include "surfaceflags.h"

GtkWidget *notebook1;

// 32 bit is the max
#define MAX_BUTTONS 32

#ifdef _WIN32
// TTimo: THIS IS UGLY
#define snprintf _snprintf
#endif

GtkWidget *surface_buttons[MAX_BUTTONS];
GtkWidget *content_buttons[MAX_BUTTONS];

GtkWidget *value_entry;
gboolean setup_buttons = TRUE;

int working_surface_flags;
int surface_mask;
int working_content_flags;
int content_mask;
int working_value;

inline void set_inconsistent( GtkWidget *toggle_button ){
	gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON( toggle_button ), TRUE );
}

inline void clear_inconsistent( GtkWidget *toggle_button ){
	if ( gtk_toggle_button_get_inconsistent( GTK_TOGGLE_BUTTON( toggle_button ) ) ) {
		gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON( toggle_button ), FALSE );
	}
}

void clear_all_inconsistent( void ){
	int i;

	for ( i = 0; i < MAX_BUTTONS; i++ ) {
		clear_inconsistent( surface_buttons[i] );
		clear_inconsistent( content_buttons[i] );
	}
}

void clear_all_buttons_and_values(){
	int i;

	for ( i = 0; i < MAX_BUTTONS; i++ ) {
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( surface_buttons[i] ), FALSE );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( content_buttons[i] ), FALSE );
	}

	gtk_entry_set_text( (GtkEntry *)value_entry, "" );
}

void set_surface_flags_button_state( texdef_to_face_t *texdef_face_list, bool b_isListEmpty ){
	int contents = 0;
	int flags = 0;
	int value = 0;
	int diff_contents = 0;
	int diff_flags = 0;
	gboolean diff_value = FALSE;
	char tex_buff[11];
	texdef_t* tmp_texdef;
	texdef_to_face_t* temp_texdef_face_list;
	int i;

	setup_buttons = TRUE;
	working_surface_flags = 0;
	surface_mask = 0;
	working_content_flags = 0;
	content_mask = 0;
	working_value = 0;

	if ( !b_isListEmpty ) {
		tmp_texdef = &texdef_face_list->texdef;
		contents = tmp_texdef->contents;
		flags = tmp_texdef->flags;
		value = tmp_texdef->value;

#if _DEBUG
		Sys_Printf( "Surface: %d\tContents: %d\tValue: %d\ttmp_texdef\n",tmp_texdef->flags,tmp_texdef->contents,tmp_texdef->value );
		Sys_Printf( "Surface: %d\tContents: %d\tValue: %d\n",flags,contents,value );
#endif

		for ( temp_texdef_face_list = texdef_face_list->next; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = &temp_texdef_face_list->texdef;
			diff_contents |= contents ^ tmp_texdef->contents;  // Figure out which buttons are inconsistent
			diff_flags |= flags ^ tmp_texdef->flags;
			if ( tmp_texdef->value != value ) {
				diff_value = TRUE;
			}

#if _DEBUG
			Sys_Printf( "Surface: %d\tContents: %d\tValue: %d\ttmp_texdef\n",tmp_texdef->flags,tmp_texdef->contents,tmp_texdef->value );
			Sys_Printf( "Surface: %d\tContents: %d\tValue: %d\n",flags,contents,value );
#endif
		}
	}

	clear_all_inconsistent();

	// If no faces/brushes are selected, clear everything and bail
	if ( b_isListEmpty ) {
		clear_all_buttons_and_values();
		setup_buttons = FALSE;
		return;
	}

	for ( i = 0; i < MAX_BUTTONS; i++ ) {
		// Set surface buttons to reflect brush/face flags, contents, and values
		if ( diff_flags & ( 1 << i ) ) {
			set_inconsistent( surface_buttons[i] );
		}
		else if ( flags & ( 1 << i ) ) {
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( surface_buttons[i] ), TRUE );
		}
		else{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( surface_buttons[i] ), FALSE );
		}

		if ( diff_contents & ( 1 << i ) ) {
			set_inconsistent( content_buttons[i] );
		}
		else if ( contents & ( 1 << i ) ) {
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( content_buttons[i] ), TRUE );
		}
		else{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( content_buttons[i] ), FALSE );
		}
	}

	// Set Value
	if ( diff_value ) {
		gtk_entry_set_text( (GtkEntry *)value_entry, "" );
	}
	else {
		working_value = value;
		sprintf( tex_buff, "%d", value );
		gtk_entry_set_text( (GtkEntry *)value_entry, tex_buff );
	}

	setup_buttons = FALSE;
}

void apply_surface_flags( texdef_to_face_t *faces ){
	texdef_to_face_t *face;
	texdef_t *tex;

	for ( face = faces; face; face = face->next )
	{
		tex = &face->texdef;
		tex->flags = ( tex->flags & ~surface_mask ) | working_surface_flags;
		tex->contents = ( tex->contents & ~content_mask ) | working_content_flags;
		tex->value = working_value;

#ifdef _DEBUG
		Sys_Printf( "Surface: %d\tContents: %d\tValue: %d\n", tex->flags, tex->contents, tex->value );
#endif
	}
}

inline void change_surfaceflag( GtkWidget *togglebutton, int sur_flag, gboolean change_flag_to ){
	if ( !setup_buttons ) { // If we're setting up the buttons, we really don't need to
		                   // set flags that are already set
		if ( gtk_toggle_button_get_inconsistent( GTK_TOGGLE_BUTTON( togglebutton ) ) ) { // Clear out inconsistent, if set
			clear_inconsistent( GTK_WIDGET( togglebutton ) );
		}

		surface_mask |= sur_flag;

		if ( change_flag_to ) {
			working_surface_flags |= sur_flag;
		}
		else{
			working_surface_flags &= ~sur_flag;
		}

		GetTexMods( false );
	}
}

inline void change_contentflag( GtkWidget *togglebutton, int content_flag, gboolean change_flag_to ){
	if ( ( !setup_buttons ) ) { // If we're setting up the buttons, we really don't need to
		                       // set flags that are already set
		if ( gtk_toggle_button_get_inconsistent( GTK_TOGGLE_BUTTON( togglebutton ) ) ) {
			clear_inconsistent( togglebutton );
		}

		content_mask |= content_flag;

		if ( change_flag_to ) {
			working_content_flags |= content_flag;
		}
		else{
			working_content_flags &= ~content_flag;
		}

		GetTexMods( false );
	}
}

// Surface Flags Callbacks
void on_surface_button_toggled( GtkToggleButton *togglebutton, gpointer user_data ){
	int flag = GPOINTER_TO_INT( user_data );
	change_surfaceflag( GTK_WIDGET( togglebutton ), flag, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( togglebutton ) ) );
}

// Content Flags Callbacks
void on_content_button_toggled( GtkToggleButton *togglebutton, gpointer user_data ){
	int flag = GPOINTER_TO_INT( user_data );
	change_contentflag( GTK_WIDGET( togglebutton ), flag, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( togglebutton ) ) );
}

// Value Entry Callback
void on_value_entry_changed( GtkEditable *editable, gpointer user_data ){
	if ( ( !setup_buttons ) ) { // If we're setting up the buttons, don't change value
		working_value = atoi( gtk_entry_get_text( GTK_ENTRY( editable ) ) );
		GetTexMods( false );
	}
}

void on_value_entry_insert_text( GtkEditable *editable, gchar *new_text, gint new_text_length, gint *position, gpointer user_data ){
	int i, count = 0;
	gchar *result;

	// Limit input to digits, throwing out anything else
	// Modified from Gtk FAQ for text filtering of GtkEntry
	result = g_new( gchar, new_text_length );

	for ( i = 0; i < new_text_length; i++ ) {
		if ( !isdigit( new_text[i] ) ) {
			continue;
		}
		result[count++] = new_text[i];
	}

	if ( count > 0 ) {
		g_signal_handlers_block_by_func( G_OBJECT( editable ),
										  (gpointer)G_CALLBACK( on_value_entry_insert_text ),
										  user_data );
		gtk_editable_insert_text( editable, result, count, position );
		g_signal_handlers_unblock_by_func( G_OBJECT( editable ),
											(gpointer)G_CALLBACK( on_value_entry_insert_text ),
											user_data );
	}
	g_signal_stop_emission_by_name( G_OBJECT( editable ), "insert-text" );

	g_free( result );
}

void on_surfacebutton_clicked( GtkButton *button, gpointer user_data ){
	gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook1 ), 0 );
}

void on_contentbutton_clicked( GtkButton *button, gpointer user_data ){
	gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook1 ), 1 );
}

#define IDTECH2_FLAG_BUTTON_BORDER 3

GtkWidget* create_SurfaceFlagsFrame( GtkWidget* surfacedialog_widget ){
	GtkWidget *frame1;
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *vbox4;
	GtkWidget *table4;
	GtkWidget *hbox2;
	GtkWidget *hbox3;
	GtkWidget *value_label;
	GtkWidget *label5;
	GtkWidget *table3;
	GtkWidget *label6;
	int i, x, y;
	const char *buttonLabel;
	char buffer[8];

	frame1 = gtk_frame_new( _( "Flags" ) );
	gtk_container_add( GTK_CONTAINER( surfacedialog_widget ), frame1 );
	gtk_widget_show( frame1 );

	vbox1 = gtk_vbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( frame1 ), vbox1 );
	gtk_widget_show( vbox1 );

	notebook1 = gtk_notebook_new();
	gtk_box_pack_start( GTK_BOX( vbox1 ), notebook1, TRUE, TRUE, 0 );
	gtk_notebook_set_show_tabs( GTK_NOTEBOOK( notebook1 ), TRUE );
	gtk_container_set_border_width( GTK_CONTAINER( notebook1 ), 5 );
	gtk_widget_show( notebook1 );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( notebook1 ), vbox2 );
	gtk_widget_show( vbox2 );

	table4 = gtk_table_new( 8, 4, TRUE );
	gtk_table_set_col_spacings( GTK_TABLE( table4 ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table4 ), 5 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table4, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( table4 ), 5 );
	gtk_widget_show( table4 );

	y = -1;
	for ( i = 0; i < MAX_BUTTONS; i++ ) {
		if ( !( i % 4 ) ) {
			y++;
		}
		x = i % 4;
		snprintf( buffer, sizeof( buffer ) - 1, "surf%i", i + 1 );
		buttonLabel = g_FuncTable.m_pfnReadProjectKey( buffer );
		//Sys_Printf( "%s: %s\n", buffer, buttonLabel );
		surface_buttons[i] = gtk_toggle_button_new_with_label( buttonLabel );
		g_signal_connect( G_OBJECT( surface_buttons[i] ), "toggled", G_CALLBACK( on_surface_button_toggled ), GINT_TO_POINTER( 1 << i ) );
		gtk_table_attach( GTK_TABLE( table4 ), surface_buttons[i], 0 + x, 1 + x, ( 0 + y ), ( 1 + y ),
						  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
						  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ), 0, 0 );
		gtk_widget_show( surface_buttons[i] );
	}

	hbox2 = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox2, FALSE, FALSE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox2 ), 4 );
	gtk_widget_show( hbox2 );

	hbox3 = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox2 ), hbox3, TRUE, TRUE, 0 );
	gtk_widget_show( hbox3 );

	vbox4 = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox3 ), vbox4, TRUE, TRUE, 0 );
	gtk_widget_show( vbox4 );

	value_label = gtk_label_new( _( "Value: " ) );
	gtk_box_pack_start( GTK_BOX( hbox3 ), value_label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( value_label ), 0.0, 0.5 );
	gtk_widget_show( value_label );

	value_entry = gtk_entry_new();
	g_signal_connect( G_OBJECT( value_entry ), "changed",
						G_CALLBACK( on_value_entry_changed ),
						NULL );
	g_signal_connect( G_OBJECT( value_entry ), "insert-text",
						G_CALLBACK( on_value_entry_insert_text ),
						NULL );
	gtk_entry_set_max_length( (GtkEntry *)value_entry, 11 );
	gtk_box_pack_start( GTK_BOX( hbox3 ), value_entry, TRUE, TRUE, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( value_entry ), 1.0 ); //right
	gtk_widget_show( value_entry );

	vbox3 = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox3 ), vbox3, TRUE, TRUE, 0 );
	gtk_widget_show( vbox3 );

	label5 = gtk_label_new( _( "Surface Flags" ) );
	gtk_notebook_set_tab_label( GTK_NOTEBOOK( notebook1 ), gtk_notebook_get_nth_page( GTK_NOTEBOOK( notebook1 ), 0 ), label5 );
	gtk_widget_show( label5 );

	table3 = gtk_table_new( 8, 4, TRUE );
	gtk_table_set_col_spacings( GTK_TABLE( table3 ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table3 ), 5 );
	gtk_container_add( GTK_CONTAINER( notebook1 ), table3 );
	gtk_container_set_border_width( GTK_CONTAINER( table3 ), 5 );
	gtk_widget_show( table3 );

	y = -1;
	for ( i = 0; i < MAX_BUTTONS; i++ ) {
		if ( !( i % 4 ) ) {
			y++;
		}
		x = i % 4;
		snprintf( buffer, sizeof( buffer ) - 1, "cont%i", i + 1 );
		buttonLabel = g_FuncTable.m_pfnReadProjectKey( buffer );
		content_buttons[i] = gtk_toggle_button_new_with_label( buttonLabel );
		g_signal_connect( G_OBJECT( content_buttons[i] ), "toggled", G_CALLBACK( on_content_button_toggled ), GINT_TO_POINTER( 1 << i ) );
		gtk_table_attach( GTK_TABLE( table3 ), content_buttons[i], 0 + x, 1 + x, ( 0 + y ), ( 1 + y ),
						  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
						  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ), 0, 0 );
		gtk_widget_show( content_buttons[i] );
	}

	label6 = gtk_label_new( _( "Content Flags" ) );
	gtk_widget_show( label6 );
	gtk_notebook_set_tab_label( GTK_NOTEBOOK( notebook1 ), gtk_notebook_get_nth_page( GTK_NOTEBOOK( notebook1 ), 1 ), label6 );

	return frame1;
}
