/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#include "multimon.h"

#include "debugging/debugging.h"

#include "gtkutil/window.h"
#include "preferences.h"


multimon_globals_t g_multimon_globals;

LatchedBool g_Multimon_enableSysMenuPopups(false, "Floating windows sysmenu icons");

void MultiMonitor_constructPreferences(PreferencesPage& page)
{
  GtkWidget* primary_monitor = page.appendCheckBox("Multi Monitor", "Start on Primary Monitor", g_multimon_globals.m_bStartOnPrimMon);
  GtkWidget* popup = page.appendCheckBox(
    "", "Disable system menu on popup windows",
    LatchedBoolImportCaller(g_Multimon_enableSysMenuPopups),
    BoolExportCaller(g_Multimon_enableSysMenuPopups.m_latched)
  );
  Widget_connectToggleDependency(popup, primary_monitor);
}

#include "preferencesystem.h"
#include "stringio.h"

#include <gdk/gdkdisplay.h>

namespace
{
	GdkRectangle primaryMonitor;
}

void PositionWindowOnPrimaryScreen(WindowPosition& position)
{
  if( position.w >= primaryMonitor.width - 12 )
  {
    position.w = primaryMonitor.width - 12;
  }
  if( position.h >= primaryMonitor.height - 24 )
  {
    position.h = primaryMonitor.height - 48;
  }
  if( position.x <= primaryMonitor.x || position.x + position.w >= (primaryMonitor.x + primaryMonitor.width) - 12 )
  {
    position.x = primaryMonitor.x + 6;
  }
  if( position.y <= primaryMonitor.y || position.y + position.h >= (primaryMonitor.y + primaryMonitor.height) - 48 )
  {
    position.y = primaryMonitor.y + 24;
  }
}

void MultiMon_Construct()
{
  // detect multiple monitors

  GdkScreen* screen = gdk_display_get_default_screen(gdk_display_get_default());
  gint m = gdk_screen_get_n_monitors(screen);
  globalOutputStream() << "default screen has " << m << " monitors\n";
  for(int j = 0; j != m; ++j)
  {
    GdkRectangle geom;
    gdk_screen_get_monitor_geometry(screen, j, &geom);
    globalOutputStream() << "monitor " << j << " geometry: " << geom.x << ", " << geom.y << ", " << geom.width << ", " << geom.height << "\n";
    if(j == 0)
    {
      // I am making the assumption that monitor 0 is always the primary monitor on win32. Tested on WinXP with gtk+-2.4.
      primaryMonitor = geom;
    }
  }

  if(m > 1)
  {
    g_multimon_globals.m_bStartOnPrimMon = true;
  }

  GlobalPreferenceSystem().registerPreference("StartOnPrimMon", BoolImportStringCaller(g_multimon_globals.m_bStartOnPrimMon), BoolExportStringCaller(g_multimon_globals.m_bStartOnPrimMon));
  GlobalPreferenceSystem().registerPreference("NoSysMenuPopups", BoolImportStringCaller(g_Multimon_enableSysMenuPopups.m_latched), BoolExportStringCaller(g_Multimon_enableSysMenuPopups.m_latched));

  g_Multimon_enableSysMenuPopups.useLatched();

  PreferencesDialog_addInterfacePreferences(FreeCaller1<PreferencesPage&, MultiMonitor_constructPreferences>());
}
void MultiMon_Destroy()
{
}
