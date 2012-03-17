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

#if !defined( AFX_TEXTURERESETDIALOG_H__42D665C1_ED84_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_TEXTURERESETDIALOG_H__42D665C1_ED84_11D4_ACF7_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureResetDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureResetDialog dialog

class CTextureResetDialog : public CDialog
{
// Construction
public:
CTextureResetDialog( CWnd* pParent = NULL );     // standard constructor

// Dialog Data
//{{AFX_DATA(CTextureResetDialog)
enum { IDD = IDD_TEXTURE_RESET_DIALOG };
BOOL m_bAllTextures;
CString m_TextureName;
int m_nRotation;
float m_fScaleHorizontal;
float m_fScaleVertical;
int m_nShiftHorizontal;
int m_nShiftVertical;
BOOL m_bOnlyTexture;
CString m_NewTextureName;
//}}AFX_DATA


// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CTextureResetDialog)
protected:
virtual void DoDataExchange( CDataExchange* pDX );      // DDX/DDV support
//}}AFX_VIRTUAL

// Implementation
protected:

// Generated message map functions
//{{AFX_MSG(CTextureResetDialog)
// NOTE: the ClassWizard will add member functions here
//}}AFX_MSG
DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURERESETDIALOG_H__42D665C1_ED84_11D4_ACF7_004095A18133__INCLUDED_)
