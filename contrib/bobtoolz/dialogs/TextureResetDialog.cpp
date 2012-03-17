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

// TextureResetDialog.cpp : implementation file
//

#include "../StdAfx.h"
#include "../bobtoolz.h"
#include "TextureResetDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureResetDialog dialog


CTextureResetDialog::CTextureResetDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CTextureResetDialog::IDD, pParent ){
	//{{AFX_DATA_INIT(CTextureResetDialog)
	m_bAllTextures = FALSE;
	m_TextureName = _T( "" );
	m_nRotation = 0;
	m_fScaleHorizontal = 0.5f;
	m_fScaleVertical = 0.5f;
	m_nShiftHorizontal = 0;
	m_nShiftVertical = 0;
	m_bOnlyTexture = FALSE;
	m_NewTextureName = _T( "" );
	//}}AFX_DATA_INIT
}


void CTextureResetDialog::DoDataExchange( CDataExchange* pDX ){
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CTextureResetDialog)
	DDX_Check( pDX, IDC_ALLTEXTURES_CHECK, m_bAllTextures );
	DDX_Text( pDX, IDC_RESET_TEXTURE_EDIT, m_TextureName );
	DDV_MaxChars( pDX, m_TextureName, 256 );
	DDX_Text( pDX, IDC_ROTATION_EDIT, m_nRotation );
	DDV_MinMaxInt( pDX, m_nRotation, 0, 360 );
	DDX_Text( pDX, IDC_SCL_HOR_EDIT, m_fScaleHorizontal );
	DDX_Text( pDX, IDC_SCL_VERT_EDIT, m_fScaleVertical );
	DDX_Text( pDX, IDC_SHFT_HOR_EDIT, m_nShiftHorizontal );
	DDX_Text( pDX, IDC_SHFT_VER_EDIT, m_nShiftVertical );
	DDX_Check( pDX, IDC_ONLYTEXTURE_CHECK, m_bOnlyTexture );
	DDX_Text( pDX, IDC_RESET_NEW_TEXTURE_EDIT, m_NewTextureName );
	DDV_MaxChars( pDX, m_NewTextureName, 256 );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CTextureResetDialog, CDialog )
//{{AFX_MSG_MAP(CTextureResetDialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureResetDialog message handlers
