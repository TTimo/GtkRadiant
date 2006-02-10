/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

#include "patchdialog.h"

#include "itexdef.h"

#include "debugging/debugging.h"

#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcheckbutton.h>

#include "gtkutil/idledraw.h"
#include "gtkutil/entry.h"
#include "gtkutil/button.h"
#include "gtkutil/nonmodal.h"
#include "dialog.h"
#include "gtkdlgs.h"
#include "mainframe.h"
#include "patchmanip.h"
#include "patch.h"
#include "commands.h"
#include "preferences.h"


#include <gdk/gdkkeysyms.h>

// the increment we are using for the patch inspector (this is saved in the prefs)
struct pi_globals_t
{
  float shift[2];
  float scale[2];
  float rotate;

  pi_globals_t()
  {
    shift[0] = 8.0f;
    shift[1] = 8.0f;
    scale[0] = 0.5f;
    scale[1] = 0.5f;
    rotate = 45.0f;
  }
};

pi_globals_t g_pi_globals;

class PatchFixedSubdivisions
{
public:
  PatchFixedSubdivisions() : m_enabled(false), m_x(0), m_y(0)
  {
  }
  PatchFixedSubdivisions(bool enabled, std::size_t x, std::size_t y) : m_enabled(enabled), m_x(x), m_y(y)
  {
  }
  bool m_enabled;
  std::size_t m_x;
  std::size_t m_y;
};

void Patch_getFixedSubdivisions(const Patch& patch, PatchFixedSubdivisions& subdivisions)
{
  subdivisions.m_enabled = patch.m_patchDef3;
  subdivisions.m_x = patch.m_subdivisions_x;
  subdivisions.m_y = patch.m_subdivisions_y;
}

const std::size_t MAX_PATCH_SUBDIVISIONS = 32;

void Patch_setFixedSubdivisions(Patch& patch, const PatchFixedSubdivisions& subdivisions)
{
  patch.undoSave();

  patch.m_patchDef3 = subdivisions.m_enabled;
  patch.m_subdivisions_x = subdivisions.m_x;
  patch.m_subdivisions_y = subdivisions.m_y;

  if(patch.m_subdivisions_x == 0)
  {
    patch.m_subdivisions_x = 4;
  }
  else if(patch.m_subdivisions_x > MAX_PATCH_SUBDIVISIONS)
  {
    patch.m_subdivisions_x = MAX_PATCH_SUBDIVISIONS;
  }
  if(patch.m_subdivisions_y == 0)
  {
    patch.m_subdivisions_y = 4;
  }
  else if(patch.m_subdivisions_y > MAX_PATCH_SUBDIVISIONS)
  {
    patch.m_subdivisions_y = MAX_PATCH_SUBDIVISIONS;
  }

  SceneChangeNotify();
  Patch_textureChanged();
  patch.controlPointsChanged();
}

class PatchGetFixedSubdivisions
{
  PatchFixedSubdivisions& m_subdivisions;
public:
  PatchGetFixedSubdivisions(PatchFixedSubdivisions& subdivisions) : m_subdivisions(subdivisions)
  {
  }
  void operator()(Patch& patch)
  {
    Patch_getFixedSubdivisions(patch, m_subdivisions);
    SceneChangeNotify();
  }
};

void Scene_PatchGetFixedSubdivisions(PatchFixedSubdivisions& subdivisions)
{
#if 1
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    Patch* patch = Node_getPatch(GlobalSelectionSystem().ultimateSelected().path().top());
    if(patch != 0)
    {
      Patch_getFixedSubdivisions(*patch, subdivisions);
    }
  }
#else
  Scene_forEachVisibleSelectedPatch(PatchGetFixedSubdivisions(subdivisions));
#endif
}

class PatchSetFixedSubdivisions
{
  const PatchFixedSubdivisions& m_subdivisions;
public:
  PatchSetFixedSubdivisions(const PatchFixedSubdivisions& subdivisions) : m_subdivisions(subdivisions)
  {
  }
  void operator()(Patch& patch) const
  {
    Patch_setFixedSubdivisions(patch, m_subdivisions);
  }
};

void Scene_PatchSetFixedSubdivisions(const PatchFixedSubdivisions& subdivisions)
{
  UndoableCommand command("patchSetFixedSubdivisions");
  Scene_forEachVisibleSelectedPatch(PatchSetFixedSubdivisions(subdivisions));
}

typedef struct _GtkCheckButton GtkCheckButton;

class Subdivisions
{
public:
  GtkCheckButton* m_enabled;
  GtkEntry* m_horizontal;
  GtkEntry* m_vertical;
  Subdivisions() : m_enabled(0), m_horizontal(0), m_vertical(0)
  {
  }
  void update()
  {
    PatchFixedSubdivisions subdivisions;
    Scene_PatchGetFixedSubdivisions(subdivisions);

    toggle_button_set_active_no_signal(GTK_TOGGLE_BUTTON(m_enabled), subdivisions.m_enabled);

    if(subdivisions.m_enabled)
    {
      entry_set_int(m_horizontal, static_cast<int>(subdivisions.m_x));
      entry_set_int(m_vertical, static_cast<int>(subdivisions.m_y));
      gtk_widget_set_sensitive(GTK_WIDGET(m_horizontal), TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(m_vertical), TRUE);
    }
    else
    {
      gtk_entry_set_text(m_horizontal, "");
      gtk_entry_set_text(m_vertical, "");
      gtk_widget_set_sensitive(GTK_WIDGET(m_horizontal), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(m_vertical), FALSE);
    }
  }
  void cancel()
  {
    update();
  }
  typedef MemberCaller<Subdivisions, &Subdivisions::cancel> CancelCaller;
  void apply()
  {
    Scene_PatchSetFixedSubdivisions(
      PatchFixedSubdivisions(
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_enabled)) != FALSE,
        static_cast<std::size_t>(entry_get_int(m_horizontal)),
        static_cast<std::size_t>(entry_get_int(m_vertical))
      )
    );
  }
  typedef MemberCaller<Subdivisions, &Subdivisions::apply> ApplyCaller;
  static void applyGtk(GtkToggleButton* toggle, Subdivisions* self)
  {
    self->apply();
  }
};

class PatchInspector : public Dialog
{
  GtkWindow* BuildDialog();
  Subdivisions m_subdivisions;
  NonModalEntry m_horizontalSubdivisionsEntry;
  NonModalEntry m_verticalSubdivisionsEntry;
public:
  IdleDraw m_idleDraw;
  WindowPositionTracker m_position_tracker;

  Patch *m_Patch;

  CopiedString m_strName;
  float	m_fS;
  float	m_fT;
  float	m_fX;
  float	m_fY;
  float	m_fZ;
/*  float	m_fHScale;
  float	m_fHShift;
  float	m_fRotate;
  float	m_fVScale;
  float	m_fVShift; */
  int   m_nCol;
  int   m_nRow;
  GtkComboBox *m_pRowCombo;
  GtkComboBox *m_pColCombo;
  std::size_t m_countRows;
  std::size_t m_countCols;

  // turn on/off processing of the "changed" "value_changed" messages
  // (need to turn off when we are feeding data in)
  // NOTE: much more simple than blocking signals
  bool m_bListenChanged;

  PatchInspector() :
    m_horizontalSubdivisionsEntry(Subdivisions::ApplyCaller(m_subdivisions), Subdivisions::CancelCaller(m_subdivisions)),
    m_verticalSubdivisionsEntry(Subdivisions::ApplyCaller(m_subdivisions), Subdivisions::CancelCaller(m_subdivisions)),
    m_idleDraw(MemberCaller<PatchInspector, &PatchInspector::GetPatchInfo>(*this))
  {
    m_fS = 0.0f;
    m_fT = 0.0f;
    m_fX = 0.0f;
    m_fY = 0.0f;
    m_fZ = 0.0f;
    m_nCol = 0;
    m_nRow = 0;
    m_countRows = 0;
    m_countCols = 0;
    m_Patch = 0;
    m_bListenChanged = true;

    m_position_tracker.setPosition(c_default_window_pos);
  }

  bool visible()
  {
    return GTK_WIDGET_VISIBLE(GetWidget());
  }

//  void UpdateInfo();
//  void SetPatchInfo();
  void GetPatchInfo();
  void UpdateSpinners(bool bUp, int nID);
  // read the current patch on map and initialize m_fX m_fY accordingly
  void UpdateRowColInfo();
  // sync the dialog our internal data structures
  // depending on the flag it will read or write
  // we use m_nCol m_nRow m_fX m_fY m_fZ m_fS m_fT m_strName
  // (NOTE: this doesn't actually commit stuff to the map or read from it)
  void importData();
  void exportData();
};

PatchInspector g_PatchInspector;

void PatchInspector_constructWindow(GtkWindow* main_window)
{
  g_PatchInspector.m_parent = main_window;
  g_PatchInspector.Create();
}
void PatchInspector_destroyWindow()
{
  g_PatchInspector.Destroy();
}

void PatchInspector_queueDraw()
{
  if(g_PatchInspector.visible())
  {
    g_PatchInspector.m_idleDraw.queueDraw();
  }
}

void DoPatchInspector()
{
  g_PatchInspector.GetPatchInfo();
  if (!g_PatchInspector.visible())
    g_PatchInspector.ShowDlg();
}

void PatchInspector_toggleShown()
{
  if (g_PatchInspector.visible())
  {
    g_PatchInspector.m_Patch = 0;
    g_PatchInspector.HideDlg();
  }
  else
    DoPatchInspector();
}


// =============================================================================
// static functions

// memorize the current state (that is don't try to undo our do before changing something else)
static void OnApply (GtkWidget *widget, gpointer data)
{
  g_PatchInspector.exportData();
  if (g_PatchInspector.m_Patch != 0)
  {
    UndoableCommand command("patchSetTexture");
    g_PatchInspector.m_Patch->undoSave();

    if (!texdef_name_valid(g_PatchInspector.m_strName.c_str()))
    {
      globalErrorStream() << "invalid texture name '" << g_PatchInspector.m_strName.c_str() << "'\n";
      g_PatchInspector.m_strName = texdef_name_default();
    }
    g_PatchInspector.m_Patch->SetShader(g_PatchInspector.m_strName.c_str());

    std::size_t r = g_PatchInspector.m_nRow;
    std::size_t c = g_PatchInspector.m_nCol;
    if(r < g_PatchInspector.m_Patch->getHeight()
      && c < g_PatchInspector.m_Patch->getWidth())
    {            
      PatchControl& p = g_PatchInspector.m_Patch->ctrlAt(r,c);
      p.m_vertex[0] = g_PatchInspector.m_fX;
      p.m_vertex[1] = g_PatchInspector.m_fY;
      p.m_vertex[2] = g_PatchInspector.m_fZ;
      p.m_texcoord[0] = g_PatchInspector.m_fS;
      p.m_texcoord[1] = g_PatchInspector.m_fT;
      g_PatchInspector.m_Patch->controlPointsChanged();
    }
  }
}

static void OnSelchangeComboColRow (GtkWidget *widget, gpointer data)
{
  if (!g_PatchInspector.m_bListenChanged)
    return;
  // retrieve the current m_nRow and m_nCol, other params are not relevant
  g_PatchInspector.exportData();
  // read the changed values ourselves
  g_PatchInspector.UpdateRowColInfo();
  // now reflect our changes
  g_PatchInspector.importData();
}

class PatchSetTextureRepeat
{
  float m_s, m_t;
public:
  PatchSetTextureRepeat(float s, float t) : m_s(s), m_t(t)
  {
  }
  void operator()(Patch& patch) const
  {
    patch.SetTextureRepeat(m_s, m_t);
  }
};

void Scene_PatchTileTexture_Selected(scene::Graph& graph, float s, float t)
{
  Scene_forEachVisibleSelectedPatch(PatchSetTextureRepeat(s, t));
  SceneChangeNotify();
}

static void OnBtnPatchdetails (GtkWidget *widget, gpointer data)
{
  UndoableCommand command("patchCapTexture");

  Scene_PatchCapTexture_Selected(GlobalSceneGraph());
}

static void OnBtnPatchfit (GtkWidget *widget, gpointer data)
{
  UndoableCommand command("patchFitTexture");

  Scene_PatchTileTexture_Selected(GlobalSceneGraph(), 1, 1);
}

static void OnBtnPatchnatural (GtkWidget *widget, gpointer data)
{
  UndoableCommand command("patchNaturalTexture");

  Scene_PatchNaturalTexture_Selected(GlobalSceneGraph());
}

static void OnBtnPatchreset (GtkWidget *widget, gpointer data)
{
  float fx, fy;
  if (DoTextureLayout (&fx, &fy) == eIDOK)
  {
    UndoableCommand command("patchTileTexture");
    Scene_PatchTileTexture_Selected(GlobalSceneGraph(), fx, fy);
  }
}

struct PatchRotateTexture
{
  float m_angle;
public:
  PatchRotateTexture(float angle) : m_angle(angle)
  {
  }
  void operator()(Patch& patch) const
  {
    patch.RotateTexture(m_angle);
  }
};

void Scene_PatchRotateTexture_Selected(scene::Graph& graph, float angle)
{
  Scene_forEachVisibleSelectedPatch(PatchRotateTexture(angle));
}

class PatchScaleTexture
{
  float m_s, m_t;
public:
  PatchScaleTexture(float s, float t) : m_s(s), m_t(t)
  {
  }
  void operator()(Patch& patch) const
  {
    patch.ScaleTexture(m_s, m_t);
  }
};

float Patch_convertScale(float scale)
{
  if(scale > 0)
  {
    return scale;
  }
  if(scale < 0)
  {
    return -1 / scale;
  }
  return 1;
}

void Scene_PatchScaleTexture_Selected(scene::Graph& graph, float s, float t)
{
  Scene_forEachVisibleSelectedPatch(PatchScaleTexture(Patch_convertScale(s), Patch_convertScale(t)));
}

class PatchTranslateTexture
{
  float m_s, m_t;
public:
  PatchTranslateTexture(float s, float t)
    : m_s(s), m_t(t)
  {
  }
  void operator()(Patch& patch) const
  {
    patch.TranslateTexture(m_s, m_t);
  }
};

void Scene_PatchTranslateTexture_Selected(scene::Graph& graph, float s, float t)
{
  Scene_forEachVisibleSelectedPatch(PatchTranslateTexture(s, t));
}

static void OnSpinChanged (GtkAdjustment *adj, gpointer data)
{
  texdef_t td;

  td.rotate = 0;
  td.scale[0] = td.scale[1] = 0;
  td.shift[0] = td.shift[1] = 0;

  if (adj->value == 0)
    return;

  if (adj == g_object_get_data (G_OBJECT (g_PatchInspector.GetWidget()), "hshift_adj"))
  {
    g_pi_globals.shift[0] = static_cast<float>(atof (gtk_entry_get_text (GTK_ENTRY (data))));

    if (adj->value > 0)
      td.shift[0] = g_pi_globals.shift[0];
    else
      td.shift[0] = -g_pi_globals.shift[0];
  }
  else if (adj == g_object_get_data (G_OBJECT (g_PatchInspector.GetWidget()), "vshift_adj"))
  {
    g_pi_globals.shift[1] = static_cast<float>(atof (gtk_entry_get_text (GTK_ENTRY (data))));

    if (adj->value > 0)
      td.shift[1] = g_pi_globals.shift[1];
    else
      td.shift[1] = -g_pi_globals.shift[1];
  }
  else if (adj == g_object_get_data (G_OBJECT (g_PatchInspector.GetWidget()), "hscale_adj"))
  {
    g_pi_globals.scale[0] = static_cast<float>(atof (gtk_entry_get_text (GTK_ENTRY (data))));
	  if (g_pi_globals.scale[0] == 0.0f)
		  return;
    if (adj->value > 0)
      td.scale[0] = g_pi_globals.scale[0];
    else
      td.scale[0] = -g_pi_globals.scale[0];
  }
  else if (adj == g_object_get_data (G_OBJECT (g_PatchInspector.GetWidget()), "vscale_adj"))
  {
    g_pi_globals.scale[1] = static_cast<float>(atof (gtk_entry_get_text (GTK_ENTRY (data))));
	  if (g_pi_globals.scale[1] == 0.0f)
		  return;
    if (adj->value > 0)
      td.scale[1] = g_pi_globals.scale[1];
    else
      td.scale[1] = -g_pi_globals.scale[1];
  }
  else if (adj == g_object_get_data (G_OBJECT (g_PatchInspector.GetWidget()), "rotate_adj"))
  {
    g_pi_globals.rotate = static_cast<float>(atof (gtk_entry_get_text (GTK_ENTRY (data))));

    if (adj->value > 0)
      td.rotate = g_pi_globals.rotate;
    else
      td.rotate = -g_pi_globals.rotate;
  }

  adj->value = 0;

  // will scale shift rotate the patch accordingly


  if (td.shift[0] || td.shift[1])
  {
    UndoableCommand command("patchTranslateTexture");
    Scene_PatchTranslateTexture_Selected (GlobalSceneGraph(), td.shift[0], td.shift[1]);
  }
  else if (td.scale[0] || td.scale[1])
  {
    UndoableCommand command("patchScaleTexture");
    Scene_PatchScaleTexture_Selected (GlobalSceneGraph(), td.scale[0], td.scale[1]);
  }
  else if (td.rotate)
  {
    UndoableCommand command("patchRotateTexture");
    Scene_PatchRotateTexture_Selected (GlobalSceneGraph(), td.rotate);
  }

  // update the point-by-point view
  OnSelchangeComboColRow(0,0);
}

static gint OnDialogKey (GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  if (event->keyval == GDK_Return)
  {
    OnApply (0, 0);
    return TRUE;
  }
  else if (event->keyval == GDK_Escape)
  {
    g_PatchInspector.GetPatchInfo();
    return TRUE;
  }
  return FALSE;
}

// =============================================================================
// PatchInspector class

GtkWindow* PatchInspector::BuildDialog()
{
  GtkWindow* window = create_floating_window("Patch Properties", m_parent);

  m_position_tracker.connect(window);

  global_accel_connect_window(window);

  window_connect_focus_in_clear_focus_widget(window);
  

  {
    GtkVBox* vbox = GTK_VBOX(gtk_vbox_new(FALSE, 5));
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5); 
    gtk_widget_show(GTK_WIDGET(vbox));
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
    {
      GtkHBox* hbox = GTK_HBOX(gtk_hbox_new(FALSE, 5));
      gtk_widget_show(GTK_WIDGET(hbox));
      gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE, 0);
      {
        GtkVBox* vbox2 = GTK_VBOX(gtk_vbox_new(FALSE, 0));
        gtk_container_set_border_width(GTK_CONTAINER(vbox2), 0);
        gtk_widget_show(GTK_WIDGET(vbox2));
        gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(vbox2), TRUE, TRUE, 0);
        {
          GtkFrame* frame = GTK_FRAME(gtk_frame_new("Details"));
          gtk_widget_show(GTK_WIDGET(frame));
          gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(frame), TRUE, TRUE, 0);
          {
            GtkVBox* vbox3 = GTK_VBOX(gtk_vbox_new(FALSE, 5));
            gtk_container_set_border_width(GTK_CONTAINER(vbox3), 5);
            gtk_widget_show(GTK_WIDGET(vbox3));
            gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox3));
            {
              GtkTable* table = GTK_TABLE(gtk_table_new(2, 2, FALSE));
              gtk_widget_show(GTK_WIDGET(table));
              gtk_box_pack_start(GTK_BOX(vbox3), GTK_WIDGET(table), TRUE, TRUE, 0);
              gtk_table_set_row_spacings(table, 5);
              gtk_table_set_col_spacings(table, 5);
              {
                GtkLabel* label = GTK_LABEL(gtk_label_new("Row:"));
                gtk_widget_show(GTK_WIDGET(label));
                gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 0, 1,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
              }
              {
                GtkLabel* label = GTK_LABEL(gtk_label_new("Column:"));
                gtk_widget_show(GTK_WIDGET(label));
                gtk_table_attach(table, GTK_WIDGET(label), 1, 2, 0, 1,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
              }
              {
                GtkComboBox* combo = GTK_COMBO_BOX(gtk_combo_box_new_text());
                g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(OnSelchangeComboColRow), this);
                AddDialogData(*combo, m_nRow);

                gtk_widget_show(GTK_WIDGET(combo));
                gtk_table_attach(table, GTK_WIDGET(combo), 0, 1, 1, 2,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
                gtk_widget_set_usize(GTK_WIDGET(combo), 60, -1);
                m_pRowCombo = combo;
              }

              {
                GtkComboBox* combo = GTK_COMBO_BOX(gtk_combo_box_new_text());
                g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(OnSelchangeComboColRow), this);
                AddDialogData(*combo, m_nCol);

                gtk_widget_show(GTK_WIDGET(combo));
                gtk_table_attach(table, GTK_WIDGET(combo), 1, 2, 1, 2,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
                gtk_widget_set_usize(GTK_WIDGET(combo), 60, -1);
                m_pColCombo = combo;
              }
            }
            GtkTable* table = GTK_TABLE(gtk_table_new(5, 2, FALSE));
            gtk_widget_show(GTK_WIDGET(table));
            gtk_box_pack_start(GTK_BOX(vbox3), GTK_WIDGET(table), TRUE, TRUE, 0);
            gtk_table_set_row_spacings(table, 5);
            gtk_table_set_col_spacings(table, 5);
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("X:"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 0, 1,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Y:"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 1, 2,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Z:"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 2, 3,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("S:"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 3, 4,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("T:"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 4, 5,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 0, 1,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              AddDialogData(*entry, m_fX);

              g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 1, 2,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              AddDialogData(*entry, m_fY);

              g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 2, 3,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              AddDialogData(*entry, m_fZ);

              g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 3, 4,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              AddDialogData(*entry, m_fS);

              g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 4, 5,
                                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              AddDialogData(*entry, m_fT);

              g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
            }
          }
        }
        if(g_pGameDescription->mGameType == "doom3")
        {
          GtkFrame* frame = GTK_FRAME(gtk_frame_new("Tesselation"));
          gtk_widget_show(GTK_WIDGET(frame));
          gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(frame), TRUE, TRUE, 0);
          {
            GtkVBox* vbox3 = GTK_VBOX(gtk_vbox_new(FALSE, 5));
            gtk_container_set_border_width(GTK_CONTAINER(vbox3), 5);
            gtk_widget_show(GTK_WIDGET(vbox3));
            gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox3));
            {
              GtkTable* table = GTK_TABLE(gtk_table_new(3, 2, FALSE));
              gtk_widget_show(GTK_WIDGET(table));
              gtk_box_pack_start(GTK_BOX(vbox3), GTK_WIDGET(table), TRUE, TRUE, 0);
              gtk_table_set_row_spacings(table, 5);
              gtk_table_set_col_spacings(table, 5);
              {
                GtkLabel* label = GTK_LABEL(gtk_label_new("Fixed"));
                gtk_widget_show(GTK_WIDGET(label));
                gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 0, 1,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
              }
              {
                GtkCheckButton* check = GTK_CHECK_BUTTON(gtk_check_button_new());
                gtk_widget_show(GTK_WIDGET(check));
                gtk_table_attach(table, GTK_WIDGET(check), 1, 2, 0, 1,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
                m_subdivisions.m_enabled = check;
                guint handler_id = g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(&Subdivisions::applyGtk), &m_subdivisions);
                g_object_set_data(G_OBJECT(check), "handler", gint_to_pointer(handler_id));
              }
              {
                GtkLabel* label = GTK_LABEL(gtk_label_new("Horizontal"));
                gtk_widget_show(GTK_WIDGET(label));
                gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 1, 2,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
              }
              {
                GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
                gtk_widget_show(GTK_WIDGET(entry));
                gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 1, 2,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
                m_subdivisions.m_horizontal = entry;
                m_horizontalSubdivisionsEntry.connect(entry);
              }
              {
                GtkLabel* label = GTK_LABEL(gtk_label_new("Vertical"));
                gtk_widget_show(GTK_WIDGET(label));
                gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 2, 3,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
              }
              {
                GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
                gtk_widget_show(GTK_WIDGET(entry));
                gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 2, 3,
                                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                                  (GtkAttachOptions)(0), 0, 0);
                m_subdivisions.m_vertical = entry;
                m_verticalSubdivisionsEntry.connect(entry);
              }
            }
          }
        }
      }
      {
        GtkFrame* frame = GTK_FRAME(gtk_frame_new("Texturing"));
        gtk_widget_show(GTK_WIDGET(frame));
        gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(frame), TRUE, TRUE, 0);
        {
          GtkVBox* vbox2 = GTK_VBOX(gtk_vbox_new(FALSE, 5));
          gtk_widget_show(GTK_WIDGET(vbox2));
          gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(vbox2));
          gtk_container_set_border_width(GTK_CONTAINER(vbox2), 5);
          {
            GtkLabel* label = GTK_LABEL(gtk_label_new("Name:"));
            gtk_widget_show(GTK_WIDGET(label));
            gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(label), TRUE, TRUE, 0);
            gtk_label_set_justify(label, GTK_JUSTIFY_LEFT);
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
          }
          {
            GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
          //  gtk_entry_set_editable (GTK_ENTRY (entry), false);
            gtk_widget_show(GTK_WIDGET(entry));
            gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(entry), TRUE, TRUE, 0);
            AddDialogData(*entry, m_strName);

            g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(OnDialogKey), 0);
          }
          {
            GtkTable* table = GTK_TABLE(gtk_table_new(5, 3, FALSE));
            gtk_widget_show(GTK_WIDGET(table));
            gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(table), TRUE, TRUE, 0);
            gtk_table_set_row_spacings(table, 5);
            gtk_table_set_col_spacings(table, 5);
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Horizontal Shift Step"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 2, 3, 0, 1,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Vertical Shift Step"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 2, 3, 1, 2,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Horizontal Stretch Step"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 2, 3, 2, 3,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Vertical Stretch Step"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 2, 3, 3, 4,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            }
            {
              GtkLabel* label = GTK_LABEL(gtk_label_new("Rotate Step"));
              gtk_widget_show(GTK_WIDGET(label));
              gtk_table_attach(table, GTK_WIDGET(label), 2, 3, 4, 5,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 0, 1, 0, 1,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(entry), 50, -2);
              g_object_set_data(G_OBJECT(window), "hshift_entry", entry);
              // we fill in this data, if no patch is selected the widgets are unmodified when the inspector is raised
              // so we need to have at least one initialisation somewhere
              entry_set_float(entry, g_pi_globals.shift[0]);

              GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -8192, 8192, 1, 1, 1));
              g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK(OnSpinChanged), entry);
              g_object_set_data(G_OBJECT(window), "hshift_adj", adj);

              GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
              gtk_widget_show(GTK_WIDGET(spin));
              gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 0, 1,
                                (GtkAttachOptions)(0),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(spin), 10, -2);
              GTK_WIDGET_UNSET_FLAGS(spin, GTK_CAN_FOCUS);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 0, 1, 1, 2,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(entry), 50, -2);
              entry_set_float(entry, g_pi_globals.shift[1]);

              GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -8192, 8192, 1, 1, 1));
              g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK(OnSpinChanged), entry);
              g_object_set_data(G_OBJECT(window), "vshift_adj", adj);

              GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
              gtk_widget_show(GTK_WIDGET(spin));
              gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 1, 2,
                                (GtkAttachOptions)(0),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(spin), 10, -2);
              GTK_WIDGET_UNSET_FLAGS(spin, GTK_CAN_FOCUS);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 0, 1, 2, 3,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(entry), 50, -2);
              entry_set_float(entry, g_pi_globals.scale[0]);

              GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -1000, 1000, 1, 1, 1));
              g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK(OnSpinChanged), entry);
              g_object_set_data(G_OBJECT(window), "hscale_adj", adj);

              GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
              gtk_widget_show(GTK_WIDGET(spin));
              gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 2, 3,
                                (GtkAttachOptions)(0),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(spin), 10, -2);
              GTK_WIDGET_UNSET_FLAGS(spin, GTK_CAN_FOCUS);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 0, 1, 3, 4,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(entry), 50, -2);
              entry_set_float(entry, g_pi_globals.scale[1]);

              GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -1000, 1000, 1, 1, 1));
              g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK(OnSpinChanged), entry);
              g_object_set_data(G_OBJECT(window), "vscale_adj", adj);

              GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
              gtk_widget_show(GTK_WIDGET(spin));
              gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 3, 4,
                                (GtkAttachOptions)(0),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(spin), 10, -2);
              GTK_WIDGET_UNSET_FLAGS(spin, GTK_CAN_FOCUS);
            }
            {
              GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
              gtk_widget_show(GTK_WIDGET(entry));
              gtk_table_attach(table, GTK_WIDGET(entry), 0, 1, 4, 5,
                                (GtkAttachOptions)(GTK_FILL),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(entry), 50, -2);
              entry_set_float(entry, g_pi_globals.rotate);

              GtkAdjustment* adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -1000, 1000, 1, 1, 1));	// NOTE: Arnout - this really should be 360 but can't change it anymore as it could break existing maps
              g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK(OnSpinChanged), entry);
              g_object_set_data(G_OBJECT(window), "rotate_adj", adj);

              GtkSpinButton* spin = GTK_SPIN_BUTTON(gtk_spin_button_new(adj, 1, 0));
              gtk_widget_show(GTK_WIDGET(spin));
              gtk_table_attach(table, GTK_WIDGET(spin), 1, 2, 4, 5,
                                (GtkAttachOptions)(0),
                                (GtkAttachOptions)(0), 0, 0);
              gtk_widget_set_usize(GTK_WIDGET(spin), 10, -2);
              GTK_WIDGET_UNSET_FLAGS(spin, GTK_CAN_FOCUS);
            }
          }
          GtkHBox* hbox2 = GTK_HBOX(gtk_hbox_new(TRUE, 5));
          gtk_widget_show(GTK_WIDGET(hbox2));
          gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(hbox2), TRUE, FALSE, 0);
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("CAP"));
            gtk_widget_show(GTK_WIDGET(button));
            gtk_box_pack_end(GTK_BOX(hbox2), GTK_WIDGET(button), TRUE, FALSE, 0);
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(OnBtnPatchdetails), 0);
            gtk_widget_set_usize(GTK_WIDGET(button), 60, -1);
          }
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("Set..."));
            gtk_widget_show(GTK_WIDGET(button));
            gtk_box_pack_end(GTK_BOX(hbox2), GTK_WIDGET(button), TRUE, FALSE, 0);
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(OnBtnPatchreset), 0);
            gtk_widget_set_usize(GTK_WIDGET(button), 60, -1);
          }
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("Natural"));
            gtk_widget_show(GTK_WIDGET(button));
            gtk_box_pack_end(GTK_BOX(hbox2), GTK_WIDGET(button), TRUE, FALSE, 0);
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(OnBtnPatchnatural), 0);
            gtk_widget_set_usize(GTK_WIDGET(button), 60, -1);
          }
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("Fit"));
            gtk_widget_show(GTK_WIDGET(button));
            gtk_box_pack_end(GTK_BOX(hbox2), GTK_WIDGET(button), TRUE, FALSE, 0);
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(OnBtnPatchfit), 0);
            gtk_widget_set_usize(GTK_WIDGET(button), 60, -1);
          }
        }
      }
    }
  }

  return window;
}

// sync the dialog our internal data structures
void PatchInspector::exportData()
{
  m_bListenChanged = false;
  Dialog::exportData();
  m_bListenChanged = true;
}
void PatchInspector::importData()
{
  m_bListenChanged = false;
  Dialog::importData();
  m_bListenChanged = true;
}

// read the map and feed in the stuff to the dialog box
void PatchInspector::GetPatchInfo()
{
  if(g_pGameDescription->mGameType == "doom3")
  {
    m_subdivisions.update();
  }

  if(GlobalSelectionSystem().countSelected() == 0)
  {
    m_Patch = 0;
  }
  else
  {
    m_Patch = Node_getPatch(GlobalSelectionSystem().ultimateSelected().path().top());
  }

  if (m_Patch != 0)
  {
    m_strName = m_Patch->GetShader();
  
    // fill in the numbers for Row / Col selection
    m_bListenChanged = false;

    {
      gtk_combo_box_set_active(m_pRowCombo, 0);

      for (std::size_t i = 0; i < m_countRows; ++i)
      {
        gtk_combo_box_remove_text(m_pRowCombo, gint(m_countRows - i - 1));
      }

      m_countRows = m_Patch->getHeight();
      for (std::size_t i = 0; i < m_countRows; ++i)
      {
        char buffer[16];
        sprintf(buffer, "%u", Unsigned(i));
        gtk_combo_box_append_text(m_pRowCombo, buffer);
      }

      gtk_combo_box_set_active(m_pRowCombo, 0);
    }

    {
      gtk_combo_box_set_active(m_pColCombo, 0);

      for (std::size_t i = 0; i < m_countCols; ++i)
      {
        gtk_combo_box_remove_text(m_pColCombo, gint(m_countCols - i - 1));
      }

      m_countCols = m_Patch->getWidth();
      for (std::size_t i = 0; i < m_countCols; ++i)
      {
        char buffer[16];
        sprintf(buffer, "%u", Unsigned(i));
        gtk_combo_box_append_text(m_pColCombo, buffer);
      }

      gtk_combo_box_set_active(m_pColCombo, 0);
    }
    
    m_bListenChanged = true;
    
  }
  else
  {
    //globalOutputStream() << "WARNING: no patch\n";
  }
  // fill in our internal structs
  m_nRow = 0; m_nCol = 0;
  UpdateRowColInfo();
  // now update the dialog box
  importData();
}

// read the current patch on map and initialize m_fX m_fY accordingly
// NOTE: don't call UpdateData in there, it's not meant for
void PatchInspector::UpdateRowColInfo()
{
  m_fX = m_fY = m_fZ = m_fS = m_fT = 0.0;

  if (m_Patch != 0)
  {
    // we rely on whatever active row/column has been set before we get called
    std::size_t r = m_nRow;
    std::size_t c = m_nCol;
    if(r < m_Patch->getHeight()
      && c < m_Patch->getWidth())
    {
      const PatchControl& p = m_Patch->ctrlAt(r,c);
      m_fX = p.m_vertex[0];
      m_fY = p.m_vertex[1];
      m_fZ = p.m_vertex[2];
      m_fS = p.m_texcoord[0];
      m_fT = p.m_texcoord[1];
    }
  }
}


void PatchInspector_SelectionChanged(const Selectable& selectable)
{
  PatchInspector_queueDraw();
}


#include "preferencesystem.h"


void PatchInspector_Construct()
{
  GlobalCommands_insert("PatchInspector", FreeCaller<PatchInspector_toggleShown>(), Accelerator('S', (GdkModifierType)GDK_SHIFT_MASK));

  GlobalPreferenceSystem().registerPreference("PatchWnd", WindowPositionTrackerImportStringCaller(g_PatchInspector.m_position_tracker), WindowPositionTrackerExportStringCaller(g_PatchInspector.m_position_tracker));
  GlobalPreferenceSystem().registerPreference("SI_PatchTexdef_Scale1", FloatImportStringCaller(g_pi_globals.scale[0]), FloatExportStringCaller(g_pi_globals.scale[0]));      
  GlobalPreferenceSystem().registerPreference("SI_PatchTexdef_Scale2", FloatImportStringCaller(g_pi_globals.scale[1]), FloatExportStringCaller(g_pi_globals.scale[1]));
  GlobalPreferenceSystem().registerPreference("SI_PatchTexdef_Shift1", FloatImportStringCaller(g_pi_globals.shift[0]), FloatExportStringCaller(g_pi_globals.shift[0]));
  GlobalPreferenceSystem().registerPreference("SI_PatchTexdef_Shift2", FloatImportStringCaller(g_pi_globals.shift[1]), FloatExportStringCaller(g_pi_globals.shift[1]));
  GlobalPreferenceSystem().registerPreference("SI_PatchTexdef_Rotate", FloatImportStringCaller(g_pi_globals.rotate), FloatExportStringCaller(g_pi_globals.rotate));

  GlobalSelectionSystem().addSelectionChangeCallback(FreeCaller1<const Selectable&, PatchInspector_SelectionChanged>());
  Patch_addTextureChangedCallback(FreeCaller<PatchInspector_queueDraw>());
}
void PatchInspector_Destroy()
{
}
