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
// Surface Dialog Module
//

//
// Nurail: Implemented to Module from the main Radiant Surface Dialog code
//


#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "surfdlg_plugin.h"



#ifdef _DEBUG
//#define DBG_SI 1
#endif

#include "gtkr_vector.h"

std::vector<texdef_to_face_t> g_texdef_face_vector;

inline texdef_to_face_t* get_texdef_face_list()
{
  return &(*g_texdef_face_vector.begin());
}

inline unsigned int texdef_face_list_empty()
{
  return g_texdef_face_vector.empty();
}

inline unsigned int texdef_face_list_size()
{
  return g_texdef_face_vector.size();
}

// For different faces having different values
bool is_HShift_conflicting;
bool is_VShift_conflicting;
bool is_HScale_conflicting;
bool is_VScale_conflicting;
bool is_Rotate_conflicting;
bool is_TextureName_conflicting;

void ShowDlg();
void HideDlg();
void SetTexMods();
void GetTexMods(bool b_SetUndoPoint = FALSE);
void BuildDialog();
void FitAll();
void InitDefaultIncrement(texdef_t *);
void DoSnapTToGrid(float hscale, float vscale);
// called to perform a fitting from the outside (shortcut key)
void SurfaceDialogFitAll();

// Quake2 Flags Functions
void SetFlagButtons_Quake2(texdef_to_face_t *texdef_face_list,  bool b_isListEmpty);
void SetChangeInFlags_Face_Quake2 (texdef_to_face_t *texdef_face_list);
GtkWidget* Create_Quake2FlagsDialog (GtkWidget* surfacedialog_widget);


// Dialog Data
int m_nHeight;
int m_nWidth;

// 0 is invalid, otherwise it's the Id of the last 'do' we are responsible for
int m_nUndoId;


texturewin_t *texturewin;
texdef_t *l_pIncrement;
texdef_t texdef_offset;
texdef_t texdef_SI_values;

// For Texture Entry, activate only on entry change
char old_texture_entry[128];

// the texdef to switch back to when the OnCancel is called
texdef_t	g_old_texdef;

// when TRUE, this thing means the surface inspector is currently being displayed
bool  g_surfwin = FALSE;
// turn on/off processing of the "changed" "value_changed" messages
// (need to turn off when we are feeding data in)
bool g_bListenChanged = true;
// turn on/off listening of the update messages
bool g_bListenUpdate = true;

GtkWidget* create_SurfaceInspector (void);
GtkWidget *SurfaceInspector = NULL;

GtkWidget *m_pWidget;
GtkWidget *GetWidget () { return SurfaceInspector; }
GtkWidget *Get_SI_Module_Widget () { return SurfaceInspector; }
void SetWidget(GtkWidget *new_widget) { m_pWidget = new_widget; }
GtkWidget *GetDlgWidget (const char* name)
  { return GTK_WIDGET (g_object_get_data (G_OBJECT (SurfaceInspector), name)); }

// Spins for FitTexture
GtkWidget *spin_width;
GtkWidget *spin_height;


GtkWidget *texture_combo;
GtkWidget *texture_combo_entry;

GtkWidget *match_grid_button;
GtkWidget *lock_valuechange_togglebutton;

GtkObject *hshift_value_spinbutton_adj;
GtkWidget *hshift_value_spinbutton;
GtkObject *vshift_value_spinbutton_adj;
GtkWidget *vshift_value_spinbutton;
GtkObject *hscale_value_spinbutton_adj;
GtkWidget *hscale_value_spinbutton;
GtkObject *vscale_value_spinbutton_adj;
GtkWidget *vscale_value_spinbutton;
GtkObject *rotate_value_spinbutton_adj;
GtkWidget *rotate_value_spinbutton;

GtkObject *hshift_offset_spinbutton_adj;
GtkWidget *hshift_offset_spinbutton;
GtkObject *vshift_offset_spinbutton_adj;
GtkWidget *vshift_offset_spinbutton;
GtkObject *hscale_offset_spinbutton_adj;
GtkWidget *hscale_offset_spinbutton;
GtkObject *vscale_offset_spinbutton_adj;
GtkWidget *vscale_offset_spinbutton;
GtkObject *rotate_offset_spinbutton_adj;
GtkWidget *rotate_offset_spinbutton;

GtkObject *hshift_step_spinbutton_adj;
GtkWidget *hshift_step_spinbutton;
GtkObject *vshift_step_spinbutton_adj;
GtkWidget *vshift_step_spinbutton;
GtkObject *hscale_step_spinbutton_adj;
GtkWidget *hscale_step_spinbutton;
GtkObject *vscale_step_spinbutton_adj;
GtkWidget *vscale_step_spinbutton;
GtkObject *rotate_step_spinbutton_adj;
GtkWidget *rotate_step_spinbutton;

GtkObject *fit_width_spinbutton_adj;
GtkWidget *fit_width_spinbutton;
GtkObject *fit_height_spinbutton_adj;
GtkWidget *fit_height_spinbutton;
GtkWidget *fit_button;
GtkWidget *axial_button;

GtkWidget *done_button;
GtkWidget *apply_button;
GtkWidget *cancel_button;

// Callbacks
gboolean on_texture_combo_entry_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_texture_combo_entry_activate (GtkEntry *entry, gpointer user_data);

static void on_match_grid_button_clicked (GtkButton *button, gpointer user_data);
static void on_lock_valuechange_togglebutton_toggled (GtkToggleButton *togglebutton, gpointer user_data);

static void on_hshift_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vshift_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_hscale_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vscale_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_rotate_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);

static void on_hshift_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vshift_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_hscale_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vscale_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_rotate_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);

static void on_hshift_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vshift_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_hscale_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_vscale_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_rotate_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);

static void on_fit_width_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_fit_height_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
static void on_fit_button_clicked (GtkButton *button, gpointer user_data);
static void on_axial_button_clicked (GtkButton *button, gpointer user_data);

static void on_done_button_clicked (GtkButton *button, gpointer user_data);
static void on_apply_button_clicked (GtkButton *button, gpointer user_data);
static void on_cancel_button_clicked (GtkButton *button, gpointer user_data);


/*
===================================================

  SURFACE INSPECTOR

===================================================
*/


void IsFaceConflicting()
{
  texdef_t* tmp_texdef;
  texdef_to_face_t* temp_texdef_face_list;
//  char buf[12];
  char texture_name[128];

  if (texdef_face_list_empty())
  {
    gtk_entry_set_text( GTK_ENTRY (hshift_value_spinbutton), "");
    gtk_entry_set_text( GTK_ENTRY (vshift_value_spinbutton), "");
    gtk_entry_set_text( GTK_ENTRY (hscale_value_spinbutton), "");
    gtk_entry_set_text( GTK_ENTRY (vscale_value_spinbutton), "");
    gtk_entry_set_text( GTK_ENTRY (rotate_value_spinbutton), "");
    gtk_entry_set_text( GTK_ENTRY (texture_combo_entry), "");
    return;
  }

  g_bListenChanged = FALSE;

  tmp_texdef = &get_texdef_face_list()->texdef;

  strcpy(texture_name, tmp_texdef->GetName() );

  texdef_SI_values.shift[0] = tmp_texdef->shift[0];
  texdef_SI_values.shift[1] = tmp_texdef->shift[1];
  texdef_SI_values.scale[0] = tmp_texdef->scale[0];
  texdef_SI_values.scale[1] = tmp_texdef->scale[1];
  texdef_SI_values.rotate = tmp_texdef->rotate;
  texdef_SI_values.SetName( texture_name );

  is_HShift_conflicting = FALSE;
  is_VShift_conflicting = FALSE;
  is_HScale_conflicting = FALSE;
  is_VScale_conflicting = FALSE;
  is_Rotate_conflicting = FALSE;
  is_TextureName_conflicting = FALSE;

  if (texdef_face_list_size() > 1)
  {
    temp_texdef_face_list = get_texdef_face_list()->next;

    for (; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = &temp_texdef_face_list->texdef;
      if ( texdef_SI_values.shift[0] != tmp_texdef->shift[0] )
        is_HShift_conflicting = TRUE;

      if ( texdef_SI_values.shift[1] != tmp_texdef->shift[1] )
        is_VShift_conflicting = TRUE;

      if ( texdef_SI_values.scale[0] != tmp_texdef->scale[0] )
        is_HScale_conflicting = TRUE;

      if ( texdef_SI_values.scale[1] != tmp_texdef->scale[1] )
        is_VScale_conflicting = TRUE;

      if ( texdef_SI_values.rotate != tmp_texdef->rotate )
        is_Rotate_conflicting = TRUE;

      if ( strcmp( texture_name, tmp_texdef->GetName() ) )
        is_TextureName_conflicting = TRUE;
    }
  }

  if(is_HShift_conflicting)
    gtk_entry_set_text( GTK_ENTRY (hshift_value_spinbutton), "");
  else
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(hshift_value_spinbutton) , texdef_SI_values.shift[0] );

  if(is_VShift_conflicting)
    gtk_entry_set_text( GTK_ENTRY (vshift_value_spinbutton), "");
  else
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(vshift_value_spinbutton) , texdef_SI_values.shift[1] );

  if(is_HScale_conflicting)
    gtk_entry_set_text( GTK_ENTRY (hscale_value_spinbutton), "");
  else
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(hscale_value_spinbutton) , texdef_SI_values.scale[0] );

  if(is_VScale_conflicting)
    gtk_entry_set_text( GTK_ENTRY (vscale_value_spinbutton), "");
  else
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(vscale_value_spinbutton) , texdef_SI_values.scale[1] );

  if(is_Rotate_conflicting)
    gtk_entry_set_text( GTK_ENTRY (rotate_value_spinbutton), "");
  else
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(rotate_value_spinbutton) , texdef_SI_values.rotate );

  g_bListenChanged = TRUE;
}

#define MAX_NUM_LIST_ITEMS 15
static void PopulateTextureComboList()
{
  texdef_t* tmp_texdef;
  texdef_to_face_t* temp_texdef_face_list;
  char blank[1];
  GList *items = NULL;
//  GList *tmp_item;
  int num_of_list_items = 0;

  blank[0] = 0;

  if (texdef_face_list_empty())
  {
    items = g_list_append (items, (gpointer) blank);
    // For Texture Entry, activate only on entry change
    strcpy (old_texture_entry, blank);
  }
  else if ( !is_TextureName_conflicting )
  {
    temp_texdef_face_list = get_texdef_face_list();
    tmp_texdef = (texdef_t *) &get_texdef_face_list()->texdef;
    items = g_list_append( items, (gpointer) tmp_texdef->GetName() );
    // For Texture Entry, activate only on entry change
    strcpy (old_texture_entry, tmp_texdef->GetName());
  }
  else
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      // Need to do a string compare, hence the custom search
      if (!( g_list_find_custom (items, tmp_texdef->GetName(), (GCompareFunc) strcmp ) ))
      {
        items = g_list_append( items, (gpointer) tmp_texdef->GetName() );
	num_of_list_items++;
      }
      // Make sure the combo list isn't too long
      if (num_of_list_items >= MAX_NUM_LIST_ITEMS)
        break;
    }
    // If this isn't added last (to the top of the list), g_list_find freaks.
    items = g_list_prepend (items, (gpointer) blank);
    // For Texture Entry, activate only on entry change
    strcpy (old_texture_entry, blank);
  }

  gtk_combo_set_popdown_strings (GTK_COMBO (texture_combo), items);
  g_list_free(items);

}

static void ZeroOffsetValues()
{
  texdef_offset.shift[0] = 0.0;
  texdef_offset.shift[1] = 0.0;
  texdef_offset.scale[0] = 0.0;
  texdef_offset.scale[1] = 0.0;
  texdef_offset.rotate = 0.0;
}

static void GetTexdefInfo_from_Radiant()
{
  g_texdef_face_vector.clear();

  unsigned int count = GetSelectedFaceCountfromBrushes();
  if(count == 0)
    count = GetSelectedFaceCount();

  g_texdef_face_vector.resize(count);

  if (!texdef_face_list_empty())
  {
//    texdef_to_face_t* p = get_texdef_face_list();
    GetSelFacesTexdef( get_texdef_face_list() );
  }

  IsFaceConflicting();
  PopulateTextureComboList();
  ZeroOffsetValues();
  if ( texdef_face_list_empty() )
    SetFlagButtons_Quake2( get_texdef_face_list() , TRUE);
  else
    SetFlagButtons_Quake2( get_texdef_face_list() , FALSE);
}

static gint delete_event_callback(GtkWidget *widget, GdkEvent* event, gpointer data)
{
  HideDlg();
  return TRUE;
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
void DoSnapTToGrid(float hscale, float vscale)
{
  l_pIncrement = Get_SI_Inc();

  if (hscale == 0.0f)
  {
    hscale = 0.5f;
  }
  if (vscale == 0.0f)
  {
     vscale = 0.5f;
  }
#ifdef _DEBUG
  Sys_Printf ("DoSnapTToGrid: hscale %g vscale %g\n", hscale, vscale);
#endif
  l_pIncrement->shift[0] = GridSize() / hscale;
  l_pIncrement->shift[1] = GridSize() / vscale;
  // now some update work
  // FIXME: doesn't look good here, seems to be called several times
  SetTexMods();
}

void UpdateSurfaceDialog()
{
  if (!g_bListenUpdate)
    return;

  if (!SurfaceInspector)
    return;

  // avoid long delays on slow computers
  while (gtk_events_pending ())
    gtk_main_iteration ();

  if (g_surfwin)
  {
#ifdef DBG_SI
    Sys_Printf("UpdateSurfaceDialog\n");
#endif
    GetTexdefInfo_from_Radiant();
    SetTexMods();
  }

}

// DoSurface will always try to show the surface inspector
// or update it because something new has been selected
void DoSurface (void)
{
#ifdef DBG_SI
  Sys_Printf("DoSurface\n");
#endif
  if (!SurfaceInspector)
    create_SurfaceInspector ();

  ShowDlg();
  SetTexMods ();
}

void ToggleSurface()
{
#ifdef DBG_SI
  Sys_Printf("ToggleSurface Module\n");
#endif
  if (!g_surfwin)
    DoSurface ();
  else
    on_cancel_button_clicked(NULL, NULL);
}

// NOTE: will raise and show the Surface inspector and exec fit for patches and brushes
void SurfaceDlgFitAll()
{
  DoSurface();
  FitAll();
}

// =============================================================================
// SurfaceDialog class

void ShowDlg()
{

  if(!SurfaceInspector)
    create_SurfaceInspector();
  else
    gtk_widget_show (SurfaceInspector);

  GetTexdefInfo_from_Radiant();
  GetTexMods(TRUE); // Set Initial Undo Point
  g_surfwin = TRUE;
}

void HideDlg()
{
  g_surfwin = FALSE;
  gtk_widget_hide (SurfaceInspector);
}


// set default values for increments (shift scale and rot)
// this is called by the prefs code if can't find the values
void InitDefaultIncrement(texdef_t *tex)
{
  tex->SetName("foo");
  tex->shift[0] = 8;
  tex->shift[1] = 8;
  tex->scale[0] = 0.25;
  tex->scale[1] = 0.25;
  tex->rotate = 10;
}

void BuildDialog ()
{
   if ( !SurfaceInspector )
    create_SurfaceInspector();
}

/*
==============
SetTexMods

Set the fields to the current texdef (i.e. map/texdef -> dialog widgets)
===============
*/

void SetTexMods()
{
  texdef_t *pt;
  GtkSpinButton *spin;
  GtkAdjustment *adjust;

  texturewin = Texturewin ();
  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("SurfaceDlg SetTexMods\n");
#endif

  if (!g_surfwin)
    return;

  pt = &texturewin->texdef;

  g_bListenChanged = false;

  if(strncmp(pt->GetName(), "textures/", 9) != 0)
    texdef_offset.SetName(SHADER_NOT_FOUND);


  spin = GTK_SPIN_BUTTON (hshift_offset_spinbutton);
  gtk_spin_button_set_value (spin, texdef_offset.shift[0]);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->shift[0];
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(hshift_step_spinbutton), l_pIncrement->shift[0]);

  spin = GTK_SPIN_BUTTON (hshift_value_spinbutton);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->shift[0];


  spin = GTK_SPIN_BUTTON (vshift_offset_spinbutton);
  gtk_spin_button_set_value (spin, texdef_offset.shift[1]);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->shift[1];
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(vshift_step_spinbutton), l_pIncrement->shift[1]);

  spin = GTK_SPIN_BUTTON (vshift_value_spinbutton);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->shift[1];


  spin = GTK_SPIN_BUTTON (hscale_offset_spinbutton);
  gtk_spin_button_set_value (spin, texdef_offset.scale[0]);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->scale[0];
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(hscale_step_spinbutton), l_pIncrement->scale[0]);

  spin = GTK_SPIN_BUTTON (hscale_value_spinbutton);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->scale[0];


  spin = GTK_SPIN_BUTTON (vscale_offset_spinbutton);
  gtk_spin_button_set_value (spin, texdef_offset.scale[1]);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->scale[1];
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(vscale_step_spinbutton), l_pIncrement->scale[1]);

  spin = GTK_SPIN_BUTTON (vscale_value_spinbutton);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->scale[1];


  spin = GTK_SPIN_BUTTON (rotate_offset_spinbutton);
  gtk_spin_button_set_value (spin, texdef_offset.rotate);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->rotate;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(rotate_step_spinbutton), l_pIncrement->rotate);

  spin = GTK_SPIN_BUTTON (rotate_value_spinbutton);
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
  adjust->step_increment = l_pIncrement->rotate;


  g_bListenChanged = true;

  // store the current texdef as our escape route if user hits OnCancel
  g_old_texdef = texturewin->texdef;
}

/*
==============
GetTexMods

Shows any changes to the main Radiant windows
===============
*/
void GetTexMods(bool b_SetUndoPoint)
{

#ifdef DBG_SI
  Sys_Printf("SurfaceDlg GetTexMods\n");
#endif

  if ( !texdef_face_list_empty() )
  {
    g_bListenUpdate=FALSE;
    SetChangeInFlags_Face_Quake2 ( get_texdef_face_list() );
    SetTexdef_FaceList( get_texdef_face_list(), b_SetUndoPoint, false );
    g_bListenUpdate=TRUE;

    if (b_SetUndoPoint)
      m_nUndoId = Undo_GetUndoId();
  }
}

void FitAll()
{
  on_fit_button_clicked(NULL, NULL);
}


////////////////////////////////////////////////////////////////////
//
//  GUI Section
//
////////////////////////////////////////////////////////////////////

GtkWidget* create_SurfaceInspector (void)
{

  GtkWidget *label;
  GtkWidget *hseparator;
  GtkWidget *eventbox;

  GtkWidget *viewport8;
  GtkWidget *viewport9;
  GtkWidget *viewport2;
  GtkWidget *viewport7;
  GtkWidget *viewport5;
  GtkWidget *viewport6;
  GtkWidget *viewport10;

  GtkWidget *table1;
  GtkWidget *table4;
  GtkWidget *table5;
  GtkWidget *table7;

  GtkWidget *alignment1;
  GtkWidget *alignment2;
  GtkWidget *alignment3;

  GtkWidget *vbox7;

  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *hbox3;
  GtkWidget *hbox4;

  GtkWidget *image1;
  GtkWidget *image2;
  GtkWidget *image3;

  GtkWidget *hbuttonbox1;

  SurfaceInspector = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (SurfaceInspector), 4);
  gtk_window_set_title (GTK_WINDOW (SurfaceInspector), "Surface Inspector");

  SetWinPos_from_Prefs(SurfaceInspector);

  viewport8 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport8);
  gtk_container_add (GTK_CONTAINER (SurfaceInspector), viewport8);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport8), GTK_SHADOW_NONE);

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (viewport8), vbox7);

  viewport9 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport9);
  gtk_box_pack_start (GTK_BOX (vbox7), viewport9, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport9), 2);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport9), GTK_SHADOW_ETCHED_IN);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (viewport9), hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 4);

  label = gtk_label_new ("Texture: ");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  texture_combo = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (texture_combo)->popwin),
                     "KeepMeAround", texture_combo);
  gtk_combo_disable_activate ( (GtkCombo*) texture_combo);
  gtk_widget_show (texture_combo);
  gtk_box_pack_start (GTK_BOX (hbox1), texture_combo, TRUE, TRUE, 0);

  texture_combo_entry = GTK_COMBO (texture_combo)->entry;
  gtk_widget_show (texture_combo_entry);
  gtk_entry_set_max_length (GTK_ENTRY (texture_combo_entry), 128);

  viewport2 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport2);
  gtk_box_pack_start (GTK_BOX (vbox7), viewport2, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport2), 2);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport2), GTK_SHADOW_ETCHED_IN);

  table1 = gtk_table_new (13, 4, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (viewport2), table1);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 1, 2, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 2, 3, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 3, 4, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new ("Offset");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label = gtk_label_new ("Step");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 3, 4, 12, 13,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  match_grid_button = gtk_button_new_with_mnemonic ("Match Grid");
  gtk_widget_show (match_grid_button);
  gtk_container_add (GTK_CONTAINER (eventbox), match_grid_button);

  label = gtk_label_new ("Value");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 1);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new ("V Shift: ");
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (eventbox), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new (" H Scale: ");
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (eventbox), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new ("V Scale: ");
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (eventbox), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new ("Rotate: ");
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (eventbox), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label = gtk_label_new ("H Shift: ");
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (eventbox), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 1, 2, 12, 13,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  lock_valuechange_togglebutton = gtk_toggle_button_new_with_mnemonic ("UNLOCK");
  gtk_widget_show (lock_valuechange_togglebutton);
  gtk_container_add (GTK_CONTAINER (eventbox), lock_valuechange_togglebutton);

  // Value Spins
  hshift_value_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  hshift_value_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hshift_value_spinbutton_adj), 1, 2);
  gtk_widget_show (hshift_value_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hshift_value_spinbutton, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hshift_value_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (hshift_value_spinbutton), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET( hshift_value_spinbutton ), FALSE );

  vshift_value_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  vshift_value_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vshift_value_spinbutton_adj), 1, 2);
  gtk_widget_show (vshift_value_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vshift_value_spinbutton, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vshift_value_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (vshift_value_spinbutton), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET( vshift_value_spinbutton ), FALSE );

  hscale_value_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  hscale_value_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hscale_value_spinbutton_adj), 1, 4);
  gtk_widget_show (hscale_value_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hscale_value_spinbutton, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hscale_value_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (hscale_value_spinbutton), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET( hscale_value_spinbutton ), FALSE );

  vscale_value_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  vscale_value_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vscale_value_spinbutton_adj), 1, 4);
  gtk_widget_show (vscale_value_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vscale_value_spinbutton, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vscale_value_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (vscale_value_spinbutton), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET( vscale_value_spinbutton ), FALSE );

  rotate_value_spinbutton_adj = gtk_adjustment_new (0.0, -360.0, 360.0, 1.0, 10.0, 10.0);
  rotate_value_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rotate_value_spinbutton_adj), 1, 0);
  gtk_widget_show (rotate_value_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), rotate_value_spinbutton, 1, 2, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (rotate_value_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (rotate_value_spinbutton), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET( rotate_value_spinbutton ), FALSE );

  // Offset Spins
  hshift_offset_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  hshift_offset_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hshift_offset_spinbutton_adj), 0, 2);
  gtk_widget_show (hshift_offset_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hshift_offset_spinbutton, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (hshift_offset_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hshift_offset_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (hshift_offset_spinbutton), TRUE);

  vshift_offset_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  vshift_offset_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vshift_offset_spinbutton_adj), 0, 2);
  gtk_widget_show (vshift_offset_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vshift_offset_spinbutton, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (vshift_offset_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vshift_offset_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (vshift_offset_spinbutton), TRUE);

  hscale_offset_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  hscale_offset_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hscale_offset_spinbutton_adj), 0, 4);
  gtk_widget_show (hscale_offset_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hscale_offset_spinbutton, 2, 3, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (hscale_offset_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hscale_offset_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (hscale_offset_spinbutton), TRUE);

  vscale_offset_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  vscale_offset_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vscale_offset_spinbutton_adj), 0, 4);
  gtk_widget_show (vscale_offset_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vscale_offset_spinbutton, 2, 3, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (vscale_offset_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vscale_offset_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (vscale_offset_spinbutton), TRUE);

  rotate_offset_spinbutton_adj = gtk_adjustment_new (0.0, -360.0, 360.0, 1.0, 10.0, 10.0);
  rotate_offset_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rotate_offset_spinbutton_adj), 0, 2);
  gtk_widget_show (rotate_offset_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), rotate_offset_spinbutton, 2, 3, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rotate_offset_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (rotate_offset_spinbutton), GTK_UPDATE_IF_VALID);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (rotate_offset_spinbutton), TRUE);

    // Step Spins
  hshift_step_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  hshift_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hshift_step_spinbutton_adj), 1, 2);
  gtk_widget_show (hshift_step_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hshift_step_spinbutton, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hshift_step_spinbutton), GTK_UPDATE_IF_VALID);

  vshift_step_spinbutton_adj = gtk_adjustment_new (0.0, -8192.0, 8192.0, 2.0, 8.0, 8.0);
  vshift_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vshift_step_spinbutton_adj), 1, 2);
  gtk_widget_show (vshift_step_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vshift_step_spinbutton, 3, 4, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vshift_step_spinbutton), GTK_UPDATE_IF_VALID);

  hscale_step_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  hscale_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (hscale_step_spinbutton_adj), 1, 4);
  gtk_widget_show (hscale_step_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), hscale_step_spinbutton, 3, 4, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (hscale_step_spinbutton), GTK_UPDATE_IF_VALID);

  vscale_step_spinbutton_adj = gtk_adjustment_new (0.0, -1024.0, 1024.0, 1.0, 4.0, 4.0);
  vscale_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (vscale_step_spinbutton_adj), 1, 4);
  gtk_widget_show (vscale_step_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), vscale_step_spinbutton, 3, 4, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (vscale_step_spinbutton), GTK_UPDATE_IF_VALID);

  rotate_step_spinbutton_adj = gtk_adjustment_new (0.0, -360.0, 360.0, 1.0, 10.0, 10.0);
  rotate_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rotate_step_spinbutton_adj), 1, 2);
  gtk_widget_show (rotate_step_spinbutton);
  gtk_table_attach (GTK_TABLE (table1), rotate_step_spinbutton, 3, 4, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (rotate_step_spinbutton), GTK_UPDATE_IF_VALID);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 2, 3, 12, 13,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 12, 13,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table1), eventbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  viewport7 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport7);
  gtk_box_pack_start (GTK_BOX (vbox7), viewport7, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport7), 2);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport7), GTK_SHADOW_ETCHED_IN);

  table4 = gtk_table_new (4, 7, FALSE);
  gtk_widget_show (table4);
  gtk_container_add (GTK_CONTAINER (viewport7), table4);

  viewport5 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport5);
  gtk_table_attach (GTK_TABLE (table4), viewport5, 1, 7, 0, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport5), 6);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport5), GTK_SHADOW_ETCHED_OUT);

  table5 = gtk_table_new (2, 3, FALSE);
  gtk_widget_show (table5);
  gtk_container_add (GTK_CONTAINER (viewport5), table5);
  gtk_container_set_border_width (GTK_CONTAINER (table5), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table5), 2);

  label = gtk_label_new ("Height");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table5), label, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 1);

  label = gtk_label_new ("Width");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table5), label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 1);

  fit_width_spinbutton_adj = gtk_adjustment_new (1, 1, 32, 1, 10, 10);
  fit_width_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (fit_width_spinbutton_adj), 1, 0);
  gtk_widget_show (fit_width_spinbutton);
  gtk_table_attach (GTK_TABLE (table5), fit_width_spinbutton, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (fit_width_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (fit_width_spinbutton), GTK_UPDATE_IF_VALID);

  fit_height_spinbutton_adj = gtk_adjustment_new (1, 1, 32, 1, 10, 10);
  fit_height_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (fit_height_spinbutton_adj), 1, 0);
  gtk_widget_show (fit_height_spinbutton);
  gtk_table_attach (GTK_TABLE (table5), fit_height_spinbutton, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 3, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (fit_height_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (fit_height_spinbutton), GTK_UPDATE_IF_VALID);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table5), eventbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table5), eventbox, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 4, 0);

  fit_button = gtk_button_new_with_mnemonic ("    Fit    ");
  gtk_widget_show (fit_button);
  gtk_container_add (GTK_CONTAINER (eventbox), fit_button);

  viewport6 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport6);
  gtk_table_attach (GTK_TABLE (table4), viewport6, 0, 1, 0, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport6), 4);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport6), GTK_SHADOW_NONE);

  table7 = gtk_table_new (2, 1, FALSE);
  gtk_widget_show (table7);
  gtk_container_add (GTK_CONTAINER (viewport6), table7);

  eventbox = gtk_event_box_new ();
  gtk_widget_show (eventbox);
  gtk_table_attach (GTK_TABLE (table7), eventbox, 0, 1, 0, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  axial_button = gtk_button_new_with_mnemonic ("Axial");
  gtk_widget_show (axial_button);
  gtk_container_add (GTK_CONTAINER (eventbox), axial_button);
  gtk_widget_set_size_request (axial_button, 56, 29);
  gtk_container_set_border_width (GTK_CONTAINER (axial_button), 4);

  // Fit in Flags sub-dialog
  Create_Quake2FlagsDialog(vbox7);

  viewport10 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport10);
  gtk_box_pack_start (GTK_BOX (vbox7), viewport10, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (viewport10), 2);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport10), GTK_SHADOW_ETCHED_IN);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_container_add (GTK_CONTAINER (viewport10), hbuttonbox1);
  gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox1), 4);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);

  done_button = gtk_button_new ();
  gtk_widget_show (done_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), done_button);
  GTK_WIDGET_SET_FLAGS (done_button, GTK_CAN_DEFAULT);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (done_button), alignment1);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox2);

  image1 = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox2), image1, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic ("Done");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  apply_button = gtk_button_new ();
  gtk_widget_show (apply_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), apply_button);
  GTK_WIDGET_SET_FLAGS (apply_button, GTK_CAN_DEFAULT);

  alignment3 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (apply_button), alignment3);

  hbox4 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox4);
  gtk_container_add (GTK_CONTAINER (alignment3), hbox4);

  image3 = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image3);
  gtk_box_pack_start (GTK_BOX (hbox4), image3, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic ("Apply");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox4), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  cancel_button = gtk_button_new ();
  gtk_widget_show (cancel_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_button);
  GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (cancel_button), alignment2);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox3);

  image2 = gtk_image_new_from_stock ("gtk-no", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox3), image2, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic ("Cancel");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);


  g_signal_connect ( (gpointer) SurfaceInspector,
                      "delete_event",
                      G_CALLBACK (delete_event_callback),
		      NULL );
  g_signal_connect ((gpointer) SurfaceInspector, "destroy",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  g_signal_connect ((gpointer) texture_combo_entry, "key_press_event",
                    G_CALLBACK (on_texture_combo_entry_key_press_event),
                    NULL);
  g_signal_connect ((gpointer) texture_combo_entry, "activate",
                    G_CALLBACK (on_texture_combo_entry_activate),
                    NULL);


  g_signal_connect ((gpointer) hshift_offset_spinbutton, "value_changed",
                    G_CALLBACK (on_hshift_offset_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vshift_offset_spinbutton, "value_changed",
                    G_CALLBACK (on_vshift_offset_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) hscale_offset_spinbutton, "value_changed",
                    G_CALLBACK (on_hscale_offset_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vscale_offset_spinbutton, "value_changed",
                    G_CALLBACK (on_vscale_offset_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) rotate_offset_spinbutton, "value_changed",
                    G_CALLBACK (on_rotate_offset_spinbutton_value_changed),
                    NULL);

  g_signal_connect ((gpointer) hshift_value_spinbutton, "value_changed",
                    G_CALLBACK (on_hshift_value_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vshift_value_spinbutton, "value_changed",
                    G_CALLBACK (on_vshift_value_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) hscale_value_spinbutton, "value_changed",
                    G_CALLBACK (on_hscale_value_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vscale_value_spinbutton, "value_changed",
                    G_CALLBACK (on_vscale_value_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) rotate_value_spinbutton, "value_changed",
                    G_CALLBACK (on_rotate_value_spinbutton_value_changed),
                    NULL);

  g_signal_connect ((gpointer) hshift_step_spinbutton, "value_changed",
                    G_CALLBACK (on_hshift_step_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vshift_step_spinbutton, "value_changed",
                    G_CALLBACK (on_vshift_step_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) hscale_step_spinbutton, "value_changed",
                    G_CALLBACK (on_hscale_step_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) vscale_step_spinbutton, "value_changed",
                    G_CALLBACK (on_vscale_step_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) rotate_step_spinbutton, "value_changed",
                    G_CALLBACK (on_rotate_step_spinbutton_value_changed),
                    NULL);

  g_signal_connect ((gpointer) match_grid_button, "clicked",
                    G_CALLBACK (on_match_grid_button_clicked),
                    NULL);
  g_signal_connect ((gpointer) lock_valuechange_togglebutton, "toggled",
                    G_CALLBACK (on_lock_valuechange_togglebutton_toggled),
                    NULL);

  g_signal_connect ((gpointer) fit_width_spinbutton, "value_changed",
                    G_CALLBACK (on_fit_width_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) fit_height_spinbutton, "value_changed",
                    G_CALLBACK (on_fit_height_spinbutton_value_changed),
                    NULL);
  g_signal_connect ((gpointer) fit_button, "clicked",
                    G_CALLBACK (on_fit_button_clicked),
                    NULL);

  g_signal_connect ((gpointer) axial_button, "clicked",
                    G_CALLBACK (on_axial_button_clicked),
                    NULL);

  g_signal_connect ((gpointer) done_button, "clicked",
                    G_CALLBACK (on_done_button_clicked),
                    NULL);
  g_signal_connect ((gpointer) apply_button, "clicked",
                    G_CALLBACK (on_apply_button_clicked),
                    NULL);
  g_signal_connect ((gpointer) cancel_button, "clicked",
                    G_CALLBACK (on_cancel_button_clicked),
                    NULL);


  return SurfaceInspector;
}


// Texture Combo
gboolean on_texture_combo_entry_key_press_event (GtkWidget *widget, GdkEventKey *event,
                                                 gpointer user_data)
{
  // Have Tab activate selection as well as Return
  if (event->keyval == GDK_Tab)
    g_signal_emit_by_name ( texture_combo_entry, "activate" );

  return FALSE;
}

void on_texture_combo_entry_activate (GtkEntry *entry, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;
  char text[128] = { 0 };

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    // activate only on entry change
    strcpy( text, gtk_entry_get_text(entry));
    if ( strcmp( old_texture_entry, text ))
    {
      // Check for spaces in shader name
      if (text[0] <= ' ' || strchr(text, ' '))
        Sys_FPrintf(SYS_WRN, "WARNING: spaces in shader names are not allowed, ignoring '%s'\n", text);
      else
      {
        for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
        {
          tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
          tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
          strcpy( old_texture_entry, text );
          tmp_texdef->SetName( text );
        }
        GetTexMods();
      }
    }
  }
}

// Offset Spins
static void on_hshift_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_offset.shift[0] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hshift_offset_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      if (is_HShift_conflicting)
        tmp_texdef->shift[0] = tmp_orig_texdef->shift[0] + texdef_offset.shift[0];
      else
        tmp_texdef->shift[0] = texdef_SI_values.shift[0] + texdef_offset.shift[0];
    }
    GetTexMods();
  }
}

static void on_vshift_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_offset.shift[1] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vshift_offset_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      if (is_VShift_conflicting)
        tmp_texdef->shift[1] = tmp_orig_texdef->shift[1] + texdef_offset.shift[1];
      else
        tmp_texdef->shift[1] = texdef_SI_values.shift[1] + texdef_offset.shift[1];
    }
    GetTexMods();
  }

}

static void on_hscale_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_offset.scale[0] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hscale_offset_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      if (is_HScale_conflicting)
        tmp_texdef->scale[0] = tmp_orig_texdef->scale[0] + texdef_offset.scale[0];
      else
        tmp_texdef->scale[0] = texdef_SI_values.scale[0] + texdef_offset.scale[0];
    }
    GetTexMods();
  }


}

static void on_vscale_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_offset.scale[1] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vscale_offset_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      if (is_VScale_conflicting)
        tmp_texdef->scale[1] = tmp_orig_texdef->scale[1] + texdef_offset.scale[1];
      else
        tmp_texdef->scale[1] = texdef_SI_values.scale[1] + texdef_offset.scale[1];
    }
    GetTexMods();
  }

}

static void on_rotate_offset_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_offset.rotate = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(rotate_offset_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      if (is_Rotate_conflicting)
        tmp_texdef->rotate = tmp_orig_texdef->rotate + texdef_offset.rotate;
      else
        tmp_texdef->rotate = texdef_SI_values.rotate + texdef_offset.rotate;
    }
    GetTexMods();
  }

}


// Match Grid
static void on_match_grid_button_clicked (GtkButton *button, gpointer user_data)
{
  float hscale, vscale;

  if( !strcmp(gtk_entry_get_text (GTK_ENTRY (hscale_value_spinbutton)), "") )
    hscale = 0.0;
  else
    hscale = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hscale_value_spinbutton) );

  if( !strcmp( gtk_entry_get_text (GTK_ENTRY (vscale_value_spinbutton)), "") )
    vscale = 0.0;
  else
    vscale = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vscale_value_spinbutton) );
  DoSnapTToGrid (hscale, vscale);
}


// Lock out changes to Value
static void on_lock_valuechange_togglebutton_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
  bool is_Locked;

  is_Locked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lock_valuechange_togglebutton));

  gtk_widget_set_sensitive( GTK_WIDGET( hscale_value_spinbutton ), is_Locked );
  gtk_widget_set_sensitive( GTK_WIDGET( vscale_value_spinbutton ), is_Locked );
  gtk_widget_set_sensitive( GTK_WIDGET( hshift_value_spinbutton ), is_Locked );
  gtk_widget_set_sensitive( GTK_WIDGET( vshift_value_spinbutton ), is_Locked );
  gtk_widget_set_sensitive( GTK_WIDGET( rotate_value_spinbutton ), is_Locked );
}


// Value Spins
static void on_hshift_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_SI_values.shift[0] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hshift_value_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      tmp_texdef->shift[0] = texdef_SI_values.shift[0] + texdef_offset.shift[0];
      is_HShift_conflicting = FALSE;
    }
    GetTexMods();
  }
}

static void on_vshift_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_SI_values.shift[1]  = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vshift_value_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      tmp_texdef->shift[1] = texdef_SI_values.shift[1] + texdef_offset.shift[1];
      is_VShift_conflicting = FALSE;
    }
    GetTexMods();
  }
}

static void on_hscale_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_SI_values.scale[0] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hscale_value_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      tmp_texdef->scale[0] = texdef_SI_values.scale[0] + texdef_offset.scale[0];
      is_HScale_conflicting = FALSE;
    }
    GetTexMods();
  }
}

static void on_vscale_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_SI_values.scale[1] = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vscale_value_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      tmp_texdef->scale[1] = texdef_SI_values.scale[1] + texdef_offset.scale[1];
      is_VScale_conflicting = FALSE;
    }
    GetTexMods();
  }
}

static void on_rotate_value_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  texdef_t* tmp_texdef;
  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  texdef_SI_values.rotate = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(rotate_value_spinbutton) );

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_orig_texdef = (texdef_t *) &temp_texdef_face_list->orig_texdef;
      tmp_texdef->rotate = texdef_SI_values.rotate + texdef_offset.rotate;
      is_Rotate_conflicting = FALSE;
    }
    GetTexMods();
  }
}


// Step Spins
static void on_hshift_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  gfloat val;
  GtkAdjustment * adjust;

  if (!g_bListenChanged)
    return;

  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("OnIncrementChanged HShift\n");
#endif

  val = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hshift_step_spinbutton) ) ;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( hshift_offset_spinbutton ));
  adjust->step_increment = val;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( hshift_value_spinbutton ));
  adjust->step_increment = val;
  l_pIncrement->shift[0] = val;
}

static void on_vshift_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  gfloat val;
  GtkAdjustment * adjust;

  if (!g_bListenChanged)
    return;

  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("OnIncrementChanged VShift\n");
#endif

  val = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vshift_step_spinbutton) ) ;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( vshift_offset_spinbutton ));
  adjust->step_increment = val;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( vshift_value_spinbutton ));
  adjust->step_increment = val;
  l_pIncrement->shift[1] = val;
}

static void on_hscale_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  gfloat val;
  GtkAdjustment * adjust;

  if (!g_bListenChanged)
    return;

  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("OnIncrementChanged HShift\n");
#endif

  val = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(hscale_step_spinbutton) ) ;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( hscale_offset_spinbutton ));
  adjust->step_increment = val;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( hscale_value_spinbutton ));
  adjust->step_increment = val;
  l_pIncrement->scale[0] = val;
}

static void on_vscale_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  gfloat val;
  GtkAdjustment * adjust;

  if (!g_bListenChanged)
    return;

  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("OnIncrementChanged HShift\n");
#endif

  val = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(vscale_step_spinbutton) ) ;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( vscale_offset_spinbutton ));
  adjust->step_increment = val;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( vscale_value_spinbutton ));
  adjust->step_increment = val;
  l_pIncrement->scale[1] = val;
}

static void on_rotate_step_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  gfloat val;
  GtkAdjustment * adjust;

  if (!g_bListenChanged)
    return;

  l_pIncrement = Get_SI_Inc();

#ifdef DBG_SI
  Sys_Printf("OnIncrementChanged HShift\n");
#endif

  val = gtk_spin_button_get_value ( GTK_SPIN_BUTTON(rotate_step_spinbutton) ) ;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( rotate_offset_spinbutton ));
  adjust->step_increment = val;
  adjust = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON ( rotate_value_spinbutton ));
  adjust->step_increment = val;
  l_pIncrement->rotate = val;
}


// Fit Texture
static void on_fit_width_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  m_nWidth = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(fit_width_spinbutton) );
}

static void on_fit_height_spinbutton_value_changed (GtkSpinButton *spinbutton, gpointer user_data)
{
  m_nHeight = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(fit_height_spinbutton) );
}

static void on_fit_button_clicked (GtkButton *button, gpointer user_data)
{
  FaceList_FitTexture(get_texdef_face_list(), m_nHeight, m_nWidth);
  Sys_UpdateWindows(W_ALL);
}


// Axial Button
static void on_axial_button_clicked (GtkButton *button, gpointer user_data)
{
  texdef_t* tmp_texdef;
//  texdef_t* tmp_orig_texdef;
  texdef_to_face_t* temp_texdef_face_list;

  if (!texdef_face_list_empty() && g_bListenChanged)
  {
    for (temp_texdef_face_list = get_texdef_face_list(); temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = (texdef_t *) &temp_texdef_face_list->texdef;
      tmp_texdef->shift[0] = 0.0;
      tmp_texdef->shift[1] = 0.0;
      tmp_texdef->scale[0] = 0.5;
      tmp_texdef->scale[1] = 0.5;
      tmp_texdef->rotate = 0.0;
    }
  }

  SetTexdef_FaceList( get_texdef_face_list(), FALSE, TRUE );
  Sys_UpdateWindows(W_ALL);
}


// Action Buttons
static void on_done_button_clicked (GtkButton *button, gpointer user_data)
{
  if ( !texdef_face_list_empty() )
    GetTexMods(TRUE);
  HideDlg();
  Sys_UpdateWindows(W_ALL);
}

static void on_apply_button_clicked (GtkButton *button, gpointer user_data)
{
  if (!g_bListenChanged)
    return;

  if ( !texdef_face_list_empty() )
  {
    GetTexMods (TRUE);
    Sys_UpdateWindows(W_CAMERA);
    GetTexdefInfo_from_Radiant();
    SetTexMods();
  }
}

static void on_cancel_button_clicked (GtkButton *button, gpointer user_data)
{
  texturewin = Texturewin ();
  texturewin->texdef = g_old_texdef;
  // cancel the last do if we own it
  if ( (m_nUndoId == Undo_GetUndoId()) && ( m_nUndoId != 0 ))
  {
#ifdef DBG_SI
    Sys_Printf("OnCancel calling Undo_Undo\n");
#endif
    g_bListenUpdate = false;
    Undo_Undo(TRUE);
    g_bListenUpdate = true;
    m_nUndoId = 0;
  }
  HideDlg();
}


