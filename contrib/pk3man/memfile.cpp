// MemFile.cpp: implementation of the CMemFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "memfile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemFile::CMemFile()
{
	data=NULL;
	ptr=NULL;
	end=NULL;
}

CMemFile::CMemFile(char *block,long len)
{
	data=block;
	ptr=block;
	end=block+len;
}


CMemFile::~CMemFile()
{

}

void CMemFile::Read(void *to,long size)
{
	memcpy(to,ptr,size);
	ptr+=size;
}
