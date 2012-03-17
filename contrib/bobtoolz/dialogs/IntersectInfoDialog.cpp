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

// IntersectInfoDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "../bobtoolz.h"
#include "IntersectInfoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntersectInfoDialog dialog


CIntersectInfoDialog::CIntersectInfoDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CIntersectInfoDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CIntersectInfoDialog)
	//}}AFX_DATA_INIT
}


void CIntersectInfoDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CIntersectInfoDialog)
	DDX_Control( pDX, IDC_PROGRESS1, m_prog1 );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CIntersectInfoDialog, CDialog )
//{{AFX_MSG_MAP(CIntersectInfoDialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntersectInfoDialog message handlers
