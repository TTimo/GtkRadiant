/*
   Copyright (c) 2001, Loki software, inc.
   modifications (c) 2001, Id software, inc.
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

#ifndef _ISTREAM_H_
#define _ISTREAM_H_

/*!
   API for data streams

   Based on an initial implementation by Loki software
   modified to be abstracted and shared across modules

   NOTE: why IDataStream and not IStream? because IStream is defined in windows IDL headers
 */

class IDataStream
{
public:
IDataStream();
virtual ~IDataStream();

virtual void IncRef() = 0;      ///< Increment the number of references to this object
virtual void DecRef() = 0;      ///< Decrement the reference count

virtual unsigned long GetPosition() const = 0;
virtual unsigned long Seek( long lOff, int nFrom ) = 0;
virtual void SetLength( unsigned long nNewLen ) = 0;
virtual unsigned long GetLength() const = 0;

virtual char* ReadString( char* pBuf, unsigned long nMax ) = 0;
virtual unsigned long Read( void* pBuf, unsigned long nCount ) = 0;
virtual unsigned long Write( const void* pBuf, unsigned long nCount ) = 0;
virtual int GetChar() = 0;
virtual int PutChar( int c ) = 0;

virtual void printf( const char*, ... ) = 0;   ///< completely matches the usual printf behaviour

virtual void Abort() = 0;
virtual void Flush() = 0;
virtual void Close() = 0;
};

#endif // _ISTREAM_H_
