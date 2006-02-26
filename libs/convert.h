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

#if !defined(INCLUDED_CONVERT_H)
#define INCLUDED_CONVERT_H

/// \file
/// \brief Character encoding conversion.

#include "debugging/debugging.h"
#include <algorithm>
#include <glib/gunicode.h>
#include <glib/gconvert.h>

#include "character.h"

/// \brief Returns the number of bytes required to represent \p character in UTF-8 encoding.
inline std::size_t utf8_character_length(const char* character)
{
  if((*character & 0xE0) == 0xC0) // 110xxxxx
  {
    return 2;
  }
  else if((*character & 0xF0) == 0xE0) // 1110xxxx
  {
    return 3;
  }
  else if((*character & 0xF8) == 0xF0) // 11110xxx
  {
    return 4;
  }
  else if((*character & 0xFC) == 0xF8) // 111110xx
  {
    return 5;
  }
  else if((*character & 0xFE) == 0xFC) // 1111110x
  {
    return 6;
  }
  ERROR_MESSAGE("");
  return 0;
}

struct UTF8Character
{
  const char* buffer;
  std::size_t length;
  UTF8Character() : buffer(0), length(0)
  {
  }
  UTF8Character(const char* bytes) : buffer(bytes), length(utf8_character_length(bytes))
  {
  }
};

inline bool operator<(const UTF8Character& self, const UTF8Character& other)
{
  return std::lexicographical_compare(self.buffer, self.buffer + self.length, other.buffer, other.buffer + other.length);
}

/// \brief Writes \p c to \p ostream in Hex form. Useful for debugging.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const UTF8Character& c)
{
  for(const char* p = c.buffer; p != c.buffer + c.length; ++p)
  {
    ostream << HexChar(*p);
  }
  return ostream;
}



/// \brief The character-set encoding for the current C locale.
///
/// Obtain the global instance with globalCharacterSet().
class CharacterSet
{
  const char* m_charSet;
public:
  CharacterSet()
  {
    if(g_get_charset(&m_charSet) != FALSE)
    {
      m_charSet = 0;
    }
  }
  bool isUTF8() const
  {
    return m_charSet == 0;
  }
  const char* get() const
  {
    return m_charSet;
  }
};

typedef LazyStatic<CharacterSet> GlobalCharacterSet;

/// \brief Returns the global instance of CharacterSet.
inline CharacterSet& globalCharacterSet()
{
  return GlobalCharacterSet::instance();
}


class UTF8CharacterToExtendedASCII
{
public:
  UTF8Character m_utf8;
  char m_c;
  UTF8CharacterToExtendedASCII() : m_c('\0')
  {
  }
  UTF8CharacterToExtendedASCII(const UTF8Character& utf8, char c) : m_utf8(utf8), m_c(c)
  {
  }
};

inline bool operator<(const UTF8CharacterToExtendedASCII& self, const UTF8CharacterToExtendedASCII& other)
{
  return self.m_utf8 < other.m_utf8;
}

inline std::size_t extended_ascii_to_index(char c)
{
  return static_cast<std::size_t>(c & 0x7F);
}

inline char extended_ascii_for_index(std::size_t i)
{
  return static_cast<char>(i | 0x80);
}

/// \brief The active extended-ascii character set encoding.
/// Performs UTF-8 encoding and decoding of extended-ascii characters.
///
/// Obtain the global instance with globalExtendedASCIICharacterSet().
class ExtendedASCIICharacterSet
{
  typedef char UTF8CharBuffer[6];
  UTF8CharBuffer m_converted[128];
  UTF8Character m_decodeMap[128];
  UTF8CharacterToExtendedASCII m_encodeMap[128];
public:
  ExtendedASCIICharacterSet()
  {
    if(!globalCharacterSet().isUTF8())
    {
      GIConv descriptor = g_iconv_open("UTF-8", globalCharacterSet().get());
      for(std::size_t i = 1; i < 128; ++i)
      {
        char c = extended_ascii_for_index(i);
        char* inbuf = &c;
        std::size_t inbytesleft = 1;
        char* outbuf = m_converted[i];
        std::size_t outbytesleft = 6;
        if(g_iconv(descriptor, &inbuf, &inbytesleft, &outbuf, &outbytesleft) != (size_t)(-1))
        {
          UTF8Character utf8(m_converted[i]);
          m_decodeMap[i] = utf8;
          m_encodeMap[i] = UTF8CharacterToExtendedASCII(utf8, c);
        }
      }
      g_iconv_close(descriptor);
      std::sort(m_encodeMap, m_encodeMap + 128);
    }
  }
  /// \brief Prints the (up to) 128 characters in the current extended-ascii character set.
  /// Useful for debugging.
  void print() const
  {
    globalOutputStream() << "UTF-8 conversion required from charset: " << globalCharacterSet().get() << "\n";
    for(std::size_t i = 1; i < 128; ++i)
    {
      if(m_decodeMap[i].buffer != 0)
      {
        globalOutputStream() << extended_ascii_for_index(i) << " = " << m_decodeMap[i] << "\n";
      }
    }
  }
  /// \brief Returns \p c decoded from extended-ascii to UTF-8.
  /// \p c must be an extended-ascii character.
  const UTF8Character& decode(char c) const
  {
    ASSERT_MESSAGE(!globalCharacterSet().isUTF8(), "locale is utf8, no conversion required");
    ASSERT_MESSAGE(!char_is_ascii(c), "decode: ascii character");
    ASSERT_MESSAGE(m_decodeMap[extended_ascii_to_index(c)].buffer != 0, "decode: invalid character: " << HexChar(c));
    return m_decodeMap[extended_ascii_to_index(c)];
  }
  /// \brief Returns \p c encoded to extended-ascii from UTF-8.
  /// \p c must map to an extended-ascii character.
  char encode(const UTF8Character& c) const
  {
    ASSERT_MESSAGE(!globalCharacterSet().isUTF8(), "locale is utf8, no conversion required");
    ASSERT_MESSAGE(!char_is_ascii(*c.buffer), "encode: ascii character");
    std::pair<const UTF8CharacterToExtendedASCII*, const UTF8CharacterToExtendedASCII*> range
      = std::equal_range(m_encodeMap, m_encodeMap + 128, UTF8CharacterToExtendedASCII(c, 0));
    ASSERT_MESSAGE(range.first != range.second, "encode: invalid character: " << c);
    return (*range.first).m_c;
  }
};

typedef LazyStatic<ExtendedASCIICharacterSet> GlobalExtendedASCIICharacterSet;

/// \brief Returns the global instance of ExtendedASCIICharacterSet.
inline ExtendedASCIICharacterSet& globalExtendedASCIICharacterSet()
{
  return GlobalExtendedASCIICharacterSet::instance();
}

class ConvertUTF8ToLocale
{
public:
  StringRange m_range;
  ConvertUTF8ToLocale(const char* string) : m_range(StringRange(string, string + strlen(string)))
  {
  }
  ConvertUTF8ToLocale(const StringRange& range) : m_range(range)
  {
  }
};

/// \brief Writes \p convert to \p ostream after encoding each character to extended-ascii from UTF-8.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const ConvertUTF8ToLocale& convert)
{
  if(globalCharacterSet().isUTF8())
  {
    return ostream << convert.m_range;
  }

  for(const char* p = convert.m_range.first; p != convert.m_range.last;)
  {
    if(!char_is_ascii(*p))
    {
      UTF8Character c(p);
      ostream << globalExtendedASCIICharacterSet().encode(c);
      p += c.length;
    }
    else
    {
      ostream << *p++;
    }
  }
  return ostream; 
}


class ConvertLocaleToUTF8
{
public:
  StringRange m_range;
  ConvertLocaleToUTF8(const char* string) : m_range(StringRange(string, string + strlen(string)))
  {
  }
  ConvertLocaleToUTF8(const StringRange& range) : m_range(range)
  {
  }
};

/// \brief Writes \p convert to \p ostream after decoding each character from extended-ascii to UTF-8.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const ConvertLocaleToUTF8& convert)
{
  if(globalCharacterSet().isUTF8())
  {
    return ostream << convert.m_range;
  }

  for(const char* p = convert.m_range.first; p != convert.m_range.last; ++p)
  {
    if(!char_is_ascii(*p))
    {
      UTF8Character c(globalExtendedASCIICharacterSet().decode(*p));
      ostream.write(c.buffer, c.length);
    }
    else
    {
      ostream << *p;
    }
  }
  return ostream; 
}


#endif
