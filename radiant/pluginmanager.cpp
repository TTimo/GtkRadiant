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

// PlugInManager.cpp: implementation of the CPlugInManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <dirent.h>
  #include <sys/time.h>
#endif
#ifdef __APPLE__
  #ifdef __cplusplus
extern "C" {
	#endif
	#include <dlfcn.h>
	#ifdef __cplusplus
}
  #endif
#endif
#ifdef _WIN32
  #include "objbase.h"
#endif
#include "pluginmanager.h"
#include "plugin.h"
#include "missing.h"
#include "filters.h"

#include "version.h"

CRadiantImageManager g_ImageManager;
CRadiantPluginManager g_PluginsManager;

_QERPlugSurfaceTable g_SurfaceTable;
_QERFileSystemTable g_FileSystemTable;
_QERShadersTable g_ShadersTable;
_QERPlugMapTable g_MapTable;
_QERPlugMapTable g_MapTable2;
_QEREntityTable g_EntityTable;
_EClassTable g_EClassDefTable;

/*!
   extending entity class formats
   this approach only allows a single additional format, but it is enough for now
 */
bool g_bHaveEClassExt = false;
_EClassTable g_EClassExtTable;


filetype_t g_pattern_all( "all files", "*.*" );
filetype_t g_pattern_projqe4v2( "qe4 v2 project files", "*.qe4" );
filetype_t g_pattern_projxml( "xml project files", "*.proj" );
filetype_t g_pattern_mapq3( "quake3 maps", "*.map" );
filetype_t g_pattern_mapxml( "xml quake3 maps", "*.xmap" );
filetype_t g_pattern_modelmd3( "md3 models", "*.md3" );
filetype_t g_pattern_modelmdc( "mdc models", "*.mdc" );
filetype_t g_pattern_modelmd2( "md2 models", "*.md2" );
filetype_t g_pattern_modelmdl( "mdl models", "*.mdl" );
filetype_t g_pattern_modelobj( "obj models", "*.obj" );
filetype_t g_pattern_soundwav( "PCM sound files", "*.wav" );
filetype_t g_pattern_ogg( "OGG sound files", "*.ogg" );
filetype_t g_pattern_regq3( "quake3 region", "*.reg" );

#include <map>

class RadiantFileTypeRegistry : public IFileTypeRegistry
{
public:
virtual ~RadiantFileTypeRegistry() {}
virtual void addType( const char* key, filetype_t type ){
	m_typelists[key].push_back( type );
}
virtual void getTypeList( const char* key, IFileTypeList* typelist ){
	filetype_list_t& list_ref = m_typelists[key];

	if (key == "model") {
		// Get the list of all supported types (adapted from kaz)
		CString allTypesFilter;
		for (unsigned int i = 0; i < list_ref.size(); ++i) {
			allTypesFilter += list_ref[i].getType().pattern;
			if (i < list_ref.size() - 1) {
				allTypesFilter += ";";
			}
		}

		// Add the item on top
		// TODO: Make is translatable
		typelist->addType(filetype_t("All supported types", allTypesFilter.GetBuffer()));
	}

	// Then add the supported types one by one
	// TODO: Sort them alphabetically (have them sorted in m_typelists[key])
	for ( unsigned int i = 0; i < list_ref.size(); ++i ) {
		typelist->addType( list_ref[i].getType() );
	}
}
private:
struct filetype_copy_t
{
	inline filetype_copy_t( const filetype_t other )
		: m_name( other.name ), m_pattern( other.pattern )
	{}
	inline filetype_t getType() const {
		return filetype_t( m_name.c_str(), m_pattern.c_str() );
	}
private:
	string_t m_name;
	string_t m_pattern;
};
typedef vector<filetype_copy_t> filetype_list_t;
map<string_t, filetype_list_t> m_typelists;
};

static RadiantFileTypeRegistry g_patterns;

IFileTypeRegistry* GetFileTypeRegistry(){
	return &g_patterns;
}

void InitFileTypes(){
	//GetFileTypeRegistry()->addType("project", g_pattern_projqe4v2);
	GetFileTypeRegistry()->addType( "project", g_pattern_projxml );

	GetFileTypeRegistry()->addType( MAP_MAJOR, g_pattern_mapq3 );
	GetFileTypeRegistry()->addType( MAP_MAJOR, g_pattern_mapxml );

	GetFileTypeRegistry()->addType( "region", g_pattern_regq3 );
/*
   GetFileTypeRegistry()->addType(MODEL_MAJOR, g_pattern_modelmd3);
   GetFileTypeRegistry()->addType(MODEL_MAJOR, g_pattern_modelmd2);
   GetFileTypeRegistry()->addType(MODEL_MAJOR, g_pattern_modelmdl);
   GetFileTypeRegistry()->addType(MODEL_MAJOR, g_pattern_modelmdc);
   //GetFileTypeRegistry()->addType(MODEL_MAJOR, g_pattern_modelea3);
 */

	GetFileTypeRegistry()->addType( "sound", g_pattern_soundwav );
        GetFileTypeRegistry()->addType( "sound", g_pattern_ogg );
        GetFileTypeRegistry()->addType( "sound", g_pattern_all );
}


class CRadiantModelModuleManager : public CSynapseAPIManager
{
typedef list<APIDescriptor_t*> APIDescriptorList;

APIDescriptorList mAPIs;
public:
CRadiantModelModuleManager(){
	SetMatchAPI( MODEL_MAJOR, "*" );
}
virtual ~CRadiantModelModuleManager(){
	APIDescriptorList::iterator i;
	for ( i = mAPIs.begin(); i != mAPIs.end(); i++ )
	{
		delete (_QERPlugModelTable*)( *i )->mpTable;
		delete *i;
		*i = NULL;
	}
	mAPIs.clear();
}

// CSynapseAPIManager interface -------------------
APIDescriptor_t* BuildRequireAPI( APIDescriptor_t* pAPI ){
	APIDescriptor_t* pRequireAPI = CSynapseAPIManager::PrepareRequireAPI( pAPI );
	pRequireAPI->mpTable = new _QERPlugModelTable;
	( (_QERPlugModelTable*)pRequireAPI->mpTable )->m_nSize = sizeof( _QERPlugModelTable );
	pRequireAPI->mSize = sizeof( _QERPlugModelTable );
	mAPIs.push_front( pRequireAPI );
	return pRequireAPI;
}

// Model Manager specific
const _QERPlugModelTable* GetModelTable( const char* version ){
	APIDescriptorList::iterator i;
	for ( i = mAPIs.begin(); i != mAPIs.end(); i++ )
		if ( strcmp( version, ( *i )->minor_name ) == 0 ) {
			return ( (_QERPlugModelTable*)( *i )->mpTable );
		}
	return NULL;
}
};

CRadiantModelModuleManager g_ModelManager;

/*! One of these exists for each unique model ID in use */
class CModelWrapper
{
friend class CModelManager;
public:
CModelWrapper ( const char *id, const char* version ) : refcount( 1 ){
	copy( id, version );
	construct();
}
void Refresh(){
	destroy();
	construct();
}
~CModelWrapper (){
	destroy();
}
private:
void copy( const char* id, const char* version ){
	m_id = id;
	m_version = version;
}
void construct(){
	m_model.pRender = NULL;
	m_model.pSelect = NULL;
	m_model.pEdit = NULL;

	const _QERPlugModelTable* pTable = g_ModelManager.GetModelTable( m_version.c_str() );

	if ( pTable != NULL ) {
		pTable->m_pfnLoadModel( &m_model, m_id.c_str() );
	}
}
void destroy(){
	if ( m_model.pRender ) {
		m_model.pRender->DecRef();
	}
	if ( m_model.pSelect ) {
		m_model.pSelect->DecRef();
	}
	if ( m_model.pEdit ) {
		m_model.pEdit->DecRef();
	}
}
string_t m_id;
string_t m_version;
entity_interfaces_t m_model;
int refcount;
};

/*! Creates and tracks CModelWrapper instances.
   Creates a new instance for each unique ID requested, keeps count of the number of
   times an ID is being referenced, and destroys any instance that is no longer in use */
class CModelManager : public IModelCache
{
	public:
	CModelManager(){
		m_ptrs = g_ptr_array_new();
	}
	virtual ~CModelManager(){
		g_ptr_array_free( m_ptrs, FALSE );
	}

	virtual void DeleteByID( const char *id, const char* version ){
		unsigned int i;
		CModelWrapper *elem;
		for ( i = 0; i < m_ptrs->len; i++ )
		{
			elem = (CModelWrapper*)m_ptrs->pdata[i];
			if (strcmp(elem->m_version.c_str(), version) == 0
				&& strcmp(elem->m_id.c_str(), id) == 0) {
				elem->refcount--;
				if (elem->refcount == 0) {
					g_ptr_array_remove_index_fast(m_ptrs, i);
					delete elem;
				}
				return;
			}
		}
	}

	virtual entity_interfaces_t *GetByID( const char *id, const char* version ){
		unsigned int i;
		CModelWrapper *elem;
		for ( i = 0; i < m_ptrs->len; i++ )
		{
			elem = (CModelWrapper*)m_ptrs->pdata[i];
			if ( strcmp( elem->m_version.c_str(), version ) == 0
				 && strcmp( elem->m_id.c_str(), id ) == 0 ) {
				elem->refcount++;
				return &elem->m_model;
			}
		}

		elem = new CModelWrapper( id, version );
		g_ptr_array_add( m_ptrs, elem );

		return &elem->m_model;
	}

	virtual void RefreshAll(){
		for ( unsigned int i = 0; i < m_ptrs->len; ++i )
			( (CModelWrapper*)m_ptrs->pdata[i] )->Refresh();
	}
	private:
	GPtrArray *m_ptrs;   // array of CModelWrapper*
};

CModelManager g_model_cache;

IModelCache* GetModelCache(){
	return &g_model_cache;
}

// toolbar manager
class CRadiantToolbarModuleManager : public CSynapseAPIManager
{
typedef list<APIDescriptor_t*> APIDescriptorList;

APIDescriptorList mAPIs;
public:
CRadiantToolbarModuleManager(){
	SetMatchAPI( TOOLBAR_MAJOR, "*" );
}
virtual ~CRadiantToolbarModuleManager(){
	APIDescriptorList::iterator i;
	for ( i = mAPIs.begin(); i != mAPIs.end(); i++ )
	{
		delete (_QERPlugToolbarTable*)( *i )->mpTable;
		delete *i;
		*i = NULL;
	}
	mAPIs.clear();
}

// CSynapseAPIManager interface -------------------
APIDescriptor_t* BuildRequireAPI( APIDescriptor_t* pAPI ){
	APIDescriptor_t* pRequireAPI = CSynapseAPIManager::PrepareRequireAPI( pAPI );
	pRequireAPI->mpTable = new _QERPlugToolbarTable;
	( (_QERPlugToolbarTable*)pRequireAPI->mpTable )->m_nSize = sizeof( _QERPlugToolbarTable );
	pRequireAPI->mSize = sizeof( _QERPlugToolbarTable );
	mAPIs.push_front( pRequireAPI );
	return pRequireAPI;
}

// Toolbar Manager specific
void ConstructToolbar(){
	APIDescriptorList::iterator i;
	for ( i = mAPIs.begin(); i != mAPIs.end(); i++ )
		AddItem( (_QERPlugToolbarTable*)( *i )->mpTable );
}

private:

void AddItem( _QERPlugToolbarTable* pTable ){
	const unsigned int count = pTable->m_pfnToolbarButtonCount();
	for ( unsigned int i = 0; i < count; ++i )
	{
		const IToolbarButton* button = pTable->m_pfnGetToolbarButton( i );
		g_pParentWnd->AddPlugInToolbarButton( button );
	}
}
};

CRadiantToolbarModuleManager g_ToolbarModuleManager;


/* image manager ---------------------------------------- */

CRadiantImageManager::~CRadiantImageManager(){
	list<CImageTableSlot *>::iterator iSlot;
	for ( iSlot = mSlots.begin(); iSlot != mSlots.end(); iSlot++ )
	{
		delete *iSlot;
		*iSlot = NULL;
	}
}

void CImageTableSlot::InitForFillAPITable( APIDescriptor_t *pAPI ){
	mpAPI = pAPI;
	mpTable = new _QERPlugImageTable;
	mpTable->m_nSize = sizeof( _QERPlugImageTable );
	mpAPI->mSize = sizeof( _QERPlugImageTable );
	mpAPI->mpTable = mpTable;
}

void CRadiantImageManager::FillAPITable( APIDescriptor_t *pAPI ){
	CImageTableSlot *pSlot = new CImageTableSlot();
	pSlot->InitForFillAPITable( pAPI );
	mSlots.push_front( pSlot );
}

/*!
   Loads an image by calling the module that handles the extension extracted from the filename
   \param name The filename to load. If no extension is provided, we walk the list of supported extensions.
   \param pic The returned image data
   \param width The returned width of the image
   \param height The returned height of the image
 */
void CRadiantImageManager::LoadImage( const char *name, byte **pic, int *width, int *height ){
	const char *ext = NULL;
	int len;

	// extract extension
	len = strlen( name );
	if ( ( len > 5 ) && ( name[len - 4] == '.' ) ) {
		ext = &name[len - 3];
	}

	if ( ext == NULL ) {
		// if no extension is provided, start walking through the list
		Str fullname;
		list<CImageTableSlot *>::iterator iSlot;
		for ( iSlot = mSlots.begin(); iSlot != mSlots.end(); iSlot++ )
		{
			APIDescriptor_t *pAPI = ( *iSlot )->GetDescriptor();
			fullname.Format( "%s.%s", name, pAPI->minor_name );
			( *iSlot )->GetTable()->m_pfnLoadImage( fullname.GetBuffer(), pic, width, height );
			if ( *pic ) {
				return; // this was the right extension, we loaded
			}
		}
		return;
	}

	// start walking the interfaces
	list<CImageTableSlot *>::iterator iSlot;
	for ( iSlot = mSlots.begin(); iSlot != mSlots.end(); iSlot++ )
	{
		APIDescriptor_t *pAPI = ( *iSlot )->GetDescriptor();
		if ( !strcmp( pAPI->minor_name, ext ) ) {
			( *iSlot )->GetTable()->m_pfnLoadImage( name, pic, width, height );
			return;
		}
	}
	Sys_FPrintf( SYS_WRN, "WARNING: no image table for extension '%s'\n", ext );
}

void CRadiantImageManager::BeginExtensionsScan(){
	mExtScanSlot = mSlots.begin();
}

const char* CRadiantImageManager::GetNextExtension(){
	if ( mExtScanSlot != mSlots.end() ) {
		char *ext = ( *mExtScanSlot )->GetDescriptor()->minor_name;
		mExtScanSlot++;
		return ext;
	}
	return NULL;
}

/* plugin manager --------------------------------------- */
APIDescriptor_t* CRadiantPluginManager::BuildRequireAPI( APIDescriptor_t *pAPI ){
	CPluginSlot *pSlot = new CPluginSlot( pAPI );
	mSlots.push_front( pSlot );
	return pSlot->GetDescriptor();
}

void CRadiantPluginManager::PopulateMenu(){
	list<CPluginSlot *>::iterator iPlug;
	for ( iPlug = mSlots.begin(); iPlug != mSlots.end(); iPlug++ )
	{
		g_pParentWnd->AddPlugInMenuItem( *iPlug );
	}
}

void CSynapseClientRadiant::ImportMap( IDataStream *in, CPtrArray *ents, const char *type ){
	if ( strcasecmp( type, "map" ) == 0 ) {
		g_MapTable.m_pfnMap_Read( in, ents );
	}
	else if (strcasecmp( type,"xmap" ) == 0 ) {
		g_MapTable2.m_pfnMap_Read( in, ents );
	}
	else{
		Sys_FPrintf( SYS_WRN, "WARNING: no module found for map interface type '%s'\n", type );
	}
}

void CSynapseClientRadiant::ExportMap( CPtrArray *ents, IDataStream *out, const char *type ){
	if (strcasecmp( type,"map" ) == 0 ) {
		g_MapTable.m_pfnMap_Write( ents, out );
	}
	else if (strcasecmp( type,"xmap" ) == 0 ) {
		g_MapTable2.m_pfnMap_Write( ents, out );
	}
	else{
		Sys_FPrintf( SYS_WRN, "WARNING: no module found for map interface type '%s'\n", type );
	}
}

CPluginSlot::CPluginSlot( APIDescriptor_t *pAPI ){
	mpAPI = CSynapseAPIManager::PrepareRequireAPI( pAPI );
	mpTable = new _QERPluginTable;
	mpTable->m_nSize = sizeof( _QERPluginTable );
	mpAPI->mSize = sizeof( _QERPluginTable );
	mpAPI->mpTable = mpTable;
	m_CommandStrings = NULL;
	m_CommandIDs = NULL;
	m_bReady = false;
}

CPluginSlot::~CPluginSlot(){
	delete mpAPI;
	delete mpTable;
	while ( m_CommandStrings )
	{
		::free( m_CommandStrings->data );
		m_CommandStrings = g_slist_remove( m_CommandStrings, m_CommandStrings->data );
	}
}

void CPluginSlot::Init(){
	CString str =   mpTable->m_pfnQERPlug_GetCommandList();
	char cTemp[1024];
	strcpy( cTemp, str );
	char* token = strtok( cTemp, ",;" );
	if ( token && *token == ' ' ) {
		while ( *token == ' ' )
			token++;
	}
	while ( token != NULL )
	{
		m_CommandStrings = g_slist_append( m_CommandStrings, strdup( token ) );
		token = strtok( NULL, ",;" );
	}
	mpTable->m_pfnQERPlug_Init( NULL, (void*)g_pParentWnd->m_pWidget );
	m_bReady = true;
}

const char* CPluginSlot::getMenuName(){
	return mpAPI->minor_name;
}

int CPluginSlot::getCommandCount(){
	if ( !m_bReady ) {
		Init();
	}
	return g_slist_length( m_CommandStrings );
}

const char* CPluginSlot::getCommand( int n ){
	if ( !m_bReady ) {
		Init();
	}
	return (char*)g_slist_nth_data( m_CommandStrings, n );
}

void CPluginSlot::addMenuID( int n ){
	m_CommandIDs = g_slist_append( m_CommandIDs, GINT_TO_POINTER( n ) );
}

bool CPluginSlot::ownsCommandID( int n ){
	GSList* lst;

	for ( lst = m_CommandIDs; lst != NULL; lst = g_slist_next( lst ) )
	{
		if ( GPOINTER_TO_INT( lst->data ) == n ) {
			return true;
		}
	}
	return false;
}

void CPluginSlot::Dispatch( const char *p ){
	vec3_t vMin, vMax;
	if ( selected_brushes.next == &selected_brushes ) {
		vMin[0] = vMin[1] = vMin[2] = 0;
		VectorCopy( vMin, vMax );
	}
	else
	{
		Select_GetBounds( vMin, vMax );
	}
	mpTable->m_pfnQERPlug_Dispatch( p, vMin, vMax, QE_SingleBrush( true ) );
}

CRadiantPluginManager::~CRadiantPluginManager(){
	list<CPluginSlot *>::iterator iSlot;
	for ( iSlot = mSlots.begin(); iSlot != mSlots.end(); iSlot++ )
	{
		delete *iSlot;
		*iSlot = NULL;
	}
}

bool CRadiantPluginManager::Dispatch( int n, const char* p ){
	list<CPluginSlot *>::iterator iPlug;
	for ( iPlug = mSlots.begin(); iPlug != mSlots.end(); iPlug++ )
	{
		CPluginSlot *pPlug = *iPlug;
		if ( pPlug->ownsCommandID( n ) ) {
			pPlug->Dispatch( p );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlugInManager::CPlugInManager(){
	PatchesMode = EActivePatches;
	m_PlugIns = NULL;
}

CPlugInManager::~CPlugInManager(){
	Cleanup();
}

void CPlugInManager::InitForDir( const Str &dir ){
	Str path;

	path = dir;
	path += g_strPluginsDir;
	// SYNAPSE
	g_pParentWnd->GetSynapseServer().AddSearchPath( path );

	if ( strcmp( g_strPluginsDir.GetBuffer(), g_strModulesDir.GetBuffer() ) != 0 ) {
		path = dir;
		path += g_strModulesDir;
		// SYNAPSE
		g_pParentWnd->GetSynapseServer().AddSearchPath( path );
	}
}

static const XMLConfigEntry_t manager_entries[] =
{
	{ VFS_MAJOR,            SYN_REQUIRE, sizeof( g_FileSystemTable ), &g_FileSystemTable },
	{ SHADERS_MAJOR,        SYN_REQUIRE, sizeof( g_ShadersTable ),    &g_ShadersTable },
	{ MAP_MAJOR,            SYN_REQUIRE, sizeof( g_MapTable ),        &g_MapTable },
	{ ECLASS_MAJOR,         SYN_REQUIRE, sizeof( g_EClassDefTable ),  &g_EClassDefTable },
	{ SURFACEDIALOG_MAJOR,  SYN_REQUIRE, sizeof( g_SurfaceTable ),    &g_SurfaceTable },
	{ NULL, SYN_UNKNOWN, 0, NULL }
};

void CPlugInManager::Init(){
	Str synapse_config;

	Cleanup();

	// set some globals
	g_qeglobals.bBSPFrontendPlugin = false;

	InitForDir( g_strGameToolsPath );
	InitForDir( g_strAppPath );

	synapse_config = g_strGameToolsPath;
	synapse_config += "synapse.config";
	if ( !g_pParentWnd->GetSynapseServer().Initialize( synapse_config.GetBuffer(), &Sys_Printf_VA ) ) {
		Error( "Synpase server initialization failed (see console)\n" );
	}

	// builtin modules
	g_pParentWnd->GetSynapseServer().EnumerateBuiltinModule( &eclass_def );

	// APIs we provide
	g_pParentWnd->GetSynapseClient().AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ) );
	g_pParentWnd->GetSynapseClient().AddAPI( SCRIPLIB_MAJOR, NULL, sizeof( _QERScripLibTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( BRUSH_MAJOR, NULL, sizeof( _QERBrushTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( APPSHADERS_MAJOR, NULL, sizeof( _QERAppShadersTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( QGL_MAJOR, NULL, sizeof( _QERQglTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( DATA_MAJOR, NULL, sizeof( _QERAppDataTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( PATCH_MAJOR, NULL, sizeof( _QERPatchTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( ECLASSMANAGER_MAJOR, NULL, sizeof( _EClassManagerTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( SELECTEDFACE_MAJOR, NULL, sizeof( _QERSelectedFaceTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( APPSURFACEDIALOG_MAJOR, NULL, sizeof( _QERAppSurfaceTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( UNDO_MAJOR, NULL, sizeof( _QERUndoTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( UI_MAJOR, NULL, sizeof( _QERUITable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( UIGTK_MAJOR, NULL, sizeof( _QERUIGtkTable ) );
	g_pParentWnd->GetSynapseClient().AddAPI( CAMERA_MAJOR, NULL, sizeof( _QERCameraTable ) );

	// modules configured by XML
	if ( !g_pParentWnd->GetSynapseClient().ConfigXML( &g_pParentWnd->GetSynapseServer(), "core", manager_entries ) ) {
		Error( "Synapse server initialization failed (see console)\n" );
	}

	// adding a manager is a special case that ConfigXML doesn't take care of
	g_pParentWnd->GetSynapseServer().SelectClientConfig( "core" );
	char *minor;
	if ( !g_pParentWnd->GetSynapseServer().GetConfigForAPI( IMAGE_MAJOR, &minor ) ) {
		Syn_Printf( "GetConfigForAPI '%s' failed - invalid XML config file?\n", IMAGE_MAJOR );
		Error( "Synapse server initialization failed (see console)\n" );
	}
	g_ImageManager.SetMatchAPI( IMAGE_MAJOR, minor );
	g_pParentWnd->GetSynapseClient().AddManager( &g_ImageManager );

	// SYN_REQUIRE entries which are still hardcoded
	g_pParentWnd->GetSynapseClient().AddAPI( MAP_MAJOR, "mapxml", sizeof( g_MapTable2 ), SYN_REQUIRE, &g_MapTable2 );
	g_pParentWnd->GetSynapseClient().AddAPI( ENTITY_MAJOR, NULL, sizeof( g_EntityTable ), SYN_REQUIRE, &g_EntityTable );

	// plugins: load anything that claims to be a plugin
	// minor becomes some kind of matching pattern
	// g_PluginsManager is an API any class, it receives several function tables as needed
	// you can't do a SYN_PROVIDE with that, has to be a SYN_REQUIRE ?
	g_PluginsManager.SetMatchAPI( PLUGIN_MAJOR, "*" );
	g_pParentWnd->GetSynapseClient().AddManager( &g_PluginsManager );
	g_pParentWnd->GetSynapseClient().AddManager( &g_ToolbarModuleManager );
	g_pParentWnd->GetSynapseClient().AddManager( &g_ModelManager );
	if ( !g_pParentWnd->GetSynapseServer().Resolve( &g_pParentWnd->GetSynapseClient() ) ) {
		Error( "synapse initialization fail (see console)" );
	}
	g_PluginsManager.PopulateMenu();
	g_ToolbarModuleManager.ConstructToolbar();
	InitFileTypes();
}

void CPlugInManager::Shutdown(){
	g_pParentWnd->GetSynapseServer().Shutdown();
}

void CPlugInManager::Cleanup(){
	int i;

	for ( i = 0; i < m_BrushHandles.GetSize(); i++ )
	{
		brush_t *pb = reinterpret_cast<brush_t*>( m_BrushHandles.GetAt( i ) );
		Brush_Free( pb );
	}
	m_BrushHandles.RemoveAll();

	for ( i = 0; i < m_EntityHandles.GetSize(); i++ )
	{
		entity_t *pe = reinterpret_cast<entity_t*>( m_EntityHandles.GetAt( i ) );
		Entity_Free( pe );
	}
	m_EntityHandles.RemoveAll();

	// patches
	// these are linked into the map
	m_PatchesHandles.RemoveAll();
	// these patches were allocated by Radiant on plugin request
	// if the list is not empty, it means either the plugin asked for allocation and never commited them to the map
	// in which case we are supposed to delete them
	// or it commited them but never called m_pfnReleasePatchHandles, in case the patches may have already been
	// erased and we are trying a second time, therefore crashing ..
	//++timo FIXME: for now I leave a leak warning, we'd need a table to keep track of commited patches
#ifdef _DEBUG
	if ( m_PluginPatches.GetSize() != 0 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: m_PluginPatches.GetSize() != 0 in CPlugInManager::Cleanup, possible leak\n" );
	}
#endif

/*	for (i = 0; i < m_PluginPatches.GetSize(); i++)
   {
    patchMesh_t *pMesh = reinterpret_cast<patchMesh_t*>(m_PluginPatches.GetAt(i));
    if (pMesh->pSymbiot)
      delete pMesh;
   }
   m_PluginPatches.RemoveAll(); */
}

void CPlugInManager::Dispatch( int n, const char * p ){
	g_PluginsManager.Dispatch( n, p );
}

void WINAPI QERApp_GetDispatchParams( vec3_t vMin, vec3_t vMax, bool *bSingleBrush ){
	if ( selected_brushes.next == &selected_brushes ) {
		vMin[0] = vMin[1] = vMin[2] = 0;
		VectorCopy( vMin, vMax );
	}
	else
	{
		Select_GetBounds( vMin, vMax );
	}

	if ( bSingleBrush ) {
		*bSingleBrush = QE_SingleBrush( true );
	}
}


// creates a dummy brush in the active brushes list
// FIXME : is this one really USED ?
void WINAPI QERApp_CreateBrush( vec3_t vMin, vec3_t vMax ){

	brush_t* pBrush = Brush_Create( vMin, vMax, &g_qeglobals.d_texturewin.texdef );
	Entity_LinkBrush( world_entity, pBrush );
	Brush_Build( pBrush );
	Brush_AddToList( pBrush, &active_brushes );
	Select_Brush( pBrush );
	Sys_UpdateWindows( W_ALL );
}

void* CPlugInManager::CreateBrushHandle(){
	brush_t *pb = Brush_Alloc();
	pb->numberId = g_nBrushId++;
	m_BrushHandles.Add( pb );
	return (void*)pb;
}

void CPlugInManager::DeleteBrushHandle( void * vp ){
	CPtrArray* pHandles[3];
	pHandles[0] = &m_SelectedBrushHandles;
	pHandles[1] = &m_ActiveBrushHandles;
	pHandles[2] = &m_BrushHandles;

	for ( int j = 0; j < 3; j++ )
	{
		for ( int i = 0; i < pHandles[j]->GetSize(); i++ )
		{
			brush_t *pb = reinterpret_cast<brush_t*>( pHandles[j]->GetAt( i ) );
			if ( pb == reinterpret_cast<brush_t*>( vp ) ) {
				if ( j == 2 ) {
					// only remove it from the list if it is work area
					// this allows the selected and active list indexes to remain constant
					// throughout a session (i.e. between an allocate and release)
					pHandles[j]->RemoveAt( i );
				}
				Brush_Free( pb );
				Sys_MarkMapModified(); // PGM
				return;
			}
		}
	}
}

void CPlugInManager::CommitBrushHandleToMap( void * vp ){
	g_bScreenUpdates = false;
	for ( int i = 0; i < m_BrushHandles.GetSize(); i++ )
	{
		brush_t *pb = reinterpret_cast<brush_t*>( m_BrushHandles.GetAt( i ) );
		if ( pb == reinterpret_cast<brush_t*>( vp ) ) {
			m_BrushHandles.RemoveAt( i );
			Entity_LinkBrush( world_entity, pb );
			Brush_Build( pb );
			Brush_AddToList( pb, &active_brushes );
			Select_Brush( pb );
		}
	}
	g_bScreenUpdates = true;
	Sys_UpdateWindows( W_ALL );
}

void CPlugInManager::AddFaceToBrushHandle( void * vp, vec3_t v1, vec3_t v2, vec3_t v3 ){
	brush_t *bp = FindBrushHandle( vp );
	if ( bp != NULL ) {
		face_t *f = Face_Alloc();
		f->texdef = g_qeglobals.d_texturewin.texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = bp->brush_faces;
		bp->brush_faces = f;
		VectorCopy( v1, f->planepts[0] );
		VectorCopy( v2, f->planepts[1] );
		VectorCopy( v3, f->planepts[2] );
	}
}

brush_t* CPlugInManager::FindBrushHandle( void * vp ){
	CPtrArray* pHandles[4];
	pHandles[0] = &m_SelectedBrushHandles;
	pHandles[1] = &m_ActiveBrushHandles;
	pHandles[2] = &m_BrushHandles;
	pHandles[3] = &m_EntityBrushHandles;

	for ( int j = 0; j < 4; j++ )
	{
		for ( int i = 0; i < pHandles[j]->GetSize(); i++ )
		{
			brush_t *pb = reinterpret_cast<brush_t*>( pHandles[j]->GetAt( i ) );
			if ( pb == reinterpret_cast<brush_t*>( vp ) ) {
				return pb;
			}
		}
	}
	return NULL;
}

patchMesh_t* CPlugInManager::FindPatchHandle( int index ){
	switch ( PatchesMode )
	{
	case EActivePatches:
	case ESelectedPatches:
		if ( index < m_PatchesHandles.GetSize() ) {
			brush_t *pb = reinterpret_cast<brush_t *>( m_PatchesHandles.GetAt( index ) );
			return pb->pPatch;
		}
#ifdef _DEBUG
		Sys_FPrintf( SYS_WRN, "WARNING: out of bounds in CPlugInManager::FindPatchHandle\n" );
#endif
		break;
	case EAllocatedPatches:
		if ( index < m_PluginPatches.GetSize() ) {
			patchMesh_t *pPatch = reinterpret_cast<patchMesh_t *>( m_PluginPatches.GetAt( index ) );
			return pPatch;
		}
#ifdef _DEBUG
		Sys_FPrintf( SYS_WRN, "WARNING: out of bounds in CPlugInManager::FindPatchHandle\n" );
#endif
		break;
	}
	return NULL;
}

void* WINAPI QERApp_CreateBrushHandle(){
	return g_pParentWnd->GetPlugInMgr().CreateBrushHandle();
}

void WINAPI QERApp_DeleteBrushHandle( void* vp ){
	g_pParentWnd->GetPlugInMgr().DeleteBrushHandle( vp );
}

void WINAPI QERApp_CommitBrushHandleToMap( void* vp ){
	g_pParentWnd->GetPlugInMgr().CommitBrushHandleToMap( vp );
}

void WINAPI QERApp_AddFace( void* vp, vec3_t v1, vec3_t v2, vec3_t v3 ){
	g_pParentWnd->GetPlugInMgr().AddFaceToBrushHandle( vp, v1, v2, v3 );
}

void WINAPI QERApp_DeleteSelection(){
	Select_Delete();
}

void QERApp_GetCamera( vec3_t origin, vec3_t angles ){
	VectorCopy( g_pParentWnd->GetCamWnd()->Camera()->origin, origin );
	VectorCopy( g_pParentWnd->GetCamWnd()->Camera()->angles, angles );
}

void QERApp_SetCamera( vec3_t origin, vec3_t angles ){
	VectorCopy( origin, g_pParentWnd->GetCamWnd()->Camera()->origin );
	VectorCopy( angles, g_pParentWnd->GetCamWnd()->Camera()->angles );

	Sys_UpdateWindows( W_ALL ); // specify
	g_pParentWnd->OnTimer();
}

void QERApp_GetCamWindowExtents( int *x, int *y, int *width, int *height ){
	GtkWidget *widget;

	if ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) {
		widget = g_pParentWnd->GetCamWnd()->m_pParent;
	}
	else{
		widget = g_pParentWnd->GetCamWnd()->GetWidget();
	}

	get_window_pos( widget, x, y );

	*width = g_pParentWnd->GetCamWnd()->Camera()->width;
	*height = g_pParentWnd->GetCamWnd()->Camera()->height;
}

//FIXME: this AcquirePath stuff is pretty much a mess and needs cleaned up
bool g_bPlugWait = false;
bool g_bPlugOK = false;
int g_nPlugCount = 0;

void _PlugDone( bool b, int n ){
	g_bPlugWait = false;
	g_bPlugOK = b;
	g_nPlugCount = n;
}

void WINAPI QERApp_GetPoints( int nMax, _QERPointData *pData, char* pMsg ){
	ShowInfoDialog( pMsg );
	g_bPlugWait = true;
	g_bPlugOK = false;
	g_nPlugCount = 0;
//  g_nPlugCount=nMax-1;
	AcquirePath( nMax, &_PlugDone );

	while ( g_bPlugWait )
		gtk_main_iteration();

	HideInfoDialog();

	pData->m_nCount = 0;
	pData->m_pVectors = NULL;

	if ( g_bPlugOK && g_nPlugCount > 0 ) {
		pData->m_nCount = g_nPlugCount;
		pData->m_pVectors = reinterpret_cast<vec3_t*>( qmalloc( g_nPlugCount * sizeof( vec3_t ) ) );
		vec3_t *pOut = pData->m_pVectors;
		for ( int i = 0; i < g_nPlugCount; i++ )
		{
			memcpy( pOut, &g_PathPoints[i],sizeof( vec3_t ) );
			pOut++;
		}
	}
}

//#define DBG_PAPI

void CheckTexture( face_t *f ){
	if ( !f->d_texture ) {
#ifdef DBG_PAPI
		Sys_Printf( "CheckTexture: requesting %s\n", f->texdef.name );
#endif
		f->pShader = QERApp_Shader_ForName( f->texdef.GetName() );
		f->pShader->IncRef();
		f->d_texture = f->pShader->getTexture();
	}
}

// expects pData->m_TextureName to be relative to "textures/"
void WINAPI QERApp_AddFaceData( void* pv, _QERFaceData *pData ){
#ifdef DBG_PAPI
	Sys_Printf( "FindBrushHandle..." );
#endif
	brush_t* pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
#ifdef DBG_PAPI
	Sys_Printf( "Done\n" );
#endif
	if ( pBrush != NULL ) {
		face_t *f = Face_Alloc();
		f->texdef = g_qeglobals.d_texturewin.texdef;
		f->texdef.flags = pData->m_nFlags;
		f->texdef.contents = pData->m_nContents;
		f->texdef.value = pData->m_nValue;
		f->texdef.SetName( pData->m_TextureName );
		f->next = pBrush->brush_faces;
		pBrush->brush_faces = f;
		VectorCopy( pData->m_v1, f->planepts[0] );
		VectorCopy( pData->m_v2, f->planepts[1] );
		VectorCopy( pData->m_v3, f->planepts[2] );
		// we might need to convert one way or the other if the input and the brush coordinates setting don't match
		if ( pData->m_bBPrimit == true ) {
			f->brushprimit_texdef = pData->brushprimit_texdef;
			if ( !g_qeglobals.m_bBrushPrimitMode ) {
				// before calling into the conversion, make sure we have a texture!
				CheckTexture( f );
#ifdef DBG_PAPI
				Sys_Printf( "BrushPrimitFaceToFace..." );
#endif

				// convert BP to regular
				BrushPrimitFaceToFace( f );
#ifdef DBG_PAPI
				Sys_Printf( "Done\n" );
#endif
			}
		}
		else
		{
#ifdef _DEBUG
			if ( pData->m_bBPrimit != false ) {
				Sys_FPrintf( SYS_WRN, "non-initialized pData->m_bBPrimit in QERApp_AddFaceData\n" );
			}
#endif
			f->texdef.rotate = pData->m_fRotate;
			f->texdef.shift[0] = pData->m_fShift[0];
			f->texdef.shift[1] = pData->m_fShift[1];
			f->texdef.scale[0] = pData->m_fScale[0];
			f->texdef.scale[1] = pData->m_fScale[1];
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				CheckTexture( f );
#ifdef DBG_PAPI
				Sys_Printf( "FaceToBrushPrimitFace..." );
#endif

				// convert regular to BP
				FaceToBrushPrimitFace( f );
#ifdef DBG_PAPI
				Sys_Printf( "Done\n" );
#endif
			}
		}
		Sys_MarkMapModified(); // PGM
	}
}

int WINAPI QERApp_GetFaceCount( void* pv ){
	int n = 0;
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		for ( face_t *f = pBrush->brush_faces ; f; f = f->next )
		{
			n++;
		}
	}
	return n;
}

_QERFaceData* WINAPI QERApp_GetFaceData( void* pv, int nFaceIndex ){
	static _QERFaceData face;
	int n = 0;
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );

	if ( pBrush != NULL ) {
		for ( face_t *f = pBrush->brush_faces ; f; f = f->next )
		{

#ifdef _DEBUG
			if ( !pBrush->brush_faces ) {
				Sys_FPrintf( SYS_WRN, "Warning : pBrush->brush_faces is NULL in QERApp_GetFaceData\n" );
				return NULL;
			}
#endif

			if ( n == nFaceIndex ) {
				face.m_nContents = f->texdef.contents;
				face.m_nFlags = f->texdef.flags;
				face.m_nValue = f->texdef.value;
				if ( g_qeglobals.m_bBrushPrimitMode ) {
					//++timo NOTE: we may want to convert back to old format for backward compatibility with some old plugins?
					face.m_bBPrimit = true;
					face.brushprimit_texdef = f->brushprimit_texdef;
				}
				else
				{
					face.m_fRotate = f->texdef.rotate;
					face.m_fScale[0] = f->texdef.scale[0];
					face.m_fScale[1] = f->texdef.scale[1];
					face.m_fShift[0] = f->texdef.shift[0];
					face.m_fShift[1] = f->texdef.shift[1];
				}
				strcpy( face.m_TextureName, f->texdef.GetName() );
				VectorCopy( f->planepts[0], face.m_v1 );
				VectorCopy( f->planepts[1], face.m_v2 );
				VectorCopy( f->planepts[2], face.m_v3 );
				return &face;
			}
			n++;
		}
	}
	return NULL;
}

void WINAPI QERApp_SetFaceData( void* pv, int nFaceIndex, _QERFaceData *pData ){
	int n = 0;
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );

	if ( pBrush != NULL ) {
		for ( face_t *f = pBrush->brush_faces ; f; f = f->next )
		{
			if ( n == nFaceIndex ) {
				f->texdef.flags = pData->m_nFlags;
				f->texdef.contents = pData->m_nContents;
				f->texdef.value = pData->m_nValue;
				f->texdef.rotate = pData->m_fRotate;
				f->texdef.shift[0] = pData->m_fShift[0];
				f->texdef.shift[1] = pData->m_fShift[1];
				f->texdef.scale[0] = pData->m_fScale[0];
				f->texdef.scale[1] = pData->m_fScale[1];
				//strcpy(f->texdef.name, pData->m_TextureName);
				f->texdef.SetName( pData->m_TextureName );
				VectorCopy( pData->m_v1, f->planepts[0] );
				VectorCopy( pData->m_v2, f->planepts[1] );
				VectorCopy( pData->m_v3, f->planepts[2] );
				Sys_MarkMapModified(); // PGM
				return;   // PGM
			}
			n++;
		}
	}
}

void WINAPI QERApp_DeleteFace( void* pv, int nFaceIndex ){
	int n = 0;
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		face_t *pPrev = pBrush->brush_faces;
		for ( face_t *f = pBrush->brush_faces; f; f = f->next )
		{
			if ( n == nFaceIndex ) {
				pPrev->next = f->next;
				Face_Free( f );
				Sys_MarkMapModified(); // PGM
				return;
			}
			n++;
			pPrev = f;
		}
	}
}

//==========
//PGM
void WINAPI QERApp_BuildBrush( void* pv ){
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		Brush_Build( pBrush );
		Sys_UpdateWindows( W_ALL );
	}
}

//Timo : another version with bConvert flag
//++timo since 1.7 is not compatible with earlier plugin versions, remove this one and update QERApp_BuildBrush
void WINAPI QERApp_BuildBrush2( void* pv, int bConvert ){
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		Brush_Build( pBrush, true, true, bConvert );
		Sys_UpdateWindows( W_ALL );
	}
}

void WINAPI QERApp_SelectBrush( void* pv ){
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		Select_Brush( pBrush, false );
		Sys_UpdateWindows( W_ALL );
	}

}

void WINAPI QERApp_DeselectBrush( void* pv ){
	// FIXME - implement this!
}

void WINAPI QERApp_ResetPlugins(){
	g_pParentWnd->OnPluginsRefresh();
}

void WINAPI QERApp_DeselectAllBrushes(){
	Select_Deselect();
	Sys_UpdateWindows( W_ALL );
}
//PGM
//==========

void WINAPI QERApp_TextureBrush( void* pv, char* pName ){
	brush_t *pBrush = g_pParentWnd->GetPlugInMgr().FindBrushHandle( pv );
	if ( pBrush != NULL ) {
		for ( face_t *f = pBrush->brush_faces ; f; f = f->next )
		{
			//strcpy(f->texdef.name, pName);
			f->texdef.SetName( pName );
		}
		Sys_MarkMapModified(); // PGM
	}
}

int WINAPI QERApp_SelectedBrushCount(){
	int n = 0;
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		n++;
	}
	return n;
}

int WINAPI QERApp_ActiveBrushCount(){
	int n = 0;
	for ( brush_t *pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		n++;
	}
	return n;
}

int WINAPI QERApp_AllocateSelectedBrushHandles(){
	int n = 0;
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		n++;
		g_pParentWnd->GetPlugInMgr().GetSelectedHandles().Add( pb );
	}
	return n;
}

int WINAPI QERApp_AllocateActiveBrushHandles(){
	int n = 0;
	for ( brush_t *pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		n++;
		g_pParentWnd->GetPlugInMgr().GetActiveHandles().Add( pb );
	}
	return n;
}

void WINAPI QERApp_ReleaseSelectedBrushHandles(){
	g_pParentWnd->GetPlugInMgr().GetSelectedHandles().RemoveAll();
	Sys_UpdateWindows( W_ALL );
}

void WINAPI QERApp_ReleaseActiveBrushHandles(){
	g_pParentWnd->GetPlugInMgr().GetActiveHandles().RemoveAll();
	Sys_UpdateWindows( W_ALL );
}

void* WINAPI QERApp_GetActiveBrushHandle( int nIndex ){
	if ( nIndex < g_pParentWnd->GetPlugInMgr().GetActiveHandles().GetSize() ) {
		return reinterpret_cast<void*>( g_pParentWnd->GetPlugInMgr().GetActiveHandles().GetAt( nIndex ) );
	}
	return NULL;
}

void* WINAPI QERApp_GetSelectedBrushHandle( int nIndex ){
	if ( nIndex < g_pParentWnd->GetPlugInMgr().GetSelectedHandles().GetSize() ) {
		return reinterpret_cast<void*>( g_pParentWnd->GetPlugInMgr().GetSelectedHandles().GetAt( nIndex ) );
	}
	return NULL;
}

int WINAPI QERApp_TextureCount(){
	//++timo TODO: replace by QERApp_GetActiveShaderCount and verify
	Texture_StartPos();
	int x, y;
	int n = 0;
	while ( 1 )
	{
		IShader *pShader = Texture_NextPos( &x, &y );
		if ( !pShader ) {
			break;
		}
		n++;
	}
	return n;
}

char* WINAPI QERApp_GetTexture( int nIndex ){
	//++timo TODO: replace by QERApp_ActiveShader_ForIndex
	// these funcs would end up being provided for backward compatibility
	static char name[QER_MAX_NAMELEN];
	Texture_StartPos();
	int x, y;
	int n = 0;
	while ( 1 )
	{
		IShader *pShader = Texture_NextPos( &x, &y );
		if ( !pShader ) {
			break;
		}
		if ( n == nIndex ) {
			strcpy( name, pShader->getName() );
			return name;
		}
		n++;
	}
	return NULL;
}

char* WINAPI QERApp_GetCurrentTexture(){
	static char current_tex[1024];
	strcpy( current_tex,g_qeglobals.d_texturewin.texdef.GetName() );
	return current_tex;
}

void WINAPI QERApp_SetCurrentTexture( char* strName ){
	//++timo hu ?? tex is not initialized ?? can be any value ..
	texdef_t tex;
	//++timo added a brushprimit_texdef ..
	// smthg to be done here
	brushprimit_texdef_t brushprimit_tex;
	//strcpy(tex.name, strName);
	tex.SetName( strName );
	Texture_SetTexture( &tex,&brushprimit_tex );
}

int WINAPI QERApp_GetEClassCount(){
	int n = 0;
	for ( eclass_t *e = eclass ; e ; e = e->next )
	{
		n++;
	}
	return n;
}

char* WINAPI QERApp_GetEClass( int nIndex ){
	int n = 0;
	for ( eclass_t *e = eclass ; e ; e = e->next )
	{
		if ( n == nIndex ) {
			return e->name;
		}
	}
	return NULL;
}

// v1.70 code
// world_entity holds the worldspawn and is indexed as 0
// other entities are in the entities doubly linked list
// QERApp_GetEntityCount counts the entities like in any C array: [0..length-1]
int WINAPI QERApp_GetEntityCount(){
	int n = 1;
	for ( entity_t *pe = entities.next ; pe != &entities ; pe = pe->next )
	{
		n++;
	}
	return n;
}

// We don't store entities in CPtrArray, we need to walk the list
void* WINAPI QERApp_GetEntityHandle( int nIndex ){
	if ( nIndex == 0 ) {
		// looks for the worldspawn
		return static_cast<void*>( world_entity );
	}
	entity_t *pe = &entities;
	int n = 0;
	while ( n < nIndex )
	{
		pe = pe->next;
		n++;
	}
	return static_cast<void*>( pe );
}

epair_t* WINAPI QERApp_AllocateEpair( const char *key, const char *val ){
	epair_t *e = (epair_t*)qmalloc( sizeof( *e ) );
	e->key = (char*)qmalloc( strlen( key ) + 1 );
	strcpy( e->key, key );
	e->value = (char*)qmalloc( strlen( val ) + 1 );
	strcpy( e->value, val );
	return e;
}

/*
   IEpair* WINAPI QERApp_IEpairForEntityHandle(void *vp)
   {
   entity_t *pe = static_cast<entity_t *>(vp);
   CEpairsWrapper *pEp = new CEpairsWrapper(pe);
   pEp->IncRef();
   return pEp;
   }

   IEpair* WINAPI QERApp_IEpairForProjectKeys()
   {
   CEpairsWrapper *pEp = new CEpairsWrapper(g_qeglobals.d_project_entity);
   pEp->IncRef();
   return pEp;
   }
 */

int WINAPI QERApp_AllocateEntityBrushHandles( void* vp ){
	entity_t *pe = static_cast<entity_t *>( vp );
	int n = 0;
	if ( !pe->brushes.onext ) {
		return 0;
	}
	g_pParentWnd->GetPlugInMgr().GetEntityBrushHandles().RemoveAll();
	for ( brush_t *pb = pe->brushes.onext ; pb != &pe->brushes ; pb = pb->onext )
	{
		n++;
		g_pParentWnd->GetPlugInMgr().GetEntityBrushHandles().Add( pb );
	}
	return n;
}

void WINAPI QERApp_ReleaseEntityBrushHandles(){
	g_pParentWnd->GetPlugInMgr().GetEntityBrushHandles().RemoveAll();
}

void* WINAPI QERApp_GetEntityBrushHandle( int nIndex ){
	if ( nIndex < g_pParentWnd->GetPlugInMgr().GetEntityBrushHandles().GetSize() ) {
		return g_pParentWnd->GetPlugInMgr().GetEntityBrushHandles().GetAt( nIndex );
	}
	return NULL;
}

// FIXME TTimo that entity handles thing sucks .. we should get rid of it ..

void* WINAPI QERApp_CreateEntityHandle(){
	entity_t *pe = reinterpret_cast<entity_t*>( qmalloc( sizeof( entity_t ) ) );
	pe->brushes.onext = pe->brushes.oprev = &pe->brushes;
	g_pParentWnd->GetPlugInMgr().GetEntityHandles().Add( static_cast<void*>( pe ) );
	return static_cast<void*>( pe );
}

// the vpBrush needs to be in m_BrushHandles
//++timo we don't have allocation nor storage for vpEntity, no checks for this one
void WINAPI QERApp_CommitBrushHandleToEntity( void* vpBrush, void* vpEntity ){
	g_pParentWnd->GetPlugInMgr().CommitBrushHandleToEntity( vpBrush, vpEntity );
	return;
}

const char* QERApp_ReadProjectKey( const char* key ){
	return ValueForKey( g_qeglobals.d_project_entity, key );
}

#ifdef USEPLUGINENTITIES

int WINAPI QERApp_ScanFileForEClass( char *filename ){
	// set single class parsing
	parsing_single = true;
	Eclass_ScanFile( filename );
	if ( eclass_found ) {
		eclass_e->nShowFlags |= ECLASS_PLUGINENTITY;
		return 1;
	}
	return 0;
}
#endif // USEPLUGINENTITIES

// the vpBrush needs to be in m_BrushHandles
//++timo add a debug check to see if we found the brush handle
// NOTE : seems there's no way to check vpEntity is valid .. this is dangerous
// links the brush to its entity, everything else is done when commiting the entity to the map
void CPlugInManager::CommitBrushHandleToEntity( void* vpBrush, void* vpEntity ){
	brush_t* pb;
	entity_t* pe;
	for ( int i = 0 ; i < m_BrushHandles.GetSize() ; i++ )
	{
		if ( vpBrush == m_BrushHandles.GetAt( i ) ) {
			m_BrushHandles.RemoveAt( i );
			pb = reinterpret_cast<brush_t*>( vpBrush );
			pe = reinterpret_cast<entity_t *>( vpEntity );
			Entity_LinkBrush( pe, pb );
		}
	}
	Sys_UpdateWindows( W_ALL );
}

// the vpEntity must be in m_EntityHandles
void WINAPI QERApp_CommitEntityHandleToMap( void* vpEntity ){
	g_pParentWnd->GetPlugInMgr().CommitEntityHandleToMap( vpEntity );
	return;
}

int WINAPI QERApp_LoadFile( const char *pLocation, void ** buffer ){
	int nSize = vfsLoadFile( pLocation, buffer, 0 );
	return nSize;
}

char * WINAPI QERApp_ExpandReletivePath( char *p ){
	return ExpandReletivePath( p );
}

qtexture_t* WINAPI QERApp_Texture_ForName( const char *name ){
	// if the texture is not loaded yet, this call will get it loaded
	// but: when we assign a GL bind number, we need to be in the g_qeglobals.d_xxxBase GL context
	// the plugin may set the GL context to whatever he likes, but then load would fail
	// NOTE: is context switching time-consuming? then maybe the plugin could handle the context
	// switch and only add a sanity check in debug mode here
	// read current context
	gtk_glwidget_make_current( g_qeglobals_gui.d_glBase );

	//++timo debugging
	Sys_FPrintf( SYS_WRN, "WARNING: QERApp_Texture_ForName ... don't call that!!\n" );
	qtexture_t* qtex = QERApp_Texture_ForName2( name );
	return qtex;
}

char* QERApp_Token(){
	return token;
}

int QERApp_ScriptLine(){
	return scriptline;
}

// we save the map and return the name .. either .map or .reg to support region compiling
char* QERApp_GetMapName(){
	static char name[PATH_MAX];
	SaveWithRegion( name );
	return name;
}

void CPlugInManager::CommitEntityHandleToMap( void* vpEntity ){
	entity_t *pe;
	eclass_t *e;
	brush_t   *b;
	vec3_t mins,maxs;
	bool has_brushes;
	for ( int i = 0 ; i < m_EntityHandles.GetSize() ; i++ )
	{
		if ( vpEntity == m_EntityHandles.GetAt( i ) ) {
			m_EntityHandles.RemoveAt( i );
			pe = reinterpret_cast<entity_t*>( vpEntity );
			// fill additional fields
			// straight copy from Entity_Parse
			// entity_t::origin
			GetVectorForKey( pe, "origin", pe->origin );
			// entity_t::eclass
			if ( pe->brushes.onext == &pe->brushes ) {
				has_brushes = false;
			}
			else{
				has_brushes = true;
			}
			e = Eclass_ForName( ValueForKey( pe, "classname" ), has_brushes );
			pe->eclass = e;
			// fixedsize
			if ( e->fixedsize ) {
				if ( pe->brushes.onext != &pe->brushes ) {
					Sys_FPrintf( SYS_WRN, "Warning : Fixed size entity with brushes in CPlugInManager::CommitEntityHandleToMap\n" );
				}
				// create a custom brush
				VectorAdd( e->mins, pe->origin, mins );
				VectorAdd( e->maxs, pe->origin, maxs );
/*
        float a = 0;
        if (e->nShowFlags & ECLASS_MISCMODEL)
        {
          char* p = ValueForKey(pe, "model");
          if (p != NULL && strlen(p) > 0)
          {
            vec3_t vMin, vMax;
            a = FloatForKey (pe, "angle");
            if (GetCachedModel(pe, p, vMin, vMax))
            {
              // create a custom brush
              VectorAdd (pe->md3Class->mins, pe->origin, mins);
              VectorAdd (pe->md3Class->maxs, pe->origin, maxs);
            }
          }
        }
 */
				b = Brush_Create( mins, maxs, &e->texdef );
/*
        if (a)
        {
          vec3_t vAngle;
          vAngle[0] = vAngle[1] = 0;
          vAngle[2] = a;
          Brush_Rotate(b, vAngle, pe->origin, false);
        }
 */
				b->owner = pe;

				b->onext = pe->brushes.onext;
				b->oprev = &pe->brushes;
				pe->brushes.onext->oprev = b;
				pe->brushes.onext = b;
			}
			else
			{ // brush entity
				if ( pe->brushes.next == &pe->brushes ) {
					Sys_FPrintf( SYS_WRN, "Warning: Brush entity with no brushes in CPlugInManager::CommitEntityHandleToMap\n" );
				}
			}

			// add brushes to the active brushes list
			// and build them along the way
			for ( b = pe->brushes.onext ; b != &pe->brushes ; b = b->onext )
			{
				// convert between old brushes and brush primitive
				if ( g_qeglobals.m_bBrushPrimitMode ) {
					// we only filled the shift scale rot fields, needs conversion
					Brush_Build( b, true, true, true );
				}
				else
				{
					// we are using old brushes
					Brush_Build( b );
				}
				b->next = active_brushes.next;
				active_brushes.next->prev = b;
				b->prev = &active_brushes;
				active_brushes.next = b;
			}

			// handle worldspawn entities
			// if worldspawn has no brushes, use the new one
			if ( !strcmp( ValueForKey( pe, "classname" ), "worldspawn" ) ) {
				if ( world_entity && ( world_entity->brushes.onext != &world_entity->brushes ) ) {
					// worldspawn already has brushes
					Sys_Printf( "Commiting worldspawn as func_group\n" );
					SetKeyValue( pe, "classname", "func_group" );
					// add the entity to the end of the entity list
					pe->next = &entities;
					pe->prev = entities.prev;
					entities.prev->next = pe;
					entities.prev = pe;
					g_qeglobals.d_num_entities++;
				}
				else
				{
					// there's a worldspawn with no brushes, we assume the map is empty
					if ( world_entity ) {
						Entity_Free( world_entity );
						world_entity = pe;
					}
					else{
						Sys_FPrintf( SYS_WRN, "Warning : unexpected world_entity == NULL in CommitEntityHandleToMap\n" );
					}
				}
			}
			else
			{
				// add the entity to the end of the entity list
				pe->next = &entities;
				pe->prev = entities.prev;
				entities.prev->next = pe;
				entities.prev = pe;
				g_qeglobals.d_num_entities++;
			}
		}
	}
}

void WINAPI QERApp_SetScreenUpdate( int bScreenUpdates ){
	g_bScreenUpdates = bScreenUpdates;
}

texturewin_t* QERApp_QeglobalsTexturewin(){
	return &g_qeglobals.d_texturewin;
}

texdef_t* QERApp_QeglobalsSavedinfo_SIInc(){
	return &g_qeglobals.d_savedinfo.m_SIIncrement;
}

patchMesh_t* QERApp_GetSelectedPatch(){
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			return pb->pPatch;
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_WRN, "WARNING: QERApp_GetSelectedPatchTexdef called with no patch selected\n" );
#endif
	return NULL;
}

const char* WINAPI QERApp_GetGamePath(){
	return g_pGameDescription->mEnginePath.GetBuffer();
}

/*!
   \todo the name of this API should prolly be changed
   would also need to prompt g_strAppPath / g_strGameToolsPath independently?
 */
// SPoG
// changed g_strGameToolsPath to g_strAppPath
const char* WINAPI QERApp_GetQERPath(){
	return g_strAppPath.GetBuffer();
}

const char* WINAPI QERApp_GetGameFile(){
	// FIXME: Arnout: temp solution, need proper 'which game is this' indicator or a different solution for plugins/modules
	return g_pGameDescription->mGameFile.GetBuffer();
}

// patches in/out -----------------------------------
int WINAPI QERApp_AllocateActivePatchHandles(){
	return g_pParentWnd->GetPlugInMgr().AllocateActivePatchHandles();
}

// Grid Size
float QERApp_QeglobalsGetGridSize(){
	return g_qeglobals.d_gridsize;
}

int CPlugInManager::AllocateActivePatchHandles(){
	int n = 0;
	for ( brush_t *pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			n++;
			m_PatchesHandles.Add( pb );
		}
	}
	return n;
}

int WINAPI QERApp_AllocateSelectedPatchHandles(){
	return g_pParentWnd->GetPlugInMgr().AllocateSelectedPatchHandles();
}

int CPlugInManager::AllocateSelectedPatchHandles(){
	int n = 0;
	// change mode
	PatchesMode = ESelectedPatches;
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			n++;
			m_PatchesHandles.Add( pb );
		}
	}
	return n;
}

void WINAPI QERApp_ReleasePatchHandles(){
	g_pParentWnd->GetPlugInMgr().ReleasePatchesHandles();
}

patchMesh_t* WINAPI QERApp_GetPatchData( int index ){
	static patchMesh_t patch;
	patchMesh_t *pPatch = g_pParentWnd->GetPlugInMgr().FindPatchHandle( index );
	if ( pPatch ) {
		memcpy( &patch, pPatch, sizeof( patchMesh_t ) );
		return &patch;
	}
	return NULL;
}

patchMesh_t* WINAPI QERApp_GetPatchHandle( int index ){
	return g_pParentWnd->GetPlugInMgr().FindPatchHandle( index );
}

void WINAPI QERApp_DeletePatch( int index ){
	patchMesh_t *pPatch = g_pParentWnd->GetPlugInMgr().FindPatchHandle( index );
	if ( pPatch ) {
		brush_t *pb = pPatch->pSymbiot;
		Patch_Delete( pPatch );
		if ( pb ) {
			Brush_Free( pb );
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_WRN, "Warning: QERApp_DeletePatch: FindPatchHandle failed\n" );
#endif
}

int WINAPI QERApp_CreatePatchHandle(){
	return g_pParentWnd->GetPlugInMgr().CreatePatchHandle();
}

int CPlugInManager::CreatePatchHandle(){
	// NOTE: we can't call the AddBrushForPatch until we have filled the patchMesh_t structure
	patchMesh_t *pPatch = MakeNewPatch();
	m_PluginPatches.Add( pPatch );
	// change mode
	PatchesMode = EAllocatedPatches;
	return m_PluginPatches.GetSize() - 1;
}

void WINAPI QERApp_CommitPatchHandleToMap( int index, patchMesh_t *pMesh, char *texName ){
#ifdef DBG_PAPI
	Sys_Printf( "QERApp_CommitPatchHandleToMap %i..", index );
#endif
	g_pParentWnd->GetPlugInMgr().CommitPatchHandleToMap( index, pMesh, texName );
#ifdef DBG_PAPI
	Sys_Printf( "Done\n" );
#endif
}

void WINAPI QERApp_CommitPatchHandleToEntity( int index, patchMesh_t *pMesh, char *texName, void* vpEntity ){
#ifdef DBG_PAPI
	Sys_Printf( "QERApp_CommitPatchHandleToEntity %i..", index );
#endif
	g_pParentWnd->GetPlugInMgr().CommitPatchHandleToEntity( index, pMesh, texName, vpEntity );
#ifdef DBG_PAPI
	Sys_Printf( "Done\n" );
#endif
}

void CPlugInManager::CommitPatchHandleToMap( int index, patchMesh_t *pMesh, char *texName ){
	if ( PatchesMode == EAllocatedPatches ) {
		patchMesh_t *pPatch = reinterpret_cast<patchMesh_t *>( m_PluginPatches.GetAt( index ) );
		memcpy( pPatch, pMesh, sizeof( patchMesh_t ) );
		// patch texturing, if none given use current texture
		if ( texName ) {
			pPatch->pShader = QERApp_Shader_ForName( texName );
		}
		else{
			pPatch->pShader = QERApp_Shader_ForName( g_qeglobals.d_texturewin.texdef.GetName() );
		}
		pPatch->d_texture = pPatch->pShader->getTexture();
		pPatch->pShader->IncRef();
		g_bScreenUpdates = false;
		// the bLinkToWorld flag in AddBrushForPatch takes care of Brush_AddToList Entity_linkBrush and Brush_Build
		brush_t *pb = AddBrushForPatch( pPatch, true );
		Select_Brush( pb );
		g_bScreenUpdates = true;
		Sys_UpdateWindows( W_ALL );
	}
	else
	{
		brush_t *pBrush = reinterpret_cast<brush_t *>( m_PatchesHandles.GetAt( index ) );
		patchMesh_t *pPatch = pBrush->pPatch;
		pPatch->width = pMesh->width;
		pPatch->height = pMesh->height;
		pPatch->contents = pMesh->contents;
		pPatch->flags = pMesh->flags;
		pPatch->value = pMesh->value;
		pPatch->type = pMesh->type;
		memcpy( pPatch->ctrl, pMesh->ctrl, sizeof( drawVert_t ) * MAX_PATCH_HEIGHT * MAX_PATCH_WIDTH );
		pPatch->bDirty = true;
	}
}

void CPlugInManager::CommitPatchHandleToEntity( int index, patchMesh_t *pMesh, char *texName, void *vpEntity ){
	entity_t* pe = reinterpret_cast<entity_t *>( vpEntity );

	if ( PatchesMode == EAllocatedPatches ) {
		patchMesh_t *pPatch = reinterpret_cast<patchMesh_t *>( m_PluginPatches.GetAt( index ) );
		memcpy( pPatch, pMesh, sizeof( patchMesh_t ) );
		// patch texturing, if none given use current texture
		if ( texName ) {
			pPatch->pShader = QERApp_Shader_ForName( texName );
		}
		else{
			pPatch->pShader = QERApp_Shader_ForName( g_qeglobals.d_texturewin.texdef.GetName() );
		}
		pPatch->d_texture = pPatch->pShader->getTexture();
		pPatch->pShader->IncRef();
		g_bScreenUpdates = false;
		brush_t *pb = AddBrushForPatch( pPatch, false ); // false, sp have to do the brush building/entity linking ourself
		Brush_AddToList( pb, &active_brushes );
		Entity_LinkBrush( pe, pb );
		Brush_Build( pb );
		g_bScreenUpdates = true;
		Sys_UpdateWindows( W_ALL );
	}
	else
	{
		brush_t *pBrush = reinterpret_cast<brush_t *>( m_PatchesHandles.GetAt( index ) );
		patchMesh_t *pPatch = pBrush->pPatch;
		pPatch->width = pMesh->width;
		pPatch->height = pMesh->height;
		pPatch->contents = pMesh->contents;
		pPatch->flags = pMesh->flags;
		pPatch->value = pMesh->value;
		pPatch->type = pMesh->type;
		memcpy( pPatch->ctrl, pMesh->ctrl, sizeof( drawVert_t ) * MAX_PATCH_HEIGHT * MAX_PATCH_WIDTH );
		pPatch->bDirty = true;
	}
}

#if 0

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
  #include <gdk/gdkx.h>

XVisualInfo* QEX_ChooseVisual( bool zbuffer ){
	int attrlist_z[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, 0};
	int attrlist[] = { GLX_RGBA, GLX_DOUBLEBUFFER, 0};
	XVisualInfo *vi;
	Display *dpy;

	dpy = GDK_DISPLAY();
	if ( dpy == NULL ) {
		Error( "OpenGL fatal error: Cannot get display.\n" );
	}
	vi = qglXChooseVisual( dpy, DefaultScreen( dpy ), zbuffer ? attrlist_z : attrlist );
	if ( vi == NULL ) {
		Error( "OpenGL fatal error: glXChooseVisual failed.\n" );
	}

	return vi;
}
#endif

#endif

/*!
   \todo FIXME TTimo broken most likely
   actually .. that's not enough, you have to go down for the game pack specific?
 */
const char* WINAPI QERApp_ProfileGetDirectory(){
	return g_strTempPath;
}

GtkWidget* WINAPI QERApp_GetQeGlobalsGLWidget(){
	return g_qeglobals_gui.d_glBase;
}

qboolean WINAPI BrushPrimitMode(){
	return g_qeglobals.m_bBrushPrimitMode;
}

brush_t* WINAPI QERApp_ActiveBrushes(){
	return &active_brushes;
}

brush_t* WINAPI QERApp_SelectedBrushes(){
	return &selected_brushes;
}

brush_t* WINAPI QERApp_FilteredBrushes(){
	return &filtered_brushes;
}

CPtrArray* WINAPI QERApp_LstSkinCache(){
	return &g_lstSkinCache;
}

qtexture_t** WINAPI QERApp_QTextures(){
	return &g_qeglobals.d_qtextures;
}

GHashTable* WINAPI QERApp_QTexmap(){
	return g_qeglobals.d_qtexmap;
}

// a simplified version of Texture_SetTexture
void WINAPI QERApp_Texture_SetTexture( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef ){
	Texture_SetTexture( texdef, brushprimit_texdef );
}

void QERApp_LoadImage( const char *name, unsigned char **pic, int *width, int *height ){
	g_ImageManager.LoadImage( name, pic, width, height );
}

unsigned long QERApp_GetTickCount(){
#ifdef _WIN32
	return GetTickCount();
#else
	struct timeval tp;
	struct timezone tzp;
	static int basetime = 0;

	gettimeofday( &tp, &tzp );
	if ( !basetime ) {
		basetime = tp.tv_sec;
	}
	return ( tp.tv_sec - basetime ) + tp.tv_usec / 1000;
#endif
}

bool CSynapseClientRadiant::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, RADIANT_MAJOR ) ) {
		_QERFuncTable_1* pTable = static_cast<_QERFuncTable_1*>( pAPI->mpTable );
		pTable->m_pfnCreateBrush = &QERApp_CreateBrush;
		pTable->m_pfnCreateBrushHandle = &QERApp_CreateBrushHandle;
		pTable->m_pfnDeleteBrushHandle = &QERApp_DeleteBrushHandle;
		pTable->m_pfnCommitBrushHandle = &QERApp_CommitBrushHandleToMap;
		pTable->m_pfnAddFace = &QERApp_AddFace;
		pTable->m_pfnAddFaceData = &QERApp_AddFaceData;
		pTable->m_pfnGetFaceData = &QERApp_GetFaceData;
		pTable->m_pfnGetFaceCount = &QERApp_GetFaceCount;
		pTable->m_pfnSetFaceData = &QERApp_SetFaceData;
		pTable->m_pfnDeleteFace = &QERApp_DeleteFace;
		pTable->m_pfnTextureBrush = &QERApp_TextureBrush;
		pTable->m_pfnBuildBrush = &QERApp_BuildBrush;       // PGM
		pTable->m_pfnSelectBrush = &QERApp_SelectBrush;     // PGM
		pTable->m_pfnDeselectBrush = &QERApp_DeselectBrush;   // PGM
		pTable->m_pfnDeselectAllBrushes = &QERApp_DeselectAllBrushes; // PGM
		pTable->m_pfnDeleteSelection = &QERApp_DeleteSelection;
		pTable->m_pfnGetPoints = &QERApp_GetPoints;
		pTable->m_pfnSelectedBrushCount = &QERApp_SelectedBrushCount;
		pTable->m_pfnAllocateSelectedBrushHandles  = &QERApp_AllocateSelectedBrushHandles;
		pTable->m_pfnReleaseSelectedBrushHandles  = &QERApp_ReleaseSelectedBrushHandles;
		pTable->m_pfnGetSelectedBrushHandle = &QERApp_GetSelectedBrushHandle;
		pTable->m_pfnActiveBrushCount = &QERApp_ActiveBrushCount;
		pTable->m_pfnAllocateActiveBrushHandles = &QERApp_AllocateActiveBrushHandles;
		pTable->m_pfnReleaseActiveBrushHandles = &QERApp_ReleaseActiveBrushHandles;
		pTable->m_pfnGetActiveBrushHandle = &QERApp_GetActiveBrushHandle;
		pTable->m_pfnTextureCount = &QERApp_TextureCount;
		pTable->m_pfnGetTexture = &QERApp_GetTexture;
		pTable->m_pfnGetCurrentTexture = &QERApp_GetCurrentTexture;
		pTable->m_pfnSetCurrentTexture = &QERApp_SetCurrentTexture;
		pTable->m_pfnGetEClassCount = &QERApp_GetEClassCount;
		pTable->m_pfnGetEClass = &QERApp_GetEClass;
		pTable->m_pfnResetPlugins = &QERApp_ResetPlugins;
		pTable->m_pfnLoadTextureRGBA = &QERApp_LoadTextureRGBA;
		pTable->m_pfnGetEntityCount = &QERApp_GetEntityCount;
		pTable->m_pfnGetEntityHandle = &QERApp_GetEntityHandle;
		pTable->m_pfnAllocateEpair = &QERApp_AllocateEpair;
		pTable->m_pfnAllocateEntityBrushHandles = &QERApp_AllocateEntityBrushHandles;
		pTable->m_pfnReleaseEntityBrushHandles = &QERApp_ReleaseEntityBrushHandles;
		pTable->m_pfnGetEntityBrushHandle = &QERApp_GetEntityBrushHandle;
		pTable->m_pfnCreateEntityHandle = &QERApp_CreateEntityHandle;
		pTable->m_pfnCommitBrushHandleToEntity = &QERApp_CommitBrushHandleToEntity;
		pTable->m_pfnCommitEntityHandleToMap = &QERApp_CommitEntityHandleToMap;
		pTable->m_pfnSetScreenUpdate = &QERApp_SetScreenUpdate;
		pTable->m_pfnBuildBrush2 = &QERApp_BuildBrush2;
		pTable->m_pfnGetDispatchParams = &QERApp_GetDispatchParams;
//    pTable->m_pfnRequestInterface = &QERApp_RequestInterface;
		pTable->m_pfnError = &Error;
		pTable->m_pfnLoadFile = &QERApp_LoadFile;
		pTable->m_pfnExpandReletivePath = &QERApp_ExpandReletivePath;
		pTable->m_pfnQE_ConvertDOSToUnixName = &QE_ConvertDOSToUnixName;
		pTable->m_pfnHasShader = QERApp_HasShader;
		pTable->m_pfnTexture_LoadSkin = &Texture_LoadSkin;
		pTable->m_pfnGetGamePath = &QERApp_GetGamePath;
		pTable->m_pfnGetQERPath = &QERApp_GetQERPath;
		pTable->m_pfnGetGameFile = &QERApp_GetGameFile;
		pTable->m_pfnAllocateActivePatchHandles = &QERApp_AllocateActivePatchHandles;
		pTable->m_pfnAllocateSelectedPatchHandles = &QERApp_AllocateSelectedPatchHandles;
		pTable->m_pfnReleasePatchHandles = &QERApp_ReleasePatchHandles;
		pTable->m_pfnGetPatchData = &QERApp_GetPatchData;
		pTable->m_pfnGetPatchHandle = &QERApp_GetPatchHandle;
		pTable->m_pfnDeletePatch = &QERApp_DeletePatch;
		pTable->m_pfnCreatePatchHandle = &QERApp_CreatePatchHandle;
		pTable->m_pfnCommitPatchHandleToMap = &QERApp_CommitPatchHandleToMap;
		pTable->m_pfnCommitPatchHandleToEntity = &QERApp_CommitPatchHandleToEntity;
		pTable->m_pfnLoadImage = &QERApp_LoadImage;
		pTable->m_pfnMessageBox = &gtk_MessageBox;
		pTable->m_pfnFileDialog = &file_dialog;
		pTable->m_pfnColorDialog = &color_dialog;
		pTable->m_pfnDirDialog = &dir_dialog;
		pTable->m_pfnLoadBitmap = &load_plugin_bitmap;
		pTable->m_pfnProfileGetDirectory = &QERApp_ProfileGetDirectory;
		pTable->m_pfnProfileSaveInt = &profile_save_int;
		pTable->m_pfnProfileSaveString = &profile_save_string;
		pTable->m_pfnProfileLoadInt = &profile_load_int;
		pTable->m_pfnProfileLoadString = &profile_load_string;
		pTable->m_pfnSysUpdateWindows = &Sys_UpdateWindows;
		pTable->m_pfnSysPrintf = &Sys_Printf;
		pTable->m_pfnSysFPrintf = &Sys_FPrintf;
		pTable->m_pfnSysBeginWait = &Sys_BeginWait;
		pTable->m_pfnSysEndWait = &Sys_EndWait;
		pTable->m_pfnSys_SetTitle = &Sys_SetTitle;
		pTable->m_pfnSysBeep = &Sys_Beep;
		pTable->m_pfnSys_Status = &Sys_Status;
		pTable->m_pfnMapFree = &Map_Free;
		pTable->m_pfnMapNew = &Map_New;
		pTable->m_pfnMapBuildBrushData = &Map_BuildBrushData;
		pTable->m_pfnMap_IsBrushFiltered = &Map_IsBrushFiltered;
		pTable->m_pfnMapStartPosition = &Map_StartPosition;
		pTable->m_pfnMapRegionOff = &Map_RegionOff;
		pTable->m_pfnSetBuildWindingsNoTexBuild = &Brush_SetBuildWindingsNoTexBuild;
		pTable->m_pfnPointFileClear = &Pointfile_Clear;
		pTable->m_pfnCSG_MakeHollow = &CSG_MakeHollow;
		pTable->m_pfnCSG_MakeHollowMode = &CSG_MakeHollowMode;
		pTable->m_pfnRegionSpawnPoint = &Region_SpawnPoint;
		pTable->m_pfnQGetTickCount = &QERApp_GetTickCount;
		pTable->m_pfnGetModelCache = &GetModelCache;
		pTable->m_pfnGetFileTypeRegistry = &GetFileTypeRegistry;
		pTable->m_pfnReadProjectKey = &QERApp_ReadProjectKey;
		pTable->m_pfnGetMapName = &QERApp_GetMapName;
		pTable->m_pfnFilterAdd = &FilterCreate;
		pTable->m_pfnFiltersActivate = &FiltersActivate;

		return true;
	}
	if ( !strcmp( pAPI->major_name, SCRIPLIB_MAJOR ) ) {
		_QERScripLibTable *pScripLibTable = static_cast<_QERScripLibTable *>( pAPI->mpTable );
		pScripLibTable->m_pfnGetToken = &GetToken;
		pScripLibTable->m_pfnGetTokenExtra = &GetTokenExtra;
		pScripLibTable->m_pfnToken = &QERApp_Token;
		pScripLibTable->m_pfnUnGetToken = &UngetToken;
		pScripLibTable->m_pfnStartTokenParsing = &StartTokenParsing;
		pScripLibTable->m_pfnScriptLine = &QERApp_ScriptLine;
		pScripLibTable->m_pfnTokenAvailable = &TokenAvailable;
		pScripLibTable->m_pfnCOM_Parse = &COM_Parse;
		pScripLibTable->m_pfnGet_COM_Token = &Get_COM_Token;

		return true;
	}
	if ( !strcmp( pAPI->major_name, BRUSH_MAJOR ) ) {
		_QERBrushTable *pBrushTable = static_cast<_QERBrushTable *>( pAPI->mpTable );
		pBrushTable->m_pfnBP_MessageBox = &BP_MessageBox;
		pBrushTable->m_pfnBrush_AddToList = &Brush_AddToList;
		pBrushTable->m_pfnBrush_Build = &Brush_Build;
		pBrushTable->m_pfnBrush_Create = &Brush_Create;
		pBrushTable->m_pfnBrush_Free = &Brush_Free;
		pBrushTable->m_pfnBrush_Rotate = &Brush_Rotate;
		pBrushTable->m_pfnBrushAlloc = &Brush_Alloc;
		pBrushTable->m_pfnFace_Alloc = &Face_Alloc;
		pBrushTable->m_pfnHasModel = NULL; // &HasModel;

		return true;
	}
	if ( !strcmp( pAPI->major_name, APPSHADERS_MAJOR ) ) {
		_QERAppShadersTable *pShadersTable = static_cast<_QERAppShadersTable*>( pAPI->mpTable );
		pShadersTable->m_pfnQTextures = QERApp_QTextures;
		pShadersTable->m_pfnQTexmap = QERApp_QTexmap;
		pShadersTable->m_pfnQeglobalsTexturewin = QERApp_QeglobalsTexturewin;
		pShadersTable->m_pfnTexture_SetTexture = QERApp_Texture_SetTexture;
		pShadersTable->m_pfnTexture_ShowInuse = Texture_ShowInuse;
		pShadersTable->m_pfnBuildShaderList = &BuildShaderList;
		pShadersTable->m_pfnPreloadShaders = &PreloadShaders;

		return true;
	}
	if ( !strcmp( pAPI->major_name, QGL_MAJOR ) ) {
		_QERQglTable *pQglTable = static_cast<_QERQglTable *>( pAPI->mpTable );
		pQglTable->m_pfn_qglAlphaFunc = qglAlphaFunc;
		pQglTable->m_pfn_qglBegin = qglBegin;
		pQglTable->m_pfn_qglBindTexture = qglBindTexture;
		pQglTable->m_pfn_qglBlendFunc = qglBlendFunc;
		pQglTable->m_pfn_qglCallList = qglCallList;
		pQglTable->m_pfn_qglCallLists = qglCallLists;
		pQglTable->m_pfn_qglClear = qglClear;
		pQglTable->m_pfn_qglClearColor = qglClearColor;
		pQglTable->m_pfn_qglClearDepth = qglClearDepth;
		pQglTable->m_pfn_qglColor3f = qglColor3f;
		pQglTable->m_pfn_qglColor3fv = qglColor3fv;
		pQglTable->m_pfn_qglColor4f = qglColor4f;
		pQglTable->m_pfn_qglColor4fv = qglColor4fv;
		pQglTable->m_pfn_qglColor4ubv = qglColor4ubv;
		pQglTable->m_pfn_qglColorPointer = qglColorPointer;
		pQglTable->m_pfn_qglCullFace = qglCullFace;
		pQglTable->m_pfn_qglDepthFunc = qglDepthFunc;
		pQglTable->m_pfn_qglDepthMask = qglDepthMask;
		pQglTable->m_pfn_qglDisable = qglDisable;
		pQglTable->m_pfn_qglDisableClientState = qglDisableClientState;
		pQglTable->m_pfn_qglDeleteLists = qglDeleteLists;
		pQglTable->m_pfn_qglDeleteTextures = qglDeleteTextures;
		pQglTable->m_pfn_qglDrawElements = qglDrawElements;
		pQglTable->m_pfn_qglEnable = qglEnable;
		pQglTable->m_pfn_qglEnableClientState = qglEnableClientState;
		pQglTable->m_pfn_qglEnd = qglEnd;
		pQglTable->m_pfn_qglEndList = qglEndList;
		pQglTable->m_pfn_qglFogf = qglFogf;
		pQglTable->m_pfn_qglFogfv = qglFogfv;
		pQglTable->m_pfn_qglFogi = qglFogi;
		pQglTable->m_pfn_qglGenLists = qglGenLists;
		pQglTable->m_pfn_qglGenTextures = qglGenTextures;
		pQglTable->m_pfn_qglGetDoublev = qglGetDoublev;
		pQglTable->m_pfn_qglGetIntegerv = qglGetIntegerv;
		pQglTable->m_pfn_qglHint = qglHint;
		pQglTable->m_pfn_qglLightfv = qglLightfv;
		pQglTable->m_pfn_qglLineStipple = qglLineStipple;
		pQglTable->m_pfn_qglLineWidth = qglLineWidth;
		pQglTable->m_pfn_qglListBase = qglListBase;
		pQglTable->m_pfn_qglLoadIdentity = qglLoadIdentity;
		pQglTable->m_pfn_qglMaterialf = qglMaterialf;
		pQglTable->m_pfn_qglMaterialfv = qglMaterialfv;
		pQglTable->m_pfn_qglMatrixMode = qglMatrixMode;
		pQglTable->m_pfn_qglMultMatrixf = qglMultMatrixf;
		pQglTable->m_pfn_qglNewList = qglNewList;
		pQglTable->m_pfn_qglNormal3f = qglNormal3f;
		pQglTable->m_pfn_qglNormal3fv = qglNormal3fv;
		pQglTable->m_pfn_qglNormalPointer = qglNormalPointer;
		pQglTable->m_pfn_qglOrtho = qglOrtho;
		pQglTable->m_pfn_qglPointSize = qglPointSize;
		pQglTable->m_pfn_qglPolygonMode = qglPolygonMode;
		pQglTable->m_pfn_qglPopAttrib = qglPopAttrib;
		pQglTable->m_pfn_qglPopMatrix = qglPopMatrix;
		pQglTable->m_pfn_qglPushAttrib = qglPushAttrib;
		pQglTable->m_pfn_qglPushMatrix = qglPushMatrix;
		pQglTable->m_pfn_qglRasterPos3fv = qglRasterPos3fv;
		pQglTable->m_pfn_qglRotated = qglRotated;
		pQglTable->m_pfn_qglRotatef = qglRotatef;
		pQglTable->m_pfn_qglScalef = qglScalef;
		pQglTable->m_pfn_qglScissor = qglScissor;
		pQglTable->m_pfn_qglShadeModel = qglShadeModel;
		pQglTable->m_pfn_qglTexCoord2f = qglTexCoord2f;
		pQglTable->m_pfn_qglTexCoord2fv = qglTexCoord2fv;
		pQglTable->m_pfn_qglTexCoordPointer = qglTexCoordPointer;
		pQglTable->m_pfn_qglTexEnvf = qglTexEnvf;
		pQglTable->m_pfn_qglTexGenf = qglTexGenf;
		pQglTable->m_pfn_qglTexImage1D = qglTexImage1D;
		pQglTable->m_pfn_qglTexImage2D = qglTexImage2D;
		pQglTable->m_pfn_qglTexParameterf = qglTexParameterf;
		pQglTable->m_pfn_qglTexParameterfv = qglTexParameterfv;
		pQglTable->m_pfn_qglTexParameteri = qglTexParameteri;
		pQglTable->m_pfn_qglTexParameteriv = qglTexParameteriv;
		pQglTable->m_pfn_qglTexSubImage1D = qglTexSubImage1D;
		pQglTable->m_pfn_qglTexSubImage2D = qglTexSubImage2D;
		pQglTable->m_pfn_qglTranslated = qglTranslated;
		pQglTable->m_pfn_qglTranslatef = qglTranslatef;
		pQglTable->m_pfn_qglVertex2f = qglVertex2f;
		pQglTable->m_pfn_qglVertex3f = qglVertex3f;
		pQglTable->m_pfn_qglVertex3fv = qglVertex3fv;
		pQglTable->m_pfn_qglVertexPointer = qglVertexPointer;
		pQglTable->m_pfn_qglViewport = qglViewport;

		pQglTable->m_pfn_QE_CheckOpenGLForErrors = &QE_CheckOpenGLForErrors;

		pQglTable->m_pfn_qgluPerspective = qgluPerspective;
		pQglTable->m_pfn_qgluLookAt = qgluLookAt;
		pQglTable->m_pfnHookGL2DWindow = QERApp_HookGL2DWindow;
		pQglTable->m_pfnUnHookGL2DWindow = QERApp_UnHookGL2DWindow;
		pQglTable->m_pfnHookGL3DWindow = QERApp_HookGL3DWindow;
		pQglTable->m_pfnUnHookGL3DWindow = QERApp_UnHookGL3DWindow;

		return true;
	}
	if ( !strcmp( pAPI->major_name, DATA_MAJOR ) ) {
		_QERAppDataTable *pDataTable = static_cast<_QERAppDataTable *>( pAPI->mpTable );
		pDataTable->m_pfnActiveBrushes = QERApp_ActiveBrushes;
		pDataTable->m_pfnSelectedBrushes = QERApp_SelectedBrushes;
		pDataTable->m_pfnFilteredBrushes = QERApp_FilteredBrushes;
		pDataTable->m_pfnLstSkinCache = QERApp_LstSkinCache;

		return true;
	}
	if ( !strcmp( pAPI->major_name, PATCH_MAJOR ) ) {
		_QERPatchTable *pPatchTable = static_cast<_QERPatchTable *>( pAPI->mpTable );
		pPatchTable->m_pfnPatch_Alloc = &Patch_Alloc;
		pPatchTable->m_pfnAddBrushForPatch = &AddBrushForPatch;
		pPatchTable->m_pfnMakeNewPatch = &MakeNewPatch;

		return true;
	}
	if ( !strcmp( pAPI->major_name, ECLASSMANAGER_MAJOR ) ) {
		_EClassManagerTable *pEClassManTable = static_cast<_EClassManagerTable *>( pAPI->mpTable );

		pEClassManTable->m_pfnEclass_InsertAlphabetized = &Eclass_InsertAlphabetized;
		pEClassManTable->m_pfnGet_Eclass_E = &Get_EClass_E;
		pEClassManTable->m_pfnSet_Eclass_Found = &Set_Eclass_Found;
		pEClassManTable->m_pfnGet_Parsing_Single = &Get_Parsing_Single;
		pEClassManTable->m_pfnEClass_Create = &EClass_Create;
		pEClassManTable->m_pfnEclass_ForName = &Eclass_ForName;

		return true;
	}
	if ( !strcmp( pAPI->major_name, SELECTEDFACE_MAJOR ) ) {
		_QERSelectedFaceTable *pSelectedFaceTable = static_cast<_QERSelectedFaceTable *>( pAPI->mpTable );

		pSelectedFaceTable->m_pfnGetSelectedFaceCount = &QERApp_GetSelectedFaceCount;
		pSelectedFaceTable->m_pfnGetFaceBrush = &QERApp_GetSelectedFaceBrush;
		pSelectedFaceTable->m_pfnGetFace = &QERApp_GetSelectedFace;
		pSelectedFaceTable->m_pfnGetFaceInfo = &QERApp_GetFaceInfo;
		pSelectedFaceTable->m_pfnSetFaceInfo = &QERApp_SetFaceInfo;
		pSelectedFaceTable->m_pfnGetTextureNumber = &QERApp_ISelectedFace_GetTextureNumber;
		pSelectedFaceTable->m_pfnGetTextureSize = &QERApp_GetTextureSize;
		pSelectedFaceTable->m_pfnSelect_SetTexture = &Select_SetTexture;
		return true;
	}
	if ( !strcmp( pAPI->major_name, APPSURFACEDIALOG_MAJOR ) ) {
		_QERAppSurfaceTable *pSurfDialogTable = static_cast<_QERAppSurfaceTable *>( pAPI->mpTable );
		pSurfDialogTable->m_pfnOnlyPatchesSelected = &OnlyPatchesSelected;
		pSurfDialogTable->m_pfnAnyPatchesSelected = &AnyPatchesSelected;
		pSurfDialogTable->m_pfnGetSelectedPatch = &QERApp_GetSelectedPatch;
		pSurfDialogTable->m_pfnGetTwoSelectedPatch = &QERApp_GetTwoSelectedPatch;
		pSurfDialogTable->m_pfnTexMatToFakeTexCoords = &TexMatToFakeTexCoords;
		pSurfDialogTable->m_pfnConvertTexMatWithQTexture = &ConvertTexMatWithQTexture;
		pSurfDialogTable->m_pfnFakeTexCoordsToTexMat = &FakeTexCoordsToTexMat;
		pSurfDialogTable->m_pfnPatch_ResetTexturing = &Patch_ResetTexturing;
		pSurfDialogTable->m_pfnPatch_FitTexturing = &Patch_FitTexturing;
		pSurfDialogTable->m_pfnPatch_NaturalizeSelected = &Patch_NaturalizeSelected;
		pSurfDialogTable->m_pfnPatch_GetTextureName = &Patch_GetTextureName;
		pSurfDialogTable->m_pfnQE_SingleBrush = &QE_SingleBrush;
		pSurfDialogTable->m_pfnIsBrushPrimitMode = &IsBrushPrimitMode;
		pSurfDialogTable->m_pfnComputeAxisBase = &ComputeAxisBase;
		pSurfDialogTable->m_pfnBPMatMul = &BPMatMul;
		pSurfDialogTable->m_pfnEmitBrushPrimitTextureCoordinates = &EmitBrushPrimitTextureCoordinates;
		pSurfDialogTable->m_pfnQeglobalsTexturewin = &QERApp_QeglobalsTexturewin;
		pSurfDialogTable->m_pfnSelect_FitTexture = &Select_FitTexture;
		pSurfDialogTable->m_pfnQERApp_QeglobalsSavedinfo_SIInc = &QERApp_QeglobalsSavedinfo_SIInc;
		pSurfDialogTable->m_pfnQeglobalsGetGridSize = &QERApp_QeglobalsGetGridSize;
		pSurfDialogTable->m_pfnFaceList_FitTexture = &SI_FaceList_FitTexture;
		pSurfDialogTable->m_pfnGetMainWindow = &SI_GetMainWindow;
		pSurfDialogTable->m_pfnSetWinPos_From_Prefs = &SI_SetWinPos_from_Prefs;
		pSurfDialogTable->m_pfnGetSelectedFaceCountfromBrushes = &SI_GetSelectedFaceCountfromBrushes;
		pSurfDialogTable->m_pfnGetSelFacesTexdef = &SI_GetSelFacesTexdef;
		pSurfDialogTable->m_pfnSetTexdef_FaceList = &SI_SetTexdef_FaceList;

		return true;
	}
	if ( !strcmp( pAPI->major_name, UNDO_MAJOR ) ) {
		_QERUndoTable *pUndoTable = static_cast<_QERUndoTable *>( pAPI->mpTable );

		pUndoTable->m_pfnUndo_Start = &Undo_Start;
		pUndoTable->m_pfnUndo_End = &Undo_End;
		pUndoTable->m_pfnUndo_AddBrush = &Undo_AddBrush;
		pUndoTable->m_pfnUndo_EndBrush = &Undo_EndBrush;
		pUndoTable->m_pfnUndo_AddBrushList = &Undo_AddBrushList;
		pUndoTable->m_pfnUndo_EndBrushList = &Undo_EndBrushList;
		pUndoTable->m_pfnUndo_AddEntity = &Undo_AddEntity;
		pUndoTable->m_pfnUndo_EndEntity = &Undo_EndEntity;
		pUndoTable->m_pfnUndo_Undo = &Undo_Undo;                 // Nurail
		pUndoTable->m_pfnUndo_Redo = &Undo_Redo;                 // Nurail
		pUndoTable->m_pfnUndo_GetUndoId = &Undo_GetUndoId;       // Nurail
		pUndoTable->m_pfnUndo_UndoAvailable = &Undo_UndoAvailable; // Nurail
		pUndoTable->m_pfnUndo_RedoAvailable = &Undo_RedoAvailable; // Nurail

		return true;
	}
	if ( !strcmp( pAPI->major_name, CAMERA_MAJOR ) ) {
		_QERCameraTable *pCameraTable = static_cast<_QERCameraTable *>( pAPI->mpTable );

		pCameraTable->m_pfnGetCamera = &QERApp_GetCamera;
		pCameraTable->m_pfnSetCamera = &QERApp_SetCamera;
		pCameraTable->m_pfnGetCamWindowExtents = &QERApp_GetCamWindowExtents;

		return true;
	}
	if ( !strcmp( pAPI->major_name, UI_MAJOR ) ) {
		_QERUITable *pUITable = static_cast<_QERUITable *>( pAPI->mpTable );

		pUITable->m_pfnCreateGLWindow = QERApp_CreateGLWindow;
		pUITable->m_pfnHookWindow = QERApp_HookWindow;
		pUITable->m_pfnUnHookWindow = QERApp_UnHookWindow;
		pUITable->m_pfnGetXYWndWrapper = QERApp_GetXYWndWrapper;
		pUITable->m_pfnHookListener = QERApp_HookListener;
		pUITable->m_pfnUnHookListener = QERApp_UnHookListener;

		return true;
	}
	if ( !strcmp( pAPI->major_name, UIGTK_MAJOR ) ) {
		_QERUIGtkTable *pUIGtkTable = static_cast<_QERUIGtkTable *>( pAPI->mpTable );

		pUIGtkTable->m_pfn_GetQeglobalsGLWidget = &QERApp_GetQeGlobalsGLWidget;
		pUIGtkTable->m_pfn_glwidget_new = &gtk_glwidget_new;
		pUIGtkTable->m_pfn_glwidget_swap_buffers = &gtk_glwidget_swap_buffers;
		pUIGtkTable->m_pfn_glwidget_make_current = &gtk_glwidget_make_current;
		pUIGtkTable->m_pfn_glwidget_destroy_context = &gtk_glwidget_destroy_context;
		pUIGtkTable->m_pfn_glwidget_create_context = &gtk_glwidget_create_context;
#if 0
		pUIGtkTable->m_pfn_glwidget_get_context = &gtk_glwidget_get_context;
#endif

		return true;
	}

	return false;
}

const char* CSynapseClientRadiant::GetInfo(){
	return "Radiant - synapse core built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientRadiant::GetName(){
	return "core";
}
