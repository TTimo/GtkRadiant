/*
PrtView plugin for GtkRadiant
Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

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

// LoadPortalFileDialog.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif

#ifdef GTK_PLUGIN

static void dialog_button_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *parent;
  int *loop, *ret;

  parent = gtk_widget_get_toplevel (widget);
  loop = (int*)g_object_get_data (G_OBJECT (parent), "loop");
  ret = (int*)g_object_get_data (G_OBJECT (parent), "ret");

  *loop = 0;
  *ret = (int)data;
}

static gint dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  int *loop;

  gtk_widget_hide (widget);
  loop = (int*)g_object_get_data (G_OBJECT (widget), "loop");
  *loop = 0;

  return TRUE;
}

static void file_sel_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *parent;
  int *loop;
  char **filename;
 
  parent = gtk_widget_get_toplevel (widget);
  loop = (int*)g_object_get_data (G_OBJECT (parent), "loop");
  filename = (char**)g_object_get_data (G_OBJECT (parent), "filename");
 
  *loop = 0;
  if ((int)data == IDOK)
    *filename = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (parent)));
}

static void change_clicked (GtkWidget *widget, gpointer data)
{
  GtkWidget* file_sel;
  char* filename = NULL;
  int loop = 1;

  file_sel = gtk_file_selection_new ("Locate portal (.prt) file");
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->ok_button), "clicked",
                      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDOK));
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_sel)->cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (file_sel_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_signal_connect (GTK_OBJECT (file_sel), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (file_sel));

  g_object_set_data (G_OBJECT (file_sel), "loop", &loop);
  g_object_set_data (G_OBJECT (file_sel), "filename", &filename);
  gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_sel), portals.fn);

  gtk_grab_add (file_sel);
  gtk_widget_show (file_sel);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (file_sel);
  gtk_widget_destroy (file_sel);

  if (filename != NULL)
  {
    strcpy (portals.fn, filename);
    gtk_entry_set_text (GTK_ENTRY (data), filename);
    g_free (filename);
  }
}

int DoLoadPortalFileDialog ()
{
  GtkWidget *dlg, *vbox, *hbox, *button, *entry, *check2d, *check3d;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), "Load .prt");
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (dlg), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_entry_set_editable (GTK_ENTRY (entry), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  check3d = gtk_check_button_new_with_label ("Show 3D");
  gtk_widget_show (check3d);
  gtk_box_pack_start (GTK_BOX (hbox), check3d, FALSE, FALSE, 0);

  check2d = gtk_check_button_new_with_label ("Show 2D");
  gtk_widget_show (check2d);
  gtk_box_pack_start (GTK_BOX (hbox), check2d, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Change");
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (change_clicked), entry);
  gtk_widget_set_usize (button, 60, -2);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Cancel");
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label ("OK");
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  strcpy (portals.fn, g_FuncTable.m_pfnGetMapName());
  char* fn = strrchr (portals.fn, '.');
  if (fn != NULL)
  {
    strcpy(fn, ".prt");
  }

  gtk_entry_set_text (GTK_ENTRY (entry), portals.fn);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check2d), portals.show_2d);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check3d), portals.show_3d);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    portals.Purge();

    portals.show_3d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check3d)) ? qtrue : qfalse;
    portals.show_2d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check2d)) ? qtrue : qfalse;
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return ret;
}

#else // GTK_PLUGIN

/////////////////////////////////////////////////////////////////////////////
// CLoadPortalFileDialog dialog

CLoadPortalFileDialog::CLoadPortalFileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadPortalFileDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadPortalFileDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoadPortalFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadPortalFileDialog)
	DDX_Control(pDX, IDC_LOAD_3D, m_3d_ctrl);
	DDX_Control(pDX, IDC_LOAD_2D, m_2d_ctrl);
	DDX_Control(pDX, IDC_LOAD_FILE_NAME, m_fn_ctrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadPortalFileDialog, CDialog)
	//{{AFX_MSG_MAP(CLoadPortalFileDialog)
	ON_BN_CLICKED(IDC_LOAD_OTHER, OnLoadOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLoadPortalFileDialog message handlers

bool CLoadPortalFileDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char fn_drive[_MAX_DRIVE];
	char fn_dir[_MAX_DIR];
	char fn_name[_MAX_FNAME];
	char fn_ext[_MAX_EXT];

	char *fn = g_IBSPTable.m_pfnGetMapName();

	_fullpath(portals.fn, fn, _MAX_PATH);
	_splitpath(fn, fn_drive, fn_dir, fn_name, fn_ext);

	strcpy(portals.fn, fn_drive);
	strcat(portals.fn, fn_dir);
	strcat(portals.fn, fn_name);
	strcat(portals.fn, ".prt");

	m_fn_ctrl.SetWindowText(portals.fn);

	m_2d_ctrl.SetCheck(portals.show_2d);
	m_3d_ctrl.SetCheck(portals.show_3d);

	return true;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoadPortalFileDialog::OnOK() 
{
	portals.Purge();

	portals.show_3d = m_3d_ctrl.GetCheck();
	portals.show_2d = m_2d_ctrl.GetCheck();
	
	CDialog::OnOK();
}

void CLoadPortalFileDialog::OnLoadOther() 
{
	CFileDialog dlg(TRUE, "prt", portals.fn, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_FILEMUSTEXIST,
		"Portal files (*.prt)|*.prt|All Files (*.*)|*.*||", NULL);

	dlg.m_ofn.lpstrTitle = "Locate portal file";

	if(IDOK == dlg.DoModal())
	{
		_fullpath(portals.fn, dlg.GetPathName().GetBuffer(1), _MAX_PATH);
		m_fn_ctrl.SetWindowText(portals.fn);
	}
}

#endif // GTK_PLUGIN
