/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

vec_t Min( vec_t a, vec_t b );

epair_t* GetNextChainItem( epair_t* lastItem, const char* key, const char* value );

// reads current texture into global, returns pointer to it
const char* GetCurrentTexture();

void FillDefaultTexture( _QERFaceData* faceData, vec3_t va, vec3_t vb, vec3_t vc, const char* texture );

void Sys_ERROR( const char* text, ... );

void BuildMiniPrt( list<Str>* exclusionList );

void MoveBlock( int dir, vec3_t min, vec3_t max, float dist );
void SetInitialStairPos( int dir, vec3_t min, vec3_t max, float width );

entity_s* FindEntityFromTargetname( const char* targetname, int* entNum );

char* UnixToDosPath( char* path );

char* GetFilename( char* buffer, const char* filename );
char* GetGameFilename( char* buffer, const char* filename );

float Determinant3x3( float a1, float a2, float a3,
					  float b1, float b2, float b3,
					  float c1, float c2, float c3 );

bool GetEntityCentre( const char* entity, vec3_t centre );
void MakeNormal( vec_t* va, vec_t* vb, vec_t* vc, vec_t* out );
