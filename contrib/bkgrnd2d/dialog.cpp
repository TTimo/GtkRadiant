/*
Copyright (C) 2003 Reed Mideke.

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
// bkgrnd2d Plugin dialog
//
// Code by reyalP aka Reed Mideke
//
// Based on various other plugins
//

#include <gtk/gtk.h>

#include "bkgrnd2d.h"
#include "dialog.h"

// spaces to make label nice and big
#define NO_FILE_MSG "        (no file loaded)        "

static GtkWidget *pDialogWnd;
static GtkWidget *pNotebook;
static GtkTooltips *pTooltips;

class CBackgroundDialogPage
{
private:
	GtkWidget *m_pWidget;
	GtkWidget *m_pTabLabel;
	GtkWidget *m_pFileLabel;
	GtkWidget *m_pPosLabel;
	VIEWTYPE m_vt;
	bool m_bValidFile;

public:
	CBackgroundImage *m_pImage;
	CBackgroundDialogPage( VIEWTYPE vt );
	void Append(GtkWidget *notebook);
	void Browse();
	void Reload();
	void SetPosLabel();
//  ~BackgroundDialogPage();
};


// dialog page callbacks
static void browse_callback( GtkWidget *widget, gpointer data )
{
	((CBackgroundDialogPage *)data)->Browse();
}

static void reload_callback( GtkWidget *widget, gpointer data )
{
	((CBackgroundDialogPage *)data)->Reload();
}

static void size_sel_callback( GtkWidget *widget, gpointer data )
{
	CBackgroundDialogPage *pPage = (CBackgroundDialogPage *)data;
	if (pPage->m_pImage->SetExtentsSel())
		pPage->SetPosLabel();
}

static void size_mm_callback( GtkWidget *widget, gpointer data )
{
	CBackgroundDialogPage *pPage = (CBackgroundDialogPage *)data;
	if(pPage->m_pImage->SetExtentsMM())
		pPage->SetPosLabel();
}

static void alpha_adjust_callback( GtkWidget *widget, gpointer data )
{
	CBackgroundDialogPage *pPage = (CBackgroundDialogPage *)data;
	pPage->m_pImage->m_alpha = (float)gtk_range_get_value (GTK_RANGE(widget));
	g_FuncTable.m_pfnSysUpdateWindows(W_XY);
}

void CBackgroundDialogPage::Reload()
{
	if(m_bValidFile)
		m_pImage->Load(gtk_label_get_text(GTK_LABEL(m_pFileLabel)));
}

void CBackgroundDialogPage::Browse()
{
	char browsedir[PATH_MAX];
	const char *ct;
	const char *newfile;
	char *t;
	
	//TODO GetMapName saves the map. eeep!
	//also with no map, returns unnamed.map, otherwise returns full path
//	Syn_Printf(MSG_PREFIX "GetMapName() %s\n",
//				g_FuncTable.m_pfnGetMapName());
	
	ct = g_FuncTable.m_pfnReadProjectKey("basepath");
	// TODO shouldn't need this stuff
	if(!ct || !strlen(ct)) {
		Syn_Printf(MSG_PREFIX "basepath = NULL or empty\n");
		return;
	}
	Syn_Printf(MSG_PREFIX "basepath: %s\n",ct);
	if(strlen(ct) >= PATH_MAX) {
		Syn_Printf(MSG_PREFIX "base game dir too long\n");
		return;
	}

	strcpy(browsedir,ct);
	// make sure we have a trailing / 
	if(browsedir[strlen(browsedir) - 1] != '/')
		strcat(browsedir,"/");

	//if we dont have a file yet, don't try to use it for default dir
	if(m_bValidFile) {
	// filename should always be a nice clean unix style relative path
		ct = gtk_label_get_text(GTK_LABEL(m_pFileLabel));
		strcat(browsedir,ct);
		Syn_Printf(MSG_PREFIX "full path: %s\n",browsedir);

		// lop off the file part
		t = browsedir + strlen(browsedir) - 1;
		while (t != browsedir && *t != '/') 
			t--;
		*t = 0;
	}
	Syn_Printf(MSG_PREFIX "browse directory %s\n",browsedir);
	
//does NOT need freeing contrary to include/qerplugin.h comments
//TODO bug/patch for comments
//TODO patern gets fucked up sometimes if empty
//http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=915
	newfile = g_FuncTable.m_pfnFileDialog(pDialogWnd,TRUE,
						                  "Load Background Image",browsedir,FILETYPE_KEY);
	if(!newfile) {
		Syn_Printf(MSG_PREFIX "newfile = NULL\n");
		return;
	}
	Syn_Printf(MSG_PREFIX "newfile: %s\n",newfile);
	newfile = g_FileSystemTable.m_pfnExtractRelativePath(newfile);

	if(!newfile) {
		Syn_Printf(MSG_PREFIX "newfile = NULL\n");
		return;
	}
	Syn_Printf(MSG_PREFIX "newfile: %s\n",newfile);

	if(m_pImage->Load(newfile)) {
		m_bValidFile = true;
		gtk_label_set_text(GTK_LABEL(m_pFileLabel),newfile);
	}
}

void CBackgroundDialogPage::SetPosLabel()
{
	char s[64];
	// TODO no snprintf ?
	sprintf(s, "Size/Position (%d,%d) (%d,%d)",(int)(m_pImage->m_xmin),
			(int)(m_pImage->m_ymin),(int)(m_pImage->m_xmax),(int)(m_pImage->m_ymax));
	gtk_label_set_text(GTK_LABEL(m_pPosLabel),s);
}

CBackgroundDialogPage::CBackgroundDialogPage(VIEWTYPE vt )
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *w;

	m_vt = vt;

	m_bValidFile = false;

	switch(m_vt)
	{
		case XY:
			m_pTabLabel = gtk_label_new("X/Y");
			m_pImage = &backgroundXY;
			break;
		case XZ:
			m_pTabLabel = gtk_label_new("X/Z");
			m_pImage = &backgroundXZ;
			break;
		case YZ:
			m_pTabLabel = gtk_label_new("Y/Z");
			m_pImage = &backgroundYZ;
			break;
	}
// A vbox to hold everything
	m_pWidget = gtk_vbox_new(FALSE,0); 
// Frame for file row
	frame = gtk_frame_new("File");
	gtk_box_pack_start (GTK_BOX (m_pWidget),frame, FALSE, FALSE, 2);

// hbox for first row
	hbox = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER (hbox),4);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

// label to display filename
	m_pFileLabel  = gtk_label_new(NO_FILE_MSG);
	gtk_label_set_selectable(GTK_LABEL(m_pFileLabel),TRUE);
//TODO set min size ? done with spaces right now
	gtk_box_pack_start (GTK_BOX (hbox),m_pFileLabel, TRUE, TRUE, 5);

  gtk_widget_show (m_pFileLabel);

	w = gtk_button_new_with_label ("Browse...");
	g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (browse_callback),
                                                              (gpointer)this);
	gtk_box_pack_start (GTK_BOX (hbox),w, FALSE, FALSE, 5);
	gtk_tooltips_set_tip (pTooltips, w, "Select a file", NULL);
  gtk_widget_show (w);
	
	w = gtk_button_new_with_label ("Reload");
	g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (reload_callback),
                                                              (gpointer)this);
	// TODO disable until we have file
	// gtk_widget_set_sensitive(w,FALSE);
	gtk_tooltips_set_tip (pTooltips, w, "Reload current file", NULL);
	gtk_box_pack_start (GTK_BOX (hbox),w, FALSE, FALSE, 5);
  gtk_widget_show (w);

	gtk_widget_show (hbox);
	gtk_widget_show (frame);

// second row (rendering options)
	frame = gtk_frame_new("Rendering");
	gtk_box_pack_start (GTK_BOX (m_pWidget),frame, FALSE, FALSE, 2);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER (hbox),4);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	w = gtk_label_new("Vertex alpha:");
	gtk_box_pack_start (GTK_BOX (hbox),w, FALSE, FALSE, 5);
  gtk_widget_show (w);

	w = gtk_hscale_new_with_range(0.0,1.0,0.01);
	gtk_range_set_value(GTK_RANGE(w),0.5);
	gtk_scale_set_value_pos(GTK_SCALE(w),GTK_POS_LEFT);
	g_signal_connect (G_OBJECT (w), "value-changed",
                          G_CALLBACK (alpha_adjust_callback), (gpointer)this);
	gtk_box_pack_start (GTK_BOX (hbox),w, TRUE, TRUE, 5);
	gtk_tooltips_set_tip (pTooltips, w, "Set image transparancy", NULL);
  gtk_widget_show (w);

	gtk_widget_show (hbox);
	gtk_widget_show (frame);
// Third row (size and position)
	frame = gtk_frame_new("Size/Position (undefined)");
	m_pPosLabel = gtk_frame_get_label_widget (GTK_FRAME(frame));
	gtk_box_pack_start ( GTK_BOX (m_pWidget), frame, FALSE, FALSE, 2);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER (hbox),4);

	w = gtk_button_new_with_label ("from selection");
	gtk_box_pack_start (GTK_BOX (hbox),w, TRUE, FALSE, 5);
	g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (size_sel_callback),
                                                                (gpointer)this);
	gtk_tooltips_set_tip (pTooltips, w, "Set the size of the image to the bounding rectangle of all selected brushes and entities", NULL);
  gtk_widget_show (w);

	if(m_vt == XY) {
		w = gtk_button_new_with_label ("from map mins/maxs");
		gtk_box_pack_start ( GTK_BOX (hbox),w, TRUE, FALSE, 2);
		g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (size_mm_callback),
                                                                (gpointer)this);
		gtk_tooltips_set_tip (pTooltips, w, "Set the size of the image using the mapcoordsmins and mapcoordsmaxs keys of the worldspawn entity", NULL);
		gtk_widget_show (w);
	}

	gtk_widget_show (hbox);
	gtk_widget_show (frame);

  gtk_widget_show ( m_pWidget );
}

void CBackgroundDialogPage::Append(GtkWidget *notebook)
{
	gtk_notebook_append_page( GTK_NOTEBOOK(notebook), m_pWidget, m_pTabLabel);
}

// dialog global callbacks
/*
static gint expose_callback( GtkWidget *widget, gpointer data )
{
	return FALSE;
}
*/

static void response_callback( GtkWidget *widget, gint response, gpointer data )
{
	if( response == GTK_RESPONSE_CLOSE )
 		gtk_widget_hide( pDialogWnd );
}

static gint close_callback( GtkWidget *widget, gpointer data )
{
 	gtk_widget_hide( pDialogWnd );
	return TRUE;
}

void InitBackgroundDialog()
{
	CBackgroundDialogPage *pPage;

	pDialogWnd = gtk_dialog_new_with_buttons ("Background Images",
                              GTK_WINDOW(g_pMainWidget),
                              (GtkDialogFlags)(GTK_DIALOG_DESTROY_WITH_PARENT),
        // TODO dialog with no buttons
        //										  GTK_STOCK_CLOSE,
        //										  GTK_RESPONSE_CLOSE,
                              NULL);
	gtk_signal_connect( GTK_OBJECT (pDialogWnd), "delete_event",
                                    GTK_SIGNAL_FUNC( close_callback ), NULL );
	gtk_signal_connect( GTK_OBJECT (pDialogWnd), "response",
                                  GTK_SIGNAL_FUNC( response_callback ), NULL );
// 	gtk_signal_connect( GTK_OBJECT (pDialogWnd), "expose_event", GTK_SIGNAL_FUNC( ci_expose ), NULL );

	pTooltips = gtk_tooltips_new();

	pNotebook = gtk_notebook_new();
	pPage = new CBackgroundDialogPage(XY);
	pPage->Append(pNotebook);
	pPage = new CBackgroundDialogPage(XZ);
	pPage->Append(pNotebook);
	pPage = new CBackgroundDialogPage(YZ);
	pPage->Append(pNotebook);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(pDialogWnd)->vbox), pNotebook, TRUE, TRUE, 0);

  gtk_widget_show ( pNotebook );
	
	gtk_widget_realize( pDialogWnd );
}

void ShowBackgroundDialog()
{
	gtk_window_present( GTK_WINDOW(pDialogWnd) );
}

void ShowBackgroundDialogPG(int page)
{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(pNotebook),page);
	ShowBackgroundDialog();
}

