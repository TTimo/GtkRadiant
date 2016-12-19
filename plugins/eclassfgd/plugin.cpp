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

//#define FGD_VERBOSE // define this for extra info in the log.

#include "plugin.h"

/*! \file plugin.cpp
    \brief .fgd entity description format

    FGD loading code by Dominic Clifton - Hydra (Hydra@Hydras-World.com)

    Overview
    ========

    This module loads .fgd files, fgd files are split into classes:

      base classes
      point classes (aka fixed size entities)
      solid classes (aka brush entity)

    This program first scans each file, building up a list structures
    in memory that contain the information for all the classes found
    in the file.

    Then the program looks at each of the non-base classes in the list
    and build the eclass_t structure from each one.

    Classes can request information in other classes.

    Solid/Base and Point/Base classes can have the same names as other
    classes but there can be only one of each solid/point/base class with
    the same name,

    e.g.:

    this is NOT allowed:

      @solidclass = "a"
      @solidclass = "a"

    this is NOT allowed:

      @pointclass = "a"
      @solidclass = "a"

    this IS allowed:

      @solidclass = "a"
      @baseclass = "a"

    Version History
    ===============

    v0.1 - 13/March/2002
      - Initial version.

    v0.2 - 16/March/2002
      - sets e->skinpath when it finds an iconsprite(<filename>) token.

    v0.3 - 21/March/2002
      - Core now supports > 8 spawnflags, changes reflected here too.
      - FIXED: mins/maxs were backwards when only w,h,d were in the FGD
        (as opposed to the actual mins/maxs being in the .def file)
      - Made sure all PointClass entities were fixed size entities
        and gave them a default bounding box size if a size() setting
        was not in the FGD file.
      - Removed the string check for classes requesting another class
        with the same name, adjusted Find_Class() so that it can search
        for baseclasses only,  this fixes the problem with PointClass "light"
        requesting the BaseClass "light".

    v0.4 - 25/March/2002
      - bleh, It turns out that non-baseclasses can request non-baseclasses
        so now I've changed Find_Class() so that it can ignore a specific class
        (i.e. the one that's asking for others, so that classes can't request
        themselves but they can request other classes of any kind with the
        same name).
      - made all spawnflag comments appear in one place, rather than being scattered
        all over the comments if a requested class also had some spawnflags

    v0.5 - 6/April/2002
      - not using skinpath for sprites anymore, apprently we can code a model
        module to display sprites and model files.
      - model() tags are now supported.

    ToDo
    ====

 * add support for setting the eclass_t's modelpath.
      (not useful for CS, but very useful for HL).

 * need to implement usage for e->skinpath in the core.

 * cleanup some areas now that GetTokenExtra() is available
      (some parts were written prior to it's creation).

 * Import the comments between each BaseClass's main [ ] set.
      (unfortunatly they're // cstyle comments, which GetToken skips over)
      But still ignore comments OUTSIDE the main [ ] set.

 */

_QERScripLibTable g_ScripLibTable;
_EClassManagerTable g_EClassManagerTable;
_QERFuncTable_1 g_FuncTable;
_QERFileSystemTable g_FileSystemTable;

// forward declare
void Eclass_ScanFile( char *filename );

const char* EClass_GetExtension(){
	return "fgd";
}

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientFGD g_SynapseClient;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" CSynapseClient * SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces( const char *version, CSynapseServer *pServer ) {
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
	if ( strcmp( version, SYNAPSE_VERSION ) ) {
		Syn_Printf( "ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version );
		return NULL;
	}
	g_pSynapseServer = pServer;
	g_pSynapseServer->IncRef();
	Set_Syn_Printf( g_pSynapseServer->Get_Syn_Printf() );

	g_SynapseClient.AddAPI( ECLASS_MAJOR, "fgd", sizeof( _EClassTable ) );
	g_SynapseClient.AddAPI( SCRIPLIB_MAJOR, NULL, sizeof( g_ScripLibTable ), SYN_REQUIRE, &g_ScripLibTable );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( ECLASSMANAGER_MAJOR, NULL, sizeof( g_EClassManagerTable ), SYN_REQUIRE, &g_EClassManagerTable );

	// Needs a 'default' option for this minor because we certainly don't load anything from wad files :)
	g_SynapseClient.AddAPI( VFS_MAJOR, "*", sizeof( g_FileSystemTable ), SYN_REQUIRE, &g_FileSystemTable ); // wad, typically

	return &g_SynapseClient;
}

bool CSynapseClientFGD::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, ECLASS_MAJOR ) ) {
		_EClassTable* pTable = static_cast<_EClassTable*>( pAPI->mpTable );

		pTable->m_pfnGetExtension = &EClass_GetExtension;
		pTable->m_pfnScanFile = &Eclass_ScanFile;

		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientFGD::GetInfo(){
	return ".fgd eclass module built " __DATE__ " " RADIANT_VERSION;
}

// ------------------------------------------------------------------------------------------------

#define CLASS_NOCLASS     0
#define CLASS_BASECLASS   1
#define CLASS_POINTCLASS  2
#define CLASS_SOLIDCLASS  3

const char *classnames[] = {"NOT DEFINED","BaseClass","PointClass","SolidClass"};

#define OPTION_NOOPTION   0
#define OPTION_STRING     1
#define OPTION_CHOICES    2
#define OPTION_INTEGER    3
#define OPTION_FLAGS      4

const char *optionnames[] = {"NOT DEFINED","String","Choices","Integer","Flags"};

typedef struct choice_s {
	int value;
	char *name;
} choice_t;

typedef struct option_s {
	int optiontype;
	char *optioninfo;
	char *epairname;
	char *optiondefault;
	GSList *choices; // list of choices_t
} option_t;

typedef struct class_s {
	int classtype; // see CLASS_* above.
	char *classname;
	GSList *l_baselist; // when building the eclass_t, other class_s's with these names are required.
	char *description;

	GSList *l_optionlist; // when building the eclass_t, other class_s's with these names are required.

	bool gotsize; // if set then boundingbox is valid.
	vec3_t boundingbox[2]; // mins, maxs

	bool gotcolor; // if set then color is valid.
	vec3_t color; // R,G,B, loaded as 0-255

	char *model; // relative path + filename to a model (.spr/.mdl) file, or NULL
} class_t;

/*
   ===========================================================
   utility functions

   ===========================================================
 */
char *strlower( char *start ){
	char    *in;
	in = start;
	while ( *in )
	{
		*in = tolower( *in );
		in++;
	}
	return start;
}

char *addstr( char *dest,const char *source ){
	if ( dest ) {
		char *ptr;
		int len = strlen( dest );
		ptr = (char *) malloc( len + strlen( source ) + 1 );
		strcpy( ptr,dest );
		strcpy( ptr + len,source );
		free( dest );
		dest = ptr;
	}
	else
	{
		dest = strdup( source );
	}
	return( dest );
}

int getindex( unsigned int a ){
	unsigned int count = 0;
	unsigned int b = 0;
	for (; b != a; count++ )
	{
		b = ( 1 << count );
		if ( count > a ) {
			return -1;
		}
	}
	return ( count );
}

void ClearGSList( GSList* lst ){
	GSList *p = lst;
	while ( p )
	{
		free( p->data );
		p = g_slist_remove( p, p->data );
	}
}

/*!
   free a choice_t structure and it's contents
 */
void Free_Choice( choice_t *p ){
	if ( p->name ) {
		free( p->name );
	}
	free( p );

}

/*
   ===========================================================
   Main functions

   ===========================================================
 */

/*!
   free an option_t structure and it's contents
 */
void Free_Option( option_t *p ){
	if ( p->epairname ) {
		free( p->epairname );
	}
	if ( p->optiondefault ) {
		free( p->optiondefault );
	}
	if ( p->optioninfo ) {
		free( p->optioninfo );
	}
	GSList *l = p->choices;
	while ( l )
	{
		Free_Choice( (choice_t *)l->data );
		l = g_slist_remove( l, l->data );
	}
	free( p );
}

/*!
   free a class_t structure and it's contents
 */
void Free_Class( class_t *p ){
	GSList *l = p->l_optionlist;
	while ( l )
	{
		Free_Option( (option_t *)l->data );
		l = g_slist_remove( l, l->data );
	}

	if ( p->classname ) {
		free( p->classname );
	}
	free( p );
}

/*!
   find a class in the list
 */
class_t *Find_Class( GSList *l,char *classname, class_t *ignore ){
	for ( GSList *clst = l; clst != NULL; clst = clst->next )
	{
		class_t *c = (class_t *)clst->data;

		if ( c == ignore ) {
			continue;
		}

		// NOTE: to speed up we could make all the classnames lower-case when they're initialised.
		if ( !stricmp( c->classname,classname ) ) {
			return c;
		}

	}
	return NULL;
}

/*!
   Import as much as possible from a class_t into an eclass_t
   Note: this is somewhat recursive, as a class can require a class that requires a class and so on..
 */
void EClass_ImportFromClass( eclass_t *e, GSList *l_classes, class_t *bc ){
	char color[128];

	// We allocate 16k here, but only the memory actually used is kept allocated.
	// this is just used for building the final comments string.
	// Note: if the FGD file contains comments that are >16k (per entity) then
	// radiant will crash upon loading such a file as the eclass_t will become
	// corrupted.
	// FIXME: we could add some length checking when building "newcomments", but
	// that'd slow it down a bit.
	char newcomments[16384] = "";

	//Note: we override the values already in e.
	//and we do it in such a way that the items that appear last in the l_baselist
	//represent the final values.

	if ( bc->description ) {
		sprintf( newcomments,"%s\n",bc->description );
		e->comments = addstr( e->comments,newcomments );
		newcomments[0] = 0; // so we don't add them twice.
	}


	// import from other classes if required.

	if ( bc->l_baselist ) {
		// this class requires other base classes.

		for ( GSList *bclst = bc->l_baselist; bclst != NULL; bclst = bclst->next )
		{
			char *requestedclass = (char *)bclst->data;

//      class_t *rbc = Find_Class(l_classes, requestedclass, true);
			class_t *rbc = Find_Class( l_classes, requestedclass, bc );

			// make sure we don't request ourself!
			if ( rbc == bc ) {
				Sys_FPrintf( SYS_WRN, "WARNING: baseclass '%s' tried to request itself!\n", bclst->data );
			}
			else
			{
				if ( !rbc ) {
					Sys_FPrintf( SYS_WRN, "WARNING: could not find the requested baseclass '%s' when building '%s'\n", requestedclass,bc->classname );
				}
				else
				{
					// call ourself!
					EClass_ImportFromClass( e, l_classes, rbc );
				}
			}
		}
	}
	// SIZE
	if ( bc->gotsize ) {
		e->fixedsize = true;
		memcpy( e->mins,bc->boundingbox[0],sizeof( vec3_t ) );
		memcpy( e->maxs,bc->boundingbox[1],sizeof( vec3_t ) );
	}
/*
   // Hydra: apparently, this would be bad.

   if (bc->sprite)
   {
    // Hydra - NOTE: e->skinpath is not currently used by the editor but the code
    // to set it is used by both this eclass_t loader and the .DEF eclass_t loader.
    // TODO: implement using e->skinpath.
    if (e->skinpath)
      free (e->skinpath);

    e->skinpath = strdup(bc->sprite);
   }
 */

	// MODEL
	if ( bc->model ) {
		if ( e->modelpath ) {
			free( e->modelpath );
		}

		e->modelpath = strdup( bc->model );
	}

	// COLOR
	if ( bc->gotcolor ) {
		memcpy( e->color,bc->color,sizeof( vec3_t ) );
		sprintf( color, "(%f %f %f)", e->color[0], e->color[1], e->color[2] );
		e->texdef.SetName( color );
	}

	// SPAWNFLAGS and COMMENTS
	if ( bc->l_optionlist ) {
		for ( GSList *optlst = bc->l_optionlist; optlst != NULL; optlst = optlst->next )
		{
			option_t *opt = (option_t*) optlst->data;

			if ( opt->optiontype != OPTION_FLAGS ) {
				// add some info to the comments.
				if ( opt->optioninfo ) {
					sprintf( newcomments + strlen( newcomments ),"%s '%s' %s%s\n",
							 opt->epairname,
							 opt->optioninfo ? opt->optioninfo : "",
							 opt->optiondefault ? ", Default: " : "",
							 opt->optiondefault ? opt->optiondefault : "" );
				}
				else
				{
					sprintf( newcomments + strlen( newcomments ),"%s %s%s\n",
							 opt->epairname,
							 opt->optiondefault ? ", Default: " : "",
							 opt->optiondefault ? opt->optiondefault : "" );
				}
			}

			GSList *choicelst;
			switch ( opt->optiontype )
			{
			case OPTION_FLAGS:
				// grab the flags.
				for ( choicelst = opt->choices; choicelst != NULL; choicelst = choicelst->next )
				{
					choice_t *choice = (choice_t*) choicelst->data;

					int index = getindex( choice->value );
					index--;
					if ( index < MAX_FLAGS ) {
						strcpy( e->flagnames[index],choice->name );
					}
					else
					{
						Sys_FPrintf( SYS_WRN, "WARNING: baseclass '%s' has a spawnflag out of range, ignored!\n", bc->classname );
					}
				}
				break;
			case OPTION_CHOICES:
				strcat( newcomments,"  Choices:\n" );
				for ( choicelst = opt->choices; choicelst != NULL; choicelst = choicelst->next )
				{
					choice_t *choice = (choice_t*) choicelst->data;
					sprintf( newcomments + strlen( newcomments ),"  %5d - %s\n",choice->value,choice->name );
				}
				break;
			}
		}
	}

	// name
	if ( e->name ) {
		free( e->name );
	}
	e->name = strdup( bc->classname );

	// fixed size initialisation
	if ( bc->classtype == CLASS_POINTCLASS ) {
		e->fixedsize = true;
		// some point classes dont seem to have size()'s in the fgd, so set up a default here..
		if ( ( e->mins[0] == 0 ) && ( e->mins[1] == 0 ) && ( e->mins[2] == 0 ) &&
			 ( e->maxs[0] == 0 ) && ( e->maxs[1] == 0 ) && ( e->maxs[2] == 0 ) ) {
			e->mins[0] = -8;
			e->mins[1] = -8;
			e->mins[2] = -8;
			e->maxs[0] = 8;
			e->maxs[1] = 8;
			e->maxs[2] = 8;
		}

		if ( e->texdef.GetName() == NULL ) {
			// no color specified for this entity in the fgd file
			// set one now
			// Note: if this eclass_t is not fully built, then this may be
			// overridden with the correct color.

			e->color[0] = 1;
			e->color[1] = 0.5; // how about a nice bright pink, mmm, nice! :)
			e->color[2] = 1;

			sprintf( color, "(%f %f %f)", e->color[0], e->color[1], e->color[2] );
			e->texdef.SetName( color );
		}
	}

	// COMMENTS
	if ( newcomments[0] ) {
		e->comments = addstr( e->comments,newcomments );
	}
}

/*!
   create the eclass_t structures and add to the global list.
 */
void Create_EClasses( GSList *l_classes ){
	int count = 0;
	// loop through the loaded structures finding all the non BaseClasses
	for ( GSList *clst = l_classes; clst != NULL; clst = clst->next )
	{
		class_t *c = (class_t *)clst->data;

		if ( c->classtype == CLASS_BASECLASS ) { // not looking for these.
			continue;
		}

		eclass_t *e = (eclass_t *) malloc( sizeof( eclass_s ) );
		memset( e,0,sizeof( eclass_s ) );

		EClass_ImportFromClass( e, l_classes, c );

		// radiant will crash if this is null, and it still could be at this point.
		if ( !e->comments ) {
			e->comments = strdup( "No description available, check documentation\n" );
		}

		// dump the spawnflags to the end of the comments.
		int i;
		bool havespawnflags;

		havespawnflags = false;
		for ( i = 0 ; i < MAX_FLAGS ; i++ )
		{
			if ( *e->flagnames[i] ) {
				havespawnflags = true;
			}
		}

		if ( havespawnflags ) {
			char spawnline[80];
			e->comments = addstr( e->comments,"Spawnflags\n" );
			for ( i = 0 ; i < MAX_FLAGS ; i++ )
			{
				if ( *e->flagnames[i] ) {
					sprintf( spawnline,"  %d - %s\n", 1 << i, e->flagnames[i] );
					e->comments = addstr( e->comments,spawnline );
				}
			}
		}

		Eclass_InsertAlphabetized( e );
		count++;
		// Hydra: ttimo, I don't think this code is required...
		// single ?
		*Get_Eclass_E() = e;
		Set_Eclass_Found( true );
		if ( Get_Parsing_Single() ) {
			break;
		}
	}

	Sys_Printf( "FGD Loaded %d entities.\n", count );
}

void Eclass_ScanFile( char *filename ){
	int size;
	char    *data;
	char temp[1024];
	GSList *l_classes = NULL;
	char token_debug[1024];  //++Hydra FIXME: cleanup this.
	bool done = false;
	int len,classtype;

	char *token = Token();

	QE_ConvertDOSToUnixName( temp, filename );

	size = vfsLoadFullPathFile( filename, (void**)&data );
	if ( size <= 0 ) {
		Sys_FPrintf( SYS_ERR, "Eclass_ScanFile: %s not found\n", filename );
		return;
	}
	Sys_Printf( "ScanFile: %s\n", temp );

	// start parsing the file
	StartTokenParsing( data );

	// build a list of base classes first

	while ( !done )
	{
		// find an @ sign.
		do
		{
			if ( !GetToken( true ) ) {
				done = true;
				break;
			}
		} while ( token[0] != '@' );

		strcpy( temp,token + 1 ); // skip the @

		classtype = CLASS_NOCLASS;
		if ( !stricmp( temp,"BaseClass" ) ) {
			classtype = CLASS_BASECLASS;
		}
		if ( !stricmp( temp,"PointClass" ) ) {
			classtype = CLASS_POINTCLASS;
		}
		if ( !stricmp( temp,"SolidClass" ) ) {
			classtype = CLASS_SOLIDCLASS;
		}

		if ( classtype ) {
			class_t *newclass = (class_t *) malloc( sizeof( class_s ) );
			memset( newclass, 0, sizeof( class_s ) );
			newclass->classtype = classtype;

			while ( 1 )
			{
				GetTokenExtra( false,"(",false ); // option or =
				strcpy( token_debug,token );

				if ( !strcmp( token,"=" ) ) {
					UnGetToken();
					break;
				}
				else
				{
					strlower( token );
					if ( !strcmp( token,"base" ) ) {
						GetTokenExtra( false,"(",true ); // (

						if ( !strcmp( token,"(" ) ) {
							while ( GetTokenExtra( false,",)",false ) ) // option) or option,
							{
								newclass->l_baselist = g_slist_append( newclass->l_baselist, strdup( token ) );

								GetTokenExtra( false,",)",true ); // , or )
								if ( !strcmp( token,")" ) ) {
									break;
								}

							}
						}
					}
					else if ( !strcmp( token,"size" ) ) {
						// parse (w h d) or (x y z, x y z)

						GetTokenExtra( false,"(",true ); // (
						if ( !strcmp( token,"(" ) ) {
							int sizedone = false;
							float w,h,d;
							GetToken( false );
							w = atof( token );
							GetToken( false );
							h = atof( token );
							GetToken( false ); // number) or number ,
							strcpy( temp,token );
							len = strlen( temp );
							if ( temp[len - 1] == ')' ) {
								sizedone = true;
							}
							temp[len - 1] = 0;
							d = atof( temp );
							if ( sizedone ) {
								// only one set of cordinates supplied, change the W,H,D to mins/maxs
								newclass->boundingbox[0][0] = 0 - ( w / 2 );
								newclass->boundingbox[1][0] = w / 2;
								newclass->boundingbox[0][1] = 0 - ( h / 2 );
								newclass->boundingbox[1][1] = h / 2;
								newclass->boundingbox[0][2] = 0 - ( d / 2 );
								newclass->boundingbox[1][2] = d / 2;
								newclass->gotsize = true;
							}
							else
							{
								newclass->boundingbox[0][0] = w;
								newclass->boundingbox[0][1] = h;
								newclass->boundingbox[0][2] = d;
								GetToken( false );
								newclass->boundingbox[1][0] = atof( token );
								GetToken( false );
								newclass->boundingbox[1][1] = atof( token );
/*
                GetToken(false); // "number)" or "number )"
                strcpy(temp,token);
                len = strlen(temp);
                if (temp[len-1] == ')')
                  temp[len-1] = 0;
                else
                  GetToken(false); // )
                newclass->boundingbox[1][2] = atof(temp);
 */
								GetTokenExtra( false,")",false ); // number
								newclass->boundingbox[1][2] = atof( token );
								newclass->gotsize = true;
								GetTokenExtra( false,")",true ); // )
							}
						}
					}
					else if ( !strcmp( token,"color" ) ) {
						GetTokenExtra( false,"(",true ); // (
						if ( !strcmp( token,"(" ) ) {
							// get the color values (0-255) and normalize them if required.
							GetToken( false );
							newclass->color[0] = atof( token );
							if ( newclass->color[0] > 1 ) {
								newclass->color[0] /= 255;
							}
							GetToken( false );
							newclass->color[1] = atof( token );
							if ( newclass->color[1] > 1 ) {
								newclass->color[1] /= 255;
							}
							GetToken( false );
							strcpy( temp,token );
							len = strlen( temp );
							if ( temp[len - 1] == ')' ) {
								temp[len - 1] = 0;
							}
							newclass->color[2] = atof( temp );
							if ( newclass->color[2] > 1 ) {
								newclass->color[2] /= 255;
							}
							newclass->gotcolor = true;
						}
					}
					else if ( !strcmp( token,"iconsprite" ) ) {
						GetTokenExtra( false,"(",true ); // (
						if ( !strcmp( token,"(" ) ) {
							GetTokenExtra( false,")",false ); // filename)
							// the model plugins will handle sprites too.
							// newclass->sprite = strdup(token);
							newclass->model = strdup( token );
							GetTokenExtra( false,")",true ); // )
						}
					}
					else if ( !strcmp( token,"model" ) ) {
						GetTokenExtra( false,"(",true ); // (
						if ( !strcmp( token,"(" ) ) {
							GetTokenExtra( false,")",false ); // filename)
							newclass->model = strdup( token );
							GetTokenExtra( false,")",true ); // )
						}
					}
					else
					{
						// Unsupported
						GetToken( false ); // skip it.
					}

				}
			}

			GetToken( false ); // =
			strcpy( token_debug,token );
			if ( !strcmp( token,"=" ) ) {
				GetToken( false );
				newclass->classname = strdup( token );
			}

			// Get the description
			if ( newclass->classtype != CLASS_BASECLASS ) {
				GetToken( false );
				if ( !strcmp( token,":" ) ) {
					GetToken( false );
					newclass->description = strdup( token );
				}
				else{ UnGetToken(); // no description
				}
			}

			// now build the option list.
			GetToken( true ); // [ or []

			if ( strcmp( token,"[]" ) ) { // got some options ?
				if ( !strcmp( token,"[" ) ) {
					// yup
					bool optioncomplete = false;
					option_t *newoption;

					while ( 1 )
					{
						GetToken( true );
						if ( !strcmp( token,"]" ) ) {
							break; // no more options

						}
						// parse the data and build the option_t

						strcpy( temp,token );
						len = strlen( temp );
						char *ptr = strchr( temp,'(' );

						if ( !ptr ) {
							break;
						}

						newoption = (option_t *) malloc( sizeof( option_s ) );
						memset( newoption, 0, sizeof( option_s ) );

						*ptr++ = 0;
						newoption->epairname = strdup( temp );

						len = strlen( ptr );
						if ( ptr[len - 1] != ')' ) {
							break;
						}

						ptr[len - 1] = 0;
						strlower( ptr );
						if ( !strcmp( ptr,"integer" ) ) {
							newoption->optiontype = OPTION_INTEGER;
						}
						else if ( !strcmp( ptr,"choices" ) ) {
							newoption->optiontype = OPTION_CHOICES;
						}
						else if ( !strcmp( ptr,"flags" ) ) {
							newoption->optiontype = OPTION_FLAGS;
						}
						else // string
						{
							newoption->optiontype = OPTION_STRING;
						}

						switch ( newoption->optiontype )
						{
						case OPTION_STRING:
						case OPTION_INTEGER:
							if ( !TokenAvailable() ) {
								optioncomplete = true;
								break;
							}
							GetToken( false ); // :
							strcpy( token_debug,token );
							if ( ( token[0] == ':' ) && ( strlen( token ) > 1 ) ) {
								newoption->optioninfo = strdup( token + 1 );
							}
							else
							{
								GetToken( false );
								newoption->optioninfo = strdup( token );
							}
							if ( TokenAvailable() ) { // default value ?
								GetToken( false );
								if ( !strcmp( token,":" ) ) {
									if ( GetToken( false ) ) {
										newoption->optiondefault = strdup( token );
										optioncomplete = true;
									}
								}
							}
							else
							{
								optioncomplete = true;
							}
							break;

						case OPTION_CHOICES:
							GetTokenExtra( false,":",true ); // : or :"something like this" (bah!)
							strcpy( token_debug,token );
							if ( ( token[0] == ':' ) && ( strlen( token ) > 1 ) ) {
								if ( token[1] == '\"' ) {
									strcpy( temp,token + 2 );
									while ( 1 )
									{
										if ( !GetToken( false ) ) {
											break;
										}
										strcat( temp," " );
										strcat( temp,token );
										len = strlen( temp );
										if ( temp[len - 1] == '\"' ) {
											temp[len - 1] = 0;
											break;
										}
									}
								}
								newoption->optioninfo = strdup( temp );
							}
							else
							{
								GetToken( false );
								newoption->optioninfo = strdup( token );
							}
							GetToken( false ); // : or =
							strcpy( token_debug,token );
							if ( !strcmp( token,":" ) ) {
								GetToken( false );
								newoption->optiondefault = strdup( token );
							}
							else
							{
								UnGetToken();
							}
						// And Follow on...
						case OPTION_FLAGS:
							GetToken( false ); // : or =
							strcpy( token_debug,token );
							if ( strcmp( token,"=" ) ) { // missing ?
								break;
							}

							GetToken( true ); // [
							strcpy( token_debug,token );
							if ( strcmp( token,"[" ) ) { // missing ?
								break;
							}

							choice_t *newchoice;
							while ( 1 )
							{
								GetTokenExtra( true,":",true ); // "]" or "number", or "number:"
								strcpy( token_debug,token );
								if ( !strcmp( token,"]" ) ) { // no more ?
									optioncomplete = true;
									break;
								}
								strcpy( temp,token );
								len = strlen( temp );
								if ( temp[len - 1] == ':' ) {
									temp[len - 1] = 0;
								}
								else
								{
									GetToken( false ); // :
									if ( strcmp( token,":" ) ) { // missing ?
										break;
									}
								}
								if ( !TokenAvailable() ) {
									break;
								}
								GetToken( false ); // the name

								newchoice = (choice_t *) malloc( sizeof( choice_s ) );
								memset( newchoice, 0, sizeof( choice_s ) );

								newchoice->value = atoi( temp );
								newchoice->name = strdup( token );

								newoption->choices = g_slist_append( newoption->choices, newchoice );

								// ignore any remaining tokens on the line
								while ( TokenAvailable() ) GetToken( false );

								// and it we found a "]" on the end of the line, put it back in the queue.
								if ( !strcmp( token,"]" ) ) {
									UnGetToken();
								}
							}
							break;

						}

						// add option to the newclass

						if ( optioncomplete ) {
							if ( newoption ) {
								// add it to the list.
								newclass->l_optionlist = g_slist_append( newclass->l_optionlist, newoption );
							}
						}
						else
						{
							Sys_FPrintf( SYS_WRN, "WARNING: Parse error occured in '%s - %s'\n",classnames[newclass->classtype],newclass->classname );
							Free_Option( newoption );
						}

					}
				}
				else
				{
					UnGetToken(); // shouldn't get here.
				}
			}

			// add it to our list.
			l_classes = g_slist_append( l_classes, newclass );

		}
	}

	// finished with the file now.
	g_free( data );

	Sys_Printf( "FGD scan complete, building entities...\n" );

	// Once we get here we should have a few (!) lists in memory that we
	// can extract all the information required to build a the eclass_t structures.

	Create_EClasses( l_classes );

	// Free everything

	GSList *p = l_classes;
	while ( p )
	{
		class_t *tmpclass = (class_t *)p->data;

#ifdef FGD_VERBOSE
		// DEBUG: dump the info...
		Sys_Printf( "%s: %s (", classnames[tmpclass->classtype],tmpclass->classname );
		for ( GSList *tmp = tmpclass->l_baselist; tmp != NULL; tmp = tmp->next )
		{
			if ( tmp != tmpclass->l_baselist ) {
				Sys_Printf( ", " );
			}
			Sys_Printf( "%s", (char *)tmp->data );
		}
		if ( tmpclass->gotsize ) {
			sprintf( temp,"(%.0f %.0f %.0f) - (%.0f %.0f %.0f)",tmpclass->boundingbox[0][0],
					 tmpclass->boundingbox[0][1],
					 tmpclass->boundingbox[0][2],
					 tmpclass->boundingbox[1][0],
					 tmpclass->boundingbox[1][1],
					 tmpclass->boundingbox[1][2] );
		}
		else{ strcpy( temp,"No Size" ); }
		Sys_Printf( ") '%s' Size: %s",tmpclass->description ? tmpclass->description : "No description",temp );
		if ( tmpclass->gotcolor ) {
			sprintf( temp,"(%d %d %d)",tmpclass->color[0],
					 tmpclass->color[1],
					 tmpclass->color[2] );
		}
		else{ strcpy( temp,"No Color" ); }
		Sys_Printf( " Color: %s Options:\n",temp );
		if ( !tmpclass->l_optionlist ) {
			Sys_Printf( "  No Options\n" );
		}
		else
		{
			option_t *tmpoption;
			int count;
			GSList *olst;
			for ( olst = tmpclass->l_optionlist, count = 1; olst != NULL; olst = olst->next, count++ )
			{
				tmpoption = (option_t *)olst->data;
				Sys_Printf( "  %d, Type: %s, EPair: %s\n", count,optionnames[tmpoption->optiontype], tmpoption->epairname );

				choice_t *tmpchoice;
				GSList *clst;
				int ccount;
				for ( clst = tmpoption->choices, ccount = 1; clst != NULL; clst = clst->next, ccount++ )
				{
					tmpchoice = (choice_t *)clst->data;
					Sys_Printf( "    %d, Value: %d, Name: %s\n", ccount, tmpchoice->value, tmpchoice->name );
				}
			}
		}

#endif

		// free the baselist.
		ClearGSList( tmpclass->l_baselist );
		Free_Class( tmpclass );
		p = g_slist_remove( p, p->data );
	}

}
