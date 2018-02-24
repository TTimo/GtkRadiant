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
// Patch Dialog
//
// Leonardo Zide (leo@lokigames.com)
//

#include <gdk/gdkkeysyms.h>
#include "stdafx.h"
#include "patchdialog.h"
#include <glib/gi18n.h>

PatchDialog g_PatchDialog;
// is the patch inspector currently displayed/active?
bool l_bIsActive = false;
// the increment we are using for the patch inspector (this is saved in the prefs)
texdef_t *l_pPIIncrement = &g_qeglobals.d_savedinfo.m_PIIncrement;

// =============================================================================
// static functions

static void OnDone( GtkWidget *widget, gpointer data ){
	g_PatchDialog.m_Patch = NULL;
	g_PatchDialog.HideDlg();
}

// memorize the current state (that is don't try to undo our do before changing something else)
static void OnApply( GtkWidget *widget, gpointer data ){
	g_PatchDialog.UpdateData( TRUE );
	if ( g_PatchDialog.m_Patch != NULL ) {
		int r = g_PatchDialog.m_nRow;
		int c = g_PatchDialog.m_nCol;
		if ( r >= 0 && r < g_PatchDialog.m_Patch->height && c >= 0 && c < g_PatchDialog.m_Patch->width ) {
			if ( g_PatchDialog.m_Patch->pShader ) {
				g_PatchDialog.m_Patch->pShader->DecRef();
			}
			if ( g_PatchDialog.m_strName.Find( ' ' ) >= 0 ) {
				Sys_FPrintf( SYS_WRN, "WARNING: spaces in shader names are not allowed, dropping '%s'\n", g_PatchDialog.m_strName.GetBuffer() );
				g_PatchDialog.m_strName = SHADER_NOT_FOUND;
			}
			g_PatchDialog.m_Patch->pShader = QERApp_Shader_ForName( g_PatchDialog.m_strName );
			g_PatchDialog.m_Patch->d_texture = g_PatchDialog.m_Patch->pShader->getTexture();
			g_PatchDialog.m_Patch->ctrl[c][r].xyz[0] = g_PatchDialog.m_fX;
			g_PatchDialog.m_Patch->ctrl[c][r].xyz[1] = g_PatchDialog.m_fY;
			g_PatchDialog.m_Patch->ctrl[c][r].xyz[2] = g_PatchDialog.m_fZ;
			g_PatchDialog.m_Patch->ctrl[c][r].st[0] = g_PatchDialog.m_fS;
			g_PatchDialog.m_Patch->ctrl[c][r].st[1] = g_PatchDialog.m_fT;
			g_PatchDialog.m_Patch->bDirty = true;
			Sys_UpdateWindows( W_ALL );
		}
	}
}

static void OnSelchangeComboColRow( GtkWidget *widget, gpointer data ){
	if ( !g_PatchDialog.m_bListenChanged ) {
		return;
	}
	// retrieve the current m_nRow and m_nCol, other params are not relevant
	// (NOTE: UpdateData has a mechanism to avoid inifinite looping)
	g_PatchDialog.UpdateData( TRUE );
	// read the changed values ourselves
	g_PatchDialog.UpdateRowColInfo();
	// now reflect our changes
	g_PatchDialog.UpdateData( FALSE );
}

static void OnBtnPatchdetails( GtkWidget *widget, gpointer data ){
	Patch_NaturalizeSelected( true );
	Sys_UpdateWindows( W_ALL );
}

static void OnBtnPatchfit( GtkWidget *widget, gpointer data ){
	Patch_ResetTexturing( 1.0, 1.0 );
	Sys_UpdateWindows( W_ALL );
}

static void OnBtnPatchnatural( GtkWidget *widget, gpointer data ){
	Patch_NaturalizeSelected();
	Sys_UpdateWindows( W_ALL );
}

static void OnBtnPatchreset( GtkWidget *widget, gpointer data ){
	float fx, fy;
	if ( DoTextureLayout( &fx, &fy ) == IDOK ) {
		Patch_ResetTexturing( fx, fy );
	}
	Sys_UpdateWindows( W_ALL );
}

static void OnSpinChanged( GtkAdjustment *adj, gpointer data ){
	texdef_t td;

	td.rotate = 0;
	td.scale[0] = td.scale[1] = 0;
	td.shift[0] = td.shift[1] = 0;
	td.contents = 0;
	td.flags = 0;
	td.value = 0;

	if ( gtk_adjustment_get_value( adj ) == 0 ) {
		return;
	}

	if ( adj == g_object_get_data( G_OBJECT( g_PatchDialog.GetWidget() ), "hshift_adj" ) ) {
		l_pPIIncrement->shift[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data ) );

		if ( gtk_adjustment_get_value( adj ) > 0 ) {
			td.shift[0] = l_pPIIncrement->shift[0];
		}
		else{
			td.shift[0] = -l_pPIIncrement->shift[0];
		}
	}
	else if ( adj == g_object_get_data( G_OBJECT( g_PatchDialog.GetWidget() ), "vshift_adj" ) ) {
		l_pPIIncrement->shift[1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data ) );

		if ( gtk_adjustment_get_value( adj ) > 0 ) {
			td.shift[1] = l_pPIIncrement->shift[1];
		}
		else{
			td.shift[1] = -l_pPIIncrement->shift[1];
		}
	}
	else if ( adj == g_object_get_data( G_OBJECT( g_PatchDialog.GetWidget() ), "hscale_adj" ) ) {
		l_pPIIncrement->scale[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data ) );
		if ( l_pPIIncrement->scale[0] == 0.0f ) {
			return;
		}
		// make sure scale factor is always >1 for increases and <1 for decreases
		if ( gtk_adjustment_get_value( adj ) > 0 ) {
			if ( l_pPIIncrement->scale[0] < 1 ) {
				td.scale[0] = l_pPIIncrement->scale[0];
			}
			else{
				td.scale[0] = 1.0f / l_pPIIncrement->scale[0];
			}
		}
		else
		{
			if ( l_pPIIncrement->scale[0] < 1 ) {
				td.scale[0] = 1.0f / l_pPIIncrement->scale[0];
			}
			else{
				td.scale[0] = l_pPIIncrement->scale[0];
			}
		}
	}
	else if ( adj == g_object_get_data( G_OBJECT( g_PatchDialog.GetWidget() ), "vscale_adj" ) ) {
		l_pPIIncrement->scale[1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data ) );
		if ( l_pPIIncrement->scale[1] == 0.0f ) {
			return;
		}
		// make sure scale factor is always >1 for increases and <1 for decreases
		if ( gtk_adjustment_get_value( adj ) > 0 ) {
			if ( l_pPIIncrement->scale[1] < 1 ) {
				td.scale[1] = l_pPIIncrement->scale[1];
			}
			else{
				td.scale[1] = 1.0f / l_pPIIncrement->scale[1];
			}
		}
		else
		{
			if ( l_pPIIncrement->scale[1] < 1 ) {
				td.scale[1] = 1.0f / l_pPIIncrement->scale[1];
			}
			else{
				td.scale[1] = l_pPIIncrement->scale[1];
			}
		}
	}
	else if ( adj == g_object_get_data( G_OBJECT( g_PatchDialog.GetWidget() ), "rotate_adj" ) ) {
		l_pPIIncrement->rotate = gtk_spin_button_get_value( GTK_SPIN_BUTTON( data ) );

		if ( gtk_adjustment_get_value( adj ) > 0 ) {
			td.rotate = l_pPIIncrement->rotate;
		}
		else{
			td.rotate = -l_pPIIncrement->rotate;
		}
	}

	gtk_adjustment_set_value( adj, 0 );

	// will scale shift rotate the patch accordingly
	Patch_SetTextureInfo( &td );
	// update the point-by-point view
	OnSelchangeComboColRow( NULL,NULL );
	Sys_UpdateWindows( W_CAMERA );
}

static gint OnDialogKey( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	if ( event->keyval == GDK_KEY_Return ) {
		OnApply( NULL, NULL );
		return TRUE;
	}
	else if ( event->keyval == GDK_KEY_Escape ) {
		OnDone( NULL, NULL );
		return TRUE;
	}
	return FALSE;
}

// =============================================================================
// Global Functions

void DoPatchInspector(){
	// do we need to create the dialog?
	if ( g_PatchDialog.GetWidget() == NULL ) {
		g_PatchDialog.Create();
		g_PatchDialog.UpdateData( FALSE );
	}
	g_PatchDialog.GetPatchInfo();
	if ( !l_bIsActive ) {
		g_PatchDialog.ShowDlg();
	}
}

void UpdatePatchInspector(){
	if ( l_bIsActive ) {
		g_PatchDialog.GetPatchInfo();
	}
}

void TogglePatchInspector(){
	if ( l_bIsActive ) {
		OnDone( NULL,NULL );
	}
	else{
		DoPatchInspector();
	}
}

// =============================================================================
// PatchDialog class

PatchDialog::PatchDialog (){
	m_strName = "";
	m_fS = 0.0f;
	m_fT = 0.0f;
	m_fX = 0.0f;
	m_fY = 0.0f;
	m_fZ = 0.0f;
	m_nCol = 0;
	m_nRow = 0;
	m_Patch = NULL;
	m_bListenChanged = true;
}

void PatchDialog::InitDefaultIncrement( texdef_t *tex ){
	tex->SetName( SHADER_NOT_FOUND );
	tex->scale[0] = 0.5f;
	tex->scale[1] = 0.5f;
	tex->rotate = 45;
	tex->shift[0] = 8.0f;
	tex->shift[1] = 8.0f;
}

// we plug into HideDlg and ShowDlg to maintain the l_bIsActive flag
void PatchDialog::HideDlg(){
	l_bIsActive = false;
	Dialog::HideDlg();
}

void PatchDialog::ShowDlg(){
	l_bIsActive = true;
	Dialog::ShowDlg();
}

void PatchDialog::BuildDialog(){
	GtkWidget *dlg, *vbox, *vbox2, *hbox, *hbox2, *frame, *table, *label;
	GtkWidget *button, *entry, *spin, *combo, *row_label, *col_label;
	GtkAdjustment *adj;
	GList *lst, *cells;
	GtkSizeGroup *size_group;
	GtkWidget *cap_button, *set_button, *nat_button, *fit_button;

	dlg = m_pWidget;

	load_window_pos( dlg, g_PrefsDlg.mWindowInfo.posPatchWnd );

	gtk_window_set_title( GTK_WINDOW( dlg ), _( "Patch Properties" ) );
	g_signal_connect( G_OBJECT( dlg ), "delete-event", G_CALLBACK( OnDone ), NULL );
	// catch 'Esc' and 'Enter'
	g_signal_connect( G_OBJECT( dlg ), "key-press-event", G_CALLBACK( OnDialogKey ), NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dlg ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );


	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	frame = gtk_frame_new( _( "Details" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	row_label = label = gtk_label_new( _( "Row:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	col_label = label = gtk_label_new( _( "Column:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	size_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( size_group, row_label );
	gtk_size_group_add_widget( size_group, col_label );
	g_object_unref( size_group );

	combo = gtk_combo_box_text_new();
	gtk_table_attach( GTK_TABLE( table ), combo, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( combo );
	g_signal_connect( G_OBJECT( GTK_COMBO_BOX( combo ) ), "changed",
						G_CALLBACK( OnSelchangeComboColRow ), this );
	AddDialogData( combo, &m_nRow, DLG_COMBO_BOX_INT );
	m_pRowCombo = combo;
	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( G_OBJECT( lst->data ), "xalign", 1.0, (char*)NULL );
	}
	g_list_free( cells );

	combo = gtk_combo_box_text_new();
	gtk_table_attach( GTK_TABLE( table ), combo, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( combo );
	g_signal_connect( G_OBJECT( GTK_COMBO_BOX( combo ) ), "changed",
						G_CALLBACK( OnSelchangeComboColRow ), this );
	AddDialogData( combo, &m_nCol, DLG_COMBO_BOX_INT );
	m_pColCombo = combo;
	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( G_OBJECT( lst->data ), "xalign", 1.0, (char*)NULL );
	}
	g_list_free( cells );

	table = gtk_table_new( 5, 2, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "X:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_set_tooltip_text( label, _( "X-Axis" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Y:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_set_tooltip_text( label, _( "Y-Axis" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Z:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_set_tooltip_text( label, _( "Z-Axis" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "S:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_set_tooltip_text( label, _( "S-coordinates correspond to the \"x\" coordinates on the texture itself" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "T:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 4, 5,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_set_tooltip_text( label, _( "T-coordinates correspond to the \"y\" coordinates on the texture itself. The measurements are in game units." ) );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	AddDialogData( spin, &m_fX, DLG_SPIN_FLOAT );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	AddDialogData( spin, &m_fY, DLG_SPIN_FLOAT );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	AddDialogData( spin, &m_fZ, DLG_SPIN_FLOAT );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	AddDialogData( spin, &m_fS, DLG_SPIN_FLOAT );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 4, 5,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	AddDialogData( spin, &m_fT, DLG_SPIN_FLOAT );

	frame = gtk_frame_new( _( "Texturing" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	label = gtk_label_new( _( "Name:" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), label, TRUE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
//  gtk_entry_set_editable (GTK_ENTRY (entry), false);
	gtk_box_pack_start( GTK_BOX( vbox2 ), entry, TRUE, TRUE, 0 );
	gtk_widget_show( entry );
	AddDialogData( entry, &m_strName, DLG_ENTRY_TEXT );

	table = gtk_table_new( 5, 3, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Horizontal Shift Step" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Vertical Shift Step" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Horizontal Stretch Step" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Vertical Stretch Step" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Rotate Step" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	// we fill in this data, if no patch is selected the widgets are unmodified when the inspector is raised
	// so we need to have at least one initialisation somewhere
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), l_pPIIncrement->shift[0] );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -8192, 8192, 1, 1, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnSpinChanged ), spin );
	g_object_set_data( G_OBJECT( m_pWidget ), "hshift_adj", adj );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 0, 1,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), l_pPIIncrement->shift[1] );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -8192, 8192, 1, 1, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnSpinChanged ), spin );
	g_object_set_data( G_OBJECT( m_pWidget ), "vshift_adj", adj );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 1, 2,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), l_pPIIncrement->scale[0] );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -1000, 1000, 1, 1, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnSpinChanged ), spin );
	g_object_set_data( G_OBJECT( m_pWidget ), "hscale_adj", adj );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 2, 3,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -INT_MAX, INT_MAX, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), l_pPIIncrement->scale[1] );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -1000, 1000, 1, 1, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnSpinChanged ), spin );
	g_object_set_data( G_OBJECT( m_pWidget ), "vscale_adj", adj );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 3, 4,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -360, 360, 1, 10, 0 ) );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 4 );
	gtk_table_attach( GTK_TABLE( table ), spin, 0, 1, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ),  l_pPIIncrement->rotate );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0, -1000, 1000, 1, 1, 0 ) ); // NOTE: Arnout - this really should be 360 but can't change it anymore as it could break existing maps
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnSpinChanged ), spin );
	g_object_set_data( G_OBJECT( m_pWidget ), "rotate_adj", adj );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 4, 5,
					  (GtkAttachOptions) ( 0 ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_widget_show( spin );

	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_set_homogeneous( GTK_BOX( hbox2 ), TRUE );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox2, TRUE, FALSE, 0 );
	gtk_widget_show( hbox2 );

	cap_button = button = gtk_button_new_with_label( _( "CAP" ) );
	gtk_box_pack_end( GTK_BOX( hbox2 ), button, TRUE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnBtnPatchdetails ), NULL );

	set_button = button = gtk_button_new_with_label( _( "Set..." ) );
	gtk_box_pack_end( GTK_BOX( hbox2 ), button, TRUE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnBtnPatchreset ), NULL );

	nat_button = button = gtk_button_new_with_label( _( "Natural" ) );
	gtk_box_pack_end( GTK_BOX( hbox2 ), button, TRUE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnBtnPatchnatural ), NULL );

	fit_button = button = gtk_button_new_with_label( _( "Fit" ) );
	gtk_box_pack_end( GTK_BOX( hbox2 ), button, TRUE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnBtnPatchfit ), NULL );

	size_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( size_group, cap_button );
	gtk_size_group_add_widget( size_group, set_button );
	gtk_size_group_add_widget( size_group, nat_button );
	gtk_size_group_add_widget( size_group, fit_button );
	g_object_unref( size_group );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, FALSE, 0 );
	gtk_widget_show( hbox );

	button = gtk_button_new_with_label( _( "Done" ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnDone ), NULL );

	button = gtk_button_new_with_label( _( "Apply" ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnApply ), NULL );
}

// sync the dialog our internal data structures
void PatchDialog::UpdateData( bool retrieve ){
	if ( m_pWidget == NULL ) {
		return;
	}

	m_bListenChanged = false;
	Dialog::UpdateData( retrieve );
	m_bListenChanged = true;
}

// read the map and feed in the stuff to the dialog box
void PatchDialog::GetPatchInfo(){
	m_Patch = SinglePatchSelected();
	if ( m_Patch != NULL ) {
		gchar buffer[11];
		int i;
		GtkListStore *store;

		m_strName = m_Patch->pShader->getName();

		// fill in the numbers for Row / Col selection
		m_bListenChanged = false;

#if GTK_CHECK_VERSION( 3, 0, 0 )
		gtk_combo_box_text_remove_all( GTK_COMBO_BOX_TEXT( m_pRowCombo ) );
#else
		store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( m_pRowCombo ) ) );
		gtk_list_store_clear( store );
#endif

		for ( i = 0; i < m_Patch->height; i++ )
		{
			g_snprintf( buffer, sizeof( buffer ), "%i", i );
			gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( m_pRowCombo ), buffer );
		}
		gtk_combo_box_set_active( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( m_pRowCombo ) ), 0 );

		for ( i = 0; i < m_Patch->width; i++ )
		{
			g_snprintf( buffer, sizeof( buffer ), "%i", i );
			gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( m_pColCombo ), buffer );
		}
		gtk_combo_box_set_active( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( m_pColCombo ) ), 0 );

		m_bListenChanged = true;

	}
	else{
		Sys_FPrintf( SYS_WRN, "WARNING: No patch selected.\n" );
	}
	// fill in our internal structs
	m_nRow = 0; m_nCol = 0;
	UpdateRowColInfo();
	// now update the dialog box
	UpdateData( false );
}

// read the current patch on map and initialize m_fX m_fY accordingly
// NOTE: don't call UpdateData in there, it's not meant for
void PatchDialog::UpdateRowColInfo(){
	m_fX = m_fY = m_fZ = m_fS = m_fT = 0.0;

	if ( m_Patch != NULL ) {
		// we rely on whatever active row/column has been set before we get called
		int r = m_nRow;
		int c = m_nCol;
		if ( r >= 0 && r < m_Patch->height && c >= 0 && c < m_Patch->width ) {
			m_fX = m_Patch->ctrl[c][r].xyz[0];
			m_fY = m_Patch->ctrl[c][r].xyz[1];
			m_fZ = m_Patch->ctrl[c][r].xyz[2];
			m_fS = m_Patch->ctrl[c][r].st[0];
			m_fT = m_Patch->ctrl[c][r].st[1];
		}
	}
}
