/*
Copyright (c) 2001, Loki software, inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Loki software nor the names of its contributors may be used
to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Some small dialogs that don't need much
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif

#ifdef _WIN32
#include <shellapi.h>
#endif

// =============================================================================
// Color selection dialog

qboolean DoColor (int iIndex)
{
  static bool bColorOpen = false;

  if(bColorOpen)
  {
    Sys_FPrintf(SYS_WRN, "DoColor dialog is already open\n");
    return false;
  }

  bColorOpen = true;

  if (color_dialog (g_pParentWnd->m_pWidget, g_qeglobals.d_savedinfo.colors[iIndex]))
  {
    /*
    ** scale colors so that at least one component is at 1.0F
    ** if this is meant to select an entity color
    */
    if (iIndex == COLOR_ENTITY)
    {
      float largest = 0.0F;

      if ( g_qeglobals.d_savedinfo.colors[iIndex][0] > largest )
	largest = g_qeglobals.d_savedinfo.colors[iIndex][0];
      if ( g_qeglobals.d_savedinfo.colors[iIndex][1] > largest )
	largest = g_qeglobals.d_savedinfo.colors[iIndex][1];
      if ( g_qeglobals.d_savedinfo.colors[iIndex][2] > largest )
	largest = g_qeglobals.d_savedinfo.colors[iIndex][2];

      if ( largest == 0.0F )
      {
	g_qeglobals.d_savedinfo.colors[iIndex][0] = 1.0F;
	g_qeglobals.d_savedinfo.colors[iIndex][1] = 1.0F;
	g_qeglobals.d_savedinfo.colors[iIndex][2] = 1.0F;
      }
      else
      {
	float scaler = 1.0F / largest;

	g_qeglobals.d_savedinfo.colors[iIndex][0] *= scaler;
	g_qeglobals.d_savedinfo.colors[iIndex][1] *= scaler;
	g_qeglobals.d_savedinfo.colors[iIndex][2] *= scaler;
      }
    }

    Sys_UpdateWindows (W_ALL);
    bColorOpen = false;
    return true;
  }
  else {
    bColorOpen = false;
    return false;
  }
}

// =============================================================================
// Project settings dialog

static void UpdateBSPCommandList (GtkWidget *dialog);

static void DoProjectAddEdit (bool edit, GtkWidget *parent)
{
  GtkWidget *dlg, *vbox, *hbox, *label, *table, *button;
  GtkWidget *cmd, *text;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (edit)
    gtk_window_set_title (GTK_WINDOW (dlg), _("Edit Command"));
  else
    gtk_window_set_title (GTK_WINDOW (dlg), _("Add Command"));
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

  table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("Menu text"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  label = gtk_label_new (_("Command"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  text = gtk_entry_new ();
  g_object_set_data (G_OBJECT (dlg), "text", text);
  gtk_widget_show (text);
  gtk_table_attach (GTK_TABLE (table), text, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_usize (text, 300, -2);

  cmd = gtk_entry_new ();
  g_object_set_data (G_OBJECT (dlg), "cmd", cmd);
  gtk_widget_show (cmd);
  gtk_table_attach (GTK_TABLE (table), cmd, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_usize (cmd, 300, -2);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  if (edit)
  {
    GtkTreeView* view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(parent), "view"));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(view);
    GtkTreeIter iter;
    GtkTreeModel* model;
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
      char* key;
      gtk_tree_model_get(model, &iter, 0, &key, -1);
      const char* value = ValueForKey (g_qeglobals.d_project_entity, key);
      gtk_entry_set_text (GTK_ENTRY (text), key);
      gtk_entry_set_text (GTK_ENTRY (cmd), value);
      g_free(key);
    }
  }

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    const char* key = gtk_entry_get_text (GTK_ENTRY (text));
    const char* value = gtk_entry_get_text (GTK_ENTRY (cmd));

    if (strlen (key) <= 0 || strlen (value) <= 0)
    {
      Sys_Printf ("Command not added\n");
    }
    else
    {
      if (edit)
      {
        SetKeyValue (g_qeglobals.d_project_entity, key, value);
        FillBSPMenu ();
      }
      else
      {
	      if (key[0] == 'b' && key[1] == 's' && key[2] == 'p')
	      {
	        SetKeyValue (g_qeglobals.d_project_entity, key, value);
	        FillBSPMenu ();
	      }
	      else
	        Sys_Printf ("BSP commands must be preceded by \"bsp\"");
      }

      UpdateBSPCommandList (parent);
    }
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

static void UpdateBSPCommandList (GtkWidget *dialog)
{
  GtkListStore* store = GTK_LIST_STORE (g_object_get_data (G_OBJECT(dialog), "bsp_commands"));

  gtk_list_store_clear(store);

  for(epair_t* ep = g_qeglobals.d_project_entity->epairs; ep != NULL; ep = ep->next)
  {
    if(ep->key[0] == 'b' && ep->key[1] == 's' && ep->key[2] == 'p')
    {
      GtkTreeIter iter;
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, ep->key, -1);
    }
  }
}

static void project_add (GtkWidget *widget, gpointer data)
{
  GtkWidget *dlg = GTK_WIDGET (data);
  DoProjectAddEdit (false, dlg);
  UpdateBSPCommandList (dlg);
}

static void project_change (GtkWidget *widget, gpointer data)
{
  GtkWidget *dlg = GTK_WIDGET (data);
  DoProjectAddEdit (true, dlg);
  UpdateBSPCommandList (dlg);
}

static void project_remove (GtkWidget *widget, gpointer data)
{
  GtkWidget* project = GTK_WIDGET(data);

  GtkTreeView* view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(project), "view"));
  GtkTreeSelection* selection = gtk_tree_view_get_selection(view);
  GtkTreeIter iter;
  GtkTreeModel* model;
  if(gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    char* key;
    gtk_tree_model_get(model, &iter, 0, &key, -1);
    DeleteKey (g_qeglobals.d_project_entity, key);
    g_free(key);

    char* index = gtk_tree_model_get_string_from_iter(model, &iter);
    Sys_Printf ("Selected %s\n", index);
    g_free(index);

    UpdateBSPCommandList(project);
    FillBSPMenu();
  }
}

static const char* sQ3ComboItem = "Quake III Arena";
static const char* sTAComboItem = "Quake III: Team Arena";
static const char* sModComboItem = "Custom Quake III modification";
static const char* sWolfComboItem = "Return To Castle Wolfenstein";
static const char* sWolfModComboItem = "Custom RTCW modification";
static const char* sHLComboItem = "Half-life";
static const char* sHLModComboItem = "Custom Half-life modification";

static const char* sWolfSPCombo = "Single Player mapping mode";
static const char* sWolfMPCombo = "Multiplayer mapping mode";

// Arnout
// HARD-CODED ET HACK
static const char* sETComboItem = "Wolfenstein: Enemy Territory";
static const char* sETModComboItem = "Custom ET modification";

// RIANT
// HARD-CODED JK2 HACK
static const char* sJK2ComboItem = "Jedi Knight II Outcast";
static const char* sJK2ModComboItem = "Custom JK2 modification";
static const char* sJK2SPCombo = "Single Player mapping mode";
static const char* sJK2MPCombo = "Multiplayer mapping mode";

// TTimo
// HARD-CODED JA HACK
static const char* sJAComboItem = "Jedi Knight Jedi Academy";
static const char* sJAModComboItem = "Custom JA modification";
static const char* sJASPCombo = "Single Player mapping mode";
static const char* sJAMPCombo = "Multiplayer mapping mode";

// RIANT
// HARD-CODED STVEF2 HACK
static const char* sSTVEFComboItem = "Star Trek Voyager : Elite Force";
static const char* sSTVEFModComboItem = "Custom Elite Force modification";
static const char* sSTVEFSPCombo = "Single Player mapping mode";
static const char* sSTVEFMPCombo = "Holo Match mapping mode";

// RIANT
// HARD-CODED SOF2 HACK
static const char* sSOF2ComboItem = "Soldier of Fortune II - Double Helix";
static const char* sSOF2ModComboItem = "Custom Sof2 modification";
static const char* sSOF2SPCombo = "Single Player mapping mode";
static const char* sSOF2MPCombo = "Multiplayer mapping mode";

static GtkWidget* game_select; // GTK_COMBO
static GtkEntry* fsgame_entry;

gint OnSelchangeComboWhatgame (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  const char *dir = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(game_select)->entry));
  // HACK: Wolf
  if (g_pGameDescription->mGameFile == "wolf.game")
  {
    if (!strcmp(dir,sWolfComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }

  }
  // HACK: ET
  else if (g_pGameDescription->mGameFile == "et.game")
  {
    if (!strcmp(dir,sETComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }

  }
  else if (g_pGameDescription->mGameFile == "hl.game")
  {
    if (!strcmp(dir,sHLComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }

  }
  // RIANT
  // HACK: JK2
  else if (g_pGameDescription->mGameFile == "jk2.game")
  {
    if (!strcmp(dir,sJK2ComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }
  }
  // TTimo
  // HACK: JA
  else if (g_pGameDescription->mGameFile == "ja.game")
  {
    if (!strcmp(dir,sJAComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }
  }
  // RIANT
  // HACK: STVEF
  else if (g_pGameDescription->mGameFile == "stvef.game")
  {
    if (!strcmp(dir,sSTVEFComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }
  }
  // RIANT
  // HACK: SOF2
  else if (g_pGameDescription->mGameFile == "sof2.game")
  {
    if (!strcmp(dir,sSOF2ComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }
  }
  // QUAKE 3
  else
  {
    if (!strcmp(dir,sQ3ComboItem))
    {
      // disable the fs_game text entry
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else if (!strcmp(dir,sTAComboItem))
    {
      gtk_entry_set_text (fsgame_entry, "missionpack");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), false);
    }
    else
    {
      gtk_entry_set_text (fsgame_entry, "");
      gtk_widget_set_sensitive(GTK_WIDGET(fsgame_entry), true);
    }
  }

  return TRUE;
}

void DoProjectSettings ()
{
  GtkWidget *project;
  GtkWidget *frame, *label, *vbox, *table1, *table2, *button;
  GtkWidget *brush;
  GtkWidget *scr;
  GtkWidget *base, *game;
  GtkWidget *gamemode_combo;
  GList *combo_list = (GList*)NULL;

  int loop = 1, ret = IDCANCEL;

  project = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (project), _("Project Settings"));
  gtk_signal_connect (GTK_OBJECT (project), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (project), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (project), "loop", &loop);
  g_object_set_data (G_OBJECT (project), "ret", &ret);
  gtk_window_set_default_size (GTK_WINDOW (project), 550, 400);

  table1 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (project), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_table_attach (GTK_TABLE (table1), vbox, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_table_attach (GTK_TABLE (table1), vbox, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  button = gtk_button_new_with_label (_("Add..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (project_add), project);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Change..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (project_change), project);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Remove"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (project_remove), project);
  gtk_widget_set_usize (button, 60, -2);

  frame = gtk_frame_new (_("Misc settings"));
  gtk_widget_show (frame);
  gtk_table_attach (GTK_TABLE (table1), frame, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  brush = gtk_check_button_new_with_label (_("Use brush primitives in MAP files (NOTE: experimental feature,\n"
					   "required by the texture tools plugin)"));
  gtk_widget_show (brush);
  gtk_container_add (GTK_CONTAINER (frame), brush);
  gtk_container_set_border_width (GTK_CONTAINER (brush), 5);

  frame = gtk_frame_new (_("Menu commands"));
  gtk_widget_show (frame);
  gtk_table_attach (GTK_TABLE (table1), frame, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);


  {
    GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    GtkTreeSelection* selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

    gtk_widget_show(view);

    g_object_set_data(G_OBJECT (project), "view", view);
    g_object_set_data(G_OBJECT (project), "bsp_commands", store);
    gtk_container_add(GTK_CONTAINER (scr), view);

    g_object_unref(G_OBJECT(store));
  }

  frame = gtk_frame_new (_("Project settings"));
  gtk_widget_show (frame);
  gtk_table_attach (GTK_TABLE (table1), frame, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  // HACK: hardcoded game stuff
  if (g_pGameDescription->mGameFile == "wolf.game" ||
    g_pGameDescription->mGameFile == "et.game" ||
    g_pGameDescription->mGameFile == "jk2.game" ||
    g_pGameDescription->mGameFile == "stvef.game" ||
    g_pGameDescription->mGameFile == "sof2.game" ||
    g_pGameDescription->mGameFile == "ja.game" )
  {
    table2 = gtk_table_new (9, 2, FALSE);
  }
  else
  {
    table2 = gtk_table_new (8, 2, FALSE);
  }
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (frame), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);

  /*
  fill in the game selection combo
  HACK: hardcoded Q3/Wolf/HL switch
  \todo that stuff would be faster to write with implementation of property bags and associated code to edit
  */
  if (g_pGameDescription->mGameFile == "wolf.game")
  {
    combo_list = g_list_append (combo_list, (void *)sWolfComboItem);
    combo_list = g_list_append (combo_list, (void *)sWolfModComboItem);
  }
  else if (g_pGameDescription->mGameFile == "et.game")
  {
    combo_list = g_list_append (combo_list, (void *)sETComboItem);
    combo_list = g_list_append (combo_list, (void *)sETModComboItem);
  }
  else if (g_pGameDescription->mGameFile == "hl.game")
  {
    combo_list = g_list_append (combo_list, (void *)sHLComboItem);
    combo_list = g_list_append (combo_list, (void *)sHLModComboItem);
  }
  // RIANT
  // JK2 HACK
  else if (g_pGameDescription->mGameFile == "jk2.game")
  {
    combo_list = g_list_append (combo_list, (void *)sJK2ComboItem);
    combo_list = g_list_append (combo_list, (void *)sJK2ModComboItem);
  }
  // TTimo
  // JA HACK
  else if (g_pGameDescription->mGameFile == "ja.game")
  {
    combo_list = g_list_append (combo_list, (void *)sJAComboItem);
    combo_list = g_list_append (combo_list, (void *)sJAModComboItem);
  }
  // RIANT
  // STVEF HACK
  else if (g_pGameDescription->mGameFile == "stvef.game")
  {
    combo_list = g_list_append (combo_list, (void *)sSTVEFComboItem);
    combo_list = g_list_append (combo_list, (void *)sSTVEFModComboItem);
  }
  // RIANT
  // SOF2 HACK A LA JK2 A LA WOLF
  else if (g_pGameDescription->mGameFile == "sof2.game")
  {
    combo_list = g_list_append (combo_list, (void *)sSOF2ComboItem);
    combo_list = g_list_append (combo_list, (void *)sSOF2ModComboItem);
  }
  else
  {
    // Q3 or default
    combo_list = g_list_append (combo_list, (void *)sQ3ComboItem);
    combo_list = g_list_append (combo_list, (void *)sTAComboItem);
    combo_list = g_list_append (combo_list, (void *)sModComboItem);
  }

  game_select = gtk_combo_new ();
  gtk_combo_set_popdown_strings (GTK_COMBO (game_select), combo_list);
  gtk_widget_show (game_select);
  gtk_table_attach (GTK_TABLE (table2), game_select, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_signal_connect (GTK_OBJECT(GTK_COMBO (game_select)->entry), "changed",
    GTK_SIGNAL_FUNC (OnSelchangeComboWhatgame), NULL);

  g_list_free (combo_list);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (game_select)->entry), FALSE);

  game = gtk_entry_new();
  fsgame_entry = GTK_ENTRY(game);
  gtk_widget_show(game);
  gtk_table_attach(GTK_TABLE(table2), game, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  /*
  wolf specific: select MP or SP mode
  */
  if (g_pGameDescription->mGameFile == "wolf.game")
  {
    combo_list = NULL;
    combo_list = g_list_append (combo_list, (void *)sWolfSPCombo);
    combo_list = g_list_append (combo_list, (void *)sWolfMPCombo);

    gamemode_combo = gtk_combo_new ();
    gtk_combo_set_popdown_strings (GTK_COMBO (gamemode_combo), combo_list);
    gtk_widget_show(gamemode_combo);
    gtk_table_attach (GTK_TABLE (table2), gamemode_combo, 1, 2, 8, 9,
      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

    g_list_free (combo_list);
    combo_list = NULL;

    label = gtk_label_new (_("Mapping mode"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 8, 9,
      (GtkAttachOptions) (GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  }

  // RIANT
  // JK2 HACK
  if (g_pGameDescription->mGameFile == "jk2.game")
  {
    combo_list = NULL;
    combo_list = g_list_append (combo_list, (void *)sJK2SPCombo);
    combo_list = g_list_append (combo_list, (void *)sJK2MPCombo);

    gamemode_combo = gtk_combo_new ();
    gtk_combo_set_popdown_strings (GTK_COMBO (gamemode_combo), combo_list);
    gtk_widget_show(gamemode_combo);
    gtk_table_attach (GTK_TABLE (table2), gamemode_combo, 1, 2, 8, 9,
      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

    g_list_free (combo_list);
    combo_list = NULL;

    label = gtk_label_new (_("Mapping mode"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 8, 9,
      (GtkAttachOptions) (GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  }
  // TTimo
  // JA HACK
  if (g_pGameDescription->mGameFile == "ja.game")
  {
    combo_list = NULL;
    combo_list = g_list_append (combo_list, (void *)sJASPCombo);
    combo_list = g_list_append (combo_list, (void *)sJAMPCombo);

    gamemode_combo = gtk_combo_new ();
    gtk_combo_set_popdown_strings (GTK_COMBO (gamemode_combo), combo_list);
    gtk_widget_show(gamemode_combo);
    gtk_table_attach (GTK_TABLE (table2), gamemode_combo, 1, 2, 8, 9,
      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

    g_list_free (combo_list);
    combo_list = NULL;

    label = gtk_label_new (_("Mapping mode"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 8, 9,
      (GtkAttachOptions) (GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  }
  // RIANT
  // STVEF HACK
  if (g_pGameDescription->mGameFile == "stvef.game")
  {
    combo_list = NULL;
    combo_list = g_list_append (combo_list, (void *)sSTVEFSPCombo);
    combo_list = g_list_append (combo_list, (void *)sSTVEFMPCombo);

    gamemode_combo = gtk_combo_new ();
    gtk_combo_set_popdown_strings (GTK_COMBO (gamemode_combo), combo_list);
    gtk_widget_show(gamemode_combo);
    gtk_table_attach (GTK_TABLE (table2), gamemode_combo, 1, 2, 8, 9,
      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

    g_list_free (combo_list);
    combo_list = NULL;

    label = gtk_label_new (_("Mapping mode"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 8, 9,
      (GtkAttachOptions) (GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  }
  // RIANT
  // SOF2 HACK
  if (g_pGameDescription->mGameFile == "sof2.game")
  {
    combo_list = NULL;
    combo_list = g_list_append (combo_list, (void *)sSOF2SPCombo);
    combo_list = g_list_append (combo_list, (void *)sSOF2MPCombo);

    gamemode_combo = gtk_combo_new ();
    gtk_combo_set_popdown_strings (GTK_COMBO (gamemode_combo), combo_list);
    gtk_widget_show(gamemode_combo);
    gtk_table_attach (GTK_TABLE (table2), gamemode_combo, 1, 2, 8, 9,
      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);

    g_list_free (combo_list);
    combo_list = NULL;

    label = gtk_label_new (_("Mapping mode"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 8, 9,
      (GtkAttachOptions) (GTK_FILL),
      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  }

  /*
  the usual stuff
  */

  base = gtk_entry_new ();
  g_object_set_data (G_OBJECT (project), "base", base);
  gtk_widget_show (base);
  gtk_table_attach (GTK_TABLE (table2), base, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);


  label = gtk_label_new (_("basepath"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);


  label = gtk_label_new (_("Select mod"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  label = gtk_label_new (_("fs_game"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table2), label, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  // Initialize fields
  gtk_entry_set_text (GTK_ENTRY (base), ValueForKey (g_qeglobals.d_project_entity, "basepath"));
  UpdateBSPCommandList (project);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (brush), (g_qeglobals.m_bBrushPrimitMode) ? TRUE : FALSE);

  // initialise the fs_game selection from the project settings into the dialog
  const char *dir = ValueForKey (g_qeglobals.d_project_entity, "gamename");
  // HACK: hardcoded wolf stuff
  if (g_pGameDescription->mGameFile == "wolf.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"main"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sWolfComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sWolfModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // HACK: hardcoded et stuff
  if (g_pGameDescription->mGameFile == "et.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"etmain"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sETComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sETModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // HACK: hardcoded half-life stuff
  else if (g_pGameDescription->mGameFile == "hl.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"valve"))
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sHLComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sHLModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // RIANT
  // JK2 HACK
  else if (g_pGameDescription->mGameFile == "jk2.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"base"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sJK2ComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sJK2ModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // TTimo
  // JA HACK
  else if (g_pGameDescription->mGameFile == "ja.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"base"))
    {
      // no fs_game set, we are running stock editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sJAComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sJAModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // RIANT
  // STVEF2 HACK
  else if (g_pGameDescription->mGameFile == "stvef.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"baseEf"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sSTVEFComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sSTVEFModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  // RIANT
  // SOF2 HACK
  else if (g_pGameDescription->mGameFile == "sof2.game")
  {
    if ((strlen(dir) == 0) || !stricmp(dir,"base"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sSOF2ComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sSOF2ModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }
  else
  {
    if ((strlen(dir) == 0) || !strcmp(dir,"baseq3"))
    {
      // no fs_game set, we are running stock Quake III Arena editing
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sQ3ComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "");
      gtk_widget_set_sensitive(game, false);
    }
    else if (!strcmp(dir,"missionpack"))
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sTAComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), "missionpack");
      gtk_widget_set_sensitive(game, false);
    }
    else
    {
      // this is a custom mod
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (game_select)->entry), sModComboItem);
      gtk_entry_set_text (GTK_ENTRY (game), dir);
      gtk_widget_set_sensitive(game, true);
    }
  }

  // HACK: hardcoded wolf stuff
  if (g_pGameDescription->mGameFile == "wolf.game")
  {
    const char *gamemode = ValueForKey (g_qeglobals.d_project_entity, "gamemode");
    if ((strlen(gamemode) == 0) || !strcmp(gamemode,"sp"))
    {
      // nothing set yet, or single player
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sWolfSPCombo);
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sWolfMPCombo);
    }
  }

  // JK2 HACK
  else if (g_pGameDescription->mGameFile == "jk2.game")
  {
    const char *gamemode = ValueForKey (g_qeglobals.d_project_entity, "gamemode");
    if ((strlen(gamemode) == 0) || !strcmp(gamemode,"sp"))
    {
      // nothing set yet, or single player
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sJK2SPCombo);
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sJK2MPCombo);
    }
  }
  // JA HACK
  else if (g_pGameDescription->mGameFile == "ja.game")
  {
    const char *gamemode = ValueForKey (g_qeglobals.d_project_entity, "gamemode");
    if ((strlen(gamemode) == 0) || !strcmp(gamemode,"sp"))
    {
      // nothing set yet, or single player
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sJASPCombo);
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sJAMPCombo);
    }
  }
  // STVEF HACK
  else if (g_pGameDescription->mGameFile == "stvef.game")
  {
    const char *gamemode = ValueForKey (g_qeglobals.d_project_entity, "gamemode");
    if ((strlen(gamemode) == 0) || !strcmp(gamemode,"sp"))
    {
      // nothing set yet, or single player
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sSTVEFSPCombo);
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sSTVEFMPCombo);
    }
  }
  // SOF2 HACK
  else if (g_pGameDescription->mGameFile == "sof2.game")
  {
    const char *gamemode = ValueForKey (g_qeglobals.d_project_entity, "gamemode");
    if ((strlen(gamemode) == 0) || !strcmp(gamemode,"sp"))
    {
      // nothing set yet, or single player
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sSOF2SPCombo);
    }
    else
    {
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gamemode_combo)->entry), sSOF2MPCombo);
    }
  }

  gtk_grab_add (project);
  gtk_widget_show (project);

  g_pGameDescription->Dump();

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    char buf[1024];
    const char *r;
    char *w;

    // convert path to unix format
    for(r = gtk_entry_get_text (GTK_ENTRY (base)), w=buf; *r != '\0'; r++, w++)
      *w = (*r == '\\') ? '/' : *r;
    // add last slash
    if(w != buf && *(w-1) != '/') *(w++) = '/';
    // terminate string
    *w = '\0';
    SetKeyValue (g_qeglobals.d_project_entity, "basepath", buf);

    dir = gtk_entry_get_text (GTK_ENTRY (game));
    // Hack: hard coded wolf stuff
    if (g_pGameDescription->mGameFile == "wolf.game")
    {
      if (!strlen(dir) || !stricmp(dir,"main"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }
    // Hack: hard coded ET stuff
    else if (g_pGameDescription->mGameFile == "et.game")
    {
      if (!strlen(dir) || !stricmp(dir,"etmain"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }
    // Hack: hard coded Half-life stuff
    else if (g_pGameDescription->mGameFile == "hl.game")
    {
      if (!strlen(dir) || !stricmp(dir,"valve"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }
    else if (g_pGameDescription->mGameFile == "jk2.game" || g_pGameDescription->mGameFile == "ja.game")
    {
      if (!strlen(dir) || !stricmp(dir,"base"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }
    // RIANT
    // STVEF HACK
    else if (g_pGameDescription->mGameFile == "stvef.game")
    {
      if (!strlen(dir) || !stricmp(dir,"baseEf"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }
    else
    {
      if (!strlen(dir) || !strcmp(dir,"baseq3"))
      {
        DeleteKey (g_qeglobals.d_project_entity, "gamename");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamename", dir);
      }
    }

    // HACK: hardcoded wolf stuff
    if (g_pGameDescription->mGameFile == "wolf.game")
    {
      // read from gamemode_combo
      const char *gamemode = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(gamemode_combo)->entry));
      if (!strlen(gamemode) || !strcmp(gamemode, sWolfSPCombo))
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "sp");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "mp");
      }
    }

    // RIANT
    // JK2 HACK
    if (g_pGameDescription->mGameFile == "jk2.game")
    {
      // read from gamemode_combo
      const char *gamemode = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(gamemode_combo)->entry));
      if (!strlen(gamemode) || !strcmp(gamemode, sJK2SPCombo))
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "sp");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "mp");
      }
    }
    // TTimo
    // JA HACK
    if (g_pGameDescription->mGameFile == "ja.game")
    {
      // read from gamemode_combo
      const char *gamemode = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(gamemode_combo)->entry));
      if (!strlen(gamemode) || !strcmp(gamemode, sJASPCombo))
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "sp");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "mp");
      }
    }

    // RIANT
    // STVEF HACK
    if (g_pGameDescription->mGameFile == "stvef.game")
    {
      // read from gamemode_combo
      const char *gamemode = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(gamemode_combo)->entry));
      if (!strlen(gamemode) || !strcmp(gamemode, sSTVEFSPCombo))
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "sp");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "mp");
      }
    }

    g_qeglobals.m_strHomeMaps = g_qeglobals.m_strHomeGame;
    const char* str = ValueForKey(g_qeglobals.d_project_entity, "gamename");
    if(str[0] == '\0') str = g_pGameDescription->mBaseGame.GetBuffer();
    g_qeglobals.m_strHomeMaps += str;
    g_qeglobals.m_strHomeMaps += '/';

    // RIANT
    // SOF2 HACK
    if (g_pGameDescription->mGameFile == "sof2.game")
    {
      // read from gamemode_combo
      const char *gamemode = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(gamemode_combo)->entry));
      if (!strlen(gamemode) || !strcmp(gamemode, sSOF2SPCombo))
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "sp");
      }
      else
      {
        SetKeyValue (g_qeglobals.d_project_entity, "gamemode", "mp");
      }
    }

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (brush)))
      g_qeglobals.m_bBrushPrimitMode = TRUE;
    else
      g_qeglobals.m_bBrushPrimitMode = FALSE;

    SetKeyValue(g_qeglobals.d_project_entity, "brush_primit", (g_qeglobals.m_bBrushPrimitMode ? "1" : "0" ));

    QE_SaveProject( g_PrefsDlg.m_strLastProject.GetBuffer() );
  }

  gtk_grab_remove (project);
  gtk_widget_destroy (project);
}

// =============================================================================
// MapInfo dialog

void DoMapInfo ()
{
  static GtkWidget *dlg;
  GtkWidget *vbox, *vbox2, *hbox, *table, *button, *label, *scr;
  GtkWidget *brushes_entry, *entities_entry, *net_entry;
  int loop = 1, ret = IDCANCEL;

  if (dlg != NULL)
    return;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  load_window_pos(dlg, g_PrefsDlg.mWindowInfo.posMapInfoWnd);

  gtk_window_set_title (GTK_WINDOW (dlg), _("Map Info"));
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

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  table = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  brushes_entry = gtk_entry_new ();
  gtk_widget_show (brushes_entry);
  gtk_table_attach (GTK_TABLE (table), brushes_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (brushes_entry), FALSE);

  entities_entry = gtk_entry_new ();
  gtk_widget_show (entities_entry);
  gtk_table_attach (GTK_TABLE (table), entities_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (entities_entry), FALSE);

  net_entry = gtk_entry_new ();
  gtk_widget_show (net_entry);
  gtk_table_attach (GTK_TABLE (table), net_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (net_entry), FALSE);

  label = gtk_label_new (_("Total Brushes"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Total Entities"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Net brush count\n(non entity)"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Close"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  label = gtk_label_new (_("Entity breakdown"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (vbox), scr, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scr), GTK_SHADOW_IN);

  GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

  {
    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(view), TRUE);

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Entity"), renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
      gtk_tree_view_column_set_sort_column_id(column, 0);
    }

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Count"), renderer, "text", 1, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
      gtk_tree_view_column_set_sort_column_id(column, 1);
    }

    gtk_widget_show(view);

    gtk_container_add(GTK_CONTAINER (scr), view);
  }

  // Initialize fields
  int TotalBrushes = 0, TotalEntities = 0, Net = 0;

  for (brush_t* pBrush = active_brushes.next; pBrush != &active_brushes; pBrush = pBrush->next)
  {
    TotalBrushes++;
    if (pBrush->owner == world_entity)
      Net++;
  }

  typedef struct
  {
    const char *name;
    int count;
  } map_t;

  GSList *l, *entitymap = NULL;
  map_t *entry;

  for (entity_t* pEntity = entities.next; pEntity != &entities; pEntity=pEntity->next)
  {
    TotalEntities++;
    bool add = true;

    for (l = entitymap; l; l = g_slist_next (l))
    {
      entry = (map_t*)l->data;

      if (strcmp (entry->name, pEntity->eclass->name) == 0)
      {
	      entry->count++;
	      add = false;
	      break;
      }
    }

    if (add)
    {
      entry = (map_t*)qmalloc (sizeof (map_t));
      entry->name = pEntity->eclass->name;
      entry->count = 1;
      entitymap = g_slist_append (entitymap, entry);
    }
  }

  while (entitymap)
  {
    entry = (map_t*)entitymap->data;
    char tmp[16];
    sprintf (tmp, "%d", entry->count);
    GtkTreeIter iter;
    gtk_list_store_append(GTK_LIST_STORE(store), &iter);
    gtk_list_store_set(GTK_LIST_STORE(store), &iter, 0, entry->name, 1, tmp, -1);
    free (entry);
    entitymap = g_slist_remove (entitymap, entry);
  }

  g_object_unref(G_OBJECT(store));

  char tmp[16];
  sprintf (tmp, "%d", TotalBrushes);
  gtk_entry_set_text (GTK_ENTRY (brushes_entry), tmp);
  sprintf (tmp, "%d", TotalEntities);
  gtk_entry_set_text (GTK_ENTRY (entities_entry), tmp);
  sprintf (tmp, "%d", Net);
  gtk_entry_set_text (GTK_ENTRY (net_entry), tmp);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  // save before exit
  save_window_pos(dlg, g_PrefsDlg.mWindowInfo.posMapInfoWnd);

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
  dlg = NULL;
}

// =============================================================================
// Entity List dialog

static void entitylist_select (GtkWidget *widget, gpointer data)
{
  GtkTreeView* view = GTK_TREE_VIEW(g_object_get_data (G_OBJECT (data), "entities"));

  GtkTreeSelection* selection = gtk_tree_view_get_selection(view);

  GtkTreeModel* model;
  GtkTreeIter selected;
  if(gtk_tree_selection_get_selected(selection, &model, &selected))
  {
    entity_t* pEntity;
    gtk_tree_model_get(model, &selected, 1, &pEntity, -1);

    if (pEntity)
    {
      for (epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next)
      {
	      Select_Deselect ();
	      Select_Brush (pEntity->brushes.onext);
	      Sys_UpdateWindows(W_ALL);
      }
    }
  }
}

static gint entitylist_click (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if (event->type == GDK_2BUTTON_PRESS)
  {
    entitylist_select (NULL, data);
    return TRUE;
  }
  return FALSE;
}

static void entitylist_selection_changed(GtkTreeSelection* selection, gpointer data)
{
  GtkListStore* store = GTK_LIST_STORE (g_object_get_data (G_OBJECT (data), "keyvalues"));

  gtk_list_store_clear(store);

  GtkTreeModel* model;
  GtkTreeIter selected;
  if(gtk_tree_selection_get_selected(selection, &model, &selected))
  {
    entity_t* pEntity;
    gtk_tree_model_get(model, &selected, 1, &pEntity, -1);

    if (pEntity)
    {
      for (epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next)
      {
        GtkTreeIter appended;
        gtk_list_store_append(store, &appended);
        gtk_list_store_set(store, &appended, 0, pEpair->key, 1, pEpair->value, -1);
      }
    }
  }
}

void DoEntityList ()
{
  static GtkWidget *dlg;
  GtkWidget *vbox, *hbox, *hbox2, *button, *scr;
  int loop = 1, ret = IDCANCEL;

  if (dlg != NULL)
    return;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  load_window_pos (dlg, g_PrefsDlg.mWindowInfo.posEntityInfoWnd);

  gtk_window_set_title (GTK_WINDOW (dlg), _("Entities"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (TRUE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (hbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

  {
    GtkTreeStore* store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_signal_connect(G_OBJECT(view), "button_press_event", G_CALLBACK(entitylist_click), dlg);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    {
      GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
      g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(entitylist_selection_changed), dlg);
    }

    gtk_widget_show(view);

    gtk_container_add(GTK_CONTAINER (scr), view);
    g_object_set_data (G_OBJECT (dlg), "entities", view);

    {
      {
        GtkTreeIter child;
        gtk_tree_store_append(store, &child, NULL);
        gtk_tree_store_set(store, &child, 0, world_entity->eclass->name, 1, world_entity, -1);
      }

      GSList *l, *entitymap = NULL;
      typedef struct
      {
        GtkTreeIter node;
        const char *name;
      } map_t;
      map_t *entry;

      for (entity_t* pEntity=entities.next; pEntity != &entities; pEntity=pEntity->next)
      {
        GtkTreeIter parent;
        bool found = false;

        for (l = entitymap; l; l = g_slist_next (l))
        {
          entry = (map_t*)l->data;

          if (strcmp (entry->name, pEntity->eclass->name) == 0)
          {
	          parent = entry->node;
	          found = true;
	          break;
          }
        }

        if (!found)
        {
          gtk_tree_store_append(store, &parent, NULL);
          gtk_tree_store_set(store, &parent, 0, pEntity->eclass->name, 1, NULL, -1);

          entry = (map_t*)malloc (sizeof(map_t));
          entitymap = g_slist_append (entitymap, entry);
          entry->name = pEntity->eclass->name;
          entry->node = parent;
        }

        GtkTreeIter child;
        gtk_tree_store_append(store, &child, &parent);
        gtk_tree_store_set(store, &child, 0, pEntity->eclass->name, 1, pEntity, -1);
      }

      while (entitymap)
      {
        free (entitymap->data);
        entitymap = g_slist_remove (entitymap, entitymap->data);
      }
    }

    g_object_unref(G_OBJECT(store));
  }

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (vbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

  {
    GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Key"), renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Value"), renderer, "text", 1, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    gtk_widget_show(view);

    g_object_set_data(G_OBJECT(dlg), "keyvalues", store);
    gtk_container_add(GTK_CONTAINER (scr), view);

    g_object_unref(G_OBJECT(store));
  }

  hbox2 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("Select"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
  		      GTK_SIGNAL_FUNC (entitylist_select), dlg);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Close"));;
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  save_window_pos (dlg, g_PrefsDlg.mWindowInfo.posMapInfoWnd);

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  dlg = NULL;
}

// =============================================================================
// Rotate dialog

static void rotatedlg_apply (GtkWidget *widget, gpointer data)
{
  GtkSpinButton *spin;
  float f;

  spin = GTK_SPIN_BUTTON (g_object_get_data (G_OBJECT (data), "x"));
  f = gtk_spin_button_get_value_as_float (spin);
  if (f != 0.0)
    Select_RotateAxis(0,f);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 0.0f); // reset to 0 on Apply

  spin = GTK_SPIN_BUTTON (g_object_get_data (G_OBJECT (data), "y"));
  f = gtk_spin_button_get_value_as_float (spin);
  if (f != 0.0)
    Select_RotateAxis(1,f);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 0.0f);

  spin = GTK_SPIN_BUTTON (g_object_get_data (G_OBJECT (data), "z"));
  f = gtk_spin_button_get_value_as_float (spin);
  if (f != 0.0)
    Select_RotateAxis(2,f);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 0.0f);
}

void DoRotateDlg ()
{
  GtkWidget *dlg, *hbox, *vbox, *table, *label, *button;
  GtkWidget *x, *y, *z;
  GtkObject *adj;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Arbitrary rotation"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  table = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("  X  "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label = gtk_label_new (_("  Y  "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label = gtk_label_new (_("  Z  "));

  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  adj = gtk_adjustment_new (0, -359, 359, 1, 10, 10);
  x = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  g_object_set_data (G_OBJECT (dlg), "x", x);
  gtk_widget_show (x);
  gtk_table_attach (GTK_TABLE (table), x, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_usize (x, 60, -2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (x), TRUE);

  adj = gtk_adjustment_new (0, -359, 359, 1, 10, 10);
  y = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  g_object_set_data (G_OBJECT (dlg), "y", y);
  gtk_widget_show (y);
  gtk_table_attach (GTK_TABLE (table), y, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (y), TRUE);

  adj = gtk_adjustment_new (0, -359, 359, 1, 10, 10);
  z = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  g_object_set_data (G_OBJECT (dlg), "z", z);
  gtk_widget_show (z);
  gtk_table_attach (GTK_TABLE (table), z, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (z), TRUE);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  button = gtk_button_new_with_label (_("Apply"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (rotatedlg_apply), dlg);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
    rotatedlg_apply (button, dlg);

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Gamma dialog

void DoGamma ()
{
  GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Gamma"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);

  label = gtk_label_new (_("0.0 is brightest\n1.0 is darkest"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);

  label = gtk_label_new (_("You must restart for the\nsettings to take effect"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  // Initialize dialog
  char buf[16];
  sprintf (buf, "%1.1f", g_qeglobals.d_savedinfo.fGamma);
  gtk_entry_set_text (GTK_ENTRY (entry), buf);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
    g_qeglobals.d_savedinfo.fGamma = g_strtod (gtk_entry_get_text (GTK_ENTRY (entry)), NULL);

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Find Brush Dialog

// helper function to walk through the active brushes only and drop the regioned out ones
bool WalkRegionBrush (brush_t **b, entity_t *e)
{
  brush_t *b2;
  do
  {
    for(b2=active_brushes.next ; b2 != &active_brushes ; b2=b2->next)
    {
      if (b2==*b)
        break; // this is an active brush
    }
    if (b2==&active_brushes)
    {
      // this is a regioned out brush
      *b = (*b)->onext;
      if (*b == &e->brushes)
      {
        Sys_Status ("No such brush", 0);
        return false;
      }
    }
  } while (b2==&active_brushes);
  return true;
}

void SelectBrush (int entitynum, int brushnum)
{
  entity_t *e;
  brush_t *b;
  int i;

  // making this work when regioning is on too

  if (entitynum == 0)
    e = world_entity;
  else
  {
    e = entities.next;
    while (--entitynum)
    {
      e = e->next;
      if (e == &entities)
      {
        Sys_Status ("No such entity", 0);
        return;
      }
      if (region_active)
      {
        // we need to make sure we walk to the next 'active' entity to have a valid --entitynum
        // that is, find a brush that belongs to this entity in the active brushes
        do
        {
          for (b = active_brushes.next ; b != &active_brushes ; b=b->next)
          {
            if (b->owner == e)
              break; // this is an active entity
          }
          if (b==&active_brushes)
          {
            // this is a regioned out entity
            e = e->next;
            // don't walk past the end either
            if (e == &entities)
            {
              Sys_Status ("No such entity", 0);
              return;
            }
          }
        } while(b==&active_brushes);
      }
    }
  }

  b = e->brushes.onext;
  if (b == &e->brushes)
  {
    Sys_Status ("No such brush", 0);
    return;
  }
  if (region_active)
  {
    if (!WalkRegionBrush(&b, e))
      return;
  }

  while (brushnum--)
  {
    b = b->onext;
    if (b == &e->brushes)
    {
      Sys_Status ("No such brush", 0);
      return;
    }
    if (region_active)
    {
      if (!WalkRegionBrush(&b, e))
        return;
    }
  }

  Brush_RemoveFromList (b);
  Brush_AddToList (b, &selected_brushes);

  Sys_UpdateWindows (W_ALL);
  for (i = 0; i < 3; i++)
  {
    if (g_pParentWnd->GetXYWnd())
      g_pParentWnd->GetXYWnd()->GetOrigin()[i] = (b->mins[i] + b->maxs[i])/2;

    if (g_pParentWnd->GetXZWnd())
      g_pParentWnd->GetXZWnd()->GetOrigin()[i] = (b->mins[i] + b->maxs[i])/2;

    if (g_pParentWnd->GetYZWnd())
      g_pParentWnd->GetYZWnd()->GetOrigin()[i] = (b->mins[i] + b->maxs[i])/2;
  }

  Sys_Status ("Selected", 0);
}

static void GetSelectionIndex (int *ent, int *brush)
{
  brush_t *b, *b2;
  entity_t *entity;

  *ent = *brush = 0;

  b = selected_brushes.next;
  if (b == &selected_brushes)
    return;

  // find entity
  if (b->owner != world_entity)
  {
    (*ent)++;
    for (entity = entities.next; entity != &entities; entity=entity->next, (*ent)++)
      ;
  }

  // find brush
  for (b2=b->owner->brushes.onext; b2 != b && b2 != &b->owner->brushes; b2=b2->onext, (*brush)++)
    ;
}

void DoFind ()
{
  GtkWidget *dlg, *vbox, *hbox, *table, *label, *button, *entity, *brush;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Find Brush"));
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

  table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("Entity number"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label = gtk_label_new (_("Brush number"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  entity = gtk_entry_new ();
  gtk_widget_show (entity);
  gtk_table_attach (GTK_TABLE (table), entity, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  brush = gtk_entry_new ();
  gtk_widget_show (brush);
  gtk_table_attach (GTK_TABLE (table), brush, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  // Initialize dialog
  char buf[16];
  int ent, br;

  GetSelectionIndex (&ent, &br);
  sprintf (buf, "%i", ent);
  gtk_entry_set_text (GTK_ENTRY (entity), buf);
  sprintf (buf, "%i", br);
  gtk_entry_set_text (GTK_ENTRY (brush), buf);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    const char *entstr = gtk_entry_get_text (GTK_ENTRY (entity));
    const char *brushstr = gtk_entry_get_text (GTK_ENTRY (brush));
    SelectBrush (atoi(entstr), atoi(brushstr));
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Arbitrary Sides dialog

void DoSides (bool bCone, bool bSphere, bool bTorus)
{
  GtkWidget *dlg, *vbox, *hbox, *button, *label, *entry;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Arbitrary sides"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new (_("Sides:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    const char *str = gtk_entry_get_text (GTK_ENTRY (entry));

    if (bCone)
      Brush_MakeSidedCone(atoi(str));
    else if (bSphere)
      Brush_MakeSidedSphere(atoi(str));
    else
      Brush_MakeSided (atoi(str));
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// New Patch dialog

void DoNewPatchDlg ()
{
  GtkWidget *dlg, *hbox, *table, *vbox, *label, *button, *combo;
  GtkWidget *width, *height;
  GList *combo_list = (GList*)NULL;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Patch density"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("Width:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Height:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  combo_list = g_list_append (combo_list, (void *)_("3"));
  combo_list = g_list_append (combo_list, (void *)_("5"));
  combo_list = g_list_append (combo_list, (void *)_("7"));
  combo_list = g_list_append (combo_list, (void *)_("9"));
  combo_list = g_list_append (combo_list, (void *)_("11"));
  combo_list = g_list_append (combo_list, (void *)_("13"));
  combo_list = g_list_append (combo_list, (void *)_("15"));

  combo = gtk_combo_new ();
  width = GTK_COMBO (combo)->entry;
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo_list);
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo = gtk_combo_new ();
  height = GTK_COMBO (combo)->entry;
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo_list);
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  // Initialize dialog
  g_list_free (combo_list);
  gtk_entry_set_text (GTK_ENTRY (width), _("3"));
  gtk_entry_set_editable (GTK_ENTRY (width), FALSE);
  gtk_entry_set_text (GTK_ENTRY (height), _("3"));
  gtk_entry_set_editable (GTK_ENTRY (height), FALSE);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    const char* w = gtk_entry_get_text (GTK_ENTRY (width));
    const char* h = gtk_entry_get_text (GTK_ENTRY (height));

    Patch_GenericMesh(atoi (w), atoi (h), g_pParentWnd->ActiveXY ()->GetViewType ());
    Sys_UpdateWindows (W_ALL);
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// New Patch dialog

void DoScaleDlg ()
{
  GtkWidget *dlg, *hbox, *table, *vbox, *label, *button;
  GtkWidget *x, *y, *z;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Scale"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  table = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("X:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Y:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Z:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  x = gtk_entry_new ();
  gtk_widget_show (x);
  gtk_table_attach (GTK_TABLE (table), x, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  y = gtk_entry_new ();
  gtk_widget_show (y);
  gtk_table_attach (GTK_TABLE (table), y, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  z = gtk_entry_new ();
  gtk_widget_show (z);
  gtk_table_attach (GTK_TABLE (table), z, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  // Initialize dialog
  gtk_entry_set_text (GTK_ENTRY (x), _("1.0"));
  gtk_entry_set_text (GTK_ENTRY (y), _("1.0"));
  gtk_entry_set_text (GTK_ENTRY (z), _("1.0"));

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    float sx, sy, sz;
    sx = atof (gtk_entry_get_text (GTK_ENTRY (x)));
    sy = atof (gtk_entry_get_text (GTK_ENTRY (y)));
    sz = atof (gtk_entry_get_text (GTK_ENTRY (z)));

    if (sx > 0 && sy > 0 && sz > 0)
    {
      Select_Scale(sx, sy, sz);
      Sys_UpdateWindows (W_ALL);
    }
    else
      Sys_Printf("Warning.. Tried to scale by a zero value.");
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Thicken Patch dialog

void DoThickenDlg ()
{
  GtkWidget *dlg, *vbox, *hbox, *vbox2, *button, *label;
  GtkWidget *amount, *seams, *group;
  int loop = 1, ret = IDCANCEL;
  static qboolean bGroupResult = true;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Thicken Patch"));
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

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  label = gtk_label_new (_("This produces a set of patches\n"
			 "that contains the original patch along with the\n"
			 "'thick' patch and an optimal set of seam patches."));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  label = gtk_label_new (_("Amount:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  amount = gtk_entry_new ();
  gtk_widget_show (amount);
  gtk_box_pack_start (GTK_BOX (hbox), amount, FALSE, FALSE, 0);

  seams = gtk_check_button_new_with_label (_("Seams"));
  gtk_widget_show (seams);
  gtk_box_pack_start (GTK_BOX (hbox), seams, FALSE, FALSE, 0);

  // bGroupResult
  group = gtk_check_button_new_with_label(_("Result to func_group"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(group), bGroupResult);
  gtk_box_pack_start(GTK_BOX(vbox), group, FALSE, FALSE, 0);
  gtk_widget_show(group);


  // Initialize dialog
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (seams), TRUE);
  gtk_entry_set_text (GTK_ENTRY (amount), "8");

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group)))
      bGroupResult = true;
    else
      bGroupResult = false;
    Patch_Thicken (atoi (gtk_entry_get_text (GTK_ENTRY (amount))),
		   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (seams)), bGroupResult);
    Sys_UpdateWindows (W_ALL);
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// About dialog (no program is complete without one)

void about_button_changelog (GtkWidget *widget, gpointer data)
{
  Str log;
  log = g_strAppPath;
  log += "changelog.txt";
  OpenURL(log.GetBuffer());
}

void about_button_credits (GtkWidget *widget, gpointer data)
{
  Str cred;
  cred = g_strAppPath;
  cred += "credits.html";
  OpenURL(cred.GetBuffer());
}

void DoAbout ()
{
  GtkWidget *dlg, *vbox, *vbox2, *hbox, *frame, *table, *label, *pixmap, *button, *sc_extensions, *text_extensions;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("About GtkRadiant"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (dlg), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, FALSE, 0);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  pixmap = new_pixmap (g_pParentWnd->m_pWidget, "logo.bmp");
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (frame), pixmap);

  label = gtk_label_new ("GtkRadiant " RADIANT_VERSION "\n"
    __DATE__ "\n\n"
    RADIANT_ABOUTMSG "\n\n"
    "By qeradiant.com\n\n"
    "This product contains software technology\n"
    "from id Software, Inc. ('id Technology').\n"
    "id Technology 2000 id Software,Inc.\n\n"
    "GtkRadiant is unsupported, however\n"
    "you may report your problems at\n"
    "http://zerowing.idsoftware.com/bugzilla"
  );

  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));

  button = gtk_button_new_with_label (_("Credits"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (about_button_credits), NULL);

  button = gtk_button_new_with_label (_("Changelog"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (about_button_changelog), NULL);

  frame = gtk_frame_new (_("OpenGL Properties"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

  table = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);

  label = gtk_label_new (_("Vendor:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Version:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Renderer:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new ((char*)qglGetString (GL_VENDOR));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new ((char*)qglGetString (GL_VERSION));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new ((char*)qglGetString (GL_RENDERER));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  frame = gtk_frame_new (_("OpenGL Extensions"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  sc_extensions = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(hbox), sc_extensions, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_extensions), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sc_extensions), GTK_SHADOW_IN);
  gtk_widget_show(sc_extensions);

  text_extensions = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_extensions), FALSE);
  gtk_container_add (GTK_CONTAINER (sc_extensions), text_extensions);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_extensions));
  gtk_text_buffer_set_text(buffer, (char *)qglGetString(GL_EXTENSIONS), -1);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_extensions), GTK_WRAP_WORD);;
  gtk_widget_show(text_extensions);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Command List dialog

void DoCommandListDlg ()
{
  GtkWidget *dlg, *vbox, *hbox, *scr, *button;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Mapped Commands"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);
  gtk_window_set_default_size (GTK_WINDOW (dlg), 400, 400);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (hbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scr), GTK_SHADOW_IN);

  {
    GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Command"), renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(_("Key"), renderer, "text", 1, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    gtk_widget_show(view);
    gtk_container_add(GTK_CONTAINER (scr), view);

    {
      // Initialize dialog
      CString path;
      path = g_strTempPath;
      path += "commandlist.txt";

      GSList *cmds = NULL;
      int n;

      for (n = 0; n < g_nCommandCount; n++)
        cmds = g_slist_append (cmds, g_Commands[n].m_strCommand);
      cmds = g_slist_sort (cmds, (gint (*)(const void *, const void *))strcmp);

      Sys_Printf("Writing the command list to %s", path.GetBuffer() );
      FILE * fileout = fopen( path.GetBuffer (), "wt" );

      while (cmds)
      {
        for (n = 0; n < g_nCommandCount; n++)
          if (cmds->data == g_Commands[n].m_strCommand)
	    break;

        char c = g_Commands[n].m_nKey;
        CString strLine, strMod(""), strKeys (c);

        for (int k = 0; k < g_nKeyCount; k++)
        {
          if (g_Keys[k].m_nVKKey == g_Commands[n].m_nKey)
          {
            strKeys = g_Keys[k].m_strName;
            break;
          }
        }

        if (g_Commands[n].m_nModifiers & RAD_SHIFT)
          strMod = "Shift";
        if (g_Commands[n].m_nModifiers & RAD_ALT)
          strMod += (strMod.GetLength() > 0) ? " + Alt" : "Alt";
        if (g_Commands[n].m_nModifiers & RAD_CONTROL)
          strMod += (strMod.GetLength() > 0) ? " + Control" : "Control";
        if (strMod.GetLength() > 0)
          strMod += " + ";
        strMod += strKeys;

        {
          GtkTreeIter iter;
          gtk_list_store_append(store, &iter);
          gtk_list_store_set(store, &iter, 0, g_Commands[n].m_strCommand, 1, strMod.GetBuffer (), -1);
        }

        if (fileout != NULL)
        {
          strLine.Format("%-25s %s\r\n", g_Commands[n].m_strCommand, strMod.GetBuffer ());
          fputs (strLine.GetBuffer (), fileout);
        }

        cmds = g_slist_remove (cmds, cmds->data);
      }

      if (fileout != NULL)
        fclose (fileout);
    }

    g_object_unref(G_OBJECT(store));
  }

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Close"));;
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Texture List dialog

void DoTextureListDlg ()
{
  GtkWidget *dlg, *vbox, *hbox, *scr, *button;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Textures"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);
  gtk_window_set_default_size (GTK_WINDOW (dlg), 400, 400);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (hbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scr), GTK_SHADOW_IN);

  GtkWidget* texture_list;

  {
    GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    gtk_widget_show(view);
    gtk_container_add(GTK_CONTAINER (scr), view);

    {
      // Initialize dialog
      GSList *textures = (GSList*)NULL;
      FillTextureMenu(&textures);
      while (textures != NULL)
      {
        {
          GtkTreeIter iter;
          gtk_list_store_append(store, &iter);
          gtk_list_store_set(store, &iter, 0, (gchar*)textures->data, -1);
        }
        free (textures->data);
        textures = g_slist_remove (textures, textures->data);
      }
    }

    g_object_unref(G_OBJECT(store));

    texture_list = view;
  }

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Load"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Close"));;
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(texture_list));

    GtkTreeModel* model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
      if(gtk_tree_path_get_depth(path) == 1)
        Texture_ShowDirectory(gtk_tree_path_get_indices(path)[0] + CMD_TEXTUREWAD);
      gtk_tree_path_free(path);
    }
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
// Cap dialog

int DoCapDlg (int *type, bool *b_GroupResult)
{
  GtkWidget *dlg, *vbox, *hbox, *table, *pixmap, *button, *group_toggle, *radio_vbox;
  GtkWidget *bevel, *endcap, *ibevel, *iendcap;
  GSList *group = (GSList*)NULL;
	int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Cap"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

	hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  // Gef: Added a vbox to contain the toggle buttons
  radio_vbox = gtk_vbox_new(FALSE, 4);
  gtk_container_add(GTK_CONTAINER(hbox), radio_vbox);
  gtk_widget_show(radio_vbox);

  table = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (radio_vbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  pixmap = new_pixmap (g_pParentWnd->m_pWidget, "cap_bevel.bmp");
  gtk_widget_show (pixmap);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  pixmap = new_pixmap (g_pParentWnd->m_pWidget, "cap_endcap.bmp");
  gtk_widget_show (pixmap);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  pixmap = new_pixmap (g_pParentWnd->m_pWidget, "cap_ibevel.bmp");
  gtk_widget_show (pixmap);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  pixmap = new_pixmap (g_pParentWnd->m_pWidget, "cap_iendcap.bmp");
  gtk_widget_show (pixmap);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  bevel = gtk_radio_button_new_with_label (group, _("Bevel"));
  gtk_widget_show (bevel);
  gtk_table_attach (GTK_TABLE (table), bevel, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (bevel));

  endcap = gtk_radio_button_new_with_label (group, _("Endcap"));
  gtk_widget_show (endcap);
  gtk_table_attach (GTK_TABLE (table), endcap, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (endcap));

  ibevel = gtk_radio_button_new_with_label (group, _("Inverted Bevel"));
  gtk_widget_show (ibevel);
  gtk_table_attach (GTK_TABLE (table), ibevel, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (ibevel));

  iendcap = gtk_radio_button_new_with_label (group, _("Inverted Endcap"));
  gtk_widget_show (iendcap);
  gtk_table_attach (GTK_TABLE (table), iendcap, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (iendcap));

  // Gef: added radio toggle for func_grouping capped patches
  group_toggle = gtk_check_button_new_with_label(_("Result to func_group"));
  gtk_container_add(GTK_CONTAINER(radio_vbox), group_toggle);
  gtk_widget_show(group_toggle);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  // Gef: Set the state of the func_group toggle
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (group_toggle), *b_GroupResult);

  // Initialize dialog
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bevel), TRUE);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bevel)))
      *type = BEVEL; //*type = CapDialog::BEVEL;
    else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (endcap)))
      *type = ENDCAP; //*type = CapDialog::ENDCAP;
    else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ibevel)))
      *type = IBEVEL; // *type = CapDialog::IBEVEL;
    else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iendcap)))
      *type = IENDCAP; // *type = CapDialog::IENDCAP;

    // Gef: Added toggle for optional cap func_grouping
    *b_GroupResult = (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group_toggle));
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return ret;
}

// =============================================================================
// Scripts dialog

void DoScriptsDlg ()
{
  GtkWidget *dlg, *vbox, *vbox2, *hbox, *label, *button, *scr;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Available Scripts - Not Implemented Yet"));
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

  label = gtk_label_new (_("WARNING: BrushScripting is in a highly experimental state and is\n"
			 "far from complete. If you attempt to use them it is VERY LIKELY\n"
			 "that Radiant will crash. Save your work before attempting to\n"
			 "make use of any scripting features."));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  scr = gtk_scrolled_window_new ((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (hbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scr), GTK_SHADOW_IN);

  GtkWidget* scripts_list;

  {
    GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

    {
      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    gtk_widget_show(view);
    gtk_container_add(GTK_CONTAINER (scr), view);

    {
      // Initialize dialog
      CString strINI;
      strINI = g_strGameToolsPath;
      strINI += "/scripts.ini";
      FILE *f;

      f = fopen (strINI.GetBuffer(), "rt");
      if (f != NULL)
      {
        char line[1024], *ptr;

        // read section names
        while (fgets (line, 1024, f) != 0)
        {
          if (line[0] != '[')
	    continue;

          ptr = strchr (line, ']');
          *ptr = '\0';

          {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, line, -1);
          }
        }
        fclose (f);
      }
    }

    g_object_unref(G_OBJECT(store));

    scripts_list = view;
  }

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Run"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("New..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Edit..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Close"));
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(scripts_list));

    GtkTreeModel* model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
      char* script;
      gtk_tree_model_get(model, &iter, 0, &script, -1);
      RunScriptByName(script, true);
      g_free(script);
    }
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);
}

// =============================================================================
//  dialog

int DoBSInputDlg (const char *fields[5], float values[5])
{
  GtkWidget *dlg, *vbox, *hbox, *label, *button;
  GtkWidget *entries[5];
  int i, loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("BrushScript Input"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  // Create entries and initialize them
  for (i = 0; i < 5; i++)
  {
    if (strlen (fields[i]) == 0)
      continue;

    label = gtk_label_new (fields[i]);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

    entries[i] = gtk_entry_new ();
    gtk_widget_show (entries[i]);
    gtk_box_pack_start (GTK_BOX (vbox), entries[i], TRUE, TRUE, 0);

    char buf[32];
    sprintf (buf, "%f", values[i]);
    gtk_entry_set_text (GTK_ENTRY (entries[i]), buf);
  }

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  for (i = 0; i < 5; i++)
  {
    if (strlen (fields[i]) == 0)
      continue;

    values[i] = atof (gtk_entry_get_text (GTK_ENTRY (entries[i])));
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return ret;
}

// =============================================================================
// TextureLayout dialog

int DoTextureLayout (float *fx, float *fy)
{
  GtkWidget *dlg, *vbox, *hbox, *table, *label, *button;
  GtkWidget *x, *y;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Patch texture layout"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Texture will be fit across the patch based\n"
			 "on the x and y values given. Values of 1x1\n"
			 "will \"fit\" the texture. 2x2 will repeat\n"
			 "it twice, etc."));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  label = gtk_label_new (_("Texture x:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  label = gtk_label_new (_("Texture y:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  x = gtk_entry_new ();
  gtk_widget_show (x);
  gtk_table_attach (GTK_TABLE (table), x, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  y = gtk_entry_new ();
  gtk_widget_show (y);
  gtk_table_attach (GTK_TABLE (table), y, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  // Initialize
  gtk_entry_set_text (GTK_ENTRY (x), _("4.0"));
  gtk_entry_set_text (GTK_ENTRY (y), _("4.0"));

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
  {
    *fx = atof (gtk_entry_get_text (GTK_ENTRY (x)));
    *fy = atof (gtk_entry_get_text (GTK_ENTRY (y)));
  }

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return ret;
}

// =============================================================================
// Name dialog

char* DoNameDlg (const char* title)
{
  GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry;
  int loop = 1, ret = IDCANCEL;
  char *str;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), title);
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new (_("Name:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
    str = strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
  else
    str = NULL;

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return str;
}

// =============================================================================
// NewProject dialog

char* DoNewProjectDlg ()
{
  GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry, *check;
  int loop = 1, ret = IDCANCEL;
  char *str;

  // start by a warning message
// mattn: URLs no longer valid
//  CString msg;
//  msg = "Are you sure you want a new project?\n";
//  msg += "Please note that creating a new project is not the prefered way to setup GtkRadiant for mod editing.\n";
//  msg += "Check http://www.qeradiant.com/faq/index.cgi?file=220 for more information";
//  if (gtk_MessageBox(NULL, msg.GetBuffer(), _("Confirm"), MB_YESNO, "http://www.qeradiant.com/faq/index.cgi?file=220" ) == IDNO)
//  {
//    return NULL;
//  }

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("New Project"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("This will create a new directory beneath your\n"
			 "game path based on the project name you give."));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  label = gtk_label_new (_("Project name:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);

  check = gtk_check_button_new_with_label (_("Include game dll files"));
  gtk_widget_show (check);
  gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, 0);
  gtk_widget_set_sensitive (check, FALSE);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_set_usize (button, 60, -2);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
    str = strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
  else
    str = NULL;

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return str;
}

// =============================================================================
// Text Editor dialog

// master window widget
static GtkWidget *text_editor = NULL;
static GtkWidget *text_widget; // slave, text widget from the gtk editor

static gint editor_delete (GtkWidget *widget, gpointer data)
{
  if (gtk_MessageBox (widget, _("Close the shader editor ?"), _("Radiant"), MB_YESNO) == IDNO)
    return TRUE;

  gtk_widget_hide (text_editor);

  return TRUE;
}

static void editor_save (GtkWidget *widget, gpointer data)
{
  FILE *f = fopen ((char*)g_object_get_data (G_OBJECT (data), "filename"), "w");
  gpointer text = g_object_get_data (G_OBJECT (data), "text");

  if (f == NULL)
  {
    gtk_MessageBox (GTK_WIDGET(data), _("Error saving file !"));
    return;
  }

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds ( buffer, &start, &end);
  char *str = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  fwrite (str, 1, strlen (str), f);
  fclose (f);
  g_free(str);
}

static void editor_close (GtkWidget *widget, gpointer data)
{
  if (gtk_MessageBox (text_editor, _("Close the shader editor ?"), _("Radiant"), MB_YESNO) == IDNO)
    return;

  gtk_widget_hide (text_editor);
}

// several attempts
#if 0
#ifdef _WIN32

HWND FindEditWindow()
{
  return FindWindow("TFormEditPadLite", NULL);
}

HWND FindEditWindow()
{
  HWND hwnd = FindWindow("TFormEditPadLite", NULL);
  if (hwnd)
  {
    hwnd = FindWindowEx(hwnd, NULL, "TPanel", NULL);
    if (hwnd)
    {
      hwnd = FindWindowEx(hwnd, NULL, "TPanel", NULL);
      if (hwnd)
      {
        hwnd = FindWindowEx(hwnd, NULL, "TEditPadEditor", NULL);
        if (hwnd)
        {
          hwnd = FindWindowEx(hwnd, NULL, "TWinControlProxy", NULL);
          return hwnd;
        }
      }
    }
  }
  return NULL;
}

HWND FindEditWindow()
{
  HWND hwnd = FindWindow("TFormEditPadLite", NULL);
  if (hwnd)
  {
    hwnd = FindWindowEx(hwnd, NULL, "TPanel", NULL);
    if (hwnd)
    {
      hwnd = FindWindowEx(hwnd, NULL, "TPanel", NULL);
      if (hwnd)
      {
        hwnd = FindWindowEx(hwnd, NULL, "TPanel", NULL);
        if (hwnd)
        {
          hwnd = FindWindowEx(hwnd, NULL, "TFrameSearchReplace", NULL);
          if (hwnd)
          {
            hwnd = FindWindowEx(hwnd, NULL, "TJGStringEditorControl", NULL);
            return hwnd;
          }
        }
      }
    }
  }
  return NULL;
}

HWND FindEditWindow()
{
  HWND hwnd = FindWindow("TEditPadForm", NULL);
  HWND hwndEdit = NULL;
  if (hwnd != NULL)
  {
    HWND hwndTab = FindWindowEx(hwnd, NULL, "TTabControl", NULL);
    if (hwndTab != NULL)
    {
      hwndEdit = FindWindowEx(hwndTab, NULL, "TRicherEdit", NULL);
    }
  }
  return hwndEdit;
}
#endif
#endif // #if 0

static void CreateGtkTextEditor ()
{
  GtkWidget *dlg;
  GtkWidget *vbox, *hbox, *button, *scr, *text;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (editor_delete), NULL);
  gtk_window_set_default_size (GTK_WINDOW (dlg), 600, 300);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (dlg), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scr);
  gtk_box_pack_start (GTK_BOX (vbox), scr, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

  text = gtk_text_view_new();
  gtk_container_add (GTK_CONTAINER (scr), text);
  gtk_widget_show (text);
  g_object_set_data (G_OBJECT (dlg), "text", text);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(text), TRUE);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("Close"));
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (editor_close), dlg);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Save"));
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (editor_save), dlg);
  gtk_widget_set_usize (button, 60, -2);

  text_editor = dlg;
  text_widget = text;
}

static void DoGtkTextEditor (const char* filename, guint cursorpos)
{
  if (!text_editor)
    CreateGtkTextEditor(); // build it the first time we need it

  // Load file
  FILE *f = fopen (filename, "r");

  if (f == NULL)
  {
    Sys_Printf("Unable to load file %s in shader editor.\n", filename);
    gtk_widget_hide (text_editor);
  }
  else
  {
    fseek (f, 0, SEEK_END);
    int len = ftell (f);
    void *buf = qmalloc (len);
    void *old_filename;

    rewind (f);
    fread (buf, 1, len, f);

    gtk_window_set_title (GTK_WINDOW (text_editor), filename);

    GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_widget));
    gtk_text_buffer_set_text(text_buffer, (char*)buf, len);

    old_filename = g_object_get_data (G_OBJECT (text_editor), "filename");
    if (old_filename)
      free(old_filename);
    g_object_set_data (G_OBJECT (text_editor), "filename", strdup (filename));

    // trying to show later
    gtk_widget_show (text_editor);

#ifdef _WIN32
    while (gtk_events_pending ())
      gtk_main_iteration ();
#endif

    // only move the cursor if it's not exceeding the size..
    // NOTE: this is erroneous, cursorpos is the offset in bytes, not in characters
    // len is the max size in bytes, not in characters either, but the character count is below that limit..
    // thinking .. the difference between character count and byte count would be only because of CR/LF?
    {
      GtkTextIter text_iter;
      // character offset, not byte offset
      gtk_text_buffer_get_iter_at_offset(text_buffer, &text_iter, cursorpos);
      gtk_text_buffer_place_cursor(text_buffer, &text_iter);
    }

#ifdef _WIN32
    gtk_widget_queue_draw(text_widget);
#endif

    free (buf);
    fclose (f);
  }
}

void DoTextEditor (const char* filename, int cursorpos)
{
  CString strEditCommand;
#ifdef _WIN32
  if (g_PrefsDlg.m_bUseWin32Editor)
  {
    Sys_Printf("opening file '%s' (line %d info ignored)\n", filename);
    ShellExecute((HWND)GDK_WINDOW_HWND (g_pParentWnd->m_pWidget->window), "open", filename, NULL, NULL, SW_SHOW );
    return;
  }
#else
  // check if a custom editor is set
  if((g_PrefsDlg.m_bUseCustomEditor) && (g_PrefsDlg.m_strEditorCommand.GetLength() > 0))
  {
    strEditCommand = g_PrefsDlg.m_strEditorCommand;
    strEditCommand += " \"";
    strEditCommand += filename;
    strEditCommand += "\"";

    Sys_Printf("Launching: %s\n", strEditCommand.GetBuffer());
    // note: linux does not return false if the command failed so it will assume success
    if (Q_Exec(NULL, (char *)strEditCommand.GetBuffer(), NULL, true) == false)
    {
      Sys_FPrintf(SYS_WRN, "Warning: Failed to execute %s, using default\n", strEditCommand.GetBuffer());
    }
    else
    {
      // the command (appeared) to run successfully, no need to do anything more
      return;
    }
  }
#endif

  DoGtkTextEditor (filename, cursorpos);

  // old win32 code with EditPad bindings, broken
#if 0
  strEditCommand = g_strAppPath.GetBuffer();
  strEditCommand += "editpad.exe";
  strEditCommand += " \"";
  strEditCommand += filename;
  strEditCommand += "\"";
  if (Q_Exec(NULL, (char *)strEditCommand.GetBuffer(), NULL, true) == false)
  {
    Sys_FPrintf(SYS_WRN, "WARNING: Gtk shader editor is not fully functional on windows in general and unstable on win98 in particular.\n");
    Sys_FPrintf(SYS_WRN, "  you can use EditPad instead (install it in Radiant's directory): http://www.qeradiant.com/?data=files&files_dir=18\n");
    DoGtkTextEditor (filename, cursorpos);
  }
  else
  {
    // TTimo: we used to call Delay here, to continue processing messages. But it seems to induce a lot of instabilities.
    // so now the user will simply have to wait.
    Sleep( 1500 );

  	// now grab the edit window and scroll to the shader we want to edit
    HWND hwndEdit = FindEditWindow();

    if (hwndEdit != NULL)
      PostMessage(hwndEdit, EM_SETSEL, cursorpos, cursorpos);
    else
      Sys_Printf("Unable to load shader editor.\n");
  }
#endif
}

// =============================================================================
// Light Intensity dialog

int DoLightIntensityDlg (int *intensity)
{
  GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry;
  int loop = 1, ret = IDCANCEL;

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (dlg), _("Light intensity"));
  gtk_signal_connect (GTK_OBJECT (dlg), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  g_object_set_data (G_OBJECT (dlg), "loop", &loop);
  g_object_set_data (G_OBJECT (dlg), "ret", &ret);

  GtkAccelGroup *accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (dlg), accel_group);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (dlg), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("ESC for default, ENTER to validate"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("OK"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
  gtk_widget_add_accelerator (button, "clicked", accel_group,
                              GDK_Return, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
  gtk_widget_set_usize (button, 60, -2);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
  gtk_widget_add_accelerator (button, "clicked", accel_group,
                              GDK_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
  gtk_widget_set_usize (button, 60, -2);

  char buf[16];
  sprintf (buf, "%d", *intensity);
  gtk_entry_set_text (GTK_ENTRY (entry), buf);

  gtk_grab_add (dlg);
  gtk_widget_show (dlg);

  while (loop)
    gtk_main_iteration ();

  if (ret == IDOK)
    *intensity = atoi (gtk_entry_get_text (GTK_ENTRY (entry)));

  gtk_grab_remove (dlg);
  gtk_widget_destroy (dlg);

  return ret;
}
