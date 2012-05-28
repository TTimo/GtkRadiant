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

#include "math_vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define M_PI        3.14159265358979323846  // matches value in gcc v2 math.h

#define LERP_DELTA 1e-6

idVec3 vec_zero( 0.0f, 0.0f, 0.0f );

Bounds boundsZero;

float idVec3::toYaw( void ) {
	float yaw;

	if ( ( y == 0 ) && ( x == 0 ) ) {
		yaw = 0;
	}
	else {
		yaw = atan2( y, x ) * 180 / M_PI;
		if ( yaw < 0 ) {
			yaw += 360;
		}
	}

	return yaw;
}

float idVec3::toPitch( void ) {
	float forward;
	float pitch;

	if ( ( x == 0 ) && ( y == 0 ) ) {
		if ( z > 0 ) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		forward = ( float )idSqrt( x * x + y * y );
		pitch = atan2( z, forward ) * 180 / M_PI;
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	return pitch;
}

/*
   angles_t idVec3::toAngles( void ) {
    float forward;
    float yaw;
    float pitch;

    if ( ( x == 0 ) && ( y == 0 ) ) {
        yaw = 0;
        if ( z > 0 ) {
            pitch = 90;
        } else {
            pitch = 270;
        }
    } else {
        yaw = atan2( y, x ) * 180 / M_PI;
        if ( yaw < 0 ) {
            yaw += 360;
        }

        forward = ( float )idSqrt( x * x + y * y );
        pitch = atan2( z, forward ) * 180 / M_PI;
        if ( pitch < 0 ) {
            pitch += 360;
        }
    }

    return angles_t( -pitch, yaw, 0 );
   }
 */

idVec3 LerpVector( idVec3 &w1, idVec3 &w2, const float t ) {
	float omega, cosom, sinom, scale0, scale1;

	cosom = w1 * w2;
	if ( ( 1.0 - cosom ) > LERP_DELTA ) {
		omega = acos( cosom );
		sinom = sin( omega );
		scale0 = sin( ( 1.0 - t ) * omega ) / sinom;
		scale1 = sin( t * omega ) / sinom;
	}
	else {
		scale0 = 1.0 - t;
		scale1 = t;
	}

	return ( w1 * scale0 + w2 * scale1 );
}

/*
   =============
   idVec3::string

   This is just a convenience function
   for printing vectors
   =============
 */
char *idVec3::string( void ) {
	static int index = 0;
	static char str[ 8 ][ 36 ];
	char    *s;

	// use an array so that multiple toString's won't collide
	s = str[ index ];
	index = ( index + 1 ) & 7;

	sprintf( s, "%.2f %.2f %.2f", x, y, z );

	return s;
}
