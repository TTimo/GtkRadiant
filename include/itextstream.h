/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_ITEXTSTREAM_H)
#define INCLUDED_ITEXTSTREAM_H

/// \file
/// \brief Text-stream interfaces.

#include <cstddef>
#include "generic/static.h"

/// \brief A read-only character-stream.
class TextInputStream
{
public:
  /// \brief Attempts to read the next \p length characters from the stream to \p buffer.
  /// Returns the number of characters actually stored in \p buffer.
  virtual std::size_t read(char* buffer, std::size_t length) = 0;
};

/// \brief A write-only character-stream.
class TextOutputStream
{
public:
  /// \brief Attempts to write \p length characters to the stream from \p buffer.
  /// Returns the number of characters actually read from \p buffer.
  virtual std::size_t write(const char* buffer, std::size_t length) = 0;
};

/// \brief Calls the overloaded function ostream_write() to perform text formatting specific to the type being written.
/*! Note that ostream_write() is not globally defined - it must be defined once for each type supported.\n
To support writing a custom type MyClass to any kind of text-output-stream with operator<<:
\code
template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const MyClass& myClass)
{
  return ostream << myClass.getName() << ' ' << myClass.getText();
}
\endcode
Expressing this as a template allows it to be used directly with any concrete text-output-stream type, not just the abstract TextOutputStream\n
\n
This overload writes a single character to any text-output-stream - ostream_write(TextOutputStreamType& ostream, char c).
*/
template<typename T>
inline TextOutputStream& operator<<(TextOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}

class NullOutputStream : public TextOutputStream
{
public:
  std::size_t write(const char*, std::size_t length)
  {
    return length;
  }
};

class OutputStreamHolder
{
  NullOutputStream m_nullOutputStream;
  TextOutputStream* m_outputStream;
public:
  OutputStreamHolder()
    : m_outputStream(&m_nullOutputStream)
  {
  }
  void setOutputStream(TextOutputStream& outputStream)
  {
    m_outputStream = &outputStream;
  }
  TextOutputStream& getOutputStream()
  {
    return *m_outputStream;
  }
};

typedef Static<OutputStreamHolder> GlobalOutputStream;

/// \brief Returns the global output stream. Used to display messages to the user.
inline TextOutputStream& globalOutputStream()
{
  return GlobalOutputStream::instance().getOutputStream();
}

class ErrorStreamHolder : public OutputStreamHolder {};
typedef Static<ErrorStreamHolder> GlobalErrorStream;

/// \brief Returns the global error stream. Used to display error messages to the user.
inline TextOutputStream& globalErrorStream()
{
  return GlobalErrorStream::instance().getOutputStream();
}

#endif
