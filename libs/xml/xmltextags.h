/*
Copyright (C) 2006, Stefan Greven.
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

#if !defined(INCLUDED_XMLTEXTAGS_H)
#define INCLUDED_XMLTEXTAGS_H

#include <set>
#include <string/string.h>
#include <vector>

#include "iscriplib.h"

#include <libxml/xpath.h>
#include <libxml/xmlwriter.h>

enum NodeTagType
{
  TAG,
  EMPTY
};

enum NodeShaderType
{
  SHADER,
  TEXTURE
};

enum TextureType
{
  STOCK,
  CUSTOM
};

class XmlTagBuilder
{
private:
  CopiedString filename;
  xmlDocPtr doc;
  xmlXPathContextPtr context;
  xmlNodeSetPtr nodePtr;

  xmlXPathObjectPtr XpathEval(const char* queryString)
  {
	xmlChar* expression = (xmlChar*)queryString;
	xmlXPathObjectPtr result = xmlXPathEvalExpression(expression, context);
	return result;
  };

  char* GetTagsXpathExpression(char* buffer, const char* shader, NodeTagType nodeTagType)
  {
	strcpy(buffer, "/root/*/*[@path='");
	strcat(buffer, shader);
	
	switch(nodeTagType)
	{
	case TAG:
		strcat(buffer, "']/tag");
		break;
	case EMPTY:
		strcat(buffer, "']");
	};

	return buffer;
  }

public:
  XmlTagBuilder();
  ~XmlTagBuilder();

  bool CreateXmlDocument();
  bool OpenXmlDoc(const char* file);
  bool SaveXmlDoc(const char* file);
  bool SaveXmlDoc(void);
  bool AddShaderNode(const char* shader, TextureType textureType, NodeShaderType nodeShaderType);
  bool DeleteShaderNode(const char* shader);
  bool CheckShaderTag(const char* shader);
  bool CheckShaderTag(const char* shader, const char* content);
  bool AddShaderTag(const char* shader, const char* content, NodeTagType nodeTagType);
  bool DeleteTag(const char* tag);
  int RenameShaderTag(const char* oldtag, CopiedString newtag);
  bool DeleteShaderTag(const char* shader, const char* tag);
  void GetShaderTags(const char* shader, std::vector<CopiedString>& tags);
  void GetUntagged(std::set<CopiedString>& shaders);
  void GetAllTags(std::set<CopiedString>& tags);
  void TagSearch(const char* expression, std::set<CopiedString>& paths);
};

#endif
