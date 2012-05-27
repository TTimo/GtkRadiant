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

void line_construct_for_vec3(line_t *line, const vec3_t start, const vec3_t end)
{
  VectorMid(start, end, line->origin);
  VectorSubtract(end, line->origin, line->extents);
}

int line_test_plane(const line_t* line, const vec4_t plane)
{
  float fDist;

  // calc distance of origin from plane
  fDist = DotProduct(plane, line->origin) + plane[3];
  
  // accept if origin is less than or equal to this distance
  if (fabs(fDist) < fabs(DotProduct(plane, line->extents))) return 1; // partially inside
  else if (fDist < 0) return 2; // totally inside
  return 0; // totally outside
}
