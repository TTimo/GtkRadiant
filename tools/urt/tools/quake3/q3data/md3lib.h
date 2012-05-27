#include <stdio.h>
#include "../common/cmdlib.h"
#include "mathlib.h"
#include "../common/qfiles.h"

void MD3_Dump( const char *filename );
void MD3_ComputeTagFromTri( md3Tag_t *pTag, const float tri[3][3] );
