/*
Sunplug plugin for GtkRadiant
Copyright (C) 2004 Topsun
Thanks to SPoG for help!

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "sunplug.h"

#include "debugging/debugging.h"

#include "iplugin.h"

#include "string/string.h"
#include "modulesystem/singletonmodule.h"

#include "iundo.h"       // declaration of undo system
#include "ientity.h"     // declaration of entity system
#include "iscenegraph.h" // declaration of datastructure of the map

#include "scenelib.h"    // declaration of datastructure of the map
#include "qerplugin.h"   // declaration to use other interfaces as a plugin

#include <gtk/gtk.h>     // to display something with gtk (windows, buttons etc.), the whole package might not be necessary

void about_plugin_window();
void MapCoordinator();

#ifdef __linux__
// linux itoa implementation
char* itoa( int value, char* result, int base )
{	
	// check that the base if valid
	if (base < 2 || base > 16)
	{
	  *result = 0;
	  return result;
	}
	
	char* out = result;
	int quotient = value;
	
	do
	{
		*out = "0123456789abcdef"[abs(quotient % base)];
		++out;
	
		quotient /= base;
	} while (quotient);
	
	// Only apply negative sign for base 10
	if( value < 0 && base == 10)
	  *out++ = '-';
	
	std::reverse(result, out);
	
	*out = 0;
	return result;
}
#endif

typedef struct _mapcoord_setting_packet {
  GtkSpinButton *spinner1, *spinner2, *spinner3, *spinner4;
  Entity* worldspawn;
} mapcoord_setting_packet;

static int map_minX, map_maxX, map_minY, map_maxY;
static int minX, maxX, minY, maxY;
mapcoord_setting_packet msp;

//  **************************
// ** find entities by class **  from radiant/map.cpp
//  **************************
class EntityFindByClassname : public scene::Graph::Walker
{
  const char* m_name;
  Entity*& m_entity;
public:
  EntityFindByClassname(const char* name, Entity*& entity) : m_name(name), m_entity(entity)
  {
    m_entity = 0;
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(m_entity == 0)
    {
      Entity* entity = Node_getEntity(path.top());
      if(entity != 0
        && string_equal(m_name, entity->getKeyValue("classname")))
      {
        m_entity = entity;
      }
    }
    return true;
  }
};

Entity* Scene_FindEntityByClass(const char* name)
{
  Entity* entity;
  GlobalSceneGraph().traverse(EntityFindByClassname(name, entity));
  return entity;
}

//  **************************
// ** GTK callback functions **
//  **************************

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  /* If you return FALSE in the "delete_event" signal handler,
   * GTK will emit the "destroy" signal. Returning TRUE means
   * you don't want the window to be destroyed.
   * This is useful for popping up 'are you sure you want to quit?'
   * type dialogs. */

  return FALSE;
}

// destroy widget if destroy signal is passed to widget
static void destroy(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(widget);
}

// function for close button to destroy the toplevel widget
static void close_window(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(widget));
}

// callback function to assign the optimal mapcoords to the spinboxes
static void input_optimal(GtkWidget *widget, gpointer data)
{
  gtk_spin_button_set_value(msp.spinner1, minX);
  gtk_spin_button_set_value(msp.spinner2, maxY);
  gtk_spin_button_set_value(msp.spinner3, maxX);
  gtk_spin_button_set_value(msp.spinner4, minY);
}

// Spinner return value function
gint grab_int_value(GtkSpinButton *a_spinner, gpointer user_data) {
  return gtk_spin_button_get_value_as_int(a_spinner);
}

// write the values of the Spinner-Boxes to the worldspawn
static void set_coordinates(GtkWidget *widget, gpointer data)
{
  //Str str_min, str_max;
  char buffer[10], str_min[20], str_max[20];

  itoa(gtk_spin_button_get_value_as_int(msp.spinner1), str_min, 10);
  itoa(gtk_spin_button_get_value_as_int(msp.spinner2), buffer, 10);
  strcat(str_min, " ");
  strcat(str_min, buffer);
  msp.worldspawn->setKeyValue("mapcoordsmins", str_min);

  itoa(gtk_spin_button_get_value_as_int(msp.spinner3), str_max, 10);
  itoa(gtk_spin_button_get_value_as_int(msp.spinner4), buffer, 10);
  strcat(str_max, " ");
  strcat(str_max, buffer);
  UndoableCommand undo("SunPlug.entitySetMapcoords");
  msp.worldspawn->setKeyValue("mapcoordsmaxs", str_max);

  close_window(widget, NULL);
}

class SunPlugPluginDependencies :
  public GlobalRadiantModuleRef,    // basic class for all other module refs
  public GlobalUndoModuleRef,       // used to say radiant that something has changed and to undo that
  public GlobalSceneGraphModuleRef, // necessary to handle data in the mapfile (change, retrieve data)
  public GlobalEntityModuleRef      // to access and modify the entities
{
public:
  SunPlugPluginDependencies() :
    GlobalEntityModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("entities"))//,
  {
  }
};

//  *************************
// ** standard plugin stuff **
//  *************************
namespace SunPlug
{
  GtkWindow* main_window;
  char MenuList[100] = "";

  const char* init(void* hApp, void* pMainWidget)
  {
    main_window = GTK_WINDOW(pMainWidget);
    return "Initializing SunPlug for GTKRadiant";
  }
  const char* getName()
  {
    return "SunPlug"; // name that is shown in the menue
  }
  const char* getCommandList()
  {
    const char about[] = "About...";
    const char etMapCoordinator[] = ";ET-MapCoordinator";

    strcat(MenuList, about);
    if (strncmp(GlobalRadiant().getGameName(), "etmain", 6) == 0) strcat(MenuList, etMapCoordinator);
    return (const char*)MenuList;
  }
  const char* getCommandTitleList()
  {
    return "";
  }
  void dispatch(const char* command, float* vMin, float* vMax, bool bSingleBrush) // message processing
  {
    if(string_equal(command, "About..."))
    {
	  about_plugin_window();
	}
    if(string_equal(command, "ET-MapCoordinator"))
    {
	  MapCoordinator();
    }
  }
} // namespace

class SunPlugModule : public TypeSystemRef
{
  _QERPluginTable			m_plugin;
public:
  typedef _QERPluginTable	Type;
  STRING_CONSTANT(Name, "SunPlug");

  SunPlugModule()
  {
	m_plugin.m_pfnQERPlug_Init = &SunPlug::init;
	m_plugin.m_pfnQERPlug_GetName = &SunPlug::getName;
	m_plugin.m_pfnQERPlug_GetCommandList = &SunPlug::getCommandList;
	m_plugin.m_pfnQERPlug_GetCommandTitleList = &SunPlug::getCommandTitleList;
	m_plugin.m_pfnQERPlug_Dispatch = &SunPlug::dispatch;
  }
  _QERPluginTable* getTable()
  {
    return &m_plugin;
  }
};

typedef SingletonModule<SunPlugModule, SunPlugPluginDependencies> SingletonSunPlugModule;

SingletonSunPlugModule g_SunPlugModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);

  g_SunPlugModule.selfRegister();
}

//  ************
// ** my stuff **
//  ************

// About dialog
void about_plugin_window()
{
    GtkWidget *window, *vbox, *label, *button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // create a window
    gtk_window_set_transient_for(GTK_WINDOW(window), SunPlug::main_window); // make the window to stay in front of the main window
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL); // connect the delete event
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL); // connect the destroy event for the window
    gtk_window_set_title(GTK_WINDOW(window), "About SunPlug"); // set the title of the window for the window
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); // don't let the user resize the window
    gtk_window_set_modal(GTK_WINDOW(window), TRUE); // force the user not to do something with the other windows
    gtk_container_set_border_width(GTK_CONTAINER(window), 10); // set the border of the window

    vbox = gtk_vbox_new(FALSE, 10); // create a box to arrange new objects vertically
    gtk_container_add(GTK_CONTAINER(window), vbox); // add the box to the window

    label = gtk_label_new("SunPlug v1.0 for GtkRadiant 1.5\nby Topsun"); // create a label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // text align left
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2); // insert the label in the box

    button = gtk_button_new_with_label("OK"); // create a button with text
    g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK (gtk_widget_destroy), window); // connect the click event to close the window
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2); // insert the button in the box

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // center the window on screen

    gtk_widget_show_all(window); // show the window and all subelements
}

// get the current bounding box and return the optimal coordinates
void GetOptimalCoordinates(AABB *levelBoundingBox)
{
  int half_width, half_heigth, center_x, center_y;

  half_width = levelBoundingBox->extents.x();
  half_heigth = levelBoundingBox->extents.y();
  center_x = levelBoundingBox->origin.x();
  center_y = levelBoundingBox->origin.y();

  if (half_width > 175 || half_heigth > 175) // the square must be at least 350x350 units
  {
    // the wider side is the indicator for the square
    if (half_width >= half_heigth)
    {
	  minX = center_x - half_width;
      maxX = center_x + half_width;
      minY = center_y - half_width;
      maxY = center_y + half_width;
    } else {
      minX = center_x - half_heigth;
      maxX = center_x + half_heigth;
	  minY = center_y - half_heigth;
      maxY = center_y + half_heigth;
    }
  } else {
    minX = center_x - 175;
    maxX = center_x + 175;
    minY = center_y - 175;
    maxY = center_y + 175;
  }
}

// MapCoordinator dialog window
void MapCoordinator()
{
  GtkWidget *window, *vbox, *table, *label, *spinnerMinX, *spinnerMinY, *spinnerMaxX, *spinnerMaxY, *button;
  GtkAdjustment *spinner_adj_MinX, *spinner_adj_MinY, *spinner_adj_MaxX, *spinner_adj_MaxY;
  Entity *theWorldspawn = NULL;
  const char *buffer;
  char line[20];

  // in any case we need a window to show the user what to do
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // create the window
  gtk_window_set_transient_for(GTK_WINDOW(window), SunPlug::main_window); // make the window to stay in front of the main window
  g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL); // connect the delete event for the window
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL); // connect the destroy event for the window
  gtk_window_set_title(GTK_WINDOW(window), "ET-MapCoordinator"); // set the title of the window for the window
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE); // don't let the user resize the window
  gtk_window_set_modal(GTK_WINDOW(window), TRUE); // force the user not to do something with the other windows
  gtk_container_set_border_width(GTK_CONTAINER(window), 10); // set the border of the window

  vbox = gtk_vbox_new(FALSE, 10); // create a box to arrange new objects vertically
  gtk_container_add(GTK_CONTAINER(window), vbox); // add the box to the window

  scene::Path path = makeReference(GlobalSceneGraph().root()); // get the path to the root element of the graph
  scene::Instance* instance = GlobalSceneGraph().find(path); // find the instance to the given path
  AABB levelBoundingBox = instance->worldAABB(); // get the bounding box of the level

  theWorldspawn = Scene_FindEntityByClass("worldspawn"); // find the entity worldspawn
  if (theWorldspawn != 0) { // need to have a worldspawn otherwise setting a value crashes the radiant
    // next two if's: get the current values of the mapcoords
    buffer = theWorldspawn->getKeyValue("mapcoordsmins"); // upper left corner
	if (strlen(buffer) > 0) {
      strncpy(line, buffer, 19);
      map_minX = atoi(strtok(line, " ")); // minimum of x value
      map_minY = atoi(strtok(NULL, " ")); // maximum of y value
	} else {
		map_minX = 0;
		map_minY = 0;
	}
    buffer = theWorldspawn->getKeyValue("mapcoordsmaxs"); // lower right corner
	if (strlen(buffer) > 0) {
	  strncpy(line, buffer, 19);
      map_maxX = atoi(strtok(line, " ")); // maximum of x value
      map_maxY = atoi(strtok(NULL, " ")); // minimum of y value
	} else {
		map_maxX = 0;
		map_maxY = 0;
	}

    globalOutputStream() << "SunPlug: calculating optimal coordinates\n"; // write to console that we are calculating the coordinates
    GetOptimalCoordinates(&levelBoundingBox); // calculate optimal mapcoords with the dimensions of the level bounding box
    globalOutputStream() << "SunPlug: adviced mapcoordsmins=" << minX << " " << maxY << "\n"; // console info about mapcoordsmins
    globalOutputStream() << "SunPlug: adviced mapcoordsmaxs=" << maxX << " " << minY << "\n"; // console info about mapcoordsmaxs

    spinner_adj_MinX = (GtkAdjustment *)gtk_adjustment_new(map_minX, -65536.0, 65536.0, 1.0, 5.0, 5.0); // create adjustment for value and range of minimum x value
    spinner_adj_MinY = (GtkAdjustment *)gtk_adjustment_new(map_minY, -65536.0, 65536.0, 1.0, 5.0, 5.0); // create adjustment for value and range of minimum y value
    spinner_adj_MaxX = (GtkAdjustment *)gtk_adjustment_new(map_maxX, -65536.0, 65536.0, 1.0, 5.0, 5.0); // create adjustment for value and range of maximum x value
    spinner_adj_MaxY = (GtkAdjustment *)gtk_adjustment_new(map_maxY, -65536.0, 65536.0, 1.0, 5.0, 5.0); // create adjustment for value and range of maximum y value
 
    button = gtk_button_new_with_label("Get optimal mapcoords"); // create button with text
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(input_optimal), NULL); // connect button with callback function
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2); // insert button into vbox

    gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 2); // insert separator into vbox

    table = gtk_table_new(4, 3, TRUE); // create table
    gtk_table_set_row_spacings(GTK_TABLE(table), 8); // set row spacings
    gtk_table_set_col_spacings(GTK_TABLE(table), 8); // set column spacings
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2); // insert table into vbox

    label = gtk_label_new("x"); // create label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // align text to the left side
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1, 2, 0, 1); // insert label into table

    label = gtk_label_new("y"); // create label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // align text to the left side
    gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, 0, 1); // insert label into table

    label = gtk_label_new("mapcoordsmins"); // create label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // align text to the left side
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2); // insert label into table

    spinnerMinX = gtk_spin_button_new(spinner_adj_MinX, 1.0, 0); // create textbox wiht value spin, value and value range
    gtk_table_attach_defaults(GTK_TABLE(table), spinnerMinX, 1, 2, 1, 2); // insert spinbox into table

    spinnerMinY = gtk_spin_button_new(spinner_adj_MinY, 1.0, 0); // create textbox wiht value spin, value and value range
    gtk_table_attach_defaults(GTK_TABLE(table), spinnerMinY, 2, 3, 1, 2); // insert spinbox into table

    label = gtk_label_new("mapcoordsmaxs"); // create label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // align text to the left side
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3); // insert label into table

    spinnerMaxX = gtk_spin_button_new(spinner_adj_MaxX, 1.0, 0); // create textbox wiht value spin, value and value range
    gtk_table_attach_defaults(GTK_TABLE(table), spinnerMaxX, 1, 2, 2, 3); // insert spinbox into table

    spinnerMaxY = gtk_spin_button_new(spinner_adj_MaxY, 1.0, 0); // create textbox wiht value spin, value and value range
    gtk_table_attach_defaults(GTK_TABLE(table), spinnerMaxY, 2, 3, 2, 3); // insert spinbox into table

    // put the references to the spinboxes and the worldspawn into the global exchange
    msp.spinner1 = GTK_SPIN_BUTTON(spinnerMinX);
    msp.spinner2 = GTK_SPIN_BUTTON(spinnerMinY);
    msp.spinner3 = GTK_SPIN_BUTTON(spinnerMaxX);
    msp.spinner4 = GTK_SPIN_BUTTON(spinnerMaxY);
    msp.worldspawn = theWorldspawn;

    button = gtk_button_new_with_label("Set"); // create button with text
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(set_coordinates), NULL); // connect button with callback function
    gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 3, 4); // insert button into table

    button = gtk_button_new_with_label("Cancel"); // create button with text
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_window), NULL); // connect button with callback function
    gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 3, 3, 4); // insert button into table
  } else {
	globalOutputStream() << "SunPlug: no worldspawn found!\n"; // output error to console
	
	label = gtk_label_new("ERROR: No worldspawn was found in the map!\nIn order to use this tool the map must have at least one brush in the worldspawn. "); // create a label
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT); // text align left
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2); // insert the label in the box

    button = gtk_button_new_with_label("OK"); // create a button with text
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_window), NULL); // connect the click event to close the window
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2); // insert the button in the box
  }

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // center the window
  gtk_widget_show_all(window); // show the window and all subelements
}