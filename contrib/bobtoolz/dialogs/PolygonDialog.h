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

#if !defined( AFX_POLYGONDIALOG_H__EF7FE400_628A_11D1_B66D_004095A18133__INCLUDED_ )
#define AFX_POLYGONDIALOG_H__EF7FE400_628A_11D1_B66D_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PolygonDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPolygonDialog dialog

class CPolygonDialog : public CDialog
{
// Construction
public:
BOOL GetChkBool( int nID );
void EnableBorderEdit( BOOL bEnable );
void EnableBordered( BOOL bEnable );
CPolygonDialog( CWnd* pParent = NULL );     // standard constructor

// Dialog Data
//{{AFX_DATA(CPolygonDialog)
enum { IDD = IDD_POLYGON_DIALOG };
UINT m_nSideCount;
BOOL m_bInverse;
BOOL m_bBorder;
UINT m_nBorderSize;
BOOL m_bAlignTop;
//}}AFX_DATA


// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CPolygonDialog)
protected:
virtual void DoDataExchange( CDataExchange* pDX );      // DDX/DDV support
//}}AFX_VIRTUAL

// Implementation
protected:

// Generated message map functions
//{{AFX_MSG(CPolygonDialog)
virtual BOOL OnInitDialog();
afx_msg void OnBorderChkClicked();
afx_msg void OnInverseChkClickrd();
//}}AFX_MSG
DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POLYGONDIALOG_H__EF7FE400_628A_11D1_B66D_004095A18133__INCLUDED_)
