/*
   This code is based on source provided under the terms of the Id Software
   LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with the
   GtkRadiant sources (see LICENSE_ID). If you did not receive a copy of
   LICENSE_ID, please contact Id Software immediately at info@idsoftware.com.

   All changes and additions to the original source which have been developed by
   other contributors (see CONTRIBUTORS) are provided under the terms of the
   license the contributors choose (see LICENSE), to the extent permitted by the
   LICENSE_ID. If you did not receive a copy of the contributor license,
   please contact the GtkRadiant maintainers at info@gtkradiant.com immediately.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// trilib.c: library for loading triangles from an Alias triangle file
//

#include <stdio.h>
#include "cmdlib.h"
#include "mathlib.h"
#include "polyset.h"
#include "trilib.h"

// on disk representation of a face


#define FLOAT_START 99999.0
#define FLOAT_END   -FLOAT_START
#define MAGIC       123322

//#define NOISY 1

#if defined ( __linux__ ) || defined ( __APPLE__ )
#define strlwr strlower
#endif

typedef struct {
	float v[3];
} vector;

typedef struct
{
	vector n;    /* normal */
	vector p;    /* point */
	vector c;    /* color */
	float u;     /* u */
	float v;     /* v */
} aliaspoint_t;

typedef struct {
	aliaspoint_t pt[3];
} tf_triangle;


static void ByteSwapTri( tf_triangle *tri ){
	int i;

	for ( i = 0 ; i < sizeof( tf_triangle ) / 4 ; i++ )
	{
		( (int *)tri )[i] = BigLong( ( (int *)tri )[i] );
	}
}

static void ReadPolysetGeometry( triangle_t *tripool, FILE *input, int count, triangle_t *ptri ){
	tf_triangle tri;
	int i;

	for ( i = 0; i < count; ++i ) {
		int j;

		fread( &tri, sizeof( tf_triangle ), 1, input );
		ByteSwapTri( &tri );
		for ( j = 0 ; j < 3 ; j++ )
		{
			int k;

			for ( k = 0 ; k < 3 ; k++ )
			{
				ptri->verts[j][k] = tri.pt[j].p.v[k];
				ptri->normals[j][k] = tri.pt[j].n.v[k];
//				ptri->colors[j][k] = tri.pt[j].c.v[k];
			}

			ptri->texcoords[j][0] = tri.pt[j].u;
			ptri->texcoords[j][1] = tri.pt[j].v;
		}

		ptri++;
		if ( ( ptri - tripool ) >= POLYSET_MAXTRIANGLES ) {
			Error( "Error: too many triangles; increase POLYSET_MAXTRIANGLES\n" );
		}
	}
}

void TRI_LoadPolysets( const char *filename, polyset_t **ppPSET, int *numpsets ){
	FILE        *input;
	float start;
	char name[256], tex[256];
	int i, count, magic, pset = 0;
	triangle_t  *ptri;
	polyset_t   *pPSET;
	int iLevel;
	int exitpattern;
	float t;

	t = -FLOAT_START;
	*( (unsigned char *)&exitpattern + 0 ) = *( (unsigned char *)&t + 3 );
	*( (unsigned char *)&exitpattern + 1 ) = *( (unsigned char *)&t + 2 );
	*( (unsigned char *)&exitpattern + 2 ) = *( (unsigned char *)&t + 1 );
	*( (unsigned char *)&exitpattern + 3 ) = *( (unsigned char *)&t + 0 );

	if ( ( input = fopen( filename, "rb" ) ) == 0 ) {
		Error( "reader: could not open file '%s'", filename );
	}

	iLevel = 0;

	fread( &magic, sizeof( int ), 1, input );
	if ( BigLong( magic ) != MAGIC ) {
		Error( "%s is not a Alias object separated triangle file, magic number is wrong.", filename );
	}

	pPSET = calloc( 1, POLYSET_MAXPOLYSETS * sizeof( polyset_t ) );
	ptri = calloc( 1, POLYSET_MAXTRIANGLES * sizeof( triangle_t ) );

	*ppPSET = pPSET;

	while ( feof( input ) == 0 ) {
		if ( fread( &start,  sizeof( float ), 1, input ) < 1 ) {
			break;
		}
		*(int *)&start = BigLong( *(int *)&start );
		if ( *(int *)&start != exitpattern ) {
			if ( start == FLOAT_START ) {
				/* Start of an object or group of objects. */
				i = -1;
				do {
					/* There are probably better ways to read a string from */
					/* a file, but this does allow you to do error checking */
					/* (which I'm not doing) on a per character basis.      */
					++i;
					fread( &( name[i] ), sizeof( char ), 1, input );
				} while ( name[i] != '\0' );

				if ( i != 0 ) {
					strncpy( pPSET[pset].name, name, sizeof( pPSET[pset].name ) - 1 );
				}
				else{
					strcpy( pPSET[pset].name, "(unnamed)" );
				}
				strlwr( pPSET[pset].name );

//				indent();
//				fprintf(stdout,"OBJECT START: %s\n",name);
				fread( &count, sizeof( int ), 1, input );
				count = BigLong( count );
				++iLevel;
				if ( count != 0 ) {
//					indent();
//					fprintf(stdout,"NUMBER OF TRIANGLES: %d\n",count);

					i = -1;
					do {
						++i;
						fread( &( tex[i] ), sizeof( char ), 1, input );
					} while ( tex[i] != '\0' );

/*
                    if ( i != 0 )
                        strncpy( pPSET[pset].texname, tex, sizeof( pPSET[pset].texname ) - 1 );
                    else
                        strcpy( pPSET[pset].texname, "(unnamed)" );
                    strlwr( pPSET[pset].texname );
 */

//					indent();
//					fprintf(stdout,"  Object texture name: '%s'\n",tex);
				}

				/* Else (count == 0) this is the start of a group, and */
				/* no texture name is present. */
			}
			else if ( start == FLOAT_END ) {
				/* End of an object or group. Yes, the name should be */
				/* obvious from context, but it is in here just to be */
				/* safe and to provide a little extra information for */
				/* those who do not wish to write a recursive reader. */
				/* Mea culpa. */
				--iLevel;
				i = -1;
				do {
					++i;
					fread( &( name[i] ), sizeof( char ), 1, input );
				} while ( name[i] != '\0' );

				if ( i != 0 ) {
					strncpy( pPSET[pset].name, name, sizeof( pPSET[pset].name ) - 1 );
				}
				else{
					strcpy( pPSET[pset].name, "(unnamed)" );
				}

				strlwr( pPSET[pset].name );

//				indent();
//				fprintf(stdout,"OBJECT END: %s\n",name);
				continue;
			}
		}

//
// read the triangles
//
		if ( count > 0 ) {
			pPSET[pset].triangles = ptri;
			ReadPolysetGeometry( pPSET[0].triangles, input, count, ptri );
			ptri += count;
			pPSET[pset].numtriangles = count;
			if ( ++pset >= POLYSET_MAXPOLYSETS ) {
				Error( "Error: too many polysets; increase POLYSET_MAXPOLYSETS\n" );
			}
		}
	}

	*numpsets = pset;

	fclose( input );
}
