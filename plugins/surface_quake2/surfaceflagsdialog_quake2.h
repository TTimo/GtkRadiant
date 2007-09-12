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

#ifndef _SURFACEFLAGSDIALOG_QUAKE2_H
  #define _SURFACEFLAGSDIALOG_QUAKE2_H


#define QUAKE2_SURF_LIGHT              0x1
#define QUAKE2_SURF_SLICK              0x2
#define QUAKE2_SURF_SKY                0x4
#define QUAKE2_SURF_WARP               0x8
#define QUAKE2_SURF_TRANS33    	0x10
#define QUAKE2_SURF_TRANS66    	0x20
#define QUAKE2_SURF_FLOWING    	0x40
#define QUAKE2_SURF_NODRAW             0x80
#define QUAKE2_SURF_HINT               0x100
#define QUAKE2_SURF_SKIP               0x200


#define QUAKE2_CONTENTS_SOLID		0x1
#define QUAKE2_CONTENTS_WINDOW		0x2
#define QUAKE2_CONTENTS_AUX		0x4
#define QUAKE2_CONTENTS_LAVA		0x8
#define QUAKE2_CONTENTS_SLIME		0x10
#define QUAKE2_CONTENTS_WATER		0x20
#define QUAKE2_CONTENTS_MIST		0x40

#define QUAKE2_CONTENTS_AREAPORTAL	0x8000
#define QUAKE2_CONTENTS_PLAYERCLIP	0x10000
#define QUAKE2_CONTENTS_MONSTERCLIP    0x20000
#define QUAKE2_CONTENTS_CURRENT_0	0x40000
#define QUAKE2_CONTENTS_CURRENT_90	0x80000
#define QUAKE2_CONTENTS_CURRENT_180	0x100000
#define QUAKE2_CONTENTS_CURRENT_270	0x200000
#define QUAKE2_CONTENTS_CURRENT_UP	0x400000
#define QUAKE2_CONTENTS_CURRENT_DOWN	0x800000
#define QUAKE2_CONTENTS_ORIGIN		0x1000000

#define QUAKE2_CONTENTS_DETAIL		0x8000000
#define QUAKE2_CONTENTS_TRANSLUCENT	0x10000000
#define QUAKE2_CONTENTS_LADDER		0x20000000
/*
extern GtkWidget *notebook1;

extern  GtkWidget *surface_lightbutton;
extern  GtkWidget *surface_slickbutton;
extern  GtkWidget *surface_skybutton;
extern  GtkWidget *surface_warpbutton;
extern  GtkWidget *surface_trans33button;
extern  GtkWidget *surface_trans66button;
extern  GtkWidget *surface_flowingbutton;
extern  GtkWidget *surface_nodrawbutton;
extern  GtkWidget *surface_hintbutton;
extern  GtkWidget *surface_skipbutton;

extern GtkWidget *content_solidbutton;
extern GtkWidget *content_windowbutton;
extern GtkWidget *content_auxbutton;
extern GtkWidget *content_lavabutton;
extern GtkWidget *content_slimebutton;
extern GtkWidget *content_waterbutton;
extern GtkWidget *content_mistbutton;
extern GtkWidget *content_areaportalbutton;
extern GtkWidget *content_playerclipbutton;
extern GtkWidget *content_monsterclipbutton;
extern GtkWidget *content_current0button;
extern GtkWidget *content_current90button;
extern GtkWidget *content_current180button;
extern GtkWidget *content_current270button;
extern GtkWidget *content_currentUPbutton;
extern GtkWidget *content_currentDOWNbutton;
extern GtkWidget *content_originbutton;
extern GtkWidget *content_detailbutton;
extern GtkWidget *content_translucentbutton;
extern GtkWidget *content_ladderbutton;

extern  GtkWidget *surfacebutton;
extern  GtkWidget *contentbutton;

extern  GtkWidget *value_entry;
extern  gboolean setup_buttons;

extern  int working_surface_flags;
extern  int surface_mask;
extern  int working_content_flags;
extern  int content_mask;
extern  int working_value;
*/

#endif // _SURFACEFLAGSDIALOG_QUAKE2_H
