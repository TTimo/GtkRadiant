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

#include "xmltextags.h"

#include <string>

#include "qerplugin.h"
#include "stream/stringstream.h"

XmlTagBuilder::XmlTagBuilder()
{
}

XmlTagBuilder::~XmlTagBuilder()
{
  // clean up
  xmlFreeDoc(doc);
  xmlXPathFreeContext(context);
}

bool XmlTagBuilder::CreateXmlDocument()
{
  /* Creates an XML file

     returns TRUE if the file was created successfully or FALSE when failed
  */

  xmlTextWriterPtr writer;

  writer = xmlNewTextWriterDoc(&doc, 0);

  // begin a new UTF-8 formatted xml document
  xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

  // create the root node with stock and custom elements
  xmlTextWriterStartElement(writer, (xmlChar*)"root");
  xmlTextWriterWriteString(writer, (xmlChar*)"\n  ");
  xmlTextWriterStartElement(writer, (xmlChar*)"stock");
  xmlTextWriterWriteString(writer, (xmlChar*)"\n  ");
  xmlTextWriterEndElement(writer);
  xmlTextWriterWriteString(writer, (xmlChar*)"\n  ");
  xmlTextWriterStartElement(writer, (xmlChar*)"custom");
  xmlTextWriterWriteString(writer, (xmlChar*)"\n  ");
  xmlTextWriterEndElement(writer);
  xmlTextWriterWriteString(writer, (xmlChar*)"\n");
  xmlTextWriterEndElement(writer);

  // end of the xml document
  xmlTextWriterEndDocument(writer);
  xmlFreeTextWriter(writer);

  if(!doc)
  {
    return false;
  } else {
    context = xmlXPathNewContext(doc);
	return true;
  }
}

bool XmlTagBuilder::OpenXmlDoc(const char* file, const char* savefile)
{
  /* Reads a XML document from a file

     returns TRUE if the document was read successfully or FALSE when failed
  */

  if(savefile)
    m_savefilename = savefile;
  else
    m_savefilename = file;
    
  doc = xmlParseFile(file);	// TODO error checking!

  if(!doc)
  {
    return false;
  } else {
    context = xmlXPathNewContext(doc);
	return true;
  }
}

bool XmlTagBuilder::SaveXmlDoc(void)
{
	return SaveXmlDoc(m_savefilename.c_str());
}

bool XmlTagBuilder::SaveXmlDoc(const char* file)
{
  /* Writes the XML document

     returns TRUE if the document was saved successfully or FALSE when saving failed
  */
  
  xmlSaveNoEmptyTags = 1;

  if(xmlSaveFile(file, doc) != -1)
  {
    return true;
  }
  return false;
}

bool XmlTagBuilder::AddShaderNode(const char* shader, TextureType textureType, NodeShaderType nodeShaderType)
{
  /* Adds a shader node

     char* shader - the name of the shader or texture (without trailing .tga or something)

     returns TRUE if the node was added successfully or FALSE when failed
  */

  xmlNodeSetPtr nodePtr = NULL;
  xmlXPathObjectPtr xpathPtr = NULL;
  
  switch(textureType)
  {
    case STOCK:
      xpathPtr = XpathEval("/root/stock");
      break;
    case CUSTOM:
      xpathPtr = XpathEval("/root/custom");
  };
  
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
    return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlNodePtr newnode, newtext;
    xmlNodePtr nodeParent = nodePtr->nodeTab[0];

    // create a new node and set the node attribute (shader path)
    switch(nodeShaderType)
    {
      case SHADER:
        newnode = xmlNewNode(NULL, (xmlChar*)"shader");
        break;
      case TEXTURE:
        newnode = xmlNewNode(NULL, (xmlChar*)"texture");
      };

      newnode = xmlDocCopyNode(newnode, doc, 1);
      xmlSetProp(newnode, (xmlChar*)"path", (xmlChar*)shader);
      xmlNodeSetContent(newnode, (xmlChar*)"\n    ");

      if(nodePtr->nodeTab[0]->children->next == NULL)   // there are no shaders yet
      {
        // add spaces
        newtext = xmlNewText((xmlChar*)"  ");
        xmlAddChild(nodeParent->children, newtext);

        // add the new node
        xmlAddNextSibling(nodeParent->children, newnode);

        // append a new line
        newtext = xmlNewText((xmlChar*)"\n  ");
        xmlAddNextSibling(nodeParent->children->next, newtext);
      } else {
        // add the node
        xmlAddNextSibling(nodeParent->children, newnode);

        // append a new line and spaces
        newtext = xmlNewText((xmlChar*)"\n    ");
        xmlAddNextSibling(nodeParent->children->next, newtext);
      }
      
      xmlXPathFreeObject(xpathPtr);
      return true;
  } else {
    xmlXPathFreeObject(xpathPtr);
    return false;
  }
}

bool XmlTagBuilder::DeleteShaderNode(const char* shader)
{
  /* Deletes a shader node

     char* shader - the name of the shader or texture (without trailing .tga or something)

     returns TRUE if the node was deleted successfully or FALSE when failed
  */

  char buffer[256];
  char* expression = GetTagsXpathExpression(buffer, shader, EMPTY);
  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
  	nodePtr = xpathPtr->nodesetval;
  else
  	return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlNodePtr ptrContent = nodePtr->nodeTab[0];
    xmlNodePtr ptrWhitespace = nodePtr->nodeTab[0]->prev;

    // delete the node
    xmlUnlinkNode(ptrContent);
    xmlFreeNode(ptrContent);

    // delete leading whitespace node
    xmlUnlinkNode(ptrWhitespace);
    xmlFreeNode(ptrWhitespace);
    xmlXPathFreeObject(xpathPtr);
    return true;
  }
  xmlXPathFreeObject(xpathPtr);
  return false;
}

bool XmlTagBuilder::CheckShaderTag(const char* shader)
{
  /* Checks whether there exists an entry for a shader/texture with at least one tag

     char* shader  - the name of the shader or texture (without trailing .tga or something)

     returns TRUE if the shader is already stored in the XML tag file and has at least one tag
  */

  // build the XPath expression to search for
  char buffer[256];
  strcpy(buffer, "/root/*/*[@path='");
  strcat(buffer, shader);
  strcat(buffer, "']");

  char* expression = buffer;

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlXPathFreeObject(xpathPtr);
    return true;
  } else {
    xmlXPathFreeObject(xpathPtr);
    return false;
  }
}

bool XmlTagBuilder::CheckShaderTag(const char* shader, const char* content)
{
  /* Checks whether a tag with content already exists

     char* shader  - the name of the shader or texture (without trailing .tga or something)
     char* content - the node content (a tag name)

     returns TRUE if the tag with content already exists or FALSE if not
  */

  // build the XPath expression to search for
  // example expression: "/stock/*[@path='textures/alpha/barb_wire'][child::tag='Alpha']";

  char buffer[256];
  strcpy(buffer, "/root/*/*[@path='");
  strcat(buffer, shader);
  strcat(buffer, "'][child::tag='");
  strcat(buffer, content);
  strcat(buffer, "']");

  char* expression = buffer;

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlXPathFreeObject(xpathPtr);
    return true;
  } else {
    xmlXPathFreeObject(xpathPtr);
    return false;
  }
}

bool XmlTagBuilder::AddShaderTag(const char* shader, const char* content, NodeTagType nodeTagType)
{
  /* Adds a tag node to an existing shader/texture node if there's no tag with the same content yet

     char* shader  - the name of the shader or texture (without trailing .tga or something)
     char* content - the node content (a tag name)

     returns TRUE if the node was added successfully or FALSE when failed
  */

  // build the XPath expression
  char buffer[256];
  char* expression = GetTagsXpathExpression(buffer, shader, EMPTY);
  
  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))	// node was found
  {
    xmlNodePtr newnode = xmlNewNode(NULL, (xmlChar*)"tag");
    xmlNodePtr nodeParent = nodePtr->nodeTab[0];
    newnode = xmlDocCopyNode(newnode, doc, 1);
    xmlNodeSetContent(newnode, (xmlChar*)content);

    if(nodePtr->nodeTab[0]->children->next == NULL)	// shader node has NO children
    {
      // add spaces
      xmlNodePtr newtext = xmlNewText((xmlChar*)"  ");
      xmlAddChild(nodeParent->children, newtext);

      // add new node
      xmlAddNextSibling(nodeParent->children, newnode);

      // append a new line + spaces
      newtext = xmlNewText((xmlChar*)"\n    ");
      xmlAddNextSibling(nodeParent->children->next, newtext);
    } else { // shader node has children already - the new node will be the first sibling
      xmlAddNextSibling(nodeParent->children, newnode);
	  xmlNodePtr newtext = xmlNewText((xmlChar*)"\n      ");
      xmlAddNextSibling(nodeParent->children->next, newtext);
    }
	xmlXPathFreeObject(xpathPtr);
    return true;
  } else {
    xmlXPathFreeObject(xpathPtr);
    return false;
  }
}

//int XmlTagBuilder::RenameShaderTag(const char* oldtag, const char* newtag)
int XmlTagBuilder::RenameShaderTag(const char* oldtag, CopiedString newtag)
{
  /* Replaces tag node contents

     char* oldtag - the <tag></tag> node content that sould be changed
     char* newtag - the new <tag></tag> node content

     returns the number of renamed shaders
  */

  int num = 0;

  // build the XPath expression
  char expression[256];
  strcpy(expression, "/root/*/*[child::tag='");
  strcat(expression, oldtag);
  strcat(expression, "']/*");

  xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*)expression, context);
  if(!result)
    return 0;
  xmlNodeSetPtr nodePtr = result->nodesetval;

  for(int i = 0; i < nodePtr->nodeNr; i++)
  {
    xmlNodePtr ptrContent = nodePtr->nodeTab[i];
    char* content = (char*)xmlNodeGetContent(ptrContent);

    if(strcmp(content, oldtag) == 0)	// found a node with old content?
    {
      xmlNodeSetContent(ptrContent, (xmlChar*)newtag.c_str());
      num++;
    }
  }

  SaveXmlDoc();
  xmlXPathFreeObject(result);// CHANGED
  return num;
}

bool XmlTagBuilder::DeleteShaderTag(const char* shader, const char* tag)
{
  /* Deletes a child node of a shader

     char* shader  - the name of the shader or texture (without trailing .tga or something)
     char* tag     - the tag being deleted

     returns TRUE if the node was deleted successfully or FALSE when failed
  */

  char buffer[256];
  char* expression = GetTagsXpathExpression(buffer, shader, TAG);
  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return false;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    for(int i = 0; i < nodePtr->nodeNr; i++)
    {
      xmlNodePtr ptrContent = nodePtr->nodeTab[i];
      char* content = (char*)(xmlChar*)xmlNodeGetContent(ptrContent);

      if(strcmp(content, tag) == 0)	// find the node
      {
        xmlNodePtr ptrWhitespace = nodePtr->nodeTab[i]->prev;
        // delete the node
        xmlUnlinkNode(ptrContent);
        xmlFreeNode(ptrContent);

        // delete leading whitespace node
        xmlUnlinkNode(ptrWhitespace);
        xmlFreeNode(ptrWhitespace);
        xmlXPathFreeObject(xpathPtr);
        return true;
      }
    }
  }
  xmlXPathFreeObject(xpathPtr);
  return false;
}

bool XmlTagBuilder::DeleteTag(const char* tag)
{
  /* Deletes a tag from all shaders

     char* tag - the tag being deleted from all shaders

     returns TRUE if the tag was deleted successfully or FALSE when failed
  */

  char expression[256];
  strcpy(expression, "/root/*/*[child::tag='");
  strcat(expression, tag);
  strcat(expression, "']");

  std::set<CopiedString> dellist;
  TagSearch(expression, dellist);
  std::set<CopiedString>::iterator iter;

  for(iter = dellist.begin(); iter != dellist.end(); iter++)
  {
    DeleteShaderTag(iter->c_str(), tag);
  }
  SaveXmlDoc();

  return true;
}

void XmlTagBuilder::GetShaderTags(const char* shader, std::vector<CopiedString>& tags)
{
  /* Gets the tags from a shader

     char* shader - the name of the shader

     returns a vector containing the tags
  */

  char* expression;

  if(shader == NULL)	// get all tags from all shaders
  {
    expression = "/root/*/*/tag";
  } else {
    char buffer[256];
    expression = GetTagsXpathExpression(buffer, shader, TAG);
  }

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    for(int i = 0; i < nodePtr->nodeNr; i++)
    {
      tags.push_back((CopiedString)(char*)xmlNodeGetContent(nodePtr->nodeTab[i]));
    }
  }
  xmlXPathFreeObject(xpathPtr);
}

void XmlTagBuilder::GetUntagged(std::set<CopiedString>& shaders)
{
  /* Gets all textures and shaders listed in the xml file that don't have any tag

     returns a set containing the shaders (with path)
  */

  char* expression = "/root/*/*[not(child::tag)]";

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return;
 
  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlNodePtr ptr;

    for(int i = 0; i < nodePtr->nodeNr; i++)
    {
      ptr = nodePtr->nodeTab[i];
	  shaders.insert((char*)xmlGetProp(ptr, (xmlChar*)"path"));
    }
  }
  
  xmlXPathFreeObject(xpathPtr);
}

void XmlTagBuilder::GetAllTags(std::set<CopiedString>& tags)
{
  /* Gets a list of all tags that are used (assigned to any shader)

     returns a set containing all used tags
  */

  char* expression = "/root/*/*/tag";

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return;
 
  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    for(int i = 0; i < nodePtr->nodeNr; i++)
    {
      tags.insert((CopiedString)(char*)xmlNodeGetContent(nodePtr->nodeTab[i]));
    }
  }
  
  xmlXPathFreeObject(xpathPtr);
}

void XmlTagBuilder::TagSearch(const char* expression, std::set<CopiedString>& paths)
{
  /* Searches shaders by tags

     char* expression - the XPath expression to search

     returns a set containing the found shaders
  */

  xmlXPathObjectPtr xpathPtr = XpathEval(expression);
  xmlNodeSetPtr nodePtr;
  if(xpathPtr)
    nodePtr = xpathPtr->nodesetval;
  else
  	return;

  if(!xmlXPathNodeSetIsEmpty(nodePtr))
  {
    xmlNodePtr ptr;
    xmlChar* xmlattrib;
    for(int i = 0; i < nodePtr->nodeNr; i++)
    {
      ptr = nodePtr->nodeTab[i];
      xmlattrib = xmlGetProp(ptr, (xmlChar*)"path");
	  paths.insert((CopiedString)(char*)xmlattrib);
    }
  }
  xmlXPathFreeObject(xpathPtr);
}

