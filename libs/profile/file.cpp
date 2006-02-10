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
// File class, can be a memory file or a regular disk file.
// Originally from LeoCAD, used with permission from the author. :)
//
// Leonardo Zide (leo@lokigames.com)
//

#include "file.h"

#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////
// File construction/destruction

MemStream::MemStream()
{
  m_nGrowBytes = 1024;
  m_nPosition = 0;
  m_nBufferSize = 0;
  m_nFileSize = 0;
  m_pBuffer = NULL;
  m_bAutoDelete = true;
}

MemStream::MemStream(size_type nLen)
{
  m_nGrowBytes = 1024;
  m_nPosition = 0;
  m_nBufferSize = 0;
  m_nFileSize = 0;
  m_pBuffer = NULL;
  m_bAutoDelete = true;

  GrowFile (nLen);
}

FileStream::FileStream()
{
  m_hFile = NULL;
  m_bCloseOnDelete = false;
}

MemStream::~MemStream()
{
  if (m_pBuffer)
    Close();

  m_nGrowBytes = 0;
  m_nPosition = 0;
  m_nBufferSize = 0;
  m_nFileSize = 0;
}

FileStream::~FileStream()
{
  if (m_hFile != NULL && m_bCloseOnDelete)
    Close();
}

/////////////////////////////////////////////////////////////////////////////
// File operations

char* MemStream::ReadString(char* pBuf, size_type nMax)
{
  int nRead = 0;
  unsigned char ch;

  if (nMax <= 0)
    return NULL;
  if (m_nPosition >= m_nFileSize)
    return NULL;

  while ((--nMax))
  {
    if (m_nPosition == m_nFileSize)
      break;

    ch = m_pBuffer[m_nPosition];
    m_nPosition++;
    pBuf[nRead++] = ch;

    if (ch == '\n')
      break;
  }

  pBuf[nRead] = '\0';
  return pBuf;
}

char* FileStream::ReadString(char* pBuf, size_type nMax)
{
  return fgets(pBuf, static_cast<int>(nMax), m_hFile);
}

MemStream::size_type MemStream::read(byte_type* buffer, size_type length)
{
  if (length == 0)
    return 0;

  if (m_nPosition > m_nFileSize)
    return 0;

  size_type nRead;
  if (m_nPosition + length > m_nFileSize)
    nRead = m_nFileSize - m_nPosition;
  else
    nRead = length;

  memcpy((unsigned char*)buffer, (unsigned char*)m_pBuffer + m_nPosition, nRead);
  m_nPosition += nRead;

  return nRead;
}

FileStream::size_type FileStream::read(byte_type* buffer, size_type length)
{
  return fread(buffer, 1, length, m_hFile);
}

int MemStream::GetChar()
{
  if (m_nPosition > m_nFileSize)
    return 0;

  unsigned char* ret = (unsigned char*)m_pBuffer + m_nPosition;
  m_nPosition++;

  return *ret;
}

int FileStream::GetChar()
{
  return fgetc(m_hFile);
}

MemStream::size_type MemStream::write(const byte_type* buffer, size_type length)
{
  if (length == 0)
    return 0;

  if (m_nPosition + length > m_nBufferSize)
    GrowFile(m_nPosition + length);

  memcpy((unsigned char*)m_pBuffer + m_nPosition, (unsigned char*)buffer, length);

  m_nPosition += size_type(length);

  if (m_nPosition > m_nFileSize)
    m_nFileSize = m_nPosition;

  return length;
}

FileStream::size_type FileStream::write(const byte_type* buffer, size_type length)
{
  return fwrite(buffer, 1, length, m_hFile);
}

int MemStream::PutChar(int c)
{
  if (m_nPosition + 1 > m_nBufferSize)
    GrowFile(m_nPosition + 1);

  unsigned char* bt = (unsigned char*)m_pBuffer + m_nPosition;
  *bt = c;

  m_nPosition++;

  if (m_nPosition > m_nFileSize)
    m_nFileSize = m_nPosition;

  return 1;
}

/*!\todo SPoG suggestion: replace printf with operator >> using c++ iostream and strstream */
void FileStream::printf(const char* s, ...)
{
  va_list args;

  va_start (args, s);
  vfprintf(m_hFile, s, args);
  va_end (args);
}

/*!\todo SPoG suggestion: replace printf with operator >> using c++ iostream and strstream */
void MemStream::printf(const char* s, ...)
{
  va_list args;

  char buffer[4096];
  va_start (args, s);
  vsprintf(buffer, s, args);
  va_end (args);
  write(reinterpret_cast<byte_type*>(buffer), strlen(buffer));
}

int FileStream::PutChar(int c)
{
  return fputc(c, m_hFile);
}

bool FileStream::Open(const char *filename, const char *mode)
{
  m_hFile = fopen(filename, mode);
  m_bCloseOnDelete = true;

  return (m_hFile != NULL);
}

void MemStream::Close()
{
  m_nGrowBytes = 0;
  m_nPosition = 0;
  m_nBufferSize = 0;
  m_nFileSize = 0;
  if (m_pBuffer && m_bAutoDelete)
    free(m_pBuffer);
  m_pBuffer = NULL;
}

void FileStream::Close()
{
  if (m_hFile != NULL)
    fclose(m_hFile);

  m_hFile = NULL;
  m_bCloseOnDelete = false;
}

int MemStream::Seek(offset_type lOff, int nFrom)
{
  size_type lNewPos = m_nPosition;

  if (nFrom == SEEK_SET)
    lNewPos = lOff;
  else if (nFrom == SEEK_CUR)
    lNewPos += lOff;
  else if (nFrom == SEEK_END)
    lNewPos = m_nFileSize + lOff;
  else
    return (position_type)-1;

  m_nPosition = lNewPos;

  return static_cast<int>(m_nPosition);
}

int FileStream::Seek(offset_type lOff, int nFrom)
{
  fseek (m_hFile, lOff, nFrom);

  return ftell(m_hFile);
}

MemStream::position_type MemStream::GetPosition() const
{
    return m_nPosition;
}

FileStream::position_type FileStream::GetPosition() const
{
  return ftell(m_hFile);
}

void MemStream::GrowFile(size_type nNewLen)
{
  if (nNewLen > m_nBufferSize)
  {
    // grow the buffer
    size_type nNewBufferSize = m_nBufferSize;

    // determine new buffer size
    while (nNewBufferSize < nNewLen)
      nNewBufferSize += m_nGrowBytes;

    // allocate new buffer
    unsigned char* lpNew;
    if (m_pBuffer == NULL)
      lpNew = static_cast<unsigned char*>(malloc(nNewBufferSize));
    else
      lpNew = static_cast<unsigned char*>(realloc(m_pBuffer, nNewBufferSize));

    m_pBuffer = lpNew;
    m_nBufferSize = nNewBufferSize;
  }
}

void MemStream::Flush()
{
  // Nothing to be done
}

void FileStream::Flush()
{
  if (m_hFile == NULL)
    return;

  fflush(m_hFile);
}

void MemStream::Abort()
{
  Close();
}

void FileStream::Abort()
{
  if (m_hFile != NULL)
  {
    // close but ignore errors
    if (m_bCloseOnDelete)
      fclose(m_hFile);
    m_hFile = NULL;
    m_bCloseOnDelete = false;
  }
}

void MemStream::SetLength(size_type nNewLen)
{
  if (nNewLen > m_nBufferSize)
    GrowFile(nNewLen);

  if (nNewLen < m_nPosition)
    m_nPosition = nNewLen;

  m_nFileSize = nNewLen;
}

void FileStream::SetLength(size_type nNewLen)
{
  fseek(m_hFile, static_cast<long>(nNewLen), SEEK_SET);
}

MemStream::size_type MemStream::GetLength() const
{
  return m_nFileSize;
}

FileStream::size_type FileStream::GetLength() const
{
  size_type nLen, nCur;

  // Seek is a non const operation
  nCur = ftell(m_hFile);
  fseek(m_hFile, 0, SEEK_END);
  nLen = ftell(m_hFile);
  fseek(m_hFile, static_cast<long>(nCur), SEEK_SET);

  return nLen;
}
