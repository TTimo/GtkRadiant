/*
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

#include "ufoai_filters.h"

#include "ibrush.h"
#include "ientity.h"
#include "iscenegraph.h"

// believe me, i'm sorry
#include "../../radiant/brush.h"

#include "generic/callback.h"

#include <list>

bool actorclip_active = false;
bool stepon_active = false;
bool nodraw_active = false;
bool weaponclip_active = false;
int level_active = 0;

// TODO: This should be added to ibrush.h
// like already done for Node_getEntity in ientity.h
// FIXME: Doesn't belong here
inline Brush* Node_getBrush(scene::Node& node)
{
	return NodeTypeCast<Brush>::cast(node);
}

void hide_node(scene::Node& node, bool hide)
{
	hide
	? node.enable(scene::Node::eHidden)
	: node.disable(scene::Node::eHidden);
}

typedef std::list<Entity*> entitylist_t;

class EntityFindByName : public scene::Graph::Walker
{
	const char* m_name;
	entitylist_t& m_entitylist;
	/* this starts at 1 << level */
	int m_flag;
	int m_hide;
public:
	EntityFindByName(const char* name, entitylist_t& entitylist, int flag, bool hide)
		: m_name(name), m_entitylist(entitylist), m_flag(flag), m_hide(hide)
	{
	}
	bool pre(const scene::Path& path, scene::Instance& instance) const
	{
		int spawnflagsInt;
		Entity* entity = Node_getEntity(path.top());
		if (entity != 0)
		{
			if (string_equal(m_name, entity->getKeyValue("classname")))
			{
				const char *spawnflags = entity->getKeyValue("spawnflags");
				globalOutputStream() << "spawnflags for " << m_name << ": " << spawnflags << ".\n";

				if (!string_empty(spawnflags)) {
					spawnflagsInt = atoi(spawnflags);
					if (!(spawnflagsInt & m_flag))
					{
						hide_node(path.top(), m_hide); // hide/unhide
						m_entitylist.push_back(entity);
					}
				}
				else
				{
					globalOutputStream() << "UFO:AI: Warning: no spawnflags for " << m_name << ".\n";
				}
			}
		}
		return true;
	}
};

class ForEachFace : public BrushVisitor
{
	Brush &m_brush;
public:
	mutable int m_contentFlagsVis;
	mutable int m_surfaceFlagsVis;

	ForEachFace(Brush& brush)
		: m_brush(brush)
	{
		m_contentFlagsVis = -1;
		m_surfaceFlagsVis = -1;
	}

	void visit(Face& face) const
	{
#if _DEBUG
		if (m_surfaceFlagsVis < 0)
			m_surfaceFlagsVis = face.getShader().m_flags.m_surfaceFlags;
		else if (m_surfaceFlagsVis >= 0 && m_surfaceFlagsVis != face.getShader().m_flags.m_surfaceFlags)
			globalOutputStream() << "Faces with different surfaceflags at brush\n";
		if (m_contentFlagsVis < 0)
			m_contentFlagsVis = face.getShader().m_flags.m_contentFlags;
		else if (m_contentFlagsVis >= 0 && m_contentFlagsVis != face.getShader().m_flags.m_contentFlags)
			globalOutputStream() << "Faces with different contentflags at brush\n";
#else
		m_surfaceFlagsVis = face.getShader().m_flags.m_surfaceFlags;
		m_contentFlagsVis = face.getShader().m_flags.m_contentFlags;
#endif
	}
};

typedef std::list<Brush*> brushlist_t;

class BrushGetLevel : public scene::Graph::Walker
{
	brushlist_t& m_brushlist;
	int m_flag;
	bool m_content; // if true - use m_contentFlags - otherwise m_surfaceFlags
	mutable bool m_notset;
	mutable bool m_hide;
public:
	BrushGetLevel(brushlist_t& brushlist, int flag, bool content, bool notset, bool hide)
		: m_brushlist(brushlist), m_flag(flag), m_content(content), m_notset(notset), m_hide(hide)
	{
	}
	bool pre(const scene::Path& path, scene::Instance& instance) const
	{
		Brush* brush = Node_getBrush(path.top());
		if (brush != 0)
		{
			ForEachFace faces(*brush);
			brush->forEachFace(faces);
			// contentflags?
			if (m_content)
			{
				// are any flags set?
				if (faces.m_contentFlagsVis > 0)
				{
					// flag should not be set
					if (m_notset && (!(faces.m_contentFlagsVis & m_flag)))
					{
						hide_node(path.top(), m_hide);
						m_brushlist.push_back(brush);
					}
					// check whether flag is set
					else if (!m_notset && ((faces.m_contentFlagsVis & m_flag)))
					{
						hide_node(path.top(), m_hide);
						m_brushlist.push_back(brush);
					}
				}
			}
			// surfaceflags?
			else
			{
				// are any flags set?
				if (faces.m_surfaceFlagsVis > 0)
				{
					// flag should not be set
					if (m_notset && (!(faces.m_surfaceFlagsVis & m_flag)))
					{
						hide_node(path.top(), m_hide);
						m_brushlist.push_back(brush);
					}
					// check whether flag is set
					else if (!m_notset && ((faces.m_surfaceFlagsVis & m_flag)))
					{
						hide_node(path.top(), m_hide);
						m_brushlist.push_back(brush);
					}
				}
			}

		}
		return true;
	}
};

/**
 * @brief Activates the level filter for the given level
 * @param[in] level Which level to show?
 * @todo Entities
 */
void filter_level(int flag)
{
	int level;
	brushlist_t brushes;
	entitylist_t entities;

	level = (flag >> 8);

	if (level_active)
	{
		GlobalSceneGraph().traverse(BrushGetLevel(brushes, (level_active << 8), true, true, false));
		GlobalSceneGraph().traverse(EntityFindByName("func_door", entities, level_active, false));
		GlobalSceneGraph().traverse(EntityFindByName("func_breakable", entities, level_active, false));
		GlobalSceneGraph().traverse(EntityFindByName("misc_model", entities, level_active, false));
		GlobalSceneGraph().traverse(EntityFindByName("misc_particle", entities, level_active, false));
		entities.erase(entities.begin(), entities.end());
		brushes.erase(brushes.begin(), brushes.end());
		if (level_active == level)
		{
			level_active = 0;
			// just disabĺe level filter
			return;
		}
	}
	level_active = level;
	globalOutputStream() << "UFO:AI: level_active: " << level_active << ", flag: " << flag << ".\n";

	// first all brushes
	GlobalSceneGraph().traverse(BrushGetLevel(brushes, flag, true, true, true));

	// now all entities
	GlobalSceneGraph().traverse(EntityFindByName("func_door", entities, level, true));
	GlobalSceneGraph().traverse(EntityFindByName("func_breakable", entities, level, true));
	GlobalSceneGraph().traverse(EntityFindByName("misc_model", entities, level, true));
	GlobalSceneGraph().traverse(EntityFindByName("misc_particle", entities, level, true));

#ifdef _DEBUG
	if (brushes.empty())
	{
		globalOutputStream() << "UFO:AI: No brushes.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Found " << Unsigned(brushes.size()) << " brushes.\n";
	}

	// now let's filter all entities like misc_model, func_breakable and func_door that have the spawnflags set
	if (entities.empty())
	{
		globalOutputStream() << "UFO:AI: No entities.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Found " << Unsigned(entities.size()) << " entities.\n";
	}
#endif
}

void filter_stepon (void)
{
	if (stepon_active) {
		stepon_active = false;
	} else {
		stepon_active = true;
	}
	brushlist_t brushes;
	GlobalSceneGraph().traverse(BrushGetLevel(brushes, CONTENTS_STEPON, true, false, stepon_active));

	if (brushes.empty())
	{
		globalOutputStream() << "UFO:AI: No brushes.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Hiding " << Unsigned(brushes.size()) << " stepon brushes.\n";
	}
}

void filter_nodraw (void)
{
	if (nodraw_active) {
		nodraw_active = false;
	} else {
		nodraw_active = true;
	}
	brushlist_t brushes;
	GlobalSceneGraph().traverse(BrushGetLevel(brushes, SURF_NODRAW, false, false, nodraw_active));

#ifdef _DEBUG
	if (brushes.empty())
	{
		globalOutputStream() << "UFO:AI: No brushes.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Hiding " << Unsigned(brushes.size()) << " nodraw brushes.\n";
	}
#endif
}

void filter_actorclip (void)
{
	if (actorclip_active) {
		actorclip_active = false;
	} else {
		actorclip_active = true;
	}
	brushlist_t brushes;
	GlobalSceneGraph().traverse(BrushGetLevel(brushes, CONTENTS_ACTORCLIP, true, false, actorclip_active));

#ifdef _DEBUG
	if (brushes.empty())
	{
		globalOutputStream() << "UFO:AI: No brushes.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Hiding " << Unsigned(brushes.size()) << " actorclip brushes.\n";
	}
#endif
}

void filter_weaponclip (void)
{
	if (weaponclip_active) {
		weaponclip_active = false;
	} else {
		weaponclip_active = true;
	}
	brushlist_t brushes;
	GlobalSceneGraph().traverse(BrushGetLevel(brushes, CONTENTS_WEAPONCLIP, true, false, weaponclip_active));

#ifdef _DEBUG
	if (brushes.empty())
	{
		globalOutputStream() << "UFO:AI: No brushes.\n";
	}
	else
	{
		globalOutputStream() << "UFO:AI: Hiding " << Unsigned(brushes.size()) << " weaponclip brushes.\n";
	}
#endif
}
