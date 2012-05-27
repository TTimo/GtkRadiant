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
#define BRUSH_PRIMIT_C



/* dependencies */
#include "q3map2.h"



/* -------------------------------------------------------------------------------

functions

------------------------------------------------------------------------------- */

/*
ComputeAxisBase()
computes the base texture axis for brush primitive texturing
note: ComputeAxisBase here and in editor code must always BE THE SAME!
warning: special case behaviour of atan2( y, x ) <-> atan( y / x ) might not be the same everywhere when x == 0
rotation by (0,RotY,RotZ) assigns X to normal
*/

void ComputeAxisBase( vec3_t normal, vec3_t texX, vec3_t texY )
{
	vec_t	RotY, RotZ;
	
	
	/* do some cleaning */
	if( fabs( normal[ 0 ] ) < 1e-6 )
		normal[ 0 ]= 0.0f;
	if( fabs( normal[ 1 ] ) < 1e-6 )
		normal[ 1 ]=0.0f;
	if( fabs( normal[ 2 ] ) < 1e-6 )
		normal[ 2 ] = 0.0f;
	
	/* compute the two rotations around y and z to rotate x to normal */
	RotY = -atan2( normal[ 2 ], sqrt( normal[ 1 ] * normal[ 1 ] + normal[ 0 ] * normal[ 0 ]) );
	RotZ = atan2( normal[ 1 ], normal[ 0 ] );
	
	/* rotate (0,1,0) and (0,0,1) to compute texX and texY */
	texX[ 0 ] = -sin( RotZ );
	texX[ 1 ] = cos( RotZ );
	texX[ 2 ] = 0;
	
	/* the texY vector is along -z (t texture coorinates axis) */
	texY[ 0 ] = -sin( RotY ) * cos( RotZ );
	texY[ 1 ] = -sin( RotY ) * sin( RotZ );
	texY[ 2 ] = -cos( RotY );
}
