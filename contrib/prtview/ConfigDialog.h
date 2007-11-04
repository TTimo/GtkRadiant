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

#if !defined(AFX_CONFIGDIALOG_H__E484E672_2088_11D4_BFFA_204C4F4F5020__INCLUDED_)
#define AFX_CONFIGDIALOG_H__E484E672_2088_11D4_BFFA_204C4F4F5020__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ConfigDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigDialog dialog

#ifdef GTK_PLUGIN

void DoConfigDialog ();

#else

class CConfigDialog : public CDialog
{
// Construction
public:
	CConfigDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigDialog)
	enum { IDD = IDD_CONFIG };
	CButton	m_clip_ctrl;
	CStatic	m_cubic_ctrl;
	CScrollBar	m_scroll_cubic_ctrl;
	CButton	m_line_ctrl;
	CScrollBar	m_scroll_3d_trans_ctrl;
	CStatic	m_3d_trans_ctrl;
	CButton	m_poly_ctrl;
	CButton	m_fog_ctrl;
	CComboBox	m_z_ctrl;
	CScrollBar	m_scroll_3d_width_ctrl;
	CButton	m_aa_3d_ctrl;
	CStatic	m_3d_width_ctrl;
	CButton	m_aa_2d_ctrl;
	CScrollBar	m_scroll_2d_width_ctrl;
	CStatic	m_2d_width_ctrl;
	CButton	m_3d_ctrl;
	CButton	m_2d_ctrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void Set2DText();
	void Set3DText();
	void Set3DTransText();
	void SetClipText();

	// Generated message map functions
	//{{AFX_MSG(CConfigDialog)
	virtual qboolean OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAntiAlias2d();
	afx_msg void OnConfig2d();
	afx_msg void OnConfig3d();
	afx_msg void OnColor2d();
	afx_msg void OnAntiAlias3d();
	afx_msg void OnColor3d();
	afx_msg void OnColorFog();
	afx_msg void OnFog();
	afx_msg void OnSelchangeZbuffer();
	afx_msg void OnPoly();
	afx_msg void OnLines();
	afx_msg void OnClip();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // GTK_PLUGIN

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDIALOG_H__E484E672_2088_11D4_BFFA_204C4F4F5020__INCLUDED_)
