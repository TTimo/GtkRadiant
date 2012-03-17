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

#if 0

/*
** ReindexTriangle
**
** Given a triangle_t, find which indices match into the associated
** surface's base triangles.
*/
static void ReindexTriangle( int surfno, triangle_t *pTri, int indices[3] ){
	int t, i;
	md3SurfaceData_t *pSurfData = &g_data.surfData[surfno];
	int matches[3][3];
	int numMatches = 0;


	indices[0] = -1;
	indices[1] = -1;
	indices[2] = -1;

	for ( i = 0; i < 3; i++ )
	{
		numMatches = 0;

		matches[i][0] = -1;
		matches[i][1] = -1;
		matches[i][2] = -1;

		for ( t = 0; t < pSurfData->header.numVerts; t++ )
		{
			if ( !VectorCompare( pTri->verts[i], pSurfData->baseVertexes[t].xyz ) ) {
				continue;
			}

/*
            if ( !VectorCompare( pTri->normals[i], pSurfData->baseVertexes[t].normal ) )
                continue;
            if ( pTri->texcoords[i][0] != pSurfData->baseVertexes[t].st[0] )
                continue;
            if ( pTri->texcoords[i][1] != pSurfData->baseVertexes[t].st[1] )
                continue;
 */

			matches[i][numMatches++] = t;
		}

		if ( indices[i] == -1 ) {
//			Error( "Could not ReindexTriangle, vertex not found" );
		}
	}

#if 0
	for ( t = 0; t < psets[i].numtriangles; t++ )
	{
		int b;

		bTri = &g_data.surfData[i].baseTriangles[t];

		for ( j = 0 ; j < 3 ; j++ )
		{
			bVert = &bTri->v[j];

			// get the xyz index
			for ( k = 0; k < g_data.surfData[i].header.numVerts; k++ )
			{
				if ( ( g_data.surfData[i].baseVertexes[k].st[0] == bVert->st[0] ) &&
					 ( g_data.surfData[i].baseVertexes[k].st[1] == bVert->st[1] ) &&
					 ( VectorCompare( bVert->xyz, g_data.surfData[i].baseVertexes[k].xyz ) ) &&
					 ( VectorCompare( bVert->normal, g_data.surfData[i].baseVertexes[k].normal ) ) ) {
					break;      // this vertex is already in the base vertex list
				}
			}

			if ( k == g_data.surfData[i].header.numVerts ) {      // new index
				g_data.surfData[i].baseVertexes[g_data.surfData[i].header.numVerts] = *bVert;
				g_data.surfData[i].header.numVerts++;
			}

			bVert->index = k;
		}
	}
#endif
}

const char  *FindFrameFile( const char *frame ){
	int time1;
	char file1[1024];
	static char retname[1024];
	char base[32];
	char suffix[32];
	const char  *s;

	if ( strstr( frame, "." ) ) {
		return frame;       // allready in dot format

	}
	// split 'run1' into 'run' and '1'
	s = frame + strlen( frame ) - 1;

	while ( s != frame && *s >= '0' && *s <= '9' )
		s--;

	strcpy( suffix, s + 1 );
	strcpy( base, frame );
	base[s - frame + 1] = 0;

	// check for 'run1.tri'
	sprintf( file1, "%s/%s%s.tri", g_cddir, base, suffix );
	time1 = FileTime( file1 );
	if ( time1 != -1 ) {
		sprintf( retname, "%s%s.tri", base, suffix );
		return retname;
	}

	// check for 'run.1'
	sprintf( file1, "%s/%s.%s",g_cddir, base, suffix );
	time1 = FileTime( file1 );
	if ( time1 != -1 ) {
		sprintf( retname, "%s.%s", base, suffix );
		return retname;
	}

	Error( "frame %s could not be found",frame );
	return NULL;
}

#endif
