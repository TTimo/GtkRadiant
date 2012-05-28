
#if !defined( INCLUDED_MODULEOBSERVERS_H )
#define INCLUDED_MODULEOBSERVERS_H

#include "debugging/debugging.h"
#include <set>
#include "moduleobserver.h"

class ModuleObservers
{
typedef std::set<ModuleObserver*> Observers;
Observers m_observers;
public:
~ModuleObservers(){
	ASSERT_MESSAGE( m_observers.empty(), "ModuleObservers::~ModuleObservers: observers still attached" );
}
void attach( ModuleObserver& observer ){
	ASSERT_MESSAGE( m_observers.find( &observer ) == m_observers.end(), "ModuleObservers::attach: cannot attach observer" );
	m_observers.insert( &observer );
}
void detach( ModuleObserver& observer ){
	ASSERT_MESSAGE( m_observers.find( &observer ) != m_observers.end(), "ModuleObservers::detach: cannot detach observer" );
	m_observers.erase( &observer );
}
void realise(){
	for ( Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i )
	{
		( *i )->realise();
	}
}
void unrealise(){
	for ( Observers::reverse_iterator i = m_observers.rbegin(); i != m_observers.rend(); ++i )
	{
		( *i )->unrealise();
	}
}
};

#endif
