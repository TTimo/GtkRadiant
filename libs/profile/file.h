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
//  file.h
////////////////////////////////////////////////////

#if !defined(INCLUDED_PROFILE_FILE_H)
#define INCLUDED_PROFILE_FILE_H

#include "idatastream.h"

/*!
API for data streams

Based on an initial implementation by Loki software
modified to be abstracted and shared across modules

NOTE: why IDataStream and not IStream? because IStream is defined in windows IDL headers
*/

class IDataStream : public InputStream, public OutputStream
{
public:  
  typedef int offset_type;
  typedef std::size_t position_type;

  virtual void IncRef() = 0;  ///< Increment the number of references to this object
  virtual void DecRef() = 0; ///< Decrement the reference count

  virtual position_type GetPosition() const = 0;
  virtual int Seek(offset_type lOff, int nFrom) = 0;

  virtual void SetLength(size_type nNewLen) = 0;
  virtual size_type GetLength() const = 0;

  virtual char* ReadString(char* pBuf, size_type nMax) = 0;
  virtual int GetChar()=0;

  virtual int PutChar(int c)=0;
  virtual void printf(const char*, ...) = 0; ///< completely matches the usual printf behaviour

  virtual void Abort() = 0;
  virtual void Flush() = 0;
  virtual void Close() = 0;
};

#include <stdio.h>

class MemStream : public IDataStream
{
public:
  MemStream();
  MemStream(size_type nLen);
  virtual ~MemStream();

  int refCount;
  void IncRef() { refCount++; }
  void DecRef() { refCount--; if (refCount <= 0) delete this; }

protected:
  // MemFile specific:
  size_type m_nGrowBytes;
  size_type m_nPosition;
  size_type m_nBufferSize;
  size_type m_nFileSize;
  unsigned char* m_pBuffer;
  bool m_bAutoDelete;
  void GrowFile(size_type nNewLen);

public:
  position_type GetPosition() const;
  int Seek(offset_type lOff, int nFrom);
  void SetLength(size_type nNewLen);
  size_type GetLength() const;

  unsigned char* GetBuffer() const
    { return m_pBuffer; }

  size_type read(byte_type* buffer, size_type length);
  size_type write(const byte_type* buffer, size_type length);

  char* ReadString(char* pBuf, size_type nMax);
  int GetChar();

  int PutChar(int c);
  void printf(const char*, ...); ///< \todo implement on MemStream

  void Abort();
  void Flush();
  void Close();
  bool Open(const char *filename, const char *mode);
};

class FileStream : public IDataStream
{
public:
  FileStream();
  virtual ~FileStream();

  int refCount;
  void IncRef() { refCount++; }
  void DecRef() { refCount--; if (refCount <= 0) delete this; }

protected:
  // DiscFile specific:
  FILE* m_hFile;
  bool m_bCloseOnDelete;

public:
  position_type GetPosition() const;
  int Seek(offset_type lOff, int nFrom);
  void SetLength(size_type nNewLen);
  size_type GetLength() const;

  size_type read(byte_type* buffer, size_type length);
  size_type write(const byte_type* buffer, size_type length);

  char* ReadString(char* pBuf, size_type nMax);
  int GetChar();

  int PutChar(int c);
  void printf(const char*, ...); ///< completely matches the usual printf behaviour

  void Abort();
  void Flush();
  void Close();
  bool Open(const char *filename, const char *mode);
};

#endif
