/* -----------------------------------------------------------------------------

   InterQuake Model - PicoModel Library

   Copyright (c) 2018-2021, FTE Team <fteqw.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the names of the copyright holders nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   ----------------------------------------------------------------------------- */

/* dependencies */
#include "picointernal.h"

extern const picoModule_t picoModuleIQM;

#define IQM_MAGIC "INTERQUAKEMODEL"	//15+null

/*
   ========================================================================

   .IQM triangle model file format

   ========================================================================
 */

enum
{
	IQM_POSITION = 0,
	IQM_TEXCOORD = 1,
	IQM_NORMAL = 2,
	IQM_TANGENT = 3,
	IQM_BLENDINDEXES = 4,
	IQM_BLENDWEIGHTS = 5,
	IQM_COLOR = 6,
	IQM_CUSTOM = 0x10
};

enum
{
	IQM_BYTE = 0,
	IQM_UBYTE = 1,
	IQM_SHORT = 2,
	IQM_USHORT = 3,
	IQM_INT = 4,
	IQM_UINT = 5,
	IQM_HALF = 6,
	IQM_FLOAT = 7,
	IQM_DOUBLE = 8
};

// animflags
#define IQM_LOOP 1

typedef struct iqmHeader_s {
	byte id[16];
	unsigned int version;
	unsigned int filesize;
	unsigned int flags;
	unsigned int num_text, ofs_text;
	unsigned int num_meshes, ofs_meshes;
	unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
	unsigned int num_triangles, ofs_triangles, ofs_neighbors;
	unsigned int num_joints, ofs_joints;
	unsigned int num_poses, ofs_poses;
	unsigned int num_anims, ofs_anims;
	unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
	unsigned int num_comment, ofs_comment;
	unsigned int num_extensions, ofs_extensions;
} iqmHeader_t;

typedef struct iqmmesh_s {
	unsigned int name;
	unsigned int material;
	unsigned int first_vertex;
	unsigned int num_vertexes;
	unsigned int first_triangle;
	unsigned int num_triangles;
} iqmmesh_t;

typedef struct iqmvertexarray_s {
	unsigned int type;
	unsigned int flags;
	unsigned int format;
	unsigned int size;
	unsigned int offset;
} iqmvertexarray_t;

//is anyone actually going to run this on a big-endian cpu?
static iqmHeader_t SwapHeader(const iqmHeader_t *h)
{
	iqmHeader_t r = *h;
	r.version = _pico_little_long(h->version);
	r.filesize = _pico_little_long(h->filesize);
	r.flags = _pico_little_long(h->flags);
	r.num_text = _pico_little_long(h->num_text);
	r.ofs_text = _pico_little_long(h->ofs_text);
	r.num_meshes = _pico_little_long(h->num_meshes);
	r.ofs_meshes = _pico_little_long(h->ofs_meshes);
	r.num_vertexarrays = _pico_little_long(h->num_vertexarrays);
	r.num_vertexes = _pico_little_long(h->num_vertexes);
	r.ofs_vertexarrays = _pico_little_long(h->ofs_vertexarrays);
	r.num_triangles = _pico_little_long(h->num_triangles);
	r.ofs_triangles = _pico_little_long(h->ofs_triangles);
	r.ofs_neighbors = _pico_little_long(h->ofs_neighbors);
	r.num_joints = _pico_little_long(h->num_joints);
	r.ofs_joints = _pico_little_long(h->ofs_joints);
	r.num_poses = _pico_little_long(h->num_poses);
	r.ofs_poses = _pico_little_long(h->ofs_poses);
	r.num_anims = _pico_little_long(h->num_anims);
	r.ofs_anims = _pico_little_long(h->ofs_anims);
	r.num_frames = _pico_little_long(h->num_frames);
	r.num_framechannels = _pico_little_long(h->num_framechannels);
	r.ofs_frames = _pico_little_long(h->ofs_frames);
	r.ofs_bounds = _pico_little_long(h->ofs_bounds);
	r.num_comment = _pico_little_long(h->num_comment);
	r.ofs_comment = _pico_little_long(h->ofs_comment);
	r.num_extensions = _pico_little_long(h->num_extensions);
	r.ofs_extensions = _pico_little_long(h->ofs_extensions);
	return r;
}

// _iqm_canload()
static int _iqm_canload( PM_PARAMS_CANLOAD ){
	iqmHeader_t h;

	//make sure there's enough data for the header...
	if ((size_t)bufSize < sizeof(h))
		return PICO_PMV_ERROR_SIZE;
	h = SwapHeader(buffer);

	//make sure its actually an iqm
	if (memcmp(h.id, IQM_MAGIC, sizeof(h.id)))
		return PICO_PMV_ERROR_IDENT;
	//v1 is flawed, we don't know about anything higher either.
	if (h.version != 2)
		return PICO_PMV_ERROR_VERSION;
	//make sure its not truncated
	if ((size_t)h.filesize != (size_t)bufSize)
		return PICO_PMV_ERROR_SIZE;

	//looks like we can probably use it.
	return PICO_PMV_OK;
}

// _iqm_load() loads an interquake model file.
static picoModel_t *_iqm_load( PM_PARAMS_LOAD ){
	picoModel_t     *picoModel;
	picoSurface_t   *picoSurface;
	picoShader_t    *picoShader;
	const float *inf;
	const byte *inb;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;

	iqmHeader_t h;
	iqmmesh_t m;
	iqmvertexarray_t a;
	size_t s, t, j, i;
	const char *stringtable;
	char skinname[512];
	const unsigned int *tri;

	//just in case
	if (_iqm_canload(fileName, buffer, bufSize) != PICO_PMV_OK)
	{
		_pico_printf( PICO_ERROR, "%s is not an IQM File!", fileName );
		return NULL;
	}
	h = SwapHeader(buffer);
	stringtable = (const char*)buffer + h.ofs_text;

	// do frame check
	if ( h.num_anims != 0 ) {
		_pico_printf( PICO_WARNING, "%s has animations! Using base pose only.", fileName );
	}

	/* create new pico model */
	picoModel = PicoNewModel();
	if ( picoModel == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model" );
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, 1 ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	for (s = 0; s < h.num_meshes; s++)
	{
		m = ((const iqmmesh_t*)((const char*)buffer + h.ofs_meshes))[s];
		m.first_triangle = _pico_little_long(m.first_triangle);
		m.first_vertex = _pico_little_long(m.first_vertex);
		m.material = _pico_little_long(m.material);
		m.name = _pico_little_long(m.name);
		m.num_triangles = _pico_little_long(m.num_triangles);
		m.num_vertexes = _pico_little_long(m.num_vertexes);

		// allocate new pico surface
		picoSurface = PicoNewSurface( picoModel );
		if ( picoSurface == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
			PicoFreeModel( picoModel );
			return NULL;
		}

		// detox Skin name
		memcpy(skinname, stringtable+m.material, sizeof(skinname));
		_pico_setfext( skinname, "" );
		_pico_unixify( skinname );

		PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );
		PicoSetSurfaceName( picoSurface, stringtable+m.name );
		picoShader = PicoNewShader( picoModel );
		if ( picoShader == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model shader" );
			PicoFreeModel( picoModel );
			return NULL;
		}

		PicoSetShaderName( picoShader, skinname );

		// associate current surface with newly created shader
		PicoSetSurfaceShader( picoSurface, picoShader );


		// spew the surface's indexes
		tri = (const unsigned int *)((const char *)buffer+h.ofs_triangles) + m.first_triangle*3;
		for (t = 0; t < m.num_triangles*3; t++)
			PicoSetSurfaceIndex( picoSurface, t, _pico_little_long(*tri++) - m.first_vertex );

		for ( j = 0; j < h.num_vertexarrays; j++)
		{
			a = ((const iqmvertexarray_t*)((const char*)buffer + h.ofs_vertexarrays))[j];
			a.flags = _pico_little_long(a.flags);
			a.format = _pico_little_long(a.format);
			a.offset = _pico_little_long(a.offset);
			a.size = _pico_little_long(a.size);
			a.type = _pico_little_long(a.type);

			switch(a.type)
			{
			case IQM_POSITION:
				if (a.format == IQM_FLOAT && a.size >= 3)
				{
					inf = (const float*)((const char *)buffer + a.offset) + m.first_vertex*a.size;
					for ( i = 0; i < m.num_vertexes; i++, inf += a.size )
					{
						xyz[0] = _pico_little_float(inf[0]);
						xyz[1] = _pico_little_float(inf[1]);
						xyz[2] = _pico_little_float(inf[2]);
						PicoSetSurfaceXYZ( picoSurface, i, xyz );
					}
				}
				break;
			case IQM_TEXCOORD:
				if (a.format == IQM_FLOAT && a.size >= 2)
				{
					inf = (const float*)((const char *)buffer + a.offset) + m.first_vertex*a.size;
					for ( i = 0; i < m.num_vertexes; i++, inf += a.size )
					{
						st[0] = _pico_little_float(inf[0]);
						st[1] = _pico_little_float(inf[1]);
						PicoSetSurfaceST( picoSurface, 0, i, st );
					}
				}
				break;
			case IQM_NORMAL:
				if (a.format == IQM_FLOAT && a.size >= 3)
				{
					inf = (const float*)((const char *)buffer + a.offset) + m.first_vertex*a.size;
					for ( i = 0; i < m.num_vertexes; i++, inf += a.size )
					{
						normal[0] = _pico_little_float(inf[0]);
						normal[1] = _pico_little_float(inf[1]);
						normal[2] = _pico_little_float(inf[2]);
						PicoSetSurfaceNormal( picoSurface, i, normal );
					}
				}
				break;
			case IQM_COLOR:
				if (a.format == IQM_UBYTE && a.size >= 3)
				{
					inb = (const byte*)((const char *)buffer + a.offset) + m.first_vertex*a.size;
					for ( i = 0; i < m.num_vertexes; i++, inb += a.size )
					{
						color[0] = inb[0];
						color[1] = inb[1];
						color[2] = inb[2];
						color[3] = (a.size>=4)?inb[3]:255;
						PicoSetSurfaceColor( picoSurface, 0, i, color );
					}
				}
				else if (a.format == IQM_FLOAT && a.size >= 3)
				{
					inf = (const float*)((const char *)buffer + a.offset) + m.first_vertex*a.size;
					for ( i = 0; i < m.num_vertexes; i++, inf += a.size )
					{
						color[0] = inf[0]*255;
						color[1] = inf[1]*255;
						color[2] = inf[2]*255;
						color[3] = (a.size>=4)?inf[3]*255:255;
						PicoSetSurfaceColor( picoSurface, 0, i, color );
					}
				}
				break;
			case IQM_TANGENT:
			case IQM_BLENDINDEXES:
			case IQM_BLENDWEIGHTS:
			case IQM_CUSTOM:
				break;	// these attributes are not relevant.
			}
		}
	}

	return picoModel;
}

/* pico file format module definition */
const picoModule_t picoModuleIQM =
{
	"0.1",				/* module version string */
	"InterQuake Model",		/* module display name */
	"Spoike",			/* author's name */
	"2018-2021 FTE Team",		/* module copyright */
	{
		"iqm", NULL, NULL, NULL	/* default extensions to use */
	},
	_iqm_canload,			/* validation routine */
	_iqm_load,			/* load routine */
	NULL,				/* save validation routine */
	NULL				/* save routine */
};
