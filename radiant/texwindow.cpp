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

//
// Texture Window
//
// Leonardo Zide (leo@lokigames.com)
//

/*!\todo
   Clean up texture menu.
   - Remove all global variables and use some objects instead.
   - Create an interface for a plugin to add texture menu items.
   - Make sure the interface is not dependent on gtk.
 */

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <dirent.h>
#endif
#include <gtk/gtk.h>
#include <assert.h>
#include <sys/stat.h>
#include "stdafx.h"
#include "texwindow.h"
#include "str.h"
#include "missing.h"
#include "texmanip.h"

#define TYP_MIPTEX  68

#define FONT_HEIGHT 10

int texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int g_nTextureOffset = 0;

// current active texture directory
//++timo FIXME: I'm not sure this is used anymore
char texture_directory[128];
// if true, the texture window will only display in-use shaders
// if false, all the shaders in memory are displayed
qboolean g_bShowAllShaders;

bool g_bFilterEnabled = false;
CString g_strFilter;

// texture layout functions
// TTimo: now based on shaders
int nActiveShadersCount;
int nCurrentShader;
IShader*  pCurrentShader;
qtexture_t  *current_texture = NULL;
int current_x, current_y, current_row;

// globals for textures
int texture_nummenus;
char texture_menunames[MAX_TEXTUREDIRS][128];

// the list of scripts/*.shader files we need to work with
// those are listed in shaderlist file
// FIXME TTimo I get the feeling that those would need to move to the shaders module
//   for now it's still more simple to just keep it here
GSList *l_shaderfiles = NULL;

void SelectTexture( int mx, int my, bool bShift, bool bFitScale = false );

void  Texture_MouseDown( int x, int y, int buttons );
void  Texture_MouseMoved( int x, int y, int buttons );

CPtrArray g_lstSkinCache;

// TTimo: modifed to add a qtexture_t, Texture_LoadSkin loads using the shader API / QERApp_TryTexture_ForName
// m_strName is a copy of qtex->name
struct SkinInfo
{
	CString m_strName;
	int m_nTextureBind;
	qtexture_t *m_qtex;
	SkinInfo( const char *pName, int n, qtexture_t *qtex ){
		m_strName = pName;
		m_nTextureBind = n;
		m_qtex = qtex;
	};
	SkinInfo(){};
};

// =============================================================================
// global functions

// gets active texture extension
//
// FIXME: fix this to be generic from project file
//
int GetTextureExtensionCount(){
	// hardcoded hack for png support
	if ( g_pGameDescription->mGameFile == "sof2.game" ) {
		return 3;
	} else {
		return 2;
	}
}

const char* GetTextureExtension( int nIndex ){
	switch ( nIndex )
	{
	case 0:
		return "tga";
		break;
	case 1:
		return "jpg";
		break;
	case 2:
		return "png";
		break;
	default:
		return NULL;
	}
}

/*
   ==============
   Texture_InitPalette
   ==============
 */
void Texture_InitPalette( byte *pal ){
	int r,g,b;
	int i;
	int inf;
	byte gammatable[256];
	float gamma;

	gamma = g_qeglobals.d_savedinfo.fGamma;

	if ( gamma == 1.0 ) {
		for ( i = 0 ; i < 256 ; i++ )
			gammatable[i] = i;
	}
	else
	{
		for ( i = 0 ; i < 256 ; i++ )
		{
			inf = (int)( 255.0f * pow( ( i + 0.5f ) / 255.5f, gamma ) + 0.5f );
			if ( inf < 0 ) {
				inf = 0;
			}
			if ( inf > 255 ) {
				inf = 255;
			}
			gammatable[i] = inf;
		}
	}

	for ( i = 0 ; i < 256 ; i++ )
	{
		r = gammatable[pal[0]];
		g = gammatable[pal[1]];
		b = gammatable[pal[2]];
		pal += 3;
	}
}

void SetTexParameters( void ){
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_mode );

	switch ( texture_mode )
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	}
}

/*
   ============
   Texture_SetMode
   ============
 */
void Texture_SetMode( int iMenu ){
	int iMode;
	qboolean texturing = true;
	gpointer item = NULL;

	switch ( iMenu )
	{
	case ID_VIEW_NEAREST:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_nearest" );
		iMode = GL_NEAREST;
		break;
	case ID_VIEW_NEARESTMIPMAP:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_nearestmipmap" );
		iMode = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ID_VIEW_LINEAR:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_linear" );
		iMode = GL_LINEAR;
		break;
	case ID_VIEW_BILINEAR:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_bilinear" );
		iMode = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ID_VIEW_BILINEARMIPMAP:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_bilinearmipmap" );
		iMode = GL_LINEAR_MIPMAP_NEAREST;
		break;
	case ID_VIEW_TRILINEAR:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_trilinear" );
		iMode = GL_LINEAR_MIPMAP_LINEAR;
		break;
	case ID_TEXTURES_WIREFRAME:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_wireframe" );
		iMode = -1;
		texturing = false;
		break;
	case ID_TEXTURES_FLATSHADE:
		item = g_object_get_data( G_OBJECT( g_pParentWnd->m_pWidget ), "menu_view_flatshade" );
		iMode = -1;
		texturing = false;
		break;
	default:
		return;
	}

	g_qeglobals.d_savedinfo.iTexMenu = iMenu;
	// NOTE: texture_mode is a GLenum used directly in glTexParameter
	if ( iMode != -1 ) {
		texture_mode = iMode;
	}

	g_bIgnoreCommands++;
	if ( item != NULL ) {
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM( item ), TRUE );
	}
	g_bIgnoreCommands--;

	if ( texturing ) {
		SetTexParameters();
	}

	if ( !texturing && iMenu == ID_TEXTURES_WIREFRAME ) {
		g_pParentWnd->GetCamWnd()->Camera()->draw_mode = cd_wire;
		Map_BuildBrushData();
		Sys_UpdateWindows( W_ALL );
		return;
	}
	else if ( !texturing && iMenu == ID_TEXTURES_FLATSHADE ) {
		g_pParentWnd->GetCamWnd()->Camera()->draw_mode = cd_solid;
		Map_BuildBrushData();
		Sys_UpdateWindows( W_ALL );
		return;
	}

	for ( qtexture_t *q = g_qeglobals.d_qtextures; q; q = q->next )
	{
		qglBindTexture( GL_TEXTURE_2D, q->texture_number );
		SetTexParameters();
	}

	// select the default texture
	qglBindTexture( GL_TEXTURE_2D, 0 );

	qglFinish();

	if ( g_pParentWnd->GetCamWnd()->Camera()->draw_mode != cd_texture ) {
		g_pParentWnd->GetCamWnd()->Camera()->draw_mode = cd_texture;
		Map_BuildBrushData();
	}

	Sys_UpdateWindows( W_ALL );
}

/*!
   gamma correction stuff
   took out of QERApp_LoadTextureRGBA for clarity
 */
byte g_gammatable[256];
void ResampleGamma( float fGamma ){
	int i,inf;
	if ( fGamma == 1.0 ) {
		for ( i = 0; i < 256; i++ )
			g_gammatable[i] = i;
	}
	else
	{
		for ( i = 0; i < 256; i++ )
		{
			inf = (int)( 255.0f * pow( ( i + 0.5f ) / 255.5f, fGamma ) + 0.5f );
			if ( inf < 0 ) {
				inf = 0;
			}
			if ( inf > 255 ) {
				inf = 255;
			}
			g_gammatable[i] = inf;
		}
	}
}

/*!
   this function does the actual processing of raw RGBA data into a GL texture
   it will also generate the mipmaps
   it looks like pPixels nWidth nHeight are the only relevant parameters
 */
qtexture_t *QERApp_LoadTextureRGBA( unsigned char* pPixels, int nWidth, int nHeight ){
	static float fGamma = -1;
	float total[3];
	byte  *outpixels = NULL;
	int i, j, resampled, width2, height2, width3, height3;
	int max_tex_size = 0, mip = 0;
	int nCount = nWidth * nHeight;

	if ( fGamma != g_qeglobals.d_savedinfo.fGamma ) {
		fGamma = g_qeglobals.d_savedinfo.fGamma;
		ResampleGamma( fGamma );
	}

	qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &max_tex_size );
	if ( !max_tex_size ) {
		max_tex_size = 1024;
	}

	qtexture_t *q = (qtexture_t*)g_malloc( sizeof( *q ) );
	q->width = nWidth;
	q->height = nHeight;

	total[0] = total[1] = total[2] = 0.0f;

	// resample texture gamma according to user settings
	for ( i = 0; i < ( nCount * 4 ); i += 4 )
	{
		for ( j = 0; j < 3; j++ )
		{
			total[j] += ( pPixels + i )[j];
			byte b = ( pPixels + i )[j];
			( pPixels + i )[j] = g_gammatable[b];
		}
	}

	q->color[0] = total[0] / ( nCount * 255 );
	q->color[1] = total[1] / ( nCount * 255 );
	q->color[2] = total[2] / ( nCount * 255 );

	qglGenTextures( 1, &q->texture_number );

	qglBindTexture( GL_TEXTURE_2D, q->texture_number );

	SetTexParameters();

	width2 = 1; while ( width2 < nWidth ) width2 <<= 1;
	height2 = 1; while ( height2 < nHeight ) height2 <<= 1;

	width3 = width2;
	height3 = height2;
	while ( width3 > max_tex_size ) width3 >>= 1;
	while ( height3 > max_tex_size ) height3 >>= 1;
	if ( width3 < 1 ) {
		width3 = 1;
	}
	if ( height3 < 1 ) {
		height3 = 1;
	}

	if ( !( width2 == nWidth && height2 == nHeight ) ) {
		resampled = 1;
		outpixels = (byte *)malloc( width2 * height2 * 4 );
		R_ResampleTexture( pPixels, nWidth, nHeight, outpixels, width2, height2, 4 );
	}
	else {
		resampled = 0;
		outpixels = pPixels;
	}

	while ( width2 > width3 || height2 > height3 )
	{
		GL_MipReduce( outpixels, outpixels, width2, height2, width3, height3 );

		if ( width2 > width3 ) {
			width2 >>= 1;
		}
		if ( height2 > height3 ) {
			height2 >>= 1;
		}
	}

	qglTexImage2D( GL_TEXTURE_2D, mip++, g_qeglobals.texture_components, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, outpixels );
	while ( width2 > 1 || height2 > 1 )
	{
		GL_MipReduce( outpixels, outpixels, width2, height2, 1, 1 );

		if ( width2 > 1 ) {
			width2 >>= 1;
		}
		if ( height2 > 1 ) {
			height2 >>= 1;
		}

		qglTexImage2D( GL_TEXTURE_2D, mip++, g_qeglobals.texture_components, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, outpixels );
	}

	qglBindTexture( GL_TEXTURE_2D, 0 );
	if ( resampled ) {
		free( outpixels );
	}

	return q;
}

/*
   ==================
   DumpUnreferencedShaders
   usefull function: dumps the list of .shader files that are not referenced to the console
   ==================
 */
void DumpUnreferencedShaders(){
	GSList *lst, *sh, *files;
	bool bFound = false;

	files = vfsGetFileList( "scripts", "shader" );
	for ( lst = files; lst; lst = lst->next )
	{
		bool listed = false;

		for ( sh = l_shaderfiles; sh != NULL; sh = g_slist_next( sh ) )
			if ( !strcmp( (char*)sh->data, (char*)lst->data ) ) {
				listed = true;
				break;
			}

		if ( !listed ) {
			if ( !bFound ) {
				bFound = true;
				Sys_FPrintf( SYS_WRN, "Following shader files are not referenced in shaderlist.txt:\n" );
			}
			Sys_FPrintf( SYS_WRN, "%s\n", (char*)lst->data );
		}
	}

	vfsClearFileDirList( &files );
}

/*
   ==================
   BuildShaderList
   build a CStringList of shader names
   ==================
 */
void BuildShaderList(){
	int count, i;
	char filename[1024];
	char *pBuff;
	char dirstring[NAME_MAX];
	int nLen;
	if ( l_shaderfiles != NULL ) {
		g_slist_free( l_shaderfiles );
		l_shaderfiles = NULL;
	}

	if ( g_pGameDescription->mGameFile != "hl.game" ) {
		strcpy( filename, g_pGameDescription->mShaderlist.GetBuffer() );
		count = vfsGetFileCount( filename, 0 );
		if ( count == 0 ) {
			Sys_FPrintf( SYS_ERR, "Couldn't find '%s'\n", g_pGameDescription->mShaderlist.GetBuffer() );
			return;
		}

		for ( i = 0; i < count; i++ )
		{
			// NOTE TTimo we use vfsGetFullPath solely to get the full path of the shader list we are gonna load
			//   but we actually send the relative path to vfsLoadFile
			//   so let's hope there is no disparity between the two functions
			if ( !vfsGetFullPath( filename, i, 0 ) ) {
				Sys_FPrintf( SYS_ERR, "Couldn't find full path for '%s'\n", g_pGameDescription->mShaderlist.GetBuffer() );
				return;
			}
			Sys_Printf( "Parsing shader files from %s\n", vfsGetFullPath( filename, i, 0 ) );
			nLen = vfsLoadFile( filename, reinterpret_cast<void**>( &pBuff ), i );
			if ( nLen > 0 ) {
				StartTokenParsing( pBuff );
				nLen = 0;
				while ( GetToken( true ) )
				{
					GSList *tmp;
					bool found = false;

					// each token should be a shader filename
					sprintf( dirstring, "%s.shader", token );

					for ( tmp = l_shaderfiles; tmp != NULL; tmp = tmp->next )
					{
						if ( !strcmp( dirstring, (char*)tmp->data ) ) {
							found = true;
							Sys_FPrintf( SYS_WRN, "duplicate entry \"%s\" in shaderlist.txt\n", (char*)tmp->data );
							break;
						}
					}

					if ( !found ) {
						l_shaderfiles = g_slist_append( l_shaderfiles, strdup( dirstring ) );
						nLen++;
					}
				}
				g_free( pBuff );
			}
		}
	}
}

bool IsValidTextureName(char* name){
	CString strTemp;

	StripExtension( name );
	strTemp = name;
	strTemp.MakeLower();

	// avoid effect textures for Q3 texture sets
	if ( strTemp.Find( ".specular" ) >= 0 ||
		strTemp.Find( ".glow" ) >= 0 ||
		strTemp.Find( ".bump" ) >= 0 ||
		strTemp.Find( ".diffuse" ) >= 0 ||
		strTemp.Find( ".blend" ) >= 0 ||
		strTemp.Find( ".alpha" ) >= 0 ) {
		return false;
	}

	if ( g_str_has_suffix( name, "_g" ) ||
			// avoid glow, heightmap, normalmap and specular maps for Q4 texture sets
			g_str_has_suffix( name, "_h" ) ||
			g_str_has_suffix( name, "_local" ) ||
			g_str_has_suffix( name, "_nm" ) ||
			g_str_has_suffix( name, "_s" ) ||
			g_str_has_suffix( name, "_bump" ) ||
			g_str_has_suffix( name, "_gloss" ) ||
			g_str_has_suffix( name, "_luma" ) ||
			g_str_has_suffix( name, "_norm" ) ||
			// more well-known suffixes
			g_str_has_suffix( name, "_p" ) || // preview (used by qer_editorimage)
			g_str_has_suffix( name, "_g" ) || // gloss
			g_str_has_suffix( name, "_n" )    // normal
			) {
		return false;
	}

	// avoid ever loading a texture name with spaces
	if ( strTemp.Find( " " ) >= 0 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Skipping texture name with spaces [%s]\n", strTemp.GetBuffer() );
		return false;
	}

	return true;
}

bool IsDirContainingTextures(const char* path){
	char name[1024];
	char dirstring[1024];
	GSList *files = NULL, *temp;

	sprintf( dirstring, "textures/%s", path );
	g_ImageManager.BeginExtensionsScan();
	const char* ext;
	while ( ( ext = g_ImageManager.GetNextExtension() ) != NULL )
	{
		files = g_slist_concat( files, vfsGetFileList( dirstring, ext ) );
	}

	for ( temp = files; temp; temp = temp->next )
	{
		sprintf( name, "%s", (char*)temp->data );

		if ( IsValidTextureName( name ) ) {
			vfsClearFileDirList( &files );
			return true;
		}
	}

	vfsClearFileDirList( &files );
	return false;
}

void Texture_ListDirectory(){
	char name[1024];
	char dirstring[1024];
	int shaders_count = 0;
	int textures_count = 0;
	GSList *files = NULL, *temp;

	// load texture_directory.shader
	// NOTE: because of above call to Texture_ClearInuse, g_ActiveShaders will have the newly loaded shaders only
	// we'll use that later to check if textures have a shader associated or not
	// NOTE: all shaders loaded through QERApp_LoadShadersFromDir will get their InUse flag to True, we'll need a call to Texture_ShowInUse for later cleanup/adjustment
	// NOTE: QERApp_LoadShadersFromDir has two criterions for loading a shader:
	//   the shaderfile is texture_directory (like "museum" will load everything in museum.shader)
	//   the shader name contains texture_directory (like "base_floor" will load museum.shader::base_floor/concfloor_rain)
	shaders_count = QERApp_LoadShadersFromDir( texture_directory );
	// load remaining texture files
	// if a texture is already in use to represent a shader, ignore it

	// need this function "GSList *lst SynapseServer::GetMinorList(char *major_name);"

	sprintf( dirstring, "textures/%s", texture_directory );
	g_ImageManager.BeginExtensionsScan();
	const char* ext;
	while ( ( ext = g_ImageManager.GetNextExtension() ) != NULL )
	{
		files = g_slist_concat( files, vfsGetFileList( dirstring, ext ) );
	}

	for ( temp = files; temp; temp = temp->next )
	{
		sprintf( name, "%s%s", texture_directory, (char*)temp->data );

		if ( !IsValidTextureName( name ) ) {
			continue;
		}

		// build a texture name that fits the conventions for qtexture_t::name
		char stdName[1024];
		sprintf( stdName, "textures/%s", name );
		// check if this texture doesn't have a shader
		if ( !QERApp_ActiveShader_ForTextureName( stdName ) ) {
			QERApp_CreateShader_ForTextureName( stdName );
			textures_count++;
		}
	}

	Sys_Printf( "Loaded %d shaders and created default shader for %d orphan textures.\n",
				shaders_count, textures_count );

	vfsClearFileDirList( &files );
}

/*
   ==================
   FillTextureMenu

   ==================
 */
void ClearGSList( GSList* lst ){
	GSList *p = lst;
	while ( p )
	{
		g_free( p->data );
		p = g_slist_remove( p, p->data );
	}
}

void FillTextureList( GSList** pArray )
{
	GSList *p;
	char dirRoot[NAME_MAX];
	int texture_num;
	GSList *texdirs = NULL;
	GSList *texdirs_tmp = NULL;

	texture_num = 0;

	// scan texture dirs and pak files only if not restricting to shaderlist
	if ( !g_PrefsDlg.m_bTexturesShaderlistOnly ) {
		texdirs_tmp = vfsGetDirList( "textures/" );
		for ( p = texdirs_tmp; p; p = g_slist_next( p ) )
		{
			// Hydra: erm, this didn't used to do anything except leak memory...
			// For Halflife support this is required to work however.
			// g_slist_append(texdirs, p->data);
			if ( !g_PrefsDlg.m_bHideEmptyDirs || IsDirContainingTextures( (char*)p->data ) )
			{
				texdirs = g_slist_append( texdirs, g_strdup( (char *)p->data ) );
			}
			else
			{
				Sys_Printf( "Hiding empty texture dir: %s\n", g_strdup( (char *)p->data ) );
			}
		}
		vfsClearFileDirList( &texdirs_tmp );
	}

	// scan the shaders in shaderlist.txt
	BuildShaderList();
	PreloadShaders();
	DumpUnreferencedShaders();
	while ( l_shaderfiles != NULL )
	{
		char shaderfile[PATH_MAX];
		char *colon;
		gboolean found = FALSE;

		ExtractFileName( (char*)l_shaderfiles->data, shaderfile );
		StripExtension( shaderfile );
		strlwr( shaderfile );

		//support for shaderlist.txt tags, forward
		colon = strstr( (char*)l_shaderfiles->data, ":" );
		if( colon )
		{
			strncat( shaderfile, colon, sizeof( shaderfile ) - strlen( shaderfile ) - 1 );
		}

		for ( GSList *tmp = texdirs; tmp; tmp = g_slist_next( tmp ) )
		{
			if ( !strcasecmp( (char*)tmp->data, shaderfile ) ) {
				found = TRUE;
				break;
			}
		}

		if ( !found ) {
			if( !g_PrefsDlg.m_bHideEmptyDirs || QERApp_IsDirContainingShaders( shaderfile ) )
			{
				texdirs = g_slist_prepend( texdirs, g_strdup( shaderfile ) );
			}
			else
			{
				Sys_Printf( "Hiding empty shader dir: %s\n", g_strdup ( shaderfile ) );
			}
		}

		free( l_shaderfiles->data );
		l_shaderfiles = g_slist_remove( l_shaderfiles, l_shaderfiles->data );
	}

	// sort the list
	texdirs = g_slist_sort( texdirs, (GCompareFunc)strcmp );

	GSList *temp = texdirs;
	while ( temp )
	{
		char* ptr = strchr( (char*)temp->data, '_' );

		// do we shrink the menus?
		if ( ptr != NULL ) {
			// extract the root
			strcpy( dirRoot, (char*)temp->data );
			dirRoot[ptr - (char*)temp->data + 1] = 0;

			// we shrink only if we have at least two things to shrink :-)
			if ( temp->next && ( strstr( (char*)temp->next->data, dirRoot ) == (char*)temp->next->data ) ) {
				do
				{
					if ( pArray ) {
						*pArray = g_slist_append( *pArray, g_strdup( (char*)temp->data ) );
					}
					if ( ++texture_num == MAX_TEXTUREDIRS ) {
						Sys_FPrintf( SYS_WRN, "WARNING: max texture directories count has been reached!\n" );
						ClearGSList( texdirs );
						return;
					}
					temp = temp->next;
				}
				while ( temp && ( strstr( (char*)temp->data, dirRoot ) == temp->data ) );

				ptr = strchr( dirRoot, '_' );
				*ptr = 0;
				continue;
			}
		}
		if ( pArray ) {
			*pArray = g_slist_append( *pArray, g_strdup( (char*)temp->data ) );
		}
		if ( ++texture_num == MAX_TEXTUREDIRS ) {
			Sys_FPrintf( SYS_WRN, "WARNING: max texture directories count has been reached!\n" );
			ClearGSList( texdirs );
			return;
		}

		temp = temp->next;
	}
	ClearGSList( texdirs );
}

void FillTextureMenu( GSList *texdirs )
{
	GtkWidget *menu, *item;
	GList *children, *lst;
	char dirRoot[NAME_MAX];

	// delete everything
	menu = GTK_WIDGET( g_object_get_data( G_OBJECT( g_qeglobals_gui.d_main_window ), "menu_texture_dirs" ) );
	children = gtk_container_get_children( GTK_CONTAINER( menu ) );
	if( children ) {
		for ( lst = g_list_first( children ); lst != NULL; lst = g_list_next( lst ) )
		{
			gtk_widget_destroy( GTK_WIDGET( lst->data ) );
		}
		g_list_free( children );
	}

	GSList *temp = texdirs;
	while ( temp )
	{
		char* ptr = strchr( (char*)temp->data, '_' );

		// do we shrink the menus?
		if ( ptr != NULL ) {
			// extract the root
			strcpy( dirRoot, (char*)temp->data );
			dirRoot[ptr - (char*)temp->data + 1] = 0;

			// we shrink only if we have at least two things to shrink :-)
			if ( temp->next && ( strstr( (char*)temp->next->data, dirRoot ) == (char*)temp->next->data ) ) {
				GtkWidget *pSubMenu = gtk_menu_new();
				GtkWidget *pSubMenuRef = pSubMenu;
				// keep going...
				do
				{
					item = gtk_menu_item_new_with_label( (char*)temp->data );
					gtk_widget_show( item );
					CheckMenuSplitting( pSubMenu );
					gtk_container_add( GTK_CONTAINER( pSubMenu ), item );
					g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
										GINT_TO_POINTER( CMD_TEXTUREWAD + texture_nummenus ) );

					strcpy( texture_menunames[texture_nummenus], (char*)temp->data );
					strcat( texture_menunames[texture_nummenus], "/" );

					if ( ++texture_nummenus == MAX_TEXTUREDIRS ) {
						Sys_FPrintf( SYS_WRN, "WARNING: max texture directories count has been reached!\n" );
						// push submenu and get out
						item = gtk_menu_item_new_with_label( dirRoot );
						gtk_widget_show( item );
						gtk_container_add( GTK_CONTAINER( menu ), item );
						gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), pSubMenu );
						ClearGSList( texdirs );
						return;
					}
					temp = temp->next;
				}
				while ( temp && ( strstr( (char*)temp->data, dirRoot ) == temp->data ) );

				ptr = strchr( dirRoot, '_' );
				*ptr = 0;
				item = gtk_menu_item_new_with_label( dirRoot );
				gtk_widget_show( item );
				CheckMenuSplitting( menu );
				gtk_container_add( GTK_CONTAINER( menu ), item );
				gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), pSubMenuRef );
				continue;
			}
		}

		item = gtk_menu_item_new_with_label( (char*)temp->data );
		gtk_widget_show( item );
		CheckMenuSplitting( menu );
		gtk_container_add( GTK_CONTAINER( menu ), item );
		g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( HandleCommand ),
							GINT_TO_POINTER( CMD_TEXTUREWAD + texture_nummenus ) );

		strcpy( texture_menunames[texture_nummenus], (char*)temp->data );
		strcat( texture_menunames[texture_nummenus], "/" );

		if ( ++texture_nummenus == MAX_TEXTUREDIRS ) {
			Sys_FPrintf( SYS_WRN, "WARNING: max texture directories count has been reached!\n" );
			ClearGSList( texdirs );
			return;
		}

		temp = temp->next;
	}
}

void FillTextureDirListWidget( GSList *texdirs )
{
	GtkWidget* treeview;
	GtkTreeModel* model;
	GtkListStore* store;
	GtkTreeIter iter;
	GSList *dir;

	treeview = GTK_WIDGET( g_object_get_data( G_OBJECT( g_qeglobals_gui.d_main_window ), "dirlist_treeview" ) );
	if( treeview == NULL ) {
		return;
	}
	model = gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) );
	store = GTK_LIST_STORE( model );

	gtk_list_store_clear( store );

	for( dir = texdirs; dir != NULL; dir = g_slist_next( dir ) )
	{
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, (gchar*)dir->data, -1 );
	}
}

void Texture_ShowDirectory_by_path( const char* pPath )
{
	snprintf( texture_directory, sizeof( texture_directory ), "%s%s", pPath, "/" );
	Texture_ShowDirectory();
}

/*
   ==============
   Texture_ShowDirectory
   relies on texture_directory global for the directory to use
   called by
   void Texture_ShowDirectory (int menunum, bool bLinked)
   void	Texture_ShowDirectory (char* pPath, bool bLinked)
   1) Load the shaders for the given directory
   2) Scan the remaining texture, load them and assign them a default shader (the "noshader" shader)
   NOTE: when writing a texture plugin, or some texture extensions, this function may need to be overriden, and made
   available through the IShaders interface
   NOTE: for texture window layout:
   all shaders are stored with alphabetical order after load
   previously loaded and displayed stuff is hidden, only in-use and newly loaded is shown
   ( the GL textures are not flushed though)
   ==============
 */

void Texture_ShowDirectory(){
	char name[1024];

	g_bScreenUpdates = false;

	// refresh the in-use textures: that will clear the IsDisplayed flag on unused stuff
	// and leave it on in-use so they'll still be displayed
	Texture_ShowInuse();
	// and textures loaded in the following lines will be displayed as well...
	// NOTE: shaders that are not in use but have been loaded previously are still in memory. But they don't get displayed.

	g_qeglobals.d_texturewin.originy = 0;

	Texture_ListDirectory();

	// sort for displaying
	QERApp_SortActiveShaders();

	sprintf( name, "Textures: %s", texture_directory );
	gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), name );

	// select the first texture in the list
	if ( !g_qeglobals.d_texturewin.texdef.GetName()[0] ) {
		SelectTexture( 16, g_qeglobals.d_texturewin.height - 16, false );
	}

	g_bScreenUpdates = true;

	Sys_UpdateWindows( W_TEXTURE );
}

/*
   ==============
   Texture_ShowDirectory
   1) Load the shaders for the given directory
   2) Scan the remaining texture, load them and assign them a default shader (the "noshader" shader)
   NOTE: when writing a texture plugin, or some texture extensions, this function may need to be overriden, and made
   available through the IShaders interface
   ==============
 */
void Texture_ShowDirectory( int menunum ){
	strcpy( texture_directory, texture_menunames[menunum - CMD_TEXTUREWAD] );
	Texture_ShowDirectory();
}

void Texture_GetSize( qtexture_t *tex, int & nWidth, int & nHeight ){
	if( !tex ) 
		return;

	if( g_PrefsDlg.m_bFixedTextureSize && g_PrefsDlg.m_nFixedTextureSizeWidth > 0 && g_PrefsDlg.m_nFixedTextureSizeHeight > 0 )
	{
		nWidth = g_PrefsDlg.m_nFixedTextureSizeWidth;
		nHeight = g_PrefsDlg.m_nFixedTextureSizeHeight;
		float ratioWidth = nHeight / nWidth;
		float ratioHeight = nWidth / nHeight;
		if( tex->width * ratioWidth > tex->height * ratioHeight )
		{
			nHeight *= tex->height * 1.0f / tex->width * ratioWidth;
		} else 
		if( tex->height * ratioHeight > tex->width * ratioWidth )
		{
			nWidth *= tex->width * 1.0f / tex->height * ratioHeight;
		}
	} else {
		nWidth = (int)( tex->width * ( (float)g_PrefsDlg.m_nTextureScale / 100 ) );
		nHeight = (int)( tex->height * ( (float)g_PrefsDlg.m_nTextureScale / 100 ) );
	}
}

// scroll origin so the current texture is completely on screen
// if current texture is not displayed, nothing is changed
void Texture_ResetPosition(){
	qtexture_t  *q;
	int x,y;

	//this shouldn't ever happen, we startup with notex
	if ( !g_qeglobals.d_texturewin.texdef.GetName()[0] ) {
		return;
	}

	// otherwise position with current texture shown
	// this used to be in Texture_SetTexture
	Texture_StartPos();
	while ( 1 )
	{
		// NOTE: return value is == pCurrentShader and pCurrentShader->getTexture == current_texture
		Texture_NextPos( &x, &y );
		q = current_texture;
		// if the current texture never found (because // 'show shaders' is off,
		// for example), do nothing
		if ( !q ) {
			break;
		}

		int nHeight;
		int nWidth;
		Texture_GetSize( q, nWidth, nHeight );
		// we have found when texdef->name and the shader name match
		// NOTE: as everywhere else for our comparisons, we are not case sensitive
		if ( !strcmpi( g_qeglobals.d_texturewin.texdef.GetName(), pCurrentShader->getName() ) ) {
			// take care of calls before initialized
			if ( !g_qeglobals.d_texturewin.height ) {
				g_qeglobals.d_texturewin.originy = 0;
				break;
			}
			// if the bottom of our selected texture will fit with origin 0, use that
			// to prevent scrolling uglyness (stuff scrolled off screen when
			// everything would fit)
			if ( -( y - nHeight - 2 * FONT_HEIGHT ) <  g_qeglobals.d_texturewin.height ) {
				g_qeglobals.d_texturewin.originy = 0;
				break;
			}
			// if current is off the top of the window, move it to the top
			if ( y > g_qeglobals.d_texturewin.originy ) {
				g_qeglobals.d_texturewin.originy = y;
				break;
			}

			// if current is off the bottom, put it on the bottom
			if ( y - nHeight - 2 * FONT_HEIGHT < g_qeglobals.d_texturewin.originy - g_qeglobals.d_texturewin.height ) {
				g_qeglobals.d_texturewin.originy = y - nHeight - 2 * FONT_HEIGHT + g_qeglobals.d_texturewin.height;
				break;
			}
			// if we made it here, it should already be in view
			break;
		}
	}
	Sys_UpdateWindows( W_TEXTURE );
}

/*
   ==============
   Texture_ShowAll
   will set the IsDisplayed flag on all the active shaders, so we see everything that's currently in memory
   ==============
 */
void Texture_ShowAll(){
	char name[1024];

#ifdef _DEBUG
	if ( g_bShowAllShaders ) {
		Sys_FPrintf( SYS_WRN, "WARNING: already showing all shaders\n" );
	}
#endif
	QERApp_ActiveShaders_SetDisplayed( true );
	g_bShowAllShaders = true;
	// put some information in the texture window title?
	sprintf( name, "Textures: in use" );
	gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), name );
	Sys_UpdateWindows( W_TEXTURE );
}

/*
   ==============
   Texture_ShowInuse
   clear all IsDisplayed flags
   scan the map, set IsInUse (will set IsDisplayed on the way)
   NOTE: don't sort the textures, don't update the windows (it's used in several contexts, not always necessary to do either)
   ==============
 */
void WINAPI Texture_ShowInuse( void ){
	face_t  *f;
	brush_t *b;
	char name[1024];

	g_qeglobals.d_texturewin.originy = 0;

	// purge
	QERApp_ActiveShaders_SetDisplayed( false );
	// scan and only display in-use stuff
	Sys_Status( "Selecting active textures", 0 );

	for ( b = active_brushes.next ; b != NULL && b != &active_brushes ; b = b->next )
	{
		if ( b->patchBrush ) {
			b->pPatch->pShader->SetInUse( true );
		}
		else
		{
			for ( f = b->brush_faces ; f ; f = f->next )
			{
				f->pShader->SetInUse( true );
			}
		}
	}
	for ( b = selected_brushes.next ; b != NULL && b != &selected_brushes ; b = b->next )
	{
		if ( b->patchBrush ) {
			b->pPatch->pShader->SetInUse( true );
		}
		else
		{
			for ( f = b->brush_faces ; f ; f = f->next )
			{
				f->pShader->SetInUse( true );
			}
		}
	}

	// we are no longer showing everything
	g_bShowAllShaders = false;
	// put some information in the texture window title?
	sprintf( name, "Textures: in use" );
	gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_entity ), name );


	// select the first texture in the list
	if ( !g_qeglobals.d_texturewin.texdef.GetName()[0] ) {
		SelectTexture( 16, g_qeglobals.d_texturewin.height - 16, false );
	}
}

void Texture_ShowStartupShaders(){
	if ( g_PrefsDlg.m_nShader == PrefsDlg::SHADER_COMMON ) {
		// RIANT
		// HACK FOR JK2 SUPPORT
		if ( g_pGameDescription->mGameFile == "jk2.game" || g_pGameDescription->mGameFile == "ja.game" ) {
			strcpy( texture_directory, "system/" );
		}
		// RIANT
		// HACK FOR SOF2 SUPPORT
		else if ( g_pGameDescription->mGameFile == "sof2.game" ) {
			strcpy( texture_directory, "tools/" );
		}
		else{ strcpy( texture_directory, "common/" ); }
		Texture_ShowDirectory();
	}

	if ( g_PrefsDlg.m_nShader == PrefsDlg::SHADER_ALL ) {
		int count, i;
		char filename[1024];
		char   *pBuff;
		char dirstring[NAME_MAX];
		int nLen;
		GSList *shaderfiles = NULL;

		strcpy( filename, g_pGameDescription->mShaderlist.GetBuffer() );
		count = vfsGetFileCount( filename, 0 );
		if ( count == 0 ) {
			Sys_FPrintf( SYS_ERR, "Couldn't find '%s'\n", g_pGameDescription->mShaderlist.GetBuffer() );
			return;
		}

		if ( !vfsGetFullPath( filename, 0, 0 ) ) {
			Sys_FPrintf( SYS_ERR, "Couldn't find full path for '%s'\n", g_pGameDescription->mShaderlist.GetBuffer() );
			return;
		}

		for ( i = 0; i < count; i++ )
		{
			Sys_Printf( "Parsing shader files from %s\n", vfsGetFullPath( filename, i, 0 ) );
			nLen = vfsLoadFile( filename, reinterpret_cast<void**>( &pBuff ), i );
			if ( nLen > 0 ) {
				StartTokenParsing( pBuff );
				nLen = 0;
				while ( GetToken( true ) )
				{
					GSList *tmp;
					bool found = false;

					// each token should be a shader filename
					sprintf( dirstring, "%s.shader", token );

					for ( tmp = shaderfiles; tmp != NULL; tmp = tmp->next )
					{
						if ( !strcmp( dirstring, (char*)tmp->data ) ) {
							found = true;
							Sys_FPrintf( SYS_WRN, "duplicate entry \"%s\" in shaderlist.txt\n", (char*)tmp->data );
							break;
						}
					}

					if ( !found ) {
						shaderfiles = g_slist_append( l_shaderfiles, strdup( dirstring ) );
						strcpy( texture_directory, dirstring );
						Texture_ShowDirectory();
						nLen++;
					}
				}
				g_free( pBuff );
			}
		}
	}
}

/*
   ============================================================================

   TEXTURE LAYOUT

   TTimo: now based on a rundown through all the shaders
   nActiveShadersCount: number of shader that have a qtexture_t and may be displayed in the tex window
   nCurrentShader: index of active shader that has the current_texture
   pCurrentShader: IShader* for current shader
   NOTE: we expect the Active shaders count doesn't change during a Texture_StartPos .. Texture_NextPos cycle
   otherwise we may need to rely on a list instead of an array storage
   ============================================================================
 */

void Texture_GetPosSize( qtexture_t *tex, int & nWidth, int & nHeight ){
	if( !tex ) 
		return;

	if( g_PrefsDlg.m_bFixedTextureSize && g_PrefsDlg.m_nFixedTextureSizeWidth > 0 && g_PrefsDlg.m_nFixedTextureSizeHeight > 0 )
	{
		nWidth = g_PrefsDlg.m_nFixedTextureSizeWidth;
		nHeight = g_PrefsDlg.m_nFixedTextureSizeHeight;
	} else {
		nWidth = (int)( tex->width * ( (float)g_PrefsDlg.m_nTextureScale / 100 ) );
		nHeight = (int)( tex->height * ( (float)g_PrefsDlg.m_nTextureScale / 100 ) );
	}
}

void Texture_StartPos( void ){
	//++timo TODO: check use of current_texture and current_row?
	current_x = 8;
	current_y = -8;
	current_row = 0;
	nActiveShadersCount = QERApp_GetActiveShaderCount();
	nCurrentShader = -1;
	current_texture = NULL;
	pCurrentShader = NULL;
}

// if texture_showinuse jump over non in-use textures
// it's not very clear what should be done here and what in Texture_Draw .. maybe merging the two would do good
IShader* Texture_NextPos( int *x, int *y ){
	qtexture_t* q;
	while ( 1 )
	{
		if ( nCurrentShader >= nActiveShadersCount - 1 ) {
			// no more shaders
			current_texture = NULL;
			pCurrentShader = NULL;
			return NULL;
		}
		nCurrentShader++;
		pCurrentShader = QERApp_ActiveShader_ForIndex( nCurrentShader );
		if ( pCurrentShader == NULL ) {
			Sys_FPrintf( SYS_ERR, "ERROR: unexpected pCurrentShader == NULL in Texture_NextPos\n" );
			return NULL;
		}
		current_texture = pCurrentShader->getTexture();
		q = current_texture;

		if ( !q ) {
			Sys_FPrintf( SYS_WRN, "WARNING: found an IShader without qtexture_t in Texture_NextPos\n" );
			return NULL;
		}

		/*
		   Never show anything other than "textures/" path,
		   This is for q1/q2/q3 .map format, which expects "textures/" path on everything we apply
		 */
		if ( strncmp( pCurrentShader->getName(), "textures/", 9 ) != 0 ) {
			continue;
		}

		// don't show shaders?
		if ( !( g_PrefsDlg.m_bShowShaders || pCurrentShader->IsDefault() ) ) {
			continue;
		}

		if ( g_PrefsDlg.m_bTextureWindow ) {
			// some basic filtering
			if ( !g_pParentWnd->GetTexWnd()->CheckFilter( pCurrentShader->getName() ) ) {
				continue;
			}
		}

		//++timo FIXME: texture_showinuse is useless? with the menu and reload we just refresh the IsDisplayed flag
		// but the IsInUse is only relevant to draw the green outline
		if ( pCurrentShader->IsDisplayed() ) {
			break;
		}

		continue;
	}

	int nWidth;
	int nHeight;
	Texture_GetPosSize( q, nWidth, nHeight );
	if ( current_x + nWidth > g_qeglobals.d_texturewin.width - 8 && current_row ) { // go to the next row unless the texture is the first on the row
		current_x = 8;
		current_y -= current_row + FONT_HEIGHT + 4;
		current_row = 0;
	}

	*x = current_x;
	*y = current_y;

	// Is our texture larger than the row? If so, grow the
	// row height to match it

	if ( current_row < nHeight ) {
		current_row = nHeight;
	}

	// never go less than 64, or the names get all crunched up
	current_x += nWidth < 64 ? 64 : nWidth;
	current_x += 8;

	return pCurrentShader;
}

/*
   ============================================================================

   MOUSE ACTIONS

   ============================================================================
 */

static int textures_cursorx, textures_cursory;

/*
   ============
   Texture_SetTexture

   brushprimit_texdef must be understood as a qtexture_t with width=2 height=2 ( the default one )
   ============
 */

//++timo NOTE: this is a mix of Shader module stuff and texture explorer
// it might need to be split in parts or moved out .. dunno
void WINAPI Texture_SetTexture( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, IPluginTexdef *pTexdef, bool bSetSelection ){
	if ( texdef->GetName()[0] == '(' ) {
		Sys_Status( "Can't select an entity texture", 0 );
		return;
	}
	g_qeglobals.d_texturewin.texdef = *texdef;
	g_qeglobals.d_texturewin.texdef.flags &= ~SURF_KEEP;
	g_qeglobals.d_texturewin.texdef.contents &= ~CONTENTS_KEEP;
	// store the shader pointer
	// NOTE: maybe passing the shader pointer would help?
	g_qeglobals.d_texturewin.pShader->DecRef();
	g_qeglobals.d_texturewin.pShader = QERApp_Shader_ForName( texdef->GetName() );
	g_qeglobals.d_texturewin.pShader->IncRef();
	// set this shader as in use
	g_qeglobals.d_texturewin.pShader->SetInUse( true );
	// store the texture coordinates for new brush primitive mode
	// be sure that all the callers are using the default 2x2 texture
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		g_qeglobals.d_texturewin.brushprimit_texdef = *brushprimit_texdef;
	}

	g_dlgFind.updateTextures( texdef->GetName() );
	if ( !g_dlgFind.isOpen() && bSetSelection ) {
		Select_SetTexture( texdef,brushprimit_texdef, bFitScale );
	}

	//plugins: send a message telling that the selected texture may have changed
	DispatchRadiantMsg( RADIANT_TEXTURE );

	// scroll origin so the texture is completely on screen
	// takes texdef from g_qeglobals.d_texturewin.texdef, set above
	Texture_ResetPosition();
}

void ViewShader( const char *pFile, const char *pName ){
	//  ask the vfs to build the full path to the file
	// (i.e. the first one found)
	char *fullName = vfsGetFullPath( pFile,0,0 );
	if ( fullName == NULL ) {
		Sys_FPrintf( SYS_ERR, "Couldn't get a full path to the shader file: %s\n", pFile );
		return;
	}

	char* pBuff = NULL;
	int nSize = vfsLoadFullPathFile( fullName, reinterpret_cast<void**>( &pBuff ) );
	if ( nSize <= 0 ) {
		Sys_FPrintf( SYS_ERR, "Failed to load shader file %s\n", fullName );
		return;
	}
	// look for the shader declaration
	int nStart;
	CString strFind = pName;
	CString strLook = pBuff;
	strLook.MakeLower();
	strFind.MakeLower();
	// offset used when jumping over commented out definitions
	int nOffset = 0;
	while ( true )
	{
		nStart = strLook.Find( strFind, nOffset );
		if ( nStart == -1 ) {
			break;
		}
		// we have found something, maybe it's a commented out shader name?
		char *strCheck = new char[strLook.GetLength() + 1];
		strcpy( strCheck, strLook.GetBuffer() );
		strCheck[nStart] = 0;
		char *pCheck = strrchr( strCheck, '\n' );
		// if there's a commentary sign in-between we'll continue
		if ( pCheck && strstr( pCheck, "//" ) ) {
			delete[] strCheck;
			nOffset = nStart + 1;
			continue;
		}
		delete[] strCheck;
		nOffset = nStart;
		break;
	}
	// now close the file
	g_free( pBuff );

	DoTextEditor( fullName, nOffset );
}

/*
   ==============
   SelectTexture

   By mouse click
   ==============
 */
void SelectTexture( int mx, int my, bool bShift, bool bFitScale ){
	int x, y;
	qtexture_t  *q;
	texdef_t tex;
	brushprimit_texdef_t brushprimit_tex;

	my += g_qeglobals.d_texturewin.originy - g_qeglobals.d_texturewin.height;

	Texture_StartPos();
	while ( 1 )
	{
		// NOTE: return value is == pCurrentShader and pCurrentShader->getTexture == current_texture
		Texture_NextPos( &x, &y );
		q = current_texture;
		if ( !q ) {
			break;
		}
		int nWidth;
		int nHeight;
		Texture_GetPosSize( q, nWidth, nHeight );
		if ( mx > x && mx - x < nWidth
			 && my < y && y - my < nHeight + FONT_HEIGHT ) {
			if ( bShift ) {
				if ( pCurrentShader->IsDefault() ) {
					Sys_FPrintf( SYS_ERR, "ERROR: %s is not a shader, it's a texture.\n", pCurrentShader->getName() );
				}
				else{
					ViewShader( pCurrentShader->getShaderFileName(), pCurrentShader->getName() );
				}
			}
			else
			{
				memset( &tex, 0, sizeof( tex ) );
				memset( &brushprimit_tex, 0, sizeof( brushprimit_tex ) );
				if ( g_qeglobals.m_bBrushPrimitMode ) {
					// brushprimit fitted to a 2x2 texture
					brushprimit_tex.coords[0][0] = 1.0f;
					brushprimit_tex.coords[1][1] = 1.0f;
				}
				else
				{
					tex.scale[0] = g_PrefsDlg.m_fDefTextureScale;
					tex.scale[1] = g_PrefsDlg.m_fDefTextureScale;
				}
				tex.flags = pCurrentShader->getFlags();
				// TTimo - shader code cleanup
				// texdef.name is the name of the shader, not the name of the actual texture file
				tex.SetName( pCurrentShader->getName() );
				// NOTE WARNING: Texture_SetTexture uses Texture_NextPos stuff to move the window position on to the texture
				// if there's some kind of fuckup in Texture_SetTexture you may end up with different pCurrentShader or even pCurrentShader == NULL
				// so we just consider pCurrentShader and current_texture are not valid after this point
				IShader *pAuxShader = pCurrentShader;
				Texture_SetTexture( &tex, &brushprimit_tex, bFitScale, NULL ); // Nurail
				CString strTex;
				CString strName;
				// if shader, print shader name, otherwise texture name
				//++timo FIXME: maybe CShader needs some properties between color / default / actual shader
#ifdef _DEBUG
				// this one is never supposed to be set as current one
				if ( pAuxShader->IsColor() ) {
					Sys_FPrintf( SYS_ERR, "ERROR: unexpected pCurrentShader->IsColor() in SelectTexture\n" );
				}
#endif
				// NOTE: IsColor is false, IsDefault the only remaining property
				if ( pAuxShader->IsDefault() ) {
					strName = q->name;
					// remove the "textures/" if needed
					if ( strName.Find( "textures/" ) != -1 ) {
						strName = strName.Mid( 9 );
					}
				}
				else
				{
					strName = pAuxShader->getName();
				}
				strTex.Format( "%s W: %i H: %i", strName.GetBuffer(), q->width, q->height );
				g_pParentWnd->SetStatusText( 3, strTex );
			}
			return;
		}
	}

	Sys_Status( "Did not select a texture", 0 );
}

/*
   ==============
   Texture_MouseDown
   ==============
 */
void Texture_MouseDown( int x, int y, int buttons ){
	Sys_GetCursorPos( &textures_cursorx, &textures_cursory );

	// lbutton = select texture
	if ( buttons == MK_LBUTTON || buttons == ( MK_LBUTTON | MK_SHIFT ) || buttons == ( MK_LBUTTON | MK_CONTROL ) ) {
		SelectTexture( x, g_qeglobals.d_texturewin.height - 1 - y, buttons & MK_SHIFT, buttons & MK_CONTROL );
		UpdateSurfaceDialog();
		UpdatePatchInspector();
	}
}

/*
   ==============
   Texture_MouseMoved
   ==============
 */

void Texture_MouseMoved( int x, int y, int buttons ){
	int scale = 1;

	if ( buttons & MK_SHIFT ) {
		scale = 4;
	}

	// rbutton = drag texture origin
	if ( buttons & MK_RBUTTON ) {
		Sys_GetCursorPos( &x, &y );
		if ( y != textures_cursory ) {
			g_qeglobals.d_texturewin.originy += ( y - textures_cursory ) * scale;
			if ( g_qeglobals.d_texturewin.originy > 0 ) {
				g_qeglobals.d_texturewin.originy = 0;
			}
			Sys_SetCursorPos( textures_cursorx, textures_cursory );

			// (g_PrefsDlg.m_bTextureScrollbar && g_qeglobals_gui.d_texture_scroll != NULL)
			// fixes broken texture scrolling when scrollbar is disabled
			GtkAdjustment *vadjustment = gtk_range_get_adjustment( GTK_RANGE( g_qeglobals_gui.d_texture_scroll ) );
			gtk_adjustment_set_value( vadjustment, abs( g_qeglobals.d_texturewin.originy ) );
			//
		}
		return;
	}
}

/*
   ============================================================================

   DRAWING

   ============================================================================
 */

int imax( int iFloor, int i ) {
	if ( i > iFloor ) {
		return iFloor;
	}
	return i;
}

/*
   ============
   Texture_Draw
   TTimo: relying on the shaders list to display the textures
   we must query all qtexture_t* to manage and display through the IShaders interface
   this allows a plugin to completely override the texture system
   ============
 */
void Texture_Draw( int width, int height ){
	int x, y, last_y = 0, last_height = 0, nWidth, nHeight;
	qtexture_t *q;
	char *name;

	qglClearColor( g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][0],
				   g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][1],
				   g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][2], 0 );
	qglViewport( 0,0,width,height );
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );
	qglOrtho( 0, width, g_qeglobals.d_texturewin.originy - height, g_qeglobals.d_texturewin.originy, -100, 100 );
	qglEnable( GL_TEXTURE_2D );

	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	g_qeglobals.d_texturewin.width = width;
	g_qeglobals.d_texturewin.height = height;

	Texture_StartPos();
	for (;; )
	{
		// NOTE: return value is == pCurrentShader and pCurrentShader->getTexture == current_texture
		Texture_NextPos( &x, &y );
		q = current_texture;
		if ( !q ) {
			break;
		}

		Texture_GetSize( q, nWidth, nHeight );

		if ( y != last_y ) {
			last_y = y;
			last_height = 0;
		}
		last_height = MAX( nHeight, last_height );

		// Is this texture visible?
		if ( ( y - nHeight - FONT_HEIGHT < g_qeglobals.d_texturewin.originy )
			 && ( y > g_qeglobals.d_texturewin.originy - height ) ) {
			// borders rules:
			// if it's the current texture, draw a thick red line, else:
			// shaders have a white border, simple textures don't
			// if !texture_showinuse: (some textures displayed may not be in use)
			// draw an additional square around with 0.5 1 0.5 color
			if ( !strcmpi( g_qeglobals.d_texturewin.texdef.GetName(), pCurrentShader->getName() ) ) {
				qglLineWidth( 3 );
				qglColor3f( 1,0,0 );
				qglDisable( GL_TEXTURE_2D );

				qglBegin( GL_LINE_LOOP );
				qglVertex2f( x - 4,y - FONT_HEIGHT + 4 );
				qglVertex2f( x - 4,y - FONT_HEIGHT - nHeight - 4 );
				qglVertex2f( x + 4 + nWidth,y - FONT_HEIGHT - nHeight - 4 );
				qglVertex2f( x + 4 + nWidth,y - FONT_HEIGHT + 4 );
				qglEnd();

				qglEnable( GL_TEXTURE_2D );
				qglLineWidth( 1 );
			}
			else
			{
				qglLineWidth( 1 );
				// shader border:
				if ( !pCurrentShader->IsDefault() ) {
					qglColor3f( 1,1,1 );
					qglDisable( GL_TEXTURE_2D );

					qglBegin( GL_LINE_LOOP );
					qglVertex2f( x - 1,y + 1 - FONT_HEIGHT );
					qglVertex2f( x - 1,y - nHeight - 1 - FONT_HEIGHT );
					qglVertex2f( x + 1 + nWidth,y - nHeight - 1 - FONT_HEIGHT );
					qglVertex2f( x + 1 + nWidth,y + 1 - FONT_HEIGHT );
					qglEnd();
					qglEnable( GL_TEXTURE_2D );
				}

				// highlight in-use textures
				if ( pCurrentShader->IsInUse() ) {
					qglColor3f( 0.5,1,0.5 );
					qglDisable( GL_TEXTURE_2D );
					qglBegin( GL_LINE_LOOP );
					qglVertex2f( x - 3,y + 3 - FONT_HEIGHT );
					qglVertex2f( x - 3,y - nHeight - 3 - FONT_HEIGHT );
					qglVertex2f( x + 3 + nWidth,y - nHeight - 3 - FONT_HEIGHT );
					qglVertex2f( x + 3 + nWidth,y + 3 - FONT_HEIGHT );
					qglEnd();
					qglEnable( GL_TEXTURE_2D );
				}
			}

			// Draw the texture
			qglBindTexture( GL_TEXTURE_2D, q->texture_number );
			QE_CheckOpenGLForErrors();
			qglColor3f( 1,1,1 );
			qglBegin( GL_QUADS );
			qglTexCoord2f( 0,0 );
			qglVertex2f( x,y - FONT_HEIGHT );
			qglTexCoord2f( 1,0 );
			qglVertex2f( x + nWidth,y - FONT_HEIGHT );
			qglTexCoord2f( 1,1 );
			qglVertex2f( x + nWidth,y - FONT_HEIGHT - nHeight );
			qglTexCoord2f( 0,1 );
			qglVertex2f( x,y - FONT_HEIGHT - nHeight );
			qglEnd();

			// draw the texture name
			qglDisable( GL_TEXTURE_2D );
			qglColor3f( 1,1,1 );

			qglRasterPos2f( x, y - FONT_HEIGHT + 2 );

			// don't draw the directory name
			name = (char*)pCurrentShader->getName();
			name += strlen( name );
			while ( name != (char*)pCurrentShader->getName() && *( name - 1 ) != '/' && *( name - 1 ) != '\\' )
				name--;

			gtk_glwidget_print_string( name );
			qglEnable( GL_TEXTURE_2D );
		}
	}

	g_qeglobals.d_texturewin.m_nTotalHeight = abs( y ) + last_height + FONT_HEIGHT + 4;

	// reset the current texture
	qglBindTexture( GL_TEXTURE_2D, 0 );
	qglFinish();
}

//++timo seems we only know hard inits now..
//void Texture_Init (bool bHardInit)
void Texture_Init(){
	g_qeglobals.d_qtextures = NULL;
	// initialize the qtexture map
	if ( g_qeglobals.d_qtexmap ) {
		Sys_FPrintf( SYS_ERR, "TODO: delete g_qeglobals.d_qtexmap in Texture_Init\n" );
	}
	g_qeglobals.d_qtexmap = g_hash_table_new( g_str_hash, g_str_equal );
	// initialize .. in some cases if no default texture / project loaded it crashes
	memset( &g_qeglobals.d_texturewin.texdef, 0, sizeof( g_qeglobals.d_texturewin.texdef ) );
	g_qeglobals.d_texturewin.texdef.SetName( SHADER_NOT_FOUND );
	g_qeglobals.d_texturewin.pShader = QERApp_Shader_ForName( SHADER_NOT_FOUND );
}

// FIXME TTimo this needs to move to the shader module along with l_shaderlist move
// preload shader files that have been listed in shaderlist.txt
void PreloadShaders(){
	GSList *lst = l_shaderfiles;
	Str shadername;
	while ( lst )
	{
		shadername = g_pGameDescription->mShaderPath;
		shadername += (char*)lst->data;
		QERApp_LoadShaderFile( shadername.GetBuffer() );
		lst = lst->next;
	}
}

// TTimo: modified to expect the reletive path to the skin as input
// will look into pak files if necessary
// uses the shader code to load the texture Try_Texture_ForName
// modified SkinInfo accordingly to store the qtexture_t and shader name (reletive version)
// the .md3 have bundled filetype extension, but they don't fit with the actual data
//   ex: models/mapobjects/gargoyle.tga doesn't exist, but models/mapobjects/gargoyle.jpg can be used instead
//   so we remove the extension before load attempt
int WINAPI Texture_LoadSkin( char *pName, int *pnWidth, int *pnHeight ){
	//  byte *pic = NULL;
	//  byte *pic32 = NULL;
	int nTex = -1;
	qtexture_t *qtex;
	SkinInfo *pInfo;
	const char *pCleanName;

	int nSize = g_lstSkinCache.GetSize();
	pCleanName = QERApp_CleanTextureName( pName, false );
	for ( int i = 0; i < nSize; i++ )
	{
		SkinInfo *pInfo = reinterpret_cast<SkinInfo*>( g_lstSkinCache.GetAt( i ) );
		if ( pInfo ) {
			if ( stricmp( pCleanName, pInfo->m_strName ) == 0 ) {
				return pInfo->m_nTextureBind;
			}
		}
	}

	// if the load is successfull, we get back a qtexture_t
	// we don't need to free it, it's in g_qeglobals.d_qtextures
	// NOTE: we need to free the SkinInfo though..
	qtex = QERApp_Try_Texture_ForName( pCleanName );
	if ( qtex ) {
		nTex = qtex->texture_number;
		pInfo = new SkinInfo( qtex->name, nTex, qtex );
	}
	else
	{
		pInfo = new SkinInfo( pCleanName, -1, NULL );
	}
	g_lstSkinCache.Add( pInfo );

	return nTex;
}

bool TexWnd::CheckFilter( const char* name ){
	const char* buf = gtk_entry_get_text( GTK_ENTRY( m_pFilter ) );
	if ( strstr( name, buf ) != 0 ) {
		return true;
	}
	return false;
}

// =============================================================================
// static functions

static void vertical_scroll( GtkWidget *widget, gpointer data ){
	( (TexWnd*)data )->OnVScroll();
}

static void filter_changed( GtkWidget *widget, gpointer data ){
	CString str;
	str = gtk_entry_get_text( GTK_ENTRY( widget ) );
	( (TexWnd*)data )->UpdateFilter( str );
}

// =============================================================================
// TexWnd class

TexWnd::TexWnd()
	: GLWindow( FALSE ){
	m_pFilter = NULL;
	m_bNeedRange = true;
}

TexWnd::~TexWnd(){
}

void TexWnd::OnCreate(){
	if ( !MakeCurrent() ) {
		Error( "glMakeCurrent in TexWnd::OnCreate failed" );
	}

	g_qeglobals_gui.d_texture = m_pWidget;
	g_nTextureOffset = 0;

	GtkAdjustment *vadjustment = gtk_range_get_adjustment( GTK_RANGE( g_qeglobals_gui.d_texture_scroll ) );
	g_signal_connect( G_OBJECT( vadjustment ), "value-changed", G_CALLBACK( vertical_scroll ), this );

	if ( g_PrefsDlg.m_bTextureScrollbar ) {
		gtk_widget_show( g_qeglobals_gui.d_texture_scroll );
	} else {
		gtk_widget_hide( g_qeglobals_gui.d_texture_scroll );
	}
	m_bNeedRange = true;

	g_signal_connect( G_OBJECT( m_pFilter ), "changed", G_CALLBACK( filter_changed ), this );
	if ( g_PrefsDlg.m_bTextureWindow ) {
		gtk_widget_show( m_pFilter );
	}
}

void TexWnd::UpdateFilter( const char* pFilter ){
	g_bFilterEnabled = false;
	if ( pFilter ) {
		g_strFilter = pFilter;
		if ( g_strFilter.GetLength() > 0 ) {
			g_bFilterEnabled = true;
		}
		QERApp_SortActiveShaders();
	}
	Sys_UpdateWindows( W_TEXTURE );
}

void TexWnd::OnSize( int cx, int cy ){
	m_bNeedRange = true;
}

void TexWnd::OnExpose() {
	int nOld = g_qeglobals.d_texturewin.m_nTotalHeight;
	if ( !MakeCurrent() ) {
		Sys_FPrintf( SYS_ERR, "ERROR: glXMakeCurrent failed..\n " );
		Sys_Printf( "Please restart Radiant if the Texture view is not working\n" );
	}
	else
	{
		QE_CheckOpenGLForErrors();
		Texture_Draw( m_pWidget->allocation.width, m_pWidget->allocation.height - g_nTextureOffset );
		QE_CheckOpenGLForErrors();
		SwapBuffers();
	}
	if ( g_PrefsDlg.m_bTextureScrollbar && ( m_bNeedRange || g_qeglobals.d_texturewin.m_nTotalHeight != nOld ) ) {
		GtkAdjustment *vadjustment = gtk_range_get_adjustment( GTK_RANGE( g_qeglobals_gui.d_texture_scroll ) );

		gtk_adjustment_set_value( vadjustment, -g_qeglobals.d_texturewin.originy );
		gtk_adjustment_set_page_size( vadjustment, m_pWidget->allocation.height );
		gtk_adjustment_set_page_increment( vadjustment, m_pWidget->allocation.height / 2 );
		gtk_adjustment_set_step_increment( vadjustment, 20 );
		gtk_adjustment_set_lower( vadjustment, 0 );
		gtk_adjustment_set_upper( vadjustment, g_qeglobals.d_texturewin.m_nTotalHeight );

		g_signal_emit_by_name( G_OBJECT( vadjustment ), "changed" );

		m_bNeedRange = false;
	}
}

void TexWnd::OnLButtonDown( guint32 flags, int pointx, int pointy ){
	SetCapture();
	Texture_MouseDown( pointx, pointy - g_nTextureOffset, flags );
}

void TexWnd::OnRButtonDown( guint32 flags, int pointx, int pointy ){
	SetCapture();
	Texture_MouseDown( pointx, pointy - g_nTextureOffset, flags );
}

void TexWnd::OnMButtonDown( guint32 flags, int pointx, int pointy ){
	SetCapture();
	Texture_MouseDown( pointx, pointy - g_nTextureOffset, flags );
}

void TexWnd::OnLButtonUp( guint32 flags, int pointx, int pointy ){
	ReleaseCapture();
	DragDropTexture( flags, pointx, pointy );
}

void TexWnd::OnRButtonUp( guint32 flags, int pointx, int pointy ){
	ReleaseCapture();
}

void TexWnd::OnMButtonUp( guint32 flags, int pointx, int pointy ){
	ReleaseCapture();
}

void TexWnd::OnMouseMove( guint32 flags, int pointx, int pointy ){
	Texture_MouseMoved( pointx, pointy - g_nTextureOffset, flags );
	// if scrollbar is hidden, we don't seem to get an update
	if ( !g_PrefsDlg.m_bTextureScrollbar ) {
		RedrawWindow();
	}
}

void TexWnd::OnVScroll(){
	GtkAdjustment *vadjustment = gtk_range_get_adjustment( GTK_RANGE( g_qeglobals_gui.d_texture_scroll ) );

	g_qeglobals.d_texturewin.originy = -(int)gtk_adjustment_get_value( vadjustment );
	RedrawWindow();
}

void TexWnd::UpdatePrefs(){
	if ( g_PrefsDlg.m_bTextureWindow ) {
		gtk_widget_show( m_pFilter );
	}
	else{
		gtk_widget_hide( m_pFilter );
	}

	if ( g_PrefsDlg.m_bTextureScrollbar ) {
		gtk_widget_show( g_qeglobals_gui.d_texture_scroll );
	}
	else{
		gtk_widget_hide( g_qeglobals_gui.d_texture_scroll );
	}
	m_bNeedRange = true;
	RedrawWindow();
}

void TexWnd::FocusEdit() {
	if ( gtk_widget_get_visible( m_pFilter ) ) {
          gtk_window_set_focus( GTK_WINDOW( g_pParentWnd->m_pWidget ), m_pFilter );
	}
}

void TexWnd::OnMouseWheel( bool bUp, int pointx, int pointy ){
	if ( bUp ) {
		if ( g_qeglobals.d_texturewin.originy < 0 ) {
			g_qeglobals.d_texturewin.originy += g_PrefsDlg.m_nWheelInc;
			// clamp so we don't get jiggle if moved by less than scrollwheel increment
			if ( g_qeglobals.d_texturewin.originy > 0 ) {
				g_qeglobals.d_texturewin.originy = 0;
			}
		}
	}
	else
	{
		if ( g_qeglobals.d_texturewin.originy > ( -g_qeglobals.d_texturewin.m_nTotalHeight + g_qeglobals.d_texturewin.height ) ) {
			g_qeglobals.d_texturewin.originy -= g_PrefsDlg.m_nWheelInc;
		}
	}
	GtkAdjustment *vadjustment = gtk_range_get_adjustment( GTK_RANGE( g_qeglobals_gui.d_texture_scroll ) );
	gtk_adjustment_set_value( vadjustment, abs( g_qeglobals.d_texturewin.originy ) );

	RedrawWindow();
}

void TexWnd::DragDropTexture( guint32 flags, int pointx, int pointy ){
	// This gets called from leftmouse up event. We see if the mouseup is above
	// the camwindow. If this is the case do a trace for a surface. If we hit a
	// surface, texture it with the current texture.

	int m_ptXcheck, m_ptYcheck;
	int m_ptX, m_ptY;
	GtkWidget *widget;
	gint x, y;
	vec3_t dir;
	float f, r, u;
	int i;

	// we only want to catch a plain mouseevent
	if ( flags ) {
		return;
	}

	// see if we are above the camwindow
	Sys_GetCursorPos( &m_ptX, &m_ptY );

	if ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) {
		widget = g_pParentWnd->GetCamWnd()->m_pParent;
	}
	else{
		widget = g_pParentWnd->GetCamWnd()->GetWidget();
	}

	get_window_pos( widget, &x, &y );

	if ( m_ptX < x || m_ptY < y ||
		 m_ptX > x + widget->allocation.width ||
		 m_ptY > y + widget->allocation.height ) {
		return;
	}

	// check if the camwindow isn't being partially hidden by another window at this point
	m_ptXcheck = m_ptX;
	m_ptYcheck = m_ptY;

	if ( gtk_widget_get_window( g_pParentWnd->GetCamWnd()->GetWidget() ) != gdk_window_at_pointer( &m_ptXcheck, &m_ptYcheck ) ) {
		return;
	}

	// calc ray direction
	x = m_ptX - x;
	y = g_pParentWnd->GetCamWnd()->Camera()->height - 1 - ( m_ptY - y );
	u = (float)( y - ( g_pParentWnd->GetCamWnd()->Camera()->height * .5f ) ) / ( g_pParentWnd->GetCamWnd()->Camera()->height * .5f );
	r = (float)( x - ( g_pParentWnd->GetCamWnd()->Camera()->width * .5f ) ) / ( g_pParentWnd->GetCamWnd()->Camera()->width * .5f );
	f = 1;

	for ( i = 0 ; i < 3 ; i++ )
		dir[i] = g_pParentWnd->GetCamWnd()->Camera()->vpn[i] * f +
				 g_pParentWnd->GetCamWnd()->Camera()->vright[i] * r +
				 g_pParentWnd->GetCamWnd()->Camera()->vup[i] * u;
	VectorNormalize( dir, dir );

	// do a trace for a surface
	trace_t t;

	t = Test_Ray( g_pParentWnd->GetCamWnd()->Camera()->origin, dir, SF_SINGLEFACE );

	if ( t.brush ) {
		texdef_t tex;
		brushprimit_texdef_t brushprimit_tex;

		memset( &tex, 0, sizeof( tex ) );
		memset( &brushprimit_tex, 0, sizeof( brushprimit_tex ) );
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			// brushprimit fitted to a 2x2 texture
			brushprimit_tex.coords[0][0] = 1.0f;
			brushprimit_tex.coords[1][1] = 1.0f;
		}
		else
		{
			tex.scale[0] = g_PrefsDlg.m_fDefTextureScale;
			tex.scale[1] = g_PrefsDlg.m_fDefTextureScale;
		}
		tex.flags = g_qeglobals.d_texturewin.texdef.flags;
		tex.value = g_qeglobals.d_texturewin.texdef.value;
		tex.contents = g_qeglobals.d_texturewin.texdef.contents;
		// TTimo - shader code cleanup
		// texdef.name is the name of the shader, not the name of the actual texture file
		tex.SetName( g_qeglobals.d_texturewin.texdef.GetName() );

		Undo_Start( "set face textures" );
		Undo_AddBrush( t.brush );
		SetFaceTexdef( t.face, &tex, &brushprimit_tex, false, NULL );
		Brush_Build( t.brush, false );
		Undo_EndBrush( t.brush );
		Undo_End();

		Sys_UpdateWindows( W_CAMERA );
		g_pParentWnd->OnTimer();
	}
}
