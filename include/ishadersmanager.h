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

#ifndef _ISHADERSMANAGER_H_
#define _ISHADERSMANAGER_H_

class IShadersManager
{
public:
IShadersManager ();
virtual ~IShadersManager ();

// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;

// free all shaders
// free the shaders, will not free the qtexture_t*
virtual void FreeShaders() = 0;

// reload all the shaders
// this will free everything (shaders and their textures), then reload all in use stuff
virtual void ReloadShaders() = 0;

// load all shaders in a given directory
// this will scan the list of in-memory shaders, and load the related qtexture_t if needed
virtual void LoadShadersFromDir( const char* path ) = 0;

// load a shader file (ie a set of shaders)
// after LoadShaderFile shaders will be in memory, next step is to load the qtexture_t Radiant uses
// to represent them if a shader with the same name exists, new one will not be loaded
// don't use this to refresh the shaders!
virtual void LoadShaderFile( const char* filename ) = 0;

// tell if a given shader exists in our shader table
// NOTE: this doesn't tell wether it's corresponding qtexture is loaded
virtual int HasShader( const char* name ) = 0;

// return the shader for a given name
// if the qtexture is not already in memory, will try loading it
// if the qtexture could not be found, will use default
// will return NULL on shader not found
virtual IShader* Try_Shader_ForName( const char* name ) = 0;

// return the shader for a given name
// if the qtexture is not already in memory, will try loading it
// will create a default shader if not found (will use a default texture)
virtual IShader* Shader_ForName( const char* name ) = 0;

// query / load a texture
// will not try loading a shader, will look for the actual image file ..
// returns NULL on file not found
// NOTE: strategy for file lookup:
//   paths must be relative, ie. textures/me/myfile
//   if a 3-letters filename extension (such as .jpg or .tga) is provided, it will get loaded first
//   if not found or no extension, will try loading after adding .tga and .jpg (in this order)
virtual qtexture_t* Try_Texture_ForName( const char* filename ) = 0;

// query / load a texture
// will not try loading a shader, will look for the actual image file ..
// on file not found will use the "texture not found"
virtual qtexture_t* Texture_ForName( const char* filename ) = 0;

// get the number of active shaders
// these are the shaders currently loaded, that have an associated qtexture_t*
virtual int GetActiveShaderCount() = 0;

// for stuff that needs to be represented by a plain texture
// the shader will get a "color" name, use GetColor to get the actual color
virtual IShader* ColorShader_ForName( const char* name ) = 0;

// reload a shaderfile - update shaders and their display properties/qtexture_t if needed
// will not reload the texture files
// will switch to "show in use" atfer use
// filename must be reletive path of the shader, ex. scripts/gothic_wall.shader
virtual void ReloadShaderFile( const char* filename ) = 0;

// retrieve a shader if exists, without loading the textures for it etc.
// use this function if you want special info on a shader
virtual IShader* Shader_ForName_NoLoad( const char* name ) = 0;
};

#endif // _ISHADERSMANAGER_H_
