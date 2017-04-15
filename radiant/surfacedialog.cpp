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
// Surface Dialog
//
// Leonardo Zide (leo@lokigames.com)
//

// FIXME: this code is mostly dead, or unused. the surface inspector logic is now in a plugin, either for idtech2 or idtech3
// would be a good idea to try and remove this file entirely from the compile and see what happens

#include <gdk/gdkkeysyms.h>
#include "stdafx.h"
#include "surfacedialog.h"
#include <glib/gi18n.h>

SurfaceDlg g_dlgSurface;

/////////////////////////////////////////////////////////////////////////////
// surface properties plugin

/*
   ===================================================

   SURFACE INSPECTOR

   ===================================================
 */

// the texdef to switch back to when the OnCancel is called
texdef_t g_old_texdef;
// when != NULL, this thing means the surface inspector is currently being displayed
// NOTE a boolean flag would have been more explicit, this is totally so ugly
GtkWidget*  g_surfwin = NULL;
// turn on/off processing of the "changed" "value-changed" messages
// (need to turn off when we are feeding data in)
bool g_bListenChanged = true;
// the struct used to store the increments (saved in registry)
texdef_t *l_pIncrement = &g_qeglobals.d_savedinfo.m_SIIncrement;
// turn on/off listening of the update messages
bool g_bListenUpdate = true;

#ifdef _DEBUG
// experimental stuff, work directly on BP
static void OnTest( GtkWidget *widget, gpointer data ){
	if ( !g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_WRN, "BP mode required\n" );
		return;
	}
	if ( g_ptrSelectedFaces.GetSize() != 1 ) {
		Sys_FPrintf( SYS_WRN, "Expected single face selection\n" );
		return;
	}
	//brush_t *b = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( 0 ) );
	face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( 0 ) );
	// get the ST axis base for the face
	vec3_t texS,texT;
	ComputeAxisBase( selFace->plane.normal, texS, texT );
	// find ST coordinates for the center of the face
	float Os = 0,Ot = 0;
	int i;
	for ( i = 0; i < selFace->face_winding->numpoints; i++ )
	{
		Os += DotProduct( selFace->face_winding->points[i],texS );
		Ot += DotProduct( selFace->face_winding->points[i],texT );
	}
	Os /= selFace->face_winding->numpoints;
	Ot /= selFace->face_winding->numpoints;
	brushprimit_texdef_t *pBP = &selFace->brushprimit_texdef;

	// (FIXME: initial version, before axis base change optimize)

	// we need to compute our BP matrix in this new axis base (O,texS,texT)
	// the general case if BPO = M * BP * M^-1
	//   where BPO is transformation expressed in (O,texS,texT)
	//         M is the axis base change from (origin,texS,texT) to (O,texS,texT)
	// here we have a special case, M is a translation and it's inverse is easy
	vec_t BPO[2][3];
	vec_t aux[2][3];
	vec_t m[2][3];
	memset( &m, 0, sizeof( vec_t ) * 6 );
	m[0][0] = 1; m[1][1] = 1; m[0][2] = -Os; m[1][2] = -Ot;
	BPMatMul( m, pBP->coords, aux );
	m[0][2] = Os; m[1][2] = Ot; // now M^-1
	BPMatMul( aux, m, BPO );

#if 0
	// apply a scaling
	// scale factors against S and T axis, we apply on top of the existing matrix
	// <1 will decrease the texel/world resolution, >1 will increase
	float sS = 1.025,sT = 1.025;
	BPMatScale( BPO,sS,sT );
#endif
#if 0
	// apply a rotation
	float theta = 5;
	BPMatRotate( BPO,theta );
#endif
#if 0
	// read the scale
	ConvertTexMatWithQTexture( BPO, selFace->d_texture, aux, NULL );
	// reset the scale (normalize the matrix)
	vec_t v1,v2;
	v1 = sqrt( aux[0][0] * aux[0][0] + aux[1][0] * aux[1][0] );
	v2 = sqrt( aux[0][1] * aux[0][1] + aux[1][1] * aux[1][1] );
	// if reading the scale values, we have them here:
	Sys_Printf( "Current Scale: S: %g T: %g\n", v1, v2 );
	return;
#endif
#if 1
	// apply a given scale (on S and T)
	ConvertTexMatWithQTexture( BPO, selFace->d_texture, aux, NULL );
	// reset the scale (normalize the matrix)
	vec_t v1,v2;
	v1 = sqrt( aux[0][0] * aux[0][0] + aux[1][0] * aux[1][0] );
	v2 = sqrt( aux[0][1] * aux[0][1] + aux[1][1] * aux[1][1] );
	vec_t sS,sT;
	// put the values for scale on S and T here:
	sS = 1.2 / v1;
	sT = 0.8 / v2;
	aux[0][0] *= sS; aux[1][0] *= sS;
	aux[0][1] *= sT; aux[1][1] *= sT;
	ConvertTexMatWithQTexture( aux, NULL, BPO, selFace->d_texture );
#endif

	// now BPO must be expressed back in (origin,texS,texT) axis base BP = M^-1 * BPO * M
	BPMatMul( m, BPO, aux ); // m is M^-1
	m[0][2] = -Os; m[1][2] = -Ot;
	BPMatMul( aux, m, pBP->coords );

	// now emit the coordinates on the winding
	EmitBrushPrimitTextureCoordinates( selFace, selFace->face_winding );
	Sys_UpdateWindows( W_CAMERA );
}

/*
   FIXME: try again, there must be a silly mistake in the formula expansion
   // we need to compute our BP matrix in this new axis base (O,texS,texT)
   // the general case is BPO = M * BP * M^-1
   //   where BPO is transformation expressed in (O,texS,texT)
   //         M is the axis base change from (origin,texS,texT) to (O,texS,texT)
   // here we have a special case, M is a translation and it's inverse is easy
   // the M * BP * M^-1 formula can be expanded and simplified
   vec_t BPO[2][3];
   memcpy(&BPO, &pBP->coords, sizeof(vec_t)*6);
   BPO[0][2] = Os*(pBP->coords[0][0]-1.0) + Ot*pBP->coords[0][1] + pBP->coords[0][2];
   BPO[1][2] = Os*pBP->coords[1][0] + Ot*(pBP->coords[1][1]-1.0) + Ot*pBP->coords[1][2];

   // apply a scaling
   // scale factors against S and T axis, we apply on top of the existing matrix
   // <1 will decrease the texel/world resolution, >1 will increase
   float sS = 1.025,sT = 1.025;
   BPMatScale(BPO,sS,sT);

   // now BPO must be expressed back in (origin,texS,texT) axis base BP = M^-1 * BPO * M
   // same expanded formula as above
   memcpy(&pBP->coords, &BPO, sizeof(vec_t)*6);
   pBP->coords[0][2] = Os*(1.0-BPO[0][0]) - Ot*BPO[0][1] + BPO[0][2];
   pBP->coords[1][2] = -Os*BPO[1][0] + Ot*(1.0-BPO[1][1]) + BPO[1][2];
 */

/*
   // initial version, before axis base change optimize

   // we need to compute our BP matrix in this new axis base (O,texS,texT)
   // the general case if BPO = M * BP * M^-1
   //   where BPO is transformation expressed in (O,texS,texT)
   //         M is the axis base change from (origin,texS,texT) to (O,texS,texT)
   // here we have a special case, M is a translation and it's inverse is easy
   vec_t BPO[2][3];
   vec_t aux[2][3];
   vec_t m[2][3];
   memset(&m, 0, sizeof(vec_t)*6);
   m[0][0] = 1; m[1][1] = 1; m[0][2] = -Os; m[1][2] = -Ot;
   BPMatMul(m, pBP->coords, aux);
   m[0][2] = Os; m[1][2] = Ot; // now M^-1
   BPMatMul(aux, m, BPO);

   // apply a scaling
   // scale factors against S and T axis, we apply on top of the existing matrix
   // <1 will decrease the texel/world resolution, >1 will increase
   float sS = 1.025,sT = 1.025;
   BPMatScale(BPO,sS,sT);

   // now BPO must be expressed back in (origin,texS,texT) axis base BP = M^-1 * BPO * M
   BPMatMul(m, BPO, aux); // m is M^-1
   m[0][2] = -Os; m[1][2] = -Ot;
   BPMatMul(aux, m, pBP->coords);
 */
#endif

static void OnDone( GtkWidget *widget, gpointer data ){
	g_dlgSurface.GetTexMods();
	g_dlgSurface.HideDlg();
	Sys_UpdateWindows( W_ALL );
}

// OnUpdate is called when something is changed in the dialog
// and must be reflected in the views. But it's not a change
// so important, so the system will try to undo our last do before applying the new changes
static void OnUpdate( GtkWidget *widget, gpointer data ){
	if ( !g_bListenChanged ) {
		return;
	}

	if ( OnlyPatchesSelected() ) {
		//++timo possible bug or misfeature in our gtk_MessageBox here..
//    gtk_MessageBox("The surface inspector doesn't work for patches, use the patch inspector instead (Shift+S)", "Surface Inspector", MB_OK );
		Sys_Printf( "The surface inspector doesn't work for patches, use the patch inspector instead (Shift+S)\n" );
		return;
	}

	// avoid long delays on slow computers
	while ( gtk_events_pending() )
		gtk_main_iteration();

	g_dlgSurface.GetTexMods();
	Sys_UpdateWindows( W_CAMERA );
}

// reflect the current changes in the views, and make sure
// the changes are stored in the undo.
static void OnApply( GtkWidget *widget, gpointer data ){
	if ( !g_bListenChanged ) {
		return;
	}

	g_dlgSurface.GetTexMods();
	g_dlgSurface.m_nUndoId = 0; // that way we are sure we won't call undo
	Sys_UpdateWindows( W_CAMERA );
}

// we use OnTextureKey to detect when the user edits something in the texture widget
// in which case next 'Enter' will be interpreted as a OnApply instead of a OnDone
static gint OnTextureKey( GtkWidget* widget, GdkEventKey* event, gpointer data ){
#ifdef DBG_SI
	Sys_Printf( "OnTextureKey\n" );
#endif
	if ( event->keyval != GDK_KEY_Return ) {
		g_dlgSurface.m_bEditingTextureWidget = true;
	}
	return FALSE;
}

static void OnCancel( GtkWidget *widget, gpointer data ){
	g_qeglobals.d_texturewin.texdef = g_old_texdef;
	// cancel the last do if we own it
	if ( g_dlgSurface.m_nUndoId == Undo_GetUndoId() ) {
#ifdef DBG_SI
		Sys_Printf( "OnCancel calling Undo_Undo\n" );
#endif
		g_bListenUpdate = false;
		Undo_Undo();
		g_bListenUpdate = true;
		g_dlgSurface.m_nUndoId = 0;
	}
	g_dlgSurface.HideDlg();
}

static gint OnDialogKey( GtkWidget* widget, GdkEventKey* event, gpointer data ) {
  if ( g_surfwin ) {
    if ( event->keyval == GDK_KEY_Return ) {
      if ( g_dlgSurface.m_bEditingTextureWidget ) {
        OnApply( NULL, NULL );
        g_dlgSurface.m_bEditingTextureWidget = false;
      } else {
        OnDone( NULL, NULL );
      }
      return TRUE;
    }
    if ( event->keyval == GDK_KEY_Escape ) {
      OnCancel( NULL, NULL );
      return TRUE;
    }
  }
  return FALSE;
}

// the widget can be one of hshift, vshift, hscale, vscale, rotate
// we use the g_bListenChanged flag to ignore when changing stuff ourselves
static void OnIncrementChanged( GtkWidget *widget, gpointer data ){
	if ( !g_bListenChanged ) {
		return;
	}

#ifdef DBG_SI
	Sys_Printf( "OnIncrementChanged\n" );
#endif

	gfloat val = 0;
	sscanf( gtk_entry_get_text( GTK_ENTRY( widget ) ), "%g", &val );
	// now push it into the appropriate spin button
	GtkAdjustment * adjust;
	if ( widget == g_dlgSurface.GetDlgWidget( "hshift_inc" ) ) {
		l_pIncrement->shift[0] = val;
		adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "hshift" ) ) );
		gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[0] );
	}
	else if ( widget == g_dlgSurface.GetDlgWidget( "vshift_inc" ) ) {
		l_pIncrement->shift[1] = val;
		adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "vshift" ) ) );
		gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[1] );
	}
	else if ( widget == g_dlgSurface.GetDlgWidget( "hscale_inc" ) ) {
		l_pIncrement->scale[0] = val;
		adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "hscale" ) ) );
		gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[0] );
	}
	else if ( widget == g_dlgSurface.GetDlgWidget( "vscale_inc" ) ) {
		l_pIncrement->scale[1] = val;
		adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "vscale" ) ) );
		gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[1] );
	}
	else if ( widget == g_dlgSurface.GetDlgWidget( "rotate_inc" ) ) {
		l_pIncrement->rotate = val;
		adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "rotate" ) ) );
		gtk_adjustment_set_step_increment( adjust, l_pIncrement->rotate );
	}
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
	if ( hscale == 0.0f ) {
		( g_qeglobals.m_bBrushPrimitMode ) ? hscale = 1.0f : hscale = 0.5f;
	}
	if ( vscale == 0.0f ) {
		( g_qeglobals.m_bBrushPrimitMode ) ? vscale = 1.0f : vscale = 0.5f;
	}
#ifdef _DEBUG
	Sys_Printf( "DoSnapTToGrid: hscale %g vscale %g\n", hscale, vscale );
#endif
	l_pIncrement->shift[0] = (int) ( (float)g_qeglobals.d_gridsize / hscale );
	l_pIncrement->shift[1] = (int) ( (float)g_qeglobals.d_gridsize / vscale );
	// now some update work
	// FIXME: doesn't look good here, seems to be called several times
	g_dlgSurface.SetTexMods();
}

// make the shift increments match the grid settings
// the objective being that the shift+arrows shortcuts move the texture by the corresponding grid size
// this depends on the current texture scale used?
// we move the textures in pixels, not world units. (i.e. increment values are in pixel)
// depending on the texture scale it doesn't take the same amount of pixels to move of g_qeglobals.d_gridsize
// increment * scale = gridsize
static void OnBtnMatchGrid( GtkWidget *widget, gpointer data ){
	float hscale, vscale;
	hscale = gtk_spin_button_get_value( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "hscale" ) ) );
	vscale = gtk_spin_button_get_value( GTK_SPIN_BUTTON( g_dlgSurface.GetDlgWidget( "vscale" ) ) );
	if ( hscale == 0.0f || vscale == 0.0f ) {
		Sys_FPrintf( SYS_ERR, "ERROR: unexpected scale == 0.0f\n" );
		return;
	}
	DoSnapTToGrid( hscale, vscale );
}

void UpdateSurfaceDialog(){
	if ( !g_bListenUpdate ) {
		return;
	}

	g_SurfaceTable.m_pfnUpdateSurfaceDialog();
}

// DoSurface will always try to show the surface inspector
// or update it because something new has been selected
void DoSurface( void ){
#ifdef DBG_SI
	Sys_Printf( "DoSurface\n" );
#endif
	g_SurfaceTable.m_pfnDoSurface();
	return;
}

void ToggleSurface(){
	g_SurfaceTable.m_pfnToggleSurface();
	return;
}

// NOTE: will raise and show the Surface inspector and exec fit for patches and brushes
void SurfaceDlgFitAll(){
	g_SurfaceTable.m_pfnSurfaceDlgFitAll();
	return;
}

static void OnBtnPatchdetails( GtkWidget *widget, gpointer data ){
	Patch_NaturalizeSelected( true );
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

static void OnBtnPatchFit( GtkWidget *widget, gpointer data ){
	Patch_ResetTexturing( 1.0, 1.0 );
	Sys_UpdateWindows( W_ALL );
}

static void OnBtnAxial( GtkWidget *widget, gpointer data ){
	Select_SetTexture( &g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef, true );
	g_dlgSurface.SetTexMods();
	Sys_UpdateWindows( W_ALL );
}

static void OnBtnFaceFit( GtkWidget *widget, gpointer data ){
	g_dlgSurface.UpdateData( TRUE );
	if ( g_ptrSelectedFaces.GetSize() == 0 ) {
		brush_t *b;
		for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
		{
			for ( face_t* pFace = b->brush_faces; pFace; pFace = pFace->next )
			{
				g_ptrSelectedFaces.Add( pFace );
				g_ptrSelectedFaceBrushes.Add( b );
			}
		}
		Select_FitTexture( g_dlgSurface.m_nHeight, g_dlgSurface.m_nWidth );
		g_dlgSurface.SetTexMods();
		g_ptrSelectedFaces.RemoveAll();
	}
	else
	{
		Select_FitTexture( g_dlgSurface.m_nHeight, g_dlgSurface.m_nWidth );
		g_dlgSurface.SetTexMods();
	}
	Sys_UpdateWindows( W_ALL );
}

// =============================================================================
// SurfaceDialog class

SurfaceDlg::SurfaceDlg (){
	m_nHeight = 1;
	m_nWidth = 1;
	m_nUndoId = 0;
}

void SurfaceDlg::ShowDlg(){
	Dialog::ShowDlg();
	if ( GetWidget() == NULL ) {
		Create();
	}
	g_surfwin = GetWidget();
}
void SurfaceDlg::HideDlg(){
	g_surfwin = NULL;
	Dialog::HideDlg();
}

GtkWidget* SurfaceDlg::GetWidget(){
	return g_SurfaceTable.m_pfnGet_SI_Module_Widget();
}

// set default values for increments (shift scale and rot)
// this is called by the prefs code if can't find the values
void SurfaceDlg::InitDefaultIncrement( texdef_t *tex ){
	tex->SetName( "foo" );
	tex->shift[0] = 8;
	tex->shift[1] = 8;
	tex->scale[0] = 0.25;
	tex->scale[1] = 0.25;
	tex->rotate = 10;
}

void SurfaceDlg::BuildDialog() {
	GtkWidget *dlg, *vbox, *hbox2, *frame, *table, *label;
	GtkWidget *button, *entry, *spin;
	GtkWidget *axial_button, *cap_button, *set_button, *nat_button, *fit_button;
	GtkSizeGroup *button_group;

	dlg = m_pWidget;

	load_window_pos( dlg, g_PrefsDlg.mWindowInfo.posSurfaceWnd );

	gtk_window_set_title( GTK_WINDOW( dlg ), _( "Surface Inspector" ) );
	//g_signal_connect (G_OBJECT (dlg), "delete-event", G_CALLBACK (OnCancel), NULL);
	// we catch 'Enter' and interpret is as OnDone
	g_signal_connect( G_OBJECT( dlg ), "key-press-event", G_CALLBACK( OnDialogKey ), NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dlg ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	// replaced by only the vbox:
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox2, FALSE, TRUE, 0 );
	gtk_widget_show( hbox2 );

	label = gtk_label_new( _( "Texture" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), label, FALSE, TRUE, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_signal_connect( G_OBJECT( entry ), "key-press-event", G_CALLBACK( OnTextureKey ), NULL );
	gtk_box_pack_start( GTK_BOX( hbox2 ), entry, TRUE, TRUE, 0 );
	gtk_widget_show( entry );
	g_object_set_data( G_OBJECT( m_pWidget ), "texture", entry );

//  table = gtk_table_new (5, 4, FALSE);
	table = gtk_table_new( 6, 4, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), table, FALSE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Horizontal shift" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -8192, 8192, 2, 8, 0 ) ), 0, 0 );
	g_object_set_data( G_OBJECT( dlg ), "hshift", spin );
	g_signal_connect( G_OBJECT( gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) ) ), "value-changed",
						G_CALLBACK( OnUpdate ), NULL );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Step" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_object_set_data( G_OBJECT( dlg ), "hshift_inc", entry );
	g_signal_connect( G_OBJECT( entry ), "changed",
						G_CALLBACK( OnIncrementChanged ), NULL );
	gtk_table_attach( GTK_TABLE( table ), entry, 3, 4, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );

	label = gtk_label_new( _( "Vertical shift" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -8192, 8192, 2, 8, 0 ) ), 0, 0 );
	g_object_set_data( G_OBJECT( dlg ), "vshift", spin );
	g_signal_connect( G_OBJECT( gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) ) ), "value-changed",
						G_CALLBACK( OnUpdate ), NULL );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Step" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_object_set_data( G_OBJECT( dlg ), "vshift_inc", entry );
	g_signal_connect( G_OBJECT( entry ), "changed",
						G_CALLBACK( OnIncrementChanged ), NULL );
	gtk_table_attach( GTK_TABLE( table ), entry, 3, 4, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );

	label = gtk_label_new( _( "Horizontal stretch" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -1000, 1000, 1, 10, 0 ) ), 0, 0 );
	g_object_set_data( G_OBJECT( dlg ), "hscale", spin );
	g_signal_connect( G_OBJECT( gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) ) ), "value-changed",
						G_CALLBACK( OnUpdate ), NULL );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Step" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 2, 3 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_object_set_data( G_OBJECT( dlg ), "hscale_inc", entry );
	g_signal_connect( G_OBJECT( entry ), "changed",
						G_CALLBACK( OnIncrementChanged ), NULL );
	gtk_table_attach( GTK_TABLE( table ), entry, 3, 4, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 2, 3 );
	gtk_widget_show( entry );

	label = gtk_label_new( _( "Vertical stretch" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -1000, 1000, 1, 10, 0 ) ), 0, 0 );
	g_object_set_data( G_OBJECT( dlg ), "vscale", spin );
	g_signal_connect( G_OBJECT( gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) ) ), "value-changed",
						G_CALLBACK( OnUpdate ), NULL );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Step" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_object_set_data( G_OBJECT( dlg ), "vscale_inc", entry );
	g_signal_connect( G_OBJECT( entry ), "changed",
						G_CALLBACK( OnIncrementChanged ), NULL );
	gtk_table_attach( GTK_TABLE( table ), entry, 3, 4, 3, 4,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );

	label = gtk_label_new( _( "Rotate" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -360, 360, 1, 10, 0 ) ), 1, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	g_object_set_data( G_OBJECT( dlg ), "rotate", spin );
	g_signal_connect( G_OBJECT( gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) ) ), "value-changed",
						G_CALLBACK( OnUpdate ), NULL );
	gtk_table_attach( GTK_TABLE( table ), spin, 1, 2, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Step" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	g_object_set_data( G_OBJECT( dlg ), "rotate_inc", entry );
	g_signal_connect( G_OBJECT( entry ), "changed",
						G_CALLBACK( OnIncrementChanged ), NULL );
	gtk_table_attach( GTK_TABLE( table ), entry, 3, 4, 4, 5,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );

	// match grid button
	button = gtk_button_new_with_label( _( "Match Grid" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 2, 4, 5, 6,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnMatchGrid ), NULL );

	frame = gtk_frame_new( _( "Texturing" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 4, 4, FALSE );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Brush" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Patch" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Width" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Height" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 3, 4, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	axial_button = button = gtk_button_new_with_label( _( "Axial" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnAxial ), NULL );

	fit_button = button = gtk_button_new_with_label( _( "Fit" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnFaceFit ), NULL );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, axial_button );
	gtk_size_group_add_widget( button_group, fit_button );
	g_object_unref( button_group );

	cap_button = button = gtk_button_new_with_label( _( "CAP" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnPatchdetails ), NULL );

	set_button = button = gtk_button_new_with_label( _( "Set..." ) );
	gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnPatchreset ), NULL );

	nat_button = button = gtk_button_new_with_label( _( "Natural" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnPatchnatural ), NULL );

	fit_button = button = gtk_button_new_with_label( _( "Fit" ) );
	gtk_table_attach( GTK_TABLE( table ), button, 3, 4, 3, 4,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnBtnPatchFit ), NULL );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, cap_button );
	gtk_size_group_add_widget( button_group, set_button );
	gtk_size_group_add_widget( button_group, nat_button );
	gtk_size_group_add_widget( button_group, fit_button );
	g_object_unref( button_group );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 1, 1, 32, 1, 10, 0 ) ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), spin, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );
	AddDialogData( spin, &m_nWidth, DLG_SPIN_INT );

	spin = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 1, 1, 32, 1, 10, 0 ) ), 1, 0 );
	gtk_table_attach( GTK_TABLE( table ), spin, 3, 4, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );
	AddDialogData( spin, &m_nHeight, DLG_SPIN_INT );

	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox2, FALSE, TRUE, 0 );
	gtk_widget_show( hbox2 );

	button = gtk_button_new_with_label( _( "Done" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnDone ), NULL );

	button = gtk_button_new_with_label( _( "Apply" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnApply ), NULL );

	button = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnCancel ), NULL );

	// that's a bit of trashy stuff from Textool-v2 branch
#ifdef _DEBUG
	// FIXME: testing only, scaling in BP mode
	button = gtk_button_new_with_label( _( "Test" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnTest ), NULL );
#endif

	// Initialize
	SetTexMods();
}

/*
   ==============
   SetTexMods

   Set the fields to the current texdef (i.e. map/texdef -> dialog widgets)
   if faces selected (instead of brushes) -> will read this face texdef, else current texdef
   if only patches selected, will read the patch texdef
   ===============
 */

void SurfaceDlg::SetTexMods(){
	texdef_t *pt;
	brushprimit_texdef_t  *bpt;
	// local copy if a width=2 height=2 qtetxture_t is needed
	brushprimit_texdef_t local_bp;

#ifdef DBG_SI
	Sys_Printf( "SurfaceDlg::SetTexMods\n" );
#endif

	if ( !g_surfwin ) {
		return;
	}

	if ( g_ptrSelectedFaces.GetSize() > 0 ) {
		face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( 0 ) );
		pt = &selFace->texdef;
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			// compute a texture matrix related to the default matrix width=2 height=2
			ConvertTexMatWithQTexture( &selFace->brushprimit_texdef, selFace->d_texture, &local_bp, NULL );
			bpt = &local_bp;
		}
	}
	else
	{
		pt = &g_qeglobals.d_texturewin.texdef;
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			bpt = &g_qeglobals.d_texturewin.brushprimit_texdef;
		}
	}
	// brush primitive mode : compute fake shift scale rot representation
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		TexMatToFakeTexCoords( bpt->coords, m_shift, &m_rotate, m_scale );
	}

	g_bListenChanged = false;

	if ( strncmp( pt->GetName(), "textures/", 9 ) != 0 ) {
		pt->SetName( SHADER_NOT_FOUND );
	}
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "texture" ) ), pt->GetName() + 9 );

	GtkSpinButton *spin;
	spin = GTK_SPIN_BUTTON( GetDlgWidget( "hshift" ) );
	gtk_spin_button_set_digits( spin, 2 );
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		gtk_spin_button_set_value( spin, m_shift[0] );
	}
	else{
		gtk_spin_button_set_value( spin, pt->shift[0] );
	}
	GtkAdjustment *adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[0] );
	char buf[10]; // got into snprintf paranoia after BoundChecker detected a stack overrun
#ifdef _WIN32
	// TTimo: THIS IS UGLY
#define snprintf _snprintf
#endif
	snprintf( buf, sizeof( buf ), "%g", l_pIncrement->shift[0] );
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "hshift_inc" ) ), buf );

	spin = GTK_SPIN_BUTTON( GetDlgWidget( "vshift" ) );
	gtk_spin_button_set_digits( spin, 2 );
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		gtk_spin_button_set_value( spin, m_shift[1] );
	}
	else{
		gtk_spin_button_set_value( spin, pt->shift[1] );
	}
	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->shift[1] );
	snprintf( buf, sizeof( buf ), "%g", l_pIncrement->shift[1] );
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "vshift_inc" ) ), buf );

	spin = GTK_SPIN_BUTTON( GetDlgWidget( "hscale" ) );
	gtk_spin_button_set_digits( spin, 5 );
	gtk_spin_button_set_value( spin, g_qeglobals.m_bBrushPrimitMode ? m_scale[0] : pt->scale[0] );

	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[0] );
	snprintf( buf, sizeof( buf ), "%g", l_pIncrement->scale[0] );
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "hscale_inc" ) ), buf );

	spin = GTK_SPIN_BUTTON( GetDlgWidget( "vscale" ) );
	gtk_spin_button_set_digits( spin, 5 );
	gtk_spin_button_set_value( spin, g_qeglobals.m_bBrushPrimitMode ? m_scale[1] : pt->scale[1] );

	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->scale[1] );
	snprintf( buf, sizeof( buf ), "%g", l_pIncrement->scale[1] );
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "vscale_inc" ) ), buf );

	//++timo compute BProtate as int ..
	spin = GTK_SPIN_BUTTON( GetDlgWidget( "rotate" ) );
	gtk_spin_button_set_digits( spin, 2 );
	gtk_spin_button_set_value( spin, g_qeglobals.m_bBrushPrimitMode ? m_rotate : pt->rotate );

	adjust = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin ) );
	gtk_adjustment_set_step_increment( adjust, l_pIncrement->rotate );
	snprintf( buf, sizeof( buf ), "%g", l_pIncrement->rotate );
	gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "rotate_inc" ) ), buf );

	g_bListenChanged = true;

	// undo tricks: set the undo id to zero so we don't attempt to undo something that does not belong to us
	m_nUndoId = 0;
	// store the current texdef as our escape route if user hits OnCancel
	g_old_texdef = g_qeglobals.d_texturewin.texdef;
	// reset the Enter key behaviour flag
	m_bEditingTextureWidget = false;
}

/*
   ==============
   GetTexMods

   Reads the fields to get the current texdef (i.e. widgets -> MAP)
   in brush primitive mode, grab the fake shift scale rot and compute a new texture matrix
   ===============
 */
void SurfaceDlg::GetTexMods(){
	char buffer[1024];
	texdef_t *pt;

#ifdef DBG_SI
	Sys_Printf( "SurfaceDlg::GetTexMods\n" );
#endif

	if ( g_ptrSelectedFaces.GetSize() > 0 ) {
		//++timo just a test, we disable the undo when working on selected faces
		m_nUndoId = 0;
		face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( 0 ) );
		g_qeglobals.d_texturewin.texdef = selFace->texdef;
#ifdef DBG_SI
		Sys_Printf( "g_qeglobals.d_texturewin.texdef = selFace->texdef\n" );
#endif
	}
//  else
//  {
	pt = &g_qeglobals.d_texturewin.texdef;
#ifdef DBG_SI
	Sys_Printf( "pt = &g_qeglobals.d_texturewin.texdef\n" );
#endif
//  }

	const char* text = gtk_entry_get_text( GTK_ENTRY( GetDlgWidget( "texture" ) ) );

#ifdef DBG_SI
	Sys_Printf( "pt->SetName(%s)\n", text );
#endif

	// TTimo: detect and refuse invalid texture names (at least the ones with spaces)
	if ( text[0] <= ' ' || strchr( text, ' ' ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: spaces in shader names are not allowed, ignoring '%s'\n", text );
		pt->SetName( SHADER_NOT_FOUND );
		gtk_entry_set_text( GTK_ENTRY( GetDlgWidget( "texture" ) ), pt->GetName() );
	}
	else
	{
		strcpy( buffer, "textures/" );
		strcpy( buffer + 9, text );
		pt->SetName( buffer );
	}


	( g_qeglobals.m_bBrushPrimitMode ? m_shift[0] : pt->shift[0] ) =
		gtk_spin_button_get_value( GTK_SPIN_BUTTON( GetDlgWidget( "hshift" ) ) );
	( g_qeglobals.m_bBrushPrimitMode ? m_shift[1] : pt->shift[1] ) =
		gtk_spin_button_get_value( GTK_SPIN_BUTTON( GetDlgWidget( "vshift" ) ) );
	( g_qeglobals.m_bBrushPrimitMode ? m_scale[0] : pt->scale[0] ) =
		gtk_spin_button_get_value( GTK_SPIN_BUTTON( GetDlgWidget( "hscale" ) ) );
	( g_qeglobals.m_bBrushPrimitMode ? m_scale[1] : pt->scale[1] ) =
		gtk_spin_button_get_value( GTK_SPIN_BUTTON( GetDlgWidget( "vscale" ) ) );
	( g_qeglobals.m_bBrushPrimitMode ? m_rotate : pt->rotate ) =
		gtk_spin_button_get_value( GTK_SPIN_BUTTON( GetDlgWidget( "rotate" ) ) );

	// a local copy of the texture matrix, given for a qtexture_t with width=2 height=2
	brushprimit_texdef_t local_bp;
	brushprimit_texdef_t  *bpt;
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		face_t *selFace = NULL;
		if ( g_ptrSelectedFaces.GetSize() > 0 ) {
			selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( 0 ) );
			bpt = &selFace->brushprimit_texdef;
		}
		else
		{
			bpt = &g_qeglobals.d_texturewin.brushprimit_texdef;
		}
		// compute texture matrix
		// the matrix returned must be understood as a qtexture_t with width=2 height=2
		FakeTexCoordsToTexMat( m_shift, m_rotate, m_scale, local_bp.coords );
		// copy the texture matrix in the global struct
		// fit the qtexture if we have a face selected, otherwise g_qeglobals.d_texturewin.brushprimit_texdef uses the basic qtexture_t with width=2 height=2

		ConvertTexMatWithQTexture( &local_bp, NULL, bpt, ( ( selFace ) ? selFace->d_texture : NULL ) );
	}
	// we are gonna do stuff, if we own the last do we undo it first
	if ( m_nUndoId != 0 ) {
		// check the do we're about to undo is the one we pushed earlier
		if ( m_nUndoId == Undo_GetUndoId() ) {
#ifdef DBG_SI
			Sys_Printf( "GetTexMods calling Undo_Undo (silent)\n" );
#endif
			g_bListenUpdate = false;
			Undo_Undo( true );
			g_bListenUpdate = true;
		}
	}
	Select_SetTexture( pt,&local_bp );
	m_nUndoId = Undo_GetUndoId();
}

void SurfaceDlg::FitAll(){
	OnBtnFaceFit( NULL, NULL );
	OnBtnPatchFit( NULL, NULL );
}
