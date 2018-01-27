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

//-----------------------------------------------------------------------------
//
//
// DESCRIPTION:
// a set of functions to manipulate textures in Radiant
//

#ifndef __ISHADERS_H_
#define __ISHADERS_H_

#define SHADERS_MAJOR "shaders"
// define a GUID for this interface so plugins can access and reference it
// {D42F798A-DF57-11d3-A3EE-0004AC96D4C3}
static const GUID QERShadersTable_GUID =
{ 0xd42f798a, 0xdf57, 0x11d3, { 0xa3, 0xee, 0x0, 0x4, 0xac, 0x96, 0xd4, 0xc3 } };

// NOTES ABOUT SYNTAX:
// if a function starts by 'Try' it means that if the requested thing could not be found / loaded it will return nothing / NULL
// otherwise a default object will be created
// the _QERShadersTable is also used by shader code inside Radiant. but for speed and "keep it simple" consideration you
// can get the static equivalent of the func pointers by adding 'QERApp_' (access to _QERShadersTable is better thought ..
// see the note to move all the shader language out of Radiant below)

/*!
   \todo FIXME TTimo
   fix the reference count strategy
   - define the policy. It seems the initial policy of doing an inc ref when you create the shader is not good
   (it doesn't work, and it's not being used right)
   so, when you request an IShader and store it, incref it yourself
   as a debugging safe check: push the created increfed objects into a list, and scan them at next idle loop
   to make sure they have been decref'ed ? (sounds easy, may not be that much).
 */

class IShader
{
public:
virtual ~IShader() { }
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
// get/set the qtexture_t* Radiant uses to represent this shader object
virtual qtexture_t* getTexture() const = 0;
virtual void setTexture( qtexture_t *pTex ) = 0;
// get shader name
virtual const char* getName() const = 0;
// is this shader in use?
// NOTE: this flag can mean this shader has been in use at least once since the last rescan of in-use stuff
// (rescan of in-use happens in several cases, user command or during a texture directory load)
// NOTE: this is used to draw the green outline in the texture window
// NOTE: when does Radiant set the InUse flag? Whenever Select_SetTexture is called (well that doesn't necessarily means the texture actually gets in use, but that's close enough)
virtual bool IsInUse() const = 0;
virtual void SetInUse( bool ) = 0;
// is this shader displayed in the texture browser?
// NOTE: if IsInUse() == true, the shader will always be displayed in the texture window and this flag ingored
virtual bool IsDisplayed() const = 0;
virtual void SetDisplayed( bool ) = 0;
// get the editor flags (QER_NOCARVE QER_TRANS)
virtual int getFlags() = 0;
// get the transparency value
virtual float getTrans() = 0;
// test if it's a true shader, or a default shader created to wrap around a texture
virtual bool IsDefault() = 0;
// test if it's a plain color shader, i.e. a shader we use on plain color stuff (like info_playerstart)
virtual bool IsColor() = 0;
// get the related color then!
virtual void getColor( vec3_t v ) = 0;
// get the alphaFunc
virtual void getAlphaFunc( int *func, float *ref ) = 0;
// get the cull type
virtual int getCull() = 0;
// get shader file name (ie the file where this one is defined)
virtual const char* getShaderFileName() const = 0;
};

// NOTE: how to move all the shader language out of Radiant in a plugin?
// -> change this _QERShadersTable into an IShadersManager
// -> let the plugin create an instance of IShadersManager
// -> make sure Radiant uses this IShadersManager to load / query the shaders

// NOTE: shader and texture names used must be full path, ie. most often with "textures/" prefix
// (since shaders are defined in .shader files with textures/)

// free all shaders
// free the shaders, will not free the qtexture_t*
typedef void ( WINAPI * PFN_FREESHADERS )();
// reload all the shaders
// this will free everything (shaders and their textures), then reload all in use stuff
typedef void ( WINAPI * PFN_RELOADSHADERS )();
// load all shaders in a given directory
// this will scan the list of in-memory shaders, and load the related qtexture_t if needed
typedef int ( WINAPI * PFN_LOADSHADERSFROMDIR )( const char* path );
// count all shaders in a given directory
// this will scan the list of in-memory shaders
typedef bool ( WINAPI * PFN_ISDIRCONTAININGSHADER )( const char* path );
// load a shader file (ie a set of shaders)
// after LoadShaderFile shaders will be in memory, next step is to load the qtexture_t Radiant uses to represent them
// if a shader with the same name exists, new one will not be loaded - don't use this to refresh the shaders!
typedef void ( WINAPI * PFN_LOADSHADERFILE )( const char* filename );
// tell if a given shader exists in our shader table
// NOTE: this doesn't tell wether it's corresponding qtexture is loaded
typedef int ( WINAPI * PFN_HASSHADER )( const char* name );
// return the shader for a given name
// if the qtexture is not already in memory, will try loading it
// if the qtexture could not be found, will use default
// will return NULL on shader not found
typedef IShader*    ( WINAPI * PFN_TRYSHADERFORNAME )( const char* name );
// return the shader for a given name
// if the qtexture is not already in memory, will try loading it
// will create a default shader if not found (will use a default texture)
typedef IShader*    ( WINAPI * PFN_SHADERFORNAME )( const char* name );
// query / load a texture
// will not try loading a shader, will look for the actual image file ..
// returns NULL on file not found
// NOTE: strategy for file lookup:
//   paths must be relative, ie. textures/me/myfile
//   if a 3-letters filename extension (such as .jpg or .tga) is provided, it will get loaded first
//   if not found or no extension, will try loading after adding .tga and .jpg (in this order)
typedef qtexture_t* ( WINAPI * PFN_TRYTEXTUREFORNAME )( const char* filename );
// query / load a texture
// will not try loading a shader, will look for the actual image file ..
// on file not found will use the "texture not found"
typedef qtexture_t* ( WINAPI * PFN_TEXTUREFORNAME )( const char* filename );
// get the number of active shaders
// these are the shaders currently loaded, that have an associated qtexture_t*
typedef int ( WINAPI * PFN_GETACTIVESHADERCOUNT )();
// for stuff that needs to be represented by a plain texture
// the shader will get a "color" name, use GetColor to get the actual color
typedef IShader*    ( WINAPI * PFN_COLORSHADERFORNAME )( const char* name );
// reload a shaderfile - update shaders and their display properties/qtexture_t if needed
// will not reload the texture files
// will switch to "show in use" atfer use
// filename must be reletive path of the shader, ex. scripts/gothic_wall.shader
typedef void ( WINAPI * PFN_RELOADSHADERFILE )( const char* filename );
// retrieve a shader if exists, without loading the textures for it etc.
// use this function if you want special info on a shader
typedef IShader* ( WINAPI * PFN_SHADERFORNAMENOLOAD )( const char* name );
// force the "in use" flag on all active shaders
typedef void ( WINAPI * PFN_ACTIVESHADERSSETINUSE )( bool b );
// sort the shaders in alphabetical order, we use the order in the texture inspector
typedef void ( WINAPI * PFN_SORTACTIVESHADERS )();
// check if there exists an active shader with the given texture name (loaded or not, doesn't matter)
// (used to detect the textures we need to create a default shader for .. while scanning a directory)
typedef IShader* ( WINAPI * PFN_ACTIVESHADERFORTEXTURENAME )( char * );
// create a shader to wrap around a texture name, we use this when loading a texture directory and some textures
// are not present as shaders
typedef IShader* ( WINAPI * PFN_CREATESHADERFORTEXTURENAME )( const char* name );
// switch the IsDisplayed flag on all the active shaders
typedef void ( WINAPI * PFN_ACTIVESHADERSSETDISPLAYED )( bool b );
// retrieve an active shader based on index
typedef IShader* ( WINAPI * PFN_ACTIVESHADERFORINDEX )( int i );
// will cleanup a texture name and force it to the right format
// the debug version is painfully slow, but will detect more problems
// the idea being to avoid loading the same file several time because of uppercase/lowercase etc.
typedef const char* ( WINAPI * PFN_CLEANTEXTURENAME )( const char* name, bool bAddTexture );

struct _QERShadersTable
{
	int m_nSize;
	PFN_FREESHADERS m_pfnFreeShaders;
	PFN_RELOADSHADERS m_pfnReloadShaders;
	PFN_LOADSHADERSFROMDIR m_pfnLoadShadersFromDir;
	PFN_ISDIRCONTAININGSHADER m_pfnIsDirContainingShaders;
	PFN_LOADSHADERFILE m_pfnLoadShaderFile;
	PFN_RELOADSHADERFILE m_pfnReloadShaderFile;
	PFN_HASSHADER m_pfnHasShader;
	PFN_TRYSHADERFORNAME m_pfnTry_Shader_ForName;
	PFN_SHADERFORNAME m_pfnShader_ForName;
	PFN_TRYTEXTUREFORNAME m_pfnTry_Texture_ForName;
	PFN_TEXTUREFORNAME m_pfnTexture_ForName;
	PFN_GETACTIVESHADERCOUNT m_pfnGetActiveShaderCount;
	PFN_COLORSHADERFORNAME m_pfnColorShader_ForName;
	PFN_SHADERFORNAMENOLOAD m_pfnShader_ForName_NoLoad;
	PFN_ACTIVESHADERSSETINUSE m_pfnActiveShaders_SetInUse;
	PFN_SORTACTIVESHADERS m_pfnSortActiveShaders;
	PFN_ACTIVESHADERFORTEXTURENAME m_pfnActiveShader_ForTextureName;
	PFN_CREATESHADERFORTEXTURENAME m_pfnCreateShader_ForTextureName;
	PFN_ACTIVESHADERSSETDISPLAYED m_pfnActiveShaders_SetDisplayed;
	PFN_ACTIVESHADERFORINDEX m_pfnActiveShader_ForIndex;
	PFN_CLEANTEXTURENAME m_pfnCleanTextureName;
};

/*!
   \todo FIXME fix the QERApp_ prototyping on shaders module
   make it homogeneous with other modules, should be straight calls
 */

#ifdef USE_SHADERSTABLE_DEFINE
  #ifndef __SHADERSTABLENAME
	#define __SHADERSTABLENAME g_ShadersTable
  #endif
#define QERApp_Shader_ForName __SHADERSTABLENAME.m_pfnShader_ForName
#define QERApp_Texture_ForName2 __SHADERSTABLENAME.m_pfnTexture_ForName
#define QERApp_FreeShaders __SHADERSTABLENAME.m_pfnFreeShaders
#define QERApp_ReloadShaders __SHADERSTABLENAME.m_pfnReloadShaders
#define QERApp_SortActiveShaders __SHADERSTABLENAME.m_pfnSortActiveShaders
#define QERApp_ReloadShaderFile __SHADERSTABLENAME.m_pfnReloadShaderFile
#define QERApp_LoadShaderFile __SHADERSTABLENAME.m_pfnLoadShaderFile
#define QERApp_HasShader __SHADERSTABLENAME.m_pfnHasShader
#define QERApp_Try_Shader_ForName __SHADERSTABLENAME.m_pfnTry_Shader_ForName
#define QERApp_Try_Texture_ForName __SHADERSTABLENAME.m_pfnTry_Texture_ForName
#define QERApp_ColorShader_ForName __SHADERSTABLENAME.m_pfnColorShader_ForName
#define QERApp_Shader_ForName_NoLoad __SHADERSTABLENAME.m_pfnShader_ForName_NoLoad
#define QERApp_LoadShadersFromDir __SHADERSTABLENAME.m_pfnLoadShadersFromDir
#define QERApp_IsDirContainingShaders __SHADERSTABLENAME.m_pfnIsDirContainingShaders
#define QERApp_CreateShader_ForTextureName __SHADERSTABLENAME.m_pfnCreateShader_ForTextureName
#define QERApp_GetActiveShaderCount __SHADERSTABLENAME.m_pfnGetActiveShaderCount
#define QERApp_ActiveShaders_SetDisplayed __SHADERSTABLENAME.m_pfnActiveShaders_SetDisplayed
#define QERApp_ActiveShader_ForIndex __SHADERSTABLENAME.m_pfnActiveShader_ForIndex
#define QERApp_ActiveShaders_SetInUse __SHADERSTABLENAME.m_pfnActiveShaders_SetInUse
#define QERApp_ActiveShader_ForTextureName __SHADERSTABLENAME.m_pfnActiveShader_ForTextureName
#define QERApp_ActiveShader_ForIndex __SHADERSTABLENAME.m_pfnActiveShader_ForIndex
#define QERApp_CleanTextureName __SHADERSTABLENAME.m_pfnCleanTextureName
#endif

#define APPSHADERS_MAJOR "appshaders"
// FIXME: remove
static const GUID QERAppShadersTable_GUID =
{ 0xec3008a8, 0xbd0b, 0x11d4, { 0x82, 0x51, 0x20, 0x4c, 0x4f, 0x4f, 0x50, 0x20 } };

// g_qeglobals.d_qtextures is used internally by the editor for actual camera drawing
typedef qtexture_t** ( WINAPI * PFN_QTEXTURES )();
// g_qeglobals.d_qtexmap is a map for fast access
typedef GHashTable* ( WINAPI * PFN_QTEXMAP )();
// d_texturewin
//++timo NOTE: this same function is also in isurface.h table, we would eventually have to merge some stuff
typedef texturewin_t* ( *PFN_QEGLOBALSTEXTUREWIN )();
// Texture_SetTexture
//++timo NOTE: this one may have to be reorganized too .. putting it here is a bit clumsy
// NOTE: the C++ function used internally has a lot of default values
typedef void ( WINAPI * PFN_TEXTURESETTEXTURE )( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef );
// Texture_ShowInuse
typedef void ( WINAPI * PFN_TEXTURESHOWINUSE )();
// BuildShaderList
typedef void ( *PFN_BUILDSHADERLIST )();
// PreloadShaders
typedef void ( *PFN_PRELOADSHADERS )();

// a table that Radiant makes available to the shader module in return
struct _QERAppShadersTable
{
	int m_nSize;
	PFN_QTEXTURES m_pfnQTextures;
	PFN_QTEXMAP m_pfnQTexmap;
	PFN_QEGLOBALSTEXTUREWIN m_pfnQeglobalsTexturewin;
	PFN_TEXTURESETTEXTURE m_pfnTexture_SetTexture;
	PFN_TEXTURESHOWINUSE m_pfnTexture_ShowInuse;
	PFN_BUILDSHADERLIST m_pfnBuildShaderList;
	PFN_PRELOADSHADERS m_pfnPreloadShaders;
};

#ifdef USE_APPSHADERSTABLE_DEFINE
  #ifndef __APPSHADERTABLENAME
	#define __APPSHADERTABLENAME g_AppShadersTable
  #endif
#define Texture_ShowInuse __APPSHADERTABLENAME.m_pfnTexture_ShowInuse
#endif

/*!
   NOTE TTimo: there is an important distinction between SHADER_NOT_FOUND and SHADER_NOTEX:
   SHADER_NOT_FOUND means we didn't find the raw texture or the shader for this
   SHADER_NOTEX means we recognize this as a shader script, but we are missing the texture to represent it
   this was in the initial design of the shader code since early GtkRadiant alpha, and got sort of foxed in 1.2 and put back in
 */
#define SHADER_NOT_FOUND "textures/radiant/notex"
#define SHADER_NOTEX "textures/radiant/shadernotex" ///< Q3 tech specific

#endif
