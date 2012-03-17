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

// BrushCheckDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "../bobtoolz.h"
#include "BrushCheckDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrushCheckDialog dialog


CBrushCheckDialog::CBrushCheckDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CBrushCheckDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CBrushCheckDialog)
	//}}AFX_DATA_INIT
}


void CBrushCheckDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CBrushCheckDialog)
	DDX_Control( pDX, IDC_PROGRESS1, m_prog1 );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CBrushCheckDialog, CDialog )
//{{AFX_MSG_MAP(CBrushCheckDialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushCheckDialog message handlers
