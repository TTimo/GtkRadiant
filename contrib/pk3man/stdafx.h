// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__214D7491_03C8_444D_AAEB_73A8A21B84CB__INCLUDED_)
#define AFX_STDAFX_H__214D7491_03C8_444D_AAEB_73A8A21B84CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define GTK_ENABLE_BROKEN

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "pk3str.h"
#include "pixmap.h"

#ifdef __linux__

#include <GL/glx.h>

typedef void* HMODULE;
typedef void* LPVOID;
typedef int BOOL;
typedef char* LPCSTR;

#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L


#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define MB_TYPEMASK                 0x0000000FL
#define MB_ICONMASK                 0x000000F0L
#define MB_DEFMASK                  0x00000F00L
#define MB_MODEMASK                 0x00003000L
#define MB_MISCMASK                 0x0000C000L

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

#define WINAPI

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
  unsigned long  Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char  Data4[8];
} GUID;
#endif

#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#endif

typedef struct tagRECT
{
	long    left;
	long    top;
	long    right;
	long    bottom;
} RECT, *PRECT, *LPRECT;

#endif // __linux__

#include "qerplugin.h"
#include "igl.h"
#include "iselectedface.h"
#include "isurfaceplugin.h"
#include "ishaders.h"
#if 0 //mattn
#include "iepairs.h"
#endif

extern _QERFuncTable_1  g_FuncTable;
extern _QERShadersTable g_ShadersTable;
#if 0 //mattn
extern _QEREpairsTable  g_EpairTable;
#endif
#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf

#define CString Str
extern CString g_strBitmapsPath;
extern CString g_strBasePath;
extern CString g_strTempPath;
extern CString g_strAppPath;
extern CString g_strModPath;


//extern int DoMessageBox (const char* lpText, const char* lpCaption, guint32 uType);

#define ID_FILE_OPEN	10000
#define ID_FILE_SAVE	10001
#define ID_FILE_NEW		10002
#define ID_WIZARD		10003
#define ID_VIEW			10004
#define ID_EXTRACT		10005
#define ID_ADD			10006
#define ID_RENAME		10007
#define ID_DELETE		10008
#define ID_SPARE		10009

#ifndef __linux__
#pragma warning(disable : 4786)
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__214D7491_03C8_444D_AAEB_73A8A21B84CB__INCLUDED_)
