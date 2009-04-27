/* -------------------------------------------------------------------------------

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

vec_t Random( void ){
	return (vec_t) rand() / RAND_MAX;
}


char *Q_strncpyz( char *dst, const char *src, size_t len ) {
	if ( len == 0 ) {
		abort();
	}

	strncpy( dst, src, len );
	dst[ len - 1 ] = '\0';
	return dst;
}


char *Q_strcat( char *dst, size_t dlen, const char *src ) {
	size_t n = strlen( dst  );

	if ( n > dlen ) {
		abort(); /* buffer overflow */
	}

	return Q_strncpyz( dst + n, src, dlen - n );
}


char *Q_strncat( char *dst, size_t dlen, const char *src, size_t slen ) {
	size_t n = strlen( dst );

	if ( n > dlen ) {
		abort(); /* buffer overflow */
	}

	return Q_strncpyz( dst + n, src, MIN( slen, dlen - n ) );
}


/*
   ExitQ3Map()
   cleanup routine
 */

static void ExitQ3Map( void ){
	BSPFilesCleanup();
	if ( mapDrawSurfs != NULL ) {
		free( mapDrawSurfs );
	}
}


/* minimap stuff */

/* borrowed from light.c */
void WriteTGA24( char *filename, byte *data, int width, int height, qboolean flip );
typedef struct minimap_s
{
	bspModel_t *model;
	int width;
	int height;
	int samples;
	float *sample_offsets;
	float sharpen_boxmult;
	float sharpen_centermult;
	float *data1f;
	float *sharpendata1f;
	vec3_t mins, size;
}
minimap_t;
static minimap_t minimap;

qboolean BrushIntersectionWithLine(bspBrush_t *brush, vec3_t start, vec3_t dir, float *t_in, float *t_out)
{
	int i;
	qboolean in = qfalse, out = qfalse;
	bspBrushSide_t *sides = &bspBrushSides[brush->firstSide];

	for(i = 0; i < brush->numSides; ++i)
	{
		bspPlane_t *p = &bspPlanes[sides[i].planeNum];
		float sn = DotProduct(start, p->normal);
		float dn = DotProduct(dir, p->normal);
		if(dn == 0)
		{
			if(sn > p->dist)
				return qfalse; // outside!
		}
		else
		{
			float t = (p->dist - sn) / dn;
			if(dn < 0)
			{
				if(!in || t > *t_in)
				{
					*t_in = t;
					in = qtrue;
					// as t_in can only increase, and t_out can only decrease, early out
					if(out && *t_in >= *t_out)
						return qfalse;
				}
			}
			else
			{
				if(!out || t < *t_out)
				{
					*t_out = t;
					out = qtrue;
					// as t_in can only increase, and t_out can only decrease, early out
					if(in && *t_in >= *t_out)
						return qfalse;
				}
			}
		}
	}
	return in && out;
}

static float MiniMapSample(float x, float y)
{
	vec3_t org, dir;
	int i, bi;
	float t0, t1;
	float samp;
	bspBrush_t *b;
	bspBrushSide_t *s;
	int cnt;

	org[0] = x;
	org[1] = y;
	org[2] = 0;
	dir[0] = 0;
	dir[1] = 0;
	dir[2] = 1;

	cnt = 0;
	samp = 0;
	for(i = 0; i < minimap.model->numBSPBrushes; ++i)
	{
		bi = minimap.model->firstBSPBrush + i;
		if(opaqueBrushes[bi >> 3] & (1 << (bi & 7)))
		{
			b = &bspBrushes[bi];

			// sort out mins/maxs of the brush
			s = &bspBrushSides[b->firstSide];
			if(x < -bspPlanes[s[0].planeNum].dist)
				continue;
			if(x > +bspPlanes[s[1].planeNum].dist)
				continue;
			if(y < -bspPlanes[s[2].planeNum].dist)
				continue;
			if(y > +bspPlanes[s[3].planeNum].dist)
				continue;

			if(BrushIntersectionWithLine(b, org, dir, &t0, &t1))
			{
				samp += t1 - t0;
				++cnt;
			}
		}
	}

	return samp;
}

void RandomVector2f(float v[2])
{
	do
	{
		v[0] = 2 * Random() - 1;
		v[1] = 2 * Random() - 1;
	}
	while(v[0] * v[0] + v[1] * v[1] > 1);
}

static void MiniMapRandomlySupersampled(int y)
{
	int x, i;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * (y / (float) minimap.height);
	float dx   =                   minimap.size[0]      / (float) minimap.width;
	float dy   =                   minimap.size[1]      / (float) minimap.height;
	float uv[2];
	float thisval;

	for(x = 0; x < minimap.width; ++x)
	{
		float xmin = minimap.mins[0] + minimap.size[0] * (x / (float) minimap.width);
		float val = 0;

		for(i = 0; i < minimap.samples; ++i)
		{
			RandomVector2f(uv);
			thisval = MiniMapSample(
				xmin + (uv[0] + 0.5) * dx, /* exaggerated random pattern for better results */
				ymin + (uv[1] + 0.5) * dy  /* exaggerated random pattern for better results */
			);
			val += thisval;
		}
		val /= minimap.samples * minimap.size[2];
		*p++ = val;
	}
}

static void MiniMapSupersampled(int y)
{
	int x, i;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * (y / (float) minimap.height);
	float dx   =                   minimap.size[0]      / (float) minimap.width;
	float dy   =                   minimap.size[1]      / (float) minimap.height;

	for(x = 0; x < minimap.width; ++x)
	{
		float xmin = minimap.mins[0] + minimap.size[0] * (x / (float) minimap.width);
		float val = 0;

		for(i = 0; i < minimap.samples; ++i)
		{
			float thisval = MiniMapSample(
				xmin + minimap.sample_offsets[2*i+0] * dx,
				ymin + minimap.sample_offsets[2*i+1] * dy
			);
			val += thisval;
		}
		val /= minimap.samples * minimap.size[2];
		*p++ = val;
	}
}

static void MiniMapNoSupersampling(int y)
{
	int x;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * ((y + 0.5) / (float) minimap.height);

	for(x = 0; x < minimap.width; ++x)
	{
		float xmin = minimap.mins[0] + minimap.size[0] * ((x + 0.5) / (float) minimap.width);
		*p++ = MiniMapSample(xmin, ymin) / minimap.size[2];
	}
}

static void MiniMapSharpen(int y)
{
	int x;
	qboolean up = (y > 0);
	qboolean down = (y < minimap.height - 1);
	float *p = &minimap.data1f[y * minimap.width];
	float *q = &minimap.sharpendata1f[y * minimap.width];

	for(x = 0; x < minimap.width; ++x)
	{
		qboolean left = (x > 0);
		qboolean right = (x < minimap.width - 1);
		float val = p[0] * minimap.sharpen_centermult;

		if(left && up)
			val += p[-1 -minimap.width] * minimap.sharpen_boxmult;
		if(left && down)
			val += p[-1 +minimap.width] * minimap.sharpen_boxmult;
		if(right && up)
			val += p[+1 -minimap.width] * minimap.sharpen_boxmult;
		if(right && down)
			val += p[+1 +minimap.width] * minimap.sharpen_boxmult;
			
		if(left)
			val += p[-1] * minimap.sharpen_boxmult;
		if(right)
			val += p[+1] * minimap.sharpen_boxmult;
		if(up)
			val += p[-minimap.width] * minimap.sharpen_boxmult;
		if(down)
			val += p[+minimap.width] * minimap.sharpen_boxmult;

		++p;
		*q++ = val;
	}
}

void MiniMapMakeMinsMaxs(vec3_t mins_in, vec3_t maxs_in, float border)
{
	vec3_t mins, maxs, extend;
	VectorCopy(mins_in, mins);
	VectorCopy(maxs_in, maxs);

	// line compatible to nexuiz mapinfo
	Sys_Printf("size %f %f %f %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);

	VectorSubtract(maxs, mins, extend);

	if(extend[1] > extend[0])
	{
		mins[0] -= (extend[1] - extend[0]) * 0.5;
		maxs[0] += (extend[1] - extend[0]) * 0.5;
	}
	else
	{
		mins[1] -= (extend[0] - extend[1]) * 0.5;
		maxs[1] += (extend[0] - extend[1]) * 0.5;
	}

	/* border: amount of black area around the image */
	/* input: border, 1-2*border, border but we need border/(1-2*border) */

	VectorSubtract(maxs, mins, extend);
	VectorScale(extend, border / (1 - 2 * border), extend);

	VectorSubtract(mins, extend, mins);
	VectorAdd(maxs, extend, maxs);

	VectorCopy(mins, minimap.mins);
	VectorSubtract(maxs, mins, minimap.size);

	// line compatible to nexuiz mapinfo
	Sys_Printf("size_texcoords %f %f %f %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);
}

/*
MiniMapSetupBrushes()
determines solid non-sky brushes in the world
*/

void MiniMapSetupBrushes( void )
{
	int				i, b, compileFlags;
	bspBrush_t		*brush;
	bspShader_t		*shader;
	shaderInfo_t	*si;
	
	
	/* note it */
	Sys_FPrintf( SYS_VRB, "--- MiniMapSetupBrushes ---\n" );
	
	/* allocate */
	if( opaqueBrushes == NULL )
		opaqueBrushes = safe_malloc( numBSPBrushes / 8 + 1 );
	
	/* clear */
	memset( opaqueBrushes, 0, numBSPBrushes / 8 + 1 );
	numOpaqueBrushes = 0;
	
	/* walk the list of worldspawn brushes */
	for( i = 0; i < minimap.model->numBSPBrushes; i++ )
	{
		/* get brush */
		b = minimap.model->firstBSPBrush + i;
		brush = &bspBrushes[ b ];
		
#if 0
		/* check all sides */
		compileFlags = 0;
		for( j = 0; j < brush->numSides; j++ )
		{
			/* do bsp shader calculations */
			side = &bspBrushSides[ brush->firstSide + j ];
			shader = &bspShaders[ side->shaderNum ];
			
			/* get shader info */
			si = ShaderInfoForShader( shader->shader );
			if( si == NULL )
				continue;
			
			/* or together compile flags */
			compileFlags |= si->compileFlags;
		}
#else
		shader = &bspShaders[ brush->shaderNum ];
		si = ShaderInfoForShader( shader->shader );
		if( si == NULL )
			compileFlags = 0;
		else
			compileFlags = si->compileFlags;
#endif
		
		/* determine if this brush is solid */
		if( (compileFlags & (C_SOLID | C_SKY)) == C_SOLID )
		{
			opaqueBrushes[ b >> 3 ] |= (1 << (b & 7));
			numOpaqueBrushes++;
			maxOpaqueBrush = i;
		}
	}
	
	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d solid brushes\n", numOpaqueBrushes );
}

qboolean MiniMapEvaluateSampleOffsets(int *bestj, int *bestk, float *bestval)
{
	float val, dx, dy;
	int j, k;

	*bestj = *bestk = -1;
	*bestval = 3; /* max possible val is 2 */

	for(j = 0; j < minimap.samples; ++j)
		for(k = j + 1; k < minimap.samples; ++k)
		{
			dx = minimap.sample_offsets[2*j+0] - minimap.sample_offsets[2*k+0];
			dy = minimap.sample_offsets[2*j+1] - minimap.sample_offsets[2*k+1];
			if(dx > +0.5) dx -= 1;
			if(dx < -0.5) dx += 1;
			if(dy > +0.5) dy -= 1;
			if(dy < -0.5) dy += 1;
			val = dx * dx + dy * dy;
			if(val < *bestval)
			{
				*bestj = j;
				*bestk = k;
				*bestval = val;
			}
		}
	
	return *bestval < 3;
}

void MiniMapMakeSampleOffsets()
{
	int i, j, k, jj, kk;
	float val, valj, valk, sx, sy, rx, ry;

	Sys_Printf( "Generating good sample offsets (this may take a while)...\n" );

	/* start with entirely random samples */
	for(i = 0; i < minimap.samples; ++i)
	{
		minimap.sample_offsets[2*i+0] = Random();
		minimap.sample_offsets[2*i+1] = Random();
	}

	for(i = 0; i < 1000; ++i)
	{
		if(MiniMapEvaluateSampleOffsets(&j, &k, &val))
		{
			sx = minimap.sample_offsets[2*j+0];
			sy = minimap.sample_offsets[2*j+1];
			minimap.sample_offsets[2*j+0] = rx = Random();
			minimap.sample_offsets[2*j+1] = ry = Random();
			if(!MiniMapEvaluateSampleOffsets(&jj, &kk, &valj))
				valj = -1;
			minimap.sample_offsets[2*j+0] = sx;
			minimap.sample_offsets[2*j+1] = sy;

			sx = minimap.sample_offsets[2*k+0];
			sy = minimap.sample_offsets[2*k+1];
			minimap.sample_offsets[2*k+0] = rx;
			minimap.sample_offsets[2*k+1] = ry;
			if(!MiniMapEvaluateSampleOffsets(&jj, &kk, &valk))
				valk = -1;
			minimap.sample_offsets[2*k+0] = sx;
			minimap.sample_offsets[2*k+1] = sy;

			if(valj > valk)
			{
				if(valj > val)
				{
					/* valj is the greatest */
					minimap.sample_offsets[2*j+0] = rx;
					minimap.sample_offsets[2*j+1] = ry;
					i = -1;
				}
				else
				{
					/* valj is the greater and it is useless - forget it */
				}
			}
			else
			{
				if(valk > val)
				{
					/* valk is the greatest */
					minimap.sample_offsets[2*k+0] = rx;
					minimap.sample_offsets[2*k+1] = ry;
					i = -1;
				}
				else
				{
					/* valk is the greater and it is useless - forget it */
				}
			}
		}
		else
			break;
	}
}

void MergeRelativePath(char *out, const char *absolute, const char *relative)
{
	const char *endpos = absolute + strlen(absolute);
	while(endpos != absolute && (endpos[-1] == '/' || endpos[-1] == '\\'))
		--endpos;
	while(relative[0] == '.' && relative[1] == '.' && (relative[2] == '/' || relative[2] == '\\'))
	{
		relative += 3;
		while(endpos != absolute)
		{
			--endpos;
			if(*endpos == '/' || *endpos == '\\')
				break;
		}
		while(endpos != absolute && (endpos[-1] == '/' || endpos[-1] == '\\'))
			--endpos;
	}
	memcpy(out, absolute, endpos - absolute);
	out[endpos - absolute] = '/';
	strcpy(out + (endpos - absolute + 1), relative);
}

int MiniMapBSPMain( int argc, char **argv )
{
	char minimapFilename[1024];
	char basename[1024];
	char path[1024];
	char relativeMinimapFilename[1024];
	float minimapSharpen;
	float border;
	byte *data3b, *p;
	float *q;
	int x, y;
	int i;
	vec3_t mins, maxs;

	/* arg checking */
	if( argc < 2 )
	{
		Sys_Printf( "Usage: q3map [-v] -minimap [-size n] [-sharpen f] [-samples n | -random n] [-o filename.tga] [-minmax Xmin Ymin Zmin Xmax Ymax Zmax] <mapname>\n" );
		return 0;
	}

	/* load the BSP first */
	strcpy( source, ExpandArg( argv[ argc - 1 ] ) );
	StripExtension( source );
	DefaultExtension( source, ".bsp" );
	Sys_Printf( "Loading %s\n", source );
	BeginMapShaderFile( source );
	LoadShaderInfo();
	LoadBSPFile( source );

	minimap.model = &bspModels[0];
	VectorCopy(minimap.model->mins, mins);
	VectorCopy(minimap.model->maxs, maxs);

	*minimapFilename = 0;
	minimapSharpen = game->miniMapSharpen;
	minimap.width = minimap.height = game->miniMapSize;
	border = game->miniMapBorder;

	minimap.samples = 1;
	minimap.sample_offsets = NULL;

	/* process arguments */
	for( i = 1; i < (argc - 1); i++ )
	{
		if( !strcmp( argv[ i ],  "-size" ) )
 		{
			minimap.width = minimap.height = atoi(argv[i + 1]);
			i++;
			Sys_Printf( "Image size set to %i\n", minimap.width );
 		}
		else if( !strcmp( argv[ i ],  "-sharpen" ) )
 		{
			minimapSharpen = atof(argv[i + 1]);
			i++;
			Sys_Printf( "Sharpening coefficient set to %f\n", minimapSharpen );
 		}
		else if( !strcmp( argv[ i ],  "-samples" ) )
 		{
			minimap.samples = atoi(argv[i + 1]);
			i++;
			Sys_Printf( "Samples set to %i\n", minimap.samples );
			if(minimap.sample_offsets)
				free(minimap.sample_offsets);
			minimap.sample_offsets = malloc(2 * sizeof(*minimap.sample_offsets) * minimap.samples);
			MiniMapMakeSampleOffsets();
 		}
		else if( !strcmp( argv[ i ],  "-random" ) )
 		{
			minimap.samples = atoi(argv[i + 1]);
			i++;
			Sys_Printf( "Random samples set to %i\n", minimap.samples );
			if(minimap.sample_offsets)
				free(minimap.sample_offsets);
			minimap.sample_offsets = NULL;
 		}
		else if( !strcmp( argv[ i ],  "-border" ) )
 		{
			border = atof(argv[i + 1]);
			i++;
			Sys_Printf( "Border set to %f\n", border );
 		}
		else if( !strcmp( argv[ i ],  "-o" ) )
 		{
			strcpy(minimapFilename, argv[i + 1]);
			i++;
			Sys_Printf( "Output file name set to %s\n", minimapFilename );
 		}
		else if( !strcmp( argv[ i ],  "-minmax" ) && i < (argc - 7) )
 		{
			mins[0] = atof(argv[i + 1]);
			mins[1] = atof(argv[i + 2]);
			mins[2] = atof(argv[i + 3]);
			maxs[0] = atof(argv[i + 4]);
			maxs[1] = atof(argv[i + 5]);
			maxs[2] = atof(argv[i + 6]);
			i += 6;
			Sys_Printf( "Map mins/maxs overridden\n" );
 		}
	}

	MiniMapMakeMinsMaxs(mins, maxs, border);

	if(!*minimapFilename)
	{
		ExtractFileBase(source, basename);
		ExtractFilePath(source, path);
		sprintf(relativeMinimapFilename, game->miniMapNameFormat, basename);
		MergeRelativePath(minimapFilename, path, relativeMinimapFilename);
		Sys_Printf("Output file name automatically set to %s\n", minimapFilename);
	}

	if(minimapSharpen >= 0)
	{
		minimap.sharpen_centermult = 8 * minimapSharpen + 1;
		minimap.sharpen_boxmult    =    -minimapSharpen;
	}

	minimap.data1f = safe_malloc(minimap.width * minimap.height * sizeof(*minimap.data1f));
	data3b = safe_malloc(minimap.width * minimap.height * 3);
	if(minimapSharpen >= 0)
		minimap.sharpendata1f = safe_malloc(minimap.width * minimap.height * sizeof(*minimap.data1f));

	MiniMapSetupBrushes();

	if(minimap.samples <= 1)
	{
		Sys_Printf( "\n--- MiniMapNoSupersampling (%d) ---\n", minimap.height );
		RunThreadsOnIndividual(minimap.height, qtrue, MiniMapNoSupersampling);
	}
	else
	{
		if(minimap.sample_offsets)
		{
			Sys_Printf( "\n--- MiniMapSupersampled (%d) ---\n", minimap.height );
			RunThreadsOnIndividual(minimap.height, qtrue, MiniMapSupersampled);
		}
		else
		{
			Sys_Printf( "\n--- MiniMapRandomlySupersampled (%d) ---\n", minimap.height );
			RunThreadsOnIndividual(minimap.height, qtrue, MiniMapRandomlySupersampled);
		}
	}

	if(minimap.sharpendata1f)
	{
		Sys_Printf( "\n--- MiniMapSharpen (%d) ---\n", minimap.height );
		RunThreadsOnIndividual(minimap.height, qtrue, MiniMapSharpen);
		q = minimap.sharpendata1f;
	}
	else
	{
		q = minimap.data1f;
	}

	Sys_Printf( "\nConverting...");
	p = data3b;
	for(y = 0; y < minimap.height; ++y)
		for(x = 0; x < minimap.width; ++x)
		{
			byte b;
			float v = *q++;
			if(v < 0) v = 0;
			if(v > 255.0/256.0) v = 255.0/256.0;
			b = v * 256;
			*p++ = b;
			*p++ = b;
			*p++ = b;
		}

	Sys_Printf( " writing to %s...", minimapFilename );
	WriteTGA24(minimapFilename, data3b, minimap.width, minimap.height, qfalse);

	Sys_Printf( " done.\n" );

	/* return to sender */
	return 0;
}

/*
   main()
   q3map mojo...
 */

int main( int argc, char **argv ){
	int i, r;
	double start, end;


	/* we want consistent 'randomness' */
	srand( 0 );

	/* start timer */
	start = I_FloatTime();

	/* this was changed to emit version number over the network */
	printf( Q3MAP_VERSION "\n" );

	/* set exit call */
	atexit( ExitQ3Map );

	/* read general options first */
	for ( i = 1; i < argc; i++ )
	{
		/* -connect */
		if ( !strcmp( argv[ i ], "-connect" ) ) {
			argv[ i ] = NULL;
			i++;
			Broadcast_Setup( argv[ i ] );
			argv[ i ] = NULL;
		}

		/* verbose */
		else if ( !strcmp( argv[ i ], "-v" ) ) {
			verbose = qtrue;
			argv[ i ] = NULL;
		}

		/* force */
		else if ( !strcmp( argv[ i ], "-force" ) ) {
			force = qtrue;
			argv[ i ] = NULL;
		}

		/* patch subdivisions */
		else if ( !strcmp( argv[ i ], "-subdivisions" ) ) {
			argv[ i ] = NULL;
			i++;
			patchSubdivisions = atoi( argv[ i ] );
			argv[ i ] = NULL;
			if ( patchSubdivisions <= 0 ) {
				patchSubdivisions = 1;
			}
		}

		/* threads */
		else if ( !strcmp( argv[ i ], "-threads" ) ) {
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
	for ( i = 0; i < MAX_JITTERS; i++ )
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
	if ( argc < 2 ) {
		Error( "Usage: %s [general options] [options] mapfile", argv[ 0 ] );
	}

	/* fixaas */
	if ( !strcmp( argv[ 1 ], "-fixaas" ) ) {
		r = FixAASMain( argc - 1, argv + 1 );
	}

	/* analyze */
	else if ( !strcmp( argv[ 1 ], "-analyze" ) ) {
		r = AnalyzeBSPMain( argc - 1, argv + 1 );
	}

	/* info */
	else if ( !strcmp( argv[ 1 ], "-info" ) ) {
		r = BSPInfoMain( argc - 2, argv + 2 );
	}

	/* vis */
	else if ( !strcmp( argv[ 1 ], "-vis" ) ) {
		r = VisMain( argc - 1, argv + 1 );
	}

	/* light */
	else if ( !strcmp( argv[ 1 ], "-light" ) ) {
		r = LightMain( argc - 1, argv + 1 );
	}

	/* vlight */
	else if ( !strcmp( argv[ 1 ], "-vlight" ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: VLight is no longer supported, defaulting to -light -fast instead\n\n" );
		argv[ 1 ] = "-fast";    /* eek a hack */
		r = LightMain( argc, argv );
	}

	/* QBall: export entities */
	else if ( !strcmp( argv[ 1 ], "-exportents" ) ) {
		r = ExportEntitiesMain( argc - 1, argv + 1 );
	}

	/* ydnar: lightmap export */
	else if ( !strcmp( argv[ 1 ], "-export" ) ) {
		r = ExportLightmapsMain( argc - 1, argv + 1 );
	}

	/* ydnar: lightmap import */
	else if ( !strcmp( argv[ 1 ], "-import" ) ) {
		r = ImportLightmapsMain( argc - 1, argv + 1 );
	}

	/* ydnar: bsp scaling */
	else if ( !strcmp( argv[ 1 ], "-scale" ) ) {
		r = ScaleBSPMain( argc - 1, argv + 1 );
	}

	/* ydnar: bsp conversion */
	else if ( !strcmp( argv[ 1 ], "-convert" ) ) {
		r = ConvertBSPMain( argc - 1, argv + 1 );
	}

	/* div0: minimap */
	else if( !strcmp( argv[ 1 ], "-minimap" ) )
		r = MiniMapBSPMain(argc - 1, argv + 1);

	/* ydnar: otherwise create a bsp */
	else{
		r = BSPMain( argc, argv );
	}

	/* emit time */
	end = I_FloatTime();
	Sys_Printf( "%9.0f seconds elapsed\n", end - start );

	/* shut down connection */
	Broadcast_Shutdown();

	/* return any error code */
	return r;
}
