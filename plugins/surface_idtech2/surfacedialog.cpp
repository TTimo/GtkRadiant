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
// Surface Dialog Module
//

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include "surfacedialog.h"
#include "surfaceflags.h"

#ifdef _DEBUG
//#define DBG_SI 1
#endif

#include "gtkr_vector.h"

vector<texdef_to_face_t> g_texdef_face_vector;

inline texdef_to_face_t* get_texdef_face_list(){
	return &( *g_texdef_face_vector.begin() );
}

inline unsigned int texdef_face_list_empty(){
	return g_texdef_face_vector.empty();
}

inline unsigned int texdef_face_list_size(){
	return g_texdef_face_vector.size();
}

// For different faces having different values
bool is_HShift_conflicting;
bool is_VShift_conflicting;
bool is_HScale_conflicting;
bool is_VScale_conflicting;
bool is_Rotate_conflicting;
bool is_TextureName_conflicting;

// Dialog Data
int m_nHeight;
int m_nWidth;

// 0 is invalid, otherwise it's the ID of the last 'do' we are responsible for
int m_nUndoId;

texturewin_t *texturewin;
texdef_t *l_pIncrement;
texdef_t texdef_SI_values;

// For Texture Entry, activate only on entry change
char old_texture_entry[128];

// when TRUE, this thing means the surface inspector is currently being displayed
bool g_surfwin = FALSE;
// turn on/off processing of the "changed" "value-changed" messages
// (need to turn off when we are feeding data in)
bool g_bListenChanged = TRUE;
// turn on/off listening of the update messages
bool g_bListenUpdate = TRUE;

extern void *g_pMainWidget;

GtkWidget* create_SurfaceInspector( void );
GtkWidget *SurfaceInspector = NULL;

GtkWidget *GetWidget() { return SurfaceInspector; }
GtkWidget *Get_SI_Module_Widget() { return SurfaceInspector; }
void SetWidget( GtkWidget *new_widget ) { }
GtkWidget *GetDlgWidget( const char* name ) { return GTK_WIDGET( g_object_get_data( G_OBJECT( SurfaceInspector ), name ) ); }

GtkWidget *texture_combo;
GtkWidget *texture_combo_entry;

GtkWidget *match_grid_button;

GtkWidget *hshift_value_spinbutton;
GtkWidget *vshift_value_spinbutton;
GtkWidget *hscale_value_spinbutton;
GtkWidget *vscale_value_spinbutton;
GtkWidget *rotate_value_spinbutton;

GtkWidget *hshift_step_spinbutton;
GtkWidget *vshift_step_spinbutton;
GtkWidget *hscale_step_spinbutton;
GtkWidget *vscale_step_spinbutton;
GtkWidget *rotate_step_spinbutton;

GtkWidget *fit_width_spinbutton;
GtkWidget *fit_height_spinbutton;
GtkWidget *fit_button;
GtkWidget *axial_button;

// Callbacks
gboolean on_texture_combo_entry_key_press_event( GtkWidget *widget, GdkEventKey *event, gpointer user_data );
void on_texture_combo_entry_activate( GtkEntry *entry, gpointer user_data );

static void on_match_grid_button_clicked( GtkButton *button, gpointer user_data );

static void on_hshift_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_vshift_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_hscale_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_vscale_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_rotate_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );

static void on_hshift_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_vshift_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_hscale_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_vscale_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_rotate_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );

static void on_fit_width_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_fit_height_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data );
static void on_fit_button_clicked( GtkButton *button, gpointer user_data );
static void on_axial_button_clicked( GtkButton *button, gpointer user_data );


/*
   ===================================================

   SURFACE INSPECTOR

   ===================================================
 */


void IsFaceConflicting(){
	texdef_t* tmp_texdef;
	texdef_to_face_t* temp_texdef_face_list;
	char texture_name[128];

	if ( texdef_face_list_empty() ) {
		gtk_entry_set_text( GTK_ENTRY( hshift_value_spinbutton ), "" );
		gtk_entry_set_text( GTK_ENTRY( vshift_value_spinbutton ), "" );
		gtk_entry_set_text( GTK_ENTRY( hscale_value_spinbutton ), "" );
		gtk_entry_set_text( GTK_ENTRY( vscale_value_spinbutton ), "" );
		gtk_entry_set_text( GTK_ENTRY( rotate_value_spinbutton ), "" );
		gtk_entry_set_text( GTK_ENTRY( texture_combo_entry ), "" );
		return;
	}

	g_bListenChanged = FALSE;

	tmp_texdef = &get_texdef_face_list()->texdef;

	strcpy( texture_name, tmp_texdef->GetName() );

	texdef_SI_values.shift[0] = tmp_texdef->shift[0];
	texdef_SI_values.shift[1] = tmp_texdef->shift[1];
	texdef_SI_values.scale[0] = tmp_texdef->scale[0];
	texdef_SI_values.scale[1] = tmp_texdef->scale[1];
	texdef_SI_values.rotate = tmp_texdef->rotate;
	texdef_SI_values.SetName( texture_name );

	is_HShift_conflicting = FALSE;
	is_VShift_conflicting = FALSE;
	is_HScale_conflicting = FALSE;
	is_VScale_conflicting = FALSE;
	is_Rotate_conflicting = FALSE;
	is_TextureName_conflicting = FALSE;

	if ( texdef_face_list_size() > 1 ) {
		temp_texdef_face_list = get_texdef_face_list()->next;

		for (; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = &temp_texdef_face_list->texdef;
			if ( texdef_SI_values.shift[0] != tmp_texdef->shift[0] ) {
				is_HShift_conflicting = TRUE;
			}

			if ( texdef_SI_values.shift[1] != tmp_texdef->shift[1] ) {
				is_VShift_conflicting = TRUE;
			}

			if ( texdef_SI_values.scale[0] != tmp_texdef->scale[0] ) {
				is_HScale_conflicting = TRUE;
			}

			if ( texdef_SI_values.scale[1] != tmp_texdef->scale[1] ) {
				is_VScale_conflicting = TRUE;
			}

			if ( texdef_SI_values.rotate != tmp_texdef->rotate ) {
				is_Rotate_conflicting = TRUE;
			}

			if ( strcmp( texture_name, tmp_texdef->GetName() ) ) {
				is_TextureName_conflicting = TRUE;
			}
		}
	}

	if ( is_HShift_conflicting ) {
		gtk_entry_set_text( GTK_ENTRY( hshift_value_spinbutton ), "" );
	}
	else{
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( hshift_value_spinbutton ), texdef_SI_values.shift[0] );
	}

	if ( is_VShift_conflicting ) {
		gtk_entry_set_text( GTK_ENTRY( vshift_value_spinbutton ), "" );
	}
	else{
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( vshift_value_spinbutton ), texdef_SI_values.shift[1] );
	}

	if ( is_HScale_conflicting ) {
		gtk_entry_set_text( GTK_ENTRY( hscale_value_spinbutton ), "" );
	}
	else{
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( hscale_value_spinbutton ), texdef_SI_values.scale[0] );
	}

	if ( is_VScale_conflicting ) {
		gtk_entry_set_text( GTK_ENTRY( vscale_value_spinbutton ), "" );
	}
	else{
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( vscale_value_spinbutton ), texdef_SI_values.scale[1] );
	}

	if ( is_Rotate_conflicting ) {
		gtk_entry_set_text( GTK_ENTRY( rotate_value_spinbutton ), "" );
	}
	else{
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( rotate_value_spinbutton ), texdef_SI_values.rotate );
	}

	g_bListenChanged = TRUE;
}

#define MAX_NUM_LIST_ITEMS 15
static void PopulateTextureComboList(){
	texdef_t* tmp_texdef;
	texdef_to_face_t* temp_texdef_face_list;
	char blank[1];
	GList *items = NULL;
	GList *lst;
	int num_of_list_items = 0;

	blank[0] = 0;

	//clear combo box
#if GTK_CHECK_VERSION( 3, 0, 0 )
	gtk_combo_box_text_remove_all( GTK_COMBO_BOX_TEXT( texture_combo ) );
#else
	GtkListStore *store;
	store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( texture_combo ) ) );
	gtk_list_store_clear( store );
#endif

	if ( texdef_face_list_empty() ) {
		items = g_list_append( items, (gpointer) blank );
		// For Texture Entry, activate only on entry change
		strcpy( old_texture_entry, blank );
	}
	else if ( !is_TextureName_conflicting ) {
		temp_texdef_face_list = get_texdef_face_list();
		tmp_texdef = (texdef_t *) &get_texdef_face_list()->texdef;
		items = g_list_append( items, (gpointer) tmp_texdef->GetName() );
		// For Texture Entry, activate only on entry change
		strcpy( old_texture_entry, tmp_texdef->GetName() );
	}
	else
	{
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			// Need to do a string compare, hence the custom search
			if ( !( g_list_find_custom( items, tmp_texdef->GetName(), (GCompareFunc) strcmp ) ) ) {
				items = g_list_append( items, (gpointer) tmp_texdef->GetName() );
				num_of_list_items++;
			}
			// Make sure the combo list isn't too long
			if ( num_of_list_items >= MAX_NUM_LIST_ITEMS ) {
				break;
			}
		}
		// If this isn't added last (to the top of the list), g_list_find freaks.
		items = g_list_prepend( items, (gpointer) blank );
		// For Texture Entry, activate only on entry change
		strcpy( old_texture_entry, blank );
	}

	for( lst = items; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( texture_combo ), (const char *)lst->data );
	}
	g_list_free( items );

	// If the texture name is not conflicting for the faces list, select the newly added item
	if ( !is_TextureName_conflicting ) {
		gtk_combo_box_set_active( GTK_COMBO_BOX( texture_combo ), 0);
	}
}

static void GetTexdefInfo_from_Radiant(){
	g_texdef_face_vector.clear();

	unsigned int count = GetSelectedFaceCountfromBrushes();
	if ( count == 0 ) {
		count = GetSelectedFaceCount();
	}

	g_texdef_face_vector.resize( count );

	if ( !texdef_face_list_empty() ) {
		GetSelFacesTexdef( get_texdef_face_list() );
	}

	IsFaceConflicting();
	PopulateTextureComboList();
	if ( texdef_face_list_empty() ) {
		set_surface_flags_button_state( get_texdef_face_list(), TRUE );
	}
	else{
		set_surface_flags_button_state( get_texdef_face_list(), FALSE );
	}
}

static gint delete_event_callback( GtkWidget *widget, GdkEvent* event, gpointer data ){
	HideDlg();
	return TRUE;
}

static gint surface_inspector_key_press_event( GtkWidget *widget, GdkEventKey* event, gpointer data ) {
	if ( event->keyval == GDK_KEY_Escape ) {
		HideDlg();
		return TRUE;
	}
	return FALSE;
}

// make the shift increments match the grid settings
// the objective being that the shift+arrows shortcuts move the texture by the corresponding grid size
// this depends on a scale value if you have selected a particular texture on which you want it to work:
// we move the textures in pixels, not world units. (i.e. increment values are in pixel)
// depending on the texture scale it doesn't take the same amount of pixels to move of g_qeglobals.d_gridsize
// increment * scale = gridsize
// hscale and vscale are optional parameters, if they are zero they will be set to the default scale
// NOTE: the default scale depends if you are using BP mode or regular.
// For regular it's 0.5f (128 pixels cover 64 world units), for BP it's simply 1.0f
// see fenris #2810
void DoSnapTToGrid( float hscale, float vscale ){
	l_pIncrement = Get_SI_Inc();

	if ( hscale == 0.0f ) {
		hscale = l_pIncrement->scale[0];
	}
	if ( vscale == 0.0f ) {
		vscale = l_pIncrement->scale[1];
	}
#ifdef _DEBUG
	Sys_Printf( "DoSnapToGrid: grid %g hscale %g vscale %g\n", GridSize(), hscale, vscale );
#endif
	l_pIncrement->shift[0] = GridSize() / hscale;
	l_pIncrement->shift[1] = GridSize() / vscale;
	// now some update work
	// FIXME: doesn't look good here, seems to be called several times
	SetTexMods();
}

void UpdateSurfaceDialog(){
	if ( !g_bListenUpdate ) {
		return;
	}

	if ( !SurfaceInspector ) {
		return;
	}

	// avoid long delays on slow computers
	while ( gtk_events_pending() )
		gtk_main_iteration();

	if ( g_surfwin ) {
#ifdef DBG_SI
		Sys_Printf( "UpdateSurfaceDialog\n" );
#endif
		GetTexdefInfo_from_Radiant();
		SetTexMods();
	}

}

// DoSurface will always try to show the surface inspector
// or update it because something new has been selected
void DoSurface( void ){
#ifdef DBG_SI
	Sys_Printf( "DoSurface\n" );
#endif
	if ( !SurfaceInspector ) {
		create_SurfaceInspector();
	}

	ShowDlg();
	SetTexMods();
}

void ToggleSurface(){
#ifdef DBG_SI
	Sys_Printf( "ToggleSurface Module\n" );
#endif
	if ( !g_surfwin ) {
		DoSurface();
	}
	else {
		HideDlg();
	}
}

// NOTE: will raise and show the Surface inspector and exec fit for patches and brushes
void SurfaceDlgFitAll(){
	DoSurface();
	FitAll();
}

// =============================================================================
// SurfaceDialog class

void ShowDlg(){

	if ( !SurfaceInspector ) {
		create_SurfaceInspector();
	} else {
		gtk_widget_show( SurfaceInspector );
	}

	GetTexdefInfo_from_Radiant();
	GetTexMods( TRUE ); // Set Initial Undo Point
	g_surfwin = TRUE;
}

void HideDlg(){
	g_bListenUpdate = FALSE;
	g_bListenChanged = FALSE;
	g_surfwin = FALSE;
	m_nUndoId = 0;

	gtk_widget_hide( SurfaceInspector );
}

// set default values for increments (shift scale and rot)
// this is called by the prefs code if can't find the values
void InitDefaultIncrement( texdef_t *tex ){
	tex->SetName( "" );
	tex->shift[0] = 8;
	tex->shift[1] = 8;
	tex->scale[0] = 0.125;
	tex->scale[1] = 0.125;
	tex->rotate = 5;
}

void BuildDialog(){
	if ( !SurfaceInspector ) {
		create_SurfaceInspector();
	}
}

/*
   ==============
   SetTexMods

   Set the fields to the current texdef (i.e. map/texdef -> dialog widgets)
   ===============
 */

void SetTexMods(){
	texdef_t *pt;
	GtkSpinButton *spin;
	GtkAdjustment *adjust;

	texturewin = Texturewin();
	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "SurfaceDlg SetTexMods\n" );
#endif

	if ( !g_surfwin ) {
		return;
	}

	pt = &texturewin->texdef;

	g_bListenChanged = FALSE;

	if ( strncmp( texturewin->texdef.GetName(), "textures/", 9 ) != 0 ) {
		texdef_SI_values.SetName( SHADER_NOT_FOUND );
	}

	spin = GTK_SPIN_BUTTON( hshift_value_spinbutton );
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[0] );
	spin = GTK_SPIN_BUTTON( hshift_step_spinbutton );
	gtk_spin_button_set_value( spin, l_pIncrement->shift[0] );
	gtk_spin_button_set_increments( spin, l_pIncrement->shift[0], l_pIncrement->shift[0] );

	spin = GTK_SPIN_BUTTON( vshift_value_spinbutton );
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[1] );
	spin = GTK_SPIN_BUTTON( vshift_step_spinbutton );
	gtk_spin_button_set_value( spin, l_pIncrement->shift[1] );
	gtk_spin_button_set_increments( spin, l_pIncrement->shift[1], l_pIncrement->shift[1] );

	spin = GTK_SPIN_BUTTON( hscale_value_spinbutton );
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[0] );
	spin = GTK_SPIN_BUTTON( hscale_step_spinbutton );
	gtk_spin_button_set_value( spin, l_pIncrement->scale[0] );
	gtk_spin_button_set_increments( spin, l_pIncrement->scale[0], l_pIncrement->scale[0] );

	spin = GTK_SPIN_BUTTON( vscale_value_spinbutton );
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[1] );
	spin = GTK_SPIN_BUTTON( vscale_step_spinbutton );
	gtk_spin_button_set_value( spin, l_pIncrement->scale[1] );
	gtk_spin_button_set_increments( spin, l_pIncrement->scale[1], l_pIncrement->scale[1] );

	spin = GTK_SPIN_BUTTON( rotate_value_spinbutton );
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->rotate );
	spin = GTK_SPIN_BUTTON( rotate_step_spinbutton );
	gtk_spin_button_set_value( spin, l_pIncrement->rotate );
	gtk_spin_button_set_increments( spin, l_pIncrement->rotate, l_pIncrement->rotate );

	g_bListenChanged = TRUE;
}

/*
   ==============
   GetTexMods

   Shows any changes to the main Radiant windows
   ===============
 */
void GetTexMods( bool b_SetUndoPoint ){

#ifdef DBG_SI
	Sys_Printf( "SurfaceDlg GetTexMods\n" );
#endif

	if ( !texdef_face_list_empty() ) {
		g_bListenUpdate = FALSE;
		apply_surface_flags( get_texdef_face_list() );
		SetTexdef_FaceList( get_texdef_face_list(), b_SetUndoPoint, FALSE );
		g_bListenUpdate = TRUE;

		if ( b_SetUndoPoint ) {
			m_nUndoId = Undo_GetUndoId();
		}
	}
}

void FitAll(){
	on_fit_button_clicked( NULL, NULL );
}

////////////////////////////////////////////////////////////////////
//
//  GUI Section
//
////////////////////////////////////////////////////////////////////

GtkWidget* create_SurfaceInspector( void ){

	GtkWidget *label;
	GtkAdjustment *adjustment;
	GtkSizeGroup *size_group;
	GtkWidget *table1;
	GtkWidget *table2;

	GtkWidget *frame1;
	GtkWidget *frame2;

	GtkWidget *vbox1;
	GtkWidget *hbox1;

	SurfaceInspector = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for( GTK_WINDOW( SurfaceInspector ), GTK_WINDOW( g_pMainWidget ) );
	gtk_container_set_border_width( GTK_CONTAINER( SurfaceInspector ), 4 );
	gtk_window_set_title( GTK_WINDOW( SurfaceInspector ), _( "Surface Inspector" ) );

	SetWinPos_from_Prefs( SurfaceInspector );

	vbox1 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( SurfaceInspector ), vbox1 );
	gtk_widget_show( vbox1 );

	hbox1 = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( vbox1 ), hbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox1 ), 4 );
	gtk_widget_show( hbox1 );

	label = gtk_label_new( _( "Texture: " ) );
	gtk_box_pack_start( GTK_BOX( hbox1 ), label, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	texture_combo = gtk_combo_box_text_new_with_entry();
	gtk_box_pack_start( GTK_BOX( hbox1 ), texture_combo, TRUE, TRUE, 0 );
	gtk_widget_show( texture_combo );

	texture_combo_entry = gtk_bin_get_child( GTK_BIN( texture_combo ) );
	gtk_entry_set_max_length( GTK_ENTRY( texture_combo_entry ), 1024 );
	gtk_widget_show( texture_combo_entry );

	frame1 = gtk_frame_new( _( "Surface" ) );
	gtk_container_add( GTK_CONTAINER( vbox1 ), frame1 );
	gtk_widget_show( frame1 );

	table1 = gtk_table_new( 7, 3, FALSE );
	gtk_table_set_col_spacings( GTK_TABLE( table1 ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table1 ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table1 ), 5 );
	gtk_container_add( GTK_CONTAINER( frame1 ), table1 );
	gtk_widget_show( table1 );

	label = gtk_label_new( _( "Step" ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Value" ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Horizontal shift: " ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Vertical shift: " ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Horizontal scale: " ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Vertical scale: " ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 0, 1, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Rotate: " ) );
	gtk_table_attach( GTK_TABLE( table1 ), label, 0, 1, 5, 6,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	// Value Spins
	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -8192.0, 8192.0, 2.0, 8.0, 0.0 ) );
	hshift_value_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), hshift_value_spinbutton, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( hshift_value_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( hshift_value_spinbutton ), TRUE );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( hshift_value_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( hshift_value_spinbutton ), 1.0 ); //right
	gtk_widget_show( hshift_value_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -8192.0, 8192.0, 2.0, 8.0, 0.0 ) );
	vshift_value_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), vshift_value_spinbutton, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( vshift_value_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( vshift_value_spinbutton ), TRUE );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( vshift_value_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( vshift_value_spinbutton ), 1.0 ); //right
	gtk_widget_show( vshift_value_spinbutton );

	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -1024.0, 1024.0, 1.0, 4.0, 0.0 ) );
	hscale_value_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table1 ), hscale_value_spinbutton, 1, 2, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( hscale_value_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( hscale_value_spinbutton ), TRUE );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( hscale_value_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( hscale_value_spinbutton ), 1.0 ); //right
	gtk_widget_show( hscale_value_spinbutton );

	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -1024.0, 1024.0, 1.0, 4.0, 0.0 ) );
	vscale_value_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table1 ), vscale_value_spinbutton, 1, 2, 4, 5,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( vscale_value_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( vscale_value_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( vscale_value_spinbutton ), 1.0 ); //right
	gtk_widget_show( vscale_value_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -360.0, 360.0, 1.0, 10.0, 0.0 ) );
	rotate_value_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), rotate_value_spinbutton, 1, 2, 5, 6,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( rotate_value_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( rotate_value_spinbutton ), TRUE );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( rotate_value_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( rotate_value_spinbutton ), 1.0 ); //right
	gtk_widget_show( rotate_value_spinbutton );

	// Step Spins
	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -8192.0, 8192.0, 2.0, 8.0, 0.0 ) );
	hshift_step_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), hshift_step_spinbutton, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( hshift_step_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( hshift_step_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( hshift_step_spinbutton ), 1.0 ); //right
	gtk_widget_show( hshift_step_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -8192.0, 8192.0, 2.0, 8.0, 0.0 ) );
	vshift_step_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), vshift_step_spinbutton, 2, 3, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( vshift_step_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( vshift_step_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( vshift_step_spinbutton ), 1.0 ); //right
	gtk_widget_show( vshift_step_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -1024.0, 1024.0, 1.0, 4.0, 0.0 ) );
	hscale_step_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table1 ), hscale_step_spinbutton, 2, 3, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( hscale_step_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( hscale_step_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( hscale_step_spinbutton ), 1.0 ); //right
	gtk_widget_show( hscale_step_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -1024.0, 1024.0, 1.0, 4.0, 0.0 ) );
	vscale_step_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table1 ), vscale_step_spinbutton, 2, 3, 4, 5,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( vscale_step_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( vscale_step_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( vscale_step_spinbutton ), 1.0 ); //right
	gtk_widget_show( vscale_step_spinbutton );


	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, -360.0, 360.0, 1.0, 10.0, 0.0 ) );
	rotate_step_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 2 );
	gtk_table_attach( GTK_TABLE( table1 ), rotate_step_spinbutton, 2, 3, 5, 6,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( rotate_step_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( rotate_step_spinbutton ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( rotate_step_spinbutton ), 1.0 ); //right
	gtk_widget_show( rotate_step_spinbutton );


	match_grid_button = gtk_button_new_with_mnemonic( _( "Match Grid" ) );
	gtk_table_attach( GTK_TABLE( table1 ), match_grid_button, 2, 3, 6, 7,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( match_grid_button );

	frame2 = gtk_frame_new( _( "Tools" ) );
	gtk_container_add( GTK_CONTAINER( vbox1 ), frame2 );
	gtk_widget_show( frame2 );

	table2 = gtk_table_new( 2, 4, TRUE );
	gtk_table_set_col_spacings( GTK_TABLE( table2), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table2 ), 0 );
	gtk_container_set_border_width( GTK_CONTAINER( table2 ), 5 );
	gtk_container_add( GTK_CONTAINER( frame2 ), table2 );
	gtk_widget_show( table2 );

	label = gtk_label_new( _( "Height" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 3, 4, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Width" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Brush" ) );
	gtk_table_attach( GTK_TABLE( table2 ), label, 0, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 1, 1, 32, 1, 10, 0 ) );
	fit_height_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table2 ), fit_height_spinbutton, 3, 4, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( fit_height_spinbutton ), TRUE );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( fit_height_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_entry_set_alignment( GTK_ENTRY( fit_height_spinbutton ), 1.0 ); //right
	gtk_widget_show( fit_height_spinbutton );

	adjustment = GTK_ADJUSTMENT( gtk_adjustment_new( 1, 1, 32, 1, 10, 0 ) );
	fit_width_spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adjustment ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table2 ), fit_width_spinbutton, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( fit_width_spinbutton ), TRUE );
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( fit_width_spinbutton ), GTK_UPDATE_IF_VALID );
	gtk_entry_set_alignment( GTK_ENTRY( fit_width_spinbutton ), 1.0 ); //right
	gtk_widget_show( fit_width_spinbutton );

	fit_button = gtk_button_new_with_mnemonic( _( "Fit" ) );
	gtk_table_attach( GTK_TABLE( table2 ), fit_button, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( fit_button );

	axial_button = gtk_button_new_with_mnemonic( _( "Axial" ) );
	gtk_table_attach( GTK_TABLE( table2 ), axial_button, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( axial_button );

	size_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( size_group, axial_button );
	gtk_size_group_add_widget( size_group, fit_button );
	g_object_unref( size_group );

	// Add the SURF_ and CONTENTS_ flags frame
	create_SurfaceFlagsFrame( vbox1 );

	g_signal_connect( (gpointer) SurfaceInspector,
					  "delete-event",
					  G_CALLBACK( delete_event_callback ),
					  NULL );
	g_signal_connect( (gpointer) SurfaceInspector, "destroy",
					  G_CALLBACK( gtk_widget_destroy ),
					  NULL );
	g_signal_connect( (gpointer) SurfaceInspector, "key-press-event",
					  G_CALLBACK( surface_inspector_key_press_event ),
					  NULL );

	g_signal_connect( (gpointer) texture_combo_entry, "key-press-event",
					  G_CALLBACK( on_texture_combo_entry_key_press_event ),
					  NULL );
	g_signal_connect( (gpointer) texture_combo_entry, "activate",
					  G_CALLBACK( on_texture_combo_entry_activate ),
					  NULL );

	g_signal_connect( (gpointer) hshift_value_spinbutton, "value-changed",
					  G_CALLBACK( on_hshift_value_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) vshift_value_spinbutton, "value-changed",
					  G_CALLBACK( on_vshift_value_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) hscale_value_spinbutton, "value-changed",
					  G_CALLBACK( on_hscale_value_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) vscale_value_spinbutton, "value-changed",
					  G_CALLBACK( on_vscale_value_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) rotate_value_spinbutton, "value-changed",
					  G_CALLBACK( on_rotate_value_spinbutton_value_changed ),
					  NULL );

	g_signal_connect( (gpointer) hshift_step_spinbutton, "value-changed",
					  G_CALLBACK( on_hshift_step_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) vshift_step_spinbutton, "value-changed",
					  G_CALLBACK( on_vshift_step_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) hscale_step_spinbutton, "value-changed",
					  G_CALLBACK( on_hscale_step_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) vscale_step_spinbutton, "value-changed",
					  G_CALLBACK( on_vscale_step_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) rotate_step_spinbutton, "value-changed",
					  G_CALLBACK( on_rotate_step_spinbutton_value_changed ),
					  NULL );

	g_signal_connect( (gpointer) match_grid_button, "clicked",
					  G_CALLBACK( on_match_grid_button_clicked ),
					  NULL );

	g_signal_connect( (gpointer) fit_width_spinbutton, "value-changed",
					  G_CALLBACK( on_fit_width_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) fit_height_spinbutton, "value-changed",
					  G_CALLBACK( on_fit_height_spinbutton_value_changed ),
					  NULL );
	g_signal_connect( (gpointer) fit_button, "clicked",
					  G_CALLBACK( on_fit_button_clicked ),
					  NULL );

	g_signal_connect( (gpointer) axial_button, "clicked",
					  G_CALLBACK( on_axial_button_clicked ),
					  NULL );

	return SurfaceInspector;
}


// Texture Combo
gboolean on_texture_combo_entry_key_press_event( GtkWidget *widget, GdkEventKey *event,
												 gpointer user_data ){
	// Have Tab activate selection as well as Return
	if ( event->keyval == GDK_KEY_Tab ) {
		g_signal_emit_by_name( texture_combo_entry, "activate" );
	}

	return FALSE;
}

void on_texture_combo_entry_activate( GtkEntry *entry, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;
	char text[128] = { 0 };

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		// activate only on entry change
		strcpy( text, gtk_entry_get_text( entry ) );
		if ( strcmp( old_texture_entry, text ) ) {
			// Check for spaces in shader name
			if ( text[0] <= ' ' || strchr( text, ' ' ) ) {
				Sys_FPrintf( SYS_WRN, "WARNING: spaces in shader names are not allowed, ignoring '%s'\n", text );
			}
			else
			{
				for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
				{
					tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
					tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
					strcpy( old_texture_entry, text );
					tmp_texdef->SetName( text );
				}
				GetTexMods();
			}
		}
	}
}


// Match Grid
static void on_match_grid_button_clicked( GtkButton *button, gpointer user_data ){
	float hscale, vscale;

	if ( !strcmp( gtk_entry_get_text( GTK_ENTRY( hscale_value_spinbutton ) ), "" ) ) {
		hscale = 0.0;
	}
	else{
		hscale = gtk_spin_button_get_value( GTK_SPIN_BUTTON( hscale_value_spinbutton ) );
	}

	if ( !strcmp( gtk_entry_get_text( GTK_ENTRY( vscale_value_spinbutton ) ), "" ) ) {
		vscale = 0.0;
	}
	else{
		vscale = gtk_spin_button_get_value( GTK_SPIN_BUTTON( vscale_value_spinbutton ) );
	}
	DoSnapTToGrid( hscale, vscale );
}

// Value Spins
static void on_hshift_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	texdef_SI_values.shift[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( hshift_value_spinbutton ) );

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
			tmp_texdef->shift[0] = texdef_SI_values.shift[0];
			is_HShift_conflicting = FALSE;
		}
		GetTexMods();
	}
}

static void on_vshift_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	texdef_SI_values.shift[1]  = gtk_spin_button_get_value( GTK_SPIN_BUTTON( vshift_value_spinbutton ) );

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
			tmp_texdef->shift[1] = texdef_SI_values.shift[1];
			is_VShift_conflicting = FALSE;
		}
		GetTexMods();
	}
}

static void on_hscale_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	texdef_SI_values.scale[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( hscale_value_spinbutton ) );

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
			tmp_texdef->scale[0] = texdef_SI_values.scale[0];
			is_HScale_conflicting = FALSE;
		}
		GetTexMods();
	}
}

static void on_vscale_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	texdef_SI_values.scale[1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( vscale_value_spinbutton ) );

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
			tmp_texdef->scale[1] = texdef_SI_values.scale[1];
			is_VScale_conflicting = FALSE;
		}
		GetTexMods();
	}
}

static void on_rotate_value_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_t* tmp_orig_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	texdef_SI_values.rotate = gtk_spin_button_get_value( GTK_SPIN_BUTTON( rotate_value_spinbutton ) );

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
			tmp_texdef->rotate = texdef_SI_values.rotate;
			is_Rotate_conflicting = FALSE;
		}
		GetTexMods();
	}
}


// Step Spins
static void on_hshift_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	gfloat val;
	GtkAdjustment * adjust;

	if ( !g_bListenChanged ) {
		return;
	}

	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged HShift\n" );
#endif

	val = gtk_spin_button_get_value( GTK_SPIN_BUTTON( hshift_step_spinbutton ) ) ;
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( hshift_value_spinbutton ) );
	gtk_adjustment_set_step_increment( adjust, val );
	l_pIncrement->shift[0] = val;
}

static void on_vshift_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	gfloat val;
	GtkAdjustment * adjust;

	if ( !g_bListenChanged ) {
		return;
	}

	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged VShift\n" );
#endif

	val = gtk_spin_button_get_value( GTK_SPIN_BUTTON( vshift_step_spinbutton ) ) ;
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( vshift_value_spinbutton ) );
	gtk_adjustment_set_step_increment( adjust, val );
	l_pIncrement->shift[1] = val;
}

static void on_hscale_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	gfloat val;
	GtkAdjustment * adjust;

	if ( !g_bListenChanged ) {
		return;
	}

	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged HScale\n" );
#endif

	val = gtk_spin_button_get_value( GTK_SPIN_BUTTON( hscale_step_spinbutton ) ) ;
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( hscale_value_spinbutton ) );
	gtk_adjustment_set_step_increment( adjust, val );
	l_pIncrement->scale[0] = val;
}

static void on_vscale_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	gfloat val;
	GtkAdjustment * adjust;

	if ( !g_bListenChanged ) {
		return;
	}

	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged VScale\n" );
#endif

	val = gtk_spin_button_get_value( GTK_SPIN_BUTTON( vscale_step_spinbutton ) ) ;
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( vscale_value_spinbutton ) );
	gtk_adjustment_set_step_increment( adjust, val );
	l_pIncrement->scale[1] = val;
}

static void on_rotate_step_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	gfloat val;
	GtkAdjustment * adjust;

	if ( !g_bListenChanged ) {
		return;
	}

	l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged Rotate\n" );
#endif

	val = gtk_spin_button_get_value( GTK_SPIN_BUTTON( rotate_step_spinbutton ) ) ;
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( rotate_value_spinbutton ) );
	gtk_adjustment_set_step_increment( adjust, val );
	l_pIncrement->rotate = val;
}


// Fit Texture
static void on_fit_width_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	m_nWidth = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( fit_width_spinbutton ) );
}

static void on_fit_height_spinbutton_value_changed( GtkSpinButton *spinbutton, gpointer user_data ){
	m_nHeight = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( fit_height_spinbutton ) );
}

static void on_fit_button_clicked( GtkButton *button, gpointer user_data ){
	FaceList_FitTexture( get_texdef_face_list(), m_nHeight, m_nWidth );
	Sys_UpdateWindows( W_ALL );
}

// Axial Button
static void on_axial_button_clicked( GtkButton *button, gpointer user_data ){
	texdef_t* tmp_texdef;
	texdef_to_face_t* temp_texdef_face_list;

	if ( !texdef_face_list_empty() && g_bListenChanged ) {
		for ( temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
		{
			tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
			tmp_texdef->shift[0] = 0.0;
			tmp_texdef->shift[1] = 0.0;
			tmp_texdef->scale[0] = 0.5;
			tmp_texdef->scale[1] = 0.5;
			tmp_texdef->rotate = 0.0;
		}
	}

	SetTexdef_FaceList( get_texdef_face_list(), FALSE, TRUE );
	Sys_UpdateWindows( W_ALL );
}
