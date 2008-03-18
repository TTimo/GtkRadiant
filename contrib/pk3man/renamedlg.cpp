// RenameDlg.cpp: implementation of the CRenameDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "renamedlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static void rename_dialog_button_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *parent;
	int *loop, *ret;

	parent = gtk_widget_get_toplevel (widget);
	loop = (int*)gtk_object_get_data (GTK_OBJECT (parent), "loop");
	ret = (int*)gtk_object_get_data (GTK_OBJECT (parent), "ret");

	*loop = 0;
	*ret = (int)data;
}

static gint rename_dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
	int *loop;

	gtk_widget_hide (widget);
	loop = (int*)gtk_object_get_data (GTK_OBJECT (widget), "loop");
	*loop = 0;

	return TRUE;
}

CRenameDlg::CRenameDlg()
{
	m_Name="foo";
}

CRenameDlg::~CRenameDlg()
{

}

int CRenameDlg::DoModal()
{
	GtkWidget *window, *w, *vbox, *hbox, *edit;
	int ret, loop = 1;
	char *lpCaption="Rename File";

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
					GTK_SIGNAL_FUNC (rename_dialog_delete_callback), NULL);
	gtk_signal_connect (GTK_OBJECT (window), "destroy",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
	gtk_window_set_title (GTK_WINDOW (window), lpCaption);
	gtk_container_border_width (GTK_CONTAINER (window), 10);
	gtk_object_set_data (GTK_OBJECT (window), "loop", &loop);
	gtk_object_set_data (GTK_OBJECT (window), "ret", &ret);
	gtk_widget_realize (window);

	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);

	edit = gtk_entry_new ();
	gtk_entry_set_text(GTK_ENTRY(edit),m_Name.c_str());
	gtk_box_pack_start (GTK_BOX (vbox), edit, FALSE, FALSE, 2);
	gtk_widget_show (edit);

	w = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
	gtk_widget_show (w);

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);

	// buttons

	w = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked",
					GTK_SIGNAL_FUNC (rename_dialog_button_callback), GINT_TO_POINTER (IDOK));
	GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (w);
	gtk_widget_show (w);

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked",
					GTK_SIGNAL_FUNC (rename_dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
	gtk_widget_show (w);
	ret = IDCANCEL;

	// show it

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);

	while (loop)
		gtk_main_iteration ();

	m_Name=gtk_entry_get_text(GTK_ENTRY(edit));

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

	return ret;

}
