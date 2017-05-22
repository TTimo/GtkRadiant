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

#include "cpicomodel.h"
#include "qertypes.h"

#include <map>
#include <vector>

#define RADIANT_ASSERT( condition, message ) if ( !( condition ) ) { Sys_Printf( "ASSERTION FAILURE: " message "\n" ); } else

template<class key_type, class value_type>
class cache_element
{
public:
inline cache_element() : m_count( 0 ), m_value( NULL ) {}
inline ~cache_element(){
	RADIANT_ASSERT( m_count == 0, "destroyed a reference before it was released\n" );
	if ( m_count > 0 ) {
		destroy();
	}
}
inline value_type* capture( const key_type& key ){
	if ( ++m_count == 1 ) {
		construct( key );
	}
	return m_value;
}
inline void release(){
	RADIANT_ASSERT( !empty(), "failed to release reference - not found in cache\n" );
	if ( --m_count == 0 ) {
		destroy();
	}
}
inline bool empty(){
	return m_count == 0;
}
inline void refresh( const key_type& key ){
	m_value->refresh( key );
}
private:
inline void construct( const key_type& key ){
	m_value = new value_type( key );
}
inline void destroy(){
	delete m_value;
}

unsigned int m_count;
value_type* m_value;
};

class ModelCache
{
typedef CPicoModel value_type;

public:
typedef PicoModelKey key_type;
typedef cache_element<key_type, value_type> elem_type;
typedef map<key_type, elem_type> cache_type;

value_type* capture( const key_type& key ){
	return m_cache[key].capture( key );
}
void release( const key_type& key ){
	m_cache[key].release();
}

private:
cache_type m_cache;
};

ModelCache g_model_cache;



typedef struct remap_s {
	char m_remapbuff[64 + 1024];
	char *original;
	char *remap;
} remap_t;

class RemapWrapper : public IRender, public ISelect
{
unsigned int m_refcount;
public:
RemapWrapper( entity_interfaces_t* model, const char* name )
	: m_refcount( 1 ){
	parse_namestr( name );

	m_model = g_model_cache.capture( ModelCache::key_type( m_name.GetBuffer(), m_frame ) );

	model->pRender = this;
	model->pRender->IncRef();
	model->pEdit = NULL;
	model->pSelect = this;
	model->pSelect->IncRef();

	construct_shaders();
}
virtual ~RemapWrapper(){
	g_model_cache.release( ModelCache::key_type( m_name.GetBuffer(), m_frame ) );

	for ( shaders_t::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i ) {
		( *i )->DecRef();
	}

	for ( remaps_t::iterator j = m_remaps.begin(); j != m_remaps.end(); ++j )
	{
		remap_t *pRemap = ( *j );
		delete pRemap;
	}
	m_remaps.clear();
}
virtual void IncRef(){
	++m_refcount;
}
virtual void DecRef(){
	if ( --m_refcount == 0 ) {
		delete this;
	}
}
virtual void Draw( int state, int rflags ) const {
	m_model->Draw( state, m_shaders, rflags );
}
virtual const bool IsModelNotNull() const {
	return m_model != NULL;
}
virtual const aabb_t *GetAABB() const {
	return m_model->GetAABB();
}
virtual bool TestRay( const ray_t *ray, vec_t *dist ) const {
	return m_model->TestRay( ray, dist );
}
private:
void add_remap( const char *remap ){
	const char *ch;
	remap_t *pRemap;

	ch = remap;

	while ( *ch && *ch != ';' )
		ch++;

	if ( *ch == '\0' ) {
		// bad remap
		Sys_FPrintf( SYS_WRN, "WARNING: Shader _remap key found in a model entity without a ; character\n" );
	}
	else {
		pRemap = new remap_t;

		strncpy( pRemap->m_remapbuff, remap, sizeof( pRemap->m_remapbuff ) );

		pRemap->m_remapbuff[ch - remap] = '\0';

		pRemap->original = pRemap->m_remapbuff;
		pRemap->remap = pRemap->m_remapbuff + ( ch - remap ) + 1;

		m_remaps.push_back( pRemap );
	}
}

void parse_namestr( const char *name ){
	const char *ptr, *s;
	char buf[1024];
	bool hasName, hasFrame;

	hasName = hasFrame = false;

	for ( s = ptr = name; *ptr; ptr++ ) {
		if ( !hasName && *ptr == ':' ) {
			// model name
			hasName = true;
			strncpy( buf, s, ptr - s );
			buf[ptr - s] = '\0';
			m_name = buf;
			s = ptr + 1;
		}
		else if ( *ptr == '?' ) {
			// model frame
			hasFrame = true;
			strncpy( buf, s, ptr - s );
			buf[ptr - s] = '\0';
			m_frame = atoi( buf );
			s = ptr + 1;
		}
		else if ( *ptr == '&' ) {
			// a remap
			strncpy( buf, s, ptr - s );
			buf[ptr - s] = '\0';
			add_remap( buf );
			s = ptr + 1;
		}
	}

	if ( !hasFrame ) {
		// model frame
		strncpy( buf, s, ptr - s );
		buf[ptr - s] = '\0';
		m_frame = atoi( buf );
	}
	else {
		// a remap
		strncpy( buf, s, ptr - s );
		buf[ptr - s] = '\0';
		add_remap( buf );
	}
}

void construct_shaders(){
	IShader* global_shader = shader_for_remap( "*" );

	unsigned int numSurfaces = m_model->GetNumSurfaces();
	m_shaders.reserve( numSurfaces );
	// now go through our surface and find our shaders, remap if needed
	for ( unsigned int j = 0; j < numSurfaces; j++ )
	{
		const char* surfShaderName = m_model->GetShaderNameForSurface( j );
		IShader* shader = shader_for_remap( surfShaderName );
		// Determine which shader it is going to be
		if ( !shader ) {
			if ( global_shader ) {
				shader = global_shader;
			}
			else {
				shader = QERApp_Shader_ForName( surfShaderName );
			}
		}
		// Add reference
		shader->IncRef();
		// Done, continue
		m_shaders.push_back( shader );
	}
}

inline IShader* shader_for_remap( const char* remap ){
	remap_t *pRemap;
	remaps_t::iterator i;
	for ( i = m_remaps.begin(); i != m_remaps.end(); ++i )
	{
		pRemap = ( *i );
		if ( stricmp( remap, pRemap->original ) == 0 ) {
			break;
		}
	}
	return ( i != m_remaps.end() ) ? QERApp_Shader_ForName( pRemap->remap ) : NULL;
}

Str m_name;
int m_frame;
CPicoModel* m_model;

typedef vector<remap_t *> remaps_t;
remaps_t m_remaps;
typedef vector<IShader*> shaders_t;
shaders_t m_shaders;
};

class ModelWrapper : public IRender, public ISelect
{
unsigned int m_refcount;
public:
ModelWrapper( entity_interfaces_t* model, const char* name )
	: m_refcount( 1 ), m_name( name ){
	m_model = g_model_cache.capture( ModelCache::key_type( m_name.GetBuffer(), 0 ) );

	model->pRender = this;
	model->pRender->IncRef();
	model->pEdit = NULL;
	model->pSelect = this;
	model->pSelect->IncRef();
}
virtual ~ModelWrapper(){
	g_model_cache.release( ModelCache::key_type( m_name.GetBuffer(), 0 ) );
}

virtual void IncRef(){
	++m_refcount;
}
virtual void DecRef(){
	if ( --m_refcount == 0 ) {
		delete this;
	}
}
virtual void Draw( int state, int rflags ) const {
	m_model->Draw( state, rflags );
}
virtual const bool IsModelNotNull() const {
	return m_model != NULL;
}
virtual const aabb_t *GetAABB() const {
	return m_model->GetAABB();
}
virtual bool TestRay( const ray_t *ray, vec_t *dist ) const {
	return m_model->TestRay( ray, dist );
}

Str m_name;
CPicoModel* m_model;
};

void LoadModel( entity_interfaces_t* model, const char* name ){
	if ( strchr( name, ':' ) != NULL || strchr( name, '?' ) != NULL || strchr( name, '&' ) != NULL ) {
		RemapWrapper* wrapper = new RemapWrapper( model, name );
		wrapper->DecRef();
	}
	else
	{
		ModelWrapper* wrapper = new ModelWrapper( model, name );
		wrapper->DecRef();
	}
}
