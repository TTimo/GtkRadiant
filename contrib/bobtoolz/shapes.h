/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// TODO: implement all this stuff via DBrush class. started with DShape
// TODO: Auto Face Scaling, no need to pass parms, calculated via brush.

// Q3MAP stuff
#define FACE_DETAIL 0x8000000

// defines for polygon stuff
#define MAX_POLYGON_FACES   128

// generic (detail added 12/01/01, for AC+)
void AddFaceWithTexture( brush_t* brush, vec3_t va, vec3_t vb, vec3_t vc, const char* texture, bool detail );

// -------------
// ---caulked---
// -------------
void Build_Wedge( int dir, vec3_t min, vec3_t max, bool bUp );

// --------------
// ---textured---
// --------------
void BuildDoorsX2( vec3_t min, vec3_t max, bool bSclMainHor, bool bSclMainVert, bool bSclTrimHor, bool bSclTrimVert, const char* mainTexture, const char* trimTexture, int direction );
void Build_StairStep( vec3_t min, vec3_t max, const char* mainTexture, const char* riserTexture, int direction );
void Build_StairStep_Wedge( int dir, vec3_t min, vec3_t max, const char* mainTexture, const char* riserTexture, bool detail );
void BuildCornerStairs( vec3_t vMin, vec3_t vMax, int nSteps, const char* mainTexture, const char* riserTex );
// stairs stuff.

//void Build_Prism_Border(vec3_t min, vec3_t max, int nSides, int nBorder, bool bAlignTop = FALSE); //moved to DShape
//void Build_Prism_Ordinary(vec3_t min, vec3_t max, int nSides, bool bAlignTop = FALSE); //moved to DShape
//void Build_Prism_Efficient(vec3_t min, vec3_t max, int nSides, bool bAlignTop = FALSE); //moved to DShape
// polygon stuff.
