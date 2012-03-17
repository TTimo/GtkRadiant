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

#if !defined( AFX_INTERSECTDIALOG_H__03507C01_D3B3_11D4_AE97_004095A18133__INCLUDED_ )
#define AFX_INTERSECTDIALOG_H__03507C01_D3B3_11D4_AE97_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntersectDialog.h : header file
//

#define BRUSH_OPT_WHOLE_MAP 0
#define BRUSH_OPT_SELECTED  1

/////////////////////////////////////////////////////////////////////////////
// CIntersectDialog dialog

class CIntersectDialog : public CDialog
{
// Construction
public:
CIntersectDialog( CWnd* pParent = NULL );     // standard constructor

// Dialog Data
//{{AFX_DATA(CIntersectDialog)
enum { IDD = IDD_INTERSECT_DIALOG };
int m_nBrushOptions;
BOOL m_bUseDetail;
BOOL m_bDuplicateOnly;
//}}AFX_DATA


// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CIntersectDialog)
protected:
virtual void DoDataExchange( CDataExchange* pDX );      // DDX/DDV support
//}}AFX_VIRTUAL

// Implementation
protected:

// Generated message map functions
//{{AFX_MSG(CIntersectDialog)
// NOTE: the ClassWizard will add member functions here
//}}AFX_MSG
DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERSECTDIALOG_H__03507C01_D3B3_11D4_AE97_004095A18133__INCLUDED_)
