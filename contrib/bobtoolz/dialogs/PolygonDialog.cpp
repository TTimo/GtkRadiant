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

// PolygonDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "PolygonDialog.h"
#include "../shapes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPolygonDialog dialog


CPolygonDialog::CPolygonDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CPolygonDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CPolygonDialog)
	m_nSideCount = 3;
	m_bInverse = FALSE;
	m_bBorder = FALSE;
	m_nBorderSize = 8;
	m_bAlignTop = FALSE;
	//}}AFX_DATA_INIT
}

void CPolygonDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CPolygonDialog)
	DDX_Text( pDX, IDC_EDIT1, m_nSideCount );
	DDV_MinMaxUInt( pDX, m_nSideCount, 3, MAX_POLYGON_FACES );
	DDX_Check( pDX, IDC_INVERSE_CHK, m_bInverse );
	DDX_Check( pDX, IDC_BORDER_CHK, m_bBorder );
	DDX_Text( pDX, IDC_BORDER_EDIT, m_nBorderSize );
	DDV_MinMaxUInt( pDX, m_nBorderSize, 1, 1024 );
	DDX_Check( pDX, IDC_ALIGN_CHK, m_bAlignTop );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CPolygonDialog, CDialog )
//{{AFX_MSG_MAP(CPolygonDialog)
ON_BN_CLICKED( IDC_BORDER_CHK, OnBorderChkClicked )
ON_BN_CLICKED( IDC_INVERSE_CHK, OnInverseChkClickrd )
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPolygonDialog message handlers

BOOL CPolygonDialog::OnInitDialog(){
	CDialog::OnInitDialog();

	EnableBordered( !GetChkBool( IDC_INVERSE_CHK ) );
	EnableBorderEdit( !GetChkBool( IDC_INVERSE_CHK ) && GetChkBool( IDC_BORDER_CHK ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPolygonDialog::EnableBordered( BOOL bEnable ){
	CWnd* dtlChk = GetDlgItem( IDC_BORDER_CHK );
	if ( dtlChk ) {
		dtlChk->EnableWindow( bEnable );
	}
}

void CPolygonDialog::EnableBorderEdit( BOOL bEnable ){
	CWnd* dtlChk = GetDlgItem( IDC_BORDER_EDIT );
	if ( dtlChk ) {
		dtlChk->EnableWindow( bEnable );
	}
}

void CPolygonDialog::OnBorderChkClicked(){
	EnableBorderEdit( !GetChkBool( IDC_INVERSE_CHK ) && GetChkBool( IDC_BORDER_CHK ) );
}

void CPolygonDialog::OnInverseChkClickrd(){
	EnableBordered( !GetChkBool( IDC_INVERSE_CHK ) );
	EnableBorderEdit( !GetChkBool( IDC_INVERSE_CHK ) && GetChkBool( IDC_BORDER_CHK ) );
}

BOOL CPolygonDialog::GetChkBool( int nID ){
	CButton* btn = (CButton*)GetDlgItem( nID );
	if ( btn ) {
		return btn->GetCheck();
	}
	return FALSE;
}
