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

#if !defined( AFX_STAIRDIALOG_H__942FFF20_5F9E_11D1_B66D_004095A18133__INCLUDED_ )
#define AFX_STAIRDIALOG_H__942FFF20_5F9E_11D1_B66D_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StairDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStairDialog dialog

class CStairDialog : public CDialog
{
// Construction
public:
CStairDialog( CWnd* pParent = NULL );     // standard constructor

// Dialog Data
//{{AFX_DATA(CStairDialog)
enum { IDD = IDD_STAIR_DIALOG };
UINT m_nStairHeight;
int m_StairDir;
int m_StairStyle;
CString m_riserTexture;
BOOL m_bDetail;
//}}AFX_DATA


// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CStairDialog)
protected:
virtual void DoDataExchange( CDataExchange* pDX );      // DDX/DDV support
//}}AFX_VIRTUAL

// Implementation
protected:

// Generated message map functions
//{{AFX_MSG(CStairDialog)
afx_msg void OnStyleBobClicked();
afx_msg void OnStyleOrigClicked();
virtual BOOL OnInitDialog();
afx_msg void OnStyleCornerClicked();
//}}AFX_MSG
DECLARE_MESSAGE_MAP()
private:
void EnableDetail( BOOL bEnable );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAIRDIALOG_H__942FFF20_5F9E_11D1_B66D_004095A18133__INCLUDED_)
