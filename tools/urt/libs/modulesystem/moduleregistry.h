
#if !defined( INCLUDED_MODULESYSTEM_MODULEREGISTRY_H )
#define INCLUDED_MODULESYSTEM_MODULEREGISTRY_H

#include "generic/static.h"
#include <list>

class ModuleRegisterable
{
public:
virtual void selfRegister() = 0;
};

class ModuleRegistryList
{
typedef std::list<ModuleRegisterable*> RegisterableModules;
RegisterableModules m_modules;
public:
void addModule( ModuleRegisterable& module ){
	m_modules.push_back( &module );
}
void registerModules() const {
	for ( RegisterableModules::const_iterator i = m_modules.begin(); i != m_modules.end(); ++i )
	{
		( *i )->selfRegister();
	}
}
};

typedef SmartStatic<ModuleRegistryList> StaticModuleRegistryList;


class StaticRegisterModule : public StaticModuleRegistryList
{
public:
StaticRegisterModule( ModuleRegisterable& module ){
	StaticModuleRegistryList::instance().addModule( module );
}
};


#endif
