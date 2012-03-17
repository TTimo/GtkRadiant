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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// internal implementation of IShaders related stuff
//

#ifndef __SHADERS_H_
#define __SHADERS_H_

//++timo TODO: track all the calls to IncRef and look for not-called DecRef bugs
//++timo TODO: move all needed stuff into the IShader interface

// Radiant's internal implementation of the IShader object
class CShader : public IShader
{
int refCount;
qtexture_t *m_pTexture;
// name is shader / texture name (if not a real shader) reletive to "textures/" directory
char m_Name[QER_MAX_NAMELEN];
char m_ShaderFileName[QER_MAX_NAMELEN];
int m_nFlags;
float m_fTrans;
// the name of the texture file to be used to represent the shader
//++timo FIXME?
// must fit the qtexture_t convention or not? I think so ..
// ((old .. NOTE: may be a straight copy of the .shader file, doesn't fit the qtexture_t naming requirements))
CString m_strTextureName;
bool m_bDisplayed;
bool m_bInUse;
// color stuff
bool m_bColor;
vec3_t m_vColor;
// alphafunc stuff
int m_nAlphaFunc;
float m_fAlphaRef;
// cull stuff
int m_nCull;

// will hook itself in g_ActiveShaders and increment ref count
// will also update the underlying qtexture_t with some information about the shader name etc.
void RegisterActivate();

public:
CShader() { refCount = 0; m_pTexture = NULL; m_Name[0] = '\0'; m_ShaderFileName[0] = '\0'; m_nFlags = 0; m_bInUse = false; m_bDisplayed = false; m_bColor = false; m_fTrans = 1.0f; m_nAlphaFunc = 0; m_fAlphaRef = 0.f; m_nCull = 0; }
virtual ~CShader() { }

// IShaders implementation -----------------
// Increment the number of references to this object
void IncRef() { refCount++; }
// Decrement the reference count
void DecRef(){
	if ( --refCount <= 0 ) {
		delete this;
	}
}
// get/set the qtexture_t* Radiant uses to represent this shader object
qtexture_t* getTexture() const { return m_pTexture; }
void setTexture( qtexture_t *pTex ) { m_pTexture = pTex; }
// get shader name
const char* getName() const { return m_Name; }
bool IsDisplayed() const { return m_bDisplayed; }
void SetDisplayed( bool b ) { m_bDisplayed = b; }
// setting in use also sets the display flag on
bool IsInUse() const { return m_bInUse; }
void SetInUse( bool b ) {
	m_bInUse = b; if ( m_pTexture ) {
		m_pTexture->inuse = true;
	}
	if ( b ) {
		m_bDisplayed = true;
	}
}
// get the shader flags
int getFlags() { return m_nFlags; }
// get the transparency value
float getTrans() { return m_fTrans; }
// test if it's a true shader, or a default shader created to wrap around a texture
bool IsDefault() { return m_ShaderFileName[0] == '\0'; }
// test if it's a plain color shader, i.e. a shader we use on plain color stuff (like info_playerstart)
bool IsColor() { return m_bColor; }
// get the related color then!
void getColor( vec3_t v ) { VectorCopy( m_vColor, v ); }
// get the alphaFunc
void getAlphaFunc( int *func, float *ref ) { *func = m_nAlphaFunc; *ref = m_fAlphaRef; };
// get the cull type
int getCull() { return m_nCull; };
// get/set shader file name (ie the file where this one is defined)
const char* getShaderFileName() const { return m_ShaderFileName; }
// -----------------------------------------

// parse yourself!
bool Parse();

// search / load the texture to be used when displaying the shader
// after a successfull call to one of these the shader will get displayed in the tex wnd
// if m_strTextureName could not be loaded will set m_pTexture to NULL
void Try_Activate();
// if m_strTextureName could not be loaded will use a default qtexture
// FIXME TTimo: Activate forces activation, always true
bool Activate();

// set shader name
void setName( const char* name ) { strcpy( m_Name, name ); }
void setShaderFileName( const char* name ) { strcpy( m_ShaderFileName, name ); }
// create a default shader for a given texture name
// will not activate!
// NOTE: CreateDefault expects a texture name reletive to the base path. Adding a "textures/" may be needed
void CreateDefault( const char* name );
const char* getTextureName() { return m_strTextureName; }

//++timo clean
// color stuff
//	void setColor( vec3_t c ) { VectorCopy( c, m_vColor ); m_bColor = true; }
// create a color shader
void CreateColor( const char* name );
};

// the classical CPtrArray with some enhancements
class CShaderArray : public CPtrArray
{
public:
CShaderArray() { }
virtual ~CShaderArray() { }
// look for a shader with a given name (may return NULL)
CShader* Shader_ForName( const char * ) const;
// look for a shader with a given texture name (may return NULL)
// NOTE: the texture name is supposed to fit qtexture_t naming conventions .. _DEBUG builds will check
CShader* Shader_ForTextureName( const char * ) const;
// will Add the given object if not already in
void AddSingle( void* );
// will copy / add another CShaderArray, and IncRef
void operator =( const class CShaderArray & );
// will empty the array, decreasing the refcount by 1
void ReleaseAll();
// will empty all shaders that match a given filename, decreasing the refcount by 1
void ReleaseForShaderFile( const char * );
// sort the array by shader name
void SortShaders();
// set the IsDisplayed flag for all shaders stored
void SetDisplayed( bool b );
// set the InUse flag for all shaders stored
void SetInUse( bool b );
};

#endif
