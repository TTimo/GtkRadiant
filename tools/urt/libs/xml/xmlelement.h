
#if !defined( INCLUDED_XML_XMLELEMENT_H )
#define INCLUDED_XML_XMLELEMENT_H

#include "xml/ixml.h"
#include "string/string.h"

#include <map>

class StaticElement : public XMLElement
{
struct strless
{
	bool operator()( const char* s1, const char* s2 ) const {
		return strcmp( s1, s2 ) < 0;
	}
};

typedef std::map<const char*, const char*, strless> attrs_t;
public:
StaticElement( const char* name )
	: m_name( name ){
}
void insertAttribute( const char* name, const char* value ){
	m_attrs.insert( attrs_t::value_type( name, value ) );
}
const char* name() const {
	return m_name;
}
const char* attribute( const char* name ) const {
	attrs_t::const_iterator i = m_attrs.find( name );
	if ( i != m_attrs.end() ) {
		return i->second;
	}
	else{
		return "";
	}
}
void forEachAttribute( XMLAttrVisitor& visitor ) const {
	for ( attrs_t::const_iterator i = m_attrs.begin(); i != m_attrs.end(); ++i )
	{
		visitor.visit( i->first, i->second );
	}
}
private:
const char* m_name;
attrs_t m_attrs;
};

class DynamicElement : public XMLElement
{
typedef std::map<CopiedString, CopiedString> attrs_t;
public:
DynamicElement( const char* name )
	: m_name( name )
{}
void insertAttribute( const char* name, const char* value ){
	m_attrs.insert( attrs_t::value_type( name, value ) );
}
const char* name() const {
	return m_name.c_str();
}
const char* attribute( const char* name ) const {
	attrs_t::const_iterator i = m_attrs.find( name );
	if ( i != m_attrs.end() ) {
		return i->second.c_str();
	}
	else{
		return "";
	}
}
void forEachAttribute( XMLAttrVisitor& visitor ) const {
	for ( attrs_t::const_iterator i = m_attrs.begin(); i != m_attrs.end(); ++i )
	{
		visitor.visit( i->first.c_str(), i->second.c_str() );
	}
}
private:
CopiedString m_name;
attrs_t m_attrs;
};

#endif
