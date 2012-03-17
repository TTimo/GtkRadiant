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

#ifndef REFERENCE_H
#define REFERENCE_H

#include "placement.h"

#define MAX_REFPOINTS       16
#define REF_MINCULLTIME     1.0

typedef struct Reference_s
{
	int activecount;
	Placement_t placement;
} Reference_t;

typedef struct LERPedReferences_s
{
	int refType;
	int         *jointIDs;
	float lastUpdate;
	Reference_t references[MAX_REFPOINTS];
	Reference_t oldReferences[MAX_REFPOINTS];
} LERPedReferences_t;

// Reference Types
enum {
	REF_NULL = -1,
	REF_CORVUS, //0
	REF_INSECT, //1
	REF_PRIESTESS, //2
	REF_MORK, //3
	NUM_REFERENCED //4
};

// Corvus Reference Points
enum {
	CORVUS_LEFTHAND, //0
	CORVUS_RIGHTHAND,
	CORVUS_LEFTFOOT,
	CORVUS_RIGHTFOOT,
	CORVUS_STAFF,
	CORVUS_BLADE,
	CORVUS_HELL_HEAD,
	NUM_REFERENCES_CORVUS //7
};

// Tchekrik Reference Points
enum {
	INSECT_STAFF, //0
	INSECT_SWORD,
	INSECT_SPEAR,
	INSECT_RIGHTFOOT,
	INSECT_LEFTFOOT,
	NUM_REFERENCES_INSECT //5
};

// High Priestess Reference Points
enum {
	PRIESTESS_BACK, //0
	PRIESTESS_STAFF,
	PRIESTESS_LHAND,
	PRIESTESS_RHAND,
	PRIESTESS_RFOOT,
	PRIESTESS_LFOOT,
	NUM_REFERENCES_PRIESTESS //6
};

// Morcalavin Reference Points
enum
{
	MORK_STAFFREF, //0
	MORK_RFOOTREF, //1
	MORK_LFOOTREF, //2
	MORK_RHANDREF, //3
	MORK_LHANDREF, //4
	MORK_LEYEREF, //5
	MORK_REYEREF, //6
	NUM_REFERENCES_MORK //7
};

#define CORVUS_LIMBS_MASK   ( ( 1 << CORVUS_LEFTHAND ) | ( 1 << CORVUS_RIGHTHAND ) | ( 1 << CORVUS_LEFTFOOT ) | ( 1 << CORVUS_RIGHTFOOT ) )
#define CORVUS_WEAPON_MASK  ( ( 1 << CORVUS_STAFF ) | ( 1 << CORVUS_BLADE ) | ( 1 << CORVUS_HELL_HEAD ) )
#define CORVUS_MASK         ( CORVUS_LIMBS_MASK | CORVUS_WEAPON_MASK )

#define INSECT_MASK         ( ( 1 << INSECT_STAFF ) | ( 1 << INSECT_SWORD ) | ( 1 << INSECT_SPEAR ) | ( 1 << INSECT_RIGHTFOOT ) | ( 1 << INSECT_LEFTFOOT ) )

#define PRIESTESS_MASK      ( ( 1 << PRIESTESS_BACK ) | ( 1 << PRIESTESS_STAFF ) | ( 1 << PRIESTESS_LHAND ) | ( 1 << PRIESTESS_RHAND ) | ( 1 << PRIESTESS_RFOOT ) | ( 1 << PRIESTESS_LFOOT ) )

#define MORK_MASK           ( ( 1 << MORK_STAFFREF ) | ( 1 << MORK_RFOOTREF ) | ( 1 << MORK_LFOOTREF ) | ( 1 << MORK_RHANDREF ) | ( 1 << MORK_LHANDREF ) | ( 1 << MORK_LEYEREF ) | ( 1 << MORK_REYEREF ) )

extern char *referenceRootNames[];
extern int referenceRootNameOffsets[];
extern int numReferences[];

void EnableRefPoints( LERPedReferences_t *refInfo, int mask );
void DisableRefPoints( LERPedReferences_t *refInfo, int mask );

void InitReferenceMngr();
void ReleaseReferenceMngr();

LERPedReferences_t *LERPedReferences_new( int init_refType );
void LERPedReferences_delete( LERPedReferences_t *toDelete );

#endif
