// RenameDlg.h: interface for the CRenameDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENAMEDLG_H__6BF675B2_311E_42EE_A926_C52EA685EA75__INCLUDED_)
#define AFX_RENAMEDLG_H__6BF675B2_311E_42EE_A926_C52EA685EA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

class CRenameDlg
{
public:
	CRenameDlg();
	virtual ~CRenameDlg();

	std::string m_Name;

	int DoModal();
};

#endif // !defined(AFX_RENAMEDLG_H__6BF675B2_311E_42EE_A926_C52EA685EA75__INCLUDED_)
