/* -------------------------------------------------------------------------------

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

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define SURFACE_FUR_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   ydnar: fur module

   ------------------------------------------------------------------------------- */

/*
   Fur()
   runs the fur processing algorithm on a map drawsurface
 */

void Fur( mapDrawSurface_t *ds ){
	int i, j, k, numLayers;
	float offset, fade, a;
	mapDrawSurface_t    *fur;
	bspDrawVert_t       *dv;


	/* dummy check */
	if ( ds == NULL || ds->fur || ds->shaderInfo->furNumLayers < 1 ) {
		return;
	}

	/* get basic info */
	numLayers = ds->shaderInfo->furNumLayers;
	offset = ds->shaderInfo->furOffset;
	fade = ds->shaderInfo->furFade * 255.0f;

	/* debug code */
	//%	Sys_FPrintf( SYS_VRB, "Fur():  layers: %d  offset: %f   fade: %f  %s\n",
	//%		numLayers, offset, fade, ds->shaderInfo->shader );

	/* initial offset */
	for ( j = 0; j < ds->numVerts; j++ )
	{
		/* get surface vert */
		dv = &ds->verts[ j ];

		/* offset is scaled by original vertex alpha */
		a = (float) dv->color[ 0 ][ 3 ] / 255.0;

		/* offset it */
		VectorMA( dv->xyz, ( offset * a ), dv->normal, dv->xyz );
	}

	/* wash, rinse, repeat */
	for ( i = 1; i < numLayers; i++ )
	{
		/* clone the surface */
		fur = CloneSurface( ds, ds->shaderInfo );
		if ( fur == NULL ) {
			return;
		}

		/* set it to fur */
		fur->fur = qtrue;

		/* walk the verts */
		for ( j = 0; j < fur->numVerts; j++ )
		{
			/* get surface vert */
			dv = &ds->verts[ j ];

			/* offset is scaled by original vertex alpha */
			a = (float) dv->color[ 0 ][ 3 ] / 255.0;

			/* get fur vert */
			dv = &fur->verts[ j ];

			/* offset it */
			VectorMA( dv->xyz, ( offset * a * i ), dv->normal, dv->xyz );

			/* fade alpha */
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
			{
				a = (float) dv->color[ k ][ 3 ] - fade;
				if ( a > 255.0f ) {
					dv->color[ k ][ 3 ] = 255;
				}
				else if ( a < 0 ) {
					dv->color[ k ][ 3 ] = 0;
				}
				else{
					dv->color[ k ][ 3 ] = a;
				}
			}
		}
	}
}
