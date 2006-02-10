/*
PrtView plugin for GtkRadiant
Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

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

#if !defined(AFX_LOADPORTALFILEDIALOG_H__6BEDE392_1FDC_11D4_BFF7_204C4F4F5020__INCLUDED_)
#define AFX_LOADPORTALFILEDIALOG_H__6BEDE392_1FDC_11D4_BFF7_204C4F4F5020__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LoadPortalFileDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadPortalFileDialog dialog

#ifdef GTK_PLUGIN

int DoLoadPortalFileDialog ();

#else

class CLoadPortalFileDialog : public CDialog
{
// Construction
public:
	CLoadPortalFileDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadPortalFileDialog)
	enum { IDD = IDD_LOAD };
	CButton	m_3d_ctrl;
	CButton	m_2d_ctrl;
	CStatic	m_fn_ctrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadPortalFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadPortalFileDialog)
	virtual qboolean OnInitDialog();
	virtual void OnOK();
	afx_msg void OnLoadOther();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // GTK_PLUGIN

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADPORTALFILEDIALOG_H__6BEDE392_1FDC_11D4_BFF7_204C4F4F5020__INCLUDED_)
