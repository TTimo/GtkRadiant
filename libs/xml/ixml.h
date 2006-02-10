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

#if !defined(INCLUDED_XML_IXML_H)
#define INCLUDED_XML_IXML_H

#include "itextstream.h"
#include "generic/constant.h"

class XMLAttrVisitor
{
public:
  virtual void visit(const char* name, const char* value) = 0;
};

class XMLElement
{
public:
  virtual const char* name() const = 0;
  virtual const char* attribute(const char* name) const = 0;
  virtual void forEachAttribute(XMLAttrVisitor& visitor) const = 0;
};

class XMLImporter : public TextOutputStream
{
public:
  STRING_CONSTANT(Name, "XMLImporter");

  virtual void pushElement(const XMLElement& element) = 0;
  virtual void popElement(const char* name) = 0;
};

class XMLExporter
{
public:
  STRING_CONSTANT(Name, "XMLExporter");

  virtual void exportXML(XMLImporter& importer) = 0;
};


#endif
