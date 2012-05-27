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


typedef struct
{
	int		numpoints;
	vec3_t	p[4];		// variable sized
} winding_t;

#define	MAX_POINTS_ON_WINDING	64

// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1
#endif

winding_t	*AllocWinding (int points);
vec_t	WindingArea (winding_t *w);
void	WindingCenter (winding_t *w, vec3_t center);
void	ClipWindingEpsilon (winding_t *in, vec3_t normal, vec_t dist, 
				vec_t epsilon, winding_t **front, winding_t **back);
winding_t	*ChopWinding (winding_t *in, vec3_t normal, vec_t dist);
winding_t	*CopyWinding (winding_t *w);
winding_t	*ReverseWinding (winding_t *w);
winding_t	*BaseWindingForPlane (vec3_t normal, vec_t dist);
void	CheckWinding (winding_t *w);
void	WindingPlane (winding_t *w, vec3_t normal, vec_t *dist);
void	RemoveColinearPoints (winding_t *w);
int		WindingOnPlaneSide (winding_t *w, vec3_t normal, vec_t dist);
void	FreeWinding (winding_t *w);
void	WindingBounds (winding_t *w, vec3_t mins, vec3_t maxs);

void	AddWindingToConvexHull( winding_t *w, winding_t **hull, vec3_t normal );

void	ChopWindingInPlace (winding_t **w, vec3_t normal, vec_t dist, vec_t epsilon);
// frees the original if clipped

void pw(winding_t *w);
