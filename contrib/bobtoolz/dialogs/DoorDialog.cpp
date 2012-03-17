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

// DoorDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "DoorDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDoorDialog dialog


CDoorDialog::CDoorDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CDoorDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CDoorDialog)
	m_fbTextureName = _T( "" );
	m_bSclMainHor = TRUE;
	m_bSclMainVert = TRUE;
	m_bSclTrimHor = TRUE;
	m_bSclTrimVert = FALSE;
	m_trimTextureName = _T( "" );
	m_trimTexSetBox = _T( "" );
	m_mainTexSetBox = _T( "" );
	m_doorDirection = -1;
	//}}AFX_DATA_INIT
}


void CDoorDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CDoorDialog)
	DDX_Text( pDX, IDC_FBTEXTURE_EDIT, m_fbTextureName );
	DDX_Check( pDX, IDC_TEXSCALE1_CHECK, m_bSclMainHor );
	DDX_Check( pDX, IDC_TEXSCALE2_CHECK, m_bSclMainVert );
	DDX_Check( pDX, IDC_TEXSCALE3_CHECK, m_bSclTrimHor );
	DDX_Check( pDX, IDC_TEXSCALE4_CHECK, m_bSclTrimVert );
	DDX_Text( pDX, IDC_TRIMTEXTURE_EDIT, m_trimTextureName );
	DDX_CBString( pDX, IDC_TRIMTEX_COMBO, m_trimTexSetBox );
	DDX_CBString( pDX, IDC_MAINTEX_COMBO, m_mainTexSetBox );
	DDX_Radio( pDX, IDC_DIR_NS_RADIO, m_doorDirection );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CDoorDialog, CDialog )
//{{AFX_MSG_MAP(CDoorDialog)
ON_BN_CLICKED( IDC_SET_MAINTEX_BTN, OnSetMaintexBtn )
ON_BN_CLICKED( IDC_SET_TRIMTEX_BTN, OnSetTrimtexBtn )
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDoorDialog message handlers

void CDoorDialog::OnSetMaintexBtn(){
	UpdateData( TRUE );
	m_fbTextureName = m_mainTexSetBox;
	UpdateData( FALSE );
}

void CDoorDialog::OnSetTrimtexBtn(){
	UpdateData( TRUE );
	m_trimTextureName = m_trimTexSetBox;
	UpdateData( FALSE );
}
