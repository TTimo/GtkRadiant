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

#define LIGHT_SHADOWS_C

#include "light.h"
#include "inout.h"



/* -------------------------------------------------------------------------------

ydnar: this code deals with shadow volume bsps

------------------------------------------------------------------------------- */

typedef struct shadowNode_s
{
	vec4_t	plane;
	int		children[ 2 ];
}
shadowNode_t;

int				numShadowNodes;
shadowNode_t	*shadowNodes;



/*
AddShadow()
adds a shadow, returning the index into the shadow list
*/



/*
MakeShadowFromPoints()
creates a shadow volume from 4 points (the first being the light origin)
*/



/*
SetupShadows()
sets up the shadow volumes for all lights in the world
*/

void SetupShadows( void )
{
	int				i, j, s;
	light_t			*light;
	dleaf_t			*leaf;
	dsurface_t		*ds;
	surfaceInfo_t	*info;
	shaderInfo_t	*si;
	byte			*tested;

	
	/* early out for weird cases where there are no lights */
	if( lights == NULL )
		return;
	
	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupShadows ---\n" );
	
	/* allocate a surface test list */
	tested = safe_malloc( numDrawSurfaces / 8 + 1 );
	
	/* walk the list of lights */
	for( light = lights; light != NULL; light = light->next )
	{
		/* do some early out testing */
		if( light->cluster < 0 )
			continue;
		
		/* clear surfacetest list */
		memset( tested, 0, numDrawSurfaces / 8 + 1 );
		
		/* walk the bsp leaves */
		for( i = 0, leaf = dleafs; i < numleafs; i++, leaf++ )
		{
			/* in pvs? */
			if( ClusterVisible( light->cluster, leaf->cluster ) == qfalse )
				continue;
			
			/* walk the surface list for this leaf */
			for( j = 0; j < leaf->numLeafSurfaces; j++ )
			{
				/* don't filter a surface more than once */
				s = dleafsurfaces[ leaf->firstLeafSurface + j ];
				if( tested[ s >> 3 ] & (1 << (s & 7)) )
					continue;
				tested[ s >> 3 ] |= (1 << (s & 7));
				
				/* get surface and info */
				ds = &drawSurfaces[ s ];
				info = &surfaceInfos[ s ];
				si = info->si;
				
				/* don't create shadow volumes from translucent surfaces */
				if( si->contents & CONTENTS_TRANSLUCENT )
					continue;
			}
		}
	}
}