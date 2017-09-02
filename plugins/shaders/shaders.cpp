/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Shaders Manager Plugin
//
// Leonardo Zide (leo@lokigames.com)
//

// standard headers
#include <stdio.h>
#include <stdlib.h>
#include "plugin.h"
#include "mathlib.h"
#include "missing.h" //++timo FIXME: this one is intended to go away some day, it's MFC compatibility classes
#include "shaders.h"

// some forward declarations
IShader * WINAPI QERApp_Shader_ForName( const char *name );
qtexture_t *WINAPI QERApp_Try_Texture_ForName( const char *name );
qtexture_t *WINAPI QERApp_Texture_ForName2( const char *filename );
IShader *WINAPI QERApp_ColorShader_ForName( const char *name );
void WINAPI QERApp_LoadShaderFile( const char *filename );

//++timo TODO: use stl::map !! (I tried having a look to CMap but it obviously sucks)
CShaderArray g_Shaders;
// whenever a shader gets activated / deactivated this list is updated
// NOTE: make sure you don't add a shader that's already in
// NOTE: all shaders in this array are in the main g_Shaders
CShaderArray g_ActiveShaders;

// clean a texture name to the qtexture_t name format we use internally
// NOTE: there are so many cases .. this may need to get updated to cover all of them
// we expect a "textures/" path on top, except if bAddTexture is set to true .. in case we add in needed
// NOTE: case sensitivity: the engine is case sensitive. we store the shader name with case information and save with case
// information as well. but we assume there won't be any case conflict and so when doing lookups based on shader name,
// we compare as case insensitive. That is Radiant is case insensitive, but knows that the engine is case sensitive.
//++timo FIXME: we need to put code somewhere to detect when two shaders that are case insensitive equal are present
const char *WINAPI QERApp_CleanTextureName( const char *name, bool bAddTexture = false ){
	static char stdName[QER_MAX_NAMELEN];
#ifdef _DEBUG
	if ( strlen( name ) > QER_MAX_NAMELEN ) {
		g_FuncTable.m_pfnSysFPrintf( SYS_WRN, "WARNING: name exceeds QER_MAX_NAMELEN in CleanTextureName\n" );
	}
#endif

	strcpy( stdName, name );
	g_FuncTable.m_pfnQE_ConvertDOSToUnixName( stdName, stdName );
	if ( stdName[strlen( name ) - 4] == '.' ) {
		// strip extension
		stdName[strlen( stdName ) - 4] = '\0';
	}

	if ( bAddTexture ) {
		char aux[QER_MAX_NAMELEN];
		sprintf( aux, "textures/%s", stdName );
		strcpy( stdName, aux );
	}
	return stdName;
}

int WINAPI QERApp_GetActiveShaderCount(){
	return g_ActiveShaders.GetSize();
}

IShader *WINAPI QERApp_ActiveShader_ForIndex( int i ){
	return static_cast < CShader * >( g_ActiveShaders.GetAt( i ) );
}

void CShaderArray::SortShaders(){
	CPtrArray aux;
	int i, icount;
	int j, jcount;
	CShader *pSort;
	const char *sSort;
	// dumb sort .. would it ever grow big enough so we would have to do something clever? noooo
	icount = CPtrArray::GetSize();
	for ( i = 0; i < icount; i++ )
	{
		pSort = static_cast < CShader * >( GetAt( i ) );
		sSort = pSort->getName();
		jcount = aux.GetSize();
		for ( j = 0; j < jcount; j++ )
		{
			if ( strcmp( sSort, static_cast < CShader * >( aux.GetAt( j ) )->getName() ) < 0 ) {
				break;
			}
		}
		aux.InsertAt( j, pSort );
	}
	CPtrArray::RemoveAll();
	CPtrArray::InsertAt( 0, &aux );
}

// will sort the active shaders list by name
// NOTE: it would be easier if the thing would stay sorted by using a map<name,CShader> thing
//++timo FIXME: would need to export that to allow external override?
void WINAPI QERApp_SortActiveShaders(){
	g_ActiveShaders.SortShaders();
}

// NOTE: case sensitivity
// although we store shader names with case information, Radiant does case insensitive searches
// (we assume there's no case conflict with the names)
CShader *CShaderArray::Shader_ForName( const char *name ) const {
	int i;
	for ( i = 0; i < CPtrArray::GetSize(); i++ )
	{
		CShader *pShader = static_cast < CShader * >( CPtrArray::GetAt( i ) );
		if ( stricmp( pShader->getName(), name ) == 0 ) {
			return pShader;
		}
	}
	return NULL;
}

void CShader::CreateDefault( const char *name ){
	const char *stdName = QERApp_CleanTextureName( name );
	m_strTextureName = stdName;
	setName( name );
}

CShader *CShaderArray::Shader_ForTextureName( const char *name ) const {
#ifdef _DEBUG
	// check we were given a texture name that fits the qtexture_t naming conventions
	if ( strcmp( name, QERApp_CleanTextureName( name ) ) != 0 ) {
		Sys_Printf
			( "WARNING: texture name %s doesn't fit qtexture_t conventions in CShaderArray::Shader_ForTextureName\n",
			name );
	}
#endif
	int i;
	for ( i = 0; i < CPtrArray::GetSize(); i++ )
	{
		CShader *pShader = static_cast < CShader * >( CPtrArray::GetAt( i ) );
		if ( strcmp( name, QERApp_CleanTextureName( pShader->getTextureName() ) ) == 0 ) {
			return pShader;
		}
	}
	return NULL;
}

IShader *WINAPI QERApp_ActiveShader_ForTextureName( char *name ){
	return g_ActiveShaders.Shader_ForTextureName( name );
}

void CShaderArray::AddSingle( void *lp ){
	int i;
	for ( i = 0; i < CPtrArray::GetSize(); i++ )
	{
		if ( CPtrArray::GetAt( i ) == lp ) {
			return;
		}
	}
	CPtrArray::Add( lp );
	static_cast < CShader * >( CPtrArray::GetAt( i ) )->IncRef();
}

void CShaderArray::operator =( const class CShaderArray & src ){
	int i;

#ifdef _DEBUG
	if ( CPtrArray::GetSize() != 0 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: CShaderArray::operator = expects an empty array\n" );
	}
#endif
	Copy( src );
	// now go through and IncRef
	for ( i = 0; i < CPtrArray::GetSize(); i++ )
		static_cast < IShader * >( CPtrArray::GetAt( i ) )->IncRef();
}

//++timo NOTE: for debugging we may need to keep track and tell wether everything has been properly unloaded
void CShaderArray::ReleaseAll(){
	int i;
	int count = CPtrArray::GetSize();
	// decref
	for ( i = 0; i < count; i++ )
		static_cast < IShader * >( CPtrArray::GetAt( i ) )->DecRef();
	// get rid
	CPtrArray::RemoveAll();
}

// NOTE TTimo:
// this was hacked to work a long time ago
// in Loki's fenris tracker as bug #104655
// since that info is no longer available, and the hack has been there for so long, it's part of the code now
// don't remember the details, but basically across a flush and reload for the shaders
// we have to keep track of the patches texture names in a seperate entry
// not sure why anymore, but I know that doesn't happen with brushes
typedef struct patchEntry_s
{
	char name[QER_MAX_NAMELEN];
	patchMesh_t *p;
} patchEntry_t;

CPtrArray PatchShaders;

void PushPatch( patchMesh_t * patch ){
	patchEntry_t *pEntry = new patchEntry_s;
	pEntry->p = patch;
	strcpy( pEntry->name, patch->pShader->getName() );
	PatchShaders.Add( pEntry );
}

char *ShaderNameLookup( patchMesh_t * patch ){
	int i;
	int count = PatchShaders.GetSize();
	for ( i = 0; i < count; i++ )
	{
		if ( static_cast < patchEntry_t * >( PatchShaders.GetAt( i ) )->p == patch ) {
			return static_cast < patchEntry_t * >( PatchShaders.GetAt( i ) )->name;
		}
	}
	Sys_FPrintf( SYS_ERR, "ERROR: failed to lookup name in ShaderNameLookup??\n" );
	return const_cast<char*>(SHADER_NOT_FOUND);
}
//++timo end clean

// will free all GL binded qtextures and shaders
// NOTE: doesn't make much sense out of Radiant exit or called during a reload
void WINAPI QERApp_FreeShaders(){
	int i;
	brush_t *b;
	brush_t *active_brushes;
	brush_t *selected_brushes;
	brush_t *filtered_brushes;
	qtexture_t **d_qtextures;

	active_brushes = g_DataTable.m_pfnActiveBrushes();
	selected_brushes = g_DataTable.m_pfnSelectedBrushes();
	filtered_brushes = g_DataTable.m_pfnFilteredBrushes();
	d_qtextures = g_ShadersTable.m_pfnQTextures();

	// store the shader names used by the patches
	for ( i = 0; i < PatchShaders.GetSize(); i++ )
		delete static_cast < patchMesh_t * >( PatchShaders.GetAt( i ) );
	PatchShaders.RemoveAll();

	for ( b = active_brushes->next; b != NULL && b != active_brushes; b = b->next )
	{
		if ( b->patchBrush ) {
			PushPatch( b->pPatch );
		}
	}
	for ( b = selected_brushes->next; b != NULL && b != selected_brushes; b = b->next )
	{
		if ( b->patchBrush ) {
			PushPatch( b->pPatch );
		}
	}
	for ( b = filtered_brushes->next; b != NULL && b != filtered_brushes; b = b->next )
	{
		if ( b->patchBrush ) {
			PushPatch( b->pPatch );
		}
	}

	// reload shaders
	// empty the actives shaders list
	g_ActiveShaders.ReleaseAll();
	g_Shaders.ReleaseAll();
	// empty the main g_qeglobals.d_qtextures list
	// FIXME: when we reload later on, we need to have the shader names
	// for brushes it's stored in the texdef
	// but patches don't have texdef
	// see bug 104655 for details
	// so the solution, build an array of patchMesh_t* and their shader names
#ifdef _DEBUG
	Sys_Printf( "FIXME: patch shader reload workaround (old fenris? bug 104655)\n" );
#endif

	//GtkWidget *widget = g_QglTable.m_pfn_GetQeglobalsGLWidget ();
	GHashTable *texmap = g_ShadersTable.m_pfnQTexmap();

	// NOTE: maybe before we'd like to set all qtexture_t in the shaders list to notex?
	// NOTE: maybe there are some qtexture_t we don't want to erase? For plain color faces maybe?
	while ( *d_qtextures )
	{
		qtexture_t *pTex = *d_qtextures;
		qtexture_t *pNextTex = pTex->next;

		//if (widget != NULL)
		g_QglTable.m_pfn_qglDeleteTextures( 1, &pTex->texture_number );

		g_hash_table_remove( texmap, pTex->name );

		// all qtexture_t should be manipulated with the glib alloc handlers for now
		g_free( pTex );
		*d_qtextures = pNextTex;
	}

	g_QglTable.m_pfn_QE_CheckOpenGLForErrors();
}

// those functions are only used during a shader reload phase
// the patch one relies on ShaderNameLookup, a table that is being built only when a flush is performed
// so it's not something we want to expose publicly

void SetShader( patchMesh_t * patch ){
	// unhook current shader
	patch->pShader->DecRef();
	// don't access this one! it has been deleted .. it's DEAD
	patch->d_texture = NULL;
	// hook the new one, increment the refcount
	// NOTE TTimo this function increments the refcount, don't incref ourselves
	patch->pShader = QERApp_Shader_ForName( ShaderNameLookup( patch ) );
	patch->d_texture = patch->pShader->getTexture();
}

void SetShader( face_t * f ){
	// unhook current shader
	f->pShader->DecRef();
	// don't access the texdef! it's DEAD
	f->d_texture = NULL;
	// hook
	// NOTE TTimo this function increments the refcount, don't incref ourselves
	f->pShader = QERApp_Shader_ForName( f->texdef.GetName() );
	f->d_texture = f->pShader->getTexture();
}

void Brush_RefreshShader( brush_t *b ){
	if ( b->patchBrush ) {
		SetShader( b->pPatch );
	}
	else if ( b->owner->eclass->fixedsize ) {
		/*eclass_t *eclass = HasModel(b);
		   if (eclass)
		   {
		   for(entitymodel *model = eclass->model; model!=NULL; model=model->pNext)
		    if(model && model->strSkin)
		      model->nTextureBind = g_FuncTable.m_pfnTexture_LoadSkin(((GString *)model->strSkin)->str, &model->nSkinWidth, &model->nSkinHeight);
		   }*/
	}
	else{
		for ( face_t *f = b->brush_faces ; f ; f = f->next )
			SetShader( f );
	}
}

void WINAPI QERApp_ReloadShaders(){
	brush_t *b;
	brush_t *active_brushes;
	brush_t *selected_brushes;
	brush_t *filtered_brushes;

	QERApp_FreeShaders();

	g_DataTable.m_pfnLstSkinCache()->RemoveAll(); //md3 skins

	active_brushes = g_DataTable.m_pfnActiveBrushes();
	selected_brushes = g_DataTable.m_pfnSelectedBrushes();
	filtered_brushes = g_DataTable.m_pfnFilteredBrushes();

	// now we must reload the shader information from shaderfiles
	g_ShadersTable.m_pfnBuildShaderList();
	g_ShadersTable.m_pfnPreloadShaders();

	// refresh the map visuals: replace our old shader objects by the new ones
	// on brush faces we have the shader name in texdef.name
	// on patches we have the shader name in PatchShaders
	// while we walk through the map data, we DecRef the old shaders and push the new ones in
	// if all goes well, most of our old shaders will get deleted on the way

	// FIXME: bug 104655, when we come accross a patch, we use the above array since the d_texture is lost
	// NOTE: both face_t and patchMesh_t store pointers to the shader and qtexture_t
	// in an ideal world they would only store shader and access the qtexture_t through it
	// reassign all current shaders
	for ( b = active_brushes->next; b != NULL && b != active_brushes; b = b->next )
		Brush_RefreshShader( b );
	for ( b = selected_brushes->next; b != NULL && b != selected_brushes; b = b->next )
		Brush_RefreshShader( b );
	// do that to the filtered brushes as well (we might have some region compiling going on)
	for ( b = filtered_brushes->next; b != NULL && b != filtered_brushes; b = b->next )
		Brush_RefreshShader( b );
}

int WINAPI QERApp_LoadShadersFromDir( const char *path ){
	int count = 0;

	// some code adds a trailing slash
	gchar* keyword = g_strdup(path);
	if ( g_str_has_suffix( keyword, "/" ) ) {
		keyword[ strlen(keyword) -1 ] = '\0';
	}

	gchar* around = g_strconcat("/", keyword, ".", NULL);
	gchar* prefix = g_strconcat("textures/", keyword, "/", NULL);

	// scan g_Shaders, and call QERApp_Shader_ForName for each in the given path
	// this will load the texture if needed and will set it in use..
	int nSize = g_Shaders.GetSize();

	for ( int i = 0; i < nSize; i++ )
	{
		CShader *pShader = reinterpret_cast < CShader * >( g_Shaders[i] );

		// does not uselessly load shader with path not starting with "textures/"
		// they will not be displayed by texture browser, because they can't be
		// applied to a surface
		if ( !g_str_has_prefix( pShader->getName(), "textures/" ) ) {
			continue;
		}

		// - proceed if shader script base name is <path>
		// for example: "scripts/eerie.shader" with "eerie" path
		// - proceed if shader script base name is <path> and shader path starts with "textures/<path>"
		// for example: "scripts/eerie.shader" providing "textures/eerie/blackness" with "eerie" path
		if ( strstr( pShader->getShaderFileName(), around ) != NULL || g_str_has_prefix( pShader->getName(), prefix ) ) {
			count++;
#ifdef _DEBUG
			// request the shader, this will load the texture if needed and set "inuse"
			//++timo FIXME: should we put an Activate member on CShader?
			// this QERApp_Shader_ForName call is a kind of hack
			IShader *pFoo = QERApp_Shader_ForName( pShader->getName() );
			// check we activated the right shader
			// NOTE: if there was something else loaded, the size of g_Shaders may have changed and strange behaviours are to be expected
			if ( pFoo != pShader ) {
				Sys_FPrintf( SYS_WRN, "WARNING: unexpected pFoo != pShader in QERApp_LoadShadersFromDir\n" );
			}
#else
			QERApp_Shader_ForName( pShader->getName() );
#endif
		}
	}

	g_free(keyword);
	g_free(around);
	g_free(prefix);

	return count;
}

bool WINAPI QERApp_IsDirContainingShaders( const char *path ){
	int nSize = g_Shaders.GetSize();
	// exclude shaders that are not starting with "textures/"
	// they will not be displayed and are not applicable to surfaces
	// exclude shaders from other paths,
	// they are not the ones we are looking for

	gchar* around = g_strconcat("/", path, ".", NULL);
	gchar* prefix = g_strconcat("textures/", path, "/", NULL);

	for ( int i = 0; i < nSize; i++ )
	{
		CShader *pShader = reinterpret_cast < CShader * >( g_Shaders[i] );

		// - returns true if shader script basename is <path> and shader path starts with "textures/"
		// for example: "scripts/rockyvalley.shader" with "rockyvalley" path providing "textures/amethyst7/rockyvalley/rockyvalley_skybox/"
		// - returns true if shader <path> startswith "textures/<path>"
		// for example: "scripts/eerie.shader" with "eerie" path providing "textures/eerie/blackness"
		if ( ( strstr( pShader->getShaderFileName(), around ) != NULL && g_str_has_prefix( pShader->getName(), "textures/" ) ) || g_str_has_prefix( pShader->getName(), prefix ) ) {
			g_free(around);
			g_free(prefix);
			return true;
		}
	}

	g_free(around);
	g_free(prefix);
	return false;
}

bool CShader::Parse(){
	char *token = g_ScripLibTable.m_pfnToken();

	// the parsing needs to be taken out in another module
//  Sys_Printf("TODO: CShader::Parse\n");

	// token is shader name (full path with a "textures\")
	// we remove the "textures\" part
	//setName ((char *) &token[9]));
	// no we don't
	setName( token );
	// name of the qtexture_t we'll use to represent this shader (this one has the "textures\" before)
	const char *stdName = QERApp_CleanTextureName( token );
	m_strTextureName = stdName; // FIXME: BC reports stdName is uninitialised?
	g_ScripLibTable.m_pfnGetToken( true );
	if ( strcmp( token, "{" ) ) {
		return false;
	}
	else
	{
		// we need to read until we hit a balanced }
		int nMatch = 1;
		while ( nMatch > 0 && g_ScripLibTable.m_pfnGetToken( true ) )
		{
			if ( strcmp( token, "{" ) == 0 ) {
				nMatch++;
				continue;
			}
			else if ( strcmp( token, "}" ) == 0 ) {
				nMatch--;
				continue;
			}
			if ( nMatch > 1 ) {
				continue;       // ignore layers for now
			}
			if ( strcmpi( token, "qer_nocarve" ) == 0 ) {
				m_nFlags |= QER_NOCARVE;
			}
			else if ( strcmpi( token, "qer_trans" ) == 0 ) {
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {
					m_fTrans = (float) atof( token );
				}
				m_nFlags |= QER_TRANS;
			}
			else if ( strcmpi( token, "qer_editorimage" ) == 0 ) {
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {
					// bAddTexture changed to false to allow editorimages in other locations than "textures/"
					m_strTextureName = QERApp_CleanTextureName( token, false );
				}
			}
			else if ( strcmpi( token, "qer_alphafunc" ) == 0 ) {
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {

					if ( stricmp( token, "greater" ) == 0 ) {
						m_nAlphaFunc = GL_GREATER;
					}
					else if ( stricmp( token, "less" ) == 0 ) {
						m_nAlphaFunc = GL_LESS;
					}
					else if ( stricmp( token, "gequal" ) == 0 ) {
						m_nAlphaFunc = GL_GEQUAL;
					}

					if ( m_nAlphaFunc ) {
						m_nFlags |= QER_ALPHAFUNC;
					}
				}
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {
					m_fAlphaRef = (float) atof( token );
				}
			}
			else if ( strcmpi( token, "cull" ) == 0 ) {
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {
					if ( stricmp( token, "none" ) == 0 || stricmp( token, "twosided" ) == 0 || stricmp( token, "disable" ) == 0 ) {
						m_nCull = 2;
					}
					else if ( stricmp( token, "back" ) == 0 || stricmp( token, "backside" ) == 0 || stricmp( token, "backsided" ) == 0 ) {
						m_nCull = 1;
					}

					if ( m_nCull ) {
						m_nFlags |= QER_CULL;
					}
				}
			}
			else if ( strcmpi( token, "surfaceparm" ) == 0 ) {
				if ( g_ScripLibTable.m_pfnGetToken( true ) ) {
					if ( strcmpi( token, "fog" ) == 0 ) {
						m_nFlags |= QER_FOG;
						if ( m_fTrans == 1.0f ) { // has not been explicitly set by qer_trans
							m_fTrans = 0.35f;
						}
					}
					else if ( strcmpi( token, "nodraw" ) == 0 ) {
						m_nFlags |= QER_NODRAW;
					}
					else if ( strcmpi( token, "nonsolid" ) == 0 ) {
						m_nFlags |= QER_NONSOLID;
					}
					else if ( strcmpi( token, "water" ) == 0 ) {
						m_nFlags |= QER_WATER;
					}
					else if ( strcmpi( token, "lava" ) == 0 ) {
						m_nFlags |= QER_LAVA;
					}
				}
			}
		}
		if ( nMatch != 0 ) {
			return false;
		}
	}
	return true;
}

void CShader::RegisterActivate(){
	// fill the qtexture_t with shader information
	//++timo FIXME: a lot of that won't be necessary, will be stored at IShader* level
//  strcpy (m_pTexture->shadername, m_Name);
	// this flag is set only if we have a shaderfile name
//  if (m_ShaderFileName[0] != '\0')
//    m_pTexture->bFromShader = true;
//  else
//    m_pTexture->bFromShader = false;
	//++timo FIXME: what do we do with that?
	//m_pTexture->fTrans = pInfo->m_fTransValue;
//  m_pTexture->fTrans = 1.0f;	// if != 1.0 it's ot getting drawn in Cam_Draw
//  m_pTexture->nShaderFlags = m_nFlags;
	// store in the active shaders list (if necessary)
	g_ActiveShaders.AddSingle( this );
	// when you activate a shader, it gets displayed in the texture browser
	m_bDisplayed = true;
	IncRef();
}

void CShader::Try_Activate(){
	m_pTexture = QERApp_Try_Texture_ForName( m_strTextureName.GetBuffer() );
	if ( m_pTexture ) {
		RegisterActivate();
	}
}

// Hydra: now returns false if the ORIGINAL shader could not be activated
// (missing texture, or incorrect shader script), true otherwise
// the shader is still activated in all cases.
bool CShader::Activate(){
	Try_Activate();
	if ( !m_pTexture ) {
		m_pTexture = QERApp_Texture_ForName2( SHADER_NOTEX );
		RegisterActivate();
		return false;
	}
	return true;
}

void WINAPI QERApp_LoadShaderFile( const char *filename ){
	char *pBuff;
	int nSize = vfsLoadFile( filename, reinterpret_cast < void **>( &pBuff ), 0 );
	if ( nSize > 0 ) {
		Sys_Printf( "Parsing shaderfile %s\n", filename );
		g_ScripLibTable.m_pfnStartTokenParsing( pBuff );
		while ( g_ScripLibTable.m_pfnGetToken( true ) )
		{
			// first token should be the path + name.. (from base)
			CShader *pShader = new CShader();
			// we want the relative filename only, it's easier for later lookup .. see QERApp_ReloadShaderFile
			char cTmp[1024];
			g_FuncTable.m_pfnQE_ConvertDOSToUnixName( cTmp, filename );
			// given the vfs, we should not store the full path
			//pShader->setShaderFileName( filename + strlen(ValueForKey(g_qeglobals.d_project_entity, "basepath")));
			pShader->setShaderFileName( filename );
			if ( pShader->Parse() ) {
				// do we already have this shader?
				//++timo NOTE: this may a bit slow, we may need to use a map instead of a dumb list
				if ( g_Shaders.Shader_ForName( pShader->getName() ) != NULL ) {
#ifdef _DEBUG
					Sys_FPrintf( SYS_WRN, "WARNING: shader %s is already in memory, definition in %s ignored.\n",
								pShader->getName(), filename );
#endif
					delete pShader;
				}
				else
				{
					pShader->IncRef();

					g_Shaders.Add( (void *) pShader );
				}
			}
			else
			{
				Sys_FPrintf( SYS_ERR, "ERROR: parsing shader %s\n", pShader->getName() );
				delete pShader;
			}
		}
		vfsFreeFile( pBuff );
	}
	else
	{
		Sys_FPrintf( SYS_ERR, "ERROR: Unable to read shaderfile %s\n", filename );
	}
}

IShader *WINAPI QERApp_Try_Shader_ForName( const char *name ){
	// look for the shader
	CShader *pShader = g_Shaders.Shader_ForName( name );
	if ( !pShader ) {
		// not found
		return NULL;
	}
	// we may need to load the texture or use the "shader without texture" one
	pShader->Activate();
	pShader->SetDisplayed( true );
	return pShader;
}

IShader *WINAPI QERApp_CreateShader_ForTextureName( const char *name ){
	CShader *pShader;
	pShader = new CShader;
	// CreateDefault expects a texture / shader name relative to the "textures" directory
	// (cause shader names are reletive to "textures/")
	pShader->CreateDefault( name );
	// hook it into the shader list
	g_Shaders.Add( (void *) pShader );
	pShader->IncRef();
	// if it can't find the texture, SHADER_NOT_FOUND will be used
	// Hydra: display an error message, so the user can quickly find a list of missing
	// textures by looking at the console.
	if ( !pShader->Activate() ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Activate shader failed for %s\n", pShader->getName() );
	}
	pShader->SetDisplayed( true );

	return pShader;
}

IShader *WINAPI QERApp_Shader_ForName( const char *name ){
	if ( name == NULL || strlen( name ) == 0 ) {
		// Hydra: This error can occur if the user loaded a map with/dropped an entity that
		// did not set a texture name "(r g b)" - check the entity definition loader

		g_FuncTable.m_pfnSysFPrintf( SYS_ERR, "FIXME: name == NULL || strlen(name) == 0 in QERApp_Shader_ForName\n" );
		return QERApp_Shader_ForName( SHADER_NOT_FOUND );
	}
	// entities that should be represented with plain colors instead of textures
	// request a texture name with (r g b) (it's stored in their class_t)
	if ( name[0] == '(' ) {
		return QERApp_ColorShader_ForName( name );
	}

	CShader *pShader = static_cast < CShader * >( QERApp_Try_Shader_ForName( name ) );
	if ( pShader ) {
		pShader->SetDisplayed( true );
		return pShader;
	}
	return QERApp_CreateShader_ForTextureName( name );
}

qtexture_t *WINAPI QERApp_Try_Texture_ForName( const char *name ){
	qtexture_t *q;
//  char f1[1024], f2[1024];
	unsigned char *pPixels = NULL;
	int nWidth, nHeight;

	// convert the texture name to the standard format we use in qtexture_t
	const char *stdName = QERApp_CleanTextureName( name );

	// use the hash table
	q = (qtexture_t*)g_hash_table_lookup( g_ShadersTable.m_pfnQTexmap(), stdName );
	if ( q ) {
		return q;
	}

#ifdef QTEXMAP_DEBUG
	for ( q = g_qeglobals.d_qtextures; q; q = q->next )
	{
		if ( !strcmp( stdName, q->name ) ) {
			Sys_FPrintf( SYS_ERR, "ERROR: %s is not in texture map, but was found in texture list\n" );
			return q;
		}
	}
#endif

	g_FuncTable.m_pfnLoadImage( name, &pPixels, &nWidth, &nHeight );

	if ( !pPixels ) {
		return NULL; // we failed
	}
	else{
		Sys_Printf( "LOADED: %s\n", name );
	}

	// instanciate a new qtexture_t
	// NOTE: when called by a plugin we must make sure we have set Radiant's GL context before binding the texture

	// we'll be binding the GL texture now
	// need to check we are using a right GL context
	// with GL plugins that have their own window, the GL context may be the plugin's, in which case loading textures will bug
	//  g_QglTable.m_pfn_glwidget_make_current (g_QglTable.m_pfn_GetQeglobalsGLWidget ());
	q = g_FuncTable.m_pfnLoadTextureRGBA( pPixels, nWidth, nHeight );
	if ( !q ) {
		return NULL;
	}
	g_free( pPixels );

	strcpy( q->name, name );
	// only strip extension if extension there is!
	if ( q->name[strlen( q->name ) - 4] == '.' ) {
		q->name[strlen( q->name ) - 4] = '\0';
	}
	// hook into the main qtexture_t list
	qtexture_t **d_qtextures = g_ShadersTable.m_pfnQTextures();
	q->next = *d_qtextures;
	*d_qtextures = q;
	// push it in the map
	g_hash_table_insert( g_ShadersTable.m_pfnQTexmap(), q->name, q );
	return q;
}

int WINAPI QERApp_HasShader( const char *pName ){
	//  mickey check the global shader array for existense of pName
	CShader *pShader = g_Shaders.Shader_ForName( pName );
	if ( pShader ) {
		return 1;
	}
	return 0;
}

IShader *WINAPI QERApp_Shader_ForName_NoLoad( const char *pName ){
	CShader *pShader = g_Shaders.Shader_ForName( pName );
	return pShader;
}

/*!
   This should NEVER return NULL, it is the last-chance call in the load cascade
 */
qtexture_t *WINAPI QERApp_Texture_ForName2( const char *filename ){
	qtexture_t *q;
	q = QERApp_Try_Texture_ForName( filename );
	if ( q ) {
		return q;
	}
	// not found? use "texture not found"
	q = QERApp_Try_Texture_ForName( SHADER_NOT_FOUND );
	if ( q ) {
		return q;
	}

	// still not found? this is a fatal error
	g_FuncTable.m_pfnError( "Failed to load " SHADER_NOT_FOUND ". Looks like your installation is broken / missing some essential elements." );
	return NULL;
}

void CShader::CreateColor( const char *name ){
	// parse
	sscanf( name, "(%g %g %g)", m_vColor, m_vColor + 1, m_vColor + 2 );
	m_strTextureName = name;
	setName( "color" );
	// create the qtexture_t
	qtexture_t *q1 = QERApp_Texture_ForName2( SHADER_NOT_FOUND );
	// copy this one
	qtexture_t *q2 = new qtexture_t;
	memcpy( q2, q1, sizeof( qtexture_t ) );
	strcpy( q2->name, m_strTextureName.GetBuffer() );
	VectorCopy( m_vColor, q2->color );
	m_pTexture = q2;
}

IShader *WINAPI QERApp_ColorShader_ForName( const char *name ){
	CShader *pShader = new CShader();
	pShader->CreateColor( name );
	// hook it into the shader list
	pShader->IncRef();
	g_Shaders.Add( (void *) pShader );
	return pShader;
}

void CShaderArray::ReleaseForShaderFile( const char *name ){
	int i;
	// decref
	for ( i = 0; i < CPtrArray::GetSize(); i++ )
	{
		IShader *pShader = static_cast < IShader * >( CPtrArray::GetAt( i ) );
		if ( !strcmp( name, pShader->getShaderFileName() ) ) {
			pShader->DecRef();
			CPtrArray::RemoveAt( i );
			i--;    // get ready for next loop
		}
	}
}

void WINAPI QERApp_ReloadShaderFile( const char *name ){
	brush_t *b;
	face_t *f;
	brush_t *active_brushes;
	brush_t *selected_brushes;
	brush_t *filtered_brushes;

//  Sys_Printf("TODO: QERApp_ReloadShaderFile\n");

	active_brushes = g_DataTable.m_pfnActiveBrushes();
	selected_brushes = g_DataTable.m_pfnSelectedBrushes();
	filtered_brushes = g_DataTable.m_pfnFilteredBrushes();

#ifdef _DEBUG
	// check the shader name is a reletive path
	// I hacked together a few quick tests to make sure :-)
	if ( strstr( name, ":\\" ) || !strstr( name, "scripts" ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: is %s a reletive path to a shader file? (QERApp_ReloadShaderFile\n" );
	}
#endif

	// in the actives and global shaders lists, decref and unhook the shaders
	//++timo NOTE: maybe we'd like to keep track of the shaders we are unhooking?
	g_ActiveShaders.ReleaseForShaderFile( name );
	g_Shaders.ReleaseForShaderFile( name );
	// go through a reload of the shader file
	QERApp_LoadShaderFile( name );
	// scan all the brushes, replace all the old ones by refs to their new equivalents
	for ( b = active_brushes->next; b != NULL && b != active_brushes; b = b->next )
	{
		if ( b->patchBrush && !strcmp( b->pPatch->pShader->getShaderFileName(), name ) ) {
			SetShader( b->pPatch );
		}
		else{
			for ( f = b->brush_faces; f; f = f->next )
				if ( !strcmp( f->pShader->getShaderFileName(), name ) ) {
					SetShader( f );
				}
		}
	}
	for ( b = selected_brushes->next; b != NULL && b != selected_brushes; b = b->next )
	{
		if ( b->patchBrush && !strcmp( b->pPatch->pShader->getShaderFileName(), name ) ) {
			SetShader( b->pPatch );
		}
		else{
			for ( f = b->brush_faces; f; f = f->next )
				if ( !strcmp( f->pShader->getShaderFileName(), name ) ) {
					SetShader( f );
				}
		}
	}
	// do that to the filtered brushes as well (we might have some region compiling going on)
	for ( b = filtered_brushes->next; b != NULL && b != filtered_brushes; b = b->next )
	{
		if ( b->patchBrush && !strcmp( b->pPatch->pShader->getShaderFileName(), name ) ) {
			SetShader( b->pPatch );
		}
		else{
			for ( f = b->brush_faces; f; f = f->next )
				if ( !strcmp( f->pShader->getShaderFileName(), name ) ) {
					SetShader( f );
				}
		}
	}
	// call Texture_ShowInUse to clean and display only what's required
	g_ShadersTable.m_pfnTexture_ShowInuse();
	QERApp_SortActiveShaders();
	g_FuncTable.m_pfnSysUpdateWindows( W_TEXTURE );
}

void CShaderArray::SetDisplayed( bool b ){
	int i, count;
	count = CPtrArray::GetSize();
	for ( i = 0; i < count; i++ )
		static_cast < IShader * >( CPtrArray::GetAt( i ) )->SetDisplayed( b );
}

void CShaderArray::SetInUse( bool b ){
	int i, count;
	count = CPtrArray::GetSize();
	for ( i = 0; i < count; i++ )
		static_cast < IShader * >( CPtrArray::GetAt( i ) )->SetInUse( b );
}

// Set the IsDisplayed flag on all active shaders
void WINAPI QERApp_ActiveShaders_SetDisplayed( bool b ){
	g_ActiveShaders.SetDisplayed( b );
}

void WINAPI QERApp_ActiveShaders_SetInUse( bool b ){
	g_ActiveShaders.SetInUse( b );
}

// =============================================================================
// SYNAPSE

bool CSynapseClientShaders::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, SHADERS_MAJOR ) ) {
		_QERShadersTable* pTable = static_cast<_QERShadersTable*>( pAPI->mpTable );

		pTable->m_pfnFreeShaders = QERApp_FreeShaders;
		pTable->m_pfnReloadShaders = QERApp_ReloadShaders;
		pTable->m_pfnLoadShadersFromDir = QERApp_LoadShadersFromDir;
		pTable->m_pfnIsDirContainingShaders = QERApp_IsDirContainingShaders;
		pTable->m_pfnReloadShaderFile = QERApp_ReloadShaderFile;
		pTable->m_pfnLoadShaderFile = QERApp_LoadShaderFile;
		pTable->m_pfnHasShader = QERApp_HasShader;
		pTable->m_pfnTry_Shader_ForName = QERApp_Try_Shader_ForName;
		pTable->m_pfnShader_ForName = QERApp_Shader_ForName;
		pTable->m_pfnTry_Texture_ForName = QERApp_Try_Texture_ForName;
		pTable->m_pfnTexture_ForName = QERApp_Texture_ForName2;
		pTable->m_pfnGetActiveShaderCount = QERApp_GetActiveShaderCount;
		pTable->m_pfnColorShader_ForName = QERApp_ColorShader_ForName;
		pTable->m_pfnShader_ForName_NoLoad = QERApp_Shader_ForName_NoLoad;
		pTable->m_pfnActiveShaders_SetInUse = QERApp_ActiveShaders_SetInUse;
		pTable->m_pfnSortActiveShaders = QERApp_SortActiveShaders;
		pTable->m_pfnActiveShader_ForTextureName = QERApp_ActiveShader_ForTextureName;
		pTable->m_pfnCreateShader_ForTextureName = QERApp_CreateShader_ForTextureName;
		pTable->m_pfnActiveShaders_SetDisplayed = QERApp_ActiveShaders_SetDisplayed;
		pTable->m_pfnActiveShader_ForIndex = QERApp_ActiveShader_ForIndex;
		pTable->m_pfnCleanTextureName = QERApp_CleanTextureName;

		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}
