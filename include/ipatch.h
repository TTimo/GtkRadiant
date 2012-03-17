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

#ifndef _IPATCH_H_
#define _IPATCH_H_

//
// API for patch stuff
//

#define PATCH_MAJOR "patch"
// {4715565b-ab3a-49fa-841f-ee965b6d88a5}
static const GUID QERPatchTable_GUID =
{ 0x4715565b, 0xab3a, 0x49fa, { 0x84, 0x1f, 0xee, 0x96, 0x5b, 0x6d, 0x88, 0xa5 } };

typedef patchMesh_t* ( *PFN_PATCHALLOC )();
typedef patchMesh_t* ( *PFN_MAKENEWPATCH )();
typedef brush_t* ( *PFN_ADDBRUSHFORPATCH )( patchMesh_t *pm, bool bLinkToWorld );

struct _QERPatchTable
{
	int m_nSize;
	PFN_PATCHALLOC m_pfnPatch_Alloc;
	PFN_MAKENEWPATCH m_pfnMakeNewPatch;
	PFN_ADDBRUSHFORPATCH m_pfnAddBrushForPatch;
};

#ifdef USE_PATCHTABLE_DEFINE
#define __PATCHTABLENAME g_PatchTable
#define Patch_Alloc __PATCHTABLENAME.m_pfnPatch_Alloc
#define MakeNewPatch __PATCHTABLENAME.m_pfnMakeNewPatch
#define AddBrushForPatch __PATCHTABLENAME.m_pfnAddBrushForPatch
#endif

#endif
