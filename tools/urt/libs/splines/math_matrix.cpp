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

#include "q_shared.h"

mat3_t mat3_default( idVec3( 1, 0, 0 ), idVec3( 0, 1, 0 ), idVec3( 0, 0, 1 ) );

void toMatrix( quat_t const &src, mat3_t &dst ) {
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = src.x + src.x;
	y2 = src.y + src.y;
	z2 = src.z + src.z;

	xx = src.x * x2;
	xy = src.x * y2;
	xz = src.x * z2;

	yy = src.y * y2;
	yz = src.y * z2;
	zz = src.z * z2;

	wx = src.w * x2;
	wy = src.w * y2;
	wz = src.w * z2;

	dst[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	dst[ 0 ][ 1 ] = xy - wz;
	dst[ 0 ][ 2 ] = xz + wy;

	dst[ 1 ][ 0 ] = xy + wz;
	dst[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	dst[ 1 ][ 2 ] = yz - wx;

	dst[ 2 ][ 0 ] = xz - wy;
	dst[ 2 ][ 1 ] = yz + wx;
	dst[ 2 ][ 2 ] = 1.0f - ( xx + yy );
}

void toMatrix( angles_t const &src, mat3_t &dst ) {
	float angle;
	static float sr, sp, sy, cr, cp, cy;    // static to help MS compiler fp bugs

	angle = src.yaw * ( M_PI * 2.0f / 360.0f );
	sy = sin( angle );
	cy = cos( angle );

	angle = src.pitch * ( M_PI * 2.0f / 360.0f );
	sp = sin( angle );
	cp = cos( angle );

	angle = src.roll * ( M_PI * 2.0f / 360.0f );
	sr = sin( angle );
	cr = cos( angle );

	dst[ 0 ].set( cp * cy, cp * sy, -sp );
	dst[ 1 ].set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	dst[ 2 ].set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
}

void toMatrix( idVec3 const &src, mat3_t &dst ) {
	angles_t sup = src;
	toMatrix( sup, dst );
}

void mat3_t::ProjectVector( const idVec3 &src, idVec3 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

void mat3_t::UnprojectVector( const idVec3 &src, idVec3 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

void mat3_t::Transpose( mat3_t &matrix ) {
	int i;
	int j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			matrix[ i ][ j ] = mat[ j ][ i ];
		}
	}
}

void mat3_t::Transpose( void ) {
	float temp;
	int i;
	int j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = i + 1; j < 3; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
		}
	}
}

mat3_t mat3_t::Inverse( void ) const {
	mat3_t inv( *this );

	inv.Transpose();

	return inv;
}

void mat3_t::Clear( void ) {
	mat[0].set( 1, 0, 0 );
	mat[1].set( 0, 1, 0 );
	mat[2].set( 0, 0, 1 );
}
