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

// PathPlotterDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "../bobtoolz.h"
#include "PathPlotterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathPlotterDialog dialog


CPathPlotterDialog::CPathPlotterDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CPathPlotterDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CPathPlotterDialog)
	m_fGravity = -800.0f;
	m_fMultiplier = 3.0f;
	m_bNoUpdate = FALSE;
	m_nPoints = 25;
	m_bShowExtra = FALSE;
	//}}AFX_DATA_INIT
}


void CPathPlotterDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CPathPlotterDialog)
	DDX_Text( pDX, IDC_GRAVITY_EDIT, m_fGravity );
	DDV_MinMaxFloat( pDX, m_fGravity, -10000.f, -1.f );
	DDX_Text( pDX, IDC_MULTIPLIER_EDIT, m_fMultiplier );
	DDV_MinMaxFloat( pDX, m_fMultiplier, 1.f, 10.f );
	DDX_Check( pDX, IDC_NOUPDATE_CHECK, m_bNoUpdate );
	DDX_Text( pDX, IDC_POINTCOUNT_EDIT, m_nPoints );
	DDV_MinMaxInt( pDX, m_nPoints, 1, 1000 );
	DDX_Check( pDX, IDC_SHOWEXTRA_CHECK, m_bShowExtra );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CPathPlotterDialog, CDialog )
//{{AFX_MSG_MAP(CPathPlotterDialog)
ON_BN_CLICKED( IDYES, OnYes )
ON_BN_CLICKED( IDNO, OnNo )
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathPlotterDialog message handlers

void CPathPlotterDialog::OnYes(){
	if ( UpdateData() ) {
		EndModalLoop( IDYES );
	}
}

void CPathPlotterDialog::OnNo(){
	EndModalLoop( IDNO );
}
