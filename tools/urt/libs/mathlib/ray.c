/*
   This code provided under the terms of the Id Software
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

#include "mathlib.h"
#include <float.h>

vec3_t identity = { 0,0,0 };

void ray_construct_for_vec3( ray_t *ray, const vec3_t origin, const vec3_t direction ){
	VectorCopy( origin, ray->origin );
	VectorCopy( direction, ray->direction );
}

void ray_transform( ray_t *ray, const m4x4_t matrix ){
	m4x4_transform_point( matrix, ray->origin );
	m4x4_transform_normal( matrix, ray->direction );
}

vec_t ray_intersect_point( const ray_t *ray, const vec3_t point, vec_t epsilon, vec_t divergence ){
	vec3_t displacement;
	vec_t depth;

	// calc displacement of test point from ray origin
	VectorSubtract( point, ray->origin, displacement );
	// calc length of displacement vector along ray direction
	depth = DotProduct( displacement, ray->direction );
	if ( depth < 0.0f ) {
		return (vec_t)FLT_MAX;
	}
	// calc position of closest point on ray to test point
	VectorMA( ray->origin, depth, ray->direction, displacement );
	// calc displacement of test point from closest point
	VectorSubtract( point, displacement, displacement );
	// calc length of displacement, subtract depth-dependant epsilon
	if ( VectorLength( displacement ) - ( epsilon + ( depth * divergence ) ) > 0.0f ) {
		return (vec_t)FLT_MAX;
	}
	return depth;
}

// Tomas Moller and Ben Trumbore. Fast, minimum storage ray-triangle intersection. Journal of graphics tools, 2(1):21-28, 1997

#define EPSILON 0.000001

vec_t ray_intersect_triangle( const ray_t *ray, qboolean bCullBack, const vec3_t vert0, const vec3_t vert1, const vec3_t vert2 ){
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det,inv_det;
	float u, v;
	vec_t depth = (vec_t)FLT_MAX;

	/* find vectors for two edges sharing vert0 */
	VectorSubtract( vert1, vert0, edge1 );
	VectorSubtract( vert2, vert0, edge2 );

	/* begin calculating determinant - also used to calculate U parameter */
	CrossProduct( ray->direction, edge2, pvec );

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DotProduct( edge1, pvec );

	if ( bCullBack == qtrue ) {
		if ( det < EPSILON ) {
			return depth;
		}

		// calculate distance from vert0 to ray origin
		VectorSubtract( ray->origin, vert0, tvec );

		// calculate U parameter and test bounds
		u = DotProduct( tvec, pvec );
		if ( u < 0.0 || u > det ) {
			return depth;
		}

		// prepare to test V parameter
		CrossProduct( tvec, edge1, qvec );

		// calculate V parameter and test bounds
		v = DotProduct( ray->direction, qvec );
		if ( v < 0.0 || u + v > det ) {
			return depth;
		}

		// calculate t, scale parameters, ray intersects triangle
		depth = DotProduct( edge2, qvec );
		inv_det = 1.0f / det;
		depth *= inv_det;
		//u *= inv_det;
		//v *= inv_det;
	}
	else
	{
		/* the non-culling branch */
		if ( det > -EPSILON && det < EPSILON ) {
			return depth;
		}
		inv_det = 1.0f / det;

		/* calculate distance from vert0 to ray origin */
		VectorSubtract( ray->origin, vert0, tvec );

		/* calculate U parameter and test bounds */
		u = DotProduct( tvec, pvec ) * inv_det;
		if ( u < 0.0 || u > 1.0 ) {
			return depth;
		}

		/* prepare to test V parameter */
		CrossProduct( tvec, edge1, qvec );

		/* calculate V parameter and test bounds */
		v = DotProduct( ray->direction, qvec ) * inv_det;
		if ( v < 0.0 || u + v > 1.0 ) {
			return depth;
		}

		/* calculate t, ray intersects triangle */
		depth = DotProduct( edge2, qvec ) * inv_det;
	}
	return depth;
}

vec_t ray_intersect_plane( const ray_t* ray, const vec3_t normal, vec_t dist ){
	return -( DotProduct( normal, ray->origin ) - dist ) / DotProduct( ray->direction, normal );
}
