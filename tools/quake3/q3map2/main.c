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

-------------------------------------------------------------------------------

This code has been altered significantly from its original form, to support
several games based on the Quake III Arena engine, in the form of "Q3Map2."

------------------------------------------------------------------------------- */



/* marker */
#define MAIN_C



/* dependencies */
#include "q3map2.h"



/*
Random()
returns a pseudorandom number between 0 and 1
*/

vec_t Random( void )
{
	return (vec_t) rand() / RAND_MAX;
}



/*
ExitQ3Map()
cleanup routine
*/

static void ExitQ3Map( void )
{
	BSPFilesCleanup();
	if( mapDrawSurfs != NULL )
		free( mapDrawSurfs );
}



/*
BSPInfo()
emits statistics about the bsp file
*/

int BSPInfo( int count, char **fileNames )
{
	int			i;
	char		source[ 1024 ], ext[ 64 ];
	int			size;
	FILE		*f;
	
	
	/* dummy check */
	if( count < 1 )
	{
		Sys_Printf( "No files to dump info for.\n");
		return -1;
	}
	
	/* enable info mode */
	infoMode = qtrue;
	
	/* walk file list */
	for( i = 0; i < count; i++ )
	{
		Sys_Printf( "---------------------------------\n" );
		
		/* mangle filename and get size */
		strcpy( source, fileNames[ i ] );
		ExtractFileExtension( source, ext );
		if( !Q_stricmp( ext, "map" ) )
			StripExtension( source );
		DefaultExtension( source, ".bsp" );
		f = fopen( source, "rb" );
		if( f )
		{
			size = Q_filelength (f);
			fclose( f );
		}
		else
			size = 0;
		
		/* load the bsp file and print lump sizes */
		Sys_Printf( "%s\n", source );
		LoadBSPFile( source );		
		PrintBSPFileSizes();
		
		/* print sizes */
		Sys_Printf( "\n" );
		Sys_Printf( "          total         %9d\n", size );
		Sys_Printf( "                        %9d KB\n", size / 1024 );
		Sys_Printf( "                        %9d MB\n", size / (1024 * 1024) );
		
		Sys_Printf( "---------------------------------\n" );
	}
	
	/* return count */
	return i;
}



/*
ScaleBSPMain()
amaze and confuse your enemies with wierd scaled maps!
*/

int ScaleBSPMain( int argc, char **argv )
{
	int			i;
	float		f, scale;
	vec3_t		vec;
	char		str[ 1024 ];
	
	
	/* arg checking */
	if( argc < 2 )
	{
		Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
		return 0;
	}
	
	/* get scale */
	scale = atof( argv[ argc - 2 ] );
	if( scale == 0.0f )
	{
		Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
		Sys_Printf( "Non-zero scale value required.\n" );
		return 0;
	}
	
	/* do some path mangling */
	strcpy( source, ExpandArg( argv[ argc - 1 ] ) );
	StripExtension( source );
	DefaultExtension( source, ".bsp" );
	
	/* load the bsp */
	Sys_Printf( "Loading %s\n", source );
	LoadBSPFile( source );
	ParseEntities();
	
	/* note it */
	Sys_Printf( "--- ScaleBSP ---\n" );
	Sys_FPrintf( SYS_VRB, "%9d entities\n", numEntities );
	
	/* scale entity keys */
	for( i = 0; i < numBSPEntities && i < numEntities; i++ )
	{
		/* scale origin */
		GetVectorForKey( &entities[ i ], "origin", vec );
		if( (vec[ 0 ] + vec[ 1 ] + vec[ 2 ]) )
		{
			VectorScale( vec, scale, vec );
			sprintf( str, "%f %f %f", vec[ 0 ], vec[ 1 ], vec[ 2 ] );
			SetKeyValue( &entities[ i ], "origin", str );
		}
		
		/* scale door lip */
		f = FloatForKey( &entities[ i ], "lip" );
		if( f )
		{
			f *= scale;
			sprintf( str, "%f", f );
			SetKeyValue( &entities[ i ], "lip", str );
		}
	}
	
	/* scale models */
	for( i = 0; i < numBSPModels; i++ )
	{
		VectorScale( bspModels[ i ].mins, scale, bspModels[ i ].mins );
		VectorScale( bspModels[ i ].maxs, scale, bspModels[ i ].maxs );
	}
	
	/* scale nodes */
	for( i = 0; i < numBSPNodes; i++ )
	{
		VectorScale( bspNodes[ i ].mins, scale, bspNodes[ i ].mins );
		VectorScale( bspNodes[ i ].maxs, scale, bspNodes[ i ].maxs );
	}
	
	/* scale leafs */
	for( i = 0; i < numBSPLeafs; i++ )
	{
		VectorScale( bspLeafs[ i ].mins, scale, bspLeafs[ i ].mins );
		VectorScale( bspLeafs[ i ].maxs, scale, bspLeafs[ i ].maxs );
	}
	
	/* scale drawverts */
	for( i = 0; i < numBSPDrawVerts; i++ )
		VectorScale( bspDrawVerts[ i ].xyz, scale, bspDrawVerts[ i ].xyz );
	
	/* scale planes */
	for( i = 0; i < numBSPPlanes; i++ )
		bspPlanes[ i ].dist *= scale;
	
	/* scale gridsize */
	GetVectorForKey( &entities[ 0 ], "gridsize", vec );
	if( (vec[ 0 ] + vec[ 1 ] + vec[ 2 ]) == 0.0f )
		VectorCopy( gridSize, vec );
	VectorScale( vec, scale, vec );
	sprintf( str, "%f %f %f", vec[ 0 ], vec[ 1 ], vec[ 2 ] );
	SetKeyValue( &entities[ 0 ], "gridsize", str );
	
	/* write the bsp */
	UnparseEntities();
	StripExtension( source );
	DefaultExtension( source, "_s.bsp" );
	Sys_Printf( "Writing %s\n", source );
	WriteBSPFile( source );
	
	/* return to sender */
	return 0;
}



/*
ConvertBSPMain()
main argument processing function for bsp conversion
*/

int ConvertBSPMain( int argc, char **argv )
{
	int		i;
	int		(*convertFunc)( char * );
	
	
	/* set default */
	convertFunc = ConvertBSPToASE;
	
	/* arg checking */
	if( argc < 1 )
	{
		Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
		return 0;
	}
	
	/* process arguments */
	for( i = 1; i < (argc - 1); i++ )
	{
		/* -format map|ase|... */
		if( !strcmp( argv[ i ],  "-format" ) )
 		{
			i++;
			if( !Q_stricmp( argv[ i ], "ase" ) )
				convertFunc = ConvertBSPToASE;
			else if( !Q_stricmp( argv[ i ], "map" ) )
				convertFunc = ConvertBSPToMap;
			else
				Sys_Printf( "Unknown conversion format \"%s\". Defaulting to ASE.\n", argv[ i ] );
 		}
	}
	
	/* clean up map name */
	strcpy( source, ExpandArg( argv[ i ] ) );
	StripExtension( source );
	DefaultExtension( source, ".bsp" );
	
	LoadShaderInfo();
	
	Sys_Printf( "Loading %s\n", source );
	
	/* ydnar: load surface file */
	//%	LoadSurfaceExtraFile( source );
	
	LoadBSPFile( source );
	
	/* parse bsp entities */
	ParseEntities();
	
	/* convert */
	return convertFunc( source );
}



/*
main()
q3map mojo...
*/

int main( int argc, char **argv )
{
	int		i, r;
	double	start, end;
	
	
	/* we want consistent 'randomness' */
	srand( 0 );
	
	/* start timer */
	start = I_FloatTime();

	/* this was changed to emit version number over the network */
	printf( Q3MAP_VERSION "\n" );
	
	/* set exit call */
	atexit( ExitQ3Map );
	
	/* read general options first */
	for( i = 1; i < argc; i++ )
	{
		/* -connect */
		if( !strcmp( argv[ i ], "-connect" ) )
		{
			argv[ i ] = NULL;
			i++;
			Broadcast_Setup( argv[ i ] );
			argv[ i ] = NULL;
		}
		
		/* verbose */
		else if( !strcmp( argv[ i ], "-v" ) )
		{
			verbose = qtrue;
			argv[ i ] = NULL;
		}
		
		/* force */
		else if( !strcmp( argv[ i ], "-force" ) )
		{
			force = qtrue;
			argv[ i ] = NULL;
		}
		
		/* patch subdivisions */
		else if( !strcmp( argv[ i ], "-subdivisions" ) )
		{
			argv[ i ] = NULL;
			i++;
			patchSubdivisions = atoi( argv[ i ] );
			argv[ i ] = NULL;
			if( patchSubdivisions <= 0 )
				patchSubdivisions = 1;
		}
		
		/* threads */
		else if( !strcmp( argv[ i ], "-threads" ) )
		{
			argv[ i ] = NULL;
			i++;
			numthreads = atoi( argv[ i ] );
			argv[ i ] = NULL;
		}
	}
	
	/* init model library */
	PicoInit();
	PicoSetMallocFunc( safe_malloc );
	PicoSetFreeFunc( free );
	PicoSetPrintFunc( PicoPrintFunc );
	PicoSetLoadFileFunc( PicoLoadFileFunc );
	PicoSetFreeFileFunc( free );
	
	/* set number of threads */
	ThreadSetDefault();
	
	/* generate sinusoid jitter table */
	for( i = 0; i < MAX_JITTERS; i++ )
	{
		jitters[ i ] = sin( i * 139.54152147 );
		//%	Sys_Printf( "Jitter %4d: %f\n", i, jitters[ i ] );
	}
	
	/* we print out two versions, q3map's main version (since it evolves a bit out of GtkRadiant)
	   and we put the GtkRadiant version to make it easy to track with what version of Radiant it was built with */
	
	Sys_Printf( "Q3Map         - v1.0r (c) 1999 Id Software Inc.\n" );
	Sys_Printf( "Q3Map (ydnar) - v" Q3MAP_VERSION "\n" );
	Sys_Printf( "GtkRadiant    - v" RADIANT_VERSION " " __DATE__ " " __TIME__ "\n" );
	Sys_Printf( "%s\n", Q3MAP_MOTD );
	
	/* ydnar: new path initialization */
	InitPaths( &argc, argv );
	
	/* check if we have enough options left to attempt something */
	if( argc < 2 )
		Error( "Usage: %s [general options] [options] mapfile", argv[ 0 ] );
	
	/* info */
	if( !strcmp( argv[ 1 ], "-info" ) )
		r = BSPInfo( argc - 2, argv + 2 );
	
	/* vis */
	else if( !strcmp( argv[ 1 ], "-vis" ) )
		r = VisMain( argc - 1, argv + 1 );
	
	/* light */
	else if( !strcmp( argv[ 1 ], "-light" ) )
		r = LightMain( argc - 1, argv + 1 );
	
	/* vlight */
	else if( !strcmp( argv[ 1 ], "-vlight" ) )
	{
		Sys_Printf( "WARNING: VLight is no longer supported, defaulting to -light -fast instead\n\n" );
		argv[ 1 ] = "-fast";	/* eek a hack */
		r = LightMain( argc, argv );
	}
	
	/* ydnar: lightmap export */
	else if( !strcmp( argv[ 1 ], "-export" ) )
		r = ExportLightmapsMain( argc - 1, argv + 1 );
	
	/* ydnar: lightmap import */
	else if( !strcmp( argv[ 1 ], "-import" ) )
		r = ImportLightmapsMain( argc - 1, argv + 1 );
	
	/* ydnar: bsp scaling */
	else if( !strcmp( argv[ 1 ], "-scale" ) )
		r = ScaleBSPMain( argc - 1, argv + 1 );
	
	/* ydnar: bsp conversion */
	else if( !strcmp( argv[ 1 ], "-convert" ) )
		r = ConvertBSPMain( argc - 1, argv + 1 );
	
	/* ydnar: otherwise create a bsp */
	else
		r = BSPMain( argc, argv );
	
	/* emit time */
	end = I_FloatTime();
	Sys_Printf( "%9.0f seconds elapsed\n", end - start );
	
	/* shut down connection */
	Broadcast_Shutdown();
	
	/* return any error code */
	return r;
}
