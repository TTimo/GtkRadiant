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

#include "entity.h"

#include "ientity.h"
#include "iselection.h"
#include "imodel.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "editable.h"

#include "eclasslib.h"
#include "scenelib.h"
#include "os/path.h"
#include "os/file.h"
#include "stream/stringstream.h"
#include "stringio.h"

#include "gtkutil/filechooser.h"
#include "gtkmisc.h"
#include "select.h"
#include "map.h"
#include "preferences.h"
#include "gtkdlgs.h"
#include "mainframe.h"
#include "qe3.h"
#include "commands.h"

struct entity_globals_t
{
  Vector3 color_entity;

  entity_globals_t() :
    color_entity(0.0f, 0.0f, 0.0f)
  {
  }
};

entity_globals_t g_entity_globals;

class EntitySetKeyValueSelected : public scene::Graph::Walker
{
  const char* m_key;
  const char* m_value;
public:
  EntitySetKeyValueSelected(const char* key, const char* value)
    : m_key(key), m_value(value)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    Entity* entity = Node_getEntity(path.top());
    if(entity != 0
      && (instance.childSelected() || Instance_getSelectable(instance)->isSelected()))
    {
      entity->setKeyValue(m_key, m_value);
    }
  }
};

class EntitySetClassnameSelected : public scene::Graph::Walker
{
  const char* m_classname;
public:
  EntitySetClassnameSelected(const char* classname)
    : m_classname(classname)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    Entity* entity = Node_getEntity(path.top());
    if(entity != 0
      && (instance.childSelected() || Instance_getSelectable(instance)->isSelected()))
    { 
      NodeSmartReference node(GlobalEntityCreator().createEntity(GlobalEntityClassManager().findOrInsert(m_classname, node_is_group(path.top()))));

      EntityCopyingVisitor visitor(*Node_getEntity(node));

      entity->forEachKeyValue(visitor);

      NodeSmartReference child(path.top().get());
      NodeSmartReference parent(path.parent().get());
      Node_getTraversable(parent)->erase(child);
      if(Node_getTraversable(child) != 0
        && Node_getTraversable(node) != 0
        && node_is_group(node))
      {
        parentBrushes(child, node);
      }
      Node_getTraversable(parent)->insert(node);
    }
  }
};

void Scene_EntitySetKeyValue_Selected(const char* key, const char* value)
{
  GlobalSceneGraph().traverse(EntitySetKeyValueSelected(key, value));
}

void Scene_EntitySetClassname_Selected(const char* classname)
{
  GlobalSceneGraph().traverse(EntitySetClassnameSelected(classname));
}


class EntityUngroupVisitor : public SelectionSystem::Visitor
{
  const scene::Path& m_parent;
public:
  EntityUngroupVisitor(const scene::Path& parent) : m_parent(parent)
  {
  }
  void visit(scene::Instance& instance) const
  {
    if(Node_getEntity(instance.path().top()) != 0
      && node_is_group(instance.path().top()))
    {
      if(m_parent.top().get_pointer() != instance.path().top().get_pointer())
      {
        parentBrushes(instance.path().top(), m_parent.top());
        Path_deleteTop(instance.path());
      }
    }
  }
};

void Entity_ungroupSelected()
{
  UndoableCommand undo("ungroupSelectedEntities");

  scene::Path world_path(makeReference(GlobalSceneGraph().root()));
  world_path.push(makeReference(Map_FindOrInsertWorldspawn(g_map)));

  GlobalSelectionSystem().foreachSelected(EntityUngroupVisitor(world_path));
}



void Entity_connectSelected()
{
  if(GlobalSelectionSystem().countSelected() == 2)
  {
    GlobalEntityCreator().connectEntities(
      GlobalSelectionSystem().penultimateSelected().path(),
      GlobalSelectionSystem().ultimateSelected().path()
    );
  }
  else
  {
    globalErrorStream() << "entityConnectSelected: exactly two instances must be selected\n";
  }
}

const float Doom3Light_defaultRadius = 300;

AABB Doom3Light_getBounds(const AABB& workzone)
{
  AABB aabb(workzone);

  if(aabb.extents[0] == 0)
  {
    aabb.extents[0] = Doom3Light_defaultRadius;
  }
  if(aabb.extents[1] == 0)
  {
    aabb.extents[1] = Doom3Light_defaultRadius;
  }
  if(aabb.extents[2] == 0)
  {
    aabb.extents[2] = Doom3Light_defaultRadius;
  }

  if(aabb_valid(aabb))
  {
    return aabb;
  }
  return AABB(Vector3(0, 0, 0), Vector3(64, 64, 64));
}

int g_iLastLightIntensity;

void Entity_createFromSelection(const char* name, const Vector3& origin)
{
#if 0
  if(string_equal_nocase(name, "worldspawn"))
  {
    gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()), "Can't create an entity with worldspawn.", "info");
    return;
  }
#endif

  EntityClass* entityClass = GlobalEntityClassManager().findOrInsert(name, true);

  bool isModel = string_equal_nocase(name, "misc_model")
    || string_equal_nocase(name, "misc_gamemodel")
    || string_equal_nocase(name, "model_static")
    || (GlobalSelectionSystem().countSelected() == 0 && string_equal_nocase(name, "func_static"));

  bool brushesSelected = Scene_countSelectedBrushes(GlobalSceneGraph()) != 0;

  if(!(entityClass->fixedsize || isModel) && !brushesSelected)
  {
    globalErrorStream() << "failed to create a group entity - no brushes are selected\n";
    return;
  }

  AABB workzone(aabb_for_minmax(Select_getWorkZone().d_work_min, Select_getWorkZone().d_work_max));


  NodeSmartReference node(GlobalEntityCreator().createEntity(entityClass));

  Node_getTraversable(GlobalSceneGraph().root())->insert(node);

  scene::Path entitypath(makeReference(GlobalSceneGraph().root()));
  entitypath.push(makeReference(node.get()));
  scene::Instance& instance = findInstance(entitypath);

  if(entityClass->fixedsize)
  {
    Select_Delete();
    
    Transformable* transform = Instance_getTransformable(instance);
    if(transform != 0)
    {
      transform->setType(TRANSFORM_PRIMITIVE);
      transform->setTranslation(origin);
      transform->freezeTransform();
    }

    GlobalSelectionSystem().setSelectedAll(false);

    Instance_setSelected(instance, true);
  }
  else
  {
    Scene_parentSelectedBrushesToEntity(GlobalSceneGraph(), node);
    Scene_forEachChildSelectable(SelectableSetSelected(true), instance.path());
  }

  // tweaking: when right clic dropping a light entity, ask for light value in a custom dialog box
  // see SF bug 105383

  if (g_pGameDescription->mGameType == "hl")
  {
    // FIXME - Hydra: really we need a combined light AND color dialog for halflife.
    if (string_equal_nocase(name, "light")
      || string_equal_nocase(name, "light_environment")
      || string_equal_nocase(name, "light_spot"))
    {
      int intensity = g_iLastLightIntensity;

      if (DoLightIntensityDlg (&intensity) == eIDOK)
      {
        g_iLastLightIntensity = intensity;
        char buf[30];
        sprintf( buf, "255 255 255 %d", intensity );
        Node_getEntity(node)->setKeyValue("_light", buf);
      }
    }
  }
  else if(string_equal_nocase(name, "light"))
  {
    if(g_pGameDescription->mGameType != "doom3")
    {
      int intensity = g_iLastLightIntensity;

      if (DoLightIntensityDlg (&intensity) == eIDOK)
      {
        g_iLastLightIntensity = intensity;
        char buf[10];
        sprintf( buf, "%d", intensity );
        Node_getEntity(node)->setKeyValue("light", buf);
      }
    }
    else if(brushesSelected) // use workzone to set light position/size for doom3 lights, if there are brushes selected
    {
      AABB bounds(Doom3Light_getBounds(workzone));
      StringOutputStream key(64);
      key << bounds.origin[0] << " " << bounds.origin[1] << " " << bounds.origin[2];
      Node_getEntity(node)->setKeyValue("origin", key.c_str());
      key.clear();
      key << bounds.extents[0] << " " << bounds.extents[1] << " " << bounds.extents[2];
      Node_getEntity(node)->setKeyValue("light_radius", key.c_str());
    }
  }

  if(isModel)
  {
    const char* model = misc_model_dialog(GTK_WIDGET(MainFrame_getWindow()));
    if(model != 0)
    {
      Node_getEntity(node)->setKeyValue("model", model);
    }
  }
}


bool DoNormalisedColor(Vector3& color)
{
  if(!color_dialog(GTK_WIDGET(MainFrame_getWindow()), color))
    return false;
  /* 
  ** scale colors so that at least one component is at 1.0F 
  */

  float largest = 0.0F;

  if ( color[0] > largest )
    largest = color[0];
  if ( color[1] > largest )
    largest = color[1];
  if ( color[2] > largest )
    largest = color[2];

  if ( largest == 0.0F )
  {
    color[0] = 1.0F;
    color[1] = 1.0F;
    color[2] = 1.0F;
  }
  else
  {
    float scaler = 1.0F / largest;

    color[0] *= scaler;
    color[1] *= scaler;
    color[2] *= scaler;
  }

  return true;
}

void Entity_setColour()
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    const scene::Path& path = GlobalSelectionSystem().ultimateSelected().path();
    Entity* entity = Node_getEntity(path.top());
    if(entity != 0)
    {
      const char* strColor = entity->getKeyValue("_color");
      if(!string_empty(strColor))
      {
        Vector3 rgb;
        if (string_parse_vector3(strColor, rgb))
        {
          g_entity_globals.color_entity = rgb;
        }
      }

      if(g_pGameDescription->mGameType == "doom3"
        ? color_dialog(GTK_WIDGET(MainFrame_getWindow()), g_entity_globals.color_entity)
        : DoNormalisedColor(g_entity_globals.color_entity))
      {
        char buffer[128];
        sprintf(buffer, "%g %g %g", g_entity_globals.color_entity[0],
                g_entity_globals.color_entity[1],
                g_entity_globals.color_entity[2]);

        Scene_EntitySetKeyValue_Selected("_color", buffer);
      }
    }
  }
}

const char* misc_model_dialog(GtkWidget* parent)
{
  StringOutputStream buffer(1024);

  buffer << g_qeglobals.m_userGamePath.c_str() << "models/";

  if(!file_readable(buffer.c_str()))
  {
    // just go to fsmain
    buffer.clear();
    buffer << g_qeglobals.m_userGamePath.c_str() << "/";
  }

  const char *filename = file_dialog (parent, TRUE, "Choose Model", buffer.c_str(), ModelLoader::Name());
  if (filename != 0)
  {
    // use VFS to get the correct relative path
    const char* relative = path_make_relative(filename, GlobalFileSystem().findRoot(filename));
    if(relative == filename)
    {
      globalOutputStream() << "WARNING: could not extract the relative path, using full path instead\n";
    }
    return relative;
  }
  return 0;
}

void LightRadiiImport(EntityCreator& self, bool value)
{
  self.setLightRadii(value);
}
typedef ReferenceCaller1<EntityCreator, bool, LightRadiiImport> LightRadiiImportCaller;

void LightRadiiExport(EntityCreator& self, const BoolImportCallback& importer)
{
  importer(self.getLightRadii());
}
typedef ReferenceCaller1<EntityCreator, const BoolImportCallback&, LightRadiiExport> LightRadiiExportCaller;

void Entity_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox(
    "Show", "Light Radii",
    LightRadiiImportCaller(GlobalEntityCreator()),
    LightRadiiExportCaller(GlobalEntityCreator())
  );
}
void Entity_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Entities", "Entity Display Preferences"));
  Entity_constructPreferences(page);
}
void Entity_registerPreferencesPage()
{
  PreferencesDialog_addDisplayPage(FreeCaller1<PreferenceGroup&, Entity_constructPage>());
}



void Entity_constructMenu(GtkMenu* menu)
{
  create_menu_item_with_mnemonic(menu, "_Ungroup", "UngroupSelection");
  create_menu_item_with_mnemonic(menu, "_Connect", "ConnectSelection");
  create_menu_item_with_mnemonic(menu, "_Select Color...", "EntityColor");
}



#include "preferencesystem.h"
#include "stringio.h"

void Entity_Construct()
{
  GlobalCommands_insert("EntityColor", FreeCaller<Entity_setColour>(), Accelerator('K'));
  GlobalCommands_insert("ConnectSelection", FreeCaller<Entity_connectSelected>(), Accelerator('K', (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("UngroupSelection", FreeCaller<Entity_ungroupSelected>());

  GlobalPreferenceSystem().registerPreference("SI_Colors5", Vector3ImportStringCaller(g_entity_globals.color_entity), Vector3ExportStringCaller(g_entity_globals.color_entity));
  GlobalPreferenceSystem().registerPreference("LastLightIntensity", IntImportStringCaller(g_iLastLightIntensity), IntExportStringCaller(g_iLastLightIntensity));

  Entity_registerPreferencesPage();
}

void Entity_Destroy()
{
}

