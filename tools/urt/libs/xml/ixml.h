
#if !defined( INCLUDED_XML_IXML_H )
#define INCLUDED_XML_IXML_H

#include "itextstream.h"

class XMLAttrVisitor
{
public:
virtual void visit( const char* name, const char* value ) = 0;
};

class XMLElement
{
public:
virtual const char* name() const = 0;
virtual const char* attribute( const char* name ) const = 0;
virtual void forEachAttribute( XMLAttrVisitor& visitor ) const = 0;
};

class XMLImporter : public TextOutputStream
{
public:
static const char* getTypeName(){
	return "XMLImporter";
}

virtual void pushElement( const XMLElement& element ) = 0;
virtual void popElement( const char* name ) = 0;
};

class XMLExporter
{
public:
static const char* getTypeName(){
	return "XMLExporter";
}

virtual void exportXML( XMLImporter& importer ) = 0;
};


#endif
