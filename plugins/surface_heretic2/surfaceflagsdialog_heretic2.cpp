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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "surfdlg_plugin.h"

#include "surfaceflagsdialog_heretic2.h"

  GtkWidget *surface_lightbutton;
  GtkWidget *surface_slickbutton;
  GtkWidget *surface_skybutton;
  GtkWidget *surface_warpbutton;
  GtkWidget *surface_trans33button;
  GtkWidget *surface_trans66button;
  GtkWidget *surface_flowingbutton;
  GtkWidget *surface_nodrawbutton;
  GtkWidget *surface_tallwallbutton;
  GtkWidget *surface_alphatexbutton;
  GtkWidget *surface_animspeedbutton;
  GtkWidget *surface_undulatebutton;

  GtkWidget *surf_gravel_radiobutton;
  GSList *surf_gravel_radiobutton_group = NULL;
  GtkWidget *surf_metal_radiobutton;
  GtkWidget *surf_stone_radiobutton;
  GtkWidget *surf_wood_radiobutton;

  GtkWidget *surf_value_entry;

  GtkWidget *notebook1;

  GtkWidget *content_solidbutton;
  GtkWidget *content_windowbutton;
  GtkWidget *content_illusbutton;
  GtkWidget *content_lavabutton;
  GtkWidget *content_slimebutton;
  GtkWidget *content_waterbutton;
  GtkWidget *content_mistbutton;
  GtkWidget *content_areaportalbutton;
  GtkWidget *content_playerclipbutton;
  GtkWidget *content_monsterclipbutton;
  GtkWidget *content_current0button;
  GtkWidget *content_current90button;
  GtkWidget *content_current180button;
  GtkWidget *content_current270button;
  GtkWidget *content_currentUPbutton;
  GtkWidget *content_currentDOWNbutton;
  GtkWidget *content_originbutton;
  GtkWidget *content_detailbutton;
  GtkWidget *content_ladderbutton;
  GtkWidget *content_camnoblockbutton;


  gboolean setup_buttons = TRUE;

  int working_surface_flags;
  int surface_mask;
  int working_content_flags;
  int content_mask;
  int working_value;
  gboolean surface_material_inconsistant = FALSE;

inline void set_inconsistent(GtkWidget *toggle_button)
{
  gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON (toggle_button), TRUE);
}

inline void clear_inconsistent(GtkWidget *toggle_button)
{
  if ( gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON (toggle_button)) )
  {
    gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON (toggle_button), FALSE);
  }

}

void clear_all_inconsistent(void)
{
  clear_inconsistent( surface_lightbutton );
  clear_inconsistent( surface_slickbutton );
  clear_inconsistent( surface_skybutton );
  clear_inconsistent( surface_warpbutton );
  clear_inconsistent( surface_trans33button );
  clear_inconsistent( surface_trans66button );
  clear_inconsistent( surface_flowingbutton );
  clear_inconsistent( surface_nodrawbutton );
  clear_inconsistent( surface_tallwallbutton );
  clear_inconsistent( surface_alphatexbutton );
  clear_inconsistent( surface_animspeedbutton );
  clear_inconsistent( surface_undulatebutton );

  clear_inconsistent( surf_gravel_radiobutton );
  clear_inconsistent( surf_metal_radiobutton );
  clear_inconsistent( surf_stone_radiobutton );
  clear_inconsistent( surf_wood_radiobutton );

  clear_inconsistent( content_solidbutton );
  clear_inconsistent( content_windowbutton );
  clear_inconsistent( content_illusbutton );
  clear_inconsistent( content_lavabutton );
  clear_inconsistent( content_slimebutton );
  clear_inconsistent( content_waterbutton );
  clear_inconsistent( content_mistbutton );
  clear_inconsistent( content_areaportalbutton );
  clear_inconsistent( content_playerclipbutton );
  clear_inconsistent( content_monsterclipbutton );
  clear_inconsistent( content_current0button );
  clear_inconsistent( content_current90button );
  clear_inconsistent( content_current180button );
  clear_inconsistent( content_current270button );
  clear_inconsistent( content_currentUPbutton );
  clear_inconsistent( content_currentDOWNbutton );
  clear_inconsistent( content_originbutton );
  clear_inconsistent( content_detailbutton );
  clear_inconsistent( content_ladderbutton );
  clear_inconsistent( content_camnoblockbutton );
}

void clear_all_buttons_and_values()
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_lightbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_slickbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_skybutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_warpbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_trans33button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_trans66button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_flowingbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_nodrawbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_tallwallbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_alphatexbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_animspeedbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( surface_undulatebutton ), FALSE);

//  surface_material_inconsistant = TRUE;

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_gravel_radiobutton ), FALSE);
  set_inconsistent(surf_gravel_radiobutton);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_metal_radiobutton ), FALSE);
  set_inconsistent(surf_metal_radiobutton);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_stone_radiobutton ), FALSE);
  set_inconsistent(surf_stone_radiobutton);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_wood_radiobutton ), FALSE);
  set_inconsistent(surf_wood_radiobutton);

  gtk_entry_set_text( (GtkEntry *)surf_value_entry, "");

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_solidbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_windowbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_illusbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_lavabutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_slimebutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_waterbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_mistbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_areaportalbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_playerclipbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_monsterclipbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_current0button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_current90button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_current180button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_current270button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_currentUPbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_currentDOWNbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_originbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_detailbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_ladderbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( content_camnoblockbutton ), FALSE);

}

void SetFlagButtons_Heretic2(texdef_to_face_t *texdef_face_list, bool b_isListEmpty)
{
  int i;
  int contents = 0;
  int flags = 0;
  int value = 0;
  int diff_contents = 0;
  int diff_flags = 0;
  gboolean diff_value = FALSE;
  char tex_buff[11];
  texdef_t* tmp_texdef;
  texdef_to_face_t* temp_texdef_face_list;
  gboolean surface_which_material_inconsistant[4];
  int surface_iterator;

  setup_buttons = TRUE;
  working_surface_flags = 0;
  surface_mask = 0;
  working_content_flags = 0;
  content_mask = 0;
  working_value = 0;
  surface_material_inconsistant = FALSE;
  surface_which_material_inconsistant[0] = FALSE;
  surface_which_material_inconsistant[1] = FALSE;
  surface_which_material_inconsistant[2] = FALSE;
  surface_which_material_inconsistant[3] = FALSE;

  if(!b_isListEmpty)
  {
    tmp_texdef = &texdef_face_list->texdef;
    contents = tmp_texdef->contents;
    flags = tmp_texdef->flags;
    value = tmp_texdef->value;

    surface_iterator = (tmp_texdef->flags & ~HERETIC2_SURF_MATERIAL_MASK) >> 24;  // Inconsistant Material?
    surface_which_material_inconsistant[surface_iterator] = TRUE;

    for (temp_texdef_face_list = texdef_face_list->next; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = &temp_texdef_face_list->texdef;
      diff_contents |= contents ^ tmp_texdef->contents;  // Figure out which buttons are inconsistent
      diff_flags |= flags ^ tmp_texdef->flags;
      if (tmp_texdef->value != value)
        diff_value = TRUE;

      surface_iterator = (tmp_texdef->flags & ~HERETIC2_SURF_MATERIAL_MASK) >> 24;  // Inconsistant Material?
      surface_which_material_inconsistant[surface_iterator] = TRUE;

      Sys_Printf("Diff_Flags: %d\t Surf_Iter: %d\n",diff_flags, surface_iterator);

    }
  }


  clear_all_inconsistent();

  // If no faces/brushes are selected, clear everything and bail
  if(b_isListEmpty)
  {
    clear_all_buttons_and_values();
    setup_buttons = FALSE;
    return;
  }

  // Set surface buttons to reflect brush/face flags, contents, and values
  if(diff_flags & HERETIC2_SURF_LIGHT)
    set_inconsistent(surface_lightbutton);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (surface_lightbutton), (flags & HERETIC2_SURF_LIGHT));

  if(diff_flags & HERETIC2_SURF_SLICK)
    set_inconsistent(surface_slickbutton);
  else if(flags & HERETIC2_SURF_SLICK)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_slickbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_slickbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_SKY)
    set_inconsistent(surface_skybutton);
  else if(flags & HERETIC2_SURF_SKY)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skybutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skybutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_WARP)
    set_inconsistent(surface_warpbutton);
  else if(flags & HERETIC2_SURF_WARP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_warpbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_warpbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_TRANS33)
    set_inconsistent(surface_trans33button);
  else if(flags & HERETIC2_SURF_TRANS33)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans33button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans33button ), FALSE);

  if(diff_flags & HERETIC2_SURF_TRANS66)
    set_inconsistent(surface_trans66button);
  else if(flags & HERETIC2_SURF_TRANS66)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans66button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans66button ), FALSE);

  if(diff_flags & HERETIC2_SURF_FLOWING)
    set_inconsistent(surface_flowingbutton);
  else if(flags & HERETIC2_SURF_FLOWING)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_flowingbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_flowingbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_NODRAW)
    set_inconsistent(surface_nodrawbutton);
  else if(flags & HERETIC2_SURF_NODRAW)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_nodrawbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_nodrawbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_TALL_WALL)
    set_inconsistent(surface_tallwallbutton);
  else if(flags & HERETIC2_SURF_TALL_WALL)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_tallwallbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_tallwallbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_ALPHA_TEXTURE)
    set_inconsistent(surface_alphatexbutton);
  else if(flags & HERETIC2_SURF_ALPHA_TEXTURE)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_alphatexbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_alphatexbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_ANIMSPEED)
    set_inconsistent(surface_animspeedbutton);
  else if(flags & HERETIC2_SURF_ANIMSPEED)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_animspeedbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_animspeedbutton ), FALSE);

  if(diff_flags & HERETIC2_SURF_UNDULATE)
    set_inconsistent(surface_undulatebutton);
  else if(flags & HERETIC2_SURF_UNDULATE)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_undulatebutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_undulatebutton ), FALSE);

  if(diff_flags & ~HERETIC2_SURF_MATERIAL_MASK)
  {
    Sys_Printf("--> %d\n", (diff_flags & ~HERETIC2_SURF_MATERIAL_MASK) );
    Sys_Printf("%d\t%d\t%d\t%d\n", surface_which_material_inconsistant[0], surface_which_material_inconsistant[1], surface_which_material_inconsistant[2], surface_which_material_inconsistant[3]);

    if (surface_which_material_inconsistant[0])
    {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_gravel_radiobutton ), FALSE);
      surface_material_inconsistant = TRUE;
      set_inconsistent(surf_gravel_radiobutton);
    }
    if (surface_which_material_inconsistant[1])
    {
      //if (!surface_material_inconsistant)
      //{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_metal_radiobutton ), FALSE);
        surface_material_inconsistant = TRUE;
      //}
      set_inconsistent(surf_metal_radiobutton);
    }
    if (surface_which_material_inconsistant[2])
    {
      //if (!surface_material_inconsistant)
      //{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_stone_radiobutton ), FALSE);
        surface_material_inconsistant = TRUE;
      //}
      set_inconsistent(surf_stone_radiobutton);
    }
    if (surface_which_material_inconsistant[3])
    {
      //if (!surface_material_inconsistant)
      //{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_wood_radiobutton ), FALSE);
        surface_material_inconsistant = TRUE;
      //}
      set_inconsistent(surf_wood_radiobutton);
    }
  }
  else
  {
    if(flags & ~HERETIC2_SURF_MATERIAL_MASK)
    {
      surface_iterator = (flags & ~HERETIC2_SURF_MATERIAL_MASK) >> 24;
      if( surface_iterator == 1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_metal_radiobutton ), TRUE);
      else if(surface_iterator == 2)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_stone_radiobutton ), TRUE);
      else if(surface_iterator == 3)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_wood_radiobutton ), TRUE);
    }
    else
    {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surf_gravel_radiobutton ), TRUE);
    }
  }

  // Set content buttons to reflect brush values
  if(diff_contents & HERETIC2_CONTENTS_SOLID)
    set_inconsistent(content_solidbutton);
  else if(contents & HERETIC2_CONTENTS_SOLID)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_solidbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_solidbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_WINDOW)
    set_inconsistent(content_windowbutton);
  else if(contents & HERETIC2_CONTENTS_WINDOW)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_windowbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_windowbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_ILLUSIONARY)
    set_inconsistent(content_illusbutton);
  else if(contents & HERETIC2_CONTENTS_ILLUSIONARY)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_illusbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_illusbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_LAVA)
    set_inconsistent(content_lavabutton);
  else if(contents & HERETIC2_CONTENTS_LAVA)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_lavabutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_lavabutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_SLIME)
    set_inconsistent(content_slimebutton);
  else if(contents & HERETIC2_CONTENTS_SLIME)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_slimebutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_slimebutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_WATER)
    set_inconsistent(content_waterbutton);
  else if(contents & HERETIC2_CONTENTS_WATER)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_waterbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_waterbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_MIST)
    set_inconsistent(content_mistbutton);
  else if(contents & HERETIC2_CONTENTS_MIST)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_mistbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_mistbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_AREAPORTAL)
    set_inconsistent(content_areaportalbutton);
  else if(contents & HERETIC2_CONTENTS_AREAPORTAL)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_areaportalbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_areaportalbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_PLAYERCLIP)
    set_inconsistent(content_playerclipbutton);
  else if(contents & HERETIC2_CONTENTS_PLAYERCLIP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_playerclipbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_playerclipbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_MONSTERCLIP)
    set_inconsistent(content_monsterclipbutton);
  else if(contents & HERETIC2_CONTENTS_MONSTERCLIP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_monsterclipbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_monsterclipbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_0)
    set_inconsistent(content_current0button);
  else if(contents & HERETIC2_CONTENTS_CURRENT_0)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current0button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current0button ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_90)
    set_inconsistent(content_current90button);
  else if(contents & HERETIC2_CONTENTS_CURRENT_90)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current90button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current90button ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_180)
    set_inconsistent(content_current180button);
  else if(contents & HERETIC2_CONTENTS_CURRENT_180)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current180button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current180button ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_270)
    set_inconsistent(content_current270button);
  else if(contents & HERETIC2_CONTENTS_CURRENT_270)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current270button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current270button ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_UP)
    set_inconsistent(content_currentUPbutton);
  else if(contents & HERETIC2_CONTENTS_CURRENT_UP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentUPbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentUPbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CURRENT_DOWN)
    set_inconsistent(content_currentDOWNbutton);
  else if(contents & HERETIC2_CONTENTS_CURRENT_DOWN)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentDOWNbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentDOWNbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_ORIGIN)
    set_inconsistent(content_originbutton);
  else if(contents & HERETIC2_CONTENTS_ORIGIN)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_originbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_originbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_DETAIL)
    set_inconsistent(content_detailbutton);
  else if(contents & HERETIC2_CONTENTS_DETAIL)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_detailbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_detailbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_LADDER)
    set_inconsistent(content_ladderbutton);
  else if(contents & HERETIC2_CONTENTS_LADDER)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_ladderbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_ladderbutton ), FALSE);

  if(diff_contents & HERETIC2_CONTENTS_CAMERANOBLOCK)
    set_inconsistent(content_camnoblockbutton);
  else if(contents & HERETIC2_CONTENTS_CAMERANOBLOCK)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_camnoblockbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_camnoblockbutton ), FALSE);

  // Set Value
  if(diff_value)
    gtk_entry_set_text( (GtkEntry *)surf_value_entry, "");
  else
  {
    working_value = value;
    sprintf( tex_buff, "%d", value);
    gtk_entry_set_text( (GtkEntry *)surf_value_entry, tex_buff);
  }

  setup_buttons = FALSE;
}

void SetChangeInFlags_Face_Heretic2 (texdef_to_face_t *texdef_face_list)
{
  texdef_to_face_t *temp_texdef_face_list;
  texdef_t *tmp_texdef;

  for (temp_texdef_face_list = texdef_face_list; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
  {
    tmp_texdef = &temp_texdef_face_list->texdef;
    tmp_texdef->flags = (tmp_texdef->flags & ~surface_mask) | working_surface_flags;
    tmp_texdef->contents = (tmp_texdef->contents & ~content_mask) | working_content_flags;
    tmp_texdef->value = working_value;
    Sys_Printf("content_flag: %d     content_mask: %d\n",working_content_flags,content_mask);
    Sys_Printf("content: %d\n",tmp_texdef->contents);
  }
}

inline void change_surfaceflag (GtkWidget *togglebutton, int sur_flag) // For Material
{
  if (!setup_buttons) // If we're setting up the buttons, we really don't need to
  {                   // set flags that are already set
    if (surface_material_inconsistant)
    {
      clear_inconsistent( surf_gravel_radiobutton );
      clear_inconsistent( surf_metal_radiobutton );
      clear_inconsistent( surf_stone_radiobutton );
      clear_inconsistent( surf_wood_radiobutton );
    }
    surface_mask |= ~HERETIC2_SURF_MATERIAL_MASK;
    working_surface_flags = (working_surface_flags & HERETIC2_SURF_MATERIAL_MASK) | sur_flag;
  }
}

inline void change_material (GtkWidget *togglebutton)
{
  if (!setup_buttons) // If we're setting up the buttons, we really don't need to
  {                   // set flags that are already set
    if (surface_material_inconsistant)
    {
      clear_inconsistent( surf_gravel_radiobutton );
      clear_inconsistent( surf_metal_radiobutton );
      clear_inconsistent( surf_stone_radiobutton );
      clear_inconsistent( surf_wood_radiobutton );
    }
    if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_gravel_radiobutton))  )
    {
      surface_mask |= ~HERETIC2_SURF_MATERIAL_MASK;
      working_surface_flags = (working_surface_flags & HERETIC2_SURF_MATERIAL_MASK) | HERETIC2_SURF_TYPE_GRAVEL;
    }
    else if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_metal_radiobutton)) )
    {
      surface_mask |= ~HERETIC2_SURF_MATERIAL_MASK;
      working_surface_flags = (working_surface_flags & HERETIC2_SURF_MATERIAL_MASK) | HERETIC2_SURF_TYPE_METAL;
    }
    else if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_stone_radiobutton)) )
    {
      surface_mask |= ~HERETIC2_SURF_MATERIAL_MASK;
      working_surface_flags = (working_surface_flags & HERETIC2_SURF_MATERIAL_MASK) | HERETIC2_SURF_TYPE_STONE;
    }
    else if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_wood_radiobutton)) )
    {
      surface_mask |= ~HERETIC2_SURF_MATERIAL_MASK;
      working_surface_flags = (working_surface_flags & HERETIC2_SURF_MATERIAL_MASK) | HERETIC2_SURF_TYPE_WOOD;
    }
  }
}

inline void change_surfaceflag (GtkWidget *togglebutton, int sur_flag, gboolean change_flag_to)
{

  if (!setup_buttons) // If we're setting up the buttons, we really don't need to
  {                   // set flags that are already set
    if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON (togglebutton))) // Clear out inconsistent, if set
	    clear_inconsistent(GTK_WIDGET (togglebutton));

    surface_mask |= sur_flag;

    if (change_flag_to)
      working_surface_flags |= sur_flag;
    else
      working_surface_flags &= ~sur_flag;
  }
}

inline void change_contentflag (GtkWidget *togglebutton, int content_flag, gboolean change_flag_to)
{

  if ( (!setup_buttons) )  // If we're setting up the buttons, we really don't need to
  {                        // set flags that are already set

    if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON (togglebutton)))
	    clear_inconsistent(togglebutton);
    //if (g_ptrSelectedFaces.GetSize() == 0)  // Only changing content flags on whole brushes, not faces.
    //{
      content_mask |= content_flag;

    if (change_flag_to)
      working_content_flags |= content_flag;
    else
      working_content_flags &= ~content_flag;
    //}
  }
}

// Surface Flags Callbacks
void
on_surface_lightbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_lightbutton, HERETIC2_SURF_LIGHT, (GTK_TOGGLE_BUTTON (surface_lightbutton)->active));
}


void
on_surface_slickbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_slickbutton, HERETIC2_SURF_SLICK, (GTK_TOGGLE_BUTTON (surface_slickbutton)->active));
}


void
on_surface_skybutton_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_skybutton, HERETIC2_SURF_SKY, (GTK_TOGGLE_BUTTON (surface_skybutton)->active));
}


void
on_surface_warpbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_warpbutton, HERETIC2_SURF_WARP, (GTK_TOGGLE_BUTTON (surface_warpbutton)->active));
}


void
on_surface_trans33button_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_trans33button, HERETIC2_SURF_TRANS33, (GTK_TOGGLE_BUTTON (surface_trans33button)->active));
}


void
on_surface_trans66button_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_trans66button, HERETIC2_SURF_TRANS66, (GTK_TOGGLE_BUTTON (surface_trans66button)->active));
}


void
on_surface_flowingbutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_flowingbutton, HERETIC2_SURF_FLOWING, (GTK_TOGGLE_BUTTON (surface_flowingbutton)->active));
}


void
on_surface_nodrawbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_nodrawbutton, HERETIC2_SURF_NODRAW, (GTK_TOGGLE_BUTTON (surface_nodrawbutton)->active));
}


void
on_surface_tallwallbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_tallwallbutton, HERETIC2_SURF_TALL_WALL, (GTK_TOGGLE_BUTTON (surface_tallwallbutton)->active));
}


void
on_surface_alphatexbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_alphatexbutton, HERETIC2_SURF_ALPHA_TEXTURE, (GTK_TOGGLE_BUTTON (surface_alphatexbutton)->active));
}


void
on_surface_animspeedbutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_animspeedbutton, HERETIC2_SURF_ANIMSPEED, (GTK_TOGGLE_BUTTON (surface_animspeedbutton)->active));
}


void
on_surface_undulatebutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_surfaceflag(surface_undulatebutton, HERETIC2_SURF_UNDULATE, (GTK_TOGGLE_BUTTON (surface_undulatebutton)->active));
}


void
on_surf_gravel_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_gravel_radiobutton)))
    change_material(GTK_WIDGET(togglebutton));
}

void
on_surf_metal_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_metal_radiobutton)))
    change_material(GTK_WIDGET(togglebutton));
}

void
on_surf_stone_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_stone_radiobutton)))
    change_material(GTK_WIDGET(togglebutton));
}

void
on_surf_wood_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(surf_wood_radiobutton)))
    change_material(GTK_WIDGET(togglebutton));
}

// Content Flags Callbacks
void
on_content_solidbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_solidbutton, HERETIC2_CONTENTS_SOLID, (GTK_TOGGLE_BUTTON (content_solidbutton)->active));
}


void
on_content_windowbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_windowbutton, HERETIC2_CONTENTS_WINDOW, (GTK_TOGGLE_BUTTON (content_windowbutton)->active));
}


void
on_content_illusbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_illusbutton, HERETIC2_CONTENTS_ILLUSIONARY, (GTK_TOGGLE_BUTTON (content_illusbutton)->active));
}


void
on_content_lavabutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_lavabutton, HERETIC2_CONTENTS_LAVA, (GTK_TOGGLE_BUTTON (content_lavabutton)->active));
}


void
on_content_slimebutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_slimebutton, HERETIC2_CONTENTS_SLIME, (GTK_TOGGLE_BUTTON (content_slimebutton)->active));
}


void
on_content_waterbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_waterbutton, HERETIC2_CONTENTS_WATER, (GTK_TOGGLE_BUTTON (content_waterbutton)->active));
}


void
on_content_mistbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_mistbutton, HERETIC2_CONTENTS_MIST, (GTK_TOGGLE_BUTTON (content_mistbutton)->active));
}


void
on_content_areaportalbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_areaportalbutton, HERETIC2_CONTENTS_AREAPORTAL, (GTK_TOGGLE_BUTTON (content_areaportalbutton)->active));
}


void
on_content_playerclipbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_playerclipbutton, HERETIC2_CONTENTS_PLAYERCLIP, (GTK_TOGGLE_BUTTON (content_playerclipbutton)->active));
}


void
on_content_monsterclipbutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_monsterclipbutton, HERETIC2_CONTENTS_MONSTERCLIP, (GTK_TOGGLE_BUTTON (content_monsterclipbutton)->active));
}


void
on_content_current0button_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_current0button, HERETIC2_CONTENTS_CURRENT_0, (GTK_TOGGLE_BUTTON (content_current0button)->active));
}


void
on_content_current90button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_current90button, HERETIC2_CONTENTS_CURRENT_90, (GTK_TOGGLE_BUTTON (content_current90button)->active));
}


void
on_content_current180button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_current180button, HERETIC2_CONTENTS_CURRENT_180, (GTK_TOGGLE_BUTTON (content_current180button)->active));
}


void
on_content_current270button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_current270button, HERETIC2_CONTENTS_CURRENT_270, (GTK_TOGGLE_BUTTON (content_current270button)->active));
}


void
on_content_currentUPbutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_currentUPbutton, HERETIC2_CONTENTS_CURRENT_UP, (GTK_TOGGLE_BUTTON (content_currentUPbutton)->active));
}


void
on_content_currentDOWNbutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_currentDOWNbutton, HERETIC2_CONTENTS_CURRENT_DOWN, (GTK_TOGGLE_BUTTON (content_currentDOWNbutton)->active));
}


void
on_content_originbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_originbutton, HERETIC2_CONTENTS_ORIGIN, (GTK_TOGGLE_BUTTON (content_originbutton)->active));
}


void
on_content_detailbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_detailbutton, HERETIC2_CONTENTS_DETAIL, (GTK_TOGGLE_BUTTON (content_detailbutton)->active));
}


void
on_content_ladderbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_ladderbutton, HERETIC2_CONTENTS_LADDER, (GTK_TOGGLE_BUTTON (content_ladderbutton)->active));
}


void
on_content_camnoblockbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  change_contentflag(content_camnoblockbutton, HERETIC2_CONTENTS_CAMERANOBLOCK, (GTK_TOGGLE_BUTTON (content_camnoblockbutton)->active));
}

// Value Entry Callback
void
on_surf_value_entry_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  if ( (!setup_buttons) )  // If we're setting up the buttons, don't change value
    working_value = atoi( gtk_entry_get_text( (GtkEntry*)editable) );
}

void
on_surf_value_entry_insert_text         (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data)
{
  int i, count=0;
  gchar *result;
  int entry_value;
  texdef_t *pt;
  brush_t *b;
  face_t *f;

  // Limit input to digits, throwing out anything else
  // Modified from Gtk FAQ for text filtering of GtkEntry
  result = g_new (gchar, new_text_length);

  for (i=0; i < new_text_length; i++) {
    if (!isdigit(new_text[i]))
      continue;
  result[count++] = new_text[i];
  }

  if (count > 0) {
    gtk_signal_handler_block_by_func (GTK_OBJECT (editable),
				      GTK_SIGNAL_FUNC (on_surf_value_entry_insert_text),
				      user_data);
    gtk_editable_insert_text (editable, result, count, position);
    gtk_signal_handler_unblock_by_func (GTK_OBJECT (editable),
					GTK_SIGNAL_FUNC (on_surf_value_entry_insert_text),
					user_data);
  }
  gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), "insert_text");

  g_free (result);
}

#define HERETIC2_FLAG_BUTTON_BORDER 3

GtkWidget* Create_Heretic2FlagsDialog (GtkWidget* surfacedialog_widget)
{
  GtkWidget *frame1;
  GtkWidget *notebook1;
  GtkWidget *vbox3;
  GtkWidget *table1;
  GtkWidget *frame2;
  GtkWidget *hbox4;
  GtkWidget *label4;
  GtkWidget *vbox4;
  GtkWidget *table3;
  GtkWidget *label5;

  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *label2;
  GtkWidget *table2;
  GtkWidget *label3;


  frame1 = gtk_frame_new ("Brush/Face Flags");
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (surfacedialog_widget), frame1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_container_add (GTK_CONTAINER (frame1), notebook1);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox3);

  table1 = gtk_table_new (3, 4, TRUE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox3), table1, TRUE, TRUE, 0);

  surface_lightbutton = gtk_toggle_button_new_with_mnemonic ("Light");
  gtk_widget_show (surface_lightbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_lightbutton, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_slickbutton = gtk_toggle_button_new_with_mnemonic ("Slick");
  gtk_widget_show (surface_slickbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_slickbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_skybutton = gtk_toggle_button_new_with_mnemonic ("Sky");
  gtk_widget_show (surface_skybutton);
  gtk_table_attach (GTK_TABLE (table1), surface_skybutton, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_warpbutton = gtk_toggle_button_new_with_mnemonic ("Warp");
  gtk_widget_show (surface_warpbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_warpbutton, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_trans33button = gtk_toggle_button_new_with_mnemonic ("Trans33");
  gtk_widget_show (surface_trans33button);
  gtk_table_attach (GTK_TABLE (table1), surface_trans33button, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_trans66button = gtk_toggle_button_new_with_mnemonic ("Trans66");
  gtk_widget_show (surface_trans66button);
  gtk_table_attach (GTK_TABLE (table1), surface_trans66button, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_flowingbutton = gtk_toggle_button_new_with_mnemonic ("Flowing");
  gtk_widget_show (surface_flowingbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_flowingbutton, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_nodrawbutton = gtk_toggle_button_new_with_mnemonic ("NoDraw");
  gtk_widget_show (surface_nodrawbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_nodrawbutton, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_tallwallbutton = gtk_toggle_button_new_with_mnemonic ("TallWall");
  gtk_widget_show (surface_tallwallbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_tallwallbutton, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_alphatexbutton = gtk_toggle_button_new_with_mnemonic ("AlphaTex");
  gtk_widget_show (surface_alphatexbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_alphatexbutton, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_animspeedbutton = gtk_toggle_button_new_with_mnemonic ("AnimSpeed");
  gtk_widget_show (surface_animspeedbutton);
  gtk_table_attach (GTK_TABLE (table1), surface_animspeedbutton, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  surface_undulatebutton = gtk_toggle_button_new_with_mnemonic ("Undulate");
  gtk_widget_show (surface_undulatebutton);
  gtk_table_attach (GTK_TABLE (table1), surface_undulatebutton, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox3), frame2, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame2), 4);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_container_add (GTK_CONTAINER (frame2), hbox4);

  surf_gravel_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, "Gravel");
  gtk_widget_show (surf_gravel_radiobutton);
  gtk_box_pack_start (GTK_BOX (hbox4), surf_gravel_radiobutton, TRUE, FALSE, 0);

  surf_metal_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(surf_gravel_radiobutton), "Metal");
  gtk_widget_show (surf_metal_radiobutton);
  gtk_box_pack_start (GTK_BOX (hbox4), surf_metal_radiobutton, TRUE, FALSE, 0);

  surf_stone_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(surf_metal_radiobutton), "Stone");
  gtk_widget_show (surf_stone_radiobutton);
  gtk_box_pack_start (GTK_BOX (hbox4), surf_stone_radiobutton, TRUE, FALSE, 0);

  surf_wood_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(surf_stone_radiobutton), "Wood");
  gtk_widget_show (surf_wood_radiobutton);
  gtk_box_pack_start (GTK_BOX (hbox4), surf_wood_radiobutton, TRUE, FALSE, 0);

  label4 = gtk_label_new ("Material");
  gtk_widget_show (label4);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label4);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  table3 = gtk_table_new (1, 4, FALSE);
  gtk_widget_show (table3);
  gtk_box_pack_start (GTK_BOX (vbox3), table3, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table3), 3);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_table_attach (GTK_TABLE (table3), hbox1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label5 = gtk_label_new ("Value: ");
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table3), label5, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_RIGHT);

  surf_value_entry = gtk_entry_new ();
  gtk_widget_show (surf_value_entry);
  gtk_table_attach (GTK_TABLE (table3), surf_value_entry, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_table_attach (GTK_TABLE (table3), hbox2, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label2 = gtk_label_new ("Surface Flags");
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label2);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  table2 = gtk_table_new (5, 4, TRUE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (notebook1), table2);

  content_solidbutton = gtk_toggle_button_new_with_mnemonic ("Solid");
  gtk_widget_show (content_solidbutton);
  gtk_table_attach (GTK_TABLE (table2), content_solidbutton, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_windowbutton = gtk_toggle_button_new_with_mnemonic ("Window");
  gtk_widget_show (content_windowbutton);
  gtk_table_attach (GTK_TABLE (table2), content_windowbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_illusbutton = gtk_toggle_button_new_with_mnemonic ("Illusion");
  gtk_widget_show (content_illusbutton);
  gtk_table_attach (GTK_TABLE (table2), content_illusbutton, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_lavabutton = gtk_toggle_button_new_with_mnemonic ("Lava");
  gtk_widget_show (content_lavabutton);
  gtk_table_attach (GTK_TABLE (table2), content_lavabutton, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_slimebutton = gtk_toggle_button_new_with_mnemonic ("Slime");
  gtk_widget_show (content_slimebutton);
  gtk_table_attach (GTK_TABLE (table2), content_slimebutton, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_waterbutton = gtk_toggle_button_new_with_mnemonic ("Water");
  gtk_widget_show (content_waterbutton);
  gtk_table_attach (GTK_TABLE (table2), content_waterbutton, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_mistbutton = gtk_toggle_button_new_with_mnemonic ("Mist");
  gtk_widget_show (content_mistbutton);
  gtk_table_attach (GTK_TABLE (table2), content_mistbutton, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_areaportalbutton = gtk_toggle_button_new_with_mnemonic ("AreaPortal");
  gtk_widget_show (content_areaportalbutton);
  gtk_table_attach (GTK_TABLE (table2), content_areaportalbutton, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_playerclipbutton = gtk_toggle_button_new_with_mnemonic ("PlayerClip");
  gtk_widget_show (content_playerclipbutton);
  gtk_table_attach (GTK_TABLE (table2), content_playerclipbutton, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_monsterclipbutton = gtk_toggle_button_new_with_mnemonic ("MonsterClip");
  gtk_widget_show (content_monsterclipbutton);
  gtk_table_attach (GTK_TABLE (table2), content_monsterclipbutton, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_current0button = gtk_toggle_button_new_with_mnemonic ("Current 0");
  gtk_widget_show (content_current0button);
  gtk_table_attach (GTK_TABLE (table2), content_current0button, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_current90button = gtk_toggle_button_new_with_mnemonic ("Current 90");
  gtk_widget_show (content_current90button);
  gtk_table_attach (GTK_TABLE (table2), content_current90button, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_current180button = gtk_toggle_button_new_with_mnemonic ("Current 180");
  gtk_widget_show (content_current180button);
  gtk_table_attach (GTK_TABLE (table2), content_current180button, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_current270button = gtk_toggle_button_new_with_mnemonic ("Current 270");
  gtk_widget_show (content_current270button);
  gtk_table_attach (GTK_TABLE (table2), content_current270button, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_currentUPbutton = gtk_toggle_button_new_with_mnemonic ("Current UP");
  gtk_widget_show (content_currentUPbutton);
  gtk_table_attach (GTK_TABLE (table2), content_currentUPbutton, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_currentDOWNbutton = gtk_toggle_button_new_with_mnemonic ("Current DOWN");
  gtk_widget_show (content_currentDOWNbutton);
  gtk_table_attach (GTK_TABLE (table2), content_currentDOWNbutton, 3, 4, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_originbutton = gtk_toggle_button_new_with_mnemonic ("Origin");
  gtk_widget_show (content_originbutton);
  gtk_table_attach (GTK_TABLE (table2), content_originbutton, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_detailbutton = gtk_toggle_button_new_with_mnemonic ("Detail");
  gtk_widget_show (content_detailbutton);
  gtk_table_attach (GTK_TABLE (table2), content_detailbutton, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_ladderbutton = gtk_toggle_button_new_with_mnemonic ("Ladder");
  gtk_widget_show (content_ladderbutton);
  gtk_table_attach (GTK_TABLE (table2), content_ladderbutton, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  content_camnoblockbutton = gtk_toggle_button_new_with_mnemonic ("Cam No Block");
  gtk_widget_show (content_camnoblockbutton);
  gtk_table_attach (GTK_TABLE (table2), content_camnoblockbutton, 3, 4, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label3 = gtk_label_new ("Content Flags");
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label3);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  // Signal Connects
  g_signal_connect ((gpointer) surface_lightbutton, "toggled",
                    G_CALLBACK (on_surface_lightbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_slickbutton, "toggled",
                    G_CALLBACK (on_surface_slickbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_skybutton, "toggled",
                    G_CALLBACK (on_surface_skybutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_warpbutton, "toggled",
                    G_CALLBACK (on_surface_warpbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_trans33button, "toggled",
                    G_CALLBACK (on_surface_trans33button_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_trans66button, "toggled",
                    G_CALLBACK (on_surface_trans66button_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_flowingbutton, "toggled",
                    G_CALLBACK (on_surface_flowingbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_nodrawbutton, "toggled",
                    G_CALLBACK (on_surface_nodrawbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_tallwallbutton, "toggled",
                    G_CALLBACK (on_surface_tallwallbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_alphatexbutton, "toggled",
                    G_CALLBACK (on_surface_alphatexbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_animspeedbutton, "toggled",
                    G_CALLBACK (on_surface_animspeedbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surface_undulatebutton, "toggled",
                    G_CALLBACK (on_surface_undulatebutton_toggled),
                    NULL);

  g_signal_connect ((gpointer) surf_gravel_radiobutton, "toggled",
                    G_CALLBACK (on_surf_gravel_radiobutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surf_metal_radiobutton, "toggled",
                    G_CALLBACK (on_surf_metal_radiobutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surf_stone_radiobutton, "toggled",
                    G_CALLBACK (on_surf_stone_radiobutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) surf_wood_radiobutton, "toggled",
                    G_CALLBACK (on_surf_wood_radiobutton_toggled),
                    NULL);

  g_signal_connect ((gpointer) surf_value_entry, "changed",
                    G_CALLBACK (on_surf_value_entry_changed),
                    NULL);
  g_signal_connect ((gpointer) surf_value_entry, "insert_text",
                    G_CALLBACK (on_surf_value_entry_insert_text),
                    NULL);
  g_signal_connect ((gpointer) content_solidbutton, "toggled",
                    G_CALLBACK (on_content_solidbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_windowbutton, "toggled",
                    G_CALLBACK (on_content_windowbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_illusbutton, "toggled",
                    G_CALLBACK (on_content_illusbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_lavabutton, "toggled",
                    G_CALLBACK (on_content_lavabutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_slimebutton, "toggled",
                    G_CALLBACK (on_content_slimebutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_waterbutton, "toggled",
                    G_CALLBACK (on_content_waterbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_mistbutton, "toggled",
                    G_CALLBACK (on_content_mistbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_areaportalbutton, "toggled",
                    G_CALLBACK (on_content_areaportalbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_playerclipbutton, "toggled",
                    G_CALLBACK (on_content_playerclipbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_monsterclipbutton, "toggled",
                    G_CALLBACK (on_content_monsterclipbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_current0button, "toggled",
                    G_CALLBACK (on_content_current0button_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_current90button, "toggled",
                    G_CALLBACK (on_content_current90button_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_current180button, "toggled",
                    G_CALLBACK (on_content_current180button_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_current270button, "toggled",
                    G_CALLBACK (on_content_current270button_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_currentUPbutton, "toggled",
                    G_CALLBACK (on_content_currentUPbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_currentDOWNbutton, "toggled",
                    G_CALLBACK (on_content_currentDOWNbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_originbutton, "toggled",
                    G_CALLBACK (on_content_originbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_detailbutton, "toggled",
                    G_CALLBACK (on_content_detailbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_ladderbutton, "toggled",
                    G_CALLBACK (on_content_ladderbutton_toggled),
                    NULL);
  g_signal_connect ((gpointer) content_camnoblockbutton, "toggled",
                    G_CALLBACK (on_content_camnoblockbutton_toggled),
                    NULL);


  return frame1;
}

