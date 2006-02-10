/*
BobToolz plugin for GtkRadiant
Copyright (C) 2001 Gordon Biggans

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../StdAfx.h"


#include "dialogs-gtk.h"
#include "../funchandlers.h"

#include "str.h"
#include "gtkr_list.h"

#include "../lists.h"
#include "../misc.h"


/*--------------------------------
		Callback Functions
---------------------------------*/

typedef struct {
	GtkWidget *cbTexChange;
	GtkWidget *editTexOld, *editTexNew;

  GtkWidget *cbScaleHor, *cbScaleVert;
	GtkWidget *editScaleHor, *editScaleVert;

  GtkWidget *cbShiftHor, *cbShiftVert;
	GtkWidget *editShiftHor, *editShiftVert;

	GtkWidget *cbRotation;
	GtkWidget *editRotation;
}dlg_texReset_t;

dlg_texReset_t dlgTexReset;

void Update_TextureReseter();

static void dialog_button_callback_texreset_update (GtkWidget *widget, gpointer data)
{
  Update_TextureReseter();
}

void Update_TextureReseter()
{
  gboolean check;

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbTexChange ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editTexNew), check);
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editTexOld), check);

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleHor ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editScaleHor), check);

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleVert ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editScaleVert), check);

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftHor ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editShiftHor), check);

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftVert ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editShiftVert), check);

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbRotation ));
  gtk_entry_set_editable (GTK_ENTRY (dlgTexReset.editRotation), check);
}

static void dialog_button_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *parent;
	int *loop;
  EMessageBoxReturn *ret;
 
	parent = gtk_widget_get_toplevel (widget);
	loop = (int*)g_object_get_data (G_OBJECT (parent), "loop");
	ret = (EMessageBoxReturn*)g_object_get_data (G_OBJECT (parent), "ret");
 
	*loop = 0;
	*ret = (EMessageBoxReturn)GPOINTER_TO_INT(data);
}
 
static gint dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
	int *loop;
 
	gtk_widget_hide (widget);
	loop = (int*)g_object_get_data (G_OBJECT (widget), "loop");
	*loop = 0;

	return TRUE;
}

static void dialog_button_callback_settex (GtkWidget *widget, gpointer data)
{
	TwinWidget* tw = (TwinWidget*)data;

	GtkEntry* entry = GTK_ENTRY( tw->one );
	GtkCombo* combo = GTK_COMBO( tw->two );

	const gchar* tex = gtk_entry_get_text(GTK_ENTRY( combo->entry ));
	gtk_entry_set_text( entry, tex);
}

/*--------------------------------
	Data validation Routines
---------------------------------*/

bool ValidateTextFloat(const char* pData, char* error_title, float* value)
{
	if(pData)
	{
		float testNum = (float)atof(pData);

		if((testNum == 0.0f) && strcmp(pData, "0"))
		{
			DoMessageBox("Please Enter A Floating Point Number", error_title, eMB_OK);
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox("Please Enter A Floating Point Number", error_title, eMB_OK);
	return FALSE;
}

bool ValidateTextFloatRange(const char* pData, float min, float max, char* error_title, float* value)
{
	char error_buffer[256];
	sprintf(error_buffer, "Please Enter A Floating Point Number Between %.3f and %.3f", min, max);

	if(pData)
	{
		float testNum = (float)atof(pData);

		if((testNum < min) || (testNum > max))
		{
			DoMessageBox(error_buffer, error_title, eMB_OK);
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox(error_buffer, error_title, eMB_OK);
	return FALSE;
}

bool ValidateTextIntRange(const char* pData, int min, int max, char* error_title, int* value)
{
	char error_buffer[256];
	sprintf(error_buffer, "Please Enter An Integer Between %i and %i", min, max);

	if(pData)
	{
		int testNum = atoi(pData);

		if((testNum < min) || (testNum > max))
		{
			DoMessageBox(error_buffer, error_title, eMB_OK);
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox(error_buffer, error_title, eMB_OK);
	return FALSE;
}

bool ValidateTextInt(const char* pData, char* error_title, int* value)
{
	if(pData)
	{
		int testNum = atoi(pData);

		if((testNum == 0) && strcmp(pData, "0"))
		{
			DoMessageBox("Please Enter An Integer", error_title, eMB_OK);
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox("Please Enter An Integer", error_title, eMB_OK);
	return FALSE;
}

/*--------------------------------
		Modal Dialog Boxes
---------------------------------*/

/*

  Major clean up of variable names etc required, excluding Mars's ones,
  which are nicely done :)

*/

EMessageBoxReturn DoMessageBox (const char* lpText, const char* lpCaption, EMessageBoxType type)
{
	GtkWidget *window, *w, *vbox, *hbox;
	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
	gtk_window_set_title (GTK_WINDOW (window), lpCaption);
	gtk_container_border_width (GTK_CONTAINER (window), 10);
	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);
	gtk_widget_realize (window);
 
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	w = gtk_label_new (lpText);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
	gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
	gtk_widget_show (w);
 
	w = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
	gtk_widget_show (w);
 
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);
 
	if (type == eMB_OK)
	{
		w = gtk_button_new_with_label ("Ok");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
		ret = eIDOK;
	}
	else if (type ==  eMB_OKCANCEL)
	{
		w = gtk_button_new_with_label ("Ok");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("Cancel");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
		gtk_widget_show (w);
		ret = eIDCANCEL;
	}
	else if (type == eMB_YESNOCANCEL)
	{
		w = gtk_button_new_with_label ("Yes");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDYES));
		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("No");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDNO));
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("Cancel");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
		gtk_widget_show (w);
		ret = eIDCANCEL;
	}
	else /* if (mode == MB_YESNO) */
	{
		w = gtk_button_new_with_label ("Yes");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDYES));
		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("No");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDNO));
		gtk_widget_show (w);
		ret = eIDNO;
	}
 
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	while (loop)
		gtk_main_iteration ();
 
	gtk_grab_remove (window);
	gtk_widget_destroy (window);
 
	return ret;
}

EMessageBoxReturn DoIntersectBox (IntersectRS* rs)
{
	GtkWidget *window, *w, *vbox, *hbox;
	GtkWidget *radio1, *radio2;
	GtkWidget *check1, *check2;
	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "Intersect");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
 

	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

		radio1 = gtk_radio_button_new_with_label(NULL, "Use Whole Map");
		gtk_box_pack_start (GTK_BOX (vbox), radio1, FALSE, FALSE, 2);
		gtk_widget_show (radio1);

		radio2 = gtk_radio_button_new_with_label(((GtkRadioButton*)radio1)->group, "Use Selected Brushes");
		gtk_box_pack_start (GTK_BOX (vbox), radio2, FALSE, FALSE, 2);
		gtk_widget_show (radio2);

		w = gtk_hseparator_new ();
		gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
		gtk_widget_show (w);

		check1 = gtk_check_button_new_with_label("Include Detail Brushes");
		gtk_box_pack_start (GTK_BOX (vbox), check1, FALSE, FALSE, 0);
		gtk_widget_show (check1);		

		check2 = gtk_check_button_new_with_label("Select Duplicate Brushes Only");
		gtk_box_pack_start (GTK_BOX (vbox), check2, FALSE, FALSE, 0);
		gtk_widget_show (check2);		

		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ---- ok/cancel buttons

		w = gtk_button_new_with_label ("Ok");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));

		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("Cancel");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
		gtk_widget_show (w);
		ret = eIDCANCEL;

		// ---- /hbox ----
 
	// ---- /vbox ----

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	while (loop)
		gtk_main_iteration ();

	if(gtk_toggle_button_get_active((GtkToggleButton*)radio1))
		rs->nBrushOptions = BRUSH_OPT_WHOLE_MAP;
	else if(gtk_toggle_button_get_active((GtkToggleButton*)radio2))
		rs->nBrushOptions = BRUSH_OPT_SELECTED;

  rs->bUseDetail = gtk_toggle_button_get_active((GtkToggleButton*)check1) ? true : false;
	rs->bDuplicateOnly = gtk_toggle_button_get_active((GtkToggleButton*)check2) ? true : false;
 
	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}

EMessageBoxReturn DoPolygonBox (PolygonRS* rs)
{
	GtkWidget *window, *w, *vbox, *hbox, *vbox2, *hbox2;

	GtkWidget *check1, *check2, *check3;
	GtkWidget *text1, *text2;

	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "Polygon Builder");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
 

	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

	
			vbox2 = gtk_vbox_new (FALSE, 10);
			gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 2);
			gtk_widget_show (vbox2);
 
			// ---- vbox2 ----

				hbox2 = gtk_hbox_new (FALSE, 10);
				gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 2);
				gtk_widget_show (hbox2);

				// ---- hbox2 ----

					text1 = gtk_entry_new_with_max_length(256);
					gtk_entry_set_text((GtkEntry*)text1, "3");
					gtk_box_pack_start (GTK_BOX (hbox2), text1, FALSE, FALSE, 2);
					gtk_widget_show (text1);

					w = gtk_label_new ("Number Of Sides");
					gtk_box_pack_start (GTK_BOX (hbox2), w, FALSE, FALSE, 2);
					gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
					gtk_widget_show (w);

				// ---- /hbox2 ----

				hbox2 = gtk_hbox_new (FALSE, 10);
				gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 2);
				gtk_widget_show (hbox2);

				// ---- hbox2 ----

					text2 = gtk_entry_new_with_max_length(256);
					gtk_entry_set_text((GtkEntry*)text2, "8");
					gtk_box_pack_start (GTK_BOX (hbox2), text2, FALSE, FALSE, 2);
					gtk_widget_show (text2);

					w = gtk_label_new ("Border Width");
					gtk_box_pack_start (GTK_BOX (hbox2), w, FALSE, FALSE, 2);
					gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
					gtk_widget_show (w);
				
				// ---- /hbox2 ----

			// ---- /vbox2 ----


			
			vbox2 = gtk_vbox_new (FALSE, 10);
			gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 2);
			gtk_widget_show (vbox2);
 
			// ---- vbox2 ----

				check1 = gtk_check_button_new_with_label("Use Border");
				gtk_box_pack_start (GTK_BOX (vbox2), check1, FALSE, FALSE, 0);
				gtk_widget_show (check1);
 

				check2 = gtk_check_button_new_with_label("Inverse Polygon");
				gtk_box_pack_start (GTK_BOX (vbox2), check2, FALSE, FALSE, 0);
				gtk_widget_show (check2);
 

				check3 = gtk_check_button_new_with_label("Align Top Edge");
				gtk_box_pack_start (GTK_BOX (vbox2), check3, FALSE, FALSE, 0);
				gtk_widget_show (check3);
 
			// ---- /vbox2 ----

		// ---- /hbox ----
 
		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

			w = gtk_button_new_with_label ("Ok");
			gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
			gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));

			GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
			gtk_widget_grab_default (w);
			gtk_widget_show (w);
 
			w = gtk_button_new_with_label ("Cancel");
			gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
			gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
			gtk_widget_show (w);
			ret = eIDCANCEL;

		// ---- /hbox ----
	
	// ---- /vbox ----

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	bool dialogError = TRUE;
	while (dialogError)
	{
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if(ret == eIDOK)
		{
			rs->bUseBorder = gtk_toggle_button_get_active((GtkToggleButton*)check1) ? true : false;
			rs->bInverse = gtk_toggle_button_get_active((GtkToggleButton*)check2) ? true : false;
			rs->bAlignTop = gtk_toggle_button_get_active((GtkToggleButton*)check3) ? true : false;

			if(!ValidateTextIntRange(gtk_entry_get_text((GtkEntry*)text1), 3, 32, "Number Of Sides", &rs->nSides))
				dialogError = TRUE;

			if(rs->bUseBorder)
			{
				if(!ValidateTextIntRange(gtk_entry_get_text((GtkEntry*)text2), 8, 256, "Border Width", &rs->nBorderWidth))
					dialogError = TRUE;
			}
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}

// mars
// for stair builder stuck as close as i could to the MFC version
// obviously feel free to change it at will :)
EMessageBoxReturn DoBuildStairsBox(BuildStairsRS* rs)
{
	// i made widgets for just about everything ... i think that's what i need to do  dunno tho 
	GtkWidget	*window, *w, *vbox, *hbox;
	GtkWidget	*textStairHeight, *textRiserTex, *textMainTex;
	GtkWidget	*radioNorth, *radioSouth, *radioEast, *radioWest;	// i'm guessing we can't just abuse 'w' for these if we're getting a value
	GtkWidget	*radioOldStyle, *radioBobStyle, *radioCornerStyle;
	GtkWidget	*checkUseDetail;
	GSList		*radioDirection, *radioStyle;
	EMessageBoxReturn ret;
  int loop = 1;

	char	*text = "Please set a value in the boxes below and press 'OK' to build the stairs";

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title( GTK_WINDOW( window ), "Stair Builder" );
	
	gtk_container_border_width( GTK_CONTAINER( window ), 10 );

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);

	// new vbox 
	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( vbox ), hbox );
	gtk_widget_show( hbox );
	
	// dunno if you want this text or not ...
	w = gtk_label_new( text );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 ); // not entirely sure on all the parameters / what they do ...
	gtk_widget_show( w );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );
	
	// ------------------------- // indenting == good way of keeping track of lines :)

		// new hbox
		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );
	
		textStairHeight = gtk_entry_new_with_max_length( 256 ); 
		gtk_box_pack_start( GTK_BOX( hbox ), textStairHeight, FALSE, FALSE, 1 );
		gtk_widget_show( textStairHeight );

		w = gtk_label_new( "Stair Height" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
		gtk_widget_show( w );

	// ------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( "Direction:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 5 );
	gtk_widget_show( w );

	// -------------------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );
		
		// radio buttons confuse me ...
		// but this _looks_ right

		// djbob: actually it looks very nice :), slightly better than the way i did it
		// edit: actually it doesn't work :P, you must pass the last radio item each time, ugh

		radioNorth = gtk_radio_button_new_with_label( NULL, "North" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioNorth, FALSE, FALSE, 3 );
		gtk_widget_show( radioNorth );

		radioDirection = gtk_radio_button_group( GTK_RADIO_BUTTON( radioNorth ) );
		
		radioSouth = gtk_radio_button_new_with_label( radioDirection, "South" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioSouth, FALSE, FALSE, 2 );
		gtk_widget_show( radioSouth );

		radioDirection = gtk_radio_button_group( GTK_RADIO_BUTTON( radioSouth ) );

		radioEast = gtk_radio_button_new_with_label( radioDirection, "East" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioEast, FALSE, FALSE, 1 );
		gtk_widget_show( radioEast );

		radioDirection = gtk_radio_button_group( GTK_RADIO_BUTTON( radioEast ) );

		radioWest = gtk_radio_button_new_with_label( radioDirection, "West" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioWest, FALSE, FALSE, 0 );
		gtk_widget_show( radioWest );

	// --------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( "Style:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 5 );
	gtk_widget_show( w );

	// --------------------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );
	
		radioOldStyle = gtk_radio_button_new_with_label( NULL, "Original" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioOldStyle, FALSE, FALSE, 0 );
		gtk_widget_show( radioOldStyle );

		radioStyle = gtk_radio_button_group( GTK_RADIO_BUTTON( radioOldStyle ) );

		radioBobStyle = gtk_radio_button_new_with_label( radioStyle, "Bob's Style" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioBobStyle, FALSE, FALSE, 0 );
		gtk_widget_show( radioBobStyle );
	
		radioStyle = gtk_radio_button_group( GTK_RADIO_BUTTON( radioBobStyle ) );

		radioCornerStyle = gtk_radio_button_new_with_label( radioStyle, "Corner Style" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioCornerStyle, FALSE, FALSE, 0 );
		gtk_widget_show( radioCornerStyle );
	
		// err, the q3r has an if or something so you need bob style checked before this 
		// is "ungreyed out" but you'll need to do that, as i suck :)

		// djbob: er.... yeah um, im not at all sure how i'm gonna sort this
		// djbob: think we need some button callback functions or smuffin
		// FIXME: actually get around to doing what i suggested!!!!

		checkUseDetail = gtk_check_button_new_with_label( "Use Detail Brushes" );
		gtk_box_pack_start( GTK_BOX( hbox ), checkUseDetail, FALSE, FALSE, 0 );
		gtk_widget_show( checkUseDetail );

	// --------------------------- // 

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textMainTex = gtk_entry_new_with_max_length( 512 );
	gtk_entry_set_text(GTK_ENTRY(textMainTex), rs->mainTexture);
	gtk_box_pack_start( GTK_BOX( hbox ), textMainTex, FALSE, FALSE, 0 );
	gtk_widget_show( textMainTex );

	w = gtk_label_new( "Main Texture" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
	gtk_widget_show( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textRiserTex = gtk_entry_new_with_max_length( 512 );
	gtk_box_pack_start( GTK_BOX( hbox ), textRiserTex, FALSE, FALSE, 0 );
	gtk_widget_show( textRiserTex );

	w = gtk_label_new( "Riser Texture" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
	gtk_widget_show( w );

	// -------------------------- //
	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( "OK" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDOK ) );
	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );
		
	w = gtk_button_new_with_label( "Cancel" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDCANCEL ) );
	gtk_widget_show( w );
		
	ret = eIDCANCEL;

// +djbob: need our "little" modal loop mars :P
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	bool dialogError = TRUE;
	while (dialogError)
	{
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if(ret == eIDOK)
		{
			rs->bUseDetail = gtk_toggle_button_get_active((GtkToggleButton*)checkUseDetail) ? true : false;

			strcpy(rs->riserTexture, gtk_entry_get_text((GtkEntry*)textRiserTex));
			strcpy(rs->mainTexture, gtk_entry_get_text((GtkEntry*)textMainTex));

			if(gtk_toggle_button_get_active((GtkToggleButton*)radioNorth))
				rs->direction = MOVE_NORTH;
			else if(gtk_toggle_button_get_active((GtkToggleButton*)radioSouth))
				rs->direction = MOVE_SOUTH;
			else if(gtk_toggle_button_get_active((GtkToggleButton*)radioEast))
				rs->direction = MOVE_EAST;
			else if(gtk_toggle_button_get_active((GtkToggleButton*)radioWest))
				rs->direction = MOVE_WEST;

			if(!ValidateTextInt(gtk_entry_get_text((GtkEntry*)textStairHeight), "Stair Height", &rs->stairHeight))
				dialogError = TRUE;

			if(gtk_toggle_button_get_active((GtkToggleButton*)radioOldStyle))
				rs->style = STYLE_ORIGINAL;
			else if(gtk_toggle_button_get_active((GtkToggleButton*)radioBobStyle))
				rs->style = STYLE_BOB;
			else if(gtk_toggle_button_get_active((GtkToggleButton*)radioCornerStyle))
				rs->style = STYLE_CORNER;
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
// -djbob
	
	// there we go, all done ... on my end at least, not bad for a night's work
}

EMessageBoxReturn DoDoorsBox(DoorRS* rs)
{
	GtkWidget	*window, *hbox, *vbox, *w;
	GtkWidget	*textFrontBackTex, *textTrimTex;
	GtkWidget	*checkScaleMainH, *checkScaleMainV, *checkScaleTrimH, *checkScaleTrimV;
	GtkWidget	*comboMain, *comboTrim;
	GtkWidget	*buttonSetMain, *buttonSetTrim;
	GtkWidget	*radioNS, *radioEW; 
	GSList		*radioOrientation;
	TwinWidget	tw1, tw2;
	EMessageBoxReturn ret;
  int loop = 1;
	
	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title( GTK_WINDOW( window ), "Door Builder" );
	
	gtk_container_border_width( GTK_CONTAINER( window ), 10 );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize (window);
	
	char buffer[256];
	GList		*listMainTextures = NULL;
	GList		*listTrimTextures = NULL;
	LoadGList(GetFilename(buffer, "plugins/bt/door-tex.txt"), &listMainTextures);
	LoadGList(GetFilename(buffer, "plugins/bt/door-tex-trim.txt"), &listTrimTextures);
	
	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		textFrontBackTex = gtk_entry_new_with_max_length( 512 );
		gtk_entry_set_text( GTK_ENTRY( textFrontBackTex ), rs->mainTexture);
		gtk_box_pack_start( GTK_BOX( hbox ), textFrontBackTex, FALSE, FALSE, 0 );
		gtk_widget_show( textFrontBackTex );

		w = gtk_label_new( "Door Front/Back Texture" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

	// ------------------------ //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		textTrimTex = gtk_entry_new_with_max_length( 512 );
		gtk_box_pack_start( GTK_BOX( hbox ), textTrimTex, FALSE, FALSE, 0 );
		gtk_widget_show( textTrimTex );

		w = gtk_label_new( "Door Trim Texture" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

	// ----------------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		// sp: horizontally ????
		// djbob: yes mars, u can spell :]
		checkScaleMainH = gtk_check_button_new_with_label( "Scale Main Texture Horizontally" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( checkScaleMainH ), TRUE);
		gtk_box_pack_start( GTK_BOX( hbox ), checkScaleMainH, FALSE, FALSE, 0 );
		gtk_widget_show( checkScaleMainH );

		checkScaleTrimH = gtk_check_button_new_with_label( "Scale Trim Texture Horizontally" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( checkScaleTrimH ), TRUE);
		gtk_box_pack_start( GTK_BOX( hbox ), checkScaleTrimH, FALSE, FALSE, 0 );
		gtk_widget_show( checkScaleTrimH );

	// ---------------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		checkScaleMainV = gtk_check_button_new_with_label( "Scale Main Texture Vertically" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( checkScaleMainV ), TRUE);
		gtk_box_pack_start( GTK_BOX( hbox ), checkScaleMainV, FALSE, FALSE, 0 );
		gtk_widget_show( checkScaleMainV );

		checkScaleTrimV = gtk_check_button_new_with_label( "Scale Trim Texture Vertically" );
		gtk_box_pack_start( GTK_BOX( hbox ), checkScaleTrimV, FALSE, FALSE, 0 );
		gtk_widget_show( checkScaleTrimV );
			
	// --------------------- // 

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		// djbob: lists added

		comboMain = gtk_combo_new();
		gtk_box_pack_start( GTK_BOX( hbox ), comboMain, FALSE, FALSE, 0 );
		gtk_combo_set_popdown_strings( GTK_COMBO( comboMain ), listMainTextures );
		gtk_combo_set_use_arrows( GTK_COMBO( comboMain ), 1 );
		gtk_widget_show( comboMain );

		tw1.one = textFrontBackTex;
		tw1.two = comboMain;

		buttonSetMain = gtk_button_new_with_label( "Set As Main Texture" );
		gtk_signal_connect( GTK_OBJECT( buttonSetMain ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback_settex ), &tw1 );
		gtk_box_pack_start( GTK_BOX( hbox ), buttonSetMain, FALSE, FALSE, 0 );
		gtk_widget_show( buttonSetMain );

	// ------------------- //
		
		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		comboTrim = gtk_combo_new();
		gtk_box_pack_start( GTK_BOX( hbox ), comboTrim, FALSE, FALSE, 0 );
		gtk_combo_set_popdown_strings( GTK_COMBO( comboTrim ), listTrimTextures );
		gtk_combo_set_use_arrows( GTK_COMBO( comboMain ), 1 );
		gtk_widget_show( comboTrim );

		tw2.one = textTrimTex;
		tw2.two = comboTrim;

		buttonSetTrim = gtk_button_new_with_label( "Set As Trim Texture" );
		gtk_signal_connect( GTK_OBJECT( buttonSetTrim ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback_settex ), &tw2 );
		gtk_box_pack_start( GTK_BOX( hbox ), buttonSetTrim, FALSE, FALSE, 0 );
		gtk_widget_show( buttonSetTrim );

	// ------------------ //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		w = gtk_label_new( "Orientation" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

		// argh more radio buttons! 
		radioNS = gtk_radio_button_new_with_label( NULL, "North - South" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioNS, FALSE, FALSE, 0 );
		gtk_widget_show( radioNS );

		radioOrientation = gtk_radio_button_group( GTK_RADIO_BUTTON( radioNS ) );

		radioEW = gtk_radio_button_new_with_label( radioOrientation, "East - West" );
		gtk_box_pack_start( GTK_BOX( hbox ), radioEW, FALSE, FALSE, 0 );
		gtk_widget_show( radioEW );

	// ----------------- //

		w = gtk_hseparator_new();
		gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

	// ----------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		w = gtk_button_new_with_label( "OK" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0);
		gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDOK ) );
		GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
		gtk_widget_grab_default( w );
		gtk_widget_show( w );
		
		w = gtk_button_new_with_label( "Cancel" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
		gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDCANCEL ) );
		gtk_widget_show( w );
		ret = eIDCANCEL;

	// ----------------- //

//+djbob
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	while (loop)
		gtk_main_iteration ();

	strcpy(rs->mainTexture, gtk_entry_get_text( GTK_ENTRY( textFrontBackTex ) ));
	strcpy(rs->trimTexture, gtk_entry_get_text( GTK_ENTRY( textTrimTex ) ));

	rs->bScaleMainH = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkScaleMainH)) ? true : false;
	rs->bScaleMainV = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkScaleMainV)) ? true : false;
	rs->bScaleTrimH = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkScaleTrimH)) ? true : false;
	rs->bScaleTrimV = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkScaleTrimV)) ? true : false;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioNS)))
		rs->nOrientation = DIRECTION_NS;
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioEW)))
		rs->nOrientation = DIRECTION_EW;
 
	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
//-djbob
}

EMessageBoxReturn DoPathPlotterBox(PathPlotterRS* rs)
{
	GtkWidget *window, *w, *vbox, *hbox;

	GtkWidget *text1, *text2, *text3;
	GtkWidget *check1, *check2;

	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "Texture Reset");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
 

	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

			text1 = gtk_entry_new_with_max_length(256);
			gtk_entry_set_text((GtkEntry*)text1, "25");
			gtk_box_pack_start (GTK_BOX (hbox), text1, FALSE, FALSE, 2);
			gtk_widget_show (text1);

			w = gtk_label_new ("Number Of Points");
			gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 2);
			gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
			gtk_widget_show (w);

		// ---- /hbox ----

		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

			text2 = gtk_entry_new_with_max_length(256);
			gtk_entry_set_text((GtkEntry*)text2, "3");
			gtk_box_pack_start (GTK_BOX (hbox), text2, FALSE, FALSE, 2);
			gtk_widget_show (text2);

			w = gtk_label_new ("Multipler");
			gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 2);
			gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
			gtk_widget_show (w);

		// ---- /hbox ----

		w = gtk_label_new ("Path Distance = dist(start -> apex) * multiplier");
		gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 0);
		gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
		gtk_widget_show (w);

		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

			text3 = gtk_entry_new_with_max_length(256);
			gtk_entry_set_text((GtkEntry*)text3, "-800");
			gtk_box_pack_start (GTK_BOX (hbox), text3, FALSE, FALSE, 2);
			gtk_widget_show (text3);

			w = gtk_label_new ("Gravity");
			gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 2);
			gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
			gtk_widget_show (w);

		// ---- /hbox ----
 	
		w = gtk_hseparator_new();
		gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

		check1 = gtk_check_button_new_with_label( "No Dynamic Update" );
		gtk_box_pack_start( GTK_BOX( vbox ), check1, FALSE, FALSE, 0 );
		gtk_widget_show( check1 );

		check2 = gtk_check_button_new_with_label( "Show Bounding Lines" );
		gtk_box_pack_start( GTK_BOX( vbox ), check2, FALSE, FALSE, 0 );
		gtk_widget_show( check2 );

	// ---- /vbox ----


	// ----------------- //

		w = gtk_hseparator_new();
		gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
		gtk_widget_show( w );

	// ----------------- //

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );

		w = gtk_button_new_with_label( "Enable" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0);
		gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDYES ) );
		gtk_widget_show( w );
		
		GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
		gtk_widget_grab_default( w );

		w = gtk_button_new_with_label( "Disable" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0);
		gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDNO ) );
		gtk_widget_show( w );
		
		w = gtk_button_new_with_label( "Cancel" );
		gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
		gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( dialog_button_callback ), GINT_TO_POINTER( eIDCANCEL ) );
		gtk_widget_show( w );
		
		ret = eIDCANCEL;

	// ----------------- //

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	bool dialogError = TRUE;
	while (dialogError)
	{
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if(ret == eIDYES)
		{
			if(!ValidateTextIntRange(gtk_entry_get_text(GTK_ENTRY(text1)), 1, 200, "Number Of Points", &rs->nPoints))
				dialogError = TRUE;

			if(!ValidateTextFloatRange(gtk_entry_get_text(GTK_ENTRY(text2)), 1.0f, 10.0f, "Multiplier", &rs->fMultiplier))
				dialogError = TRUE;

			if(!ValidateTextFloatRange(gtk_entry_get_text(GTK_ENTRY(text3)), -10000.0f, -1.0f, "Gravity", &rs->fGravity))
				dialogError = TRUE;

			rs->bNoUpdate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check1)) ? true : false;
			rs->bShowExtra = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check2)) ? true : false;
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}

EMessageBoxReturn DoCTFColourChangeBox ()
{
	GtkWidget *window, *w, *vbox, *hbox;
	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "CTF Colour Changer");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
 

	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

		hbox = gtk_hbox_new( FALSE, 10 );
		gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
		gtk_widget_show( hbox );

 		// ---- hbox ---- ok/cancel buttons

		w = gtk_button_new_with_label ("Red->Blue");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));

		GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
		gtk_widget_grab_default (w);
		gtk_widget_show (w);
 
		w = gtk_button_new_with_label ("Blue->Red");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDYES));
		gtk_widget_show (w);

		w = gtk_button_new_with_label ("Cancel");
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
		gtk_widget_show (w);
		ret = eIDCANCEL;

		// ---- /hbox ----
 
	// ---- /vbox ----

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	while (loop)
		gtk_main_iteration ();

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}

EMessageBoxReturn DoResetTextureBox (ResetTextureRS* rs)
{
	Str texSelected;

	GtkWidget *window, *w, *vbox, *hbox, *frame, *table;

	EMessageBoxReturn ret;
  int loop = 1;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "Texture Reset");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);
 	
	// ---- hbox ----

	texSelected = "Currently Selected Face:   ";
#if 0
	if(g_SelectedFaceTable.m_pfnGetSelectedFaceCount() == 1) {
		texSelected += GetCurrentTexture();
	}
#endif

	w = gtk_label_new (texSelected);
	gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 2);
	gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
	gtk_widget_show (w);

	// ---- /hbox ----

	frame = gtk_frame_new ("Reset Texture Names");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	dlgTexReset.cbTexChange = gtk_check_button_new_with_label("Enabled");
	gtk_signal_connect (GTK_OBJECT (dlgTexReset.cbTexChange), "toggled", GTK_SIGNAL_FUNC (dialog_button_callback_texreset_update), NULL);
	gtk_widget_show (dlgTexReset.cbTexChange);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbTexChange, 0, 1, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("Old Name: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editTexOld = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editTexOld), rs->textureName);
    gtk_table_attach (GTK_TABLE (table), dlgTexReset.editTexOld, 2, 3, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editTexOld);

	w = gtk_label_new ("New Name: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editTexNew = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editTexNew), rs->textureName);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editTexNew, 2, 3, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editTexNew);

	// ---- /frame ----

    frame = gtk_frame_new ("Reset Scales");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	dlgTexReset.cbScaleHor = gtk_check_button_new_with_label("Enabled");
	gtk_signal_connect (GTK_OBJECT (dlgTexReset.cbScaleHor), "toggled", GTK_SIGNAL_FUNC (dialog_button_callback_texreset_update), NULL);
	gtk_widget_show (dlgTexReset.cbScaleHor);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbScaleHor, 0, 1, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("New Horizontal Scale: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editScaleHor = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editScaleHor), "0.5");
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editScaleHor, 2, 3, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editScaleHor);


	dlgTexReset.cbScaleVert = gtk_check_button_new_with_label("Enabled");
	gtk_signal_connect (GTK_OBJECT (dlgTexReset.cbScaleVert), "toggled", GTK_SIGNAL_FUNC (dialog_button_callback_texreset_update), NULL);
	gtk_widget_show (dlgTexReset.cbScaleVert);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbScaleVert, 0, 1, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("New Vertical Scale: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editScaleVert = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editScaleVert), "0.5");
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editScaleVert, 2, 3, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editScaleVert);

	// ---- /frame ----

    frame = gtk_frame_new ("Reset Shift");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

		// ---- frame ----

	dlgTexReset.cbShiftHor = gtk_check_button_new_with_label("Enabled");
	gtk_signal_connect (GTK_OBJECT (dlgTexReset.cbShiftHor), "toggled", GTK_SIGNAL_FUNC (dialog_button_callback_texreset_update), NULL);
	gtk_widget_show (dlgTexReset.cbShiftHor);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbShiftHor, 0, 1, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("New Horizontal Shift: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editShiftHor = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editShiftHor), "0");
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editShiftHor, 2, 3, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editShiftHor);


	dlgTexReset.cbShiftVert = gtk_check_button_new_with_label("Enabled");
	gtk_signal_connect (GTK_OBJECT (dlgTexReset.cbShiftVert), "toggled", GTK_SIGNAL_FUNC (dialog_button_callback_texreset_update), NULL);
	gtk_widget_show (dlgTexReset.cbShiftVert);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbShiftVert, 0, 1, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("New Vertical Shift: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editShiftVert = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editShiftVert), "0");
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editShiftVert, 2, 3, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editShiftVert);

	// ---- /frame ----

    frame = gtk_frame_new ("Reset Rotation");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (1, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

		// ---- frame ----

	dlgTexReset.cbRotation = gtk_check_button_new_with_label("Enabled");
	gtk_widget_show (dlgTexReset.cbRotation);
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.cbRotation, 0, 1, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);

	w = gtk_label_new ("New Rotation Value: ");
	gtk_table_attach (GTK_TABLE (table), w, 1, 2, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	dlgTexReset.editRotation = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(dlgTexReset.editRotation), "0");
	gtk_table_attach (GTK_TABLE (table), dlgTexReset.editRotation, 2, 3, 0, 1,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (dlgTexReset.editRotation);

	// ---- /frame ----

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);
 	
	// ---- hbox ----

	w = gtk_button_new_with_label ("Use Selected Brushes");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));

	GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (w);
	gtk_widget_show (w);
 
	w = gtk_button_new_with_label ("Use All Brushes");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDYES));
	gtk_widget_show (w);

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
	gtk_widget_show (w);
	ret = eIDCANCEL;

	// ---- /hbox ----
	
	// ---- /vbox ----

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	Update_TextureReseter();

	bool dialogError = TRUE;
	while (dialogError)
	{
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if(ret != eIDCANCEL)
		{
			rs->bResetRotation =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbRotation ));
			if(rs->bResetRotation)
				if(!ValidateTextInt(gtk_entry_get_text(GTK_ENTRY(dlgTexReset.editRotation)), "Rotation", &rs->rotation))
					dialogError = TRUE;

			rs->bResetScale[0] =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleHor ));
			if(rs->bResetScale[0])
				if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(dlgTexReset.editScaleHor)), "Horizontal Scale", &rs->fScale[0]))
					dialogError = TRUE;

			rs->bResetScale[1] =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleVert ));
			if(rs->bResetScale[1])
				if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(dlgTexReset.editScaleVert)), "Vertical Scale", &rs->fScale[1]))
					dialogError = TRUE;

			rs->bResetShift[0] =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftHor ));
			if(rs->bResetShift[0])
				if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(dlgTexReset.editShiftHor)), "Horizontal Shift", &rs->fShift[0]))
					dialogError = TRUE;

			rs->bResetShift[1] =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftVert ));
			if(rs->bResetShift[1])
				if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(dlgTexReset.editShiftVert)), "Vertical Shift", &rs->fShift[1]))
					dialogError = TRUE;

			rs->bResetTextureName =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( dlgTexReset.cbTexChange ));
			if(rs->bResetTextureName)
			{
				strcpy(rs->textureName,     gtk_entry_get_text(GTK_ENTRY( dlgTexReset.editTexOld )));
				strcpy(rs->newTextureName,  gtk_entry_get_text(GTK_ENTRY( dlgTexReset.editTexNew )));
			}
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}

EMessageBoxReturn DoTrainThingBox (TrainThingRS* rs)
{
	Str texSelected;

	GtkWidget *window, *w, *vbox, *hbox, *frame, *table;

	GtkWidget *radiusX, *radiusY;
	GtkWidget *angleStart, *angleEnd;
	GtkWidget *heightStart, *heightEnd;
	GtkWidget *numPoints;

	EMessageBoxReturn ret;
  int loop = 1;
 
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "Train Thing");
	gtk_container_border_width (GTK_CONTAINER (window), 10);

	gtk_object_set_data (GTK_OBJECT (window), "loop", &loop);
	gtk_object_set_data (GTK_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);
	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
 
	// ---- vbox ----

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);

	// ---- /hbox ----

	frame = gtk_frame_new ("Radii");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	w = gtk_label_new ("X: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	radiusX = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(radiusX), "100");
    gtk_table_attach (GTK_TABLE (table), radiusX, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (radiusX);



	w = gtk_label_new ("Y: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	radiusY = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(radiusY), "100");
	gtk_table_attach (GTK_TABLE (table), radiusY, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (radiusY);



	frame = gtk_frame_new ("Angles");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	w = gtk_label_new ("Start: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	angleStart = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(angleStart), "0");
    gtk_table_attach (GTK_TABLE (table), angleStart, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (angleStart);



	w = gtk_label_new ("End: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	angleEnd = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(angleEnd), "90");
	gtk_table_attach (GTK_TABLE (table), angleEnd, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (angleEnd);


	frame = gtk_frame_new ("Height");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	w = gtk_label_new ("Start: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	heightStart = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(heightStart), "0");
    gtk_table_attach (GTK_TABLE (table), heightStart, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (heightStart);



	w = gtk_label_new ("End: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	heightEnd = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(heightEnd), "0");
	gtk_table_attach (GTK_TABLE (table), heightEnd, 1, 2, 1, 2,
		        (GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (heightEnd);



	frame = gtk_frame_new ("Points");
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);

    table = gtk_table_new (2, 3, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	// ---- frame ----

	w = gtk_label_new ("Number: ");
	gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (w);

	numPoints = gtk_entry_new_with_max_length(256);
	gtk_entry_set_text(GTK_ENTRY(numPoints), "0");
    gtk_table_attach (GTK_TABLE (table), numPoints, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_FILL),
		        (GtkAttachOptions) (0), 0, 0);
	gtk_widget_show (numPoints);


		hbox = gtk_hbox_new (FALSE, 10);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
		gtk_widget_show (hbox);
 	
		// ---- hbox ----

			w = gtk_button_new_with_label ("Ok");
			gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
			gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));

			GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
			gtk_widget_grab_default (w);
			gtk_widget_show (w);
 
			w = gtk_button_new_with_label ("Cancel");
			gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
			gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
			gtk_widget_show (w);
			ret = eIDCANCEL;

		// ---- /hbox ----
	


	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);
 
	bool dialogError = TRUE;
	while (dialogError)
	{
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if(ret != eIDCANCEL)
		{
			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(radiusX)), "Radius (X)", &rs->fRadiusX))
				dialogError = TRUE;

			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(radiusY)), "Radius (Y)", &rs->fRadiusY))
				dialogError = TRUE;

			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(angleStart)), "Angle (Start)", &rs->fStartAngle))
				dialogError = TRUE;

			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(angleEnd)), "Angle (End)", &rs->fEndAngle))
				dialogError = TRUE;

			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(heightStart)), "Height (Start)", &rs->fStartHeight))
				dialogError = TRUE;

			if(!ValidateTextFloat(gtk_entry_get_text(GTK_ENTRY(heightEnd)), "Height (End)", &rs->fEndHeight))
				dialogError = TRUE;

			if(!ValidateTextInt(gtk_entry_get_text(GTK_ENTRY(numPoints)), "Num Points", &rs->iNumPoints))
				dialogError = TRUE;
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;
}
