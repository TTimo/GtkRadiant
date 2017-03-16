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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// classes used for describing geometry information from q3map feedback
//

#include "stdafx.h"

#include "feedback.h"
#include "glib.h"
#include <assert.h>
#include <glib/gi18n.h>

CDbgDlg g_DbgDlg;

void CSelectMsg::saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs ){
	if ( strcmp( (char *)name, "select" ) == 0 ) {
		// read the message
		ESelectState = SELECT_MESSAGE;
	}
	else
	{
		// read the brush
		assert( strcmp( (char *)name, "brush" ) == 0 );
		assert( ESelectState == SELECT_MESSAGE );
		ESelectState = SELECT_BRUSH;
	}
}

void CSelectMsg::saxEndElement( message_info_t *ctx, const xmlChar *name ){
	if ( strcmp( (char *)name, "select" ) == 0 ) {
		ctx->bGeometry = false;
	}
}

void CSelectMsg::saxCharacters( message_info_t *ctx, const xmlChar *ch, int len ){
	if ( ESelectState == SELECT_MESSAGE ) {
		message = g_string_sized_new( len + 1 );
		memcpy( message->str, ch, len );
		message->str[len] = '\0';
		Sys_Printf( "%s\n", message->str );
	}
	else
	{
		assert( ESelectState == SELECT_BRUSH );
		sscanf( (char *)ch, "%i %i", &entitynum, &brushnum );
	}
}

void CSelectMsg::Highlight(){
	Select_Deselect();
	SelectBrush( entitynum, brushnum );
}

void CPointMsg::saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs ){
	if ( strcmp( (char *)name, "pointmsg" ) == 0 ) {
		// read the message
		EPointState = POINT_MESSAGE;
	}
	else
	{
		// read the brush
		assert( strcmp( (char *)name, "point" ) == 0 );
		assert( EPointState == POINT_MESSAGE );
		EPointState = POINT_POINT;
	}
}

void CPointMsg::saxEndElement( message_info_t *ctx, const xmlChar *name ){
	if ( strcmp( (char *)name, "pointmsg" ) == 0 ) {
		ctx->bGeometry = false;
	}
}

void CPointMsg::saxCharacters( message_info_t *ctx, const xmlChar *ch, int len ){
	if ( EPointState == POINT_MESSAGE ) {
		message = g_string_sized_new( len + 1 );
		memcpy( message->str, ch, len );
		message->str[len] = '\0';
		Sys_Printf( "%s\n", message->str );
	}
	else
	{
		assert( EPointState == POINT_POINT );
		sscanf( (char *)ch, "%g %g %g", &( pt[0] ), &( pt[1] ), &( pt[2] ) );
	}
}

void CPointMsg::Highlight(){
	// use the entity API to push a point
	// the API requires a ref count, we do it manually for the current instance
	if ( refCount == 0 ) {
		refCount++;
		QERApp_HookGL2DWindow( this );
	}
}

void CPointMsg::DropHighlight(){
	assert( refCount > 0 );
	QERApp_UnHookGL2DWindow( this );
	// do a refCount-- locally (see Highlight)
	refCount--;
}

void CPointMsg::Draw2D( VIEWTYPE vt ){
	int nDim1 = ( vt == YZ ) ? 1 : 0;
	int nDim2 = ( vt == XY ) ? 1 : 2;
	qglPointSize( 4 );
	qglColor3f( 1.0f,0.0f,0.0f );
	qglBegin( GL_POINTS );
	qglVertex2f( pt[nDim1], pt[nDim2] );
	qglEnd();
	qglBegin( GL_LINE_LOOP );
	qglVertex2f( pt[nDim1] - 8, pt[nDim2] - 8 );
	qglVertex2f( pt[nDim1] + 8, pt[nDim2] - 8 );
	qglVertex2f( pt[nDim1] + 8, pt[nDim2] + 8 );
	qglVertex2f( pt[nDim1] - 8, pt[nDim2] + 8 );
	qglEnd();
}

void CWindingMsg::saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs ){
	if ( strcmp( (char *)name, "windingmsg" ) == 0 ) {
		// read the message
		EPointState = WINDING_MESSAGE;
	}
	else
	{
		// read the brush
		assert( strcmp( (char *)name, "winding" ) == 0 );
		assert( EPointState == WINDING_MESSAGE );
		EPointState = WINDING_WINDING;
	}
}

void CWindingMsg::saxEndElement( message_info_t *ctx, const xmlChar *name ){
	if ( strcmp( (char *)name, "windingmsg" ) == 0 ) {
		ctx->bGeometry = false;
	}
}

void CWindingMsg::saxCharacters( message_info_t *ctx, const xmlChar *ch, int len ){
	if ( EPointState == WINDING_MESSAGE ) {
		message = g_string_sized_new( len + 1 );
		memcpy( message->str, ch, len );
		message->str[len] = '\0';
		Sys_Printf( "%s\n", message->str );
	}
	else
	{
		char* c;
		int i;

		assert( EPointState == WINDING_WINDING );


		c = (char*)ch;
		sscanf( c, "%i ", &numpoints );

		for ( i = 0; i < numpoints; i++ )
		{
			c = strchr( ++c, '(' );
			if ( c ) { // even if we are given the number of points when the cycle begins .. don't trust it too much
				sscanf( c, "(%g %g %g)", &wt[i][0], &wt[i][1], &wt[i][2] );
			}
			else{
				break;
			}
		}
		numpoints = i;
	}
}

void CWindingMsg::Highlight(){
	// use the entity API to push a point
	// the API requires a ref count, we do it manually for the current instance
	if ( refCount == 0 ) {
		refCount++;
		QERApp_HookGL2DWindow( this );
	}
}

void CWindingMsg::DropHighlight(){
	assert( refCount > 0 );
	QERApp_UnHookGL2DWindow( this );
	// do a refCount-- locally (see Highlight)
	refCount--;
}

void CWindingMsg::Draw2D( VIEWTYPE vt ){
	int i;

	int nDim1 = ( vt == YZ ) ? 1 : 0;
	int nDim2 = ( vt == XY ) ? 1 : 2;
	qglColor3f( 1.0f,0.f,0.0f );

	qglPointSize( 4 );
	qglBegin( GL_POINTS );
	for ( i = 0; i < numpoints; i++ )
		qglVertex2f( wt[i][nDim1], wt[i][nDim2] );
	qglEnd();
	qglPointSize( 1 );

	qglEnable( GL_BLEND );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	qglColor4f( 0.133f,0.4f,1.0f,0.5f );
	qglBegin( GL_POLYGON );
	for ( i = 0; i < numpoints; i++ )
		qglVertex2f( wt[i][nDim1], wt[i][nDim2] );
	qglEnd();
	qglDisable( GL_BLEND );
}

// triggered when the user selects an entry in the feedback box
static void feedback_selection_changed( GtkTreeSelection* selection, gpointer data ){
	g_DbgDlg.DropHighlight();

	GtkTreeModel* model;
	GtkTreeIter selected;
	if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		GtkTreePath* path = gtk_tree_model_get_path( model, &selected );
		g_DbgDlg.SetHighlight( gtk_tree_path_get_indices( path )[0] );
		gtk_tree_path_free( path );
	}
}

void CDbgDlg::DropHighlight(){
	if ( m_pHighlight ) {
		m_pHighlight->DropHighlight();
		m_pHighlight = NULL;
	}
}

void CDbgDlg::SetHighlight( gint row ){
	ISAXHandler *h = GetElement( row );
	if ( h != NULL ) {
		h->Highlight();
		m_pHighlight = h;
	}
}

ISAXHandler *CDbgDlg::GetElement( gint row ) {
	return static_cast<ISAXHandler *>( g_ptr_array_index( m_pFeedbackElements, row ) );
}

void CDbgDlg::ClearFeedbackArray() {
	// free all the ISAXHandler*, clean it
	while ( m_pFeedbackElements->len ) {
		// some ISAXHandler are static and passed around but should never be deleted
		ISAXHandler *handler = static_cast< ISAXHandler * >( g_ptr_array_index( m_pFeedbackElements, 0 ) );
		if ( handler->ShouldDelete() ) {
			delete handler;
		}
		g_ptr_array_remove_index( m_pFeedbackElements, 0 );
	}
}

void CDbgDlg::Init() {
	DropHighlight();

	ClearFeedbackArray();

	if ( m_clist != NULL ) {
		gtk_list_store_clear( m_clist );
	}
}

void CDbgDlg::Push( ISAXHandler *pHandler ) {
	// push in the list
	g_ptr_array_add( m_pFeedbackElements, (void *)pHandler );

	if ( m_pWidget == NULL ) {
		Create();
	}
	// put stuff in the list
	gtk_list_store_clear( m_clist );
	unsigned int i;
	for ( i = 0; i < m_pFeedbackElements->len; i++ ) {
		GtkTreeIter iter;
		gtk_list_store_append( m_clist, &iter );
		gtk_list_store_set( m_clist, &iter, 0, GetElement( i )->getName(), -1 );
	}

	ShowDlg();
}

void CDbgDlg::BuildDialog(){
	gtk_window_set_title( GTK_WINDOW( m_pWidget ), _( "Q3Map debug window" ) );

	GtkWidget* scr = gtk_scrolled_window_new( NULL, NULL );
	gtk_widget_show( scr );
	gtk_container_add( GTK_CONTAINER( m_pWidget ), GTK_WIDGET( scr ) );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
			gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
			g_signal_connect( G_OBJECT( selection ), "changed", G_CALLBACK( feedback_selection_changed ), NULL );
		}

		gtk_widget_show( view );

		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );

		m_clist = store;
	}
}
