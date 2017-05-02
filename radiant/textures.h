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


// a texturename of the form (0 0 0) will
// create a solid color texture

void Texture_Init();
void    Texture_ShowDirectory( int menunum );
void Texture_ShowDirectory();
void Texture_ShowDirectory_by_path( const char* pPath );
void    Texture_ShowAll();
void WINAPI Texture_ShowInuse();
extern char texture_directory[];

// Timo
// added an optional IPluginTexdef when one is available
// we need a forward declaration, this is crap
class IPluginTexdef;
//++timo clean
void    Texture_SetTexture2( IShader *pShader, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, IPluginTexdef *pTexdef = NULL, bool bSetSelection = true );
void WINAPI Texture_SetTexture( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, IPluginTexdef *pTexdef = (IPluginTexdef*)NULL, bool bSetSelection = true );

void    Texture_SetMode( int iMenu ); // GL_TEXTURE_NEAREST, etc..
void Texture_ResetPosition();

// build the list of shader files used by PreloadShaders
void BuildShaderList();
// preload the shaders: build a list of shader names and properties .. don't load their assets
void PreloadShaders();
int WINAPI Texture_LoadSkin( char *pName, int *pnWidth, int *pnHeight );
qtexture_t* Texture_LoadFromPlugIn( void* vp );
void Texture_StartPos( void );
IShader* Texture_NextPos( int *x, int *y );
