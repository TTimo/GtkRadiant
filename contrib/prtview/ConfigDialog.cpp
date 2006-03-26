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

#include "ConfigDialog.h"
#include <stdio.h>
#include <gtk/gtk.h>

#include "iscenegraph.h"

#include "prtview.h"
#include "portals.h"

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

// =============================================================================
// Color selection dialog

static int DoColor (PackedColour *c)
{
  GtkWidget* dlg;
  double clr[4];
  int loop = 1, ret = IDCANCEL;

  clr[0] = ((double)GetRValue (*c)) / 255.0;
  clr[1] = ((double)GetGValue (*c)) / 255.0;
  clr[2] = ((double)GetBValue (*c)) / 255.0;

  dlg = gtk_color_selection_dialog_new ("Choose Color");
  gtk_color_selection_set_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), clr);
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG (dlg)->ok_button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG (dlg)->cancel_button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  gtk_widget_show(dlg);
  gtk_grab_add(dlg);

  while (loop)
    gtk_main_iteration ();

  gtk_color_selection_get_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dlg)->colorsel), clr);

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  if (ret == IDOK)
  {
    *c = RGB (clr[0]*255, clr[1]*255, clr[2]*255);
  }

  return ret;
}

static void Set2DText (GtkWidget* label)
{
  char s[40];

  sprintf(s, "Line Width = %6.3f", portals.width_2d * 0.5f);

  gtk_label_set_text (GTK_LABEL (label), s);
}

static void Set3DText (GtkWidget* label)
{
  char s[40];

  sprintf(s, "Line Width = %6.3f", portals.width_3d * 0.5f);

  gtk_label_set_text (GTK_LABEL (label), s);
}

static void Set3DTransText (GtkWidget* label)
{
  char s[40];

  sprintf(s, "Polygon transparency = %d%%", (int)portals.trans_3d);

  gtk_label_set_text (GTK_LABEL (label), s);
}

static void SetClipText (GtkWidget* label)
{
  char s[40];

  sprintf(s, "Cubic clip range = %d", (int)portals.clip_range * 64);

  gtk_label_set_text (GTK_LABEL (label), s);
}

static void OnScroll2d (GtkAdjustment *adj, gpointer data)
{
  portals.width_2d = static_cast<float>(adj->value);
  Set2DText (GTK_WIDGET (data));

  Portals_shadersChanged();
  SceneChangeNotify();
}

static void OnScroll3d (GtkAdjustment *adj, gpointer data)
{
  portals.width_3d = static_cast<float>(adj->value);
  Set3DText (GTK_WIDGET (data));

  SceneChangeNotify();
}

static void OnScrollTrans (GtkAdjustment *adj, gpointer data)
{
  portals.trans_3d = static_cast<float>(adj->value);
  Set3DTransText (GTK_WIDGET (data));

  SceneChangeNotify();
}

static void OnScrollClip (GtkAdjustment *adj, gpointer data)
{
  portals.clip_range = static_cast<float>(adj->value);
  SetClipText (GTK_WIDGET (data));

  SceneChangeNotify();
}

static void OnAntiAlias2d (GtkWidget *widget, gpointer data)
{
  portals.aa_2d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  Portals_shadersChanged();

  SceneChangeNotify();
}

static void OnConfig2d (GtkWidget *widget, gpointer data)
{
  portals.show_2d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  SceneChangeNotify();
}

static void OnColor2d (GtkWidget *widget, gpointer data)
{
  if (DoColor (&portals.color_2d) == IDOK)
  {
    Portals_shadersChanged();

    SceneChangeNotify();
  }
}

static void OnConfig3d (GtkWidget *widget, gpointer data)
{
  portals.show_3d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  SceneChangeNotify();
}


static void OnAntiAlias3d (GtkWidget *widget, gpointer data)
{
  portals.aa_3d = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  Portals_shadersChanged();
  SceneChangeNotify();
}

static void OnColor3d (GtkWidget *widget, gpointer data)
{
  if (DoColor (&portals.color_3d) == IDOK)
  {
    Portals_shadersChanged();

    SceneChangeNotify();
  }
}

static void OnColorFog (GtkWidget *widget, gpointer data)
{
  if (DoColor (&portals.color_fog) == IDOK)
  {
    Portals_shadersChanged();

    SceneChangeNotify();
  }
}

static void OnFog (GtkWidget *widget, gpointer data)
{
  portals.fog = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  Portals_shadersChanged();
  SceneChangeNotify();
}

static void OnSelchangeZbuffer (GtkWidget *widget, gpointer data)
{
  portals.zbuffer = GPOINTER_TO_INT (data);

  Portals_shadersChanged();
  SceneChangeNotify();
}

static void OnPoly (GtkWidget *widget, gpointer data)
{
  portals.polygons = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  SceneChangeNotify();
}

static void OnLines (GtkWidget *widget, gpointer data)
{
  portals.lines = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  SceneChangeNotify();
}

static void OnClip (GtkWidget *widget, gpointer data)
{
  portals.clip = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? true : false;

  SceneChangeNotify();
}

void DoConfigDialog ()
{
  GtkWidget *dlg, *hbox, *vbox, *vbox2, *button, *table, *frame;
  GtkWidget *lw3slider, *lw3label, *lw2slider, *lw2label, *zlist, *menu, *item;
  GtkWidget *aa2check, *aa3check, *depthcheck, *linescheck, *polyscheck;
  GtkWidget *transslider, *translabel, *clipslider, *cliplabel;
  GtkWidget *show2check, *show3check, *portalcheck;
  int loop = 1, ret = IDCANCEL;
  GtkObject *adj;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), "Portal Viewer Configuration");
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

  frame = gtk_frame_new ("3D View");
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, TRUE, 0);

  adj = gtk_adjustment_new (portals.width_3d, 2, 40, 1, 1, 1);
  lw3slider = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_show (lw3slider);
  gtk_box_pack_start (GTK_BOX (hbox), lw3slider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (lw3slider), FALSE);

  lw3label = gtk_label_new ("");
  gtk_widget_show (lw3label);
  gtk_box_pack_start (GTK_BOX (hbox), lw3label, FALSE, TRUE, 0);
  gtk_signal_connect (adj, "value_changed", GTK_SIGNAL_FUNC (OnScroll3d), lw3label);

  table = gtk_table_new (2, 4, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox2), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  button = gtk_button_new_with_label ("Color");
  gtk_widget_show (button);
  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (OnColor3d), NULL);

  button = gtk_button_new_with_label ("Depth Color");
  gtk_widget_show (button);
  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (OnColorFog), NULL);

  aa3check = gtk_check_button_new_with_label ("Anti-Alias (May not work on some video cards)");
  gtk_widget_show (aa3check);
  gtk_table_attach (GTK_TABLE (table), aa3check, 1, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (aa3check), "toggled", GTK_SIGNAL_FUNC (OnAntiAlias3d), NULL);

  depthcheck = gtk_check_button_new_with_label ("Depth Cue");
  gtk_widget_show (depthcheck);
  gtk_table_attach (GTK_TABLE (table), depthcheck, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (depthcheck), "toggled", GTK_SIGNAL_FUNC (OnFog), NULL);

  linescheck = gtk_check_button_new_with_label ("Lines");
  gtk_widget_show (linescheck);
  gtk_table_attach (GTK_TABLE (table), linescheck, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (linescheck), "toggled", GTK_SIGNAL_FUNC (OnLines), NULL);

  polyscheck = gtk_check_button_new_with_label ("Polygons");
  gtk_widget_show (polyscheck);
  gtk_table_attach (GTK_TABLE (table), polyscheck, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_signal_connect (GTK_OBJECT (polyscheck), "toggled", GTK_SIGNAL_FUNC (OnPoly), NULL);

  zlist = gtk_option_menu_new ();
  gtk_widget_show (zlist);
  gtk_box_pack_start (GTK_BOX (vbox2), zlist, TRUE, FALSE, 0);

  menu = gtk_menu_new ();
  gtk_widget_show (menu);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (zlist), menu);

  item = gtk_menu_item_new_with_label ("Z-Buffer Test and Write (recommended for solid or no polygons)");
  gtk_widget_show (item);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
		      GTK_SIGNAL_FUNC (OnSelchangeZbuffer), GINT_TO_POINTER (0));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Z-Buffer Test Only (recommended for transparent polygons)");
  gtk_widget_show (item);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
		      GTK_SIGNAL_FUNC (OnSelchangeZbuffer), GINT_TO_POINTER (1));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Z-Buffer Off");
  gtk_widget_show (item);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
		      GTK_SIGNAL_FUNC (OnSelchangeZbuffer), GINT_TO_POINTER (2));
  gtk_menu_append (GTK_MENU (menu), item);

  table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox2), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  adj = gtk_adjustment_new (portals.trans_3d, 0, 100, 1, 1, 1);
  transslider = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_show (transslider);
  gtk_table_attach (GTK_TABLE (table), transslider, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_scale_set_draw_value (GTK_SCALE (transslider), FALSE);

  translabel = gtk_label_new ("");
  gtk_widget_show (translabel);
  gtk_table_attach (GTK_TABLE (table), translabel, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (translabel), 0.0, 0.0);
  gtk_signal_connect (adj, "value_changed", GTK_SIGNAL_FUNC (OnScrollTrans), translabel);

  adj = gtk_adjustment_new (portals.clip_range, 1, 128, 1, 1, 1);
  clipslider = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_show (clipslider);
  gtk_table_attach (GTK_TABLE (table), clipslider, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_scale_set_draw_value (GTK_SCALE (clipslider), FALSE);

  cliplabel = gtk_label_new ("");
  gtk_widget_show (cliplabel);
  gtk_table_attach (GTK_TABLE (table), cliplabel, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cliplabel), 0.0, 0.0);
  gtk_signal_connect (adj, "value_changed", GTK_SIGNAL_FUNC (OnScrollClip), cliplabel);

  hbox = gtk_hbox_new (TRUE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, FALSE, 0);

  show3check = gtk_check_button_new_with_label ("Show");
  gtk_widget_show (show3check);
  gtk_box_pack_start (GTK_BOX (hbox), show3check, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (show3check), "toggled", GTK_SIGNAL_FUNC (OnConfig3d), NULL);

  portalcheck = gtk_check_button_new_with_label ("Portal cubic clipper");
  gtk_widget_show (portalcheck);
  gtk_box_pack_start (GTK_BOX (hbox), portalcheck, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (portalcheck), "toggled", GTK_SIGNAL_FUNC (OnClip), NULL);

  frame = gtk_frame_new ("2D View");
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, FALSE, 0);

  adj = gtk_adjustment_new (portals.width_2d, 2, 40, 1, 1, 1);
  lw2slider = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_show (lw2slider);
  gtk_box_pack_start (GTK_BOX (hbox), lw2slider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (lw2slider), FALSE);

  lw2label = gtk_label_new ("");
  gtk_widget_show (lw2label);
  gtk_box_pack_start (GTK_BOX (hbox), lw2label, FALSE, TRUE, 0);
  gtk_signal_connect (adj, "value_changed", GTK_SIGNAL_FUNC (OnScroll2d), lw2label);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, FALSE, 0);

  button = gtk_button_new_with_label ("Color");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (OnColor2d), NULL);
  gtk_widget_set_usize (button, 60, -2);

  aa2check = gtk_check_button_new_with_label ("Anti-Alias (May not work on some video cards)");
  gtk_widget_show (aa2check);
  gtk_box_pack_start (GTK_BOX (hbox), aa2check, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (aa2check), "toggled", GTK_SIGNAL_FUNC (OnAntiAlias2d), NULL);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, FALSE, 0);

  show2check = gtk_check_button_new_with_label ("Show");
  gtk_widget_show (show2check);
  gtk_box_pack_start (GTK_BOX (hbox), show2check, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (show2check), "toggled", GTK_SIGNAL_FUNC (OnConfig2d), NULL);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("OK");
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  // initialize dialog
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show2check), portals.show_2d);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (aa2check), portals.aa_2d);
  Set2DText (lw2label);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show3check), portals.show_3d);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (depthcheck), portals.fog);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (polyscheck), portals.polygons);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linescheck), portals.lines);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (aa3check), portals.aa_3d);
  gtk_option_menu_set_history (GTK_OPTION_MENU (zlist), portals.zbuffer);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (portalcheck), portals.clip);

  Set3DText (lw3label);
  Set3DTransText (translabel);
  SetClipText (cliplabel);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

