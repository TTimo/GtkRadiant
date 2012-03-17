/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// DPoint.cpp: implementation of the DPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DPoint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DPoint::DPoint(){

}

DPoint::~DPoint(){

}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

bool DPoint::operator ==( vec3_t other ){
	vec3_t test;
	VectorSubtract( other, _pnt, test );
	if ( fabs( VectorLength( test ) ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}
	return TRUE;
}
