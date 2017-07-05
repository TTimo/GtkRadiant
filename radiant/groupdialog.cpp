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
// Floating dialog that contains a notebook with at least Entities and Group tabs
// I merged the 2 MS Windows dialogs in a single class
//
// Leonardo Zide (leo@lokigames.com)
//

#ifndef _WIN32
  #include <unistd.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include "stdafx.h"
#include "groupdialog.h"

GtkWidget*  EntWidgets[EntLast];
int inspector_mode;                     // W_TEXTURE, W_ENTITY, or W_CONSOLE
qboolean multiple_entities;
qboolean disable_spawn_get = false;
entity_t        *edit_entity;
/*
   static GdkPixmap *tree_pixmaps[7];
   static GdkBitmap *tree_masks[7];
 */
#define IMG_PATCH 0
#define IMG_BRUSH 1
#define IMG_GROUP 2
#define IMG_ENTITY 3
#define IMG_ENTITYGROUP 4
#define IMG_MODEL 5
#define IMG_SCRIPT 6

// misc group support
#define MAX_GROUPS 4096
#define GROUP_DELIMETER '@'
#define GROUPNAME "QER_Group_%i"

GroupDlg g_wndGroup;
GroupDlg *g_pGroupDlg = &g_wndGroup;

// group_t are loaded / saved through "group_info" entities
// they hold epairs for group settings and additionnal access info (tree nodes)
group_t *g_pGroups = NULL;

// the number of active spawnflags
static int spawnflag_count;
// table: index, match spawnflag item to the spawnflag index (i.e. which bit)
static int spawn_table[MAX_FLAGS];
// we change the layout depending on how many spawn flags we need to display
// the table is a 4x4 in which we need to put the comment box EntWidgets[EntComment] and the spawn flags..
static GtkWidget *LayoutTable;
// 0: none of them are hooked
// 1: only the text, 2: text and four checks, 3: text and 8 checks
static int widget_state = 0;

static void entity_check( GtkWidget *widget, gpointer data );

// =============================================================================
// Global functions

/*
   ===============================================================

   ENTITY WINDOW

   ===============================================================
 */

void FillClassList(){
	GtkListStore *store;
	GtkTreeModel * model;
	model = GTK_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( EntWidgets[EntList] ) ) );
	store = GTK_LIST_STORE( model );

	gtk_list_store_clear( store );

	for ( eclass_t* e = eclass ; e ; e = e->next )
	{
		GtkTreeIter iter;
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, e->name, 1, e, -1 );
	}
}

// SetKeyValuePairs
//
// Reset the key/value (aka property) listbox and fill it with the
// k/v pairs from the entity being edited.
//

void SetKeyValuePairs( bool bClearMD3 ){
	GtkListStore *store;
	GtkTreeModel * model;
	model = GTK_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( EntWidgets[EntProps] ) ) );
	store = GTK_LIST_STORE( model );

	gtk_list_store_clear( store );

	if ( edit_entity == NULL ) {
		// if there's no entity, then display no key/values
		return;
	}

	// save current key/val pair around filling epair box
	// row_select wipes it and sets to first in list
	Str strKey = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntKeyField] ) );
	Str strVal = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntValueField] ) );


	// Walk through list and add pairs
	for ( epair_t* epair = edit_entity->epairs ; epair ; epair = epair->next )
	{
		GtkTreeIter iter;
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, epair->key, 1, epair->value, -1 );
	}

	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), strKey.GetBuffer() );
	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), strVal.GetBuffer() );

	Sys_UpdateWindows( W_CAMERA | W_XY );
}

// SetSpawnFlags
//
// Update the checkboxes to reflect the flag state of the entity
//
void SetSpawnFlags( void ){
	int f, i, v;

	disable_spawn_get = true;

	f = atoi( ValueForKey( edit_entity, "spawnflags" ) );
	for ( i = 0 ; i < spawnflag_count ; i++ )
	{
		v = !!( f & ( 1 << spawn_table[i] ) );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( EntWidgets[EntCheck1 + i] ), v );
	}
	// take care of the remaining ones
	for ( i = spawnflag_count ; i < MAX_FLAGS ; i++ )
	{
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( EntWidgets[EntCheck1 + i] ), FALSE );
	}

	disable_spawn_get = false;
}

// GetSpawnFlags
//
// Update the entity flags to reflect the state of the checkboxes
//
// NOTE: this function had a tendency to add "spawnflags" "0" on most entities
//   if this wants to set spawnflags to zero, remove the key

void GetSpawnFlags( void ){
	int f, i, v;
	char sz[32];

	f = 0;
	for ( i = 0 ; i < spawnflag_count ; i++ )
	{
		v = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( EntWidgets[EntCheck1 + i] ) );
		f |= v << spawn_table[i];
	}

	if ( f == 0 ) {
		// remove all "spawnflags" keys
		if ( multiple_entities ) {
			brush_t   *b;

			for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
				DeleteKey( b->owner, "spawnflags" );
		}
		else{
			DeleteKey( edit_entity, "spawnflags" );
		}
	}
	else
	{
		sprintf( sz, "%i", f );
		if ( multiple_entities ) {
			brush_t   *b;

			for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
				SetKeyValue( b->owner, "spawnflags", sz );
		}
		else{
			SetKeyValue( edit_entity, "spawnflags", sz );
		}
	}
	SetKeyValuePairs();
}

//#define DBG_UPDATESEL

// UpdateSel
//
// Update the listbox, checkboxes and k/v pairs to reflect the new selection
// iIndex is the index in the list box with the class name, -1 if not found
bool UpdateSel( int iIndex, eclass_t *pec ){
	int i, next_state;
	brush_t *b;

	// syndrom of crappy code, we may get into stack overflowing crap with this function and Gtk
	// if we play with the list of entity classes
	// using a static flag to prevent recursion
	static bool bBlockUpdate = false;

	if ( bBlockUpdate ) {
		return FALSE; // NOTE TTimo wtf is the return value for anyway?

	}
#ifdef DBG_UPDATESEL
	Sys_FPrintf( SYS_WRN, "UpdateSel\n" );
#endif

	if ( selected_brushes.next == &selected_brushes ) {
		edit_entity = world_entity;
		multiple_entities = false;
	}
	else
	{
		edit_entity = selected_brushes.next->owner;
		for ( b = selected_brushes.next->next ; b != &selected_brushes ; b = b->next )
		{
			if ( b->owner != edit_entity ) {
				multiple_entities = true;
				break;
			}
		}
	}

	if ( iIndex != -1 ) {
#ifdef DBG_UPDATESEL
		Sys_FPrintf( SYS_WRN,"Setting focus_row to %d\n", iIndex );
#endif
		bBlockUpdate = true;

		GtkTreeView* view = GTK_TREE_VIEW( EntWidgets[EntList] );
		GtkTreePath* path = gtk_tree_path_new();
		gtk_tree_path_append_index( path, iIndex );
		gtk_tree_selection_select_path( gtk_tree_view_get_selection( view ), path );
		gtk_tree_view_scroll_to_cell( view, path, NULL, FALSE, 0, 0 );
		gtk_tree_path_free( path );

		bBlockUpdate = false;
	}

	if ( pec == NULL ) {
		return TRUE;
	}

	// Set up the description
	{
		GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( EntWidgets[EntComment] ) );
		gtk_text_buffer_set_text( buffer, pec->comments, -1 );
	}

	spawnflag_count = 0;

	// do a first pass to count the spawn flags, don't touch the widgets, we don't know in what state they are
	for ( i = 0 ; i < MAX_FLAGS ; i++ )
	{
		if ( pec->flagnames[i] && pec->flagnames[i][0] != 0 && strcmp( pec->flagnames[i],"-" ) ) {
			spawn_table[spawnflag_count] = i;
			spawnflag_count++;
		}
	}

	// what's new widget state
	if ( spawnflag_count == 0 ) {
		next_state = 1;
	}
	else if ( spawnflag_count <= 4 ) {
		next_state = 2;
	}
	else if ( spawnflag_count <= 8 ) {
		next_state = 3;
	}
	else if ( spawnflag_count <= 12 ) {
		next_state = 4;
	}
	else{
		next_state = 5;
	}
	widget_state = next_state;
	static int last_count = 0;

	// disable all remaining boxes
	// NOTE: these boxes might not even be on display
	for ( i = 0; i < last_count; i++ )
	{
		GtkWidget *widget = EntWidgets[EntCheck1 + i];

		gtk_button_set_label( GTK_BUTTON( widget ), " " );

		g_object_ref( widget );
		gtk_widget_hide( widget );
		gtk_container_remove( GTK_CONTAINER( LayoutTable ), widget );
	}
	last_count = spawnflag_count;

	for ( i = 0 ; i < spawnflag_count ; i++ )
	{
		GtkWidget* widget = EntWidgets[EntCheck1 + i];
		gtk_widget_show( widget );

		Str str;
		str = pec->flagnames[spawn_table[i]];
		str.MakeLower();

//    gtk_table_attach (GTK_TABLE (LayoutTable), widget, i%4, i%4+1, i/4, i/4+1,
		gtk_table_attach( GTK_TABLE( LayoutTable ), widget, i % 4, i % 4 + 1, i / 4, i / 4 + 1,
						  (GtkAttachOptions) ( GTK_FILL ),
						  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
		g_object_unref( widget );

		gtk_button_set_label( GTK_BUTTON( widget ), str.GetBuffer() );
	}

	SetSpawnFlags();

	SetKeyValuePairs();

	return TRUE;
}

bool UpdateEntitySel( eclass_t *pec ){
#ifdef DBG_UPDATESEL
	Sys_FPrintf( SYS_WRN, "UpdateEntitySel\n" );
#endif

	GtkTreeModel* model = GTK_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( EntWidgets[EntList] ) ) );
	GtkTreeIter iter;
	unsigned int i = 0;
	for ( gboolean good = gtk_tree_model_get_iter_first( model, &iter ); good != FALSE; good = gtk_tree_model_iter_next( model, &iter ) )
	{
		char* text;
		gtk_tree_model_get( model, &iter, 0, &text, -1 );
		if ( strcmp( text, pec->name ) == 0 ) {
#ifdef DBG_UPDATESEL
			Sys_FPrintf( SYS_WRN, "found a match: %d %s\n", i, pec->name );
#endif
			return UpdateSel( i, pec );
		}
		g_free( text );
		++i;
	}
	return UpdateSel( -1, pec );
}

// CreateEntity
//
// Creates a new entity based on the currently selected brush and entity type.
//

void CreateEntity( void ){
	GtkTreeView* view = GTK_TREE_VIEW( EntWidgets[EntList] );

	// check to make sure we have a brush
	if ( selected_brushes.next == &selected_brushes ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "You must have a selected brush to create an entity" ), _( "info" ) );
		return;
	}

	// find out what type of entity we are trying to create
	GtkTreeModel* model;
	GtkTreeIter iter;
	if ( gtk_tree_selection_get_selected( gtk_tree_view_get_selection( view ), &model, &iter ) == FALSE ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "You must have a selected class to create an entity" ), _( "info" ) );
		return;
	}

	char* text;
	gtk_tree_model_get( model, &iter, 0, &text, -1 );
	CreateEntityFromName( text, vec3_origin );
	g_free( text );

	if ( selected_brushes.next == &selected_brushes ) {
		edit_entity = world_entity;
	}
	else{
		edit_entity = selected_brushes.next->owner;
	}

	SetKeyValuePairs();
	Select_Deselect();
	Select_Brush( edit_entity->brushes.onext );
	Sys_UpdateWindows( W_ALL );
}

/*
   ===============
   AddProp

   ===============
 */
void AddProp(){
	if ( edit_entity == NULL ) {
		return;
	}

	// Get current selection text
	const char* key = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntKeyField] ) );
	const char* value = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntValueField] ) );


	// TTimo: if you change the classname to worldspawn you won't merge back in the structural brushes but create a parasite entity
	if ( !strcmp( key, "classname" ) && !strcmp( value, "worldspawn" ) ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget,  _( "Cannot change \"classname\" key back to worldspawn." ), NULL, MB_OK );
		return;
	}


	// RR2DO2: we don't want spaces in entity keys
	if ( strstr( key, " " ) ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "No spaces are allowed in entity keys." ), NULL, MB_OK );
		return;
	}

	if ( multiple_entities ) {
		brush_t *b;

		for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
			SetKeyValue( b->owner, key, value );
	}
	else{
		SetKeyValue( edit_entity, key, value );
	}

	// refresh the prop listbox
	SetKeyValuePairs();


#ifdef USEPLUGINENTITIES
	// if it's a plugin entity, perhaps we need to update some drawing parameters
	// NOTE: perhaps moving this code to a seperate func would help if we need it in other places
	// TODO: we need to call some update func in the IPluginEntity in case model name changes etc.
	// ( for the moment only bounding brush is updated ), see UpdateModelBrush in Ritual's Q3Radiant
	if ( edit_entity->eclass->nShowFlags & ECLASS_PLUGINENTITY ) {
		vec3_t mins, maxs;
		edit_entity->pPlugEnt->GetBounds( mins, maxs );
		// replace old bounding brush by newly computed one
		// NOTE: this part is similar to Entity_BuildModelBrush in Ritual's Q3Radiant, it can be
		// usefull moved into a seperate func
		brush_t *b,*oldbrush;
		if ( edit_entity->brushes.onext != &edit_entity->brushes ) {
			oldbrush = edit_entity->brushes.onext;
		}
		b = Brush_Create( mins, maxs, &edit_entity->eclass->texdef );
		Entity_LinkBrush( edit_entity, b );
		Brush_Build( b, true );
		Select_Deselect();
		Brush_AddToList( edit_entity->brushes.onext, &selected_brushes );
		if ( oldbrush ) {
			Brush_Free( oldbrush );
		}
	}
#endif // USEPLUGINENTITIES
}

/*
   ===============
   DelProp

   ===============
 */
void DelProp( void ){
	if ( edit_entity == NULL ) {
		return;
	}

	// Get current selection text
	const char* key = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntKeyField] ) );

	if ( multiple_entities ) {
		brush_t *b;

		for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
			DeleteKey( b->owner, key );
	}
	else{
		DeleteKey( edit_entity, key );
	}

	// refresh the prop listbox
	SetKeyValuePairs();
}

void ResetEntity(){
	epair_t *pep;
	int i;

	if ( edit_entity == NULL ) {
		return;
	}

	if ( multiple_entities ) {
		brush_t *b;

		for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
			for ( pep = b->owner->epairs; pep; )
			{
				if ( strcmp( pep->key, "classname" ) != 0 ) {
					DeleteKey( b->owner, pep->key );
					pep = b->owner->epairs;
				}
				else{
					pep = pep->next;
				}
			}
	}
	else{
		for ( pep = edit_entity->epairs; pep; )
		{
			if ( strcmp( pep->key, "classname" ) != 0 ) {
				DeleteKey( edit_entity, pep->key );
				pep = edit_entity->epairs;
			}
			else{
				pep = pep->next;
			}
		}
	}

	// refresh the dialog
	SetKeyValuePairs();
	for ( i = EntCheck1; i <= EntCheck16; i++ )
		g_signal_handlers_block_by_func( G_OBJECT( EntWidgets[i] ), (gpointer)G_CALLBACK( entity_check ), NULL );
	SetSpawnFlags();
	for ( i = EntCheck1; i <= EntCheck16; i++ )
		g_signal_handlers_unblock_by_func( G_OBJECT( EntWidgets[i] ), (gpointer)G_CALLBACK( entity_check ), NULL );
}

bool GetSelectAllCriteria( CString &strKey, CString &strVal ){
	GtkTreeModel* model;
	GtkTreeIter iter;
	if ( gtk_tree_selection_get_selected( gtk_tree_view_get_selection( GTK_TREE_VIEW( EntWidgets[EntProps] ) ), &model, &iter )
		 && ( inspector_mode == W_ENTITY )
		 && gtk_widget_get_visible( g_pGroupDlg->m_pWidget ) ) {
		strKey = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntKeyField] ) );
		strVal = gtk_entry_get_text( GTK_ENTRY( EntWidgets[EntValueField] ) );
		return TRUE;
	}
	return FALSE;
}


void AssignSound(){
	char buffer[NAME_MAX];

	strcpy( buffer, g_qeglobals.m_strHomeMaps.GetBuffer() );
	strcat( buffer, "sound/" );

	if ( access( buffer, R_OK ) != 0 ) {
		// just go to fsmain
		strcpy( buffer, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buffer, "/" );
	}

	const char *filename = file_dialog( g_pGroupDlg->m_pWidget, TRUE, _( "Open Wav File" ), buffer, "sound" );
	if ( filename != NULL ) {
		gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), "noise" );
		char *aux = vfsExtractRelativePath( filename );
		CString str;
		if ( aux ) {
			str = aux;
		}
		else
		{
			Sys_FPrintf( SYS_WRN, "WARNING: could not extract the relative path, using full path instead\n" );
			str = filename;
		}

		gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), str.GetBuffer() );
		AddProp();
	}
}

void AssignModel(){
	char buffer[NAME_MAX];

	strcpy( buffer, g_qeglobals.m_strHomeMaps.GetBuffer() );
	strcat( buffer, "models/" );

	if ( access( buffer, R_OK ) != 0 ) {
		// just go to fsmain
		strcpy( buffer, g_qeglobals.m_strHomeMaps.GetBuffer() );
		strcat( buffer, "/" );
	}

	const char *filename = file_dialog( g_pGroupDlg->m_pWidget, TRUE, _( "Open Model" ), buffer, MODEL_MAJOR );
	if ( filename != NULL ) {
		gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), "model" );
		// use VFS to get the correct relative path
		char *aux = vfsExtractRelativePath( filename );
		CString str;
		if ( aux ) {
			str = aux;
		}
		else
		{
			Sys_FPrintf( SYS_WRN, "WARNING: could not extract the relative path, using full path instead\n" );
			str = filename;
		}

		gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), str.GetBuffer() );
		AddProp();
		edit_entity->brushes.onext->bModelFailed = false;
	}
}

void cam2angles()
{
	Str value;
	camera_t *cam;

	cam = g_pParentWnd->GetCamWnd()->Camera();
	//pitch yaw roll
	value.Format( "%g %g %g", cam->angles[0], cam->angles[1], cam->angles[2] );

	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), "angles" );
	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), value.GetBuffer() );
	AddProp();
}

/*
   ==============
   SetInspectorMode
   ==============
 */
void SetInspectorMode( int iType ){
	if ( iType == W_GROUP ) {
		gtk_MessageBox( g_pParentWnd->m_pWidget, _( "Brush grouping is not functional yet" ), NULL, MB_OK | MB_ICONWARNING );
	}

	if ( !g_pParentWnd->FloatingGroupDialog() &&
		 ( iType == W_TEXTURE || iType == W_CONSOLE ) ) {
		return;
	}

	// Is the caller asking us to cycle to the next window?
	if ( iType == -1 ) {
		if ( inspector_mode == W_ENTITY ) {
			iType = W_TEXTURE;
		}
		else if ( inspector_mode == W_TEXTURE ) {
			iType = W_CONSOLE;
		}
		else if ( inspector_mode == W_CONSOLE ) {
			iType = W_GROUP;
		}
		else{
			iType = W_ENTITY;
		}
	}

	switch ( iType ) {
	case W_ENTITY:
		// entity is always first in the inspector
		gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), _( "Entities" ) );
		gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 0 );
		break;

	case W_TEXTURE:
		g_pParentWnd->GetTexWnd()->FocusEdit();
		gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), _( "Textures" ) );
		if ( g_pParentWnd->FloatingGroupDialog() ) {
                  // if the notebook page is already at 1, no expose event fires up on the embedded GLWindow, leading in the texture window not drawing
                  // I did witness an expose event on the notebook widget though, but for some reason it's not traveling down..
                  // so when hiding the group dialog, we are setting the page to 0, the page switch does an expose and triggers drawing.. (see OnDialogKey)
                  gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 1 );
                  // FIXME: https://github.com/TTimo/GtkRadiant/issues/192
//                  Sys_Printf( "show widget: set page to 1\n" );
		}
		break;

	case W_CONSOLE:
		gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), _( "Console" ) );
		if ( g_pParentWnd->FloatingGroupDialog() ) {
			gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 2 );
		}
		break;

	case W_GROUP:
		if ( g_pParentWnd->FloatingGroupDialog() ) {
			gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 3 );
		} else {
			gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 1 );
		}
		break;

	default:
		break;
	}
}

void Group_Add( entity_t *e ){
	/*
	   group_t *g = (group_t*)qmalloc(sizeof(group_t));
	   g->epairs = e->epairs;
	   g->next = NULL;
	   e->epairs = NULL;

	   // create a new group node
	   char *text = ValueForKey(g->epairs, "group");
	   g->itemOwner = gtk_ctree_insert_node (GTK_CTREE (g_wndGroup.m_pTree), g_wndGroup.m_hWorld, NULL, &text, 0,
	                  tree_pixmaps[IMG_GROUP], tree_masks[IMG_GROUP],
	                  tree_pixmaps[IMG_GROUP], tree_masks[IMG_GROUP], TRUE, TRUE);
	   g->next = g_pGroups;
	   g_pGroups = g;
	 */
}
/*
   group_t* Group_Alloc(char *name)
   {
   group_t *g = (group_t*)qmalloc(sizeof(group_t));
   SetKeyValue( g->epairs, "group", name );
   return g;
   }

   group_t* Group_ForName(const char * name)
   {
   group_t *g = g_pGroups;
   while (g != NULL)
   {
    if (strcmp( ValueForKey(g->epairs,"group"), name ) == 0)
      break;
    g = g->next;
   }
   return g;
   }

   void Group_AddToItem(brush_t *b, GtkCTreeNode* item)
   {
   int nImage = IMG_BRUSH;
   if (!g_qeglobals.m_bBrushPrimitMode)
   {
    return;
   }
   const char *pName = NULL;
   //  const char *pNamed = Brush_GetKeyValue(b, "name");

   if (!b->owner || (b->owner == world_entity))
   {
    if (b->patchBrush)
    {
      pName = "Generic Patch";
      nImage = IMG_PATCH;
    }
    else
    {
      pName = "Generic Brush";
      nImage = IMG_BRUSH;
    }
   }
   else
   {
    pName = b->owner->eclass->name;
    if (b->owner->eclass->fixedsize)
    {
      nImage = IMG_ENTITY;
    }
    else
    {
      nImage = IMG_ENTITYGROUP;
    }
   }

   GtkCTreeNode *newItem;
   int i = (b->patchBrush) ? IMG_PATCH : IMG_BRUSH;
   newItem = gtk_ctree_insert_node (GTK_CTREE (g_wndGroup.m_pTree), item, NULL, (gchar**)&pName, 0,
                   tree_pixmaps[i], tree_masks[i], tree_pixmaps[i],
                   tree_masks[i], TRUE, TRUE);
   gtk_ctree_node_set_row_data (GTK_CTREE (g_wndGroup.m_pTree), newItem, b);
   b->itemOwner = newItem;
   }
 */
void Group_RemoveBrush( brush_t *b ){
	/*
	   if (!g_qeglobals.m_bBrushPrimitMode)
	   {
	   return;
	   }
	   if (b->itemOwner)
	   {
	   gtk_ctree_remove_node (GTK_CTREE (g_pGroupDlg->m_pTree), b->itemOwner);
	   b->itemOwner = NULL;
	   }
	   DeleteKey(b->epairs, "group");
	 */
}
/*
   void Group_AddToWorld(brush_t *b)
   {
   if (!g_qeglobals.m_bBrushPrimitMode)
   {
    return;
   }
   GtkCTreeNode *parent = gtk_ctree_node_nth (GTK_CTREE (g_pGroupDlg->m_pTree), 0);
   Group_AddToItem(b, parent);
   }
 */
void Group_AddToProperGroup( brush_t *b ){
	/*
	   if (!g_qeglobals.m_bBrushPrimitMode)
	   {
	   return;
	   }

	   // NOTE: we do a local copy of the "group" key because it gets erased by Group_RemoveBrush
	   const char *pGroup = Brush_GetKeyValue(b, "group");
	   // remove the entry in the tree if there's one
	   if (b->itemOwner)
	   {
	   gtk_ctree_remove_node (GTK_CTREE (g_pGroupDlg->m_pTree), b->itemOwner);
	   b->itemOwner = NULL;
	   }

	   if (*pGroup != 0)
	   {
	   // find the item
	   group_t *g = Group_ForName(pGroup);
	   if (g)
	    Group_AddToItem(b, g->itemOwner);
	   #ifdef _DEBUG
	   else
	    Sys_FPrintf(SYS_WRN, "WARNING: unexpected Group_ForName not found in Group_AddToProperGroup\n");
	   #endif
	   }
	   else
	   {
	   Group_AddToWorld(b);
	   }
	 */
}
/*
   void Group_AddToSelected(brush_t *b)
   {
   if (!g_qeglobals.m_bBrushPrimitMode)
   {
    return;
   }
   GtkCTreeNode *item;
   item = gtk_ctree_node_nth (GTK_CTREE (g_pGroupDlg->m_pTree), GTK_CLIST (g_pGroupDlg->m_pTree)->focus_row);
   if (item == NULL)
   {
    item = gtk_ctree_node_nth (GTK_CTREE (g_pGroupDlg->m_pTree), 0);
   }
   Group_AddToItem(b, item);
   }
 */
/*
   void Group_Save(FILE *f)
   {
   group_t *g = g_pGroups;
   while (g)
   {
    fprintf(f,"{\n\"classname\" \"group_info\"\n\"group\" \"%s\"\n}\n", ValueForKey( g->epairs, "group" ));
    g = g->next;
   }
   }
 */

void Group_Init(){
	if ( !g_qeglobals.m_bBrushPrimitMode ) {
		return;
	}
	// start by cleaning everything
	// clean the groups
	//++timo FIXME: we leak, delete the groups on the way (I don't have time to do it now)
#ifdef _DEBUG
	Sys_Printf( "TODO: fix leak in Group_Init\n" );
#endif
	group_t *g = g_pGroups;
	while ( g )
	{
		epair_t *ep,*enext;
		for ( ep = g->epairs ; ep ; ep = enext )
		{
			enext = ep->next;
			free( ep->key );
			free( ep->value );
			free( ep );
		}
		g = g->next;
	}
	/*
	   GtkCTreeNode *world;
	   char *text = "World";
	   g_pGroups = NULL;
	   gtk_clist_clear (GTK_CLIST (g_wndGroup.m_pTree));
	   world = gtk_ctree_insert_node (GTK_CTREE (g_wndGroup.m_pTree), NULL, NULL, &text, 0,
	               tree_pixmaps[IMG_GROUP], tree_masks[IMG_GROUP], tree_pixmaps[IMG_GROUP],
	               tree_masks[IMG_GROUP], FALSE, TRUE);
	 */
	// walk through all the brushes, remove the itemOwner key and add them back where they belong
	brush_t *b;
	for ( b = active_brushes.next; b != &active_brushes; b = b->next )
	{
		b->itemOwner = NULL;
		Group_AddToProperGroup( b );
	}
	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		b->itemOwner = NULL;
		Group_AddToProperGroup( b );
	}
}
/*
   // scan through world_entity for groups in this map?
   // we use GROUPNAME "QER_group_%i" to look for existing groups and their naming
   //++timo FIXME: is this actually needed for anything?
   void Group_GetListFromWorld(GSList **pArray)
   {
   if (!g_qeglobals.m_bBrushPrimitMode)
   {
    return;
   }

   if (world_entity == NULL)
   {
    return;
   }

   char cBuff[1024];
   for (int i =0; i < MAX_GROUPS; i++)
   {
    sprintf(cBuff, GROUPNAME, i);
    char *pGroup = ValueForKey(world_entity, cBuff);
    if (pGroup && strlen(pGroup) > 0)
    {
   *pArray = g_slist_append (*pArray, g_strdup (pGroup));
    }
    else
    {
      break;
    }
   }
   }

   void Group_RemoveListFromWorld()
   {
   if (!g_qeglobals.m_bBrushPrimitMode)
   {
    return;
   }
   GSList* array = NULL;
   Group_GetListFromWorld(&array);

   while (array)
   {
    DeleteKey(world_entity, (char*)array->data);
    g_free (array->data);
    array = g_slist_remove (array, array->data);
   }
   }

   int CountChar(const char *p, char c)
   {
   int nCount = 0;
   int nLen = strlen(p)-1;
   while (nLen-- >= 0)
   {
    if (p[nLen] == c)
    {
      nCount++;
    }
   }
   return nCount;
   }
 */
// =============================================================================
// callbacks

static void eclasslist_selection_changed( GtkTreeSelection* selection, gpointer data ){
	GtkTreeModel* model;
	GtkTreeIter selected;
	// no world entity, we are not ready yet
	if ( !world_entity ) {
		return;
	}
	if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		eclass_t* eclass;
		gtk_tree_model_get( model, &selected, 1, &eclass, -1 );
		if ( eclass != NULL ) {
			GtkTreePath* path = gtk_tree_model_get_path( model, &selected );
			UpdateSel( gtk_tree_path_get_indices( path )[0], eclass );
			gtk_tree_path_free( path );
		}
	}
}

static gint eclasslist_button_press( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	if ( event->type == GDK_2BUTTON_PRESS ) {
		CreateEntity();
		return TRUE;
	}
	return FALSE;
}

static gint eclasslist_keypress( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	unsigned int code = gdk_keyval_to_upper( event->keyval );

	if ( event->keyval == GDK_KEY_Return ) {
		CreateEntity();
		return TRUE;
	}

	// select the entity that starts with the key pressed
	if ( code <= 'Z' && code >= 'A' ) {
		GtkTreeView* view = GTK_TREE_VIEW( EntWidgets[EntList] );
		GtkTreeModel* model;
		GtkTreeIter iter;
		if ( gtk_tree_selection_get_selected( gtk_tree_view_get_selection( view ), &model, &iter ) == FALSE
			 || gtk_tree_model_iter_next( model, &iter ) == FALSE ) {
			gtk_tree_model_get_iter_first( model, &iter );
		}

		for ( unsigned int count = gtk_tree_model_iter_n_children( model, NULL ); count > 0; --count )
		{
			char* text;
			gtk_tree_model_get( model, &iter, 0, &text, -1 );

			if ( toupper( text[0] ) == (int)code ) {
				GtkTreePath* path = gtk_tree_model_get_path( model, &iter );
				gtk_tree_selection_select_path( gtk_tree_view_get_selection( view ), path );
				gtk_tree_view_scroll_to_cell( view, path, NULL, FALSE, 0, 0 );
				gtk_tree_path_free( path );
				count = 1;
			}

			g_free( text );

			if ( gtk_tree_model_iter_next( model, &iter ) == FALSE ) {
				gtk_tree_model_get_iter_first( model, &iter );
			}
		}

		return TRUE;
	}
	return FALSE;
}

static void proplist_selection_changed( GtkTreeSelection* selection, gpointer data ){
	// find out what type of entity we are trying to create
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkWidget *dialog, *del_button;
	gboolean selected;

	dialog = GTK_WIDGET( data );
	del_button = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "del_button" ) );

	selected = gtk_tree_selection_get_selected( selection, &model, &iter );

	gtk_widget_set_sensitive( del_button, selected );

	if ( selected == FALSE ) {
		return;
	}

	char* key;
	char* val;
	gtk_tree_model_get( model, &iter, 0, &key, 1, &val, -1 );

	if ( stricmp( key, "classname" ) == 0 ) {
		gtk_widget_set_sensitive( del_button, FALSE );
	}

	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntKeyField] ), key );
	gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), val );

	g_free( key );
	g_free( val );
}
static void proplist_view_realize( GtkWidget *widget, gpointer data )
{
	proplist_selection_changed( gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) ), data );
}

static void entity_check( GtkWidget *widget, gpointer data ){
	if ( !disable_spawn_get ) {
		GetSpawnFlags();
	}
}

static void entitylist_angle( GtkWidget *widget, gpointer data ){
	SetKeyValue( edit_entity, "angle", (char*)data );
	SetKeyValuePairs();
}

static gint entityentry_keypress( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	if ( event->keyval == GDK_KEY_Tab ) {
		if ( widget == EntWidgets[EntKeyField] ) {
			//gtk_entry_set_text (GTK_ENTRY (EntWidgets[EntValueField]), "");
			gtk_window_set_focus( GTK_WINDOW( g_pGroupDlg->m_pWidget ), EntWidgets[EntValueField] );
		}
		else{
			gtk_window_set_focus( GTK_WINDOW( g_pGroupDlg->m_pWidget ), EntWidgets[EntKeyField] );
		}

		return TRUE;
	}
	else if ( event->keyval == GDK_KEY_Return ) {
		if ( widget == EntWidgets[EntKeyField] ) {
			gtk_entry_set_text( GTK_ENTRY( EntWidgets[EntValueField] ), "" );
			gtk_window_set_focus( GTK_WINDOW( g_pGroupDlg->m_pWidget ), EntWidgets[EntValueField] );
		}
		else
		{
			AddProp();
		}
		return TRUE;
	}

	return FALSE;
}

static void switch_page( GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer data ){
	const gchar *text;
	text = gtk_label_get_text( GTK_LABEL( gtk_notebook_get_tab_label( notebook, gtk_notebook_get_nth_page( notebook, page_num ) ) ) );
	gtk_window_set_title( GTK_WINDOW( data ), text );

	gpointer item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_misc_selectentitycolor" );

	if ( g_pParentWnd->FloatingGroupDialog() ) {
		switch ( page_num )
		{
		case 0: inspector_mode = W_ENTITY; break;
		case 1: inspector_mode = W_TEXTURE; break;
		case 2: inspector_mode = W_CONSOLE; break;
		default: inspector_mode = W_GROUP; break;
		}
	}
	else
	{
		if ( page_num == 0 ) {
			inspector_mode = W_ENTITY;
		}
		else{
			inspector_mode = W_GROUP;
		}
	}

	if ( inspector_mode == W_ENTITY ) {
		gtk_widget_set_sensitive( GTK_WIDGET( item ), TRUE );
	}
	else{
		gtk_widget_set_sensitive( GTK_WIDGET( item ), FALSE );
	}
}

// =============================================================================
// GroupDlg class

static gint OnDeleteHide( GtkWidget *widget ) {
  gtk_widget_hide( widget );
  // see OnDialogKey
  gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 0 );
  return TRUE;
}

// NOTE: when a key is hit with group window focused, we catch in this handler but it gets propagated to mainframe too
//   therefore the message will be intercepted and used as a ID_SELECTION_DESELECT
static gint OnDialogKey( GtkWidget* widget, GdkEventKey* event, gpointer data ) {
  // make the "ViewTextures" and "ViewEntityInfo" keys that normally bring this dialog up hide it as well - copypasta from mainframe_keypress
  // NOTE: maybe we could also check the state of the notebook, see if those are actually displayed .. if they are not, then switch the notebook pages rather than hiding?
  bool hide = false;
  // Disable this, it makes the dialog hide whenever you type 'T' when editing entities etc. - Esc key is enough
#if 0
  unsigned int code = gdk_keyval_to_upper( event->keyval );
  for ( int i = 0; i < g_nCommandCount; i++ ) {
    if ( g_Commands[i].m_nKey == code ) { // find a match?
      // check modifiers
      unsigned int nState = 0;
      if ( Sys_AltDown() ) {
        nState |= RAD_ALT;
      }
      if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
        nState |= RAD_CONTROL;
      }
      if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
        nState |= RAD_SHIFT;
      }
      if ( ( g_Commands[i].m_nModifiers & 0x7 ) == nState ) {
        Sys_Printf( "Floating group dialog: %s\n", g_Commands[i].m_strCommand );
        if ( g_Commands[i].m_nCommand == ID_VIEW_TEXTURE || g_Commands[i].m_nCommand == ID_VIEW_ENTITY ) {
          hide = true;
        }
        break;
      }
    }
  }
#endif
  if ( g_pParentWnd->CurrentStyle() != MainFrame::eFloating && ( hide || event->keyval == GDK_KEY_Escape ) ) {
    // toggle off the group view (whatever part of it is currently displayed)
    // this used to be done with a g_pParentWnd->OnViewEntity(); but it had bad consequences
    gtk_widget_hide( widget );
    // set the group notebook page back to 0, so that when we recall the texture view there is an expose event coming up
    gtk_notebook_set_current_page( GTK_NOTEBOOK( g_pGroupDlg->m_pNotebook ), 0 );
    // FIXME: https://github.com/TTimo/GtkRadiant/issues/192
//    Sys_Printf( "hide widget and set page to 0\n" );
    return TRUE;
  }
  return FALSE;
}

GroupDlg::GroupDlg (){
	m_pWidget = NULL;
}

#ifdef _WIN32
extern void PositionWindowOnPrimaryScreen( window_position_t& position );
#endif

void GroupDlg::Create(){
	GtkWidget *dialog, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( m_pWidget != NULL ) {
		return;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Entities/Entity View" ), NULL, flags, NULL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

#ifdef _WIN32
	if ( g_PrefsDlg.m_bStartOnPrimMon ) {
		PositionWindowOnPrimaryScreen( g_PrefsDlg.mWindowInfo.posEntityWnd );
	}
#endif
	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posEntityWnd );

	g_signal_connect( G_OBJECT( dialog ), "delete-event", G_CALLBACK( OnDeleteHide ), NULL );
	// catch 'Esc'
	g_signal_connect( G_OBJECT( dialog ), "key-press-event", G_CALLBACK( OnDialogKey ), NULL );

	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	g_qeglobals_gui.d_entity = dialog;

	{
		GtkWidget* notebook = gtk_notebook_new();
		gtk_container_add( GTK_CONTAINER( content_area ), notebook );
		gtk_notebook_set_tab_pos( GTK_NOTEBOOK( notebook ), GTK_POS_BOTTOM );
		gtk_widget_show( notebook );
		m_pNotebook = notebook;

		{
			GtkWidget* vbox = gtk_vbox_new( FALSE, 2 );
			gtk_container_set_border_width( GTK_CONTAINER( vbox ), 2 );
			gtk_widget_show( vbox );

			{
				GtkWidget* label = gtk_label_new( _( "Entities" ) );
				gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), vbox, label );
				gtk_widget_show( label );
			}

			{
				GtkWidget* split1 = gtk_vpaned_new();
				gtk_box_pack_start( GTK_BOX( vbox ), split1, TRUE, TRUE, 0 );
				gtk_widget_show( split1 );

				{
					GtkWidget* split2 = gtk_vpaned_new();
					gtk_paned_pack1( GTK_PANED( split1 ), split2, TRUE, FALSE );
					gtk_widget_show( split2 );

					g_object_set_data( G_OBJECT( dialog ), "split1", split1 );
					g_object_set_data( G_OBJECT( dialog ), "split2", split2 );

					{
						GtkWidget* vbox2 = gtk_vbox_new( FALSE, 2 );
						gtk_paned_pack2( GTK_PANED( split1 ), vbox2, TRUE, FALSE );
						gtk_widget_show( vbox2 );

						{
							GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
							gtk_paned_pack1( GTK_PANED( split2 ), scr, TRUE, FALSE );
							gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
							gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
							gtk_widget_show( scr );

							{
								GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_POINTER );

								GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
								gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );
								g_signal_connect( G_OBJECT( view ), "button-press-event", G_CALLBACK( eclasslist_button_press ), NULL );
								g_signal_connect( G_OBJECT( view ), "key-press-event", G_CALLBACK( eclasslist_keypress ), this );

								{
									GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
									GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "Key", renderer, "text", 0, (char *) NULL );
									gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
								}

								{
									GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
									g_signal_connect( G_OBJECT( selection ), "changed", G_CALLBACK( eclasslist_selection_changed ), dialog );
								}

								gtk_container_add( GTK_CONTAINER( scr ), view );

								gtk_widget_show( view );

								g_object_unref( G_OBJECT( store ) );
								EntWidgets[EntList] = view;
							}
						}

						{
							GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
							gtk_paned_pack2( GTK_PANED( split2 ), scr, TRUE, FALSE );
							gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
							gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
							gtk_widget_show( scr );

							{
								GtkWidget* text = gtk_text_view_new();
								gtk_widget_set_size_request( text, 0, -1 ); // allow shrinking
								gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( text ), GTK_WRAP_WORD );
								gtk_text_view_set_editable( GTK_TEXT_VIEW( text ), FALSE );
								gtk_container_add( GTK_CONTAINER( scr ), text );
								gtk_widget_show( text );
								EntWidgets[EntComment] = text;
							}
						}

						{
							// Spawnflags (4 colums wide max, or window gets too wide.)
							LayoutTable = gtk_table_new( 4, 4, FALSE );
							gtk_box_pack_start( GTK_BOX( vbox2 ), LayoutTable, FALSE, TRUE, 0 );
							gtk_widget_show( LayoutTable );

							for ( int i = 0; i < MAX_FLAGS; i++ )
							{
								GtkWidget* check = gtk_check_button_new_with_label( "" );
								g_object_ref( check );
								g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( entity_check ), NULL );
								EntWidgets[EntCheck1 + i] = check;
							}

						}

						{
							GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
							gtk_box_pack_start( GTK_BOX( vbox2 ), scr, TRUE, TRUE, 0 );
							gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
							gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
							gtk_widget_show( scr );

							{
								GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

								GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
								gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

								{
									GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
									GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
									gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
								}

								{
									GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
									GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 1, (char *) NULL );
									gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
								}

								{
									GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
									g_signal_connect( G_OBJECT( selection ), "changed", G_CALLBACK( proplist_selection_changed ), dialog );
									g_signal_connect( G_OBJECT( view ), "realize", G_CALLBACK( proplist_view_realize ), dialog );
								}

								gtk_container_add( GTK_CONTAINER( scr ), view );

								gtk_widget_show( view );
								g_object_unref( G_OBJECT( store ) );

								EntWidgets[EntProps] = view;
							}
						}
					}

					int x = g_PrefsDlg.mWindowInfo.nEntitySplit1;
					if ( x != -1 ) {
						gtk_paned_set_position( GTK_PANED( split1 ), x );

						while ( gtk_events_pending() ) gtk_main_iteration();
						x = g_PrefsDlg.mWindowInfo.nEntitySplit2;

						if ( x != -1 ) {
							gtk_paned_set_position( GTK_PANED( split2 ), x );
						}
					}
				}
			}

			{
				GtkWidget* table = gtk_table_new( 2, 2, FALSE );
				gtk_box_pack_start( GTK_BOX( vbox ), table, FALSE, TRUE, 0 );
				gtk_table_set_row_spacings( GTK_TABLE( table ), 3 );
				gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
				gtk_widget_show( table );

				{
					GtkWidget* entry = gtk_entry_new();
					gtk_table_attach( GTK_TABLE( table ), entry, 1, 2, 0, 1,
									  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
									  (GtkAttachOptions) ( 0 ), 0, 0 );
					gtk_widget_show( entry );
					gtk_widget_set_events( entry, GDK_KEY_PRESS_MASK );
					g_signal_connect( G_OBJECT( entry ), "key-press-event",
										G_CALLBACK( entityentry_keypress ), this );
					EntWidgets[EntKeyField] = entry;
				}

				{
					GtkWidget* entry = gtk_entry_new();
					gtk_table_attach( GTK_TABLE( table ), entry, 1, 2, 1, 2,
									  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
									  (GtkAttachOptions) ( 0 ), 0, 0 );
					gtk_widget_show( entry );
					gtk_widget_set_events( entry, GDK_KEY_PRESS_MASK );
					g_signal_connect( G_OBJECT( entry ), "key-press-event",
										G_CALLBACK( entityentry_keypress ), this );
					EntWidgets[EntValueField] = entry;
				}

				{
					GtkWidget* label = gtk_label_new( _( "Value" ) );
					gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
									  (GtkAttachOptions) ( GTK_FILL ),
									  (GtkAttachOptions) ( 0 ), 0, 0 );
					gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
					gtk_widget_show( label );
				}

				{
					GtkWidget* label = gtk_label_new( _( "Key" ) );
					gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
									  (GtkAttachOptions) ( GTK_FILL ),
									  (GtkAttachOptions) ( 0 ), 0, 0 );
					gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
					gtk_widget_show( label );
				}
			}

			{
				GtkWidget* hbox = gtk_hbox_new( FALSE, 5 );
				gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
				gtk_widget_show( hbox );

				{
					GtkWidget* table = gtk_table_new( 3, 3, TRUE );
					gtk_box_pack_start( GTK_BOX( hbox ), table, FALSE, TRUE, 0 );
					gtk_widget_show( table );

					{
						GtkWidget* button = gtk_button_new_with_label( _( "360" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 1, 2,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"360" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "45" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 0, 1,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"45" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "90" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 0, 1,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"90" );
					}


					{
						GtkWidget* button = gtk_button_new_with_label( _( "135" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 0, 1,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"135" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "180" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 1, 2,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"180" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "225" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 2, 3,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"225" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "270" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 2, 3,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"270" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "315" ) );
						gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 2, 3,
										  (GtkAttachOptions) ( GTK_FILL ),
										  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"315" );
					}
				}

				{
					GtkWidget* vbox2 = gtk_vbox_new( FALSE, 0 );
					gtk_box_pack_start( GTK_BOX( hbox ), vbox2, TRUE, TRUE, 0 );
					gtk_widget_show( vbox2 );

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Reset" ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( ResetEntity ), NULL );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Up" ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"-1" );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Dn" ) );

						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( entitylist_angle ), (void *)"-2" );
					}
				}

				{
					GtkWidget* vbox2 = gtk_vbox_new( FALSE, 0 );
					gtk_box_pack_start( GTK_BOX( hbox ), vbox2, TRUE, TRUE, 0 );
					gtk_widget_show( vbox2 );

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Del Key/Pair" ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( DelProp ), NULL );
						g_object_set_data( G_OBJECT( dialog ), "del_button", button );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Sound..." ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( AssignSound ), NULL );
					}

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Model..." ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( AssignModel ), NULL );
					}
				}

				{
					GtkWidget* vbox2 = gtk_vbox_new( FALSE, 0 );
					gtk_box_pack_start( GTK_BOX( hbox ), vbox2, TRUE, TRUE, 0 );
					gtk_widget_show( vbox2 );

					{
						GtkWidget* button = gtk_button_new_with_label( _( "Cam to angles" ) );
						gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
						gtk_widget_show( button );
						g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( cam2angles ), NULL );

					}
				}
			}
		}

		if ( g_pParentWnd->FloatingGroupDialog() ) {
			{
				GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
				gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
				gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
				gtk_container_set_border_width( GTK_CONTAINER( scr ), 3 );
				gtk_widget_show( scr );

				{
					GtkWidget* text = gtk_text_view_new();
					gtk_widget_set_size_request( text, 0, -1 ); // allow shrinking
					gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( text ), GTK_WRAP_WORD );
					gtk_text_view_set_editable( GTK_TEXT_VIEW( text ), FALSE );
					gtk_container_add( GTK_CONTAINER( scr ), text );
					gtk_widget_show( text );
					g_qeglobals_gui.d_edit = text;
				}

				{
					GtkWidget* label = gtk_label_new( _( "Console" ) );
					gtk_widget_show( label );
					gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), scr, label );
				}
			}
		}

		inspector_mode = W_ENTITY;
		m_pWidget = dialog;
		g_signal_connect( G_OBJECT( notebook ), "switch-page", G_CALLBACK( switch_page ), dialog );
	}
}
