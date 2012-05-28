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

#ifndef __POLYSET_H__
#define __POLYSET_H__

#define POLYSET_MAXTRIANGLES    4096
#define POLYSET_MAXPOLYSETS     64

typedef float st_t[2];
typedef float rgb_t[3];

typedef struct {
	vec3_t verts[3];
	vec3_t normals[3];
	st_t texcoords[3];
} triangle_t;

typedef struct
{
	char name[100];
	char materialname[100];
	triangle_t *triangles;
	int numtriangles;
} polyset_t;

polyset_t *Polyset_LoadSets( const char *file, int *numpolysets, int maxTrisPerSet );
polyset_t *Polyset_CollapseSets( polyset_t *psets, int numpolysets );
polyset_t *Polyset_SplitSets( polyset_t *psets, int numpolysets, int *pNumNewPolysets, int maxTris );
void Polyset_SnapSets( polyset_t *psets, int numpolysets );
void Polyset_ComputeNormals( polyset_t *psets, int numpolysets );

#endif
