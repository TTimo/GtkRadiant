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

#include "ufoai_level.h"
#include "ufoai_filters.h"

#include "ibrush.h"
#include "ientity.h"
#include "iscenegraph.h"

#include "string/string.h"
#include <list>

class Level;

/**
 * @brief find entities by class
 * @note from radiant/map.cpp
 */
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
			if(entity != 0 && string_equal(m_name, entity->getKeyValue("classname")))
			{
				m_entity = entity;
			}
		}
		return true;
	}
};

/**
 * @brief
 */
Entity* Scene_FindEntityByClass(const char* name)
{
	Entity* entity = NULL;
	GlobalSceneGraph().traverse(EntityFindByClassname(name, entity));
	return entity;
}

/**
 * @brief finds start positions
 */
class EntityFindTeams : public scene::Graph::Walker
{
	const char *m_classname;
	int *m_count;
	int *m_team;

public:
	EntityFindTeams(const char *classname, int *count, int *team) : m_classname(classname), m_count(count), m_team(team)
	{
	}
	bool pre(const scene::Path& path, scene::Instance& instance) const
	{
		const char *str;
		Entity* entity = Node_getEntity(path.top());
		if(entity != 0 && string_equal(m_classname, entity->getKeyValue("classname")))
		{
			if (m_count)
				(*m_count)++;
			// now get the highest teamnum
			if (m_team)
			{
				str = entity->getKeyValue("team");
				if (!string_empty(str))
				{
					if (atoi(str) > *m_team)
						(*m_team) = atoi(str);
				}
			}
		}
		return true;
	}
};

/**
 * @brief
 */
void get_team_count (const char *classname, int *count, int *team)
{
	GlobalSceneGraph().traverse(EntityFindTeams(classname, count, team));
	globalOutputStream() << "UFO:AI: classname: " << classname << ": #" << (*count) << "\n";
}

/**
 * @brief Some default values to worldspawn like maxlevel, maxteams and so on
 */
void assign_default_values_to_worldspawn (bool override, bool day, char **returnMsg)
{
	static char message[1024];
	Entity* worldspawn;
	int teams = 0;
	int count = 0;
	char str[64];

	worldspawn = Scene_FindEntityByClass("worldspawn");
	if (!worldspawn)
	{
		globalOutputStream() << "UFO:AI: Could not find worldspawn.\n";
		*returnMsg = "Could not find worldspawn";
		return;
	}

	*message = '\0';
	*str = '\0';

	get_team_count("info_player_start", &count, &teams);

	// TODO: Get highest brush - a level has 64 units
	worldspawn->setKeyValue("maxlevel", "5");

	if (string_empty(worldspawn->getKeyValue("maxteams"))
	 || atoi(worldspawn->getKeyValue("maxteams")) != teams)
	{
		snprintf(str, sizeof(str) - 1, "%i", teams);
		worldspawn->setKeyValue("maxteams", str);
		strncat(message, "Worldspawn: Set maxteams to ", sizeof(message) - 1);
		strncat(message, str, sizeof(message) - 1);
		strncat(message, "\n", sizeof(message) - 1);
	}

	if (day)
	{
		if (override)
		{
			worldspawn->setKeyValue("light", "160");
			worldspawn->setKeyValue("_color", "1 0.8 0.8");
			worldspawn->setKeyValue("angles", "30 210");
			worldspawn->setKeyValue("ambient", "0.4 0.4 0.4");
		}
		else
		{
			if (string_empty(worldspawn->getKeyValue("light")))
			{
				worldspawn->setKeyValue("light", "160");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("_color")))
			{
				worldspawn->setKeyValue("_color", "1 0.8 0.8");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("angles")))
			{
				worldspawn->setKeyValue("angles", "30 210");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("ambient")))
			{
				worldspawn->setKeyValue("ambient", "0.4 0.4 0.4");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
		}
	}
	else
	{
		if (override)
		{
			worldspawn->setKeyValue("light", "60");
			worldspawn->setKeyValue("_color", "0.8 0.8 1");
			worldspawn->setKeyValue("angles", "15 60");
			worldspawn->setKeyValue("ambient", "0.25 0.25 0.275");
		}
		else
		{
			if (string_empty(worldspawn->getKeyValue("light")))
			{
				worldspawn->setKeyValue("light", "60");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("_color")))
			{
				worldspawn->setKeyValue("_color", "0.8 0.8 1");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("angles")))
			{
				worldspawn->setKeyValue("angles", "15 60");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
			if (string_empty(worldspawn->getKeyValue("ambient")))
			{
				worldspawn->setKeyValue("ambient", "0.25 0.25 0.275");
				snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message), "Set ambient to: %s", worldspawn->getKeyValue("ambient"));
			}
		}
	}

	if (override)
	{
		snprintf(&message[strlen(message)], sizeof(message) - 1 - strlen(message),
			"Set light to: %s\n"
			"Set _color to: %s\n"
			"Set angles to: %s\n"
			"Set ambient to: %s\n",
			worldspawn->getKeyValue("light"),
			worldspawn->getKeyValue("_color"),
			worldspawn->getKeyValue("angles"),
			worldspawn->getKeyValue("ambient")
		);
	}

	// no errors - no warnings
	if (!strlen(message))
		return;

	*returnMsg = message;
}

/**
 * @brief Will check e.g. the map entities for valid values
 * @todo: Check whether all misc_model and func_breakable have spawnflags
 * @todo: check for maxlevel
 */
void check_map_values (char **returnMsg)
{
	static char message[1024];
	int count = 0;
	int teams = 0;
	Entity* worldspawn;
	char str[64];

	worldspawn = Scene_FindEntityByClass("worldspawn");
	if (!worldspawn)
	{
		globalOutputStream() << "UFO:AI: Could not find worldspawn.\n";
		*returnMsg = "Could not find worldspawn";
		return;
	}

	*message = '\0';
	*str = '\0';

	// multiplayer start positions
	get_team_count("info_player_start", &count, &teams);
	if (!count)
		strncat(message, "No multiplayer start positions (info_player_start)\n", sizeof(message) - 1);
	else if (string_empty(worldspawn->getKeyValue("maxteams")))
	{
		snprintf(message, sizeof(message) - 1, "Worldspawn: No maxteams defined (#info_player_start) (set to: %i)\n", teams);
		snprintf(str, sizeof(str) - 1, "%i", teams);
		worldspawn->setKeyValue("maxteams", str);
	}
	else if (teams != atoi(worldspawn->getKeyValue("maxteams")))
		snprintf(message, sizeof(message) - 1, "Worldspawn: Settings for maxteams (%s) doesn't match team count (%i)\n", worldspawn->getKeyValue("maxteams"), teams);

	// singleplayer map?
	count = 0;
	get_team_count("info_human_start", &count, NULL);
	if (!count)
		strncat(message, "No singleplayer start positions (info_human_start)\n", sizeof(message) - 1);

	// search for civilians
	count = 0;
	get_team_count("info_civilian_start", &count, NULL);
	if (!count)
		strncat(message, "No civilian start positions (info_civilian_start)\n", sizeof(message) - 1);

	// check maxlevel
	if (string_empty(worldspawn->getKeyValue("maxlevel")))
		strncat(message, "Worldspawn: No maxlevel defined\n", sizeof(message) - 1);
	else if (atoi(worldspawn->getKeyValue("maxlevel")) > 8)
	{
		strncat(message, "Worldspawn: Highest maxlevel is 8\n", sizeof(message) - 1);
		worldspawn->setKeyValue("maxlevel", "8");
	}
	// no errors - no warnings
	if (!strlen(message))
		return;

	*returnMsg = message;
}
