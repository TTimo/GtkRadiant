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

// DEPair.h: interface for the DEPair class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DEPAIR_H__35B2C521_F0A7_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DEPAIR_H__35B2C521_F0A7_11D4_ACF7_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DEPair
{
public:
DEPair();
virtual ~DEPair();

void Build( const char* pKey, const char* pValue );

Str key;
Str value;
};

//typedef CList<DEPair*, DEPair*> DEPairList;

#endif // !defined(AFX_DEPAIR_H__35B2C521_F0A7_11D4_ACF7_004095A18133__INCLUDED_)
