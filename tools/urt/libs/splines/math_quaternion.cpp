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

#include "math_quaternion.h"
#include "math_matrix.h"

void toQuat( idVec3 &src, quat_t &dst ) {
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
	dst.w = 0.0f;
}

void toQuat( angles_t &src, quat_t &dst ) {
	mat3_t temp;

	toMatrix( src, temp );
	toQuat( temp, dst );
}

void toQuat( mat3_t &src, quat_t &dst ) {
	float trace;
	float s;
	int i;
	int j;
	int k;

	static int next[ 3 ] = { 1, 2, 0 };

	trace = src[ 0 ][ 0 ] + src[ 1 ][ 1 ] + src[ 2 ][ 2 ];
	if ( trace > 0.0f ) {
		s = ( float )sqrt( trace + 1.0f );
		dst.w = s * 0.5f;
		s = 0.5f / s;

		dst.x = ( src[ 2 ][ 1 ] - src[ 1 ][ 2 ] ) * s;
		dst.y = ( src[ 0 ][ 2 ] - src[ 2 ][ 0 ] ) * s;
		dst.z = ( src[ 1 ][ 0 ] - src[ 0 ][ 1 ] ) * s;
	}
	else {
		i = 0;
		if ( src[ 1 ][ 1 ] > src[ 0 ][ 0 ] ) {
			i = 1;
		}
		if ( src[ 2 ][ 2 ] > src[ i ][ i ] ) {
			i = 2;
		}

		j = next[ i ];
		k = next[ j ];

		s = ( float )sqrt( ( src[ i ][ i ] - ( src[ j ][ j ] + src[ k ][ k ] ) ) + 1.0f );
		dst[ i ] = s * 0.5f;

		s = 0.5f / s;

		dst.w       = ( src[ k ][ j ] - src[ j ][ k ] ) * s;
		dst[ j ]    = ( src[ j ][ i ] + src[ i ][ j ] ) * s;
		dst[ k ]    = ( src[ k ][ i ] + src[ i ][ k ] ) * s;
	}
}
