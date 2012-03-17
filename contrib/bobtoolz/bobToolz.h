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

// plugin.h : main header file for the PLUGIN DLL
//

#if !defined( AFX_PLUGIN_H__3BA55F6A_1D27_11D3_BC7B_F7EFD9765E37__INCLUDED_ )
#define AFX_PLUGIN_H__3BA55F6A_1D27_11D3_BC7B_F7EFD9765E37__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'StdAfx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPluginApp
// See plugin.cpp for the implementation of this class
//

class CPluginApp : public CWinApp
{
public:
CPluginApp();

// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CPluginApp)
//}}AFX_VIRTUAL

//{{AFX_MSG(CPluginApp)
// NOTE - the ClassWizard will add and remove member functions here.
//    DO NOT EDIT what you see in these blocks of generated code !
//}}AFX_MSG
DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGIN_H__3BA55F6A_1D27_11D3_BC7B_F7EFD9765E37__INCLUDED_)
