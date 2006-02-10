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

#include "help.h"

#include "debugging/debugging.h"

#include <vector>
#include <list>

#include "libxml/parser.h"
#include "generic/callback.h"
#include "gtkutil/menu.h"
#include "stream/stringstream.h"
#include "os/file.h"

#include "url.h"
#include "preferences.h"
#include "mainframe.h"

/*!
the urls to fire up in the game packs help menus
*/
namespace
{
  std::list<CopiedString> mHelpURLs;
}

/*!
needed for hooking in Gtk+
*/
void HandleHelpCommand(CopiedString& str)
{
  OpenURL(str.c_str());
}

void process_xlink(const char* filename, const char *menu_name, const char *base_url, GtkMenu *menu)
{
  if(file_exists(filename))
  {
    xmlDocPtr pDoc = xmlParseFile(filename);
    if (pDoc)
    {
      globalOutputStream() << "Processing .xlink file '" << filename << "'\n";
      // create sub menu
      GtkMenu* menu_in_menu = create_sub_menu_with_mnemonic(menu, menu_name);
      // start walking the nodes, find the 'links' one
      xmlNodePtr pNode = pDoc->children;
      while (pNode && strcmp((const char*)pNode->name, "links"))
        pNode=pNode->next;
      if (pNode)
      {
        pNode = pNode->children;
        while(pNode)
        {
          if(!strcmp((const char*)pNode->name, "item"))
          {
            // process the URL
            CopiedString url;
            
            xmlChar* prop = xmlGetProp(pNode, reinterpret_cast<const xmlChar*>("url"));
            ASSERT_NOTNULL(prop);
            if(strstr(reinterpret_cast<const char*>(prop), "http://"))
            {
              // complete URL
              url = reinterpret_cast<const char*>(prop);
            }
            else
            {
              // relative URL
              StringOutputStream full(256);
              full << base_url << reinterpret_cast<const char*>(prop);
              url = full.c_str();
            }

            mHelpURLs.push_back(url);

            xmlFree(prop);
            
            prop = xmlGetProp(pNode, reinterpret_cast<const xmlChar*>("name"));
            ASSERT_NOTNULL(prop);
            create_menu_item_with_mnemonic(menu_in_menu, reinterpret_cast<const char*>(prop), ReferenceCaller<CopiedString, HandleHelpCommand>(mHelpURLs.back()));
            xmlFree(prop);
          }
          pNode=pNode->next;
        }
      }
      xmlFreeDoc(pDoc);
    }
    else
    {
      globalOutputStream() << "'" << filename << "' parse failed\n";
    }
  }
  else
  {
    globalOutputStream() << "'" << filename << "' not found\n";
  }
}

void create_game_help_menu(GtkMenu *menu)
{
  StringOutputStream filename(256);
  filename << AppPath_get() << "global.xlink";
  process_xlink(filename.c_str(), "General", AppPath_get(), menu);

#if 1
  filename.clear();
  filename << g_pGameDescription->mGameToolsPath.c_str() << "game.xlink";
  process_xlink(filename.c_str(), g_pGameDescription->getRequiredKeyValue("name"), g_pGameDescription->mGameToolsPath.c_str(), menu);
#else
  for(std::list<CGameDescription *>::iterator iGame = g_GamesDialog.mGames.begin(); iGame != g_GamesDialog.mGames.end(); ++iGame)
  {
    filename.clear();
    filename << (*iGame)->mGameToolsPath.c_str() << "game.xlink";
    process_xlink(filename.c_str(), (*iGame)->getRequiredKeyValue("name"), (*iGame)->mGameToolsPath.c_str(), menu);
  }
#endif
}

