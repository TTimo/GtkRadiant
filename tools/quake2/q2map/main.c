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


/* marker */
#define MAIN_C



/* dependencies */
#include "q2map.h"

#define qtrue true
#define qfalse false

char    *mapname;
char game[64];
extern qboolean verbose;

/// BSP
extern qboolean drawflag;
extern qboolean noprune;
extern qboolean glview;
extern qboolean nodetail;
extern qboolean fulldetail;
extern qboolean onlyents;
extern qboolean nomerge;
extern qboolean nowater;
extern qboolean nofill;
extern qboolean nocsg;
extern qboolean noweld;
extern qboolean noshare;
extern qboolean nosubdiv;
extern qboolean notjunc;
extern qboolean noopt;
extern qboolean leaktest;
extern qboolean verboseentities;
extern char outbase[32];
extern int block_xl, block_xh, block_yl, block_yh;
extern vec_t microvolume;
extern float subdivide_size;

// VIS
extern char inbase[32];
extern qboolean fastvis;
extern qboolean nosort;
extern int testlevel;

// RAD
extern qboolean dumppatches;
extern int numbounce;
extern qboolean extrasamples;
extern float subdiv;
extern float lightscale;
extern float direct_scale;
extern float entity_scale;
extern qboolean nopvs;
extern float ambient;
extern float maxlight;


void InitPaths( int *argc, char **argv );

/*
   Random()
   returns a pseudorandom number between 0 and 1
 */
/*
   vec_t Random( void )
   {
    return (vec_t) rand() / RAND_MAX;
   }
 */


/*
   ExitQ2Map()
   cleanup routine
 */
/*
   static void ExitQ2Map( void )
   {
    BSPFilesCleanup();
    if( mapDrawSurfs != NULL )
        free( mapDrawSurfs );
   }
 */


/*
   BSPInfo()
   emits statistics about the bsp file
 */

int BSPInfo(){
	char source[ 1024 ], ext[ 64 ];
	int size;
	FILE        *f;

	Sys_Printf( "\n----- INFO ----\n\n" );

	/* dummy check */
	if ( mapname == NULL ) {
		Sys_Printf( "No files to dump info for.\n" );
		return -1;
	}

	/* enable info mode */
	//infoMode = qtrue;


	/* mangle filename and get size */
	strcpy( source, mapname );
	ExtractFileExtension( source, ext );
	if ( !Q_stricmp( ext, "map" ) ) {
		StripExtension( source );
	}
	DefaultExtension( source, ".bsp" );
	f = fopen( source, "rb" );
	if ( f ) {
		size = Q_filelength( f );
		fclose( f );
	}
	else{
		size = 0;
	}

	/* load the bsp file and print lump sizes */
	Sys_Printf( "Map: %s\n\n", source );

	Sys_Printf( "-----------------------------------------------------\n" );

	LoadBSPFile( source );
	PrintBSPFileSizes();

	Sys_Printf( "-----------------------------------------------------\n" );

	/* print sizes */
	Sys_Printf( "Total:  %d B = %.3f kB = %.3f MB\n", size, size / 1024.0, size / ( 1024.0 * 1024.0 ) );

	Sys_Printf( "-----------------------------------------------------\n" );

	/* return count */
	return 0;
}



/*
   ScaleBSPMain()
   amaze and confuse your enemies with wierd scaled maps!
 */
/*
   int ScaleBSPMain( int argc, char **argv )
   {
    int			i;
    float		f, scale;
    vec3_t		vec;
    char		str[ 1024 ];


    // arg checking
    if( argc < 2 )
    {
        Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
        return 0;
    }

    // get scale
    scale = atof( argv[ argc - 2 ] );
    if( scale == 0.0f )
    {
        Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
        Sys_Printf( "Non-zero scale value required.\n" );
        return 0;
    }

    // do some path mangling
    strcpy( source, ExpandArg( argv[ argc - 1 ] ) );
    StripExtension( source );
    DefaultExtension( source, ".bsp" );

    // load the bsp
    Sys_Printf( "Loading %s\n", source );
    LoadBSPFile( source );
    ParseEntities();

    // note it
    Sys_Printf( "--- ScaleBSP ---\n" );
    Sys_FPrintf( SYS_VRB, "%9d entities\n", numEntities );

    // scale entity keys
    for( i = 0; i < numBSPEntities && i < numEntities; i++ )
    {
        // scale origin
        GetVectorForKey( &entities[ i ], "origin", vec );
        if( (vec[ 0 ] + vec[ 1 ] + vec[ 2 ]) )
        {
            VectorScale( vec, scale, vec );
            sprintf( str, "%f %f %f", vec[ 0 ], vec[ 1 ], vec[ 2 ] );
            SetKeyValue( &entities[ i ], "origin", str );
        }

        // scale door lip
        f = FloatForKey( &entities[ i ], "lip" );
        if( f )
        {
            f *= scale;
            sprintf( str, "%f", f );
            SetKeyValue( &entities[ i ], "lip", str );
        }
    }

    // scale models
    for( i = 0; i < numBSPModels; i++ )
    {
        VectorScale( bspModels[ i ].mins, scale, bspModels[ i ].mins );
        VectorScale( bspModels[ i ].maxs, scale, bspModels[ i ].maxs );
    }

    // scale nodes
    for( i = 0; i < numBSPNodes; i++ )
    {
        VectorScale( bspNodes[ i ].mins, scale, bspNodes[ i ].mins );
        VectorScale( bspNodes[ i ].maxs, scale, bspNodes[ i ].maxs );
    }

    // scale leafs
    for( i = 0; i < numBSPLeafs; i++ )
    {
        VectorScale( bspLeafs[ i ].mins, scale, bspLeafs[ i ].mins );
        VectorScale( bspLeafs[ i ].maxs, scale, bspLeafs[ i ].maxs );
    }

    // scale drawverts
    for( i = 0; i < numBSPDrawVerts; i++ )
        VectorScale( bspDrawVerts[ i ].xyz, scale, bspDrawVerts[ i ].xyz );

    // scale planes
    for( i = 0; i < numBSPPlanes; i++ )
        bspPlanes[ i ].dist *= scale;

    // scale gridsize
    GetVectorForKey( &entities[ 0 ], "gridsize", vec );
    if( (vec[ 0 ] + vec[ 1 ] + vec[ 2 ]) == 0.0f )
        VectorCopy( gridSize, vec );
    VectorScale( vec, scale, vec );
    sprintf( str, "%f %f %f", vec[ 0 ], vec[ 1 ], vec[ 2 ] );
    SetKeyValue( &entities[ 0 ], "gridsize", str );

    // write the bsp
    UnparseEntities();
    StripExtension( source );
    DefaultExtension( source, "_s.bsp" );
    Sys_Printf( "Writing %s\n", source );
    WriteBSPFile( source );

    // return to sender
    return 0;
   }
 */


/*
   ConvertBSPMain()
   main argument processing function for bsp conversion
 */
/*
   int ConvertBSPMain( int argc, char **argv )
   {
    int		i;
    int		(*convertFunc)( char * );


    // set default
    convertFunc = ConvertBSPToASE;

    // arg checking
    if( argc < 1 )
    {
        Sys_Printf( "Usage: q3map -scale <value> [-v] <mapname>\n" );
        return 0;
    }

    // process arguments
    for( i = 1; i < (argc - 1); i++ )
    {
        // -format map|ase|...
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

    // clean up map name
    strcpy( source, ExpandArg( argv[ i ] ) );
    StripExtension( source );
    DefaultExtension( source, ".bsp" );

    LoadShaderInfo();

    Sys_Printf( "Loading %s\n", source );

    // ydnar: load surface file
    //%	LoadSurfaceExtraFile( source );

    LoadBSPFile( source );

    // parse bsp entities
    ParseEntities();

    // convert
    return convertFunc( source );
   }
 */

int Check_BSP_Options( int argc, char **argv ){
	int i;

	for ( i = 1 ; i < argc ; i++ )
	{
		if ( !strcmp( argv[i],"-glview" ) ) {
			glview = true;
		}
		else if ( !strcmp( argv[i], "-draw" ) ) {
			Sys_Printf( "drawflag = true\n" );
			drawflag = true;
		}
		else if ( !strcmp( argv[i], "-noweld" ) ) {
			Sys_Printf( "noweld = true\n" );
			noweld = true;
		}
		else if ( !strcmp( argv[i], "-nocsg" ) ) {
			Sys_Printf( "nocsg = true\n" );
			nocsg = true;
		}
		else if ( !strcmp( argv[i], "-noshare" ) ) {
			Sys_Printf( "noshare = true\n" );
			noshare = true;
		}
		else if ( !strcmp( argv[i], "-notjunc" ) ) {
			Sys_Printf( "notjunc = true\n" );
			notjunc = true;
		}
		else if ( !strcmp( argv[i], "-nowater" ) ) {
			Sys_Printf( "nowater = true\n" );
			nowater = true;
		}
		else if ( !strcmp( argv[i], "-noopt" ) ) {
			Sys_Printf( "noopt = true\n" );
			noopt = true;
		}
		else if ( !strcmp( argv[i], "-noprune" ) ) {
			Sys_Printf( "noprune = true\n" );
			noprune = true;
		}
		else if ( !strcmp( argv[i], "-nofill" ) ) {
			Sys_Printf( "nofill = true\n" );
			nofill = true;
		}
		else if ( !strcmp( argv[i], "-nomerge" ) ) {
			Sys_Printf( "nomerge = true\n" );
			nomerge = true;
		}
		else if ( !strcmp( argv[i], "-nosubdiv" ) ) {
			Sys_Printf( "nosubdiv = true\n" );
			nosubdiv = true;
		}
		else if ( !strcmp( argv[i], "-nodetail" ) ) {
			Sys_Printf( "nodetail = true\n" );
			nodetail = true;
		}
		else if ( !strcmp( argv[i], "-fulldetail" ) ) {
			Sys_Printf( "fulldetail = true\n" );
			fulldetail = true;
		}
		else if ( !strcmp( argv[i], "-onlyents" ) ) {
			Sys_Printf( "onlyents = true\n" );
			onlyents = true;
		}
		else if ( !strcmp( argv[i], "-micro" ) ) {
			microvolume = atof( argv[i + 1] );
			Sys_Printf( "microvolume = %f\n", microvolume );
			i++;
		}
		else if ( !strcmp( argv[i], "-leaktest" ) ) {
			Sys_Printf( "leaktest = true\n" );
			leaktest = true;
		}
		else if ( !strcmp( argv[i], "-verboseentities" ) ) {
			Sys_Printf( "verboseentities = true\n" );
			verboseentities = true;
		}
		else if ( !strcmp( argv[i], "-chop" ) ) {
			subdivide_size = atof( argv[i + 1] );
			Sys_Printf( "subdivide_size = %f\n", subdivide_size );
			i++;
		}
		else if ( !strcmp( argv[i], "-block" ) ) {
			block_xl = block_xh = atoi( argv[i + 1] );
			block_yl = block_yh = atoi( argv[i + 2] );
			Sys_Printf( "block: %i,%i\n", block_xl, block_yl );
			i += 2;
		}
		else if ( !strcmp( argv[i], "-blocks" ) ) {
			block_xl = atoi( argv[i + 1] );
			block_yl = atoi( argv[i + 2] );
			block_xh = atoi( argv[i + 3] );
			block_yh = atoi( argv[i + 4] );
			Sys_Printf( "blocks: %i,%i to %i,%i\n",
						block_xl, block_yl, block_xh, block_yh );
			i += 4;
		}
		else if ( !strcmp( argv[i],"-tmpout" ) ) {
			strcpy( outbase, "/tmp" );
		}
		else{
			break;
		}
	}


	return 0;
}

int Check_VIS_Options( int argc, char **argv ){
	int i;

	for ( i = 1 ; i < argc ; i++ )
	{
		if ( !strcmp( argv[i], "-fast" ) ) {
			Sys_Printf( "fastvis = true\n" );
			fastvis = true;
		}
		else if ( !strcmp( argv[i], "-level" ) ) {
			testlevel = atoi( argv[i + 1] );
			Sys_Printf( "testlevel = %i\n", testlevel );
			i++;
		}
		else if ( !strcmp( argv[i],"-nosort" ) ) {
			Sys_Printf( "nosort = true\n" );
			nosort = true;
		}
		else if ( !strcmp( argv[i],"-tmpin" ) ) {
			strcpy( inbase, "/tmp" );
		}
		else if ( !strcmp( argv[i],"-tmpout" ) ) {
			strcpy( outbase, "/tmp" );
		}
		else{
			break;
		}
	}

	return 0;
}

int Check_RAD_Options( int argc, char **argv ){
	int i;

	for ( i = 1 ; i < argc ; i++ )
	{
		if ( !strcmp( argv[i],"-dump" ) ) {
			dumppatches = true;
		}
		else if ( !strcmp( argv[i],"-bounce" ) ) {
			numbounce = atoi( argv[i + 1] );
			i++;
		}
		else if ( !strcmp( argv[i],"-extra" ) ) {
			extrasamples = true;
			Sys_Printf( "extrasamples = true\n" );
		}
		else if ( !strcmp( argv[i],"-chop" ) ) {
			subdiv = atoi( argv[i + 1] );
			i++;
		}
		else if ( !strcmp( argv[i],"-scale" ) ) {
			lightscale = atof( argv[i + 1] );
			i++;
		}
		else if ( !strcmp( argv[i],"-direct" ) ) {
			direct_scale *= atof( argv[i + 1] );
			Sys_Printf( "direct light scaling at %f\n", direct_scale );
			i++;
		}
		else if ( !strcmp( argv[i],"-entity" ) ) {
			entity_scale *= atof( argv[i + 1] );
			Sys_Printf( "entity light scaling at %f\n", entity_scale );
			i++;
		}
		else if ( !strcmp( argv[i],"-glview" ) ) {
			glview = true;
			Sys_Printf( "glview = true\n" );
		}
		else if ( !strcmp( argv[i],"-nopvs" ) ) {
			nopvs = true;
			Sys_Printf( "nopvs = true\n" );
		}
		else if ( !strcmp( argv[i],"-ambient" ) ) {
			ambient = atof( argv[i + 1] ) * 128;
			i++;
		}
		else if ( !strcmp( argv[i],"-maxlight" ) ) {
			maxlight = atof( argv[i + 1] ) * 128;
			i++;
		}
		else if ( !strcmp( argv[i],"-tmpin" ) ) {
			strcpy( inbase, "/tmp" );
		}
		else if ( !strcmp( argv[i],"-tmpout" ) ) {
			strcpy( outbase, "/tmp" );
		}
		else{
			break;
		}
	}

	return 0;
}

/*
   main()
 */

int main( int argc, char **argv ){
	int i;
	int r = 0;
	int total_time;
	double start, end;
	int alt_argc;
	char**  alt_argv;
	qboolean do_info = qfalse;
	qboolean do_bsp = qfalse;
	qboolean do_vis = qfalse;
	qboolean do_rad = qfalse;


	/* we want consistent 'randomness' */
	srand( 0 );

	/* start timer */
	start = I_FloatTime();

	Sys_Printf( "\nQ2Map - Ver. 1.0\n" );

	/* set exit call */
	//atexit( ExitQ3Map );

	game[0] = 0;

	if ( argc < 2 ) {
		Sys_Printf( " %s: -game [quake2,heretic2] -fs_basepath basepath -info -bsp -vis -rad mapname\n",argv[0] );
		return -1;
	}
	/* read general options first */
	for ( i = 1; i < argc; i++ )
	{
		/* -connect */
		if ( !strcmp( argv[ i ], "-connect" ) ) {
			i++;
			Broadcast_Setup( argv[ i ] );
		}

		/* verbose */
		else if ( !strcmp( argv[ i ], "-v" ) ) {
			verbose = qtrue;
		}

		/* threads */
		else if ( !strcmp( argv[ i ], "-threads" ) ) {
			i++;
			numthreads = atoi( argv[ i ] );
		}
		else if ( !strcmp( argv[ i ], "-game" ) ) {
			i++;
			strncpy( game, argv[ i ], 64 );
			strlower( game );
		}
	}

	/* set number of threads */
	ThreadSetDefault();

	/* ydnar: new path initialization */
	InitPaths( &argc, argv );

	/* read compiling options */
	for ( i = 1; i < argc; i++ )
	{
		/* info */
		if ( !strcmp( argv[ i ], "-info" ) ) {
			do_info = qtrue;
		}

		/* bsp */
		if ( !strcmp( argv[ i ], "-bsp" ) ) {
			do_bsp = qtrue;
			alt_argc = argc - i;
			alt_argv = (char **) ( argv + i );
			Check_BSP_Options( alt_argc, alt_argv );
		}

		/* vis */
		if ( !strcmp( argv[ i ], "-vis" ) ) {
			do_vis = qtrue;
			alt_argc = argc - i;
			alt_argv = (char **) ( argv + i );
			Check_VIS_Options( alt_argc, alt_argv );
		}

		/* rad */
		if ( !strcmp( argv[ i ], "-rad" ) ) {
			do_rad = qtrue;
			alt_argc = argc - i;
			alt_argv = (char **) ( argv + i );
			Check_RAD_Options( alt_argc, alt_argv );
		}
	}

	if ( game[0] == 0 ) {
		strncpy( game, "quake2", 7 );
	}

	Sys_Printf( "Game: %s\n", game );

	if ( !do_info && !do_bsp && !do_vis && !do_rad ) {
		Sys_FPrintf( SYS_ERR, "ERROR: -bsp, -vis, -light, nor -info specified.\nWhat to you want me to do?\n\n" );
	}
	else
	{
		mapname = argv[argc - 1];

		if ( do_bsp ) {
			BSP_Main();
		}
		if ( do_vis ) {
			VIS_Main();
		}
		if ( do_rad ) {
			RAD_Main();
		}
		if ( do_info ) {
			BSPInfo();
		}

	}

	/* emit time */
	end = I_FloatTime();
	total_time = (int) ( end - start );
	Sys_Printf( "\nTotal Time: " );
	if ( total_time > 59 ) {
		Sys_Printf( "%d Minutes ", total_time / 60 );
	}
	Sys_Printf( "%d Seconds\n", total_time % 60 );

	/* shut down connection */
	Broadcast_Shutdown();

	/* return any error code */
	return r;
}
