/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
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

#ifndef __SYNAPSE_H__
#define __SYNAPSE_H__

/*!
   synapse library
   code and utilities to deal with dynamic components programming

   "the point at which a nervous impulse passes from one neuron to another"

   dependencies:
   libxml for parsing
   STL for some algorithms and data structures
   glib for Str.h (Str class)

   this is a utility library, it provides typical synapse client and server
   could be split into two independent libraries actually, the server part and the client part
   (that's just a matter of reducing binary size)
 */

// compile time settings
#ifdef _DEBUG
  #define SYNAPSE_VERBOSE // be verbose about the loading process
#endif

// ydnar: required for os x
#if defined ( __APPLE__ )
  #include <sys/types.h>
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <dlfcn.h>
  #include <dirent.h>
#endif

#if defined( _WIN32 )
  #include <windows.h>
#endif

#if defined( _WIN32 )
  #define SYNAPSE_DLL_EXPORT WINAPI
#elif defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )  /* ydnar */
  #define SYNAPSE_DLL_EXPORT
#else
  #error unknown architecture
#endif

// NOTE TTimo: VC6 crap, gets confused when some variable names in function declarations
//   are 'allocator' or 'list'
//   if you #include glib *after* STL, you get those errors .. better be safe then
#include <glib.h>

#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/parser.h"

#include "irefcount.h"
#include "gtkr_list.h"
#include "gtkr_vector.h"

#include "str.h"

/*!
   use when API change make things incompatible at synapse level
   i.e. entry point and classes API changes
 */
#define SYNAPSE_VERSION "3"

/*!
   =======================================================================
   diagnostic printing facility
   independently from any API negotiation stuff,
   we need a diagnostic facility that's available at all times
   =======================================================================
 */
extern "C"
{
/*!
   prototype to provide to synapse to redirect the output appropriately
 */
typedef void ( *PFN_SYN_PRINTF_VA )( const char *text, va_list args );
void Set_Syn_Printf( PFN_SYN_PRINTF_VA pf ); ///< change the handler, set back to NULL for default
/*!
   use this for synapse code diagnostics, it will be piped through the handler if necessary
 */
void Syn_Printf( const char *text, ... );
};

/*
   =======================================================================
   client
   =======================================================================
 */

/*!
   description of an API:
   a module requires and provides several APIs
   the basic rule is that we will avoid asking an API from a module if the APIs it requires are not filled in yet
   the exception being the 'resolve' operation of a given client, which we 'activate'
   (that is we make the interfaces it provides available, leave the ones it requires unsolved, and try to get back to a stable situation)
 */

typedef enum { SYN_UNKNOWN = 0, SYN_PROVIDE, SYN_REQUIRE, SYN_REQUIRE_ANY } EAPIType;

#define MAX_APINAME 128
typedef struct APIDescriptor_s
{
	/*!
	   major version, this must be UNIQUE for each API
	   NOTE: we used to rely on GUID for this, that was a good solution to make sure we never get conflicts
	   but it was a bit overkill, so we dropped and use a string now
	 */
	char major_name[MAX_APINAME];
	/*!
	   what kind of interface
	   for instance for "image" API, "tga" "jpg" etc.
	 */
	char minor_name[MAX_APINAME];
	EAPIType mType; ///< is this an API we provide or an API we require
	/*!
	   pointer to the table to be filled in
	   this is valid for SYN_REQUIRE APIs only
	 */
	void *mpTable;
	bool mbTableInitDone; ///< turned to true by the server after the function table has been filled in
	/*!
	   gives the size of the expected function table
	 */
	int mSize;
	/*!
	   refcounts how many times this API is being used through the app
	   this is valid for SYN_PROVIDE APIs only
	 */
	int mRefCount;
} APIDescriptor_t;

typedef struct XMLConfigEntry_s {
	const char *api;
	EAPIType type;
	int size;
	void *pTable;
} XMLConfigEntry_t;

/*!
   \class CSynapseAPIManager
   derive from this class if you want to manage several APIs through the same object
   (typically, loading plugins, or an unknown number of APIs that match some criteria)
   this class has some pure virtual members that need to be implemented by the childs

   we deal with two types of API managers:
   - the 'loose' ones have a matching pattern and load everything that matches criteria
   typically used for plugins
   - the 'list' ones have a fixed list of things they require. They are used to provide
   easy access to multiple interfaces

   those two types of managers are not stored in the same structs, and not handled the
   same way. For instance the 'list' manager will require ALL it's APIs to be loaded, or
   the init will fail. They also play a role in the client activation.

   apart from the multiple API management facility, the main difference with static tables
   and individual calls to CSynapseClient::AddAPI is the fact that the APIDescriptor_t are
   allocated on demand
 */

/* we do some matching, or store minors list, the strings need to be bigger */
#define MAX_PATTERN_STRING 512

/*! \enum EAPIManagerType
   \brief type of this manager, loosely matching with "*", or a fixed list of required interfaces
 */
typedef enum { API_MATCH = 0,  API_LIST } EAPIManagerType;

class CSynapseAPIManager : public IRefCounted
{
EAPIManagerType mType;

// the list of APIs we have obtained (SYN_REQUIRE_ANY)
vector< APIDescriptor_t * > mAPIs;
/*!
   pattern for matching the major version
   NOTE: only supported for now: exact match
 */
char major_pattern[MAX_PATTERN_STRING];
/*!
   pattern for matching the minor
 */
char minor_pattern[MAX_PATTERN_STRING];

public:
CSynapseAPIManager() { mType = API_MATCH; }
virtual ~CSynapseAPIManager();

EAPIManagerType GetType() { return mType; }
void SetType( EAPIManagerType type ) { mType = type; }

/*!
   set the API matching pattern
   supported syntax:
   any minor for a given major, for instance: PLUGIN_MAJOR, "*"
   a space separated list of minors for a given major: IMAGE_MAJOR, "tga jpg"
 */
void SetMatchAPI( const char *major, const char *minor );

/*!
   utility function
   start building a SYN_REQUIRE_ANY descriptor from a SYN_PROVIDE interface that we found matching
 */
static APIDescriptor_t* PrepareRequireAPI( APIDescriptor_t *pAPI );

/*!
   for managers that require a fixed list of things, we are not active until everything has been loaded up
   managers that work on a loose pattern like "*" are always active (since they don't know what they want for sure)
 */
bool CheckSetActive();

/*!
   the manager answers whether it wants to load this or not
   we provide a default implementation, but this can be completely overridden if needed
   see SetMatchAPI for the documentation of the default implementation
   NOTE: this should only be called on API_MATCH type of managers
 */
virtual bool MatchAPI( const char *major, const char *minor );

/*!
   build an APIDescriptor_t configured as SYN_REQUIRE_ANY from the SYN_PROVIDE API we found
   used when we scan the available interfaces for a match that would be interesting to this manager
   NOTE: only for API_MATCH managers
 */
virtual APIDescriptor_t *BuildRequireAPI( APIDescriptor_t *pAPI ) { return NULL; }

/*!
   below is relevant to API_LIST only ---------------------------------------------------------------------
 */

/*!
   fill in the table info to this descriptor, store it as a new slot
   NOTE: only for API_LIST
 */
virtual void FillAPITable( APIDescriptor_t *pAPI ) { }

/*!
   initialize the list of APIDescriptor_t* with all the stuff we expect
 */
void InitializeAPIList();

/*!
   access the API descriptors
 */
int GetAPICount();
APIDescriptor_t *GetAPI( int );
};

/*!
   \class CSynapseClient
 */
class CSynapseServer; // forward declare
class CSynapseClient : public IRefCounted
{
/*!
   this flag indicates whether this client is active
   i.e. whether you can ask it for interfaces
   this is either a client for which all required interfaces have been filled in
   or a client we are trying to resolve (i.e. load with all it's stuff)
 */
bool mbActive;

/*!
   we store APIDescriptor_t*, the module fills that in at startup
 */
vector<APIDescriptor_t *> mAPIDescriptors;

/*!
   managers for multiple APIs management
   mManagersMatch are managers with loose matching / undefined number of APIs
   mManagersList are managers with a fixed list of required interfaces
 */
vector<CSynapseAPIManager *> mManagersMatch;
vector<CSynapseAPIManager *> mManagersList;

protected:
friend class CSynapseServer;
/*!
   use of this is restricted to the server, expecting it knows what it is doing
   will make the client we are trying to resolve able to provide interfaces even though all interfaces it requires have not been filled in yet.
 */
void ForceSetActive() { mbActive = true; }

public:
CSynapseClient();
virtual ~CSynapseClient();

int GetAPICount() const;   ///< returns the number of APIs that this module provides
APIDescriptor_t* GetAPIDescriptor( int ) const; ///< retrieve specific information about on of the APIs

/*!
   Add the API to the CSynapseClient information

   \param minor
   minor can be NULL, some APIs don't need to have a 'minor' description

   \param type
   SYN_PROVIDE: means this is an API we provide if anyone needs it
   SYN_REQUIRE: means this is an API we will require for operation
   SYN_REQUIRE_ANY: means this is an API we want to load *any* minor found
   (for instance a list of image formats, or the plugins)

   \param pTable
   the function table
   only valid for SYN_REQUIRE APIs

   \param size
   the size of the function table
   if SYN_REQUIRE, you should set the size in pTable and AddAPI will work it out
   if SYN_PROVIDE, you need to provide this parameter

   returns a bool:
   operation may fail, since we have a few safe checks
 */
bool AddAPI( const char *major, const char *minor = NULL, int size = 0, EAPIType type = SYN_PROVIDE, void *pTable = NULL );

/*!
   Add an API manager to the client
   this class is designed to handle multiple APIs
   is not memory managed by CSynapseClient (should it? or ref counted maybe?)
   writing it with support for multiple managers, that may be a bit overkill right now
 */
void AddManager( CSynapseAPIManager *pManager );

int GetManagerMatchCount();   ///< how many API managers
CSynapseAPIManager* GetManagerMatch( int ); ///< get corresponding API manager

int GetManagerListCount();   ///< how many API managers
CSynapseAPIManager* GetManagerList( int ); ///< get corresponding API manager

/*!
   each client has to implement this function itself
   it will fill in the function table
   and increment the ref counting in it's own SYN_PROVIDE descriptor
   returns a bool, false if you ask for an API that's not available
 */
virtual bool RequestAPI( APIDescriptor_t *pAPI ) = 0;

/*!
   return the build date, can be overridden by client module
 */
virtual const char* GetInfo();

/*!
   \brief a shirt name to identify the client
   we use this string to identify individual clients, for instance when some XML configuration nodes are required
   should be unique, the synapse server should't accept multiple occurences?
 */
virtual const char* GetName() { return ""; }

bool IsActive() { return mbActive; }
/*!
   check wether all interfaces have been filled in
   in which case we will switch to 'activated' state, that is this client can provide interfaces to others now
 */
bool CheckSetActive();

/*!
   \brief called when the client is being shutdown, before the dlclose happens
   this is the last call before the dlclose, there's no turning back
   just do what you have to do before you die.. decref and stuff
 */
void Shutdown();

/*!
   override this one in clients that need to proceed through some init steps when activated
   if returning false, the init will abort
 */
virtual bool OnActivate() { return true; }

/*!
   \brief walk the XML config and initialize from structures
   when you use this function, OnActivate will also make sure all the interfaces listed were properly initialized
   two tables, one for the regular single interface, one for the listings
   need to store for later and check in OnActivate

   \param pServer, pass the server to talk to
   NOTE: might want to store it in the class if that's needed too often

   \param client_name, the name of the client node to look for. If NULL, use GetName()

   \return whether all APIs given were successfully found in the config
 */
bool ConfigXML( CSynapseServer *pServer, const char *client_name, const XMLConfigEntry_t entries[] );

/*!
  search for a SYN_PROVIDE with that major in this client, return the index, or -1 if fail
 */
APIDescriptor_t * FindProvidesMajor( const char * major ) const;
};

/*!
   prototype for the only exported function needed in a synapse client
 */
#define NAME_SYNAPSE_ENUMERATEINTERFACES "Synapse_EnumerateInterfaces"

class CSynapseServer; // forward declare
typedef CSynapseClient* ( SYNAPSE_DLL_EXPORT * PFN_SYNAPSE_ENUMERATEINTERFACES )( const char *version, CSynapseServer *server );

/*!
   a derived version of CSynapseClient that can be used to provide builtin module without having to go through DLLs
   this is useful for things we want to isolate behind an abstract API, but that we feel better about having present at all times (such as .def class loader)
 */
class CSynapseBuiltinClient : public CSynapseClient
{
public:
CSynapseBuiltinClient() {}
virtual ~CSynapseBuiltinClient() {}

virtual void EnumerateInterfaces( CSynapseServer *server ) = 0;

};

/*
   =======================================================================
   server
   =======================================================================
 */

/*!
   \enum EClientType
   \brief we can have clients that are builtin to a server
 */
typedef enum { SYN_SO, SYN_BUILTIN } EClientType;

/*!
   server side slot for a synapse client
   is OS dependent, except for the ISynapseClient part
 */
class CSynapseClientSlot
{
public:
/*!
   \todo cleanup, make that private with accessors
 */
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
void *mpDLL;   ///< handle to the shared object (invalid if SYN_BUILTIN)
#elif defined( _WIN32 )
HMODULE mpDLL;   ///< handle to the shared object (invalid if SYN_BUILTIN)
#endif
PFN_SYNAPSE_ENUMERATEINTERFACES mpEnumerate;   ///< function pointer to the enumeration entry point (invalid if SYN_BUILTIN)

CSynapseClient *mpClient;   ///< the full client API
Str mFileName;   ///< path to the file

EClientType mType;

/*!
   \brief release the shared object. NOTE: OS dependent
 */
void ReleaseSO();

CSynapseClientSlot() { mpDLL = NULL; mpEnumerate = NULL; mpClient = NULL; mType = SYN_SO; }
/*!
   NOTE: the slot is stored as static object, and copy constructors used
 */
virtual ~CSynapseClientSlot() { }

};

/*!
   \class CSynapseServer
   dynamic modules manager class
   this class provides the server functionality:
   initialize, get a list of modules, load them, link them together..
 */
class CSynapseServer : public IRefCounted
{
list<char *> mSearchPaths;
list<CSynapseClientSlot> mClients;

/*!
   used for resolve operations
 */
list<APIDescriptor_t*> mStack;
/*!
   set this when mStack is modified with new stuff to resolve
   NOTE: if this hack becomes too tricky to use we could just encapsulate mStack
 */
bool mbStackChanged;

xmlDocPtr mpDoc;
xmlNodePtr mpFocusedNode;   ///< currently focused node while we are scanning the config (strictly for GetNextConfig usage)
xmlNodePtr mpCurrentClientConfig;
/*!
   stores the allocated strings for each call to GetNextConfig
   need to be freed if != NULL
 */
xmlChar *m_api_name;
gchar *m_content;

/*!
   push required interfaces for this client into the stack of things to be resolved
   it is possible that several matching interfaces be in the stack at the same time
   (for instance several modules that want to get the VFS)
   but we should never have the same APIDescriptor_t twice
   NOTE: as this function is called repeatedly during the resolve (because the list of required things is refining),
   we often have to drop APIDescriptor_t requests that are already there.
   NOTE CSynapseAPIManager: if there are CSynapseAPIManager objects in the CSynapseClient,
   we will scan and push all the matching APIs too
 */
void PushRequired( CSynapseClient *pClient );

/*!
   work on resolving this particular APIDescriptor_t
   returns true if we were able to resolve the interface
   returns false otherwise
   if the API was found, but not requested because of more required APIs, we push them in mStack
 */
bool ResolveAPI( APIDescriptor_t* pAPI );

/*!
   push an APIDescriptor_t* into the stack of things to be resolved
   will check that this is not already present first
   will update the mbStackChanged flag
 */
void TryPushStack( APIDescriptor_t * );

/*!
   \brief 'client shutdown' (see libs/synapse/docs/unload.txt)
   performs a 'client shutdown'
   will free the DLL module
   before calling here, the client must be in a 'non active' state
   (i.e. it was not used at all during startup, or we have properly done a 'release' already)
   we scan the mStack for the SYN_REQUIRE that this client owns, and remove them
   \param iSlot is an mClients iterator, invalid when the function returns as the item will have been removed from the list
   \return the iterator after erase call so that the caller iteration can continue
 */
list<CSynapseClientSlot>::iterator ShutdownClient( list<CSynapseClientSlot>::iterator iSlot );

/*!
   \brief actual implementation of the Resolve function
 */
bool DoResolve( CSynapseClient *pClient );

public:
CSynapseServer();
virtual ~CSynapseServer();

void AddSearchPath( char* ); ///< add a new directory to the module search path
/*!
   do the big thing, scan for modules, scan their APIs, load up everything
   providing pf is optional, will set the diagnostics printing
   \param conf_file is the XML configuration file for the initialization (see docs/runtime.txt)
   \return false if the init failed (for instance not found/invalid configuration file
 */
bool Initialize( const char* conf_file = NULL, PFN_SYN_PRINTF_VA pf = NULL );

/*!
   enumerate the interfaces for a given module
   this will load it, query it's entry point, and request the APIs
 */
void EnumerateInterfaces( Str & );

/*!
   enumerate the interfaces for a module that is builtin to the server
 */
void EnumerateBuiltinModule( CSynapseBuiltinClient * );

/*!
   \brief resolve the function table loading for this client
   if the client is not listed in the known slots yet, it will be added
   wraps around internal DoResolve implementation to unload the unused modules
   \return whether the resolution has been successful
 */
bool Resolve( CSynapseClient *pClient );

/*!
   \brief shutdown all the clients. Should only be called when the core is about to exit
   this will force all clients to shutdown. it may destroy refcounted APIs and such
   \todo have to use the release/refresh code before doing actual shutdown
   (i.e. when that code is written later on)
   we need to 'broadcast' to all the clients .. that all the modules are going to be reloaded sorta
   should clear up as many interfaces as possible to avoid unexpected crashes in the final stages of app exit
 */
void Shutdown();

/*!
   diagnostic print function
   NOTE:
   it is essential that those functions should be virtual,
   otherwise when accessing the g_pPrintf global we could mismatch
   (happens because the same library is linked into server and client)
 */
virtual PFN_SYN_PRINTF_VA Get_Syn_Printf();

/*!
   \return true if those APIs are matching
   we provide two APIs for convenience, actual implementation is MatchAPI
   the minors have to be both NULL, or equal, or one the minors be '*'
   NOTE: the '*' minor should ONLY be used on an API that will be unique. It is hackish and kinda dangerous
 */
bool MatchAPI( APIDescriptor_t *p1, APIDescriptor_t *p2 );
/*!
  \return 0: not matching
  \return 1: matching
  \return 2: do extended checks for a minor of "*"
 */
static int MatchAPI( const char* major1, const char* minor1, const char* major2, const char* minor2 );

#if defined( _WIN32 )
/*!
   utility function to retrieve formatted GetLastError message
   ANSI text, static string
 */
static const char* FormatGetLastError();
#endif

/*!
   dump the stack of interfaces to be solved
   this is used when synapse initialization failed to quickly identify the missing/broken pieces
 */
void DumpStack();

/*!
   general purpose information, list what modules are loaded up
 */
void DumpActiveClients();

/*!
   \brief select the config node that has this name
   call this to locate the right node in XML config
   this will focus and get ready to walk through the api nodes
   \return wether the config node was correctly selected
 */
bool SelectClientConfig( const char *client_name );

/*!
   \brief walk through the apis
   the pointers don't need to be freed
   you need to copy them over as they are invalidated between each call to GetNextConfig
   \return false when all apis have been parsed
 */
bool GetNextConfig( char **api_name, char **minor );

/*!
   \brief read the minor for a given api in the current config
   \return false if this node doesn't exist
 */
bool GetConfigForAPI( const char *api, char **minor );

/*!
   returns the filename of the module that the passed on client exists in
 */
const char *GetModuleFilename( CSynapseClient *pClient );

/*!
  look for a client that is active, and provides a specific major
  \return 0: not found
  \return 1: single found
  \return 2: multiple found
 */
int FindActiveMajorClient( const char * major, APIDescriptor_t ** ret ) const;

};

#endif
