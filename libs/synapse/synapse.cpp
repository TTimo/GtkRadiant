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

#include <assert.h>

// seems to be required for str.h
#include <glib.h>
#include <glib/gstdio.h>

#include "synapse.h"
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <dirent.h>
#endif

/*
   ===================================================
   diagnostic stuff
   ===================================================
 */

extern "C"
{

static PFN_SYN_PRINTF_VA g_pPrintf = NULL;

void Set_Syn_Printf( PFN_SYN_PRINTF_VA pf ){
	g_pPrintf = pf;
}

#define BUFFER_SIZE 4096

void Syn_Printf( const char *text, ... ){
	char buf[BUFFER_SIZE];
	va_list args;

	if ( !text ) {
		return;
	}

	if ( g_pPrintf ) {
		va_start( args, text );
		( *g_pPrintf )( text, args );
		va_end( args );
	}
	else
	{
		va_start( args, text );
		vsnprintf( buf, BUFFER_SIZE, text, args );
		buf[BUFFER_SIZE - 1] = 0;
		printf( "%s", buf );
		va_end( args );
	}
}

}

/*
   =======================================================================
   server
   =======================================================================
 */

// this must be kept in sync with EAPIType
static const char* APITypeName[4] =
{
	"SYN_UNKNOWN",
	"SYN_PROVIDE",
	"SYN_REQUIRE",
	"SYN_REQUIRE_ANY"
};

CSynapseServer::CSynapseServer(){
	mpDoc = NULL;
	m_api_name = NULL;
	m_content = NULL;
	mpFocusedNode = NULL;
}

CSynapseServer::~CSynapseServer(){
	if ( m_api_name ) {
		xmlFree( m_api_name );
                m_api_name = NULL;
	}
	if ( m_content ) {
		g_free( m_content );
	}
	Syn_Printf( "TODO: free API managers\n" );
}

void CSynapseServer::AddSearchPath( char* path ){
	char *pLocalPath = new char[strlen( path ) + 1];
	strcpy( pLocalPath, path );
	mSearchPaths.push_front( pLocalPath );
}

bool CSynapseServer::Initialize( const char* conf_file, PFN_SYN_PRINTF_VA pf ){
	// browse the paths to locate all potential modules

	Set_Syn_Printf( pf );

	if ( conf_file ) {
		// if a config file is specified and we fail to load it, we fail
		Syn_Printf( "loading synapse XML config file '%s'\n", conf_file );
		mpDoc = xmlParseFile( conf_file );
		if ( !mpDoc ) {
			Syn_Printf( "'%s' invalid/not found\n", conf_file );
			return false;
		}
	}

	for ( list<char *>::iterator iPath = mSearchPaths.begin(); iPath != mSearchPaths.end(); iPath++ )
	{
		const char* path = *iPath;

		Syn_Printf( "Synapse Scanning modules path: %s\n", path );

		GDir* dir = g_dir_open( path, 0, NULL );

		if ( dir != NULL ) {
			while ( 1 )
			{
				const gchar* name = g_dir_read_name( dir );
				if ( name == NULL ) {
					break;
				}

				// too small to be isolated in win32/ and linux/ directories..
#if defined( _WIN32 )
				const char* ext_so = ".dll";
#elif defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
				const char* ext_so = ".so";
#endif
				const char* ext = strrchr( name, '.' );
				if ( ( ext == NULL ) || ( stricmp( ext, ext_so ) != 0 ) ) {
					continue;
				}

				Str newModule;
				newModule.Format( "%s%s", path, name );
				Syn_Printf( "Found '%s'\n", newModule.GetBuffer() );
				EnumerateInterfaces( newModule );
			}

			g_dir_close( dir );
		}
	}
	return true;
}

#if defined( _WIN32 )
#define FORMAT_BUFSIZE 2048
const char* CSynapseServer::FormatGetLastError(){
	static char buf[FORMAT_BUFSIZE];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
		buf,
		FORMAT_BUFSIZE,
		NULL
		);
	return buf;
}

void CSynapseServer::EnumerateInterfaces( Str &soname ){
	CSynapseClientSlot slot;
	slot.mpDLL = LoadLibrary( soname.GetBuffer() );
	if ( !slot.mpDLL ) {
		Syn_Printf( "LoadLibrary '%s' failed\n", soname.GetBuffer() );
		Syn_Printf( "  GetLastError: %s", FormatGetLastError() );
		return;
	}
	slot.mpEnumerate = (PFN_SYNAPSE_ENUMERATEINTERFACES)GetProcAddress( slot.mpDLL, NAME_SYNAPSE_ENUMERATEINTERFACES );
	if ( !slot.mpEnumerate ) {
		Syn_Printf( "GetProcAddress('%s') failed\n", NAME_SYNAPSE_ENUMERATEINTERFACES );
		Syn_Printf( "  GetLastError: %s", FormatGetLastError() );
		return;
	}
	Syn_Printf( "Enumerate interfaces on '%s'\n", soname.GetBuffer() );
	slot.mpClient = slot.mpEnumerate( SYNAPSE_VERSION, this );
	if ( !slot.mpClient ) {
		Syn_Printf( "Enumerate interfaces on '%s' returned NULL, unloading.\n", soname.GetBuffer() );
		if ( !FreeLibrary( slot.mpDLL ) ) {
			Syn_Printf( " FreeLibrary failed: GetLastError: '%s'\n", CSynapseServer::FormatGetLastError() );
		}
		return;
	}
	slot.mFileName = soname;
	mClients.push_front( slot );
}

void CSynapseClientSlot::ReleaseSO(){
	if ( !mpDLL ) {
		Syn_Printf( "ERROR: no shared object handle for client '%s' in CSynapseClientSlot::ReleaseSO\n", mpClient->GetInfo() );
		return;
	}
	Syn_Printf( "FreeLibrary '%s'\n", mpClient->GetInfo() );
	if ( !FreeLibrary( mpDLL ) ) {
		Syn_Printf( " FreeLibrary failed: GetLastError: '%s'\n", CSynapseServer::FormatGetLastError() );
	}
	mpDLL = NULL;
}

#elif defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
void CSynapseServer::EnumerateInterfaces( Str &soname ){
	CSynapseClientSlot slot;
	slot.mpDLL = dlopen( soname.GetBuffer(), RTLD_NOW );
	if ( !slot.mpDLL ) {
		const char* error;
		if ( ( error = (char *)dlerror() ) == NULL ) {
			error = "Unknown";
		}
		Syn_Printf( "dlopen '%s' failed\n  dlerror: '%s'\n", soname.GetBuffer(), error );
		return;
	}
	slot.mpEnumerate = (PFN_SYNAPSE_ENUMERATEINTERFACES)dlsym( slot.mpDLL, NAME_SYNAPSE_ENUMERATEINTERFACES );
	if ( !slot.mpEnumerate ) {
		const char* error;
		if ( ( error = (char *)dlerror() ) == NULL ) {
			error = "Unknown";
		}
		Syn_Printf( "dlsym '%s' failed on shared object '%s'\n  dlerror: '%s'\n", NAME_SYNAPSE_ENUMERATEINTERFACES, soname.GetBuffer(), error );
		return;
	}
	Syn_Printf( "Enumerate interfaces on '%s'\n", soname.GetBuffer() );
	slot.mpClient = slot.mpEnumerate( SYNAPSE_VERSION, this );
	if ( !slot.mpClient ) {
		Syn_Printf( "Enumerate interfaces on '%s' returned NULL, unloading.\n", soname.GetBuffer() );
		if ( dlclose( slot.mpDLL ) ) {
			const char* error;
			if ( ( error = (char *)dlerror() ) == NULL ) {
				error = "Unknown";
			}
			Syn_Printf( "  dlclose failed: dlerror: '%s'\n", error );
		}
		return;
	}
	slot.mFileName = soname;
	mClients.push_front( slot );
}

void CSynapseClientSlot::ReleaseSO(){
	if ( !mpDLL ) {
		Syn_Printf( "ERROR: no shared object handle for client '%s' in CSynapseClientSlot::ReleaseSO\n", mpClient->GetInfo() );
		return;
	}
	Syn_Printf( "dlclose '%s'\n", mpClient->GetInfo() );
	if ( dlclose( mpDLL ) ) {
		const char* error;
		if ( ( error = (char *)dlerror() ) == NULL ) {
			error = "Unknown";
		}
		Syn_Printf( "  dlclose failed: dlerror: '%s'\n", error );
	}
	mpDLL = NULL;
}

#endif

void CSynapseServer::EnumerateBuiltinModule( CSynapseBuiltinClient *pClient ){
	CSynapseClientSlot slot;
	pClient->EnumerateInterfaces( this );
	slot.mpClient = pClient;
	slot.mType = SYN_BUILTIN;
	mClients.push_front( slot );
}

PFN_SYN_PRINTF_VA CSynapseServer::Get_Syn_Printf(){
	return g_pPrintf;
}

void CSynapseServer::TryPushStack( APIDescriptor_t *pAPI ){
	list<APIDescriptor_t*>::iterator iAPI;
	for ( iAPI = mStack.begin(); iAPI != mStack.end(); iAPI++ )
	{
		if ( ( *iAPI ) == pAPI ) {
			return;
		}
	}
	mStack.push_front( pAPI );
	mbStackChanged = true;
}

list<CSynapseClientSlot>::iterator CSynapseServer::ShutdownClient( list<CSynapseClientSlot>::iterator iSlot ){
	CSynapseClientSlot *pClientSlot = &( *iSlot );
	if ( pClientSlot->mpClient->IsActive() ) {
		// this should not happen except during core shutdown (i.e. editor is shutting down)
		Syn_Printf( "WARNING: ShutdownClient attempted on an active module '%s'\n", pClientSlot->mpClient->GetInfo() );
	}
	// cleanup mStack
	int i,api_count;
	api_count = pClientSlot->mpClient->GetAPICount();
	for ( i = 0; i < api_count; i++ )
	{
		APIDescriptor_t *pAPI = pClientSlot->mpClient->GetAPIDescriptor( i );
		// search this API in mStack
		list< APIDescriptor_t *>::iterator iStack = mStack.begin();
		while ( iStack != mStack.end() )
		{
			if ( *iStack == pAPI ) {
				break;
			}
			iStack++;
		}
		if ( iStack != mStack.end() ) {
			if ( pAPI->mType == SYN_REQUIRE ) {
				if ( pAPI->mbTableInitDone ) {
					// even if non active, some SYN_REQUIRE may have been filled up
					// look for the corresponding SYN_PROVIDE and decref
					list< APIDescriptor_t *>::iterator iStackRequire = mStack.begin();
					APIDescriptor_t *pMatchAPI;
					while ( iStackRequire != mStack.end() )
					{
						pMatchAPI = *iStackRequire;
						if ( pMatchAPI->mType == SYN_PROVIDE && MatchAPI( pMatchAPI, pAPI ) ) {
							break;
						}
						iStackRequire++;
					}
					if ( iStackRequire != mStack.end() ) {
						// we have found the corresponding SYN_PROVIDE
						pMatchAPI->mRefCount--;
					}
					else
					{
						// this is not supposed to happen at all
						Syn_Printf( "ERROR: couldn't find the SYN_PROVIDE for an initialized SYN_REQUIRE API '%s' '%s' '%s'\n", pAPI->major_name, pAPI->minor_name, pClientSlot->mpClient->GetInfo() );
					}
				}
			}
			else if ( pAPI->mType == SYN_PROVIDE ) {
				// this should never happen on non active clients, it may happen during a core shutdown though
				// if the mRefCount is != 0, that means there is at least a function table out there that will segfault things
				Syn_Printf( "WARNING: found a SYN_PROVIDE API '%s' '%s' with refcount %d in CSynapseServer::ShutdownClient for '%s'\n", pAPI->major_name, pAPI->minor_name, pAPI->mRefCount, pClientSlot->mpClient->GetInfo() );
			}
			// mostly safe to remove it now
			mStack.erase( iStack );
		}
	}
	// we can actually release the module now
	// NOTE: do we want to have a 'final shutdown' call to the client? (not as long as we don't have a use for it)
	if ( pClientSlot->mType == SYN_SO ) {
		pClientSlot->ReleaseSO();
	}
	return mClients.erase( iSlot );
}

void CSynapseServer::PushRequired( CSynapseClient *pClient ){
	/* walk through the standard APIs and push them in */
	int i,max = pClient->GetAPICount();
	for ( i = 0; i < max; i++ )
	{
		APIDescriptor_t* pAPI = pClient->GetAPIDescriptor( i );
		if ( pAPI->mType == SYN_REQUIRE && !pAPI->mbTableInitDone ) {
			TryPushStack( pAPI );
		}
	}

	/* if this client has 'List' API Manager types, walk through them for addition too */
	max = pClient->GetManagerListCount();
	for ( i = 0; i < max; i++ )
	{
		CSynapseAPIManager *pManager = pClient->GetManagerList( i );
		assert( pManager->GetType() == API_LIST );
		pManager->InitializeAPIList();
		int j;
		for ( j = 0; j < pManager->GetAPICount(); j++ )
		{
			TryPushStack( pManager->GetAPI( j ) );
		}
	}

	/* if there are loose match managers, prompt them against the current list of SYN_PROVIDE interfaces
	 * and let them decide which ones they might want
	 */

	max = pClient->GetManagerMatchCount();

	for ( i = 0; i < max; i++ )
	{
		CSynapseAPIManager *pManager = pClient->GetManagerMatch( i );
		// start matching all known SYN_PROVIDE APIs against this manager
		list<CSynapseClientSlot>::iterator iClientSlot;
		for ( iClientSlot = mClients.begin(); iClientSlot != mClients.end(); iClientSlot++ )
		{
			CSynapseClient *pScanClient = ( *iClientSlot ).
										  mpClient;
			int j,jmax = pScanClient->GetAPICount();
			for ( j = 0; j < jmax; j++ )
			{
				APIDescriptor_t *pAPI = pScanClient->GetAPIDescriptor( j );
				if ( pAPI->mType == SYN_PROVIDE ) {
					if ( pManager->MatchAPI( pAPI->major_name, pAPI->minor_name ) ) {
						/*! we are going to want to load this one
						 * NOTE TTimo: what if this can not be resolved in the end?
						 * if this happens, then the whole startup will fail instead
						 * or we can use SYN_REQUIRE_ANY and drop it without consequences
						 */
						APIDescriptor_t *pPushAPI = pManager->BuildRequireAPI( pAPI );
						TryPushStack( pPushAPI );
					}
				}
			}
		}
	}
}

int CSynapseServer::FindActiveMajorClient( const char * major, APIDescriptor_t ** ret ) const {
    Syn_Printf( "checking if we have a single active client for major \"%s\"\n", major );
    *ret = NULL;
    list<CSynapseClientSlot>::const_iterator iClient;
    for ( iClient = mClients.begin(); iClient != mClients.end(); iClient++ ) {
        CSynapseClient *pClient = ( *iClient ).mpClient;
        if ( !pClient->IsActive() ) {
            continue;
        }
        APIDescriptor_t * found = pClient->FindProvidesMajor( major );
        if ( found == NULL ) {
            continue;
        }
        Syn_Printf( "  found in %s\n", pClient->GetInfo() );
        if ( *ret != NULL ) {
            return 2;
        }
        *ret = found;
    }
    if ( *ret == NULL ) {
        return 0;
    }
    return 1;
}

bool CSynapseServer::MatchAPI( APIDescriptor_t *p1, APIDescriptor_t *p2 ){
	int ret = MatchAPI( p1->major_name, p1->minor_name, p2->major_name, p2->minor_name );
    if ( ret == 2 ) {
        // find out if we can resolve the minor "*" situation
        APIDescriptor_t * any_minor_descriptor;
        APIDescriptor_t * provider_descriptor;
        if ( strcmp( p1->minor_name, "*" ) == 0 ) {
            any_minor_descriptor = p1;
            provider_descriptor = p2;
        } else {
            assert( strcmp( p2->minor_name, "*" ) == 0 );
            any_minor_descriptor = p2;
            provider_descriptor = p1;
        }
        assert( any_minor_descriptor->mType == SYN_REQUIRE );
        assert( provider_descriptor->mType == SYN_PROVIDE );
        APIDescriptor_t * search_major;
        int search_ret = FindActiveMajorClient( provider_descriptor->major_name, &search_major );
        if ( search_ret == 2 ) {
            // FIXME: ERROR
            Syn_Printf( "ERROR: Multiple modules active for major \"%s\": cannot resolve \"*\" for it\n", provider_descriptor->major_name );
            return false;
        }
        if ( search_ret == 0 ) {
            // can't resolve yet
            return false;
        }
        if ( search_major != provider_descriptor ) {
            // the provider_descriptor we were passed is likely not active yet, so just ignore
            return false;
        }
        return true; // this is a go, we have a unique match!
    }
    return ( ret != 0 );
}

int CSynapseServer::MatchAPI( const char* major1, const char* minor1, const char* major2, const char* minor2 ){
	if ( strcmp( major1, major2 ) != 0 ) {
		return 0;
	}
	// either no minor at all for this API, or matching
	if ( ( minor1 != NULL && minor2 != NULL ) && strcmp( minor1, minor2 ) == 0 ) {
		return 1;
	}
	if ( ( minor1 != NULL && strcmp( minor1, "*" ) == 0 ) || ( minor2 != NULL && strcmp( minor2, "*" ) == 0 ) ) {
        // one of the minors is "*", and the majors are matching
        // there may be multiple SYN_PROVIDE for this major though, and we can't decide which with only this information
        // e.g. "*" means "the one", not "any" (counter-intuitive, yeah)
        // so let the caller know, maybe he'll figure it out
		return 2;
	}
	return 0;
}

bool CSynapseServer::ResolveAPI( APIDescriptor_t* pAPI ){
	//Syn_Printf("In ResolveAPI %s %p '%s' '%s'\n", APITypeName[pAPI->mType], pAPI, pAPI->major_name, pAPI->minor_name);
	// loop through active clients, search for a client providing what we are looking for
	list<CSynapseClientSlot>::iterator iClient;
	for ( iClient = mClients.begin(); iClient != mClients.end(); iClient++ )
	{
		// walk through interfaces on this client for a match
		CSynapseClient *pScanClient = ( *iClient ).mpClient;
		int i,max = pScanClient->GetAPICount();
		for ( i = 0; i < max; i++ )
		{
			APIDescriptor_t *pScanClientAPI = pScanClient->GetAPIDescriptor( i );
			if ( pScanClientAPI->mType == SYN_PROVIDE ) {
				if ( MatchAPI( pAPI, pScanClientAPI ) ) {
					// can this client provide APIs yet
					// it is possible that all of it's APIs have been filled and it's not been activated yet
					pScanClient->CheckSetActive();
					if ( pScanClient->IsActive() ) {
						// make sure this interface has correct size (this is our version check)
						if ( pAPI->mSize != pScanClientAPI->mSize ) {
							Syn_Printf( "ERROR: version mismatch for API '%s' '%s' found in '%s' (size %d != %d)\n", pAPI->major_name, pAPI->minor_name, pScanClient->GetInfo(), pAPI->mSize, pScanClientAPI->mSize );
							Syn_Printf( "  the module and the server are incompatible\n" );
							// keep going to other APIs
							continue;
						}
						// this is an active client, we can request
#ifdef SYNAPSE_VERBOSE
						Syn_Printf( "RequestAPI '%s' '%s' from '%s' for API %p\n", pAPI->major_name, pAPI->minor_name, pScanClient->GetInfo(), pAPI );
#endif
						if ( !pScanClient->RequestAPI( pAPI ) ) {
							// this should never happen, means we think this module provides the API, but it answers that it doesn't
							Syn_Printf( "ERROR: RequestAPI failed\n" );
							return false;
						}
						pScanClientAPI->mRefCount++;
						pAPI->mbTableInitDone = true;
						return true; // job done
					}
					else
					{
						// this client is not active yet, some of it's required interfaces are not filled in
						PushRequired( pScanClient );
						// we will exit the scan through the APIDescriptor of this client and look at other clients
						break;
					}
				}
			}
		}
	}
	return false;
}

bool CSynapseServer::DoResolve( CSynapseClient *pClient ){
	list<CSynapseClientSlot>::iterator iSlot;
	for ( iSlot = mClients.begin(); iSlot != mClients.end(); iSlot++ )
	{
		if ( ( *iSlot ).mpClient == pClient ) {
			break;
		}
	}
	if ( iSlot == mClients.end() ) {
		Syn_Printf( "CSynapserServer::Resolve adding new client slot '%s'\n", pClient->GetInfo() );
		CSynapseClientSlot slot;
		slot.mpClient = pClient;
		slot.mFileName = "local client";
		// make it active so we can request the interfaces already
		pClient->ForceSetActive();
		mClients.push_front( slot );
	}
	else
	{
		// make it active so we can request the interfaces already
		( *iSlot ).mpClient->ForceSetActive();
	}

	// push the interfaces that need to be resolved for this client
	// NOTE: this doesn't take care of the SYN_REQUIRE_ANY interfaces
	PushRequired( pClient );
	// start resolving now
	// working till the stack is emptied or till we reach a dead end situation
	// we do a depth first traversal, we will grow the interface stack to be resolved till we start finding solutions
	list<APIDescriptor_t*>::iterator iCurrent;
	mbStackChanged = true; // init to true so we try the first elem
	while ( !mStack.empty() )
	{
		//DumpStack();
		if ( !mbStackChanged ) {
			// the stack didn't change last loop
			iCurrent++;
			if ( iCurrent == mStack.end() ) {
				Syn_Printf( "ERROR: CSynapseServer::Resolve, failed to resolve config for %s\n", pClient->GetName() );
				DumpStack();
				return false;
			}
			if ( ResolveAPI( *iCurrent ) ) {
				iCurrent = mStack.erase( iCurrent );
				mbStackChanged = true;
			}
		}
		else
		{
			// the stack changed at last loop
			mbStackChanged = false;
			iCurrent = mStack.begin();
			if ( ResolveAPI( *iCurrent ) ) {
				iCurrent = mStack.erase( iCurrent );
				mbStackChanged = true;
			}
		}
	}
	return true;
}

bool CSynapseServer::Resolve( CSynapseClient *pClient ){
	bool ret = DoResolve( pClient );
	list<CSynapseClientSlot>::iterator iClient;
	iClient = mClients.begin();
	while ( iClient != mClients.end() )
	{
		CSynapseClient *pClient = ( *iClient ).mpClient;
		if ( !pClient->IsActive() ) {
			Syn_Printf( "Unloading an unused module: '%s'\n", pClient->GetInfo() );
			iClient = ShutdownClient( iClient );
		}
		else{
			iClient++;
		}
	}
	return ret;
}

void CSynapseServer::Shutdown(){
	Syn_Printf( "Synapse server core is shutting down\n" );
	// do a first pass to shutdown the clients nicely (i.e. decref, release memory and drop everything)
	// we seperate the client shutdown calls from the dlclose cause that part is a clean decref / free situation whereas dlclose will break links without advice
	list<CSynapseClientSlot>::iterator iClient;
	iClient = mClients.begin();
	for ( iClient = mClients.begin(); iClient != mClients.end(); iClient++ )
	{
		( *iClient ).mpClient->Shutdown();
	}
	// now release them from the server's point of view
	iClient = mClients.begin();
	while ( iClient != mClients.end() )
	{
		iClient = ShutdownClient( iClient );
	}
}

void CSynapseServer::DumpStack(){
	list<APIDescriptor_t*>::iterator iCurrent;
	for ( iCurrent = mStack.begin(); iCurrent != mStack.end(); iCurrent++ )
	{
		APIDescriptor_t*pAPI = *iCurrent;
		Syn_Printf( " interface %s %p '%s' '%s'\n", APITypeName[pAPI->mType], pAPI, pAPI->major_name, pAPI->minor_name );
	}
}

void CSynapseServer::DumpActiveClients(){
	list<CSynapseClientSlot>::iterator iClient;
	for ( iClient = mClients.begin(); iClient != mClients.end(); iClient++ )
	{
		CSynapseClient *pClient = ( *iClient ).mpClient;
		Syn_Printf( "%s", pClient->GetInfo() );
		if ( pClient->IsActive() ) {
			Syn_Printf( "\n" );
		}
		else{
			Syn_Printf( " (not active)\n" );
		}
	}
}

bool CSynapseServer::SelectClientConfig( const char *client_name ){
	if ( !mpDoc ) {
		return false;
	}
	xmlNodePtr pNode = xmlDocGetRootElement( mpDoc );
	if ( !pNode ) {
		return false;
	}
	// look for the client
	pNode = pNode->children;
	while ( pNode )
	{
		if ( pNode->type == XML_ELEMENT_NODE ) {
			xmlChar *prop = xmlGetProp( pNode, (const xmlChar *)"name" );
			if ( !strcmp( (const char *)prop, client_name ) ) {
				xmlFree( prop );
				break;
			}
			xmlFree( prop );
		}
		pNode = pNode->next;
	}
	if ( !pNode ) {
		return false; // config you asked for isn't there
	}
	// focus
	mpFocusedNode = pNode->children;
	mpCurrentClientConfig = pNode;
	return true;
}

bool CSynapseServer::GetNextConfig( char **api_name, char **minor ){
	while ( mpFocusedNode && mpFocusedNode->name )
	{
		if ( mpFocusedNode->type == XML_ELEMENT_NODE && !strcmp( (const char *)mpFocusedNode->name, "api" ) ) {
			if ( m_api_name ) {
				xmlFree( m_api_name );
                                m_api_name = NULL;
			}
			m_api_name = xmlGetProp( mpFocusedNode, (const xmlChar *)"name" );
			*api_name = (char *)m_api_name;
			if ( m_content ) {
				g_free( m_content );
			}
			m_content = g_strdup( (const gchar *)mpFocusedNode->children->content );
			g_strstrip( m_content );
			*minor = m_content;
			mpFocusedNode = mpFocusedNode->next;
			return true;
		}
		mpFocusedNode = mpFocusedNode->next;
	}
	return false;
}

bool CSynapseServer::GetConfigForAPI( const char *api, char **minor ) {
	xmlNodePtr pNode = mpCurrentClientConfig->children;
	while ( pNode && pNode->name ) {
		if ( pNode->type == XML_ELEMENT_NODE && !strcmp( (const char *)pNode->name, "api" ) ) {
			if ( m_api_name ) {
				xmlFree( m_api_name );
                                m_api_name = NULL;
			}
			m_api_name = xmlGetProp( pNode, (const xmlChar *)"name" );
			if ( !strcmp( (const char *)m_api_name, api ) ) {
				if ( m_content ) {
					g_free( m_content );
				}
				m_content = g_strdup( (const gchar *)pNode->children->content );
				g_strstrip( m_content );
				*minor = m_content;
				return true;
			}
		}
		pNode = pNode->next;
	}
	return false;
}

const char *CSynapseServer::GetModuleFilename( CSynapseClient *pClient ){
	list<CSynapseClientSlot>::iterator iSlot;
	for ( iSlot = mClients.begin(); iSlot != mClients.end(); iSlot++ )
	{
		if ( ( *iSlot ).mpClient == pClient ) {
			if ( ( *iSlot ).mType == SYN_BUILTIN ) {
				return ""; // FIXME
			}
			else
			{
				return ( *iSlot ).mFileName;
			}
		}
	}
	return NULL;
}

/*
   =======================================================================
   client
   =======================================================================
 */

CSynapseClient::CSynapseClient(){
}

void CSynapseClient::Shutdown(){
	vector<APIDescriptor_t *>::iterator iAPI;
	for ( iAPI = mAPIDescriptors.begin(); iAPI != mAPIDescriptors.end(); iAPI++ )
	{
		APIDescriptor_t *pAPI = *iAPI;
		if ( pAPI->mRefCount != 0 ) {
			Syn_Printf( "WARNING: ~CSynapseClient '%s' has non-zero ref count for interface '%s' '%s'\n", GetInfo(), pAPI->major_name, pAPI->minor_name );
		} else {
			delete pAPI;
		}
		*iAPI = NULL;
	}
	mAPIDescriptors.clear();
	vector<CSynapseAPIManager *>::iterator iManager;
	for ( iManager = mManagersList.begin(); iManager != mManagersList.end(); iManager++ )
	{
		CSynapseAPIManager *pManager = *iManager;
		pManager->DecRef();
		*iManager = NULL;
	}
	mManagersList.clear();
	for ( iManager = mManagersMatch.begin(); iManager != mManagersMatch.end(); iManager++ )
	{
		CSynapseAPIManager *pManager = *iManager;
		pManager->DecRef();
		*iManager = NULL;
	}
	mManagersMatch.clear();
}

CSynapseClient::~CSynapseClient(){
	// this should not be doing anything when called from here if everything went right
	// otherwise it's likely to crash .. at least that's the sign we missed something
	Shutdown();
}

int CSynapseClient::GetAPICount() const {
	return mAPIDescriptors.size();
}

APIDescriptor_t* CSynapseClient::GetAPIDescriptor( int i ) const {
	return mAPIDescriptors[i];
}

int CSynapseClient::GetManagerMatchCount(){
	return mManagersMatch.size();
}

CSynapseAPIManager* CSynapseClient::GetManagerMatch( int i ){
	return mManagersMatch[i];
}

int CSynapseClient::GetManagerListCount(){
	return mManagersList.size();
}

CSynapseAPIManager* CSynapseClient::GetManagerList( int i ){
	return mManagersList[i];
}

bool CSynapseClient::AddAPI( const char *major, const char *minor, int size, EAPIType type, void *pTable ){
	// do some safe checks before actual addition
	if ( type == SYN_REQUIRE && !pTable ) {
		Syn_Printf( "ERROR: interface '%s' '%s' from '%s' is SYN_REQUIRE and doesn't provide a function table pointer\n", major, minor, GetInfo() );
		return false;
	}
	if ( pTable ) {
		int *pi = (int *)pTable;
		if ( pi == 0 ) {
			Syn_Printf( "ERROR: forgot to init function table size for interface '%s' '%s' from '%s'?\n", major, minor, GetInfo() );
			return false;
		}
	}
	APIDescriptor_t *pAPI = new APIDescriptor_t;
	memset( pAPI, 0, sizeof( APIDescriptor_t ) );
	strncpy( pAPI->major_name, major, MAX_APINAME );
	if ( minor ) {
		strncpy( pAPI->minor_name, minor, MAX_APINAME );
	}
	pAPI->mType = type;
	pAPI->mpTable = pTable;
	// store the interface size
	if ( type == SYN_PROVIDE ) {
		if ( size == 0 ) {
			Syn_Printf( "ERROR: size of the interface required for a SYN_PROVIDE ('%s' '%s' from '%s')\n", major, minor, GetInfo() );
			delete pAPI;
			return false;
		}
		pAPI->mSize = size;
	}
	else if ( type == SYN_REQUIRE ) {
		if ( size != 0 ) {
			// if a non-zero value is given in function call, use this instead of the val in table
			*( (int *)pAPI->mpTable ) = size;
			pAPI->mSize = size;
		}
		else
		{
			pAPI->mSize = *( (int *)pAPI->mpTable );
			if ( pAPI->mSize == 0 ) {
				Syn_Printf( "ERROR: didn't get an interface size ('%s' '%s' from '%s')\n", major, minor, GetInfo() );
				delete pAPI;
				return false;
			}
		}
	}
	else
	{
		Syn_Printf( "ERROR: AddAPI type '%d' not supported\n", type );
		return false;
	}
	mAPIDescriptors.push_back( pAPI );
  #ifdef SYNAPSE_VERBOSE
	Syn_Printf( "AddAPI: %s %p '%s' '%s' from '%s', size %d\n", APITypeName[pAPI->mType], pAPI, major, minor, GetInfo(), pAPI->mSize );
  #endif
	return true;
}

#include "version.h"

const char* CSynapseClient::GetInfo(){
	return "CSynapseClient built " __DATE__ " " RADIANT_VERSION;
}

bool CSynapseClient::CheckSetActive(){
	if ( mbActive ) {
		return true;
	}
	int i,max = GetAPICount();
	for ( i = 0; i < max; i++ )
	{
		APIDescriptor_t *pAPI = GetAPIDescriptor( i );
		if ( pAPI->mType == SYN_REQUIRE && !pAPI->mbTableInitDone ) {
			return false;
		}
	}
	// if we have managers with fixed list, those need to be completely filled in too
	vector<CSynapseAPIManager *>::iterator iManager;
	for ( iManager = mManagersList.begin(); iManager != mManagersList.end(); iManager++ )
	{
		if ( !( *iManager )->CheckSetActive() ) {
			return false; // one of the managers doesn't have all it needs yet
		}
	}
	// call OnActivate to let the client perform last minute checks
	// NOTE: this should be fatal instead of letting the engine try other combinations
	if ( !OnActivate() ) {
		return false;
	}
	// yes, all required interfaces have been initialized
	Syn_Printf( "'%s' activated\n", GetInfo() );
	mbActive = true;
	return true;
}

bool CSynapseClient::ConfigXML( CSynapseServer *pServer, const char *client_name, const XMLConfigEntry_t entries[] ) {

	if ( !client_name ) {
		client_name = GetName();
	}

	Syn_Printf( "Dynamic APIs for client '%s'\n", GetInfo() );
	if ( !pServer->SelectClientConfig( client_name ) ) {
		Syn_Printf( "Failed to select synapse client config '%s'\n", client_name );
		return false;
	}

	int i = 0;
	while ( entries[i].type != SYN_UNKNOWN ) { // don't test pTable, for a SYN_PROVIDE it will be empty
		char *minor;
		if ( !pServer->GetConfigForAPI( entries[i].api, &minor ) ) {
			Syn_Printf( "GetConfigForAPI '%s' failed - invalid XML config file?\n", entries[i].api );
			return false;
		}
		AddAPI( entries[i].api, minor, entries[i].size, entries[i].type, entries[i].pTable );
		i++;
	}
	Syn_Printf( "%d dynamic interfaces parsed for '%s'\n", i, client_name );
	return true;
}

void CSynapseClient::AddManager( CSynapseAPIManager *pManager ){
	pManager->IncRef();
	if ( pManager->GetType() == API_LIST ) {
		mManagersList.push_back( pManager );
	}
	else{
		mManagersMatch.push_back( pManager );
	}
}

APIDescriptor_t * CSynapseClient::FindProvidesMajor( const char * major ) const {
	int i,max = GetAPICount();
	for ( i = 0; i < max; i++ )
	{
		APIDescriptor_t *pAPI = GetAPIDescriptor( i );
        if ( pAPI->mType == SYN_PROVIDE && strcmp( pAPI->major_name, major ) == 0 ) {
            return pAPI;
        }
	}
    return NULL;
}

CSynapseAPIManager::~CSynapseAPIManager(){
	vector<APIDescriptor_t *>::iterator iAPI;
	for ( iAPI = mAPIs.begin(); iAPI != mAPIs.end(); iAPI++ )
	{
		APIDescriptor_t *pAPI = *iAPI;
		if ( pAPI->mRefCount != 0 ) {
			Syn_Printf( "WARNING: ~CSynapseAPIManager has non-zero ref count for interface '%s' '%s'\n", pAPI->major_name, pAPI->minor_name );
		}
		delete pAPI;
		*iAPI = NULL;
	}
}

APIDescriptor_t* CSynapseAPIManager::PrepareRequireAPI( APIDescriptor_t *pAPI ){
#ifdef _DEBUG
	if ( pAPI->mType != SYN_PROVIDE ) {
		Syn_Printf( "ERROR: unexpected pAPI->mType != SYN_PROVIDE in CSynapseAPIManager::PrepareRequireAPI\n" );
		return NULL;
	}
#endif
	APIDescriptor_t *pRequireAPI = new APIDescriptor_t;
	memcpy( pRequireAPI, pAPI, sizeof( APIDescriptor_t ) );
	pRequireAPI->mType = SYN_REQUIRE_ANY;
	pRequireAPI->mpTable = NULL;
	pRequireAPI->mbTableInitDone = false;
	pRequireAPI->mSize = 0; // this will have to be set correctly by the child for version checking
	pRequireAPI->mRefCount = 0;
	return pRequireAPI;
}

void CSynapseAPIManager::SetMatchAPI( const char *major, const char *minor ){
	if ( strlen( minor ) > MAX_PATTERN_STRING ) {
		Syn_Printf( "ERROR: MAX_TOKEN_STRING exceeded in CSynapseAPIManager::SetMatchAPI: '%s'\n", minor );
		return;
	}
	strcpy( major_pattern, major );
	strcpy( minor_pattern, minor );
	if ( strcmp( minor, "*" ) ) {
		mType = API_LIST;
	}
}

bool CSynapseAPIManager::MatchAPI( const char *major, const char *minor ){
	assert( mType == API_MATCH );

	/*!
	   if this interface has been allocated already, avoid requesting it again..
	 */
	vector<APIDescriptor_t *>::iterator iAPI;
	for ( iAPI = mAPIs.begin(); iAPI != mAPIs.end(); iAPI++ )
	{
		if ( CSynapseServer::MatchAPI( ( *iAPI )->major_name, ( *iAPI )->minor_name, major, minor ) ) {
			return false;
		}
	}

	if ( !strcmp( major, major_pattern ) ) {
		return true;
	}
	return false;
}

bool CSynapseAPIManager::CheckSetActive(){
	if ( mType == API_MATCH ) {
		return false;
	}
	// mType == API_LIST
	int i,max = GetAPICount();
	for ( i = 0; i < max; i++ )
	{
		if ( !GetAPI( i )->mbTableInitDone ) {
			return false;
		}
	}
	return true;
}

void CSynapseAPIManager::InitializeAPIList(){
	char minor_tok[MAX_PATTERN_STRING];
	char *token;

	if ( mAPIs.size() ) {
		Syn_Printf( "WARNING: CSynapseAPIManager::InitializeAPIList on an already initialized APIManager\n" );
		return;
	}

	strncpy( minor_tok, minor_pattern, MAX_PATTERN_STRING );
	token = strtok( minor_tok, " " );
	while ( token )
	{
		/* ask the child to build from scratch */
		APIDescriptor_t *pAPI = new APIDescriptor_t;
		memset( pAPI, 0, sizeof( APIDescriptor_t ) );
		strncpy( pAPI->major_name, major_pattern, MAX_APINAME );
		strncpy( pAPI->minor_name, token, MAX_APINAME );
		pAPI->mType = SYN_REQUIRE_ANY;
		FillAPITable( pAPI );
		mAPIs.push_back( pAPI );
		token = strtok( NULL, " " );
	}
}

int CSynapseAPIManager::GetAPICount(){
	return mAPIs.size();
}

APIDescriptor_t* CSynapseAPIManager::GetAPI( int i ){
	return mAPIs[i];
}
