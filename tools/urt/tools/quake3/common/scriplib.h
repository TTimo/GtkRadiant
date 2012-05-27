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

// scriplib.h

#ifndef __CMDLIB__
#include "../common/cmdlib.h"
#endif
#ifndef __MATHLIB__
#include "mathlib.h"
#endif

#define	MAXTOKEN	1024

extern	char	token[MAXTOKEN];
extern	char	*scriptbuffer,*script_p,*scriptend_p;
extern	int		grabbed;
extern	int		scriptline;
extern	qboolean	endofscript;


void LoadScriptFile (const char *filename, int index);
void ParseFromMemory (char *buffer, int size);

qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

void MatchToken( char *match );

void Parse1DMatrix (int x, vec_t *m);
void Parse2DMatrix (int y, int x, vec_t *m);
void Parse3DMatrix (int z, int y, int x, vec_t *m);

void Write1DMatrix (FILE *f, int x, vec_t *m);
void Write2DMatrix (FILE *f, int y, int x, vec_t *m);
void Write3DMatrix (FILE *f, int z, int y, int x, vec_t *m);
