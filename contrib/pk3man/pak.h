// Pak.h: interface for the CPak class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PAK_H__CB5AFBB6_E37C_4898_AB04_1ABEA6AC9B45__INCLUDED_)
#define AFX_PAK_H__CB5AFBB6_E37C_4898_AB04_1ABEA6AC9B45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

extern "C"
{
#include "./zlib/zlib.h"
#include "./zlib/unzip.h"
#include "./zlib/zip.h" 
}
#include <list>
#include <string>

typedef struct
{
	// file
	std::string filename;
	std::string pathname;
	
	// external pak
	BOOL frompak;
	std::string pakname;

	// rename
	BOOL rename;
	std::string newname;

	//sizes
	unsigned long uncompressed;
	unsigned long compressed;

} pak_entry;


class CPak  
{
public:
	CPak();
	virtual ~CPak();


	Str m_filename;
	std::list<pak_entry> entries;
	bool m_bModified;

	bool Open(const char *filename);
	void Close();
	bool IsModified();
	bool Save(const char *);
	bool IsEmpty();

	pak_entry FindPE(const char *path);
	bool IsInPak(const char *path);

	bool AddExternalFile(const char *name,const char *path);
	bool AddPakFile(const char *name,const char *pakname,long comp, long uncomp);
	bool Delete(const char *path);
	bool Rename(const char *path,const char *newname);

	bool ExtractTo(const char *path, char *pathto);
	unsigned char *Extract(const char *path, long &size);
	
};

#endif // !defined(AFX_PAK_H__CB5AFBB6_E37C_4898_AB04_1ABEA6AC9B45__INCLUDED_)
