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

#include "grid.h"

#include <math.h>
#include <vector>
#include <algorithm>

#include "preferencesystem.h"

#include "gtkutil/widget.h"
#include "stringio.h"

#include "gtkmisc.h"
#include "commands.h"
#include "preferences.h"



std::vector<Callback> g_gridChange_callbacks;

void AddGridChangeCallback(const Callback& callback)
{
  g_gridChange_callbacks.push_back(callback);
  callback();
}

void GridChangeNotify()
{
  std::for_each(g_gridChange_callbacks.begin(), g_gridChange_callbacks.end(), CallbackInvoke());
}

enum GridPower
{
  GRIDPOWER_0125 = -3,
  GRIDPOWER_025 = -2,
  GRIDPOWER_05 = -1,
  GRIDPOWER_1 = 0,
  GRIDPOWER_2 = 1,
  GRIDPOWER_4 = 2,
  GRIDPOWER_8 = 3,
  GRIDPOWER_16 = 4,
  GRIDPOWER_32 = 5,
  GRIDPOWER_64 = 6,
  GRIDPOWER_128 = 7,
  GRIDPOWER_256 = 8,
};


typedef const char* GridName;
// this must match the GridPower enumeration
const GridName g_gridnames[] = {
  "0.125",
  "0.25",
  "0.5",
  "1",
  "2",
  "4",
  "8",
  "16",
  "32",
  "64",
  "128",
  "256",
};

inline GridPower GridPower_forGridDefault(int gridDefault)
{
  return static_cast<GridPower>(gridDefault - 3);
}

inline int GridDefault_forGridPower(GridPower gridPower)
{
  return gridPower + 3;
}

int g_grid_default = GridDefault_forGridPower(GRIDPOWER_8);

int g_grid_power = GridPower_forGridDefault(g_grid_default);

int Grid_getPower()
{
  return g_grid_power;
}

inline float GridSize_forGridPower(int gridPower)
{
  return pow(2.0f, gridPower);
}

float g_gridsize = GridSize_forGridPower(g_grid_power);

float GetGridSize()
{
  return g_gridsize;
}


void setGridPower(GridPower power);

class GridMenuItem
{
  GridPower m_id;

  GridMenuItem(const GridMenuItem& other); // NOT COPYABLE
  GridMenuItem& operator=(const GridMenuItem& other); // NOT ASSIGNABLE
public:
  ToggleItem m_item;

  GridMenuItem(GridPower id) : m_id(id), m_item(ExportCaller(*this))
  {
  }
  void set()
  {
    g_grid_power = m_id;
    m_item.update();
    setGridPower(m_id);
  }
  typedef MemberCaller<GridMenuItem, &GridMenuItem::set> SetCaller;
  void active(const BoolImportCallback& importCallback)
  {
    importCallback(g_grid_power == m_id);
  }
  typedef MemberCaller1<GridMenuItem, const BoolImportCallback&, &GridMenuItem::active> ExportCaller;
};

GridMenuItem g_gridMenu0125(GRIDPOWER_0125);
GridMenuItem g_gridMenu025(GRIDPOWER_025);
GridMenuItem g_gridMenu05(GRIDPOWER_05);
GridMenuItem g_gridMenu1(GRIDPOWER_1);
GridMenuItem g_gridMenu2(GRIDPOWER_2);
GridMenuItem g_gridMenu4(GRIDPOWER_4);
GridMenuItem g_gridMenu8(GRIDPOWER_8);
GridMenuItem g_gridMenu16(GRIDPOWER_16);
GridMenuItem g_gridMenu32(GRIDPOWER_32);
GridMenuItem g_gridMenu64(GRIDPOWER_64);
GridMenuItem g_gridMenu128(GRIDPOWER_128);
GridMenuItem g_gridMenu256(GRIDPOWER_256);

void setGridPower(GridPower power)
{
  g_gridsize = GridSize_forGridPower(power);

  g_gridMenu0125.m_item.update();
  g_gridMenu025.m_item.update();
  g_gridMenu05.m_item.update();
  g_gridMenu1.m_item.update();
  g_gridMenu2.m_item.update();
  g_gridMenu4.m_item.update();
  g_gridMenu8.m_item.update();
  g_gridMenu16.m_item.update();
  g_gridMenu32.m_item.update();
  g_gridMenu64.m_item.update();
  g_gridMenu128.m_item.update();
  g_gridMenu256.m_item.update();
  GridChangeNotify();
}

void GridPrev()
{
  if(g_grid_power > GRIDPOWER_0125)
  {
    setGridPower(static_cast<GridPower>(--g_grid_power));
  }
}

void GridNext()
{
  if(g_grid_power < GRIDPOWER_256)
  {
    setGridPower(static_cast<GridPower>(++g_grid_power));
  }
}


void Grid_registerCommands()
{
  GlobalCommands_insert("GridDown", FreeCaller<GridPrev>(), Accelerator('['));
  GlobalCommands_insert("GridUp", FreeCaller<GridNext>(), Accelerator(']'));

  GlobalToggles_insert("SetGrid0.125", GridMenuItem::SetCaller(g_gridMenu0125), ToggleItem::AddCallbackCaller(g_gridMenu0125.m_item));
  GlobalToggles_insert("SetGrid0.25", GridMenuItem::SetCaller(g_gridMenu025), ToggleItem::AddCallbackCaller(g_gridMenu025.m_item));
  GlobalToggles_insert("SetGrid0.5", GridMenuItem::SetCaller(g_gridMenu05), ToggleItem::AddCallbackCaller(g_gridMenu05.m_item));
  GlobalToggles_insert("SetGrid1", GridMenuItem::SetCaller(g_gridMenu1), ToggleItem::AddCallbackCaller(g_gridMenu1.m_item), Accelerator('1'));
  GlobalToggles_insert("SetGrid2", GridMenuItem::SetCaller(g_gridMenu2), ToggleItem::AddCallbackCaller(g_gridMenu2.m_item), Accelerator('2'));
  GlobalToggles_insert("SetGrid4", GridMenuItem::SetCaller(g_gridMenu4), ToggleItem::AddCallbackCaller(g_gridMenu4.m_item), Accelerator('3'));
  GlobalToggles_insert("SetGrid8", GridMenuItem::SetCaller(g_gridMenu8), ToggleItem::AddCallbackCaller(g_gridMenu8.m_item), Accelerator('4'));
  GlobalToggles_insert("SetGrid16", GridMenuItem::SetCaller(g_gridMenu16), ToggleItem::AddCallbackCaller(g_gridMenu16.m_item), Accelerator('5'));
  GlobalToggles_insert("SetGrid32", GridMenuItem::SetCaller(g_gridMenu32), ToggleItem::AddCallbackCaller(g_gridMenu32.m_item), Accelerator('6'));
  GlobalToggles_insert("SetGrid64", GridMenuItem::SetCaller(g_gridMenu64), ToggleItem::AddCallbackCaller(g_gridMenu64.m_item), Accelerator('7'));
  GlobalToggles_insert("SetGrid128", GridMenuItem::SetCaller(g_gridMenu128), ToggleItem::AddCallbackCaller(g_gridMenu128.m_item), Accelerator('8'));
  GlobalToggles_insert("SetGrid256", GridMenuItem::SetCaller(g_gridMenu256), ToggleItem::AddCallbackCaller(g_gridMenu256.m_item), Accelerator('9'));
}


void Grid_constructMenu(GtkMenu* menu)
{
  create_check_menu_item_with_mnemonic(menu, "Grid0.125", "SetGrid0.125");
  create_check_menu_item_with_mnemonic(menu, "Grid0.25", "SetGrid0.25");
  create_check_menu_item_with_mnemonic(menu, "Grid0.5", "SetGrid0.5");
  create_check_menu_item_with_mnemonic(menu, "Grid1", "SetGrid1");
  create_check_menu_item_with_mnemonic(menu, "Grid2", "SetGrid2");
  create_check_menu_item_with_mnemonic(menu, "Grid4", "SetGrid4");
  create_check_menu_item_with_mnemonic(menu, "Grid8", "SetGrid8");
  create_check_menu_item_with_mnemonic(menu, "Grid16", "SetGrid16");
  create_check_menu_item_with_mnemonic(menu, "Grid32", "SetGrid32");
  create_check_menu_item_with_mnemonic(menu, "Grid64", "SetGrid64");
  create_check_menu_item_with_mnemonic(menu, "Grid128", "SetGrid128");
  create_check_menu_item_with_mnemonic(menu, "Grid256", "SetGrid256");
}

void Grid_registerShortcuts()
{
  command_connect_accelerator("ToggleGrid");
  command_connect_accelerator("GridDown");
  command_connect_accelerator("GridUp");
}

void Grid_constructPreferences(PreferencesPage& page)
{
  page.appendCombo(
    "Default grid spacing",
    g_grid_default,
    ARRAY_RANGE(g_gridnames)
  );
}
void Grid_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Grid", "Grid Settings"));
  Grid_constructPreferences(page);
}
void Grid_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Grid_constructPage>());
}

void Grid_construct()
{
  Grid_registerPreferencesPage();

  g_grid_default = GridDefault_forGridPower(GRIDPOWER_8);

  GlobalPreferenceSystem().registerPreference("GridDefault", IntImportStringCaller(g_grid_default), IntExportStringCaller(g_grid_default));

  g_grid_power = GridPower_forGridDefault(g_grid_default);
  g_gridsize = GridSize_forGridPower(g_grid_power);
}

void Grid_destroy()
{
}
