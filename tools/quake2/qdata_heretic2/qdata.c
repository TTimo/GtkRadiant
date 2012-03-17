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

#include "qdata.h"

void TK_Init();

qboolean g_compress_pak;
qboolean g_release;             // don't grab, copy output data to new tree
qboolean g_pak;                 // if true, copy to pak instead of release
char g_releasedir[1024];        // c:\quake2\baseq2, etc
qboolean g_archive;             // don't grab, copy source data to new tree
qboolean do3ds;
char g_only[256];               // if set, only grab this cd
qboolean g_skipmodel;           // set true when a cd is not g_only
int g_forcemodel = MODEL_AUTO;
qboolean g_verbose = false;
qboolean g_allow_newskin = true;
qboolean g_ignoreTriUV = false;
qboolean g_publishOutput = false;

char        *ext_3ds = "3ds";
char        *ext_tri = "tri";
char        *trifileext;

char g_materialFile[256] = "none";          // default for Heretic2
char        *g_outputDir;
extern char *g_publishDir;

extern qboolean g_nomkdir;

/*
   =======================================================

   PAK FILES

   =======================================================
 */

unsigned Com_BlockChecksum( void *buffer, int length );

typedef struct
{
	char name[56];
	int filepos, filelen;
} packfile_t;

typedef struct
{
	char id[4];
	int dirofs;
	int dirlen;
} packheader_t;

packfile_t pfiles[16384];
FILE            *pakfile;
packfile_t      *pf;
packheader_t pakheader;



/*
   ==============
   BeginPak
   ==============
 */
void BeginPak( char *outname ){
	if ( !g_pak ) {
		return;
	}

	pakfile = SafeOpenWrite( outname );

	// leave space for header
	SafeWrite( pakfile, &pakheader, sizeof( pakheader ) );

	pf = pfiles;
}


/*
   ==============
   ReleaseFile

   Filename should be gamedir reletive.
   Either copies the file to the release dir, or adds it to
   the pak file.
   ==============
 */
void ReleaseFile( char *filename ){
	int len;
	byte    *buf;
	char source[1024];
	char dest[1024];

	if ( !g_release ) {
		return;
	}

	sprintf( source, "%s%s", gamedir, filename );

	if ( !g_pak ) { // copy it
		sprintf( dest, "%s/%s", g_releasedir, filename );
		printf( "copying to %s\n", dest );
		QCopyFile( source, dest );
		return;
	}

	// pak it
	printf( "paking %s\n", filename );
	if ( strlen( filename ) >= sizeof( pf->name ) ) {
		Error( "Filename too long for pak: %s", filename );
	}

	len = LoadFile( source, (void **)&buf );

	// segment moved to old.c

	strcpy( pf->name, filename );
	pf->filepos = LittleLong( ftell( pakfile ) );
	pf->filelen = LittleLong( len );
	pf++;

	SafeWrite( pakfile, buf, len );

	free( buf );
}


/*
   ==============
   FinishPak
   ==============
 */
void FinishPak( void ){
	int dirlen;
	int d;
	int i;
	unsigned checksum;

	if ( !g_pak ) {
		return;
	}

	pakheader.id[0] = 'P';
	pakheader.id[1] = 'A';
	pakheader.id[2] = 'C';
	pakheader.id[3] = 'K';
	dirlen = (byte *)pf - (byte *)pfiles;
	pakheader.dirofs = LittleLong( ftell( pakfile ) );
	pakheader.dirlen = LittleLong( dirlen );

	checksum = Com_BlockChecksum( (void *)pfiles, dirlen );

	SafeWrite( pakfile, pfiles, dirlen );

	i = ftell( pakfile );

	fseek( pakfile, 0, SEEK_SET );
	SafeWrite( pakfile, &pakheader, sizeof( pakheader ) );
	fclose( pakfile );

	d = pf - pfiles;
	printf( "%i files packed in %i bytes\n",d, i );
	printf( "checksum: 0x%x\n", checksum );
}


/*
   ===============
   Cmd_File

   This is only used to cause a file to be copied during a release
   build (default.cfg, maps, etc)
   ===============
 */
void Cmd_File( void ){
	GetScriptToken( false );
	ReleaseFile( token );
}

/*
   ===============
   PackDirectory_r

   ===============
 */
#ifdef _WIN32
#include "io.h"
void PackDirectory_r( char *dir ){
	struct _finddata_t fileinfo;
	int handle;
	char dirstring[1024];
	char filename[1024];

	sprintf( dirstring, "%s%s/*.*", gamedir, dir );

	handle = _findfirst( dirstring, &fileinfo );
	if ( handle == -1 ) {
		return;
	}

	do
	{
		sprintf( filename, "%s/%s", dir, fileinfo.name );
		if ( fileinfo.attrib & _A_SUBDIR ) { // directory
			if ( fileinfo.name[0] != '.' ) {  // don't pak . and ..
				PackDirectory_r( filename );
			}
			continue;
		}
		// copy or pack the file
		ReleaseFile( filename );
	} while ( _findnext( handle, &fileinfo ) != -1 );

	_findclose( handle );
}
#else

#include <sys/types.h>
#ifdef NeXT
#include <sys/dir.h>
#else
#include <dirent.h>
#endif

void PackDirectory_r( char *dir ){
#ifdef NeXT
	struct direct **namelist, *ent;
#else
	struct dirent **namelist, *ent;
#endif
	int count;
	struct stat st;
	int i;
	int len;
	char fullname[1024];
	char dirstring[1024];
	char        *name;

	sprintf( dirstring, "%s%s", gamedir, dir );
	count = scandir( dirstring, &namelist, NULL, NULL );

	for ( i = 0 ; i < count ; i++ )
	{
		ent = namelist[i];
		name = ent->d_name;

		if ( name[0] == '.' ) {
			continue;
		}

		sprintf( fullname, "%s/%s", dir, name );
		sprintf( dirstring, "%s%s/%s", gamedir, dir, name );

		if ( stat( dirstring, &st ) == -1 ) {
			Error( "fstating %s", pf->name );
		}
		if ( st.st_mode & S_IFDIR ) { // directory
			PackDirectory_r( fullname );
			continue;
		}

		// copy or pack the file
		ReleaseFile( fullname );
	}
}
#endif


/*
   ===============
   Cmd_Dir

   This is only used to cause a directory to be copied during a
   release build (sounds, etc)
   ===============
 */
void Cmd_Dir( void ){
	GetScriptToken( false );
	PackDirectory_r( token );
}

//========================================================================

#define MAX_RTEX    16384
int numrtex;
char rtex[MAX_RTEX][64];

void ReleaseTexture( char *name ){
	int i;
	char path[1024];

	for ( i = 0 ; i < numrtex ; i++ )
		if ( !Q_strcasecmp( name, rtex[i] ) ) {
			return;
		}

	if ( numrtex == MAX_RTEX ) {
		Error( "numrtex == MAX_RTEX" );
	}

	strcpy( rtex[i], name );
	numrtex++;

	sprintf( path, "textures/%s.wal", name );
	ReleaseFile( path );
}

/*
   ===============
   Cmd_Maps

   Only relevent for release and pak files.
   Releases the .bsp files for the maps, and scans all of the files to
   build a list of all textures used, which are then released.
   ===============
 */
void Cmd_Maps( void ){
	char map[1024];
	int i;

	while ( ScriptTokenAvailable() )
	{
		GetScriptToken( false );
		sprintf( map, "maps/%s.bsp", token );
		ReleaseFile( map );

		if ( !g_release ) {
			continue;
		}

		// get all the texture references
		sprintf( map, "%smaps/%s.bsp", gamedir, token );
		LoadBSPFileTexinfo( map );
		for ( i = 0 ; i < numtexinfo ; i++ )
			ReleaseTexture( texinfo[i].texture );
	}
}


//==============================================================

/*
   ===============
   ParseScript
   ===============
 */
void ParseScript( void ){
	while ( 1 )
	{
		do
		{   // look for a line starting with a $ command
			GetScriptToken( true );
			if ( endofscript ) {
				return;
			}
			if ( token[0] == '$' ) {
				break;
			}
			while ( ScriptTokenAvailable() )
				GetScriptToken( false );
		} while ( 1 );

		//
		// model commands
		//
		if ( !strcmp( token, "$modelname" ) ) {
			MODELCMD_Modelname( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$cd" ) ) {
			MODELCMD_Cd( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$origin" ) ) {
			MODELCMD_Origin( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$cluster" ) ) {
			MODELCMD_Cluster( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$base" ) ) {
			MODELCMD_Base( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$scale" ) ) {
			MODELCMD_ScaleUp( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$frame" ) ) {
			MODELCMD_Frame( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$skin" ) ) {
			MODELCMD_Skin( MODEL_MD2 );
		}
		else if ( !strcmp( token, "$skinsize" ) ) {
			MODELCMD_Skinsize( MODEL_MD2 );
		}
		//
		// flexible model commands
		//
		else if ( !strcmp( token, "$fm_modelname" ) ) {
			MODELCMD_Modelname( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_base" ) ) {
			MODELCMD_Base( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_basest" ) ) {
			MODELCMD_BaseST( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_cd" ) ) {
			MODELCMD_Cd( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_origin" ) ) {
			MODELCMD_Origin( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_cluster" ) ) {
			MODELCMD_Cluster( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_skeleton" ) ) {
			MODELCMD_Skeleton( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_scale" ) ) {
			MODELCMD_ScaleUp( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_frame" ) ) {
			MODELCMD_Frame( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_skeletal_frame" ) ) { // left in for compadibility with qdt already using fm_skeletal_frame
			MODELCMD_Frame( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_skin" ) ) {
			MODELCMD_Skin( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_skinsize" ) ) {
			MODELCMD_Skinsize( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_begin_group" ) ) {
			MODELCMD_BeginGroup( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_end_group" ) ) {
			MODELCMD_EndGroup( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_referenced" ) ) {
			MODELCMD_Referenced( MODEL_FM );
		}
		else if ( !strcmp( token, "$fm_node_order" ) ) {
			MODELCMD_NodeOrder( MODEL_FM );
		}

		//
		// sprite commands
		//
		else if ( !strcmp( token, "$spritename" ) ) {
			Cmd_SpriteName();
		}
		else if ( !strcmp( token, "$sprdir" ) ) {
			Cmd_Sprdir();
		}
		else if ( !strcmp( token, "$load" ) ) {
			Cmd_Load();
		}
		else if ( !strcmp( token, "$spriteframe" ) ) {
			Cmd_SpriteFrame();
		}
		//
		// image commands
		//
		else if ( !strcmpi( token, "$grab" ) ) {
			Cmd_Grab();
		}
		else if ( !strcmpi( token, "$raw" ) ) {
			Cmd_Raw();
		}
		else if ( !strcmpi( token, "$colormap" ) ) {
			Cmd_Colormap();
		}
		else if ( !strcmpi( token, "$mippal" ) ) {
			Cmd_Mippal();
		}
		else if ( !strcmpi( token, "$mipdir" ) ) {
			Cmd_Mipdir();
		}
		else if ( !strcmpi( token, "$mip" ) ) {
			Cmd_Mip();
		}
		else if ( !strcmp( token, "$environment" ) ) {
			Cmd_Environment();
		}
		//
		// pics
		//
		else if ( !strcmp( token, "$picdir" ) ) {
			Cmd_Picdir();
		}
		else if ( !strcmp( token, "$pic" ) ) {
			Cmd_Pic();
		}
		//
		// book
		//
		else if ( !strcmp( token, "$bookdir" ) ) {
			Cmd_Bookdir();
		}
		else if ( !strcmp( token, "$book" ) ) {
			Cmd_Book();
		}
		//
		// tmix
		//
		else if ( !strcmp( token, "$texturemix" ) ) {
			Cmd_TextureMix();
		}
		//
		// video
		//
		else if ( !strcmp( token, "$video" ) ) {
			Cmd_Video();
		}
		//
		// misc
		//
		else if ( !strcmp( token, "$file" ) ) {
			Cmd_File();
		}
		else if ( !strcmp( token, "$dir" ) ) {
			Cmd_Dir();
		}
		else if ( !strcmp( token, "$maps" ) ) {
			Cmd_Maps();
		}
		else if ( !strcmp( token, "$alphalight" ) ) {
			Cmd_Alphalight();
		}
		else if ( !strcmp( token, "$inverse16table" ) ) {
			Cmd_Inverse16Table();
		}
		else{
			Error( "bad command %s\n", token );
		}
	}
}

//=======================================================

/*
   ==============
   main
   ==============
 */
int main( int argc, char **argv ){
	int i;
	char path[1024];
	char        *basedir;
	double starttime, endtime;

	printf( "Qdata Plus : "__TIME__ " "__DATE__ "\n" );

	starttime = I_FloatTime();
	basedir = NULL;

	TK_Init();
	ExpandWildcards( &argc, &argv );

	for ( i = 1 ; i < argc ; i++ )
	{
		if ( !strcmp( argv[i], "-archive" ) ) {
			// -archive f:/quake2/release/dump_11_30
			archive = true;
			strcpy( archivedir, argv[i + 1] );
			printf( "Archiving source to: %s\n", archivedir );
			i++;
		}
		else if ( !strcmp( argv[i], "-release" ) ) {
			g_release = true;
			strcpy( g_releasedir, argv[i + 1] );
			printf( "Copy output to: %s\n", g_releasedir );
			i++;
		}
		else if ( !strcmp( argv[i], "-base" ) ) {
			i++;
			basedir = argv[i];
		}
		else if ( !strcmp( argv[i], "-compress" ) ) {
			g_compress_pak = true;
			printf( "Compressing pakfile\n" );
		}
		else if ( !strcmp( argv[i], "-pak" ) ) {
			g_release = true;
			g_pak = true;
			printf( "Building pakfile: %s\n", argv[i + 1] );
			BeginPak( argv[i + 1] );
			i++;
		}
		else if ( !strcmp( argv[i], "-only" ) ) {
			strcpy( g_only, argv[i + 1] );
			printf( "Only grabbing %s\n", g_only );
			i++;
		}
		else if ( !strcmpi( argv[i], "-keypress" ) ) {
			g_dokeypress = true;
		}
		else if ( !strcmp( argv[i], "-3ds" ) ) {
			do3ds = true;
			printf( "loading .3ds files\n" );
		}
		else if ( !strcmp( argv[i], "-materialfile" ) ) {
			strcpy( g_materialFile, argv[i + 1] );
			printf( "Setting material file to %s\n", g_materialFile );
			i++;
		}
/*		else if (!strcmpi(argv[i], "-newgen"))
        {
            if (i < argc-4)
            {
                printf("run new triangle grouping routine here\n");
                NewGen(argv[i+1],argv[i+2],atoi(argv[i+3]),atoi(argv[i+4]));
            }
            else
            {
                printf("qdata -newskin <base.hrc> <skin.pcx> width height\n");
            }
            return 0;
        }
 */     else if ( !strcmpi( argv[i], "-genskin" ) ) {
			i++;
			if ( i < argc - 3 ) {
				GenSkin( argv[i],argv[i + 1],atol( argv[i + 2] ),atol( argv[i + 3] ) );
			}
			else
			{
				printf( "qdata -genskin <base.hrc> <skin.pcx> <desired width> <desired height>\n" );
			}
			return 0;

		}
		else if ( !strcmpi( argv[i], "-noopts" ) ) {
			g_no_opimizations = true;
			printf( "not performing optimizations\n" );
		}
		else if ( !strcmpi( argv[i], "-md2" ) ) {
			g_forcemodel = MODEL_MD2;
		}
		else if ( !strcmpi( argv[i], "-fm" ) ) {
			g_forcemodel = MODEL_FM;
		}
		else if ( !strcmpi( argv[i], "-verbose" ) ) {
			g_verbose = true;
		}
		else if ( !strcmpi( argv[i], "-oldskin" ) ) {
			g_allow_newskin = false;
		}
		else if ( !strcmpi( argv[i], "-ignoreUV" ) ) {
			g_ignoreTriUV = true;
		}
		else if ( !strcmpi( argv[i], "-publish" ) ) {
			g_publishOutput = true;
		}
		else if ( !strcmpi( argv[i], "-nomkdir" ) ) {
			g_nomkdir = true;
		}
		else if ( argv[i][0] == '-' ) {
			Error( "Unknown option \"%s\"", argv[i] );
		}
		else{
			break;
		}
	}

	if ( i >= argc ) {
		Error( "usage: qdata [-archive <directory>]\n"
			   "             [-release <directory>]\n"
			   "             [-base <directory>]\n"
			   "             [-compress]\n"
			   "             [-pak <file>]\n"
			   "             [-only <model>]\n"
			   "             [-keypress]\n"
			   "             [-3ds]\n"
			   "             [-materialfile <file>]\n"
			   "             [-noopts]\n"
			   "             [-md2]\n"
			   "             [-fm]\n"
			   "             [-verbose]\n"
			   "             [-ignoreUV]\n"
			   "             [-oldskin]\n"
			   "             [-publish]\n"
			   "             [-nomkdir]\n"
			   "             file.qdt\n"
			   "or\n"
			   "       qdata -genskin <base.hrc> <skin.pcx> <desired width> <desired height>" );
	}

	if ( do3ds ) {
		trifileext = ext_3ds;
	}
	else{
		trifileext = ext_tri;
	}

	for ( ; i < argc ; i++ )
	{
		printf( "--------------- %s ---------------\n", argv[i] );
		// load the script
		strcpy( path, argv[i] );
		DefaultExtension( path, ".qdt" );
		DefaultExtension( g_materialFile, ".mat" );
		SetQdirFromPath( path );

		printf( "workingdir='%s'\n", gamedir );
		if ( basedir ) {
			qdir[0] = 0;
			g_outputDir = basedir;
		}

		printf( "outputdir='%s'\n", g_outputDir );

		QFile_ReadMaterialTypes( g_materialFile );
		LoadScriptFile( ExpandArg( path ) );

		//
		// parse it
		//
		ParseScript();

		// write out the last model
		FinishModel();
		FMFinishModel();
		FinishSprite();
	}

	if ( total_textures ) {
		printf( "\n" );
		printf( "Total textures processed: %d\n",total_textures );
		printf( "Average size: %d x %d\n",total_x / total_textures, total_y / total_textures );
	}

	if ( g_pak ) {
		FinishPak();
	}

	endtime = I_FloatTime();
	printf( "Time elapsed:  %f\n", endtime - starttime );

	if ( g_dokeypress ) {
		printf( "Success! ... Hit a key: " );
		getchar();
	}

	return 0;
}
