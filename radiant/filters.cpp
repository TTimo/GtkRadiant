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

#include "stdafx.h"

/*
    Rambetter on Sun Jan 23, 2011:

    What follows is a discussion of the introduction to some new members and functions
    such as baseFilters, baseFilterExcludes, FilterAddImpl(), and so on.

    There was a problem that "base" filters were not taking effect as a result of
    user action such as clicking a filter checkbox.  The reason why the base filters
    were not being updated is because the previous logic of updating the filters had
    been commented out.  It was commented out because the previous logic deleted ALL
    filters (including plugin filters) and re-created only the base filters.  So in
    effect, all plugin filters would be deleted whenever a filter checkbox was altered.

    So, we need some way of knowing which are the base filters so that we can take
    action when a filter checkbox is marked [or unmarked].  Then we can update
    the base filters by either deleting and recreating them, or by changing their
    active state in an intelligent manner.  I am choosing to do the latter.

    My goal here is to preserve the structure bfilter_t completely as it has been
    historically.  I feel that modifying bfilter_t is dangerous with respect to breaking
    plugins.  So what I'm doing instead is tracking which are the base filters via
    external array, baseFilters.  The array baseFilterExcludes keeps track of the "exclude"
    parameter for the corresponding filter.
 */

#define MAX_BASE_FILTERS 32
static bfilter_t   *baseFilters[MAX_BASE_FILTERS];
static int baseFilterExcludes[MAX_BASE_FILTERS];
static int numBaseFilters = 0;

// This shall not be called from outside filters.cpp.
bfilter_t *FilterAddImpl( bfilter_t *pFilter, int type, int bmask, const char *str, int exclude, bool baseFilter ){
	bfilter_t *pNew = new bfilter_t;
	pNew->next = pFilter;
	pNew->attribute = type;
	if ( type == 1 || type == 3 ) {
		pNew->string = str;
	}
	if ( type == 2 || type == 4 || type == 5 || type == 6 || type == 7 ) {
		pNew->mask = bmask;
	}
	if ( g_qeglobals.d_savedinfo.exclude & exclude ) {
		pNew->active = true;
	}
	else{
		pNew->active = false;
	}
	if ( baseFilter ) {
		if ( numBaseFilters >= MAX_BASE_FILTERS ) {
			Error( "Base filters count exceeds MAX_BASE_FILTERS" );
		}
		baseFilters[numBaseFilters] = pNew;
		baseFilterExcludes[numBaseFilters] = exclude;
		numBaseFilters++;
	}
	return pNew;
}

// type 1 = texture filter (name)
// type 3 = entity filter (name)
// type 2 = QER_* shader flags
// type 4 = entity classes
// type 5 = surface flags (q2)
// type 6 = content flags (q2)
// type 7 = content flags - no match (q2)
bfilter_t *FilterAdd( bfilter_t *pFilter, int type, int bmask, const char *str, int exclude ){
	return FilterAddImpl( pFilter, type, bmask, str, exclude, false );
}

bfilter_t *FilterCreate( int type, int bmask, const char *str, int exclude ){
	g_qeglobals.d_savedinfo.filters = FilterAdd( g_qeglobals.d_savedinfo.filters, type, bmask, str, exclude );
	Syn_Printf( "Added filter %s (type: %i, bmask: %i, exclude: %i)\n", str, type, bmask, exclude );
	return g_qeglobals.d_savedinfo.filters;
}

extern void PerformFiltering();

void FiltersActivate( void ){
	PerformFiltering();
	Sys_UpdateWindows( W_XY | W_CAMERA );
}

// removes the filter list at *pFilter, returns NULL pointer
// IMPORTANT NOTE: Some plugins add filters, and those will be removed here as well.
// Therefore, this function should rarely be used.
bfilter_t *FilterListDelete( bfilter_t *pFilter ){
	if ( pFilter != NULL ) {
		FilterListDelete( pFilter->next );
		delete pFilter;
	}
	else
	{
		memset( baseFilters, 0, sizeof( baseFilters ) ); // Strictly speaking we don't really need this.
		memset( baseFilterExcludes, 0, sizeof( baseFilterExcludes ) ); // Strictly speaking we don't really need this.
		numBaseFilters = 0; // We do need this.
	}
	return NULL;
}


bfilter_t *FilterAddBase( bfilter_t *pFilter ){
	pFilter = FilterAddImpl( pFilter,1,0,"clip",EXCLUDE_CLIP,true );
	pFilter = FilterAddImpl( pFilter,1,0,"caulk",EXCLUDE_CAULK,true );
	pFilter = FilterAddImpl( pFilter,1,0,"liquids",EXCLUDE_LIQUIDS,true );
	pFilter = FilterAddImpl( pFilter,1,0,"hint",EXCLUDE_HINTSSKIPS,true );
	pFilter = FilterAddImpl( pFilter,1,0,"clusterportal",EXCLUDE_CLUSTERPORTALS,true );
	pFilter = FilterAddImpl( pFilter,1,0,"areaportal",EXCLUDE_AREAPORTALS,true );
	pFilter = FilterAddImpl( pFilter,2,QER_TRANS,NULL,EXCLUDE_TRANSLUCENT,true );
	pFilter = FilterAddImpl( pFilter,3,0,"trigger",EXCLUDE_TRIGGERS,true );
	pFilter = FilterAddImpl( pFilter,4,ECLASS_MISCMODEL,NULL,EXCLUDE_MODELS,true );
	pFilter = FilterAddImpl( pFilter,4,ECLASS_LIGHT,NULL,EXCLUDE_LIGHTS,true );
	pFilter = FilterAddImpl( pFilter,4,ECLASS_PATH,NULL,EXCLUDE_PATHS,true );
	pFilter = FilterAddImpl( pFilter,1,0,"lightgrid",EXCLUDE_LIGHTGRID,true );
	pFilter = FilterAddImpl( pFilter,1,0,"botclip",EXCLUDE_BOTCLIP,true );
	pFilter = FilterAddImpl( pFilter,1,0,"clipmonster",EXCLUDE_BOTCLIP,true );
	return pFilter;
}

void FilterUpdateBase(){
	int i;
	bfilter_t   *filter;
	int exclude;
	for ( i = 0; i < numBaseFilters; i++ )
	{
		filter = baseFilters[i];
		exclude = baseFilterExcludes[i];
		if ( g_qeglobals.d_savedinfo.exclude & exclude ) {
			filter->active = true;
		}
		else{
			filter->active = false;
		}
	}
}

/*
   ==================
   FilterBrush
   ==================
 */

bool FilterBrush( brush_t *pb ){

	if ( !pb->owner ) {
		return FALSE;       // during construction

	}
	if ( pb->hiddenBrush ) {
		return TRUE;
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD ) {
		if ( strcmp( pb->owner->eclass->name, "worldspawn" ) == 0 || !strcmp( pb->owner->eclass->name,"func_group" ) ) { // hack, treating func_group as world
			return TRUE;
		}
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT ) {
		if ( strcmp( pb->owner->eclass->name, "worldspawn" ) != 0 && strcmp( pb->owner->eclass->name,"func_group" ) ) { // hack, treating func_group as world
			return TRUE;
		}
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CURVES ) {
		if ( pb->patchBrush ) {
			return TRUE;
		}
	}


	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_DETAILS ) {
		if ( !pb->patchBrush && pb->brush_faces->texdef.contents & CONTENTS_DETAIL ) {
			return TRUE;
		}
	}
	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_STRUCTURAL ) {
		if ( !pb->patchBrush && !( pb->brush_faces->texdef.contents & CONTENTS_DETAIL ) ) {
			return TRUE;
		}
	}

	// if brush belongs to world entity or a brushmodel entity and is not a patch
	if ( ( strcmp( pb->owner->eclass->name, "worldspawn" ) == 0
		   || !strncmp( pb->owner->eclass->name, "func", 4 )
		   || !strncmp( pb->owner->eclass->name, "trigger", 7 ) ) && !pb->patchBrush ) {
		bool filterbrush = false;
		for ( face_t *f = pb->brush_faces; f != NULL; f = f->next )
		{
			filterbrush = false;
			for ( bfilter_t *filters = g_qeglobals.d_savedinfo.filters;
				  filters != NULL;
				  filters = filters->next )
			{
				if ( !filters->active ) {
					continue;
				}
				// exclude by attribute 1 brush->face->pShader->getName()
				if ( filters->attribute == 1 ) {
					if ( strstr( f->pShader->getName(),filters->string ) ) {
						filterbrush = true;
						break;
					}
				}
				// exclude by attribute 2 brush->face->pShader->getFlags()
				else if ( filters->attribute == 2 ) {
					if ( f->pShader->getFlags() & filters->mask ) {
						filterbrush = true;
						break;
					}
					// quake2 - 5 == surface flags, 6 == content flags
				}
				else if ( filters->attribute == 5 ) {
					if ( f->texdef.flags && f->texdef.flags & filters->mask ) {
						filterbrush = true;
						break;
					}
				}
				else if ( filters->attribute == 6 ) {
					if ( f->texdef.contents && f->texdef.contents & filters->mask ) {
						filterbrush = true;
						break;
					}
				}
				else if ( filters->attribute == 7 ) {
					if ( f->texdef.contents && !( f->texdef.contents & filters->mask ) ) {
						filterbrush = true;
						break;
					}
				}
			}
			if ( !filterbrush ) {
				break;
			}
		}
		if ( filterbrush ) { // if no face is found that should not be excluded
			return true; // exclude this brush
		}
	}

	// if brush is a patch
	if ( pb->patchBrush ) {
		bool drawpatch = true;
		for ( bfilter_t *filters = g_qeglobals.d_savedinfo.filters;
			  filters != NULL;
			  filters = filters->next )
		{
			// exclude by attribute 1 (for patch) brush->pPatch->pShader->getName()
			if ( filters->active
				 && filters->attribute == 1 ) {
				if ( strstr( pb->pPatch->pShader->getName(),filters->string ) ) {
					drawpatch = false;
					break;
				}
			}

			// exclude by attribute 2 (for patch) brush->pPatch->pShader->getFlags()
			if ( filters->active
				 && filters->attribute == 2 ) {
				if ( pb->pPatch->pShader->getFlags() & filters->mask ) {
					drawpatch = false;
					break;
				}
			}
		}
		if ( !drawpatch ) { // if a shader is found that should be excluded
			return TRUE; // exclude this patch
		}
	}

	if ( strcmp( pb->owner->eclass->name, "worldspawn" ) != 0 ) { // if brush does not belong to world entity
		bool drawentity = true;
		for ( bfilter_t *filters = g_qeglobals.d_savedinfo.filters;
			  filters != NULL;
			  filters = filters->next )
		{
			// exclude by attribute 3 brush->owner->eclass->name
			if ( filters->active
				 && filters->attribute == 3 ) {
				if ( strstr( pb->owner->eclass->name,filters->string ) ) {
					drawentity = false;
					break;
				}
			}

			// exclude by attribute 4 brush->owner->eclass->nShowFlags
			else if ( filters->active
					  && filters->attribute == 4 ) {
				if ( pb->owner->eclass->nShowFlags & filters->mask ) {
					drawentity = false;
					break;
				}
			}
		}
		if ( !drawentity ) { // if an eclass property is found that should be excluded
			return TRUE; // exclude this brush
		}
	}
	return FALSE;
}
