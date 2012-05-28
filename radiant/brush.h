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


// brush.h

// some usefull flags to control the behaviour of Brush_Build
extern bool g_bBuildWindingsNoTexBuild;

void        Brush_AddToList( brush_t *b, brush_t *lst );
void        Brush_Build( brush_t *b, bool bSnap = true, bool bMarkMap = true, bool bConvert = false, bool bFilterTest = true );
void    Brush_SetBuildWindingsNoTexBuild( bool bBuild );
void        Brush_BuildWindings( brush_t *b, bool bSnap = true );
brush_t*    Brush_Clone( brush_t *b );
brush_t*    Brush_FullClone( brush_t *b );
brush_t*    Brush_Create( vec3_t mins, vec3_t maxs, texdef_t *texdef );
void    Brush_Resize( brush_t *b, vec3_t vMin, vec3_t vMax );
void    Brush_FaceDraw( face_t *face, int nGLState );
void        Brush_Draw( brush_t *b );
void        Brush_DrawXY( brush_t *b, int nViewType );
// set bRemoveNode to false to avoid trying to delete the item in group view tree control
void        Brush_Free( brush_t *b, bool bRemoveNode = true );
int         Brush_MemorySize( brush_t *b );
void        Brush_MakeSided( int sides );
void        Brush_MakeSidedCone( int sides );
void        Brush_Move( brush_t *b, const vec3_t move, bool bSnap = true );
int     Brush_MoveVertex( brush_t *b, vec3_t vertex, vec3_t delta, vec3_t end, bool bSnap = true );
void        Brush_ResetFaceOriginals( brush_t *b );
face_t*     Brush_Ray( vec3_t origin, vec3_t dir, brush_t *b, float *dist, int nFlags = 0 );
void        Brush_RemoveFromList( brush_t *b );
// bCaulk means the faces created during the operation will be caulked, this is used in conjunction with g_PrefsDlg.m_bClipCaulk
void        Brush_SplitBrushByFace( brush_t *in, face_t *f, brush_t **front, brush_t **back, qboolean bCaulk = false );
void        Brush_SelectFaceForDragging( brush_t *b, face_t *f, qboolean shear );
void        Brush_SetTexture( brush_t *b, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, IPluginTexdef* pPlugTexdef = (IPluginTexdef*)NULL );
void        Brush_SideSelect( brush_t *b, vec3_t origin, vec3_t dir, qboolean shear );
void        Brush_SnapToGrid( brush_t *pb );
void        Brush_Rotate( brush_t *b, vec3_t vAngle, vec3_t vOrigin, bool bBuild = true );
void        Brush_MakeSidedSphere( int sides );
//void		Brush_Write (brush_t *b, FILE *f);
//void		Brush_Write (brush_t *b, MemStream* pMemFile);
void        Brush_RemoveEmptyFaces( brush_t *b );
winding_t*  Brush_MakeFaceWinding( brush_t *b, face_t *face );

void Brush_RefreshShader( brush_t *b );

int         AddPlanept( float *f );
float       SetShadeForPlane( plane_t *p );

face_t* Face_Alloc( void );
void        Face_Free( face_t *f );
face_t* Face_Clone( face_t *f );
void    Face_SetShader( face_t *face, const char *name );
/*!
   faster version if you know the IShader already
   (instead of hash table lookup by name)
 */
void    Face_SetShader( face_t *face, IShader *shader );
void    Face_MakePlane( face_t *f );
void        Face_Draw( face_t *face );
void Face_TextureVectors( face_t * f, float STfromXYZ[2][4] );
void        SetFaceTexdef( face_t *f, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, IPluginTexdef* pPlugTexdef = NULL );

void Face_FitTexture( face_t * face, int nHeight, int nWidth );
void Brush_FitTexture( brush_t *b, int nHeight, int nWidth );
//void Brush_SetEpair(brush_t *b, const char *pKey, const char *pValue);
//const char* Brush_GetKeyValue(brush_t *b, const char *pKey);
brush_t *Brush_Alloc();
const char* Brush_Name( brush_t *b );

//eclass_t* HasModel(brush_t *b);
void aabb_draw( const aabb_t *aabb, int mode );
