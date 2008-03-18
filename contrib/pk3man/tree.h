// Tree.h: interface for the CTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREE_H__C0DDC824_48DF_4A80_B393_83FF736FCB98__INCLUDED_)
#define AFX_TREE_H__C0DDC824_48DF_4A80_B393_83FF736FCB98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include <string>

void cb_select_child (GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree);

class CTree
{
public:
	CTree();
	virtual ~CTree();

	GtkWidget *m_pMainWnd;
	GtkWidget *m_Tree;
	GtkWidget *m_Root;

	void Init(GtkWidget *win);
	void AddPath(const char *path);
	void AddFile(const char *file,GtkWidget *node=NULL);
	GtkWidget *AddFolder(const char *folder, GtkWidget *node=NULL);
	void RemovePath(const char*path);

	void SelectChild(GtkWidget *child);
	std::string GetSelected();
	std::string GetFullPath(GtkWidget *item);

	void Clear();

};

#endif // !defined(AFX_TREE_H__C0DDC824_48DF_4A80_B393_83FF736FCB98__INCLUDED_)
