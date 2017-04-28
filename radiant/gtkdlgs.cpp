/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Some small dialogs that don't need much
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif

#ifdef _WIN32
#include <shellapi.h>
#endif

static GtkWidget *EntityList_dialog = NULL;

// =============================================================================
// Color selection dialog

qboolean DoColor( int iIndex ){
	static bool bColorOpen = false;

	if ( bColorOpen ) {
		Sys_FPrintf( SYS_WRN, "DoColor dialog is already open\n" );
		return false;
	}

	bColorOpen = true;

	if ( color_dialog( g_pParentWnd->m_pWidget, g_qeglobals.d_savedinfo.colors[iIndex] ) ) {
		/*
		** scale colors so that at least one component is at 1.0F
		** if this is meant to select an entity color
		*/
		if ( iIndex == COLOR_ENTITY ) {
			float largest = 0.0F;

			if ( g_qeglobals.d_savedinfo.colors[iIndex][0] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][0];
			}
			if ( g_qeglobals.d_savedinfo.colors[iIndex][1] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][1];
			}
			if ( g_qeglobals.d_savedinfo.colors[iIndex][2] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][2];
			}

			if ( largest == 0.0F ) {
				g_qeglobals.d_savedinfo.colors[iIndex][0] = 1.0F;
				g_qeglobals.d_savedinfo.colors[iIndex][1] = 1.0F;
				g_qeglobals.d_savedinfo.colors[iIndex][2] = 1.0F;
			}
			else
			{
				float scaler = 1.0F / largest;

				g_qeglobals.d_savedinfo.colors[iIndex][0] *= scaler;
				g_qeglobals.d_savedinfo.colors[iIndex][1] *= scaler;
				g_qeglobals.d_savedinfo.colors[iIndex][2] *= scaler;
			}
		}

		Sys_UpdateWindows( W_ALL );
		bColorOpen = false;
		return true;
	}
	else {
		bColorOpen = false;
		return false;
	}
}

// =============================================================================
// Project settings dialog

static void UpdateBSPCommandList( GtkWidget *dialog );

static void DoProjectAddEdit( bool edit, GtkWidget *parent ){
	GtkWidget *dialog, *vbox, *hbox, *label, *table;
	GtkWidget *cmd, *text, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( edit ) {
		dialog = gtk_dialog_new_with_buttons( _( "Edit Command" ), NULL, flags, NULL, NULL );
	}
	else{
		dialog = gtk_dialog_new_with_buttons( _( "Add Command" ), NULL, flags, NULL, NULL );
	}
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( parent ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), table, FALSE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Menu text" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Command" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	text = gtk_entry_new();
	g_object_set_data( G_OBJECT( dialog ), "text", text );
	gtk_widget_show( text );
	g_object_set_data( G_OBJECT( dialog ), "text", text );

	cmd = gtk_entry_new();
	gtk_table_attach( GTK_TABLE( table ), cmd, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( cmd );
	g_object_set_data( G_OBJECT( dialog ), "cmd", cmd );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	if ( edit ) {
		GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( parent ), "view" ) );
		GtkTreeSelection* selection = gtk_tree_view_get_selection( view );
		GtkTreeIter iter;
		GtkTreeModel* model;
		if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
			char* key;
			gtk_tree_model_get( model, &iter, 0, &key, -1 );
			const char* value = ValueForKey( g_qeglobals.d_project_entity, key );
			gtk_entry_set_text( GTK_ENTRY( text ), key );
			gtk_entry_set_text( GTK_ENTRY( cmd ), value );
			g_free( key );
		}
	}


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if ( response_id == GTK_RESPONSE_OK ) {
		const char* key = gtk_entry_get_text( GTK_ENTRY( text ) );
		const char* value = gtk_entry_get_text( GTK_ENTRY( cmd ) );

		if ( strlen( key ) <= 0 || strlen( value ) <= 0 ) {
			Sys_Printf( "Command not added\n" );
		}
		else
		{
			if ( edit ) {
				SetKeyValue( g_qeglobals.d_project_entity, key, value );
				FillBSPMenu();
			}
			else
			{
				if ( key[0] == 'b' && key[1] == 's' && key[2] == 'p' ) {
					SetKeyValue( g_qeglobals.d_project_entity, key, value );
					FillBSPMenu();
				}
				else{
					Sys_Printf( "BSP commands must be preceded by \"bsp\"" );
				}
			}

			UpdateBSPCommandList( parent );
		}
	}

	gtk_widget_destroy( dialog );
}

static void UpdateBSPCommandList( GtkWidget *dialog ){
	GtkListStore* store = GTK_LIST_STORE( g_object_get_data( G_OBJECT( dialog ), "bsp_commands" ) );

	gtk_list_store_clear( store );

	for ( epair_t* ep = g_qeglobals.d_project_entity->epairs; ep != NULL; ep = ep->next )
	{
		if ( ep->key[0] == 'b' && ep->key[1] == 's' && ep->key[2] == 'p' ) {
			GtkTreeIter iter;
			gtk_list_store_append( store, &iter );
			gtk_list_store_set( store, &iter, 0, ep->key, -1 );
		}
	}
}

static void project_add( GtkWidget *widget, gpointer data ){
	GtkWidget *dlg = GTK_WIDGET( data );
	DoProjectAddEdit( false, dlg );
	UpdateBSPCommandList( dlg );
}

static void project_change( GtkWidget *widget, gpointer data ){
	GtkWidget *dlg = GTK_WIDGET( data );
	DoProjectAddEdit( true, dlg );
	UpdateBSPCommandList( dlg );
}

static void project_remove( GtkWidget *widget, gpointer data ){
	GtkWidget* project = GTK_WIDGET( data );

	GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( project ), "view" ) );
	GtkTreeSelection* selection = gtk_tree_view_get_selection( view );
	GtkTreeIter iter;
	GtkTreeModel* model;
	if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
		char* key;
		gtk_tree_model_get( model, &iter, 0, &key, -1 );
		DeleteKey( g_qeglobals.d_project_entity, key );
		g_free( key );

		char* index = gtk_tree_model_get_string_from_iter( model, &iter );
		Sys_Printf( "Selected %s\n", index );
		g_free( index );

		UpdateBSPCommandList( project );
		FillBSPMenu();
	}
}

static const char* sQ3ComboItem = "Quake III Arena";
static const char* sTAComboItem = "Quake III: Team Arena";
static const char* sModComboItem = "Custom Quake III modification";
static const char* sWolfComboItem = "Return To Castle Wolfenstein";
static const char* sWolfModComboItem = "Custom RTCW modification";
static const char* sHLComboItem = "Half-life";
static const char* sHLModComboItem = "Custom Half-life modification";

static const char* sWolfSPCombo = "Single Player mapping mode";
static const char* sWolfMPCombo = "Multiplayer mapping mode";

// Arnout
// HARD-CODED ET HACK
static const char* sETComboItem = "Wolfenstein: Enemy Territory";
static const char* sETModComboItem = "Custom ET modification";

// RIANT
// HARD-CODED JK2 HACK
static const char* sJK2ComboItem = "Jedi Knight II Outcast";
static const char* sJK2ModComboItem = "Custom JK2 modification";
static const char* sJK2SPCombo = "Single Player mapping mode";
static const char* sJK2MPCombo = "Multiplayer mapping mode";

// TTimo
// HARD-CODED JA HACK
static const char* sJAComboItem = "Jedi Knight Jedi Academy";
static const char* sJAModComboItem = "Custom JA modification";
static const char* sJASPCombo = "Single Player mapping mode";
static const char* sJAMPCombo = "Multiplayer mapping mode";

// RIANT
// HARD-CODED STVEF2 HACK
static const char* sSTVEFComboItem = "Star Trek Voyager : Elite Force";
static const char* sSTVEFModComboItem = "Custom Elite Force modification";
static const char* sSTVEFSPCombo = "Single Player mapping mode";
static const char* sSTVEFMPCombo = "Holo Match mapping mode";

// RIANT
// HARD-CODED SOF2 HACK
static const char* sSOF2ComboItem = "Soldier of Fortune II - Double Helix";
static const char* sSOF2ModComboItem = "Custom Sof2 modification";
static const char* sSOF2SPCombo = "Single Player mapping mode";
static const char* sSOF2MPCombo = "Multiplayer mapping mode";

struct gamemode_s {
	const char *gameFile;
	const char *name;
	const char *mode;
	qboolean base; //default mode
};
typedef struct gamemode_s gamemode_t;

gamemode_t gameModeList[] = {
	{ "wolf.game", sWolfSPCombo, "sp", qtrue },
	{ "wolf.game", sWolfMPCombo, "mp", qfalse },

	{ "jk2.game", sJK2SPCombo, "sp", qtrue },
	{ "jk2.game", sJK2MPCombo, "mp", qfalse },

	{ "ja.game", sJASPCombo, "sp", qtrue },
	{ "ja.game", sJAMPCombo, "mp", qfalse },

	{ "stvef.game", sSTVEFSPCombo, "sp", qtrue },
	{ "stvef.game", sSTVEFMPCombo, "mp", qfalse },

	{ "sof2.game", sSOF2SPCombo, "sp", qtrue },
	{ "sof2.game", sSOF2MPCombo, "mp", qfalse },

};

struct game_s {
	const char *gameFile;
	const char *name;
	const char *fs_game; //filesystem gamename
	qboolean base; //default basegame, ie baseq3
	qboolean custom; //ie Custom Quake III modification
};
typedef struct game_s game_t;

game_t gameList[] = {
	{ "q3.game", sQ3ComboItem, "baseq3", qtrue, qfalse },
	{ "q3.game", sTAComboItem, "missionpack", qfalse, qfalse },
	{ "q3.game", "Defrag", "defrag", qfalse, qfalse },
	{ "q3.game", sModComboItem, "", qfalse, qtrue },

	{ "wolf.game", sWolfComboItem, "main", qtrue, qfalse },
	{ "wolf.game", sWolfModComboItem, "", qfalse, qfalse },

	{ "hl.game", sHLComboItem, "valve", qtrue, qfalse },
	{ "hl.game", sHLModComboItem, "", qfalse, qtrue },
	
	{ "et.game", sETComboItem, "etmain", qtrue, qfalse },
	{ "et.game", sETModComboItem, "", qfalse, qtrue },

	{ "jk2.game", sJK2ComboItem, "base", qtrue, qfalse },
	{ "jk2.game", sJK2ModComboItem, "", qfalse, qtrue },

	{ "ja.game", sJAComboItem, "base", qtrue, qfalse },
	{ "ja.game", sJAModComboItem, "", qfalse, qtrue },

	{ "stvef.game", sSTVEFComboItem, "baseEf", qtrue, qfalse },
	{ "stvef.game", sSTVEFModComboItem, "", qfalse, qtrue },

	{ "sof2.game", sSOF2ComboItem, "base", qtrue, qfalse },
	{ "sof2.game", sSOF2ModComboItem, "", qfalse, qtrue },

	{ "q2.game", "Quake II", "baseq2", qtrue, qfalse },
	{ "q2.game", "Capture The Flag", "ctf", qfalse, qfalse },
	{ "q2.game", "Quake II Mission Pack: The Reckoning", "xatrix", qfalse, qfalse },
	{ "q2.game", "Quake II Mission Pack: Ground Zero", "rogue", qfalse, qfalse },
	{ "q2.game", "Custom Quake II modification", "", qfalse, qtrue },

};

GList *newMappingModesListForGameFile( Str & mGameFile ){
	GList *mode_list;
	size_t x;

	mode_list = NULL;
	for( x = 0; x < G_N_ELEMENTS( gameModeList ); x++ )
	{
		if( strcmp( mGameFile.GetBuffer(), gameModeList[x].gameFile ) == 0 ) {
			mode_list = g_list_append( mode_list, &gameModeList[x] );
		}
	}
	return mode_list;
}

GList *newModListForGameFile( Str & mGameFile ){
	GList *mod_list;
	size_t x;

	mod_list = NULL;
	for( x = 0; x < G_N_ELEMENTS( gameList ); x++ )
	{
		if( strcmp( mGameFile.GetBuffer(), gameList[x].gameFile ) == 0 ) {
			mod_list = g_list_append( mod_list, &gameList[x] );
		}
	}
	return mod_list;
}

void OnSelchangeComboWhatgame( GtkWidget *widget, gpointer data ){
	GtkWidget *fs_game_entry;
	GtkWidget* game_select;
	size_t x;
#if GTK_CHECK_VERSION( 3, 0, 0 )
	const gchar *fs_game;
#else
	gchar *name;
#endif

	game_select = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "game_select" ) );
#if GTK_CHECK_VERSION( 3, 0, 0 )
	fs_game = gtk_combo_box_get_active_id( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( game_select ) ) );
 	if( !fs_game ) {
		return;
	}
#else
	name = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( game_select ) );
	if( !name ) {
		return;
	}
#endif
	fs_game_entry = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "fs_game_entry" ) );

	for( x = 0; x < G_N_ELEMENTS( gameList ); x++ )
	{
		if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gameList[x].gameFile ) == 0 
#if GTK_CHECK_VERSION( 3, 0, 0 )
			&& strcmp( fs_game, gameList[x].fs_game ) == 0 ) {
#else
			&& strcmp( name, gameList[x].name ) == 0 ) {
#endif
			if( gameList[x].custom ) {
				gtk_entry_set_text( GTK_ENTRY( fs_game_entry ), gameList[x].fs_game );
				gtk_widget_set_sensitive( GTK_WIDGET( fs_game_entry ), true );
				gtk_widget_grab_focus( GTK_WIDGET( fs_game_entry ) );
			} else {
				gtk_entry_set_text( GTK_ENTRY( fs_game_entry ), gameList[x].fs_game );
				gtk_widget_set_sensitive( GTK_WIDGET( fs_game_entry ), false );
			}
			break;
		}
	}
#if GTK_CHECK_VERSION( 3, 0, 0 )
	//combo active id is an interned string
#else
	g_free( name );
#endif

}

void DoProjectSettings(){
	GtkWidget *dialog, *content_area;
	GtkWidget *frame, *label, *vbox, *table1, *table2, *button;
	GtkWidget *add_button, *change_button, *remove_button;
	GtkWidget *brush;
	GtkWidget *scr, *entry;
	GtkWidget *base, *game_select;
	GtkWidget *gamemode_combo, *fs_game_entry;
	GList *mod_list, *gamemode_list;
	GList *lst;
	GList *combo_list = (GList*)NULL;
	const char *fs_game;
	qboolean isBasegame;
	GtkSizeGroup *button_group;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Project Settings" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_window_set_default_size( GTK_WINDOW( dialog ), 550, 400 );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	table1 = gtk_table_new( 3, 2, FALSE );
	gtk_container_add( GTK_CONTAINER( content_area ), table1 );
	gtk_container_set_border_width( GTK_CONTAINER( table1 ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table1 ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table1 ), 5 );
	gtk_widget_show( table1 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_table_attach( GTK_TABLE( table1 ), vbox, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( vbox );

	add_button = button = gtk_button_new_with_label( _( "Add..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( project_add ), dialog );
	gtk_widget_show( button );

	change_button = button = gtk_button_new_with_label( _( "Change..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( project_change ), dialog );
	gtk_widget_show( button );

	remove_button = button = gtk_button_new_with_label( _( "Remove" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( project_remove ), dialog );
	gtk_widget_show( button );

	frame = gtk_frame_new( _( "Misc settings" ) );
	gtk_table_attach( GTK_TABLE( table1 ), frame, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( frame );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, add_button );
	gtk_size_group_add_widget( button_group, change_button );
	gtk_size_group_add_widget( button_group, remove_button );
	g_object_unref( button_group );

	brush = gtk_check_button_new_with_label( _( "Use brush primitives in MAP files (NOTE: experimental feature,\n"
												"required by the texture tools plugin)" ) );
	gtk_container_add( GTK_CONTAINER( frame ), brush );
	gtk_container_set_border_width( GTK_CONTAINER( brush ), 5 );
	gtk_widget_show( brush );

	frame = gtk_frame_new( _( "Menu commands" ) );
	gtk_table_attach( GTK_TABLE( table1 ), frame, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ), 0, 0 );
	gtk_widget_show( frame );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_container_add( GTK_CONTAINER( frame ), scr );
	gtk_container_set_border_width( GTK_CONTAINER( scr ), 5 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );


	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
		gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );

		GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
		gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );

		gtk_widget_show( view );

		g_object_set_data( G_OBJECT( dialog ), "view", view );
		g_object_set_data( G_OBJECT( dialog ), "bsp_commands", store );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );
	}

	frame = gtk_frame_new( _( "Project settings" ) );
	gtk_table_attach( GTK_TABLE( table1 ), frame, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( frame );

	table2 = gtk_table_new( 9, 2, FALSE );
	gtk_container_add( GTK_CONTAINER( frame ), table2 );
	gtk_container_set_border_width( GTK_CONTAINER( table2 ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table2 ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table2 ), 5 );
	gtk_widget_show( table2 );

	mod_list = newModListForGameFile( g_pGameDescription->mGameFile );
	game_select = gtk_combo_box_text_new();
	for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
	{
		const game_t *game_x = (const game_t *)lst->data;
		if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
			gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( game_select ), game_x->fs_game, game_x->name );
#else
			gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( game_select ), game_x->name );
			combo_list = g_list_append( combo_list, (void*)game_x->name );
#endif
		}
	}
	gtk_table_attach( GTK_TABLE( table2 ), game_select, 1, 2, 6, 7,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( game_select );
	g_signal_connect( G_OBJECT( GTK_COMBO_BOX( game_select ) ), "changed",
						G_CALLBACK( OnSelchangeComboWhatgame ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "game_select", game_select );

	fs_game_entry = entry = gtk_entry_new();
	gtk_widget_set_sensitive( GTK_WIDGET( fs_game_entry ), false );
	gtk_table_attach( GTK_TABLE( table2 ), entry, 1, 2, 7, 8,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );
	g_object_set_data( G_OBJECT( dialog ), "fs_game_entry", entry );

	fs_game = ValueForKey( g_qeglobals.d_project_entity, "gamename" );
	isBasegame = qtrue;
	if( fs_game && strlen( fs_game ) > 0 ) {
		for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const game_t *game_x = (const game_t *)lst->data;
			if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 && strcmp( game_x->fs_game, fs_game ) == 0 ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
				gtk_combo_box_set_active_id( GTK_COMBO_BOX( game_select ), fs_game );
#else
				gtk_combo_box_set_active( GTK_COMBO_BOX( game_select ), g_list_index( combo_list, game_x->name ) );
#endif
				isBasegame = qfalse;
				break;
			}
		}
		if( isBasegame ) {
			for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
			{
				const game_t *game_x = (const game_t *)lst->data;
				if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 && game_x->custom ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
					gtk_combo_box_set_active_id( GTK_COMBO_BOX( game_select ), game_x->fs_game );
#else
					gtk_combo_box_set_active( GTK_COMBO_BOX( game_select ), g_list_index( combo_list, game_x->name ) );
#endif
					gtk_widget_set_sensitive( GTK_WIDGET( fs_game_entry ), true );
					isBasegame = qfalse;
					break;
				}
			}
		}
	}
	if( isBasegame ) {
		for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const game_t *game_x = (const game_t *)lst->data;
			if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 && game_x->base ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
				gtk_combo_box_set_active_id( GTK_COMBO_BOX( game_select ), game_x->fs_game );
#else
				gtk_combo_box_set_active( GTK_COMBO_BOX( game_select ), g_list_index( combo_list, game_x->name ) );
#endif
				fs_game = game_x->fs_game;
				break;
			}
		}
	}
	gtk_entry_set_text( GTK_ENTRY( fs_game_entry ), fs_game );
	g_list_free( combo_list );

	gamemode_list = newMappingModesListForGameFile( g_pGameDescription->mGameFile );
	if( gamemode_list ) {
		const char *gamemode;
		qboolean isBasemode;

		combo_list = NULL;


		gamemode_combo = gtk_combo_box_text_new();
		gtk_table_attach( GTK_TABLE( table2 ), gamemode_combo, 1, 2, 8, 9,
						  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
						  (GtkAttachOptions) ( 0 ), 0, 0 );
		gtk_widget_show( gamemode_combo );

		for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
			if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gamemode_x->gameFile ) == 0 ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
				gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( gamemode_combo ), gamemode_x->mode, gamemode_x->name );
#else
				gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( gamemode_combo ), gamemode_x->name );
				combo_list = g_list_append( combo_list, (void*)gamemode_x->name );
#endif
			}
		}

		label = gtk_label_new( _( "Mapping mode" ) );
		gtk_table_attach( GTK_TABLE( table2 ), label, 0, 1, 8, 9,
						  (GtkAttachOptions) ( GTK_FILL ),
						  (GtkAttachOptions) ( 0 ), 0, 0 );
		gtk_misc_set_alignment( GTK_MISC( label ), 1.0, 0.5 );
		gtk_widget_show( label );

		gamemode = ValueForKey( g_qeglobals.d_project_entity, "gamemode" );
		isBasemode = qtrue;
		for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
			if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gamemode_x->gameFile ) == 0 && strcmp( gamemode_x->mode, gamemode ) == 0 ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
				gtk_combo_box_set_active_id( GTK_COMBO_BOX( gamemode_combo ), gamemode_x->mode );
#else
				gtk_combo_box_set_active( GTK_COMBO_BOX( gamemode_combo ), g_list_index( combo_list, gamemode_x->name ) );
#endif
				isBasemode = qfalse;
				break;
			}
		}
		if( isBasemode ) {
			for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
			{
				const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
				if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gamemode_x->gameFile ) == 0 && gamemode_x->base ) {
#if GTK_CHECK_VERSION( 3, 0, 0 )
					gtk_combo_box_set_active_id( GTK_COMBO_BOX( gamemode_combo ), gamemode_x->mode );
#else
					gtk_combo_box_set_active( GTK_COMBO_BOX( gamemode_combo ), g_list_index( combo_list, gamemode_x->name ) );
#endif
					break;
				}
			}
		}
		g_list_free( combo_list );
	}

	/*
	   the usual stuff
	 */

	base = gtk_entry_new();
	g_object_set_data( G_OBJECT( dialog ), "base", base );
	gtk_table_attach( GTK_TABLE( table2 ), base, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( base );


	label = gtk_label_new( _( "basepath" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );


	label = gtk_label_new( _( "Select mod" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 0, 1, 6, 7,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "fs_game" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 0, 1, 7, 8,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	// Initialize fields
	gtk_entry_set_text( GTK_ENTRY( base ), ValueForKey( g_qeglobals.d_project_entity, "basepath" ) );
	UpdateBSPCommandList( dialog );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( brush ), ( g_qeglobals.m_bBrushPrimitMode ) ? TRUE : FALSE );


	g_pGameDescription->Dump();

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		char buf[1024];
		const char *r;
		char *w;
		const char *custom_fs_game, *new_fs_game;
#if GTK_CHECK_VERSION( 3, 0, 0 )
		const gchar *selected_game;
#else
		gchar *selected_game;
#endif

		// convert path to unix format
		for ( r = gtk_entry_get_text( GTK_ENTRY( base ) ), w = buf; *r != '\0'; r++, w++ )
			*w = ( *r == '\\' ) ? '/' : *r;
		// add last slash
		if ( w != buf && *( w - 1 ) != '/' ) {
			*( w++ ) = '/';
		}
		// terminate string
		*w = '\0';
		SetKeyValue( g_qeglobals.d_project_entity, "basepath", buf );

#if GTK_CHECK_VERSION( 3, 0, 0 )
		selected_game = gtk_combo_box_get_active_id( GTK_COMBO_BOX( game_select ) );
#else
		selected_game = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( game_select ) );
#endif
		custom_fs_game = gtk_entry_get_text( GTK_ENTRY( fs_game_entry ) );

		isBasegame = qfalse;
		new_fs_game = NULL;

		if( !selected_game ) {
			isBasegame = qtrue; //should never happen that none is selected
		} else {
			for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
			{
				const game_t *game_x = (const game_t *)lst->data;
#if GTK_CHECK_VERSION( 3, 0, 0 )
				if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 && strcmp( game_x->fs_game, selected_game ) == 0 ) {
#else
				if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), game_x->gameFile ) == 0 && strcmp( game_x->name, selected_game ) == 0 ) {
#endif
					if( game_x->base ) {
						isBasegame = qtrue;
					} else if( game_x->custom ) {
						if( !custom_fs_game || strlen( custom_fs_game ) == 0 ) {
							isBasegame = qtrue;
						} else {
							new_fs_game = custom_fs_game;
						}
					} else {
						new_fs_game = game_x->fs_game;
					}
				}
			}
		}
		if( new_fs_game == NULL ) {
			isBasegame = qtrue;
		}
		if( isBasegame ) {
			DeleteKey( g_qeglobals.d_project_entity, "gamename" );
		} else {
			SetKeyValue( g_qeglobals.d_project_entity, "gamename", new_fs_game );
		}

		if( gamemode_list ) {
			const char *selected_mode;
			const char *new_mode;
			
			selected_mode = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( gamemode_combo ) );
			new_mode = NULL;

			if( !selected_mode ) {
				new_mode = NULL;
			} else {
				for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
				{
					const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
					if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gamemode_x->gameFile ) == 0 && strcmp( gamemode_x->mode, selected_mode ) == 0 ) {
						new_mode = selected_mode;
						break;
					}
				}
			}
			if( !new_mode ) {
				for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
				{
					const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
					if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gamemode_x->gameFile ) == 0 ) {
						new_mode = gamemode_x->mode;
						break;
					}
				}
			}
			if( new_mode ) {
				SetKeyValue( g_qeglobals.d_project_entity, "gamemode", new_mode );
			}
		}

		g_qeglobals.m_strHomeMaps = g_qeglobals.m_strHomeGame;
		const char* str = ValueForKey( g_qeglobals.d_project_entity, "gamename" );
		if ( str[0] == '\0' ) {
			str = g_pGameDescription->mBaseGame.GetBuffer();
		}
		g_qeglobals.m_strHomeMaps += str;
		g_qeglobals.m_strHomeMaps += G_DIR_SEPARATOR;

		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( brush ) ) ) {
			g_qeglobals.m_bBrushPrimitMode = TRUE;
		}
		else{
			g_qeglobals.m_bBrushPrimitMode = FALSE;
		}

		SetKeyValue( g_qeglobals.d_project_entity, "brush_primit", ( g_qeglobals.m_bBrushPrimitMode ? "1" : "0" ) );

		QE_SaveProject( g_PrefsDlg.m_strLastProject.GetBuffer() );

#if GTK_CHECK_VERSION( 3, 0, 0 )
		//combo active id is an interned string
#else
		g_free( selected_game );
#endif

	}

	g_list_free( mod_list );
	g_list_free( gamemode_list );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// MapInfo dialog

void DoMapInfo(){
	static GtkWidget *dialog;
	GtkWidget *vbox, *hbox, *table, *button, *label, *scr;
	GtkWidget *brushes_label, *entities_label, *net_label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( dialog != NULL ) {
		return;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Map Info" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posMapInfoWnd );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_grab_focus( button );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	table = gtk_table_new( 3, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( hbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	brushes_label = gtk_label_new( "" );
	gtk_table_attach( GTK_TABLE( table ), brushes_label, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( brushes_label ), 1.0, 0.5 );
	gtk_widget_show( brushes_label );

	entities_label = gtk_label_new( "" );
	gtk_table_attach( GTK_TABLE( table ), entities_label, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( entities_label ), 1.0, 0.5 );
	gtk_widget_show( entities_label );

	net_label = gtk_label_new( "" );
	gtk_table_attach( GTK_TABLE( table ), net_label, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( net_label ), 1.0, 0.5 );
	gtk_widget_show( net_label );

	label = gtk_label_new( _( "Total Brushes" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Total Entities" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Net brush count\n(non entity)" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );



	label = gtk_label_new( _( "Entity breakdown" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );

	gtk_box_pack_start( GTK_BOX( vbox ), scr, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( scr ), 5 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );

	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

	{
		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_clickable( GTK_TREE_VIEW( view ), TRUE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Entity" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
			gtk_tree_view_column_set_sort_column_id( column, 0 );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Count" ), renderer, "text", 1, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
			gtk_tree_view_column_set_sort_column_id( column, 1 );
		}


		gtk_container_add( GTK_CONTAINER( scr ), view );
		gtk_widget_show( view );
	}

	// Initialize fields
	int TotalBrushes = 0, TotalEntities = 0, Net = 0;

	for ( brush_t* pBrush = active_brushes.next; pBrush != &active_brushes; pBrush = pBrush->next )
	{
		TotalBrushes++;
		if ( pBrush->owner == world_entity ) {
			Net++;
		}
	}

	typedef struct
	{
		const char *name;
		int count;
	} map_t;

	GSList *l, *entitymap = NULL;
	map_t *entry;

	for ( entity_t* pEntity = entities.next; pEntity != &entities; pEntity = pEntity->next )
	{
		TotalEntities++;
		bool add = true;

		for ( l = entitymap; l; l = g_slist_next( l ) )
		{
			entry = (map_t*)l->data;

			if ( strcmp( entry->name, pEntity->eclass->name ) == 0 ) {
				entry->count++;
				add = false;
				break;
			}
		}

		if ( add ) {
			entry = (map_t*)qmalloc( sizeof( map_t ) );
			entry->name = pEntity->eclass->name;
			entry->count = 1;
			entitymap = g_slist_append( entitymap, entry );
		}
	}

	while ( entitymap )
	{
		entry = (map_t*)entitymap->data;
		char tmp[16];
		sprintf( tmp, "%d", entry->count );
		GtkTreeIter iter;
		gtk_list_store_append( GTK_LIST_STORE( store ), &iter );
		gtk_list_store_set( GTK_LIST_STORE( store ), &iter, 0, entry->name, 1, tmp, -1 );
		free( entry );
		entitymap = g_slist_remove( entitymap, entry );
	}

	g_object_unref( G_OBJECT( store ) );

	char tmp[16];
	sprintf( tmp, "%d", TotalBrushes );
	gtk_label_set_text( GTK_LABEL( brushes_label ), tmp );
	sprintf( tmp, "%d", TotalEntities );
	gtk_label_set_text( GTK_LABEL( entities_label ), tmp );
	sprintf( tmp, "%d", Net );
	gtk_label_set_text( GTK_LABEL( net_label ), tmp );


	gtk_dialog_run( GTK_DIALOG( dialog ) );

	save_window_pos( dialog, g_PrefsDlg.mWindowInfo.posMapInfoWnd );

	gtk_widget_destroy( dialog );
	dialog = NULL;
}

// =============================================================================
// Entity List dialog

static void entitylist_select( GtkWidget *widget, gpointer data ){
	GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( data ), "entities" ) );

	GtkTreeSelection* selection = gtk_tree_view_get_selection( view );

	GtkTreeModel* model;
	GtkTreeIter selected;
	if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		entity_t* pEntity;
		gtk_tree_model_get( model, &selected, 1, &pEntity, -1 );

		if ( pEntity ) {
			for ( epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next )
			{
				Select_Deselect();
				Select_Brush( pEntity->brushes.onext );
				Sys_UpdateWindows( W_ALL );
			}
		}
	}
}

static void add_entity_selection( entity_t *pEntity ) {
	if( pEntity ) {
		for ( epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next )
		{
			Select_Brush( pEntity->brushes.onext );
		}
	}
}
static void entitylist_selected_foreach( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data ) {
	entity_t* pEntity;
	gtk_tree_model_get( model, iter, 1, &pEntity, -1 );
	add_entity_selection( pEntity );
	
	//if parent (classname) node is selected then all child nodes are included
	if( !pEntity ) {
		GtkTreeIter child;
		unsigned int i = 0;
		while( gtk_tree_model_iter_nth_child( model, &child, iter, i++ ) ) {
			gtk_tree_model_get( model, &child, 1, &pEntity, -1 );
			add_entity_selection( pEntity );
		}
	}
}
static void entitylist_multiselect( GtkWidget *widget, gpointer data ) {
	GtkTreeView *view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( data ), "entities" ) );

	GtkTreeSelection *selection = gtk_tree_view_get_selection( view );

	Select_Deselect();

	gtk_tree_selection_selected_foreach( selection, entitylist_selected_foreach, NULL );

	Sys_UpdateWindows( W_ALL );
}

static gint entitylist_click( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	if ( event->type == GDK_2BUTTON_PRESS ) {
		entitylist_select( NULL, data );
		return TRUE;
	}
	return FALSE;
}


static void entitylist_selection_changed( GtkTreeSelection* selection, gpointer data ){
	GtkTreeModel *model;
	GtkTreeIter selected;
	GtkTreeIter child;
	entity_t *pEntity = NULL;
	entity_t *firstEntity = NULL;
	GtkListStore* store = GTK_LIST_STORE( g_object_get_data( G_OBJECT( data ), "keyvalues" ) );
	GtkWidget *notebook = (GtkWidget*)g_object_get_data( G_OBJECT( data ), "notebook" );
	GtkWidget *keyvalue_page = (GtkWidget*)g_object_get_data( G_OBJECT( data ), "keyvalue_page" );
	GtkWidget *desc_page = (GtkWidget*)g_object_get_data( G_OBJECT( data ), "description_page" );
	GtkWidget *textview = (GtkWidget*)g_object_get_data( G_OBJECT( data ), "description_textview" );


	gtk_list_store_clear( store );

	if ( gtk_tree_selection_get_mode( selection ) == GTK_SELECTION_MULTIPLE ) {
		GList *rows, *last;
		rows = gtk_tree_selection_get_selected_rows( selection, &model );
		//only the keys/values of the last selected node with entity
		last = g_list_last( rows );
		if ( last ) {
			if ( gtk_tree_model_get_iter( model, &selected, (GtkTreePath *)last->data ) == TRUE ) {
				gtk_tree_model_get( model, &selected, 1, &pEntity, -1 );
				if ( !pEntity ) {
					if( gtk_tree_model_iter_nth_child( model, &child, &selected, 0 ) ) {
						gtk_tree_model_get( model, &child, 1, &firstEntity, -1 );
					}
				}
			}
		}
		g_list_free_full( rows, (GDestroyNotify)gtk_tree_path_free );
		
	} else if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		entity_t* pEntity;
		gtk_tree_model_get( model, &selected, 1, &pEntity, -1 );
	}
	if ( pEntity ) {
		for ( epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next )
		{
			GtkTreeIter appended;
			gtk_list_store_append( store, &appended );
			gtk_list_store_set( store, &appended, 0, pEpair->key, 1, pEpair->value, -1 );
		}

		gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook ), gtk_notebook_page_num( GTK_NOTEBOOK( notebook ), keyvalue_page ) );
	} else {
		GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( textview ) );
		if( firstEntity && firstEntity->eclass && firstEntity->eclass->comments ) {
			gtk_text_buffer_set_text( buffer, firstEntity->eclass->comments, -1 );
		} else {
			gtk_text_buffer_set_text( buffer, _( "No description available." ), -1 );
		}
		gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook ), gtk_notebook_page_num( GTK_NOTEBOOK( notebook ), desc_page ) );
	}

}

static void EnitityList_response( GtkDialog *dialog, gint response_id, gpointer user_data )
{
	save_window_pos( GTK_WIDGET( dialog ), g_PrefsDlg.mWindowInfo.posEntityInfoWnd );

	gtk_widget_destroy( GTK_WIDGET( dialog ) );

	EntityList_dialog = NULL;
}
void DoEntityList(){
	static GtkWidget *dialog;
	GtkWidget *vbox, *hbox, *hbox2, *button, *scr, *content_area;
	GtkWidget *notebook, *label, *textview, *keyvalue_scr, *desc_scr;
	gint keyvalue_index;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( EntityList_dialog != NULL ) {
		return;
	}

	EntityList_dialog = dialog = gtk_dialog_new_with_buttons( _( "Entity Info" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posEntityInfoWnd );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_grab_default( button );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	{
		GtkTreeStore* store = gtk_tree_store_new( 2, G_TYPE_STRING, G_TYPE_POINTER );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		g_signal_connect( G_OBJECT( view ), "button-press-event", G_CALLBACK( entitylist_click ), dialog );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
			gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
			g_signal_connect( G_OBJECT( selection ), "changed", G_CALLBACK( entitylist_selection_changed ), dialog );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );
		g_object_set_data( G_OBJECT( dialog ), "entities", view );

		gtk_widget_show( view );

		{
			{
				GtkTreeIter child;
				gtk_tree_store_append( store, &child, NULL );
				gtk_tree_store_set( store, &child, 0, world_entity->eclass->name, 1, world_entity, -1 );
			}

			GSList *l, *entitymap = NULL;
			typedef struct
			{
				GtkTreeIter node;
				const char *name;
			} map_t;
			map_t *entry;

			for ( entity_t* pEntity = entities.next; pEntity != &entities; pEntity = pEntity->next )
			{
				GtkTreeIter parent;
				bool found = false;

				for ( l = entitymap; l; l = g_slist_next( l ) )
				{
					entry = (map_t*)l->data;

					if ( strcmp( entry->name, pEntity->eclass->name ) == 0 ) {
						parent = entry->node;
						found = true;
						break;
					}
				}

				if ( !found ) {
					gtk_tree_store_append( store, &parent, NULL );
					gtk_tree_store_set( store, &parent, 0, pEntity->eclass->name, 1, NULL, -1 );

					entry = (map_t*)malloc( sizeof( map_t ) );
					entitymap = g_slist_append( entitymap, entry );
					entry->name = pEntity->eclass->name;
					entry->node = parent;
				}

				GtkTreeIter child;
				gtk_tree_store_append( store, &child, &parent );
				gtk_tree_store_set( store, &child, 0, pEntity->eclass->name, 1, pEntity, -1 );
			}

			while ( entitymap )
			{
				free( entitymap->data );
				entitymap = g_slist_remove( entitymap, entitymap->data );
			}
		}

		g_object_unref( G_OBJECT( store ) );
	}

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	notebook = gtk_notebook_new();
	// hide the notebook tabs since its not supposed to look like a notebook
	gtk_notebook_set_show_tabs( GTK_NOTEBOOK( notebook ), FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), notebook, TRUE, TRUE, 0 );
	gtk_widget_show( notebook );

	label = gtk_label_new( _( "Keys/Values" ) );
	gtk_widget_show( label );

	keyvalue_scr = scr = gtk_scrolled_window_new( NULL, NULL );
	keyvalue_index = gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), scr, label );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	{
		GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Key" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Value" ), renderer, "text", 1, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_widget_show( view );

		g_object_set_data( G_OBJECT( dialog ), "keyvalues", store );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );
	}

	label = gtk_label_new( _( "Description" ) );
	gtk_widget_show( label );

	desc_scr = scr = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), scr, label );
	gtk_widget_show( scr );

	textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( textview ), GTK_WRAP_WORD );
	gtk_text_view_set_editable( GTK_TEXT_VIEW( textview ), FALSE );
	gtk_container_add( GTK_CONTAINER( scr ), textview );
	gtk_widget_show( textview );

	gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook ), keyvalue_index );

	g_object_set_data( G_OBJECT( dialog ), "notebook", notebook );
	g_object_set_data( G_OBJECT( dialog ), "keyvalue_page", keyvalue_scr );
	g_object_set_data( G_OBJECT( dialog ), "description_page", desc_scr );
	g_object_set_data( G_OBJECT( dialog ), "description_textview", textview );


	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox2, FALSE, FALSE, 0 );
	gtk_widget_show( hbox2 );

	button = gtk_button_new_with_label( _( "Select" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( entitylist_multiselect ), dialog );
	gtk_widget_set_size_request( button, 60, -1 );
	gtk_widget_show( button );


	g_signal_connect( dialog, "response", G_CALLBACK( EnitityList_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Rotate dialog

static void rotatedlg_apply( GtkWidget *widget, gpointer data ){
	GtkSpinButton *spin;
	float f;

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "x" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 0, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f ); // reset to 0 on Apply

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "y" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 1, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "z" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 2, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f );
}

static void rotatedialog_response( GtkWidget *widget, gint response_id, gpointer data ){

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) {
		rotatedlg_apply( widget, data );
	}

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_CANCEL ) {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}

static void rotatedialog_activate( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog = (GtkWidget *)data;
	rotatedlg_apply( dialog, dialog );
}

void DoRotateDlg(){
	GtkWidget *dialog, *hbox, *table, *label;
	GtkWidget *x, *y, *z, *content_area;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Arbitrary rotation" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Apply" ), GTK_RESPONSE_APPLY );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	table = gtk_table_new( 3, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( hbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "X" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Y" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Z" ) );

	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -359, 359, 1, 10, 0 ) );
	x = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), x, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( x ), 1.0 ); //right align numbers
	gtk_widget_show( x );
	g_object_set_data( G_OBJECT( dialog ), "x", x );
	g_signal_connect_after( x, "activate", G_CALLBACK( rotatedialog_activate ), dialog );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -359, 359, 1, 10, 0 ) );
	y = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), y, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( y ), 1.0 ); //right align numbers
	gtk_widget_show( y );
	g_signal_connect_after( y, "activate", G_CALLBACK( rotatedialog_activate ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "y", y );


	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -359, 359, 1, 10, 0 ) );
	z = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), z, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( z ), 1.0 ); //right align numbers
	gtk_widget_show( z );
	g_signal_connect_after( z, "activate", G_CALLBACK( rotatedialog_activate ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "z", z );



	g_signal_connect( G_OBJECT( dialog ), "response", G_CALLBACK( rotatedialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Gamma dialog

void DoGamma(){
	GtkWidget *dialog, *vbox, *label, *spin, *content_area;
	gint response_id;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Gamma" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "0.0 is brightest\n1.0 is darkest" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "You must restart for the\nsettings to take effect" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 1, 0, 1, 0.1, 0.01, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 2 );
	gtk_box_pack_start( GTK_BOX( vbox ), spin, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	// Initialize dialog
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), g_qeglobals.d_savedinfo.fGamma );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		g_qeglobals.d_savedinfo.fGamma = gtk_spin_button_get_value( GTK_SPIN_BUTTON( spin ) );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Find Brush Dialog

// helper function to walk through the active brushes only and drop the regioned out ones
bool WalkRegionBrush( brush_t **b, entity_t *e ){
	brush_t *b2;
	do
	{
		for ( b2 = active_brushes.next ; b2 != &active_brushes ; b2 = b2->next )
		{
			if ( b2 == *b ) {
				break; // this is an active brush
			}
		}
		if ( b2 == &active_brushes ) {
			// this is a regioned out brush
			*b = ( *b )->onext;
			if ( *b == &e->brushes ) {
				Sys_Status( "No such brush", 0 );
				return false;
			}
		}
	} while ( b2 == &active_brushes );
	return true;
}

void SelectBrush( int entitynum, int brushnum ){
	entity_t *e;
	brush_t *b;
	int i;

	// making this work when regioning is on too

	if ( entitynum == 0 ) {
		e = world_entity;
	}
	else
	{
		e = entities.next;
		while ( --entitynum )
		{
			e = e->next;
			if ( e == &entities ) {
				Sys_Status( "No such entity", 0 );
				return;
			}
			if ( region_active ) {
				// we need to make sure we walk to the next 'active' entity to have a valid --entitynum
				// that is, find a brush that belongs to this entity in the active brushes
				do
				{
					for ( b = active_brushes.next ; b != &active_brushes ; b = b->next )
					{
						if ( b->owner == e ) {
							break; // this is an active entity
						}
					}
					if ( b == &active_brushes ) {
						// this is a regioned out entity
						e = e->next;
						// don't walk past the end either
						if ( e == &entities ) {
							Sys_Status( "No such entity", 0 );
							return;
						}
					}
				} while ( b == &active_brushes );
			}
		}
	}

	b = e->brushes.onext;
	if ( b == &e->brushes ) {
		Sys_Status( "No such brush", 0 );
		return;
	}
	if ( region_active ) {
		if ( !WalkRegionBrush( &b, e ) ) {
			return;
		}
	}

	while ( brushnum-- )
	{
		b = b->onext;
		if ( b == &e->brushes ) {
			Sys_Status( "No such brush", 0 );
			return;
		}
		if ( region_active ) {
			if ( !WalkRegionBrush( &b, e ) ) {
				return;
			}
		}
	}

	Brush_RemoveFromList( b );
	Brush_AddToList( b, &selected_brushes );

	Sys_UpdateWindows( W_ALL );
	for ( i = 0; i < 3; i++ )
	{
		if ( g_pParentWnd->GetXYWnd() ) {
			g_pParentWnd->GetXYWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}

		if ( g_pParentWnd->GetXZWnd() ) {
			g_pParentWnd->GetXZWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}

		if ( g_pParentWnd->GetYZWnd() ) {
			g_pParentWnd->GetYZWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}
	}

	Sys_Status( "Selected", 0 );
}

static void GetSelectionIndex( int *ent, int *brush ){
	brush_t *b, *b2;
	entity_t *entity;

	*ent = *brush = 0;

	b = selected_brushes.next;
	if ( b == &selected_brushes ) {
		return;
	}

	// find entity
	if ( b->owner != world_entity ) {
		( *ent )++;
		for ( entity = entities.next; entity != &entities; entity = entity->next, ( *ent )++ )
			;
	}

	// find brush
	for ( b2 = b->owner->brushes.onext; b2 != b && b2 != &b->owner->brushes; b2 = b2->onext, ( *brush )++ )
		;
}
static void findbrushdialog_apply( GtkWidget *widget, gpointer data ){
	int ent_num;
	int brush_num;
	GtkSpinButton *spin;

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "entity-spin" ) );
	ent_num = gtk_spin_button_get_value_as_int( spin );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "brush-spin" ) );
	brush_num = gtk_spin_button_get_value_as_int( spin );

	SelectBrush( ent_num, brush_num );
}
static void findbrushdialog_response( GtkWidget *widget, gint response_id, gpointer data ){

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) {
		findbrushdialog_apply( widget, widget );
	}

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_CANCEL ) {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}

void DoFind(){
	GtkWidget *dialog, *vbox, *table, *label, *entity, *brush, *content_area, *spin;
	GtkAdjustment *adj;
	int ent, br;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Find Brush" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Entity number" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Brush number" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, G_MAXINT, 1, 10, 0 ) );
	entity = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), entity, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right align numbers
	gtk_widget_show( spin );
	g_object_set_data( G_OBJECT( dialog ), "entity-spin", spin );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, G_MAXINT, 1, 10, 0 ) );
	brush = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), brush, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right align numbers
	gtk_widget_show( spin );
	g_object_set_data( G_OBJECT( dialog ), "brush-spin", spin );

	// Initialize dialog
	GetSelectionIndex( &ent, &br );

	gtk_spin_button_set_value( GTK_SPIN_BUTTON( entity ), ent );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( brush ), br );

	g_signal_connect( dialog, "response", G_CALLBACK( findbrushdialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Arbitrary Sides dialog

void DoSides( bool bCone, bool bSphere, bool bTorus ){
	GtkWidget *dialog, *hbox, *label, *content_area, *spin;
	GtkAdjustment *adj;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Arbitrary sides" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "Sides:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 3, 3, 100, 1, 10, 0 ) );
	spin = gtk_spin_button_new( adj, 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), spin, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		int sides = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spin ) );

		if ( bCone ) {
			Brush_MakeSidedCone( sides );
		}
		else if ( bSphere ) {
			Brush_MakeSidedSphere( sides );
		}
		else{
			Brush_MakeSided( sides );
		}
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// New Patch dialog

void DoNewPatchDlg(){
	GtkWidget *dialog, *table, *hbox, *label, *combo;
	GtkWidget *width_combo, *height_combo, *content_area;
	GList *combo_list;
	GList *lst, *cells;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Patch density" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( hbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Width:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Height:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	combo_list = (GList*)NULL;
	combo_list = g_list_append( combo_list, (void *)_( "3" ) );
	combo_list = g_list_append( combo_list, (void *)_( "5" ) );
	combo_list = g_list_append( combo_list, (void *)_( "7" ) );
	combo_list = g_list_append( combo_list, (void *)_( "9" ) );
	combo_list = g_list_append( combo_list, (void *)_( "11" ) );
	combo_list = g_list_append( combo_list, (void *)_( "13" ) );
	combo_list = g_list_append( combo_list, (void *)_( "15" ) );

	width_combo = combo = gtk_combo_box_text_new();
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( combo ), (const char *)lst->data );
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
	gtk_table_attach( GTK_TABLE( table ), combo, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( combo );

	//right align the combo elements
	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( G_OBJECT( lst->data ), "xalign", 1.0, (char*)NULL );
	}
	g_list_free( cells );

	height_combo = combo = gtk_combo_box_text_new();
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( combo ), (const char *)lst->data );
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
	gtk_table_attach( GTK_TABLE( table ), combo, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( combo );

	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( G_OBJECT( lst->data ), "xalign", 1.0, (char*)NULL );
	}
	g_list_free( cells );

	g_list_free( combo_list );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		char* w = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( width_combo ) );
		char* h = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( height_combo ) );

		Patch_GenericMesh( atoi( w ), atoi( h ), g_pParentWnd->ActiveXY()->GetViewType() );
		Sys_UpdateWindows( W_ALL );

		g_free( w );
		g_free( h );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// New Patch dialog

static void scaledlg_apply( GtkWidget *widget, gpointer data ){
	float sx, sy, sz;
	GtkWidget *x, *y, *z;

	x = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "x" ) );
	y = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "y" ) );
	z = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "z" ) );

	sx = gtk_spin_button_get_value( GTK_SPIN_BUTTON( x ) );
	sy = gtk_spin_button_get_value( GTK_SPIN_BUTTON( y ) );
	sz = gtk_spin_button_get_value( GTK_SPIN_BUTTON( z ) );

	gtk_spin_button_set_value( GTK_SPIN_BUTTON( x ), 1.0f );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( y ), 1.0f );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( z ), 1.0f );

	if ( sx > 0 && sy > 0 && sz > 0 ) {
		Select_Scale( sx, sy, sz );
		Sys_UpdateWindows( W_ALL );
	}
	else{
		Sys_FPrintf( SYS_WRN, _( "Warning.. Tried to scale by a zero value." ) );
	}
}

static void scaledlg_activate( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog = (GtkWidget *)data;
	scaledlg_apply( dialog, dialog );
}

static void ScaleDialog_response( GtkWidget *widget, gint response_id, gpointer data ){

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) {
		scaledlg_apply( widget, widget );
	}

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_CANCEL ) {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}

void DoScaleDlg(){
	GtkWidget *dialog, *hbox, *table, *label;
	GtkWidget *x, *y, *z, *content_area;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Scale" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Apply" ), GTK_RESPONSE_APPLY );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	table = gtk_table_new( 3, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( hbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "X:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Y:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Z:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 1.0, 0, 100, 0.1, 1, 0 ) );
	x = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), x, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( x ), 1.0 ); //right align numbers
	gtk_widget_show( x );
	g_object_set_data( G_OBJECT( dialog ), "x", x );
	g_signal_connect_after( x, "activate", G_CALLBACK( scaledlg_activate ), dialog );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 1.0, 0, 100, 0.1, 1, 0 ) );
	y = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), y, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( y ), 1.0 ); //right align numbers
	gtk_widget_show( y );
	g_object_set_data( G_OBJECT( dialog ), "y", y );
	g_signal_connect_after( y, "activate", G_CALLBACK( scaledlg_activate ), dialog );


	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 1.0, 0, 100, 0.1, 1, 0 ) );
	z = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), z, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( z ), 1.0 ); //right align numbers
	gtk_widget_show( z );
	g_object_set_data( G_OBJECT( dialog ), "z", z );
	g_signal_connect_after( z, "activate", G_CALLBACK( scaledlg_activate ), dialog );

	g_signal_connect( dialog, "response", G_CALLBACK( ScaleDialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Thicken Patch dialog

void DoThickenDlg(){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area;
	GtkWidget *amount, *seams, *group, *spin;
	GtkAdjustment *adj;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	static qboolean bGroupResult = true;

	dialog = gtk_dialog_new_with_buttons( _( "Thicken Patch" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "This produces a set of patches\n"
							  "that contains the original patch along with the\n"
							  "'thick' patch and an optimal set of seam patches." ) );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "Amount:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 1, 1, 100, 1, 10, 0 ) );
	amount = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), amount, FALSE, FALSE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );

	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	seams = gtk_check_button_new_with_label( _( "Seams" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), seams, FALSE, FALSE, 0 );
	gtk_widget_show( seams );

	// bGroupResult
	group = gtk_check_button_new_with_label( _( "Result to func_group" ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( group ), bGroupResult );
	gtk_box_pack_start( GTK_BOX( vbox ), group, FALSE, FALSE, 0 );
	gtk_widget_show( group );


	// Initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( seams ), TRUE );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( amount ), 8 );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		int new_amount;

		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( group ) ) ) {
			bGroupResult = true;
		}
		else{
			bGroupResult = false;
		}
		new_amount = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( amount ) );
		Patch_Thicken( new_amount, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( seams ) ), bGroupResult );
		Sys_UpdateWindows( W_ALL );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// About dialog (no program is complete without one)

static const int ABT_WIDGET_PADDING = 8;

void DoAbout(){
	GtkWidget *dialog, *content_area, *button;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	// create dialog window
	dialog = gtk_dialog_new_with_buttons( _( "About GtkRadiant" ), GTK_WINDOW( g_pParentWnd->m_pWidget ), flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_resizable( GTK_WINDOW( dialog ), FALSE );  

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_grab_focus( button );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	// layout top logo and everything else vertically without border padding
	GtkWidget *outer_vbox = gtk_vbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( content_area ), outer_vbox );
	gtk_container_set_border_width( GTK_CONTAINER( outer_vbox ), 0 ); 
	gtk_widget_show( outer_vbox );

	// radiant logo
	CString s = g_strBitmapsPath;
	s += "logo.png"; 
	GtkWidget *logo_image = gtk_image_new_from_file( s.GetBuffer() );
	gtk_box_pack_start( GTK_BOX( outer_vbox ), logo_image, FALSE, TRUE, 0 );
	gtk_widget_show( logo_image );

	// all other widgets layout
	GtkWidget *inner_vbox = gtk_vbox_new( FALSE, ABT_WIDGET_PADDING );
	gtk_box_pack_start( GTK_BOX( outer_vbox ), inner_vbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( inner_vbox ), ABT_WIDGET_PADDING );
	gtk_widget_show( inner_vbox );

	// informative text
	GtkWidget *info_hbox = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), info_hbox, FALSE, FALSE, 0 );
	gtk_widget_show( info_hbox );

	GtkWidget *info_label = gtk_label_new( 
		"GtkRadiant " RADIANT_VERSION " - " __DATE__ "\n"
		RADIANT_ABOUTMSG "\n\n"
		"This product contains software technology from id Software, Inc.\n"
		"('id Technology'). id Technology 2000 id Software, Inc.\n\n"
		"Visit http://icculus.org/gtkradiant/ to view a full list of credits,\n"
		"changelogs, and to report problems with this software." );

	gtk_box_pack_start( GTK_BOX( info_hbox ), info_label, FALSE, FALSE, 0 );
	gtk_label_set_justify( GTK_LABEL( info_label ), GTK_JUSTIFY_LEFT );
	gtk_misc_set_alignment( GTK_MISC( info_label ), 0.0, 0.5 );
	gtk_label_set_selectable( GTK_LABEL( info_label ), TRUE );
	gtk_widget_show( info_label );

	// OpenGL properties 
	GtkWidget *gl_prop_frame = gtk_frame_new( _( "OpenGL Properties" ) );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), gl_prop_frame, FALSE, TRUE, 0 );
	gtk_widget_show( gl_prop_frame );

	GtkWidget *gl_prop_table = gtk_table_new( 3, 2, FALSE );
	gtk_container_add( GTK_CONTAINER( gl_prop_frame ), gl_prop_table );
	gtk_table_set_row_spacings( GTK_TABLE( gl_prop_table ), 4 );
	gtk_table_set_col_spacings( GTK_TABLE( gl_prop_table ), 4 );
	gtk_container_set_border_width( GTK_CONTAINER( gl_prop_table ), 4 );
	gtk_widget_show( gl_prop_table );

	GtkWidget *vendor_label = gtk_label_new( _( "Vendor:" ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), vendor_label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( vendor_label ), 0.0, 0.5 );
	gtk_widget_show( vendor_label );

	GtkWidget *version_label = gtk_label_new( _( "Version:" ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), version_label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( version_label ), 0.0, 0.5 );
	gtk_widget_show( version_label );

	GtkWidget *renderer_label = gtk_label_new( _( "Renderer:" ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), renderer_label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( renderer_label ), 0.0, 0.5 );
	gtk_widget_show( renderer_label );

	GtkWidget *gl_vendor_label = gtk_label_new( (char*)qglGetString( GL_VENDOR ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), gl_vendor_label, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( gl_vendor_label ), 0.0, 0.5 );
	gtk_label_set_selectable( GTK_LABEL( gl_vendor_label ), TRUE );
	gtk_widget_show( gl_vendor_label );

	GtkWidget *gl_version_label = gtk_label_new( (char*)qglGetString( GL_VERSION ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), gl_version_label, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( gl_version_label ), 0.0, 0.5 );
	gtk_label_set_selectable( GTK_LABEL( gl_version_label ), TRUE );
	gtk_widget_show( gl_version_label );

	GtkWidget *gl_renderer_label = gtk_label_new( (char*)qglGetString( GL_RENDERER ) );
	gtk_table_attach( GTK_TABLE( gl_prop_table ), gl_renderer_label, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( gl_renderer_label ), 0.0, 0.5 );
	gtk_label_set_selectable( GTK_LABEL( gl_renderer_label ), TRUE );
	gtk_widget_show( gl_renderer_label );

	// OpenGL extensions
	GtkWidget *gl_ext_frame = gtk_frame_new( _( "OpenGL Extensions" ) );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), gl_ext_frame, TRUE, TRUE, 0 );
	gtk_widget_show( gl_ext_frame );

	GtkWidget *gl_ext_hbox = gtk_hbox_new( FALSE, ABT_WIDGET_PADDING );
	gtk_container_add( GTK_CONTAINER( gl_ext_frame ), gl_ext_hbox );
	gtk_container_set_border_width( GTK_CONTAINER( gl_ext_hbox ), 4 );
	gtk_widget_show( gl_ext_hbox );

	GtkWidget *gl_ext_scroll = gtk_scrolled_window_new( NULL, NULL );
	gtk_box_pack_start( GTK_BOX( gl_ext_hbox ), gl_ext_scroll, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( gl_ext_scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( gl_ext_scroll ), GTK_SHADOW_IN );
	gtk_widget_show( gl_ext_scroll );

	GtkWidget *gl_ext_textview = gtk_text_view_new();
	gtk_text_view_set_editable( GTK_TEXT_VIEW( gl_ext_textview ), FALSE );
	gtk_container_add( GTK_CONTAINER( gl_ext_scroll ), gl_ext_textview );
	GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( gl_ext_textview ) );
	gtk_text_buffer_set_text( buffer, (char *)qglGetString( GL_EXTENSIONS ), -1 );
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( gl_ext_textview ), GTK_WRAP_WORD );
	gtk_widget_show( gl_ext_textview );


	gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Command List dialog

void DoCommandListDlg(){
	GtkWidget *dialog, *hbox, *scr, *content_area, *button;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Shortcut List" ), NULL, flags, NULL );
    gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_default_size( GTK_WINDOW( dialog ), 400, 400 );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_grab_focus( button );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	{
		GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Command" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Key" ), renderer, "text", 1, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_widget_show( view );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		{
			// Initialize dialog
			CString path;
			path = g_strTempPath;
			path += "commandlist.txt";

			GSList *cmds = NULL;
			int n;

			for ( n = 0; n < g_nCommandCount; n++ )
				cmds = g_slist_append( cmds, (gpointer)g_Commands[n].m_strCommand );
			cmds = g_slist_sort( cmds, ( gint ( * )( const void *, const void * ) )strcmp );

			Sys_Printf( "Writing the command list to %s", path.GetBuffer() );
			FILE * fileout = fopen( path.GetBuffer(), "wt" );

			while ( cmds )
			{
				for ( n = 0; n < g_nCommandCount; n++ )
					if ( cmds->data == g_Commands[n].m_strCommand ) {
						break;
					}

				char c = g_Commands[n].m_nKey;
				CString strLine, strMod( "" ), strKeys( c );

				for ( int k = 0; k < g_nKeyCount; k++ )
				{
					if ( g_Keys[k].m_nVKKey == g_Commands[n].m_nKey ) {
						strKeys = g_Keys[k].m_strName;
						break;
					}
				}

				if ( g_Commands[n].m_nModifiers & RAD_SHIFT ) {
					strMod = "Shift";
				}
				if ( g_Commands[n].m_nModifiers & RAD_ALT ) {
					strMod += ( strMod.GetLength() > 0 ) ? " + Alt" : "Alt";
				}
				if ( g_Commands[n].m_nModifiers & RAD_CONTROL ) {
					strMod += ( strMod.GetLength() > 0 ) ? " + Control" : "Control";
				}
				if ( strMod.GetLength() > 0 ) {
					strMod += " + ";
				}
				strMod += strKeys;

				{
					GtkTreeIter iter;
					gtk_list_store_append( store, &iter );
					gtk_list_store_set( store, &iter, 0, g_Commands[n].m_strCommand, 1, strMod.GetBuffer(), -1 );
				}

				if ( fileout != NULL ) {
					strLine.Format( "%-25s %s\r\n", g_Commands[n].m_strCommand, strMod.GetBuffer() );
					fputs( strLine.GetBuffer(), fileout );
				}

				cmds = g_slist_remove( cmds, cmds->data );
			}

			if ( fileout != NULL ) {
				fclose( fileout );
			}
		}

		g_object_unref( G_OBJECT( store ) );
	}


	gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Texture List dialog

static void TextureListDialog_apply( GtkWidget *widget, gpointer data ){

	GtkWidget *texture_list;
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter iter;

	texture_list = GTK_WIDGET( g_object_get_data( G_OBJECT( widget ), "view" ) );
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( texture_list ) );

	if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
		GtkTreePath* path = gtk_tree_model_get_path( model, &iter );
		if ( gtk_tree_path_get_depth( path ) == 1 ) {
			Texture_ShowDirectory( gtk_tree_path_get_indices( path )[0] + CMD_TEXTUREWAD );
		}
		gtk_tree_path_free( path );
	}
}
static void TextureListDialog_response( GtkWidget *widget, gint response_id, gpointer data ){

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) {
		TextureListDialog_apply( widget, widget );
	}

	if ( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_CANCEL ) {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}
void DoTextureListDlg(){
	GtkWidget *dialog, *hbox, *scr, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Textures" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Load" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	gtk_window_set_default_size( GTK_WINDOW( dialog ), 400, 400 );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	GtkWidget* texture_list;

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );

		{
			// Initialize dialog
			GSList *textures = (GSList*)NULL;
			FillTextureMenu( &textures );
			while ( textures != NULL )
			{
				{
					GtkTreeIter iter;
					gtk_list_store_append( store, &iter );
					gtk_list_store_set( store, &iter, 0, (gchar*)textures->data, -1 );
				}
				free( textures->data );
				textures = g_slist_remove( textures, textures->data );
			}
		}

		g_object_unref( G_OBJECT( store ) );
		gtk_widget_show( view );

		texture_list = view;
		g_object_set_data( G_OBJECT( dialog ), "view", view );
	}

	g_signal_connect( dialog, "response", G_CALLBACK( TextureListDialog_response ), texture_list );

	gtk_widget_show( dialog );
}

// =============================================================================
// Cap dialog

int DoCapDlg( int *type, bool *b_GroupResult ){
	GtkWidget *dialog, *hbox, *table, *pixmap, *group_toggle, *radio_vbox, *content_area;
	GtkWidget *bevel, *endcap, *ibevel, *iendcap;
	gint response_id;
	int ret;
	GSList *group = (GSList*)NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Cap" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	// Gef: Added a vbox to contain the toggle buttons
	radio_vbox = gtk_vbox_new( FALSE, 4 );
	gtk_container_add( GTK_CONTAINER( hbox ), radio_vbox );
	gtk_widget_show( radio_vbox );

	table = gtk_table_new( 4, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( radio_vbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	pixmap = new_image_icon("cap_bevel.png");
	gtk_table_attach( GTK_TABLE( table ), pixmap, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon("cap_endcap.png");
	gtk_table_attach( GTK_TABLE( table ), pixmap, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon("cap_ibevel.png");
	gtk_table_attach( GTK_TABLE( table ), pixmap, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon("cap_iendcap.png");
	gtk_table_attach( GTK_TABLE( table ), pixmap, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( pixmap );

	bevel = gtk_radio_button_new_with_label( group, _( "Bevel" ) );
	gtk_table_attach( GTK_TABLE( table ), bevel, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL | GTK_EXPAND ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( bevel );

	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( bevel ) );

	endcap = gtk_radio_button_new_with_label( group, _( "Endcap" ) );
	gtk_table_attach( GTK_TABLE( table ), endcap, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL | GTK_EXPAND ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( endcap );

	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( endcap ) );

	ibevel = gtk_radio_button_new_with_label( group, _( "Inverted Bevel" ) );
	gtk_table_attach( GTK_TABLE( table ), ibevel, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL | GTK_EXPAND ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( ibevel );

	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( ibevel ) );

	iendcap = gtk_radio_button_new_with_label( group, _( "Inverted Endcap" ) );
	gtk_table_attach( GTK_TABLE( table ), iendcap, 1, 2, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL | GTK_EXPAND ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( iendcap );

	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( iendcap ) );

	// Gef: added radio toggle for func_grouping capped patches
	group_toggle = gtk_check_button_new_with_label( _( "Result to func_group" ) );
	gtk_container_add( GTK_CONTAINER( radio_vbox ), group_toggle );
	gtk_widget_show( group_toggle );

	// Gef: Set the state of the func_group toggle
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( group_toggle ), *b_GroupResult );

	// Initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( bevel ), TRUE );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( bevel ) ) ) {
			*type = BEVEL; //*type = CapDialog::BEVEL;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( endcap ) ) ) {
			*type = ENDCAP; //*type = CapDialog::ENDCAP;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( ibevel ) ) ) {
			*type = IBEVEL; // *type = CapDialog::IBEVEL;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iendcap ) ) ) {
			*type = IENDCAP; // *type = CapDialog::IENDCAP;

		}
		// Gef: Added toggle for optional cap func_grouping
		*b_GroupResult = (bool)gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( group_toggle ) );

		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// Scripts dialog

void DoScriptsDlg(){
	GtkWidget *dialog, *vbox, *vbox2, *hbox, *label, *button, *scr;
	GtkWidget *run_button, *new_button, *edit_button, *content_area;
	GtkSizeGroup *button_group;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Available Scripts - Not Implemented Yet" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

//	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
//	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Close" ), GTK_RESPONSE_CANCEL );
	
	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "WARNING: BrushScripting is in a highly experimental state and is\n"
							  "far from complete. If you attempt to use them it is VERY LIKELY\n"
							  "that Radiant will crash. Save your work before attempting to\n"
							  "make use of any scripting features." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
	gtk_widget_show( label );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	GtkWidget* scripts_list;

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_widget_show( view );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		{
			// Initialize dialog
			CString strINI;
			strINI = g_strGameToolsPath;
			strINI += "/scripts.ini";
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

					{
						GtkTreeIter iter;
						gtk_list_store_append( store, &iter );
						gtk_list_store_set( store, &iter, 0, line, -1 );
					}
				}
				fclose( f );
			}
		}

		g_object_unref( G_OBJECT( store ) );

		scripts_list = view;
	}

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox2, FALSE, FALSE, 0 );
	gtk_widget_show( vbox2 );

	run_button = button = gtk_button_new_with_label( _( "Run" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_show( button );

	new_button = button = gtk_button_new_with_label( _( "New..." ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_set_sensitive( button, FALSE );
	gtk_widget_show( button );

	edit_button = button = gtk_button_new_with_label( _( "Edit..." ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_set_sensitive( button, FALSE );
	gtk_widget_show( button );


	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, run_button );
	gtk_size_group_add_widget( button_group, new_button );
	gtk_size_group_add_widget( button_group, edit_button );
	g_object_unref( button_group );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( scripts_list ) );

		GtkTreeModel* model;
		GtkTreeIter iter;
		if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
			char* script;
			gtk_tree_model_get( model, &iter, 0, &script, -1 );
			RunScriptByName( script, true );
			g_free( script );
		}
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
//  dialog

int DoBSInputDlg( const char *fields[5], float values[5] ){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area;
	GtkWidget *entries[5];
	int i, ret;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "BrushScript Input" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	// Create entries and initialize them
	for ( i = 0; i < 5; i++ )
	{
		if ( strlen( fields[i] ) == 0 ) {
			continue;
		}

		label = gtk_label_new( fields[i] );
		gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
		gtk_misc_set_alignment( GTK_MISC( label ), 1.0, 0.5 );
		gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
		gtk_widget_show( label );

		entries[i] = gtk_entry_new();
		gtk_box_pack_start( GTK_BOX( vbox ), entries[i], TRUE, TRUE, 0 );
		gtk_widget_show( entries[i] );

		char buf[32];
		sprintf( buf, "%f", values[i] );
		gtk_entry_set_text( GTK_ENTRY( entries[i] ), buf );
	}


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	for ( i = 0; i < 5; i++ )
	{
		if ( strlen( fields[i] ) == 0 ) {
			continue;
		}

		values[i] = atof( gtk_entry_get_text( GTK_ENTRY( entries[i] ) ) );
	}
	switch( response_id ) {
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	//case GTK_RESPONSE_CANCEL:
	default:
		ret = IDCANCEL;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// TextureLayout dialog

int DoTextureLayout( float *fx, float *fy ){
	GtkWidget *dialog, *vbox, *hbox, *table, *label;
	GtkWidget *x, *y, *content_area;
	gint response_id;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Patch texture layout" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "Texture will be fit across the patch based\n"
							  "on the x and y values given. Values of 1x1\n"
							  "will \"fit\" the texture. 2x2 will repeat\n"
							  "it twice, etc." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
	gtk_widget_show( label );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Texture x:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Texture y:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	x = gtk_entry_new();
	gtk_table_attach( GTK_TABLE( table ), x, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( x );

	y = gtk_entry_new();
	gtk_table_attach( GTK_TABLE( table ), y, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( y );


	// Initialize
	gtk_entry_set_text( GTK_ENTRY( x ), _( "4.0" ) );
	gtk_entry_set_text( GTK_ENTRY( y ), _( "4.0" ) );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {

		*fx = atof( gtk_entry_get_text( GTK_ENTRY( x ) ) );
		*fy = atof( gtk_entry_get_text( GTK_ENTRY( y ) ) );

		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// Name dialog

char* DoNameDlg( const char* title ){
	GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry;
	int loop = 1, ret = IDCANCEL;
	char *str;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), title );
	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_container_add( GTK_CONTAINER( dlg ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );

	label = gtk_label_new( _( "Name:" ) );
	gtk_widget_show( label );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

	entry = gtk_entry_new();
	gtk_widget_show( entry );
	gtk_box_pack_start( GTK_BOX( hbox ), entry, TRUE, TRUE, 0 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_size_request( button, 60, -2 );

	button = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_set_size_request( button, 60, -2 );

	gtk_grab_add( dlg );
	gtk_widget_show( dlg );

	while ( loop )
		gtk_main_iteration();

	if ( ret == IDOK ) {
		str = strdup( gtk_entry_get_text( GTK_ENTRY( entry ) ) );
	}
	else{
		str = NULL;
	}

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	return str;
}

// =============================================================================
// NewProject dialog


char* DoNewProjectDlg(){
	GtkWidget *dialog, *vbox, *label, *entry, *check, *content_area;
	gint response_id;
	char *str;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	// start by a warning message
// mattn: URLs no longer valid
//  CString msg;
//  msg = "Are you sure you want a new project?\n";
//  msg += "Please note that creating a new project is not the prefered way to setup GtkRadiant for mod editing.\n";
//  msg += "Check http://www.qeradiant.com/faq/index.cgi?file=220 for more information";
//  if (gtk_MessageBox(NULL, msg.GetBuffer(), _("Confirm"), MB_YESNO, "http://www.qeradiant.com/faq/index.cgi?file=220" ) == IDNO)
//  {
//    return NULL;
//  }

	dialog = gtk_dialog_new_with_buttons( _( "New Project" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "This will create a new directory beneath your\n"
							  "game path based on the project name you give." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Project name:" ) );

	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( vbox ), entry, TRUE, TRUE, 0 );
	gtk_widget_show( entry );

	check = gtk_check_button_new_with_label( _( "Include game dll files" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_widget_set_sensitive( check, FALSE );
	gtk_widget_show( check );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		str = strdup( gtk_entry_get_text( GTK_ENTRY( entry ) ) );
	} else {
		str = NULL;
	}

	gtk_widget_destroy( dialog );

	return str;
}

// =============================================================================
// Text Editor dialog

// master window widget
static GtkWidget *text_editor = NULL;
static GtkWidget *text_widget; // slave, text widget from the gtk editor

static gint editor_delete( GtkWidget *widget, gpointer data ){
	if ( gtk_MessageBox( widget, _( "Close the shader editor ?" ), _( "Radiant" ), MB_YESNO ) == IDNO ) {
		return TRUE;
	}

	gtk_widget_hide( text_editor );

	return TRUE;
}

static void editor_save( GtkWidget *widget, gpointer data ){
	FILE *f = fopen( (char*)g_object_get_data( G_OBJECT( data ), "filename" ), "w" );
	gpointer text = g_object_get_data( G_OBJECT( data ), "text" );

	if ( f == NULL ) {
		gtk_MessageBox( GTK_WIDGET( data ), _( "Error saving file !" ) );
		return;
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text ) );
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds( buffer, &start, &end );
	char *str = gtk_text_buffer_get_text( buffer, &start, &end, FALSE );
	fwrite( str, 1, strlen( str ), f );
	fclose( f );
	g_free( str );
}

static void editor_close( GtkWidget *widget, gpointer data ){
	if ( gtk_MessageBox( text_editor, _( "Close the shader editor ?" ), _( "Radiant" ), MB_YESNO ) == IDNO ) {
		return;
	}

	gtk_widget_hide( text_editor );
}

// several attempts
#if 0
#ifdef _WIN32

HWND FindEditWindow(){
	return FindWindow( "TFormEditPadLite", NULL );
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TFormEditPadLite", NULL );
	if ( hwnd ) {
		hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
		if ( hwnd ) {
			hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
			if ( hwnd ) {
				hwnd = FindWindowEx( hwnd, NULL, "TEditPadEditor", NULL );
				if ( hwnd ) {
					hwnd = FindWindowEx( hwnd, NULL, "TWinControlProxy", NULL );
					return hwnd;
				}
			}
		}
	}
	return NULL;
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TFormEditPadLite", NULL );
	if ( hwnd ) {
		hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
		if ( hwnd ) {
			hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
			if ( hwnd ) {
				hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
				if ( hwnd ) {
					hwnd = FindWindowEx( hwnd, NULL, "TFrameSearchReplace", NULL );
					if ( hwnd ) {
						hwnd = FindWindowEx( hwnd, NULL, "TJGStringEditorControl", NULL );
						return hwnd;
					}
				}
			}
		}
	}
	return NULL;
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TEditPadForm", NULL );
	HWND hwndEdit = NULL;
	if ( hwnd != NULL ) {
		HWND hwndTab = FindWindowEx( hwnd, NULL, "TTabControl", NULL );
		if ( hwndTab != NULL ) {
			hwndEdit = FindWindowEx( hwndTab, NULL, "TRicherEdit", NULL );
		}
	}
	return hwndEdit;
}
#endif
#endif // #if 0

static void CreateGtkTextEditor(){
	GtkWidget *dlg;
	GtkWidget *vbox, *hbox, *button, *scr, *text;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( editor_delete ), NULL );
	gtk_window_set_default_size( GTK_WINDOW( dlg ), 600, 300 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	scr = gtk_scrolled_window_new( NULL, NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	text = gtk_text_view_new();
	gtk_container_add( GTK_CONTAINER( scr ), text );
	g_object_set_data( G_OBJECT( dlg ), "text", text );
	gtk_text_view_set_editable( GTK_TEXT_VIEW( text ), TRUE );
	gtk_widget_show( text );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	button = gtk_button_new_with_label( _( "Close" ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( editor_close ), dlg );
	gtk_widget_set_size_request( button, 60, -2 );
	gtk_widget_show( button );

	button = gtk_button_new_with_label( _( "Save" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( editor_save ), dlg );
	gtk_widget_set_size_request( button, 60, -2 );

	text_editor = dlg;
	text_widget = text;
}

static void DoGtkTextEditor( const char* filename, guint cursorpos ){
	if ( !text_editor ) {
		CreateGtkTextEditor(); // build it the first time we need it

	}
	// Load file
	FILE *f = fopen( filename, "r" );

	if ( f == NULL ) {
		Sys_Printf( "Unable to load file %s in shader editor.\n", filename );
		gtk_widget_hide( text_editor );
	}
	else
	{
		fseek( f, 0, SEEK_END );
		int len = ftell( f );
		void *buf = qmalloc( len );
		void *old_filename;

		rewind( f );
		fread( buf, 1, len, f );

		gtk_window_set_title( GTK_WINDOW( text_editor ), filename );

		GtkTextBuffer* text_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_widget ) );
		gtk_text_buffer_set_text( text_buffer, (char*)buf, len );

		old_filename = g_object_get_data( G_OBJECT( text_editor ), "filename" );
		if ( old_filename ) {
			free( old_filename );
		}
		g_object_set_data( G_OBJECT( text_editor ), "filename", strdup( filename ) );

		// trying to show later
		gtk_widget_show( text_editor );

#ifdef _WIN32
		while ( gtk_events_pending() )
			gtk_main_iteration();
#endif

		// only move the cursor if it's not exceeding the size..
		// NOTE: this is erroneous, cursorpos is the offset in bytes, not in characters
		// len is the max size in bytes, not in characters either, but the character count is below that limit..
		// thinking .. the difference between character count and byte count would be only because of CR/LF?
		{
			GtkTextIter text_iter;
			// character offset, not byte offset
			gtk_text_buffer_get_iter_at_offset( text_buffer, &text_iter, cursorpos );
			gtk_text_buffer_place_cursor( text_buffer, &text_iter );
		}

#ifdef _WIN32
		gtk_widget_queue_draw( text_widget );
#endif

		free( buf );
		fclose( f );
	}
}

void DoTextEditor( const char* filename, int cursorpos ){
	CString strEditCommand;
#ifdef _WIN32
	if ( g_PrefsDlg.m_bUseWin32Editor ) {
		HINSTANCE result;
		Sys_Printf( "Opening file '%s'.\n", filename );
		result = ShellExecute( (HWND)GDK_WINDOW_HWND( gtk_widget_get_window( g_pParentWnd->m_pWidget ) ), "open", filename, NULL, NULL, SW_SHOW );
		if( (int)result <= 32 ) {
			const char *errstr;
			switch( (int)result ) {
				case SE_ERR_OOM:
				case 0:
					errstr = _( "The operating system is out of memory or resources." );
					break;
				case ERROR_FILE_NOT_FOUND:
				//case SE_ERR_FNF:
					errstr = _( "The specified file was not found." );
					break;
				case SE_ERR_NOASSOC: 
				{
					SHELLEXECUTEINFO sei = {0};
					sei.cbSize = sizeof( sei );
					sei.nShow = SW_SHOWNORMAL;
					sei.lpFile = TEXT( filename );
					sei.fMask = SEE_MASK_CLASSNAME;
					sei.lpVerb = TEXT( "open" );
					sei.lpClass = TEXT( ".txt" );

					if( ShellExecuteEx( &sei ) )
					{
						return;
					}
					errstr = _( "There is no application associated with the given file name extension." );
					break;
				}
				case ERROR_PATH_NOT_FOUND:
				//case SE_ERR_PNF:
					errstr = _( "The specified path was not found." );
					break;
				default:
					errstr = "";
					break;
			}
			Sys_FPrintf( SYS_WRN, "Failed to open file '%s'. %s\n", filename, errstr );
		}
		return;
	}
#else
	// check if a custom editor is set
	if ( ( g_PrefsDlg.m_bUseCustomEditor ) && ( g_PrefsDlg.m_strEditorCommand.GetLength() > 0 ) ) {
		strEditCommand = g_PrefsDlg.m_strEditorCommand;
		strEditCommand += " \"";
		strEditCommand += filename;
		strEditCommand += "\"";

		Sys_Printf( "Launching: %s\n", strEditCommand.GetBuffer() );
		// note: linux does not return false if the command failed so it will assume success
		if ( Q_Exec( NULL, (char *)strEditCommand.GetBuffer(), NULL, true ) == false ) {
			Sys_FPrintf( SYS_WRN, "Warning: Failed to execute %s, using default\n", strEditCommand.GetBuffer() );
		}
		else
		{
			// the command (appeared) to run successfully, no need to do anything more
			return;
		}
	}
#endif

	DoGtkTextEditor( filename, cursorpos );

	// old win32 code with EditPad bindings, broken
#if 0
	strEditCommand = g_strAppPath.GetBuffer();
	strEditCommand += "editpad.exe";
	strEditCommand += " \"";
	strEditCommand += filename;
	strEditCommand += "\"";
	if ( Q_Exec( NULL, (char *)strEditCommand.GetBuffer(), NULL, true ) == false ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Gtk shader editor is not fully functional on windows in general and unstable on win98 in particular.\n" );
		Sys_FPrintf( SYS_WRN, "  you can use EditPad instead (install it in Radiant's directory): http://www.qeradiant.com/?data=files&files_dir=18\n" );
		DoGtkTextEditor( filename, cursorpos );
	}
	else
	{
		// TTimo: we used to call Delay here, to continue processing messages. But it seems to induce a lot of instabilities.
		// so now the user will simply have to wait.
		Sleep( 1500 );

		// now grab the edit window and scroll to the shader we want to edit
		HWND hwndEdit = FindEditWindow();

		if ( hwndEdit != NULL ) {
			PostMessage( hwndEdit, EM_SETSEL, cursorpos, cursorpos );
		}
		else{
			Sys_Printf( "Unable to load shader editor.\n" );
		}
	}
#endif
}

// =============================================================================
// Light Intensity dialog

int DoLightIntensityDlg( int *intensity ){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area, *spinbutton, *button;
	GtkAdjustment *adj;
	gint response_id;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Light intensity" ), NULL, flags, NULL );	
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel_group );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_add_accelerator( button, "clicked", accel_group,
								GDK_KEY_Return, (GdkModifierType)0, GTK_ACCEL_VISIBLE );
	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_widget_add_accelerator( button, "clicked", accel_group,
								GDK_KEY_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "ESC for default, ENTER to validate" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( *intensity, 0, G_MAXINT, 1, 10, 0 ) );
	spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), spinbutton, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spinbutton ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spinbutton ), 1.0 ); //right
	gtk_widget_show( spinbutton );

	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spinbutton ), *intensity );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {
		*intensity = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinbutton ) );
		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}