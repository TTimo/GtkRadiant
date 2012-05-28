
#if !defined ( INCLUDED_INSTANCELIB_H )
#define INCLUDED_INSTANCELIB_H

#include "debugging/debugging.h"

#include "iscenegraph.h"

#include "scenelib.h"
#include "generic/reference.h"
#include "generic/callback.h"
#include <map>

class InstanceSubgraphWalker : public scene::Traversable::Walker
{
scene::Instantiable::Observer* m_observer;
mutable scene::Path m_path;
mutable Stack<scene::Instance*> m_parent;
public:
InstanceSubgraphWalker( scene::Instantiable::Observer* observer, const scene::Path& path, scene::Instance* parent )
	: m_observer( observer ), m_path( path ), m_parent( parent ){
}
bool pre( scene::Node& node ) const {
	m_path.push( makeReference( node ) );
	scene::Instance* instance = Node_getInstantiable( node )->create( m_path, m_parent.top() );
	m_observer->insert( instance );
	Node_getInstantiable( node )->insert( m_observer, m_path, instance );
	m_parent.push( instance );
	return true;
}
void post( scene::Node& node ) const {
	m_path.pop();
	m_parent.pop();
}
};

class UninstanceSubgraphWalker : public scene::Traversable::Walker
{
scene::Instantiable::Observer* m_observer;
mutable scene::Path m_path;
public:
UninstanceSubgraphWalker( scene::Instantiable::Observer* observer, const scene::Path& parent )
	: m_observer( observer ), m_path( parent ){
}
bool pre( scene::Node& node ) const {
	m_path.push( makeReference( node ) );
	return true;
}
void post( scene::Node& node ) const {
	scene::Instance* instance = Node_getInstantiable( node )->erase( m_observer, m_path );
	m_observer->erase( instance );
	delete instance;
	m_path.pop();
}
};

class Instances : public scene::Traversable::Observer
{
typedef std::pair<scene::Instantiable::Observer*, PathConstReference> CachePath;

typedef CachePath key_type;

typedef std::map<key_type, scene::Instance*> InstanceMap;
InstanceMap m_instances;
public:

typedef InstanceMap::iterator iterator;

iterator begin(){
	return m_instances.begin();
}
iterator end(){
	return m_instances.end();
}

// traverse observer
void insert( scene::Node& child ){
	for ( iterator i = begin(); i != end(); ++i )
	{
		Node_traverseSubgraph( child, InstanceSubgraphWalker( ( *i ).first.first, ( *i ).first.second, ( *i ).second ) );
		( *i ).second->boundsChanged();
	}
}
void erase( scene::Node& child ){
	for ( iterator i = begin(); i != end(); ++i )
	{
		Node_traverseSubgraph( child, UninstanceSubgraphWalker( ( *i ).first.first, ( *i ).first.second ) );
		( *i ).second->boundsChanged();
	}
}

// instance
void forEachInstance( const scene::Instantiable::Visitor& visitor ){
	for ( iterator i = begin(); i != end(); ++i )
	{
		visitor.visit( *( *i ).second );
	}
}

void insert( scene::Instantiable::Observer* observer, const scene::Path& path, scene::Instance* instance ){
	ASSERT_MESSAGE( m_instances.find( key_type( observer, PathConstReference( instance->path() ) ) ) == m_instances.end(), "Instances::insert - element already exists" );
	m_instances.insert( InstanceMap::value_type( key_type( observer, PathConstReference( instance->path() ) ), instance ) );
}
scene::Instance* erase( scene::Instantiable::Observer* observer, const scene::Path& path ){
	ASSERT_MESSAGE( m_instances.find( key_type( observer, PathConstReference( path ) ) ) != m_instances.end(), "Instances::erase - failed to find element" );
	InstanceMap::iterator i = m_instances.find( key_type( observer, PathConstReference( path ) ) );
	scene::Instance* instance = i->second;
	m_instances.erase( i );
	return instance;
}

void transformChanged(){
	for ( InstanceMap::iterator i = m_instances.begin(); i != m_instances.end(); ++i )
	{
		( *i ).second->transformChanged();
	}
}
typedef MemberCaller<Instances, &Instances::transformChanged> TransformChangedCaller;
void boundsChanged(){
	for ( InstanceMap::iterator i = m_instances.begin(); i != m_instances.end(); ++i )
	{
		( *i ).second->boundsChanged();
	}
}
typedef MemberCaller<Instances, &Instances::boundsChanged> BoundsChangedCaller;
};

#endif
