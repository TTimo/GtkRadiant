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
#define LEAKFILE_C



/* dependencies */
#include "q3map2.h"



/*
==============================================================================

LEAK FILE GENERATION

Save out name.line for qe3 to read
==============================================================================
*/


/*
=============
LeakFile

Finds the shortest possible chain of portals
that leads from the outside leaf to a specifically
occupied leaf

TTimo: builds a polyline xml node
=============
*/
xmlNodePtr LeakFile (tree_t *tree)
{
	vec3_t	mid;
	FILE	*linefile;
	char	filename[1024];
	node_t	*node;
	int		count;
	xmlNodePtr xml_node, point;

	if (!tree->outside_node.occupied)
		return NULL;

	Sys_FPrintf (SYS_VRB,"--- LeakFile ---\n");

	//
	// write the points to the file
	//
	sprintf (filename, "%s.lin", source);
	linefile = fopen (filename, "w");
	if (!linefile)
		Error ("Couldn't open %s\n", filename);

  xml_node = xmlNewNode (NULL, "polyline");

	count = 0;
	node = &tree->outside_node;
	while (node->occupied > 1)
	{
		int			next;
		portal_t	*p, *nextportal;
		node_t		*nextnode;
		int			s;

		// find the best portal exit
		next = node->occupied;
		for (p=node->portals ; p ; p = p->next[!s])
		{
			s = (p->nodes[0] == node);
			if (p->nodes[s]->occupied
				&& p->nodes[s]->occupied < next)
			{
				nextportal = p;
				nextnode = p->nodes[s];
				next = nextnode->occupied;
			}
		}
		node = nextnode;
		WindingCenter (nextportal->winding, mid);
		fprintf (linefile, "%f %f %f\n", mid[0], mid[1], mid[2]);
		point = xml_NodeForVec(mid);
		xmlAddChild(xml_node, point);
		count++;
	}
	// add the occupant center
	GetVectorForKey (node->occupant, "origin", mid);

	fprintf (linefile, "%f %f %f\n", mid[0], mid[1], mid[2]);
	point = xml_NodeForVec(mid);
	xmlAddChild(xml_node, point);
	Sys_FPrintf( SYS_VRB, "%9d point linefile\n", count+1);

	fclose (linefile);
	
	return xml_node;
}

