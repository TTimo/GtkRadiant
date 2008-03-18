// MemFile.h: interface for the CMemFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMFILE_H__FCA4B794_CC06_4F09_8B54_B1BF10291ECA__INCLUDED_)
#define AFX_MEMFILE_H__FCA4B794_CC06_4F09_8B54_B1BF10291ECA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMemFile  
{
public:
	CMemFile();
	CMemFile(char *data, long len);
	virtual ~CMemFile();

	char *data;
	char *ptr;
	char *end;
	
	void Read(void *to,long size);

};

#endif // !defined(AFX_MEMFILE_H__FCA4B794_CC06_4F09_8B54_B1BF10291ECA__INCLUDED_)
