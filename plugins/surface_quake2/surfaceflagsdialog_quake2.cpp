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
#include <gdk/gdkkeysyms.h>

#include "surfdlg_plugin.h"

#include "surfaceflagsdialog_quake2.h"

  GtkWidget *notebook1;

  GtkWidget *surface_lightbutton;
  GtkWidget *surface_slickbutton;
  GtkWidget *surface_skybutton;
  GtkWidget *surface_warpbutton;
  GtkWidget *surface_trans33button;
  GtkWidget *surface_trans66button;
  GtkWidget *surface_flowingbutton;
  GtkWidget *surface_nodrawbutton;
  GtkWidget *surface_hintbutton;
  GtkWidget *surface_skipbutton;

  GtkWidget *content_solidbutton;
  GtkWidget *content_windowbutton;
  GtkWidget *content_auxbutton;
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
  GtkWidget *content_translucentbutton;
  GtkWidget *content_ladderbutton;

  GtkWidget *surfacebutton;
  GtkWidget *contentbutton;

  GtkWidget *value_entry;
  gboolean setup_buttons = TRUE;

  int working_surface_flags;
  int surface_mask;
  int working_content_flags;
  int content_mask;
  int working_value;

inline void set_inconsistent(GtkWidget *toggle_button)
{
  gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON (toggle_button), TRUE);
}

inline void clear_inconsistent(GtkWidget *toggle_button)
{
  GtkWidget *button_label;

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
  clear_inconsistent( surface_hintbutton );
  clear_inconsistent( surface_skipbutton );

  clear_inconsistent( content_solidbutton );
  clear_inconsistent( content_windowbutton );
  clear_inconsistent( content_auxbutton );
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
  clear_inconsistent( content_translucentbutton );
  clear_inconsistent( content_ladderbutton );
}

void clear_all_buttons_and_values()
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_lightbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_slickbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skybutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_warpbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans33button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans66button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_flowingbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_nodrawbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_hintbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skipbutton ), FALSE);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_solidbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_windowbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_auxbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_lavabutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_slimebutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_waterbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_mistbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_areaportalbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_playerclipbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_monsterclipbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current0button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current90button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current180button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current270button ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentUPbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentDOWNbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_originbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_detailbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_translucentbutton ), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_ladderbutton ), FALSE);

  gtk_entry_set_text( (GtkEntry *)value_entry, "");
}

void SetFlagButtons_Quake2(texdef_to_face_t *texdef_face_list, bool b_isListEmpty)
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


  setup_buttons = TRUE;
  working_surface_flags = 0;
  surface_mask = 0;
  working_content_flags = 0;
  content_mask = 0;
  working_value = 0;

  if(!b_isListEmpty)
  {
    tmp_texdef = &texdef_face_list->texdef;
    contents = tmp_texdef->contents;
    flags = tmp_texdef->flags;
    value = tmp_texdef->value;

    Sys_Printf("Surface: %d\tContents: %d\tValue: %d\ttmp_texdef\n",tmp_texdef->flags,tmp_texdef->contents,tmp_texdef->value);
    Sys_Printf("Surface: %d\tContents: %d\tValue: %d\n",flags,contents,value);

    for (temp_texdef_face_list = texdef_face_list->next; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next)
    {
      tmp_texdef = &temp_texdef_face_list->texdef;
      diff_contents |= contents ^ tmp_texdef->contents;  // Figure out which buttons are inconsistent
      diff_flags |= flags ^ tmp_texdef->flags;
      if (tmp_texdef->value != value)
        diff_value = TRUE;

      Sys_Printf("Surface: %d\tContents: %d\tValue: %d\ttmp_texdef\n",tmp_texdef->flags,tmp_texdef->contents,tmp_texdef->value);
      Sys_Printf("Surface: %d\tContents: %d\tValue: %d\n",flags,contents,value);

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
  if(diff_flags & QUAKE2_SURF_LIGHT)
    set_inconsistent(surface_lightbutton);
  else if(flags & QUAKE2_SURF_LIGHT)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (surface_lightbutton), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (surface_lightbutton), FALSE);

  if(diff_flags & QUAKE2_SURF_SLICK)
    set_inconsistent(surface_slickbutton);
  else if(flags & QUAKE2_SURF_SLICK)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_slickbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_slickbutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_SKY)
    set_inconsistent(surface_skybutton);
  else if(flags & QUAKE2_SURF_SKY)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skybutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skybutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_WARP)
    set_inconsistent(surface_warpbutton);
  else if(flags & QUAKE2_SURF_WARP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_warpbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_warpbutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_TRANS33)
    set_inconsistent(surface_trans33button);
  else if(flags & QUAKE2_SURF_TRANS33)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans33button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans33button ), FALSE);

  if(diff_flags & QUAKE2_SURF_TRANS66)
    set_inconsistent(surface_trans66button);
  else if(flags & QUAKE2_SURF_TRANS66)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans66button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_trans66button ), FALSE);

  if(diff_flags & QUAKE2_SURF_FLOWING)
    set_inconsistent(surface_flowingbutton);
  else if(flags & QUAKE2_SURF_FLOWING)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_flowingbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_flowingbutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_NODRAW)
    set_inconsistent(surface_nodrawbutton);
  else if(flags & QUAKE2_SURF_NODRAW)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_nodrawbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_nodrawbutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_HINT)
    set_inconsistent(surface_hintbutton);
  else if(flags & QUAKE2_SURF_HINT)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_hintbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_hintbutton ), FALSE);

  if(diff_flags & QUAKE2_SURF_SKIP)
    set_inconsistent(surface_skipbutton);
  else if(flags & QUAKE2_SURF_SKIP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skipbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( surface_skipbutton ), FALSE);

  // Set content buttons to reflect brush values
  if(diff_contents & QUAKE2_CONTENTS_SOLID)
    set_inconsistent(content_solidbutton);
  else if(contents & QUAKE2_CONTENTS_SOLID)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_solidbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_solidbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_WINDOW)
    set_inconsistent(content_windowbutton);
  else if(contents & QUAKE2_CONTENTS_WINDOW)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_windowbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_windowbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_AUX)
    set_inconsistent(content_auxbutton);
  else if(contents & QUAKE2_CONTENTS_AUX)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_auxbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_auxbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_LAVA)
    set_inconsistent(content_lavabutton);
  else if(contents & QUAKE2_CONTENTS_LAVA)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_lavabutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_lavabutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_SLIME)
    set_inconsistent(content_slimebutton);
  else if(contents & QUAKE2_CONTENTS_SLIME)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_slimebutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_slimebutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_WATER)
    set_inconsistent(content_waterbutton);
  else if(contents & QUAKE2_CONTENTS_WATER)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_waterbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_waterbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_MIST)
    set_inconsistent(content_mistbutton);
  else if(contents & QUAKE2_CONTENTS_MIST)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_mistbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_mistbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_AREAPORTAL)
    set_inconsistent(content_areaportalbutton);
  else if(contents & QUAKE2_CONTENTS_AREAPORTAL)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_areaportalbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_areaportalbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_PLAYERCLIP)
    set_inconsistent(content_playerclipbutton);
  else if(contents & QUAKE2_CONTENTS_PLAYERCLIP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_playerclipbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_playerclipbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_MONSTERCLIP)
    set_inconsistent(content_monsterclipbutton);
  else if(contents & QUAKE2_CONTENTS_MONSTERCLIP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_monsterclipbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_monsterclipbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_0)
    set_inconsistent(content_current0button);
  else if(contents & QUAKE2_CONTENTS_CURRENT_0)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current0button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current0button ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_90)
    set_inconsistent(content_current90button);
  else if(contents & QUAKE2_CONTENTS_CURRENT_90)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current90button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current90button ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_180)
    set_inconsistent(content_current180button);
  else if(contents & QUAKE2_CONTENTS_CURRENT_180)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current180button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current180button ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_270)
    set_inconsistent(content_current270button);
  else if(contents & QUAKE2_CONTENTS_CURRENT_270)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current270button ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_current270button ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_UP)
    set_inconsistent(content_currentUPbutton);
  else if(contents & QUAKE2_CONTENTS_CURRENT_UP)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentUPbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentUPbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_CURRENT_DOWN)
    set_inconsistent(content_currentDOWNbutton);
  else if(contents & QUAKE2_CONTENTS_CURRENT_DOWN)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentDOWNbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_currentDOWNbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_ORIGIN)
    set_inconsistent(content_originbutton);
  else if(contents & QUAKE2_CONTENTS_ORIGIN)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_originbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_originbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_DETAIL)
    set_inconsistent(content_detailbutton);
  else if(contents & QUAKE2_CONTENTS_DETAIL)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_detailbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_detailbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_TRANSLUCENT)
    set_inconsistent(content_translucentbutton);
  else if(contents & QUAKE2_CONTENTS_TRANSLUCENT)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_translucentbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_translucentbutton ), FALSE);

  if(diff_contents & QUAKE2_CONTENTS_LADDER)
    set_inconsistent(content_ladderbutton);
  else if(contents & QUAKE2_CONTENTS_LADDER)
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_ladderbutton ), TRUE);
  else
  	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON ( content_ladderbutton ), FALSE);

  // Set Value
  if(diff_value)
    gtk_entry_set_text( (GtkEntry *)value_entry, "");
  else
  {
    working_value = value;
    sprintf( tex_buff, "%d", value);
    gtk_entry_set_text( (GtkEntry *)value_entry, tex_buff);
  }

  setup_buttons = FALSE;
}

void SetChangeInFlags_Face_Quake2 (texdef_to_face_t *texdef_face_list)
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
    Sys_Printf("content_flag: %d     content_mask: %d\n",content_flag,content_mask);
  }
}

// Surface Flags Callbacks
void
on_surface_lightbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
       change_surfaceflag(surface_lightbutton, QUAKE2_SURF_LIGHT, (GTK_TOGGLE_BUTTON (surface_lightbutton)->active));

}

void
on_surface_slickbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_slickbutton, QUAKE2_SURF_SLICK, (GTK_TOGGLE_BUTTON (surface_slickbutton)->active));

}

void
on_surface_skybutton_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_skybutton, QUAKE2_SURF_SKY, (GTK_TOGGLE_BUTTON (surface_skybutton)->active));
}

void
on_surface_warpbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_warpbutton, QUAKE2_SURF_WARP, (GTK_TOGGLE_BUTTON (surface_warpbutton)->active));
}

void
on_surface_trans33button_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_trans33button, QUAKE2_SURF_TRANS33, (GTK_TOGGLE_BUTTON (surface_trans33button)->active));
}

void
on_surface_trans66button_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_trans66button, QUAKE2_SURF_TRANS66, (GTK_TOGGLE_BUTTON (surface_trans66button)->active));
}

void
on_surface_flowingbutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_flowingbutton, QUAKE2_SURF_FLOWING, (GTK_TOGGLE_BUTTON (surface_flowingbutton)->active));
}

void
on_surface_nodrawbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_nodrawbutton, QUAKE2_SURF_NODRAW, (GTK_TOGGLE_BUTTON (surface_nodrawbutton)->active));
}

void
on_surface_hintbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_hintbutton, QUAKE2_SURF_HINT, (GTK_TOGGLE_BUTTON (surface_hintbutton)->active));
}

void
on_surface_skipbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_surfaceflag(surface_skipbutton, QUAKE2_SURF_SKIP, (GTK_TOGGLE_BUTTON (surface_skipbutton)->active));
}

// Content Flags Callbacks
void
on_content_solidbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_solidbutton, QUAKE2_CONTENTS_SOLID, (GTK_TOGGLE_BUTTON (content_solidbutton)->active));
}

void
on_content_windowbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_windowbutton, QUAKE2_CONTENTS_WINDOW, (GTK_TOGGLE_BUTTON (content_windowbutton)->active));
}

void
on_content_auxbutton_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_auxbutton, QUAKE2_CONTENTS_AUX, (GTK_TOGGLE_BUTTON (content_auxbutton)->active));
}

void
on_content_lavabutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_lavabutton, QUAKE2_CONTENTS_LAVA, (GTK_TOGGLE_BUTTON (content_lavabutton)->active));
}

void
on_content_slimebutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_slimebutton, QUAKE2_CONTENTS_SLIME, (GTK_TOGGLE_BUTTON (content_slimebutton)->active));
}

void
on_content_waterbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_waterbutton, QUAKE2_CONTENTS_WATER, (GTK_TOGGLE_BUTTON (content_waterbutton)->active));
}

void
on_content_mistbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_mistbutton, QUAKE2_CONTENTS_MIST, (GTK_TOGGLE_BUTTON (content_mistbutton)->active));
}

void
on_content_areaportalbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_areaportalbutton, QUAKE2_CONTENTS_AREAPORTAL, (GTK_TOGGLE_BUTTON (content_areaportalbutton)->active));
}

void
on_content_playerclipbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_playerclipbutton, QUAKE2_CONTENTS_PLAYERCLIP, (GTK_TOGGLE_BUTTON (content_playerclipbutton)->active));
}

void
on_content_monsterclipbutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_monsterclipbutton, QUAKE2_CONTENTS_MONSTERCLIP, (GTK_TOGGLE_BUTTON (content_monsterclipbutton)->active));
}

void
on_content_current0button_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_current0button, QUAKE2_CONTENTS_CURRENT_0, (GTK_TOGGLE_BUTTON (content_current0button)->active));
}

void
on_content_current90button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_current90button, QUAKE2_CONTENTS_CURRENT_90, (GTK_TOGGLE_BUTTON (content_current90button)->active));
}

void
on_content_current180button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_current180button, QUAKE2_CONTENTS_CURRENT_180, (GTK_TOGGLE_BUTTON (content_current180button)->active));
}

void
on_content_current270button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_current270button, QUAKE2_CONTENTS_CURRENT_270, (GTK_TOGGLE_BUTTON (content_current270button)->active));
}

void
on_content_currentUPbutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_currentUPbutton, QUAKE2_CONTENTS_CURRENT_UP, (GTK_TOGGLE_BUTTON (content_currentUPbutton)->active));
}

void
on_content_currentDOWNbutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_currentDOWNbutton, QUAKE2_CONTENTS_CURRENT_DOWN, (GTK_TOGGLE_BUTTON (content_currentDOWNbutton)->active));
}

void
on_content_originbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_originbutton, QUAKE2_CONTENTS_ORIGIN, (GTK_TOGGLE_BUTTON (content_originbutton)->active));
}

void
on_content_detailbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_detailbutton, QUAKE2_CONTENTS_DETAIL, (GTK_TOGGLE_BUTTON (content_detailbutton)->active));
}

void
on_content_translucentbutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_translucentbutton, QUAKE2_CONTENTS_TRANSLUCENT, (GTK_TOGGLE_BUTTON (content_translucentbutton)->active));
}

void
on_content_ladderbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	change_contentflag(content_ladderbutton, QUAKE2_CONTENTS_LADDER, (GTK_TOGGLE_BUTTON (content_ladderbutton)->active));
}

// Value Entry Callback
void
on_value_entry_changed                (GtkEditable     *editable,
                                        gpointer         user_data)
{
  if ( (!setup_buttons) )  // If we're setting up the buttons, don't change value
    working_value = atoi( gtk_entry_get_text( (GtkEntry*)editable) );
}

void
on_value_entry_insert_text             (GtkEditable     *editable,
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
				      GTK_SIGNAL_FUNC (on_value_entry_insert_text),
				      user_data);
    gtk_editable_insert_text (editable, result, count, position);
    gtk_signal_handler_unblock_by_func (GTK_OBJECT (editable),
					GTK_SIGNAL_FUNC (on_value_entry_insert_text),
					user_data);
  }
  gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), "insert_text");

  g_free (result);
}

void
on_surfacebutton_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_notebook_set_page (GTK_NOTEBOOK(notebook1), 0);
}

void
on_contentbutton_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_notebook_set_page (GTK_NOTEBOOK(notebook1), 1);
}


#define QUAKE2_FLAG_BUTTON_BORDER 3

GtkWidget* Create_Quake2FlagsDialog (GtkWidget* surfacedialog_widget)
{
  GtkWidget *frame1;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *vbox3;
  GtkWidget *vbox4;
  GtkWidget *table4;
  GtkWidget *hbox2;
  GtkWidget *hbox3;
  GtkWidget *hseparator1;
  GtkWidget *value_label;
  GtkWidget *label5;
  GtkWidget *table3;
  GtkWidget *label6;
  GtkWidget *table1;


  frame1 = gtk_frame_new ("Flags");
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (surfacedialog_widget), frame1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (frame1), vbox1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox1), notebook1, TRUE, TRUE, 0);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook1), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (notebook1), 5);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox2);

  table4 = gtk_table_new (3, 4, FALSE);
  gtk_widget_show (table4);
  gtk_box_pack_start (GTK_BOX (vbox2), table4, TRUE, TRUE, 0);

  surface_lightbutton = gtk_toggle_button_new_with_label ("Light");
  gtk_signal_connect (GTK_OBJECT (surface_lightbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_lightbutton_toggled),
                      NULL);
  gtk_widget_show (surface_lightbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_lightbutton, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_lightbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_slickbutton = gtk_toggle_button_new_with_label ("Slick");
  gtk_signal_connect (GTK_OBJECT (surface_slickbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_slickbutton_toggled),
                      NULL);
  gtk_widget_show (surface_slickbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_slickbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_slickbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_skybutton = gtk_toggle_button_new_with_label ("Sky");
  gtk_signal_connect (GTK_OBJECT (surface_skybutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_skybutton_toggled),
                      NULL);
  gtk_widget_show (surface_skybutton);
  gtk_table_attach (GTK_TABLE (table4), surface_skybutton, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_skybutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_warpbutton = gtk_toggle_button_new_with_label ("Warp");
  gtk_signal_connect (GTK_OBJECT (surface_warpbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_warpbutton_toggled),
                      NULL);
  gtk_widget_show (surface_warpbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_warpbutton, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_warpbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_trans33button = gtk_toggle_button_new_with_label ("Trans 33");
  gtk_signal_connect (GTK_OBJECT (surface_trans33button), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_trans33button_toggled),
                      NULL);
  gtk_widget_show (surface_trans33button);
  gtk_table_attach (GTK_TABLE (table4), surface_trans33button, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_trans33button), QUAKE2_FLAG_BUTTON_BORDER);

  surface_trans66button = gtk_toggle_button_new_with_label ("Trans 66");
  gtk_signal_connect (GTK_OBJECT (surface_trans66button), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_trans66button_toggled),
                      NULL);
  gtk_widget_show (surface_trans66button);
  gtk_table_attach (GTK_TABLE (table4), surface_trans66button, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_trans66button), QUAKE2_FLAG_BUTTON_BORDER);

  surface_flowingbutton = gtk_toggle_button_new_with_label ("Flowing");
  gtk_signal_connect (GTK_OBJECT (surface_flowingbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_flowingbutton_toggled),
                      NULL);
  gtk_widget_show (surface_flowingbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_flowingbutton, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_flowingbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_nodrawbutton = gtk_toggle_button_new_with_label ("NoDraw");
  gtk_signal_connect (GTK_OBJECT (surface_nodrawbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_nodrawbutton_toggled),
                      NULL);
  gtk_widget_show (surface_nodrawbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_nodrawbutton, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_nodrawbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_hintbutton = gtk_toggle_button_new_with_label ("Hint");
  gtk_signal_connect (GTK_OBJECT (surface_hintbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_hintbutton_toggled),
                      NULL);
  gtk_widget_show (surface_hintbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_hintbutton, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_hintbutton), QUAKE2_FLAG_BUTTON_BORDER);

  surface_skipbutton = gtk_toggle_button_new_with_label ("Skip");
  gtk_signal_connect (GTK_OBJECT (surface_skipbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_surface_skipbutton_toggled),
                      NULL);
  gtk_widget_show (surface_skipbutton);
  gtk_table_attach (GTK_TABLE (table4), surface_skipbutton, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (surface_skipbutton), QUAKE2_FLAG_BUTTON_BORDER);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, FALSE, FALSE, 0);
  gtk_widget_set_usize (hseparator1, -2, 5);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (hbox2), hbox3, TRUE, TRUE, 0);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox4, TRUE, TRUE, 0);

  value_label = gtk_label_new (" Value: ");
  gtk_widget_show (value_label);
  gtk_box_pack_start (GTK_BOX (hbox3), value_label, FALSE, FALSE, 0);

  value_entry = gtk_entry_new ();
  gtk_signal_connect (GTK_OBJECT (value_entry), "changed",
                      GTK_SIGNAL_FUNC (on_value_entry_changed),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (value_entry), "insert_text",
                      GTK_SIGNAL_FUNC (on_value_entry_insert_text),
                      NULL);
  gtk_entry_set_max_length( (GtkEntry *)value_entry, 11);
  gtk_widget_show (value_entry);
  gtk_box_pack_start (GTK_BOX (hbox3), value_entry, TRUE, TRUE, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox3, TRUE, TRUE, 0);

  label5 = gtk_label_new ("Surface Flags");
  gtk_widget_show (label5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label5);

  table3 = gtk_table_new (5, 4, FALSE);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (notebook1), table3);

  content_solidbutton = gtk_toggle_button_new_with_label ("Solid");
  gtk_signal_connect (GTK_OBJECT (content_solidbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_solidbutton_toggled),
                      NULL);
  gtk_widget_show (content_solidbutton);
  gtk_table_attach (GTK_TABLE (table3), content_solidbutton, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_solidbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_windowbutton = gtk_toggle_button_new_with_label ("Window");
  gtk_signal_connect (GTK_OBJECT (content_windowbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_windowbutton_toggled),
                      NULL);
  gtk_widget_show (content_windowbutton);
  gtk_table_attach (GTK_TABLE (table3), content_windowbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_windowbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_auxbutton = gtk_toggle_button_new_with_label ("Aux");
  gtk_signal_connect (GTK_OBJECT (content_auxbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_auxbutton_toggled),
                      NULL);
  gtk_widget_show (content_auxbutton);
  gtk_table_attach (GTK_TABLE (table3), content_auxbutton, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_auxbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_lavabutton = gtk_toggle_button_new_with_label ("Lava");
  gtk_signal_connect (GTK_OBJECT (content_lavabutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_lavabutton_toggled),
                      NULL);
  gtk_widget_show (content_lavabutton);
  gtk_table_attach (GTK_TABLE (table3), content_lavabutton, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_lavabutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_slimebutton = gtk_toggle_button_new_with_label ("Slime");
  gtk_signal_connect (GTK_OBJECT (content_slimebutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_slimebutton_toggled),
                      NULL);
  gtk_widget_show (content_slimebutton);
  gtk_table_attach (GTK_TABLE (table3), content_slimebutton, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_slimebutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_waterbutton = gtk_toggle_button_new_with_label ("Water");
  gtk_signal_connect (GTK_OBJECT (content_waterbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_waterbutton_toggled),
                      NULL);
  gtk_widget_show (content_waterbutton);
  gtk_table_attach (GTK_TABLE (table3), content_waterbutton, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_waterbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_mistbutton = gtk_toggle_button_new_with_label ("Mist");
  gtk_signal_connect (GTK_OBJECT (content_mistbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_mistbutton_toggled),
                      NULL);
  gtk_widget_show (content_mistbutton);
  gtk_table_attach (GTK_TABLE (table3), content_mistbutton, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_mistbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_areaportalbutton = gtk_toggle_button_new_with_label ("AreaPortal");
  gtk_signal_connect (GTK_OBJECT (content_areaportalbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_areaportalbutton_toggled),
                      NULL);
  gtk_widget_show (content_areaportalbutton);
  gtk_table_attach (GTK_TABLE (table3), content_areaportalbutton, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_areaportalbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_playerclipbutton = gtk_toggle_button_new_with_label ("PlayerClip");
  gtk_signal_connect (GTK_OBJECT (content_playerclipbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_playerclipbutton_toggled),
                      NULL);
  gtk_widget_show (content_playerclipbutton);
  gtk_table_attach (GTK_TABLE (table3), content_playerclipbutton, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_playerclipbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_monsterclipbutton = gtk_toggle_button_new_with_label ("MonsterClip");
  gtk_signal_connect (GTK_OBJECT (content_monsterclipbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_monsterclipbutton_toggled),
                      NULL);
  gtk_widget_show (content_monsterclipbutton);
  gtk_table_attach (GTK_TABLE (table3), content_monsterclipbutton, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_monsterclipbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_current0button = gtk_toggle_button_new_with_label ("Current 0");
  gtk_signal_connect (GTK_OBJECT (content_current0button), "toggled",
                      GTK_SIGNAL_FUNC (on_content_current0button_toggled),
                      NULL);
  gtk_widget_show (content_current0button);
  gtk_table_attach (GTK_TABLE (table3), content_current0button, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_current0button), QUAKE2_FLAG_BUTTON_BORDER);

  content_current90button = gtk_toggle_button_new_with_label ("Current 90");
  gtk_signal_connect (GTK_OBJECT (content_current90button), "toggled",
                      GTK_SIGNAL_FUNC (on_content_current90button_toggled),
                      NULL);
  gtk_widget_show (content_current90button);
  gtk_table_attach (GTK_TABLE (table3), content_current90button, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_current90button), QUAKE2_FLAG_BUTTON_BORDER);

  content_current180button = gtk_toggle_button_new_with_label ("Current 180");
  gtk_signal_connect (GTK_OBJECT (content_current180button), "toggled",
                      GTK_SIGNAL_FUNC (on_content_current180button_toggled),
                      NULL);
  gtk_widget_show (content_current180button);
  gtk_table_attach (GTK_TABLE (table3), content_current180button, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_current180button), QUAKE2_FLAG_BUTTON_BORDER);

  content_current270button = gtk_toggle_button_new_with_label ("Current 270");
  gtk_signal_connect (GTK_OBJECT (content_current270button), "toggled",
                      GTK_SIGNAL_FUNC (on_content_current270button_toggled),
                      NULL);
  gtk_widget_show (content_current270button);
  gtk_table_attach (GTK_TABLE (table3), content_current270button, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_current270button), QUAKE2_FLAG_BUTTON_BORDER);

  content_currentUPbutton = gtk_toggle_button_new_with_label ("Current UP");
  gtk_signal_connect (GTK_OBJECT (content_currentUPbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_currentUPbutton_toggled),
                      NULL);
  gtk_widget_show (content_currentUPbutton);
  gtk_table_attach (GTK_TABLE (table3), content_currentUPbutton, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_currentUPbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_currentDOWNbutton = gtk_toggle_button_new_with_label ("Current DOWN");
  gtk_signal_connect (GTK_OBJECT (content_currentDOWNbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_currentDOWNbutton_toggled),
                      NULL);
  gtk_widget_show (content_currentDOWNbutton);
  gtk_table_attach (GTK_TABLE (table3), content_currentDOWNbutton, 3, 4, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_currentDOWNbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_originbutton = gtk_toggle_button_new_with_label ("Origin");
  gtk_signal_connect (GTK_OBJECT (content_originbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_originbutton_toggled),
                      NULL);
  gtk_widget_show (content_originbutton);
  gtk_table_attach (GTK_TABLE (table3), content_originbutton, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_originbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_detailbutton = gtk_toggle_button_new_with_label ("Detail");
  gtk_signal_connect (GTK_OBJECT (content_detailbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_detailbutton_toggled),
                      NULL);
  gtk_widget_show (content_detailbutton);
  gtk_table_attach (GTK_TABLE (table3), content_detailbutton, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_detailbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_translucentbutton = gtk_toggle_button_new_with_label ("Translucent");
  gtk_signal_connect (GTK_OBJECT (content_translucentbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_translucentbutton_toggled),
                      NULL);
  gtk_widget_show (content_translucentbutton);
  gtk_table_attach (GTK_TABLE (table3), content_translucentbutton, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_translucentbutton), QUAKE2_FLAG_BUTTON_BORDER);

  content_ladderbutton = gtk_toggle_button_new_with_label ("Ladder");
  gtk_signal_connect (GTK_OBJECT (content_ladderbutton), "toggled",
                      GTK_SIGNAL_FUNC (on_content_ladderbutton_toggled),
                      NULL);
  gtk_widget_show (content_ladderbutton);
  gtk_table_attach (GTK_TABLE (table3), content_ladderbutton, 3, 4, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (content_ladderbutton), QUAKE2_FLAG_BUTTON_BORDER);

  label6 = gtk_label_new ("Content Flags");
  gtk_widget_show (label6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label6);

  return frame1;
}

