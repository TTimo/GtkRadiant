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

#include "qfiles.h"
#include "surfaceflags.h"

extern  int     bsp_version;

extern	int			nummodels;
extern	dmodel_t	dmodels[MAX_MAP_MODELS];

extern	int			numShaders;
extern	dshader_t	dshaders[MAX_MAP_MODELS];

extern	int			entdatasize;
extern	char		dentdata[MAX_MAP_ENTSTRING];

extern	int			numleafs;
extern	dleaf_t		dleafs[MAX_MAP_LEAFS];

extern	int			numplanes;
extern	dplane_t	dplanes[MAX_MAP_PLANES];

extern	int			numnodes;
extern	dnode_t		dnodes[MAX_MAP_NODES];

extern	int			numleafsurfaces;
extern	int			dleafsurfaces[MAX_MAP_LEAFFACES];

extern	int			numleafbrushes;
extern	int			dleafbrushes[MAX_MAP_LEAFBRUSHES];

extern	int			numbrushes;
extern	dbrush_t	dbrushes[MAX_MAP_BRUSHES];

extern	int			numbrushsides;
extern	dbrushside_t	dbrushsides[MAX_MAP_BRUSHSIDES];

void SetLightBytes(int n);
extern	int			numLightBytes;
extern	byte		*lightBytes;

void SetGridPoints(int n);
extern	int			numGridPoints;
extern	byte		*gridData;

extern	int			numVisBytes;
extern	byte		visBytes[MAX_MAP_VISIBILITY];

void SetDrawVerts(int n);
void IncDrawVerts();
extern	int			numDrawVerts;
extern	drawVert_t	*drawVerts;

extern	int			numDrawIndexes;
extern	int			drawIndexes[MAX_MAP_DRAW_INDEXES];

void SetDrawSurfaces(int n);
void SetDrawSurfacesBuffer();
extern	int			numDrawSurfaces;
extern	dsurface_t	*drawSurfaces;

extern	int			numFogs;
extern	dfog_t		dfogs[MAX_MAP_FOGS];

void	LoadBSPFile( const char *filename );
void	WriteBSPFile( const char *filename );
void	PrintBSPFileSizes( void );

//===============


typedef struct epair_s {
	struct epair_s	*next;
	char	*key;
	char	*value;
} epair_t;

typedef struct {
	vec3_t		origin;
	struct bspbrush_s	*brushes;
	struct parseMesh_s	*patches;
	int			firstDrawSurf;
	epair_t		*epairs;
} entity_t;

extern	int			num_entities;
extern	entity_t	entities[MAX_MAP_ENTITIES];

void	ParseEntities( void );
void	UnparseEntities( void );

void 	SetKeyValue( entity_t *ent, const char *key, const char *value );
const char 	*ValueForKey( const entity_t *ent, const char *key );
// will return "" if not present

vec_t	FloatForKey( const entity_t *ent, const char *key );
void 	GetVectorForKey( const entity_t *ent, const char *key, vec3_t vec );

epair_t *ParseEpair( void );

void	PrintEntity( const entity_t *ent );

