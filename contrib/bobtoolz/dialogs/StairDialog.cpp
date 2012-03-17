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

// StairDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "StairDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStairDialog dialog


CStairDialog::CStairDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CStairDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CStairDialog)
	m_nStairHeight = 8;
	m_StairDir = 0;
	m_StairStyle = 0;
	m_riserTexture = _T( "" );
	m_bDetail = TRUE;
	//}}AFX_DATA_INIT
}

void CStairDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CStairDialog)
	DDX_Text( pDX, IDC_EDIT1, m_nStairHeight );
	DDV_MinMaxUInt( pDX, m_nStairHeight, 1, 256 );
	DDX_Radio( pDX, IDC_DIR_N_RADIO, m_StairDir );
	DDX_Radio( pDX, IDC_STYLE_ORIG_RADIO, m_StairStyle );
	DDX_Text( pDX, IDC_RISER_EDIT, m_riserTexture );
	DDV_MaxChars( pDX, m_riserTexture, 256 );
	DDX_Check( pDX, IDC_DETAIL_CHK, m_bDetail );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CStairDialog, CDialog )
//{{AFX_MSG_MAP(CStairDialog)
ON_BN_CLICKED( IDC_STYLE_BOB_RADIO, OnStyleBobClicked )
ON_BN_CLICKED( IDC_STYLE_ORIG_RADIO, OnStyleOrigClicked )
ON_BN_CLICKED( IDC_STYLE_CORNER_RADIO, OnStyleCornerClicked )
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStairDialog message handlers

void CStairDialog::OnStyleBobClicked(){
	EnableDetail( TRUE );
}

void CStairDialog::OnStyleOrigClicked(){
	EnableDetail( FALSE );
}

void CStairDialog::EnableDetail( BOOL bEnable ){
	CWnd* dtlChk = GetDlgItem( IDC_DETAIL_CHK );
	if ( dtlChk ) {
		dtlChk->EnableWindow( bEnable );
	}
}


BOOL CStairDialog::OnInitDialog(){
	CDialog::OnInitDialog();

	EnableDetail( m_StairStyle == 1 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStairDialog::OnStyleCornerClicked(){
	EnableDetail( FALSE );
}
