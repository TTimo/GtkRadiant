/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

#if !defined(INCLUDED_MDLFORMAT_H)
#define INCLUDED_MDLFORMAT_H

typedef unsigned char byte;

const unsigned char MDL_IDENT[4] = { 'I', 'D', 'P', 'O', };

#define MDL_VERSION	6

#define MDL_ONSEAM				0x0020

typedef enum { MDL_FRAME_SINGLE=0, MDL_FRAME_GROUP } aliasframetype_t;

typedef enum { MDL_SKIN_SINGLE=0, MDL_SKIN_GROUP } aliasskintype_t;

typedef float float3[3];

typedef struct {
	byte ident[4];
	int			version;
	float3		scale;
	float3		scale_origin;
	float		boundingradius;
	float3		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	int	synctype;
	int			flags;
	float		size;
} mdlHeader_t;

// TODO: could be shorts

typedef struct {
	int		onseam;
	int		s;
	int		t;
} mdlSt_t;

typedef struct dtriangle_s {
	int					facesfront;
	int					vertindex[3];
} mdlTriangle_t;

#define MDL_FACES_FRONT				1

// This mirrors trivert_t in trilib.h, is present so Quake knows how to
// load this data

typedef struct {
	byte	v[3];
	byte	lightnormalindex;
} mdlXyzNormal_t;

const int MDL_XYZNORMAL_SIZE = 4;

typedef struct {
	mdlXyzNormal_t	bboxmin;	// lightnormal isn't used
	mdlXyzNormal_t	bboxmax;	// lightnormal isn't used
	char		name[16];	// frame name from grabbing
} mdlFrame_t;

const int MDL_FRAME_SIZE = (MDL_XYZNORMAL_SIZE * 2) + 16;

typedef struct {
	int			numframes;
	mdlXyzNormal_t	bboxmin;	// lightnormal isn't used
	mdlXyzNormal_t	bboxmax;	// lightnormal isn't used
} mdlFrameGroup_t;

const int MDL_FRAMEGROUP_SIZE = 4 + (MDL_XYZNORMAL_SIZE * 2);

typedef struct {
	int			numskins;
} mdlSkinGroup_t;

typedef struct {
	float	interval;
} mdlFrameInterval_t;

typedef struct {
	float	interval;
} mdlSkinInterval_t;

typedef struct {
	aliasframetype_t	type;
} mdlFrameType_t;

typedef struct {
	aliasskintype_t	type;
} mdlSkinType_t;

#endif
