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

//
// Missing functions
//
// Leonardo Zide (leo@lokigames.com)
//

#if defined (__linux__) || defined (__APPLE__)

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "missing.h"

bool CopyFile(const char *lpExistingFileName, const char *lpNewFileName)
{
  FILE *src, *dst;
  void* buf;
  int l, ret = 0;
  char realsrc[PATH_MAX], realdest[PATH_MAX];

  realpath (lpExistingFileName, realsrc);
  realpath (lpNewFileName, realdest);

  src = fopen (realsrc, "rb");
  if (!src)
    return 0;
  dst = fopen (realdest, "wb");
  if (!dst)
  {
    fclose (src);
    return 0;
  }
 
  fseek (src, 0, SEEK_END);
  l = ftell (src);
  rewind (src);
  buf = g_malloc (l);

  if (buf != NULL)
    if (fread (buf, l, 1, src) == 1)
      if (fwrite (buf, l, 1, dst) == 1)
	ret = 1;

  g_free (buf);
  fclose (src);
  fclose (dst);

  return ret;
}

int GetFullPathName(const char *lpFileName, int nBufferLength, char *lpBuffer, char **lpFilePart)
{
  if (lpFileName[0] == '/')
  {
    strcpy (lpBuffer, lpFileName);
    *lpFilePart = strrchr (lpBuffer, '/');
    return strlen (lpBuffer);
  }

  if (getcwd (lpBuffer, nBufferLength) == NULL)
    return 0;

  strcat (lpBuffer, "/");
  *lpFilePart = lpBuffer + strlen (lpBuffer);
  strcat (lpBuffer, lpFileName);

  char *scr = lpBuffer, *dst = lpBuffer;
  for (int i = 0; (i < nBufferLength) && (*scr != 0); i++)
  {
    if (*scr == '/' && *(scr+1) == '.' && *(scr+2) == '.')
    {
      scr += 4;
      while (dst != lpBuffer && *dst != '/')
      {
	dst--;
	i--;
      }
    }

    *dst = *scr;

    scr++; dst++;
  }
  *dst = 0;

  return strlen (lpBuffer);
}
/*
static void g_string_sprintfa_int (GString *string, const gchar *fmt, va_list args)
{
  gchar *buffer;

  buffer = g_strdup_vprintf (fmt, args);
  g_string_append (string, buffer);
  g_free (buffer);
}

const CString& CString::operator=(const char* lpsz)
{
  g_string_assign (m_str, lpsz);
  return *this;
}

const CString& CString::operator+=(const char* lpsz)
{
  g_string_append (m_str, lpsz);
  return *this;
}

CString::operator char*() const
{ 
  return m_str->str;
}

void CString::Format(const char* fmt, ...)
{
  va_list args;
 
  g_string_truncate (m_str, 0);
 
  va_start (args, fmt);
  g_string_sprintfa_int (m_str, fmt, args);
  va_end (args);
}

CString CString::Right(int nCount) const
{
  if (nCount < 0)
    nCount = 0;
  else if (nCount > m_str->len)
    nCount = m_str->len;

  CString dest (&m_str->str[m_str->len-nCount]);
  return dest;
}

CString CString::Left(int nCount) const
{
  if (nCount < 0)
    nCount = 0;
  else if (nCount > m_str->len)
    nCount = m_str->len;

  CString dest;
  dest.m_str = g_string_sized_new (nCount);
  memcpy (dest.m_str->str, m_str->str, nCount);
  dest.m_str->str[nCount] = 0;
  return dest;
}

void CString::SetAt(int nIndex, char ch)
{
  if (nIndex >= 0 && nIndex < m_str->len)
    m_str->str[nIndex] = ch;
}

char CString::GetAt(int nIndex) const
{
  if (nIndex >= 0 && nIndex < m_str->len)
    return m_str->str[nIndex];
  return 0;
}

char CString::operator[](int nIndex) const
{
  if (nIndex >= 0 && nIndex < m_str->len)
    return m_str->str[nIndex];
  return 0;
}
*/

#endif
