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

// IntersectDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "IntersectDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntersectDialog dialog


CIntersectDialog::CIntersectDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CIntersectDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CIntersectDialog)
	m_nBrushOptions = 1;
	m_bUseDetail = FALSE;
	m_bDuplicateOnly = FALSE;
	//}}AFX_DATA_INIT
}


void CIntersectDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CIntersectDialog)
	DDX_Radio( pDX, IDC_WHOLEMAP_RADIO, m_nBrushOptions );
	DDX_Check( pDX, IDC_DETAIL_INCLUDE_CHECK, m_bUseDetail );
	DDX_Check( pDX, IDC_DUPLICATEONLY_CHECK, m_bDuplicateOnly );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CIntersectDialog, CDialog )
//{{AFX_MSG_MAP(CIntersectDialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntersectDialog message handlers
