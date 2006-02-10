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

#include "entityinspector.h"

#include "debugging/debugging.h"

#include "ientity.h"
#include "ifilesystem.h"
#include "imodel.h"
#include "iscenegraph.h"
#include "iselection.h"
#include "iundo.h"

#include <map>
#include <set>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktextview.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkcombobox.h>


#include "os/path.h"
#include "eclasslib.h"
#include "scenelib.h"
#include "generic/callback.h"
#include "os/file.h"
#include "stream/stringstream.h"
#include "moduleobserver.h"
#include "convert.h"
#include "stringio.h"

#include "gtkutil/accelerator.h"
#include "gtkutil/dialog.h"
#include "gtkutil/filechooser.h"
#include "gtkutil/messagebox.h"
#include "gtkutil/nonmodal.h"
#include "gtkutil/button.h"
#include "gtkutil/entry.h"
#include "gtkutil/container.h"

#include "qe3.h"
#include "gtkmisc.h"
#include "gtkdlgs.h"
#include "entity.h"
#include "mainframe.h"
#include "textureentry.h"

GtkEntry* numeric_entry_new()
{
  GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
  gtk_widget_show(GTK_WIDGET(entry));
  gtk_widget_set_size_request(GTK_WIDGET(entry), 64, -1);
  return entry;
}

namespace
{
  typedef std::map<CopiedString, CopiedString> KeyValues;
  KeyValues g_selectedKeyValues;
  KeyValues g_selectedDefaultKeyValues;
}

const char* SelectedEntity_getValueForKey(const char* key)
{
  {
    KeyValues::const_iterator i = g_selectedKeyValues.find(key);
    if(i != g_selectedKeyValues.end())
    {
      return (*i).second.c_str();
    }
  }
  {
    KeyValues::const_iterator i = g_selectedDefaultKeyValues.find(key);
    if(i != g_selectedDefaultKeyValues.end())
    {
      return (*i).second.c_str();
    }
  }
  return "";
}

void Scene_EntitySetKeyValue_Selected_Undoable(const char* key, const char* value)
{
  StringOutputStream command(256);
  command << "entitySetKeyValue -key " << makeQuoted(key) << " -value " << makeQuoted(value);
  UndoableCommand undo(command.c_str());
  Scene_EntitySetKeyValue_Selected(key, value);
}

class EntityAttribute
{
public:
  virtual GtkWidget* getWidget() const = 0;
  virtual void update() = 0;
  virtual void release() = 0;
};

class BooleanAttribute : public EntityAttribute
{
  CopiedString m_key;
  GtkCheckButton* m_check;

  static gboolean toggled(GtkWidget *widget, BooleanAttribute* self)
  {
    self->apply();
    return FALSE;
  }
public:
  BooleanAttribute(const char* key) :
    m_key(key),
    m_check(0)
  {
    GtkCheckButton* check = GTK_CHECK_BUTTON(gtk_check_button_new());
    gtk_widget_show(GTK_WIDGET(check));

    m_check = check;

    guint handler = g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(toggled), this);
    g_object_set_data(G_OBJECT(check), "handler", gint_to_pointer(handler));

    update();
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_check);
  }
  void release()
  {
    delete this;
  }
  void apply()
  {
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_check)) ? "1" : "0");
  }
  typedef MemberCaller<BooleanAttribute, &BooleanAttribute::apply> ApplyCaller;

  void update()
  {
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    if(!string_empty(value))
    {
      toggle_button_set_active_no_signal(GTK_TOGGLE_BUTTON(m_check), atoi(value) != 0);
    }
    else
    {
      toggle_button_set_active_no_signal(GTK_TOGGLE_BUTTON(m_check), false);
    }
  }
  typedef MemberCaller<BooleanAttribute, &BooleanAttribute::update> UpdateCaller;
};


class StringAttribute : public EntityAttribute
{
  CopiedString m_key;
  GtkEntry* m_entry;
  NonModalEntry m_nonModal;
public:
  StringAttribute(const char* key) :
    m_key(key),
    m_entry(0),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
    gtk_widget_show(GTK_WIDGET(entry));
    gtk_widget_set_size_request(GTK_WIDGET(entry), 50, -1);

    m_entry = entry;
    m_nonModal.connect(m_entry);    
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_entry);
  }
  GtkEntry* getEntry() const
  {
    return m_entry;
  }

  void release()
  {
    delete this;
  }
  void apply()
  {
    StringOutputStream value(64);
    value << ConvertUTF8ToLocale(gtk_entry_get_text(m_entry));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), value.c_str());
  }
  typedef MemberCaller<StringAttribute, &StringAttribute::apply> ApplyCaller;

  void update()
  {
    StringOutputStream value(64);
    value << ConvertLocaleToUTF8(SelectedEntity_getValueForKey(m_key.c_str()));
    gtk_entry_set_text(m_entry, value.c_str());
  }
  typedef MemberCaller<StringAttribute, &StringAttribute::update> UpdateCaller;
};

class ShaderAttribute : public StringAttribute
{
public:
  ShaderAttribute(const char* key) : StringAttribute(key)
  {
    GlobalShaderEntryCompletion::instance().connect(StringAttribute::getEntry());
  }
};


class ModelAttribute : public EntityAttribute
{
  CopiedString m_key;
  BrowsedPathEntry m_entry;
  NonModalEntry m_nonModal;
public:
  ModelAttribute(const char* key) :
    m_key(key),
    m_entry(BrowseCaller(*this)),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    m_nonModal.connect(m_entry.m_entry.m_entry);
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_entry.m_entry.m_frame);
  }
  void apply()
  {
    StringOutputStream value(64);
    value << ConvertUTF8ToLocale(gtk_entry_get_text(GTK_ENTRY(m_entry.m_entry.m_entry)));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), value.c_str());
  }
  typedef MemberCaller<ModelAttribute, &ModelAttribute::apply> ApplyCaller;
  void update()
  {
    StringOutputStream value(64);
    value << ConvertLocaleToUTF8(SelectedEntity_getValueForKey(m_key.c_str()));
    gtk_entry_set_text(GTK_ENTRY(m_entry.m_entry.m_entry), value.c_str());
  }
  typedef MemberCaller<ModelAttribute, &ModelAttribute::update> UpdateCaller;
  void browse(const BrowsedPathEntry::SetPathCallback& setPath)
  {
    const char *filename = misc_model_dialog(GTK_WIDGET(m_entry.m_entry.m_frame));
    
    if(filename != 0)
    {
      setPath(filename);
      apply();
    }
  }
  typedef MemberCaller1<ModelAttribute, const BrowsedPathEntry::SetPathCallback&, &ModelAttribute::browse> BrowseCaller;
};

const char* browse_sound(GtkWidget* parent)
{
  StringOutputStream buffer(1024);

  buffer << g_qeglobals.m_userGamePath.c_str() << "sound/";

  if(!file_readable(buffer.c_str()))
  {
    // just go to fsmain
    buffer.clear();
    buffer << g_qeglobals.m_userGamePath.c_str() << "/";
  }

  const char* filename = file_dialog(parent, TRUE, "Open Wav File", buffer.c_str(), "sound");
  if(filename != 0)
  {
    const char* relative = path_make_relative(filename, GlobalFileSystem().findRoot(filename));
    if(relative == filename)
    {
      globalOutputStream() << "WARNING: could not extract the relative path, using full path instead\n";
    }
    return relative;
  }
  return filename;
}

class SoundAttribute : public EntityAttribute
{
  CopiedString m_key;
  BrowsedPathEntry m_entry;
  NonModalEntry m_nonModal;
public:
  SoundAttribute(const char* key) :
    m_key(key),
    m_entry(BrowseCaller(*this)),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    m_nonModal.connect(m_entry.m_entry.m_entry);
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_entry.m_entry.m_frame);
  }
  void apply()
  {
    StringOutputStream value(64);
    value << ConvertUTF8ToLocale(gtk_entry_get_text(GTK_ENTRY(m_entry.m_entry.m_entry)));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), value.c_str());
  }
  typedef MemberCaller<SoundAttribute, &SoundAttribute::apply> ApplyCaller;
  void update()
  {
    StringOutputStream value(64);
    value << ConvertLocaleToUTF8(SelectedEntity_getValueForKey(m_key.c_str()));
    gtk_entry_set_text(GTK_ENTRY(m_entry.m_entry.m_entry), value.c_str());
  }
  typedef MemberCaller<SoundAttribute, &SoundAttribute::update> UpdateCaller;
  void browse(const BrowsedPathEntry::SetPathCallback& setPath)
  {
    const char *filename = browse_sound(gtk_widget_get_toplevel(GTK_WIDGET(m_entry.m_entry.m_frame)));
    
    if(filename != 0)
    {
      setPath(filename);
      apply();
    }
  }
  typedef MemberCaller1<SoundAttribute, const BrowsedPathEntry::SetPathCallback&, &SoundAttribute::browse> BrowseCaller;
};

inline double angle_normalised(double angle)
{
  return float_mod(angle, 360.0);
}

class AngleAttribute : public EntityAttribute
{
  CopiedString m_key;
  GtkEntry* m_entry;
  NonModalEntry m_nonModal;
public:
  AngleAttribute(const char* key) :
    m_key(key),
    m_entry(0),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    GtkEntry* entry = numeric_entry_new();
    m_entry = entry;
    m_nonModal.connect(m_entry);
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_entry);
  }
  void apply()
  {
    StringOutputStream angle(32);
    angle << angle_normalised(entry_get_float(m_entry));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), angle.c_str());
  }
  typedef MemberCaller<AngleAttribute, &AngleAttribute::apply> ApplyCaller;

  void update()
  {
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    if(!string_empty(value))
    {
      StringOutputStream angle(32);
      angle << angle_normalised(atof(value));
      gtk_entry_set_text(m_entry, angle.c_str());
    }
    else
    {
      gtk_entry_set_text(m_entry, "0");
    }
  }
  typedef MemberCaller<AngleAttribute, &AngleAttribute::update> UpdateCaller;
};

namespace
{
  typedef const char* String;
  const String buttons[] = { "up", "down", "z-axis" };
}

class DirectionAttribute : public EntityAttribute
{
  CopiedString m_key;
  GtkEntry* m_entry;
  NonModalEntry m_nonModal;
  RadioHBox m_radio;
  NonModalRadio m_nonModalRadio;
  GtkHBox* m_hbox;
public:
  DirectionAttribute(const char* key) :
    m_key(key),
    m_entry(0),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this)),
    m_radio(RadioHBox_new(STRING_ARRAY_RANGE(buttons))),
    m_nonModalRadio(ApplyRadioCaller(*this))
  {
    GtkEntry* entry = numeric_entry_new();
    m_entry = entry;
    m_nonModal.connect(m_entry);

    m_nonModalRadio.connect(m_radio.m_radio);

    m_hbox = GTK_HBOX(gtk_hbox_new(FALSE, 4));
    gtk_widget_show(GTK_WIDGET(m_hbox));

    gtk_box_pack_start(GTK_BOX(m_hbox), GTK_WIDGET(m_radio.m_hbox), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(m_hbox), GTK_WIDGET(m_entry), TRUE, TRUE, 0);
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_hbox);
  }
  void apply()
  {
    StringOutputStream angle(32);
    angle << angle_normalised(entry_get_float(m_entry));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), angle.c_str());
  }
  typedef MemberCaller<DirectionAttribute, &DirectionAttribute::apply> ApplyCaller;

  void update()
  {
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    if(!string_empty(value))
    {
      float f = float(atof(value));
      if(f == -1)
      {
        gtk_widget_set_sensitive(GTK_WIDGET(m_entry), FALSE);
        radio_button_set_active_no_signal(m_radio.m_radio, 0);
        gtk_entry_set_text(m_entry, "");
      }
      else if(f == -2)
      {
        gtk_widget_set_sensitive(GTK_WIDGET(m_entry), FALSE);
        radio_button_set_active_no_signal(m_radio.m_radio, 1);
        gtk_entry_set_text(m_entry, "");
      }
      else
      {
        gtk_widget_set_sensitive(GTK_WIDGET(m_entry), TRUE);
        radio_button_set_active_no_signal(m_radio.m_radio, 2);
        StringOutputStream angle(32);
        angle << angle_normalised(f);
        gtk_entry_set_text(m_entry, angle.c_str());
      }
    }
    else
    {
      gtk_entry_set_text(m_entry, "0");
    }
  }
  typedef MemberCaller<DirectionAttribute, &DirectionAttribute::update> UpdateCaller;

  void applyRadio()
  {
    int index = radio_button_get_active(m_radio.m_radio);
    if(index == 0)
    {
      Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), "-1");
    }
    else if(index == 1)
    {
      Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), "-2");
    }
    else if(index == 2)
    {
      apply();
    }
  }
  typedef MemberCaller<DirectionAttribute, &DirectionAttribute::applyRadio> ApplyRadioCaller;
};


class AnglesEntry
{
public:
  GtkEntry* m_roll;
  GtkEntry* m_pitch;
  GtkEntry* m_yaw;
  AnglesEntry() : m_roll(0), m_pitch(0), m_yaw(0)
  {
  }
};

typedef BasicVector3<double> DoubleVector3;

class AnglesAttribute : public EntityAttribute
{
  CopiedString m_key;
  AnglesEntry m_angles;
  NonModalEntry m_nonModal;
  GtkBox* m_hbox;
public:
  AnglesAttribute(const char* key) :
    m_key(key),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    m_hbox = GTK_BOX(gtk_hbox_new(TRUE, 4));
    gtk_widget_show(GTK_WIDGET(m_hbox));
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_angles.m_pitch = entry;
      m_nonModal.connect(m_angles.m_pitch);
    }
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_angles.m_yaw = entry;
      m_nonModal.connect(m_angles.m_yaw);
    }
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_angles.m_roll = entry;
      m_nonModal.connect(m_angles.m_roll);
    }
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_hbox);
  }
  void apply()
  {
    StringOutputStream angles(64);
    angles << angle_normalised(entry_get_float(m_angles.m_pitch))
      << " " << angle_normalised(entry_get_float(m_angles.m_yaw))
      << " " << angle_normalised(entry_get_float(m_angles.m_roll));
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), angles.c_str());
  }
  typedef MemberCaller<AnglesAttribute, &AnglesAttribute::apply> ApplyCaller;

  void update()
  {
    StringOutputStream angle(32);
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    if(!string_empty(value))
    {
      DoubleVector3 pitch_yaw_roll;
      if(!string_parse_vector3(value, pitch_yaw_roll))
      {
        pitch_yaw_roll = DoubleVector3(0, 0, 0);
      }

      angle << angle_normalised(pitch_yaw_roll.x());
      gtk_entry_set_text(m_angles.m_pitch, angle.c_str());
      angle.clear();

      angle << angle_normalised(pitch_yaw_roll.y());
      gtk_entry_set_text(m_angles.m_yaw, angle.c_str());
      angle.clear();

      angle << angle_normalised(pitch_yaw_roll.z());
      gtk_entry_set_text(m_angles.m_roll, angle.c_str());
      angle.clear();
    }
    else
    {
      gtk_entry_set_text(m_angles.m_pitch, "0");
      gtk_entry_set_text(m_angles.m_yaw, "0");
      gtk_entry_set_text(m_angles.m_roll, "0");
    }
  }
  typedef MemberCaller<AnglesAttribute, &AnglesAttribute::update> UpdateCaller;
};

class Vector3Entry
{
public:
  GtkEntry* m_x;
  GtkEntry* m_y;
  GtkEntry* m_z;
  Vector3Entry() : m_x(0), m_y(0), m_z(0)
  {
  }
};

class Vector3Attribute : public EntityAttribute
{
  CopiedString m_key;
  Vector3Entry m_vector3;
  NonModalEntry m_nonModal;
  GtkBox* m_hbox;
public:
  Vector3Attribute(const char* key) :
    m_key(key),
    m_nonModal(ApplyCaller(*this), UpdateCaller(*this))
  {
    m_hbox = GTK_BOX(gtk_hbox_new(TRUE, 4));
    gtk_widget_show(GTK_WIDGET(m_hbox));
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_vector3.m_x = entry;
      m_nonModal.connect(m_vector3.m_x);
    }
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_vector3.m_y = entry;
      m_nonModal.connect(m_vector3.m_y);
    }
    {
      GtkEntry* entry = numeric_entry_new();
      gtk_box_pack_start(m_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
      m_vector3.m_z = entry;
      m_nonModal.connect(m_vector3.m_z);
    }
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_hbox);
  }
  void apply()
  {
    StringOutputStream vector3(64);
    vector3 << entry_get_float(m_vector3.m_x)
      << " " << entry_get_float(m_vector3.m_y)
      << " " << entry_get_float(m_vector3.m_z);
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), vector3.c_str());
  }
  typedef MemberCaller<Vector3Attribute, &Vector3Attribute::apply> ApplyCaller;

  void update()
  {
    StringOutputStream buffer(32);
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    if(!string_empty(value))
    {
      DoubleVector3 x_y_z;
      if(!string_parse_vector3(value, x_y_z))
      {
        x_y_z = DoubleVector3(0, 0, 0);
      }

      buffer << x_y_z.x();
      gtk_entry_set_text(m_vector3.m_x, buffer.c_str());
      buffer.clear();

      buffer << x_y_z.y();
      gtk_entry_set_text(m_vector3.m_y, buffer.c_str());
      buffer.clear();

      buffer << x_y_z.z();
      gtk_entry_set_text(m_vector3.m_z, buffer.c_str());
      buffer.clear();
    }
    else
    {
      gtk_entry_set_text(m_vector3.m_x, "0");
      gtk_entry_set_text(m_vector3.m_y, "0");
      gtk_entry_set_text(m_vector3.m_z, "0");
    }
  }
  typedef MemberCaller<Vector3Attribute, &Vector3Attribute::update> UpdateCaller;
};

class NonModalComboBox
{
  Callback m_changed;
  guint m_changedHandler;

  static gboolean changed(GtkComboBox *widget, NonModalComboBox* self)
  {
    self->m_changed();
    return FALSE;
  }

public:
  NonModalComboBox(const Callback& changed) : m_changed(changed), m_changedHandler(0)
  {
  }
  void connect(GtkComboBox* combo)
  {
    m_changedHandler = g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(changed), this);
  }
  void setActive(GtkComboBox* combo, int value)
  {
    g_signal_handler_disconnect(G_OBJECT(combo), m_changedHandler);
    gtk_combo_box_set_active(combo, value);
    connect(combo);
  }
};

class ListAttribute : public EntityAttribute
{
  CopiedString m_key;
  GtkComboBox* m_combo;
  NonModalComboBox m_nonModal;
  const ListAttributeType& m_type;
public:
  ListAttribute(const char* key, const ListAttributeType& type) :
    m_key(key),
    m_combo(0),
    m_nonModal(ApplyCaller(*this)),
    m_type(type)
  {
    GtkComboBox* combo = GTK_COMBO_BOX(gtk_combo_box_new_text());

    for(ListAttributeType::const_iterator i = type.begin(); i != type.end(); ++i)
    {
      gtk_combo_box_append_text(GTK_COMBO_BOX(combo), (*i).first.c_str());
    }

    gtk_widget_show(GTK_WIDGET(combo));
    m_nonModal.connect(combo);

    m_combo = combo;
  }
  void release()
  {
    delete this;
  }
  GtkWidget* getWidget() const
  {
    return GTK_WIDGET(m_combo);
  }
  void apply()
  {
    Scene_EntitySetKeyValue_Selected_Undoable(m_key.c_str(), m_type[gtk_combo_box_get_active(m_combo)].second.c_str());
  }
  typedef MemberCaller<ListAttribute, &ListAttribute::apply> ApplyCaller;

  void update()
  {
    const char* value = SelectedEntity_getValueForKey(m_key.c_str());
    ListAttributeType::const_iterator i = m_type.findValue(value);
    if(i != m_type.end())
    {
      m_nonModal.setActive(m_combo, static_cast<int>(std::distance(m_type.begin(), i)));
    }
    else
    {
      m_nonModal.setActive(m_combo, 0);
    }
  }
  typedef MemberCaller<ListAttribute, &ListAttribute::update> UpdateCaller;
};


namespace
{
  GtkWidget* g_entity_split1 = 0;
  GtkWidget* g_entity_split2 = 0;
  int g_entitysplit1_position;
  int g_entitysplit2_position;

  bool g_entityInspector_windowConstructed = false;

  GtkTreeView* g_entityClassList;
  GtkTextView* g_entityClassComment;

  GtkCheckButton* g_entitySpawnflagsCheck[MAX_FLAGS];

  GtkEntry* g_entityKeyEntry;
  GtkEntry* g_entityValueEntry;

  GtkListStore* g_entlist_store;
  GtkListStore* g_entprops_store;
  const EntityClass* g_current_flags = 0;
  const EntityClass* g_current_comment = 0;
  const EntityClass* g_current_attributes = 0;

  // the number of active spawnflags
  int g_spawnflag_count;
  // table: index, match spawnflag item to the spawnflag index (i.e. which bit)
  int spawn_table[MAX_FLAGS];
  // we change the layout depending on how many spawn flags we need to display
  // the table is a 4x4 in which we need to put the comment box g_entityClassComment and the spawn flags..
  GtkTable* g_spawnflagsTable;

  GtkVBox* g_attributeBox = 0;
  typedef std::vector<EntityAttribute*> EntityAttributes;
  EntityAttributes g_entityAttributes;
}

void GlobalEntityAttributes_clear()
{
  for(EntityAttributes::iterator i = g_entityAttributes.begin(); i != g_entityAttributes.end(); ++i)
  {
    (*i)->release();
  }
  g_entityAttributes.clear();
}

class GetKeyValueVisitor : public Entity::Visitor
{
  KeyValues& m_keyvalues;
public:
  GetKeyValueVisitor(KeyValues& keyvalues)
    : m_keyvalues(keyvalues)
  {
  }

  void visit(const char* key, const char* value)
  {
    m_keyvalues.insert(KeyValues::value_type(CopiedString(key), CopiedString(value)));
  }

};

void Entity_GetKeyValues(const Entity& entity, KeyValues& keyvalues, KeyValues& defaultValues)
{
  GetKeyValueVisitor visitor(keyvalues);

  entity.forEachKeyValue(visitor);

  const EntityClassAttributes& attributes = entity.getEntityClass().m_attributes;

  for(EntityClassAttributes::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
  {
    defaultValues.insert(KeyValues::value_type((*i).first, (*i).second.m_value));
  }
}

void Entity_GetKeyValues_Selected(KeyValues& keyvalues, KeyValues& defaultValues)
{
  class EntityGetKeyValues : public SelectionSystem::Visitor
  {
    KeyValues& m_keyvalues;
    KeyValues& m_defaultValues;
    mutable std::set<Entity*> m_visited;
  public:
    EntityGetKeyValues(KeyValues& keyvalues, KeyValues& defaultValues)
      : m_keyvalues(keyvalues), m_defaultValues(defaultValues)
    {
    }
    void visit(scene::Instance& instance) const
    {
      Entity* entity = Node_getEntity(instance.path().top());
      if(entity == 0 && instance.path().size() != 1)
      {
        entity = Node_getEntity(instance.path().parent());
      }
      if(entity != 0 && m_visited.insert(entity).second)
      {
        Entity_GetKeyValues(*entity, m_keyvalues, m_defaultValues);
      }
    }
  } visitor(keyvalues, defaultValues);
  GlobalSelectionSystem().foreachSelected(visitor);
}

const char* keyvalues_valueforkey(KeyValues& keyvalues, const char* key)
{
  KeyValues::iterator i = keyvalues.find(CopiedString(key));
  if(i != keyvalues.end())
    return (*i).second.c_str();
  return "";
}

class EntityClassListStoreAppend : public EntityClassVisitor
{
  GtkListStore* store;
public:
  EntityClassListStoreAppend(GtkListStore* store_) : store(store_)
  {
  }
  void visit(EntityClass* e)
  {
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, e->name(), 1, e, -1);
  }
};

void EntityClassList_fill()
{
  EntityClassListStoreAppend append(g_entlist_store);
  GlobalEntityClassManager().forEach(append);
}

void EntityClassList_clear()
{
  gtk_list_store_clear(g_entlist_store);
}

void SetComment(EntityClass* eclass)
{
  if(eclass == g_current_comment)
    return;

  g_current_comment = eclass;

  GtkTextBuffer* buffer = gtk_text_view_get_buffer(g_entityClassComment);
  gtk_text_buffer_set_text(buffer, eclass->comments(), -1);
}

void SurfaceFlags_setEntityClass(EntityClass* eclass)
{
  if(eclass == g_current_flags)
    return;

  g_current_flags = eclass;

  int spawnflag_count = 0;

  {
    // do a first pass to count the spawn flags, don't touch the widgets, we don't know in what state they are
    for (int i=0 ; i<MAX_FLAGS ; i++)
    {
      if (eclass->flagnames[i] && eclass->flagnames[i][0] != 0 && strcmp(eclass->flagnames[i],"-"))
      {
        spawn_table[spawnflag_count] = i;
        spawnflag_count++;
      }
    }
  }

  // disable all remaining boxes
  // NOTE: these boxes might not even be on display
  {
    for (int i = 0; i < g_spawnflag_count; ++i)
    {
      GtkWidget* widget = GTK_WIDGET(g_entitySpawnflagsCheck[i]);
      gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), " ");
      gtk_widget_hide(widget);
      gtk_widget_ref(widget);
      gtk_container_remove(GTK_CONTAINER(g_spawnflagsTable), widget);
    }
  }

  g_spawnflag_count = spawnflag_count;

  {
    for (int i = 0; i < g_spawnflag_count; ++i)
    {
      GtkWidget* widget = GTK_WIDGET(g_entitySpawnflagsCheck[i]);
      gtk_widget_show (widget);
  
      StringOutputStream str(16);
      str << LowerCase(eclass->flagnames[spawn_table[i]]);
  
      gtk_table_attach(g_spawnflagsTable, widget, i%4, i%4+1, i/4, i/4+1,
            (GtkAttachOptions)(GTK_FILL),
            (GtkAttachOptions)(GTK_FILL), 0, 0);
      gtk_widget_unref(widget);
  
      gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), str.c_str());
    }
  }
}

void EntityClassList_selectEntityClass(EntityClass* eclass)
{
  GtkTreeModel* model = GTK_TREE_MODEL(g_entlist_store);
  GtkTreeIter iter;
  for(gboolean good = gtk_tree_model_get_iter_first(model, &iter); good != FALSE; good = gtk_tree_model_iter_next(model, &iter))
  {
    char* text;
    gtk_tree_model_get(model, &iter, 0, &text, -1);
    if (strcmp (text, eclass->name()) == 0)
    {
      GtkTreeView* view = g_entityClassList;
      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
      gtk_tree_selection_select_path(gtk_tree_view_get_selection(view), path);
      gtk_tree_view_scroll_to_cell(view, path, 0, FALSE, 0, 0);
      gtk_tree_path_free(path);
      good = FALSE;
    }
    g_free(text);
  }
}

void EntityInspector_appendAttribute(const char* name, EntityAttribute& attribute)
{
  GtkTable* row = DialogRow_new(name, attribute.getWidget());
  DialogVBox_packRow(g_attributeBox, GTK_WIDGET(row));
}


template<typename Attribute>
class StatelessAttributeCreator
{
public:
  static EntityAttribute* create(const char* name)
  {
    return new Attribute(name);
  }
};

class EntityAttributeFactory
{
  typedef EntityAttribute* (*CreateFunc)(const char* name);
  typedef std::map<const char*, CreateFunc, RawStringLess> Creators;
  Creators m_creators;
public:
  EntityAttributeFactory()
  {
    m_creators.insert(Creators::value_type("string", &StatelessAttributeCreator<StringAttribute>::create));
    m_creators.insert(Creators::value_type("color", &StatelessAttributeCreator<StringAttribute>::create));
    m_creators.insert(Creators::value_type("integer", &StatelessAttributeCreator<StringAttribute>::create));
    m_creators.insert(Creators::value_type("real", &StatelessAttributeCreator<StringAttribute>::create));
    m_creators.insert(Creators::value_type("shader", &StatelessAttributeCreator<ShaderAttribute>::create));
    m_creators.insert(Creators::value_type("boolean", &StatelessAttributeCreator<BooleanAttribute>::create));
    m_creators.insert(Creators::value_type("angle", &StatelessAttributeCreator<AngleAttribute>::create));
    m_creators.insert(Creators::value_type("direction", &StatelessAttributeCreator<DirectionAttribute>::create));
    m_creators.insert(Creators::value_type("angles", &StatelessAttributeCreator<AnglesAttribute>::create));
    m_creators.insert(Creators::value_type("model", &StatelessAttributeCreator<ModelAttribute>::create));
    m_creators.insert(Creators::value_type("sound", &StatelessAttributeCreator<SoundAttribute>::create));
    m_creators.insert(Creators::value_type("vector3", &StatelessAttributeCreator<Vector3Attribute>::create));
  }
  EntityAttribute* create(const char* type, const char* name)
  {
    Creators::iterator i = m_creators.find(type);
    if(i != m_creators.end())
    {
      return (*i).second(name);
    }
    const ListAttributeType* listType = GlobalEntityClassManager().findListType(type);
    if(listType != 0)
    {
      return new ListAttribute(name, *listType);
    }
    return 0;
  }
};

typedef Static<EntityAttributeFactory> GlobalEntityAttributeFactory;

void EntityInspector_setEntityClass(EntityClass *eclass)
{
  EntityClassList_selectEntityClass(eclass);
  SurfaceFlags_setEntityClass(eclass);

  if(eclass != g_current_attributes)
  {
    g_current_attributes = eclass;

    container_remove_all(GTK_CONTAINER(g_attributeBox));
    GlobalEntityAttributes_clear();

    for(EntityClassAttributes::const_iterator i = eclass->m_attributes.begin(); i != eclass->m_attributes.end(); ++i)
    {
      EntityAttribute* attribute = GlobalEntityAttributeFactory::instance().create((*i).second.m_type.c_str(), (*i).first.c_str());
      if(attribute != 0)
      {
        g_entityAttributes.push_back(attribute);
        EntityInspector_appendAttribute(EntityClassAttributePair_getName(*i), *g_entityAttributes.back());
      }
    }
  }
}

void EntityInspector_updateSpawnflags()
{
  {
    int f = atoi(SelectedEntity_getValueForKey("spawnflags"));
    for (int i = 0; i < g_spawnflag_count; ++i)
    {
      int v = !!(f&(1<<spawn_table[i]));
      
      toggle_button_set_active_no_signal(GTK_TOGGLE_BUTTON(g_entitySpawnflagsCheck[i]), v);
    }
  }
  {
    // take care of the remaining ones
    for (int i = g_spawnflag_count; i < MAX_FLAGS; ++i)
    {
      toggle_button_set_active_no_signal(GTK_TOGGLE_BUTTON(g_entitySpawnflagsCheck[i]), FALSE);
    }
  }
}

void EntityInspector_applySpawnflags()
{
  int f, i, v;
  char sz[32];

  f = 0;
  for (i = 0; i < g_spawnflag_count; ++i)
  {
    v = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_entitySpawnflagsCheck[i]));
    f |= v<<spawn_table[i];
  }

  sprintf (sz, "%i", f);    
  const char* value = (f == 0) ? "" : sz;

  {
    StringOutputStream command;
    command << "entitySetFlags -flags " << f;
    UndoableCommand undo("entitySetSpawnflags");

    Scene_EntitySetKeyValue_Selected("spawnflags", value);
  }
}


void EntityInspector_updateKeyValues()
{
  g_selectedKeyValues.clear();
  g_selectedDefaultKeyValues.clear();
  Entity_GetKeyValues_Selected(g_selectedKeyValues, g_selectedDefaultKeyValues);

  EntityInspector_setEntityClass(GlobalEntityClassManager().findOrInsert(keyvalues_valueforkey(g_selectedKeyValues, "classname"), false));

  EntityInspector_updateSpawnflags();

  GtkListStore* store = g_entprops_store;

  // save current key/val pair around filling epair box
  // row_select wipes it and sets to first in list
  CopiedString strKey(gtk_entry_get_text(g_entityKeyEntry));
  CopiedString strVal(gtk_entry_get_text(g_entityValueEntry));

  gtk_list_store_clear(store);
  // Walk through list and add pairs
  for(KeyValues::iterator i = g_selectedKeyValues.begin(); i != g_selectedKeyValues.end(); ++i)
  {
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    StringOutputStream key(64);
    key << ConvertLocaleToUTF8((*i).first.c_str());
    StringOutputStream value(64);
    value << ConvertLocaleToUTF8((*i).second.c_str());
    gtk_list_store_set(store, &iter, 0, key.c_str(), 1, value.c_str(), -1);
  }

  gtk_entry_set_text(g_entityKeyEntry, strKey.c_str());
  gtk_entry_set_text(g_entityValueEntry, strVal.c_str());

  for(EntityAttributes::const_iterator i = g_entityAttributes.begin(); i != g_entityAttributes.end(); ++i)
  {
    (*i)->update();
  }
}

class EntityInspectorDraw
{
  IdleDraw m_idleDraw;
public:
  EntityInspectorDraw() : m_idleDraw(FreeCaller<EntityInspector_updateKeyValues>())
  {
  }
  void queueDraw()
  {
    m_idleDraw.queueDraw();
  }
};

EntityInspectorDraw g_EntityInspectorDraw;


void EntityInspector_keyValueChanged()
{
  g_EntityInspectorDraw.queueDraw();
}
void EntityInspector_selectionChanged(const Selectable&)
{
  EntityInspector_keyValueChanged();
}

// Creates a new entity based on the currently selected brush and entity type.
//
void EntityClassList_createEntity()
{
  GtkTreeView* view = g_entityClassList;

  // find out what type of entity we are trying to create
  GtkTreeModel* model;
  GtkTreeIter iter;
  if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(view), &model, &iter) == FALSE)
  {
    gtk_MessageBox(gtk_widget_get_toplevel(GTK_WIDGET(g_entityClassList)), "You must have a selected class to create an entity", "info");
    return;
  }

  char* text;
  gtk_tree_model_get(model, &iter, 0, &text, -1);

  {
    StringOutputStream command;
    command << "entityCreate -class " << text;

    UndoableCommand undo(command.c_str());

    Entity_createFromSelection(text, g_vector3_identity);
  }
  g_free(text);
}

void EntityInspector_applyKeyValue()
{
  // Get current selection text
  StringOutputStream key(64);
  key << ConvertUTF8ToLocale(gtk_entry_get_text(g_entityKeyEntry));
  StringOutputStream value(64);
  value << ConvertUTF8ToLocale(gtk_entry_get_text(g_entityValueEntry));


  // TTimo: if you change the classname to worldspawn you won't merge back in the structural brushes but create a parasite entity
  if (!strcmp(key.c_str(), "classname") && !strcmp(value.c_str(), "worldspawn"))
  {
    gtk_MessageBox(gtk_widget_get_toplevel(GTK_WIDGET(g_entityKeyEntry)),  "Cannot change \"classname\" key back to worldspawn.", 0, eMB_OK );
    return;
  }


  // RR2DO2: we don't want spaces in entity keys
  if (strstr( key.c_str(), " " ))
  {
    gtk_MessageBox(gtk_widget_get_toplevel(GTK_WIDGET(g_entityKeyEntry)), "No spaces are allowed in entity keys.", 0, eMB_OK );
    return;
  }

  if(strcmp(key.c_str(), "classname") == 0)
  {
    StringOutputStream command;
    command << "entitySetClass -class " << value.c_str();
    UndoableCommand undo(command.c_str());
    Scene_EntitySetClassname_Selected(value.c_str());
  }
  else
  {
    Scene_EntitySetKeyValue_Selected_Undoable(key.c_str(), value.c_str());
  }
}

void EntityInspector_clearKeyValue()
{
  // Get current selection text
  StringOutputStream key(64);
  key << ConvertUTF8ToLocale(gtk_entry_get_text(g_entityKeyEntry));

  if(strcmp(key.c_str(), "classname") != 0)
  {
    StringOutputStream command;
    command << "entityDeleteKey -key " << key.c_str();
    UndoableCommand undo(command.c_str());
    Scene_EntitySetKeyValue_Selected(key.c_str(), "");
  }
}

void EntityInspector_clearAllKeyValues()
{
  UndoableCommand undo("entityClear");

  // remove all keys except classname
  for(KeyValues::iterator i = g_selectedKeyValues.begin(); i != g_selectedKeyValues.end(); ++i)
  {
    if(strcmp((*i).first.c_str(), "classname") != 0)
    {
      Scene_EntitySetKeyValue_Selected((*i).first.c_str(), "");
    }
  }
}

// =============================================================================
// callbacks

static void EntityClassList_selection_changed(GtkTreeSelection* selection, gpointer data)
{
  GtkTreeModel* model;
  GtkTreeIter selected;
  if(gtk_tree_selection_get_selected(selection, &model, &selected))
  {
    EntityClass* eclass;
    gtk_tree_model_get(model, &selected, 1, &eclass, -1);
    if(eclass != 0)
    {
      SetComment(eclass);
    }
  }
}

static gint EntityClassList_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if (event->type == GDK_2BUTTON_PRESS)
  {
    EntityClassList_createEntity();
    return TRUE;
  }
  return FALSE;
}

static gint EntityClassList_keypress(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  unsigned int code = gdk_keyval_to_upper (event->keyval);

  if (event->keyval == GDK_Return)
  {
    EntityClassList_createEntity();
    return TRUE;
  }

  // select the entity that starts with the key pressed
  if (code <= 'Z' && code >= 'A')
  {
    GtkTreeView* view = g_entityClassList;
    GtkTreeModel* model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(view), &model, &iter) == FALSE
      || gtk_tree_model_iter_next(model, &iter) == FALSE)
    {
      gtk_tree_model_get_iter_first(model, &iter);
    }

    for(std::size_t count = gtk_tree_model_iter_n_children(model, 0); count > 0; --count)
    {
      char* text;
      gtk_tree_model_get(model, &iter, 0, &text, -1);

      if (toupper (text[0]) == (int)code)
      {
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
        gtk_tree_selection_select_path(gtk_tree_view_get_selection(view), path);
        gtk_tree_view_scroll_to_cell(view, path, 0, FALSE, 0, 0);
        gtk_tree_path_free(path);
        count = 1;
      }

      g_free(text);

      if(gtk_tree_model_iter_next(model, &iter) == FALSE)
        gtk_tree_model_get_iter_first(model, &iter);
    }

    return TRUE;
  }
  return FALSE;
}

static void EntityProperties_selection_changed(GtkTreeSelection* selection, gpointer data)
{
  // find out what type of entity we are trying to create
  GtkTreeModel* model;
  GtkTreeIter iter;
  if(gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
  {
    return;
  }

  char* key;
  char* val;
  gtk_tree_model_get(model, &iter, 0, &key, 1, &val, -1);

  gtk_entry_set_text(g_entityKeyEntry, key);
  gtk_entry_set_text(g_entityValueEntry, val);

  g_free(key);
  g_free(val);
}

static void SpawnflagCheck_toggled(GtkWidget *widget, gpointer data)
{
  EntityInspector_applySpawnflags();
}

static gint EntityEntry_keypress(GtkEntry* widget, GdkEventKey* event, gpointer data)
{
  if (event->keyval == GDK_Return)
  {
    if(widget == g_entityKeyEntry)
    {
      gtk_entry_set_text(g_entityValueEntry, "");
      gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(widget))), GTK_WIDGET(g_entityValueEntry));
    }
    else
    {
      EntityInspector_applyKeyValue();
    }
    return TRUE;
  }
  if (event->keyval == GDK_Escape)
  {
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(widget))), NULL);
    return TRUE;
  }

  return FALSE;
}

void EntityInspector_destroyWindow(GtkWidget* widget, gpointer data)
{
  g_entitysplit1_position = gtk_paned_get_position(GTK_PANED(g_entity_split1));
  g_entitysplit2_position = gtk_paned_get_position(GTK_PANED(g_entity_split2));

  g_entityInspector_windowConstructed = false;
  GlobalEntityAttributes_clear();
}

GtkWidget* EntityInspector_constructWindow(GtkWindow* toplevel)
{
  GtkWidget* vbox = gtk_vbox_new(FALSE, 2);
  gtk_widget_show (vbox);
  gtk_container_set_border_width(GTK_CONTAINER (vbox), 2);

  g_signal_connect(G_OBJECT(vbox), "destroy", G_CALLBACK(EntityInspector_destroyWindow), 0);

  {
    GtkWidget* split1 = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(vbox), split1, TRUE, TRUE, 0);
    gtk_widget_show (split1);

    g_entity_split1 = split1;

    {
      GtkWidget* split2 = gtk_vpaned_new();
      gtk_paned_add1 (GTK_PANED (split1), split2);
      gtk_widget_show (split2);

      g_entity_split2 = split2;

      {
        // class list
        GtkWidget* scr = gtk_scrolled_window_new (0, 0);
        gtk_widget_show (scr);
        gtk_paned_add1 (GTK_PANED (split2), scr);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

        {
          GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

          GtkTreeView* view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));
          gtk_tree_view_set_headers_visible(view, FALSE);
          g_signal_connect(G_OBJECT(view), "button_press_event", G_CALLBACK(EntityClassList_button_press), 0);
          g_signal_connect(G_OBJECT(view), "key_press_event", G_CALLBACK(EntityClassList_keypress), 0);

          {
            GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Key", renderer, "text", 0, 0);
            gtk_tree_view_append_column(view, column);
          }

          {
            GtkTreeSelection* selection = gtk_tree_view_get_selection(view);
            g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(EntityClassList_selection_changed), 0);
          }

          gtk_widget_show(GTK_WIDGET(view));

          gtk_container_add(GTK_CONTAINER(scr), GTK_WIDGET(view));

          g_object_unref(G_OBJECT(store));
          g_entityClassList = view;
          g_entlist_store = store;
        }
      }

      {
        GtkWidget* scr = gtk_scrolled_window_new (0, 0);
        gtk_widget_show (scr);
        gtk_paned_add2 (GTK_PANED (split2), scr);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

        {
          GtkTextView* text = GTK_TEXT_VIEW(gtk_text_view_new());
          gtk_widget_set_size_request(GTK_WIDGET(text), 0, -1); // allow shrinking
          gtk_text_view_set_wrap_mode(text, GTK_WRAP_WORD);
          gtk_text_view_set_editable(text, FALSE);
          gtk_widget_show(GTK_WIDGET(text));
          gtk_container_add(GTK_CONTAINER(scr), GTK_WIDGET(text));
          g_entityClassComment = text;
        }
      }
    }

    {
      GtkWidget* split2 = gtk_vpaned_new();
      gtk_paned_add2 (GTK_PANED (split1), split2);
      gtk_widget_show(split2);

      {
        GtkWidget* vbox2 = gtk_vbox_new (FALSE, 2);
        gtk_widget_show (vbox2);
        gtk_paned_pack1(GTK_PANED(split2), vbox2, FALSE, FALSE);

        {
          // Spawnflags (4 colums wide max, or window gets too wide.)
          GtkTable* table = GTK_TABLE(gtk_table_new(4, 4, FALSE));
          gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(table), FALSE, TRUE, 0);
          gtk_widget_show(GTK_WIDGET(table));
          
          g_spawnflagsTable = table;

          for (int i = 0; i < MAX_FLAGS; i++)
          {
            GtkCheckButton* check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(""));
            gtk_widget_ref(GTK_WIDGET(check));
            g_object_set_data(G_OBJECT(check), "handler", gint_to_pointer(g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(SpawnflagCheck_toggled), 0)));
            g_entitySpawnflagsCheck[i] = check;
          }
        }

        {
          // key/value list
          GtkWidget* scr = gtk_scrolled_window_new (0, 0);
          gtk_widget_show (scr);
          gtk_box_pack_start (GTK_BOX (vbox2), scr, TRUE, TRUE, 0);
          gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
          gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scr), GTK_SHADOW_IN);

          {
            GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

            GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
            gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

            {
              GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
              GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, 0);
              gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
            }

            {
              GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
              GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 1, 0);
              gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
            }

            {
              GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
              g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(EntityProperties_selection_changed), 0);
            }

            gtk_widget_show(view);

            gtk_container_add(GTK_CONTAINER (scr), view);

            g_object_unref(G_OBJECT(store));

            g_entprops_store = store;
          }
        }

        {
          // key/value entry
          GtkTable* table = GTK_TABLE(gtk_table_new(2, 2, FALSE));
          gtk_widget_show(GTK_WIDGET(table));
          gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(table), FALSE, TRUE, 0);
          gtk_table_set_row_spacings(table, 3);
          gtk_table_set_col_spacings(table, 5);

          {
            GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_show(GTK_WIDGET(entry));
            gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 0, 1,
                              (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                              (GtkAttachOptions)(0), 0, 0);
            gtk_widget_set_events(GTK_WIDGET(entry), GDK_KEY_PRESS_MASK);
            g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(EntityEntry_keypress), 0);
            g_entityKeyEntry = entry;
          }

          {
            GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_show(GTK_WIDGET(entry));
            gtk_table_attach(table, GTK_WIDGET(entry), 1, 2, 1, 2,
                              (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                              (GtkAttachOptions)(0), 0, 0);
            gtk_widget_set_events(GTK_WIDGET(entry), GDK_KEY_PRESS_MASK);
            g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(EntityEntry_keypress), 0);
            g_entityValueEntry = entry;
          }

          {
            GtkLabel* label = GTK_LABEL(gtk_label_new("Value"));
            gtk_widget_show(GTK_WIDGET(label));
            gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 1, 2,
                              (GtkAttachOptions)(GTK_FILL),
                              (GtkAttachOptions)(0), 0, 0);
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
          }

          {
            GtkLabel* label = GTK_LABEL(gtk_label_new("Key"));
            gtk_widget_show(GTK_WIDGET(label));
            gtk_table_attach(table, GTK_WIDGET(label), 0, 1, 0, 1,
                              (GtkAttachOptions)(GTK_FILL),
                              (GtkAttachOptions)(0), 0, 0);
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
          }
        }

        {
          GtkBox* hbox = GTK_BOX(gtk_hbox_new(TRUE, 4));
          gtk_widget_show(GTK_WIDGET(hbox));
          gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(hbox), FALSE, TRUE, 0);
          
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("Clear All"));
            gtk_widget_show(GTK_WIDGET(button));
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(EntityInspector_clearAllKeyValues), 0);
            gtk_box_pack_start(hbox, GTK_WIDGET(button), TRUE, TRUE, 0);
          }
          {
            GtkButton* button = GTK_BUTTON(gtk_button_new_with_label("Delete Key"));
            gtk_widget_show(GTK_WIDGET(button));
            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(EntityInspector_clearKeyValue), 0);
            gtk_box_pack_start(hbox, GTK_WIDGET(button), TRUE, TRUE, 0);
          }
        }
      }

      {
        GtkWidget* scr = gtk_scrolled_window_new(0, 0);
        gtk_widget_show(scr);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scr), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

        GtkWidget* viewport = gtk_viewport_new(0, 0);
        gtk_widget_show(viewport);
        gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);

        g_attributeBox = GTK_VBOX(gtk_vbox_new(FALSE, 2));
        gtk_widget_show(GTK_WIDGET(g_attributeBox));

        gtk_container_add(GTK_CONTAINER(viewport), GTK_WIDGET(g_attributeBox));
        gtk_container_add(GTK_CONTAINER(scr), viewport);
        gtk_paned_pack2(GTK_PANED(split2), scr, FALSE, FALSE);
      }
    }
  }


  {
    int x = g_entitysplit1_position;
    if (x != -1)
    {
      gtk_paned_set_position(GTK_PANED(g_entity_split1), x);

      x = g_entitysplit2_position;

      if (x != -1)
        gtk_paned_set_position (GTK_PANED(g_entity_split2), x);
    }
  }

  g_entityInspector_windowConstructed = true;
  EntityClassList_fill();

  GlobalSelectionSystem().addSelectionChangeCallback(FreeCaller1<const Selectable&, EntityInspector_selectionChanged>());
  GlobalEntityCreator().setKeyValueChangedFunc(EntityInspector_keyValueChanged);

  // hack
  gtk_container_set_focus_chain(GTK_CONTAINER(vbox), NULL);

  return vbox;
}

class EntityInspector : public ModuleObserver
{
  std::size_t m_unrealised;
public:
  EntityInspector() : m_unrealised(1)
  {
  }
  void realise()
  {
    if(--m_unrealised == 0)
    {
      if(g_entityInspector_windowConstructed)
      {
        //globalOutputStream() << "Entity Inspector: realise\n";
        EntityClassList_fill();
      }
    }
  }
  void unrealise()
  {
    if(++m_unrealised == 1)
    {
      if(g_entityInspector_windowConstructed)
      {
        //globalOutputStream() << "Entity Inspector: unrealise\n";
        EntityClassList_clear();
      }
    }
  }
};

EntityInspector g_EntityInspector;

#include "preferencesystem.h"
#include "stringio.h"

void EntityInspector_construct()
{
  GlobalEntityClassManager().attach(g_EntityInspector);

  GlobalPreferenceSystem().registerPreference("EntitySplit1", IntImportStringCaller(g_entitysplit1_position), IntExportStringCaller(g_entitysplit1_position));
  GlobalPreferenceSystem().registerPreference("EntitySplit2", IntImportStringCaller(g_entitysplit2_position), IntExportStringCaller(g_entitysplit2_position));

}

void EntityInspector_destroy()
{
  GlobalEntityClassManager().detach(g_EntityInspector);
}

