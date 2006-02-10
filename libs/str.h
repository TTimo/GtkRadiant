/*
Copyright (c) 2001, Loki software, inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Loki software nor the names of its contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef __STR__
#define __STR__

//
// class Str
// loose replacement for CString from MFC
//

#include <string.h>
#include <ctype.h>

#include <stdio.h>
#include <stdarg.h>

#include <cstdio>

#ifdef WIN32
#define strcasecmp strcmpi
#if _MSC_VER < 1400 
#define vsnprintf std::vsnprintf
#endif
#else
#include <cstddef>
#endif

// NOTE TTimo __StrDup was initially implemented in pakstuff.cpp
//   causing a bunch of issues for broader targets that use Str.h (such as plugins and modules)
//   Q_StrDup should be used now, using a #define __StrDup for easy transition

#define __StrDup Q_StrDup

inline char* Q_StrDup(const char* pStr)
{ 
  if (pStr == 0)
    pStr = "";

  return strcpy(new char[strlen(pStr)+1], pStr); 
}

#if defined (__linux__) || defined (__APPLE__)
#define strcmpi strcasecmp
#define stricmp strcasecmp
#define strnicmp strncasecmp

inline char* strlwr(char* string)
{
  char *cp;
  for (cp = string; *cp; ++cp)
  {
    if ('A' <= *cp && *cp <= 'Z')
      *cp += 'a' - 'A';
  }

  return string;
}

inline char* strupr(char* string)
{
  char *cp;
  for (cp = string; *cp; ++cp)
  {
    if ('a' <= *cp && *cp <= 'z')
      *cp += 'A' - 'a';
  }

  return string;
}
#endif

static char *g_pStrWork = 0;

class Str
{
protected:
  bool m_bIgnoreCase;
  char *m_pStr;

public:
  Str()
  {
    m_bIgnoreCase = true;
    m_pStr = new char[1];
    m_pStr[0] = '\0';
  }

  Str(char *p)
  {
    m_bIgnoreCase = true;
    m_pStr = __StrDup(p);
  }

  Str(const char *p)
  {
    m_bIgnoreCase = true;
    m_pStr = __StrDup(p);
  }

  Str(const unsigned char *p)
  {
    m_bIgnoreCase = true;
    m_pStr = __StrDup(reinterpret_cast<const char *>(p));
  }

  Str(const char c)
  {
    m_bIgnoreCase = true;
    m_pStr = new char[2];
    m_pStr[0] = c;
    m_pStr[1] = '\0';    
  }

  const char* GetBuffer() const
  {
    return m_pStr;
  }

  char* GetBuffer()
  {
    return m_pStr;
  }

  Str(const Str &s)
  {
    m_bIgnoreCase = true;
    m_pStr = __StrDup(s.GetBuffer());
  }

  void Deallocate()
  {
    delete []m_pStr;
    m_pStr = 0;
  }

  void Allocate(std::size_t n)
  {
    Deallocate();
    m_pStr = new char[n];
  }

  void MakeEmpty()
  {
    Deallocate();
    m_pStr = __StrDup("");
  }

  ~Str()
  {
    Deallocate();
    // NOTE TTimo: someone explain this g_pStrWork to me?
    if (g_pStrWork)
      delete []g_pStrWork;
    g_pStrWork = 0;
  }

  void MakeLower()
  {
    if (m_pStr)
    {
      strlwr(m_pStr);
    }
  }

  void MakeUpper()
  {
    if (m_pStr)
    {
      strupr(m_pStr);
    }
  }

  void TrimRight()
    {
      char* lpsz = m_pStr;
      char* lpszLast = 0;
      while (*lpsz != '\0')
      {
	if (isspace(*lpsz))
	{
	  if (lpszLast == 0)
	    lpszLast = lpsz;
	}
	else
	  lpszLast = 0;
	lpsz++;
      }
 
      if (lpszLast != 0)
      {
	// truncate at trailing space start
	*lpszLast = '\0';
      }
    }

  void TrimLeft()
    {
      // find first non-space character
      char* lpsz = m_pStr;
      while (isspace(*lpsz))
	lpsz++;
 
      // fix up data and length
      std::size_t nDataLength = GetLength() - (lpsz - m_pStr);
      memmove(m_pStr, lpsz, (nDataLength+1));
    }

  char* Find(const char *p)
  {
    return strstr(m_pStr, p);
  }

  // search starting at a given offset
  char* Find(const char *p, std::size_t offset)
  {
    return strstr(m_pStr+offset, p);
  }

  char* Find(const char ch)
  {
    return strchr (m_pStr, ch);
  }

  char* ReverseFind(const char ch)
  {
    return strrchr(m_pStr, ch);
  }

  int Compare (const char* str) const
  {
    return strcmp (m_pStr, str);
  }
  
  int CompareNoCase (const char* str) const
  {
    return strcasecmp (m_pStr, str);
  }

  std::size_t GetLength()
  {
    return (m_pStr) ? strlen(m_pStr) : 0;
  }

  const char* Left(std::size_t n)
  {
    delete []g_pStrWork;
    if (n > 0)
    {
      g_pStrWork = new char[n+1];
      strncpy(g_pStrWork, m_pStr, n);
      g_pStrWork[n] = '\0';
    }
    else
    {
      g_pStrWork = "";
      g_pStrWork = new char[1];
      g_pStrWork[0] = '\0';
    }
    return g_pStrWork;
  }

  const char* Right(std::size_t n)
  {
    delete []g_pStrWork;
    if (n > 0)
    {
      g_pStrWork = new char[n+1];
      std::size_t nStart = GetLength() - n;
      strncpy(g_pStrWork, &m_pStr[nStart], n);
      g_pStrWork[n] = '\0';
    }
    else
    {
      g_pStrWork = new char[1];
      g_pStrWork[0] = '\0';
    }
    return g_pStrWork;
  }

  const char* Mid(std::size_t nFirst) const
  {
    return Mid(nFirst, strlen (m_pStr) - nFirst);
  }

  const char* Mid(std::size_t first, std::size_t n) const
  {
    delete []g_pStrWork;
    if (n > 0)
    {
      g_pStrWork = new char[n+1];
      strncpy(g_pStrWork, m_pStr+first, n);
      g_pStrWork[n] = '\0';
    }
    else
    {
      g_pStrWork = "";
      g_pStrWork = new char[1];
      g_pStrWork[0] = '\0';
    }
    return g_pStrWork;
  }

#if 0 // defined(__G_LIB_H__)
  void Format(const char* fmt, ...)
  {
    va_list args;
    char *buffer;
  
    va_start (args, fmt);
    buffer = g_strdup_vprintf (fmt, args);
    va_end (args);

    delete[] m_pStr;
    m_pStr = __StrDup(buffer);
    g_free (buffer);
  }
#else
  void Format(const char* fmt, ...)
  {
    char buffer[1024];

    {
      va_list args;
      va_start (args, fmt);
      vsnprintf(buffer, 1023, fmt, args);
      va_end (args);
    }

    delete[] m_pStr;
    m_pStr = __StrDup(buffer);
  }
#endif

  void SetAt(std::size_t n, char ch)
  {
    if (n < GetLength())
      m_pStr[n] = ch;
  }

	// NOTE: unlike CString, this looses the pointer
  void ReleaseBuffer(std::size_t n)
  {
    char* tmp = m_pStr;
    tmp[n] = '\0';
    m_pStr = __StrDup(tmp);
    delete []tmp;
  }
  void ReleaseBuffer()
  {
    ReleaseBuffer(GetLength());
  }

  char* GetBufferSetLength(std::size_t n)
  {
    char *p = new char[n+1];
    strncpy (p, m_pStr, n);
		p[n] = '\0';
    delete []m_pStr;
    m_pStr = p;
    return m_pStr;
  }

  //  char& operator *() { return *m_pStr; }
  //  char& operator *() const { return *const_cast<Str*>(this)->m_pStr; }
  operator void*() { return m_pStr; }
  operator char*() { return m_pStr; }
  operator const char*() const{ return reinterpret_cast<const char*>(m_pStr); }
  operator unsigned char*() { return reinterpret_cast<unsigned char*>(m_pStr); }
  operator const unsigned char*() const { return reinterpret_cast<const unsigned char*>(m_pStr); }
  Str& operator =(const Str& rhs)
  {
    if (&rhs != this)
    {
      delete[] m_pStr;
      m_pStr = __StrDup(rhs.m_pStr);
    }
    return *this;
  }
  
  Str& operator =(const char* pStr)
  {
    if (m_pStr != pStr)
    {
      delete[] m_pStr;
      m_pStr = __StrDup(pStr);
    }
    return *this;
  }

  Str& operator +=(const char ch)
  {
    std::size_t len = GetLength();
    char *p = new char[len + 1 + 1];

    if (m_pStr)
    {
      strcpy(p, m_pStr);
      delete[] m_pStr;
    }

    m_pStr = p;
    m_pStr[len] = ch;
    m_pStr[len+1] = '\0';

    return *this;
  }

  Str& operator +=(const char *pStr)
  {
    if (pStr)
    {
      if (m_pStr)
      {
        char *p = new char[strlen(m_pStr) + strlen(pStr) + 1];
        strcpy(p, m_pStr);
        strcat(p, pStr);
        delete[] m_pStr;
        m_pStr = p;
      }
      else
      {
        m_pStr = __StrDup(pStr);
      }
    }
    return *this;
  }
  

  bool operator ==(const Str& rhs) const { return (m_bIgnoreCase) ? stricmp(m_pStr, rhs.m_pStr) == 0 : strcmp(m_pStr, rhs.m_pStr) == 0; }
  bool operator ==(char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) == 0 : strcmp(m_pStr, pStr) == 0; }
  bool operator ==(const char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) == 0 : strcmp(m_pStr, pStr) == 0; }
  bool operator !=(Str& rhs) const { return (m_bIgnoreCase) ? stricmp(m_pStr, rhs.m_pStr) != 0 : strcmp(m_pStr, rhs.m_pStr) != 0; }
  bool operator !=(char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) != 0 : strcmp(m_pStr, pStr) != 0; }
  bool operator !=(const char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) != 0 : strcmp(m_pStr, pStr) != 0; }
  bool operator <(const Str& rhs) const { return (m_bIgnoreCase) ? stricmp(m_pStr, rhs.m_pStr) < 0 : strcmp(m_pStr, rhs.m_pStr) < 0; }
  bool operator <(char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) < 0 : strcmp(m_pStr, pStr) < 0; }
  bool operator <(const char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) < 0 : strcmp(m_pStr, pStr) < 0; }
  bool operator >(const Str& rhs) const { return (m_bIgnoreCase) ? stricmp(m_pStr, rhs.m_pStr) > 0 : strcmp(m_pStr, rhs.m_pStr) > 0; }
  bool operator >(char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) > 0 : strcmp(m_pStr, pStr) > 0; }
  bool operator >(const char* pStr) const { return (m_bIgnoreCase) ? stricmp(m_pStr, pStr) > 0 : strcmp(m_pStr, pStr) > 0; }
  char& operator [](std::size_t nIndex) { return m_pStr[nIndex]; }
  const char& operator [](std::size_t nIndex) const { return m_pStr[nIndex]; }
  const char GetAt (std::size_t nIndex) { return m_pStr[nIndex]; }
};


template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const Str& str)
{
  return ostream << str.GetBuffer();
}


inline void AddSlash(Str& strPath)
{
  if (strPath.GetLength() > 0)
  {
    if ((strPath.GetAt(strPath.GetLength()-1) != '/') &&
	(strPath.GetAt(strPath.GetLength()-1) != '\\'))
      strPath += '/';
  }
}

inline bool ExtractPath_and_Filename(const char* pPath, Str& strPath, Str& strFilename)
{
  Str strPathName;
  strPathName = pPath;
  const char* substr = strPathName.ReverseFind('\\');
  if (substr == 0)
    // TTimo: try forward slash, some are using forward
    substr = strPathName.ReverseFind('/');
  if (substr != 0)
  {
    std::size_t nSlash = substr - strPathName.GetBuffer();
    strPath = strPathName.Left(nSlash+1);
    strFilename = strPathName.Right(strPathName.GetLength() - nSlash - 1);
  }
  else
    strFilename = pPath;
  return true;
}



#endif
