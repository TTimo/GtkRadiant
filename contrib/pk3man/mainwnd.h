// MainWnd.h: interface for the CMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINWND_H__E2FFC5D8_4D6E_4752_8976_811551035B7A__INCLUDED_)
#define AFX_MAINWND_H__E2FFC5D8_4D6E_4752_8976_811551035B7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "tree.h"
#include "pak.h"


//void HandleCommand(GtkWidget *widget, gpointer data);


class CMainWnd
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	GtkWidget* m_pMainWnd;
	GtkWidget *m_pStatusLabel[3];
	GtkWidget *m_ScrolledWin;
	GtkWidget *m_Toolbar;

	GtkWidget* m_mnuDrop;

	CTree m_Tree;
	CPak pak;

	std::list<std::string> texlist;
	std::list<CPak*> paklist;
	std::list<std::string> exclusions;
	std::list<std::string> temp_files;

	BOOL Create(GtkWidget *parent);
	BOOL CreateToolbar(GtkWidget *window,GtkWidget *vbox);
	BOOL CreateStatusBar(GtkWidget *window,GtkWidget *vbox);
	BOOL CreateTreeView(GtkWidget *window,GtkWidget *vbox);

	void GetPaths();
	void UpdateTree();
	void UpdateStatus();
	void UpdateToolBar();

	void HandleDrop();

	// command handlers
	bool OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnWizard();
	void OnView();
	void OnExtract();
	void OnAdd();
	void OnRename();
	void OnDelete();
	bool OnClose();
	void OnDestroy();

	bool ProcessFile(const char *name,const char *suffix,const char *folder);
	bool ProcessFile(const char *name);

	std::string GetLine(FILE *fp);
	void SkipLines(FILE *fp, int n);
	void AddToTexList(const char *buf);
	bool IsInTexList(const char *tex);
	void ParseShader(const char*sname, const char *tex);
	std::string TrimString(const char *str);
	void ParseModel(const char *pathname);

	void InitPakList();
	void InitPaksInDir(const char *folder);
	void FreePakList();
	bool FindInAPk3(const char *name);

	void LoadExclusions();
	bool IsExcluded(const char *name);
	void DeleteTempFiles();
};

#endif // !defined(AFX_MAINWND_H__E2FFC5D8_4D6E_4752_8976_811551035B7A__INCLUDED_)
