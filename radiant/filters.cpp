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

#include "filters.h"

#include "debugging/debugging.h"

#include "ifilter.h"

#include "scenelib.h"

#include <list>
#include <set>

#include "gtkutil/widget.h"
#include "gtkutil/menu.h"
#include "gtkmisc.h"
#include "mainframe.h"
#include "commands.h"
#include "preferences.h"

struct filters_globals_t
{
  std::size_t exclude;

  filters_globals_t() :
    exclude(0)
  {
  }
};

filters_globals_t g_filters_globals;

inline bool filter_active(int mask)
{
  return (g_filters_globals.exclude & mask) > 0;
}

class FilterWrapper
{
public:
  FilterWrapper(Filter& filter, int mask) : m_filter(filter), m_mask(mask)
  {
  }
  void update()
  {
    m_filter.setActive(filter_active(m_mask));
  }
private:
  Filter& m_filter;
  int m_mask;
};

typedef std::list<FilterWrapper> Filters;
Filters g_filters;

typedef std::set<Filterable*> Filterables;
Filterables g_filterables;

void UpdateFilters()
{
  {
    for(Filters::iterator i = g_filters.begin(); i != g_filters.end(); ++i)
    {
      (*i).update();
    }
  }

  {
    for(Filterables::iterator i = g_filterables.begin(); i != g_filterables.end(); ++i)
    {
      (*i)->updateFiltered();
    }
  }
}


class BasicFilterSystem : public FilterSystem
{
public:
  void addFilter(Filter& filter, int mask)
  {
    g_filters.push_back(FilterWrapper(filter, mask));
    g_filters.back().update();
  }
  void registerFilterable(Filterable& filterable)
  {
    ASSERT_MESSAGE(g_filterables.find(&filterable) == g_filterables.end(), "filterable already registered");
    filterable.updateFiltered();
    g_filterables.insert(&filterable);
  }
  void unregisterFilterable(Filterable& filterable)
  {
    ASSERT_MESSAGE(g_filterables.find(&filterable) != g_filterables.end(), "filterable not registered");
    g_filterables.erase(&filterable);
  }
};

BasicFilterSystem g_FilterSystem;

FilterSystem& GetFilterSystem()
{
  return g_FilterSystem;
}

void PerformFiltering()
{
  UpdateFilters();
  SceneChangeNotify();
}

class ToggleFilterFlag
{
  const unsigned int m_mask;
public:
  ToggleItem m_item;

  ToggleFilterFlag(unsigned int mask) : m_mask(mask), m_item(ActiveCaller(*this))
  {
  }
  ToggleFilterFlag(const ToggleFilterFlag& other) : m_mask(other.m_mask), m_item(ActiveCaller(*this))
  {
  }
  void active(const BoolImportCallback& importCallback)
  {
    importCallback((g_filters_globals.exclude & m_mask) != 0);
  }
  typedef MemberCaller1<ToggleFilterFlag, const BoolImportCallback&, &ToggleFilterFlag::active> ActiveCaller;
  void toggle()
  {
    g_filters_globals.exclude ^= m_mask;
    m_item.update();
    PerformFiltering();
  }
  typedef MemberCaller<ToggleFilterFlag, &ToggleFilterFlag::toggle> ToggleCaller;
};


typedef std::list<ToggleFilterFlag> ToggleFilterFlags;
ToggleFilterFlags g_filter_items;

void add_filter_command(unsigned int flag, const char* command, const Accelerator& accelerator)
{
  g_filter_items.push_back(ToggleFilterFlag(flag));
  GlobalToggles_insert(command, ToggleFilterFlag::ToggleCaller(g_filter_items.back()), ToggleItem::AddCallbackCaller(g_filter_items.back().m_item), accelerator);
}

void Filters_constructMenu(GtkMenu* menu_in_menu)
{
  create_check_menu_item_with_mnemonic(menu_in_menu, "World", "FilterWorldBrushes");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Entities", "FilterEntities");
  if(g_pGameDescription->mGameType == "doom3")
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Visportals", "FilterVisportals");
  }
  else
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Areaportals", "FilterAreaportals");
  }
  create_check_menu_item_with_mnemonic(menu_in_menu, "Translucent", "FilterTranslucent");
  if(g_pGameDescription->mGameType != "doom3")
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Liquids", "FilterLiquids");
  }
  create_check_menu_item_with_mnemonic(menu_in_menu, "Caulk", "FilterCaulk");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Clips", "FilterClips");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Paths", "FilterPaths");
  if(g_pGameDescription->mGameType != "doom3")
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Clusterportals", "FilterClusterportals");
  }
  create_check_menu_item_with_mnemonic(menu_in_menu, "Lights", "FilterLights");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Structural", "FilterStructural");
  if(g_pGameDescription->mGameType != "doom3")
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Lightgrid", "FilterLightgrid");
  }
  create_check_menu_item_with_mnemonic(menu_in_menu, "Patches", "FilterPatches");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Details", "FilterDetails");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Hints", "FilterHintsSkips");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Models", "FilterModels");
  create_check_menu_item_with_mnemonic(menu_in_menu, "Triggers", "FilterTriggers");
  if(g_pGameDescription->mGameType != "doom3")
  {
    create_check_menu_item_with_mnemonic(menu_in_menu, "Botclips", "FilterBotClips");
  }
}


#include "preferencesystem.h"
#include "stringio.h"

void ConstructFilters()
{
  GlobalPreferenceSystem().registerPreference("SI_Exclude", SizeImportStringCaller(g_filters_globals.exclude), SizeExportStringCaller(g_filters_globals.exclude));

  add_filter_command(EXCLUDE_WORLD, "FilterWorldBrushes", Accelerator('1', (GdkModifierType)GDK_MOD1_MASK));
  add_filter_command(EXCLUDE_ENT, "FilterEntities", Accelerator('2', (GdkModifierType)GDK_MOD1_MASK));
  if(g_pGameDescription->mGameType == "doom3")
  {
    add_filter_command(EXCLUDE_VISPORTALS, "FilterVisportals", Accelerator('3', (GdkModifierType)GDK_MOD1_MASK));
  }
  else
  {
    add_filter_command(EXCLUDE_AREAPORTALS, "FilterAreaportals", Accelerator('3', (GdkModifierType)GDK_MOD1_MASK));
  }
  add_filter_command(EXCLUDE_TRANSLUCENT, "FilterTranslucent", Accelerator('4', (GdkModifierType)GDK_MOD1_MASK));
  add_filter_command(EXCLUDE_LIQUIDS, "FilterLiquids", Accelerator('5', (GdkModifierType)GDK_MOD1_MASK));
  add_filter_command(EXCLUDE_CAULK, "FilterCaulk", Accelerator('6', (GdkModifierType)GDK_MOD1_MASK ));
  add_filter_command(EXCLUDE_CLIP, "FilterClips", Accelerator('7', (GdkModifierType)GDK_MOD1_MASK));
  add_filter_command(EXCLUDE_PATHS, "FilterPaths", Accelerator('8', (GdkModifierType)GDK_MOD1_MASK));
  if(g_pGameDescription->mGameType != "doom3")
  {
    add_filter_command(EXCLUDE_CLUSTERPORTALS, "FilterClusterportals", Accelerator('9', (GdkModifierType)GDK_MOD1_MASK));
  }
  add_filter_command(EXCLUDE_LIGHTS, "FilterLights", Accelerator('0', (GdkModifierType)GDK_MOD1_MASK));
  add_filter_command(EXCLUDE_STRUCTURAL, "FilterStructural", Accelerator('D', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  if(g_pGameDescription->mGameType != "doom3")
  {
    add_filter_command(EXCLUDE_LIGHTGRID, "FilterLightgrid", accelerator_null());
  }
  add_filter_command(EXCLUDE_CURVES, "FilterPatches", Accelerator('P', (GdkModifierType)GDK_CONTROL_MASK));
  add_filter_command(EXCLUDE_DETAILS, "FilterDetails", Accelerator('D', (GdkModifierType)GDK_CONTROL_MASK));
  add_filter_command(EXCLUDE_HINTSSKIPS, "FilterHintsSkips", Accelerator('H', (GdkModifierType)GDK_CONTROL_MASK));
  add_filter_command(EXCLUDE_MODELS, "FilterModels", Accelerator('M', (GdkModifierType)GDK_SHIFT_MASK));
  add_filter_command(EXCLUDE_TRIGGERS, "FilterTriggers", Accelerator('T', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  if(g_pGameDescription->mGameType != "doom3")
  {
    add_filter_command(EXCLUDE_BOTCLIP, "FilterBotClips", Accelerator('M', (GdkModifierType)GDK_MOD1_MASK));
  }

  PerformFiltering();
}

void DestroyFilters()
{
  g_filters.clear();
}

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class FilterAPI
{
  FilterSystem* m_filter;
public:
  typedef FilterSystem Type;
  STRING_CONSTANT(Name, "*");

  FilterAPI()
  {
    ConstructFilters();

    m_filter = &GetFilterSystem();
  }
  ~FilterAPI()
  {
    DestroyFilters();
  }
  FilterSystem* getTable()
  {
    return m_filter;
  }
};

typedef SingletonModule<FilterAPI> FilterModule;
typedef Static<FilterModule> StaticFilterModule;
StaticRegisterModule staticRegisterFilter(StaticFilterModule::instance());


