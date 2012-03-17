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

// AutoCaulkDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "../bobtoolz.h"
#include "AutoCaulkDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoCaulkDialog dialog


CAutoCaulkDialog::CAutoCaulkDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CAutoCaulkDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CAutoCaulkDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAutoCaulkDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CAutoCaulkDialog)
	DDX_Control( pDX, IDC_PROGRESS2, m_prog2 );
	DDX_Control( pDX, IDC_PROGRESS1, m_prog1 );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CAutoCaulkDialog, CDialog )
//{{AFX_MSG_MAP(CAutoCaulkDialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoCaulkDialog message handlers
