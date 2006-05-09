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

#if !defined (INCLUDED_STRINGIO_H)
#define INCLUDED_STRINGIO_H

#include <stdlib.h>
#include <cctype>

#include "generic/vector.h"
#include "iscriplib.h"
#include "string/string.h"
#include "generic/callback.h"

inline float string_read_float(const char* string)
{
  return static_cast<float>(atof(string));
}

inline int string_read_int(const char* string)
{
  return atoi(string);
}

inline bool char_is_whitespace(char c)
{
  return c == ' ' || c == '\t';
}

inline const char* string_remove_whitespace(const char* string)
{
  for(;;)
  {
    if(!char_is_whitespace(*string))
    {
      break;
    }
    ++string;
  }
  return string;
}

inline const char* string_remove_zeros(const char* string)
{
  for(;;)
  {
    char c = *string;
    if(c != '0')
    {
      break;
    }
    ++string;
  }
  return string;
}

inline const char* string_remove_sign(const char* string)
{
  if(*string == '-' || *string == '+') // signed zero - acceptable
  {
    return ++string;
  }
  return string;
}

inline bool string_is_unsigned_zero(const char* string)
{
  for(;*string != '\0'; ++string)
  {
    if(*string != '0')
    {
      return false;
    }
  }
  return true;
}

inline bool string_is_signed_zero(const char* string)
{
  return string_is_unsigned_zero(string_remove_sign(string));
}

//[whitespaces][+|-][nnnnn][.nnnnn][e|E[+|-]nnnn]
//(where whitespaces are any tab or space character and nnnnn may be any number of digits)
inline bool string_is_float_zero(const char* string)
{
  string = string_remove_whitespace(string);
  if(string_empty(string))
  {
    return false;
  }

  string = string_remove_sign(string);
  if(string_empty(string))
  {
    // no whole number or fraction part
    return false;
  }

  // whole-number part
  string = string_remove_zeros(string);
  if(string_empty(string))
  {
    // no fraction or exponent
    return true;
  }
  if(*string == '.')
  {
    // fraction part
    if(*string++ != '0')
    {
      // invalid fraction
      return false;
    }
    string = string_remove_zeros(++string);
    if(string_empty(string))
    {
      // no exponent
      return true;
    }
  }
  if(*string == 'e' || *string == 'E')
  {
    // exponent part
    string = string_remove_sign(++string);
    if(*string++ != '0')
    {
      // invalid exponent
      return false;
    }
    string = string_remove_zeros(++string);
    if(string_empty(string))
    {
      // no trailing whitespace
      return true;
    }
  }
  string = string_remove_whitespace(string);
  return string_empty(string);
}

inline double buffer_parse_floating_literal(const char*& buffer)
{
  return strtod(buffer, const_cast<char**>(&buffer));
}

inline int buffer_parse_signed_decimal_integer_literal(const char*& buffer)
{
  return strtol(buffer, const_cast<char**>(&buffer), 10);
}

inline int buffer_parse_unsigned_decimal_integer_literal(const char*& buffer)
{
  return strtoul(buffer, const_cast<char**>(&buffer), 10);
}

// [+|-][nnnnn][.nnnnn][e|E[+|-]nnnnn]
inline bool string_parse_float(const char* string, float& f)
{
  if(string_empty(string))
  {
    return false;
  }
  f = float(buffer_parse_floating_literal(string));
  return string_empty(string);
}

// format same as float
inline bool string_parse_double(const char* string, double& f)
{
  if(string_empty(string))
  {
    return false;
  }
  f = buffer_parse_floating_literal(string);
  return string_empty(string);
}

// <float><space><float><space><float>
template<typename Element>
inline bool string_parse_vector3(const char* string, BasicVector3<Element>& v)
{
  if(string_empty(string) || *string == ' ')
  {
    return false;
  }
  v[0] = float(buffer_parse_floating_literal(string));
  if(*string++ != ' ')
  {
    return false;
  }
  v[1] = float(buffer_parse_floating_literal(string));
  if(*string++ != ' ')
  {
    return false;
  }
  v[2] = float(buffer_parse_floating_literal(string));
  return string_empty(string);
}

template<typename Float>
inline bool string_parse_vector(const char* string, Float* first, Float* last)
{
  if(first != last && (string_empty(string) || *string == ' '))
  {
    return false;
  }
  for(;;)
  {
    *first = float(buffer_parse_floating_literal(string));
    if(++first == last)
    {
      return string_empty(string);
    }
    if(*string++ != ' ')
    {
      return false;
    }
  }
}

// decimal signed integer
inline bool string_parse_int(const char* string, int& i)
{
  if(string_empty(string))
  {
    return false;
  }
  i = buffer_parse_signed_decimal_integer_literal(string);
  return string_empty(string);
}

// decimal unsigned integer
inline bool string_parse_size(const char* string, std::size_t& i)
{
  if(string_empty(string))
  {
    return false;
  }
  i = buffer_parse_unsigned_decimal_integer_literal(string);
  return string_empty(string);
}


#define RETURN_FALSE_IF_FAIL(expression) if(!expression) return false; else

inline void Tokeniser_unexpectedError(Tokeniser& tokeniser, const char* token, const char* expected)
{
  globalErrorStream() << Unsigned(tokeniser.getLine()) << ":" << Unsigned(tokeniser.getColumn()) << ": parse error at '" << (token != 0 ? token : "#EOF") << "': expected '" << expected << "'\n";
}


inline bool Tokeniser_getFloat(Tokeniser& tokeniser, float& f)
{
  const char* token = tokeniser.getToken();
  if(token != 0 && string_parse_float(token, f))
  {
    return true;
  }
  Tokeniser_unexpectedError(tokeniser, token, "#number");
  return false;
}

inline bool Tokeniser_getDouble(Tokeniser& tokeniser, double& f)
{
  const char* token = tokeniser.getToken();
  if(token != 0 && string_parse_double(token, f))
  {
    return true;
  }
  Tokeniser_unexpectedError(tokeniser, token, "#number");
  return false;
}

inline bool Tokeniser_getInteger(Tokeniser& tokeniser, int& i)
{
  const char* token = tokeniser.getToken();
  if(token != 0 && string_parse_int(token, i))
  {
    return true;
  }
  Tokeniser_unexpectedError(tokeniser, token, "#integer");
  return false;
}

inline bool Tokeniser_getSize(Tokeniser& tokeniser, std::size_t& i)
{
  const char* token = tokeniser.getToken();
  if(token != 0 && string_parse_size(token, i))
  {
    return true;
  }
  Tokeniser_unexpectedError(tokeniser, token, "#unsigned-integer");
  return false;
}

inline bool Tokeniser_parseToken(Tokeniser& tokeniser, const char* expected)
{
  const char* token = tokeniser.getToken();
  if(token != 0 && string_equal(token, expected))
  {
    return true;
  }
  Tokeniser_unexpectedError(tokeniser, token, expected);
  return false;
}

inline bool Tokeniser_nextTokenIsDigit(Tokeniser& tokeniser)
{
  const char* token = tokeniser.getToken();
  if(token == 0)
  {
    return false;
  }
  char c = *token;
  tokeniser.ungetToken();
  return std::isdigit(c) != 0;
}

template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& outputStream, const Vector3& v)
{
  return outputStream << '(' << v.x() << ' ' << v.y() << ' ' << v.z() << ')';
}




inline void CopiedString_importString(CopiedString& self, const char* string)
{
  self = string;
}
typedef ReferenceCaller1<CopiedString, const char*, CopiedString_importString> CopiedStringImportStringCaller;
inline void CopiedString_exportString(const CopiedString& self, const StringImportCallback& importer)
{
  importer(self.c_str());
}
typedef ConstReferenceCaller1<CopiedString, const StringImportCallback&, CopiedString_exportString> CopiedStringExportStringCaller;

inline void Bool_importString(bool& self, const char* string)
{
  self = string_equal(string, "true");
}
typedef ReferenceCaller1<bool, const char*, Bool_importString> BoolImportStringCaller;
inline void Bool_exportString(const bool& self, const StringImportCallback& importer)
{
  importer(self ? "true" : "false");
}
typedef ConstReferenceCaller1<bool, const StringImportCallback&, Bool_exportString> BoolExportStringCaller;

inline void Int_importString(int& self, const char* string)
{
  if(!string_parse_int(string, self))
  {
    self = 0;
  }
}
typedef ReferenceCaller1<int, const char*, Int_importString> IntImportStringCaller;
inline void Int_exportString(const int& self, const StringImportCallback& importer)
{
  char buffer[16];
  sprintf(buffer, "%d", self);
  importer(buffer);
}
typedef ConstReferenceCaller1<int, const StringImportCallback&, Int_exportString> IntExportStringCaller;

inline void Size_importString(std::size_t& self, const char* string)
{
  int i;
  if(string_parse_int(string, i) && i >= 0)
  {
    self = i;
  }
  else
  {
    self = 0;
  }
}
typedef ReferenceCaller1<std::size_t, const char*, Size_importString> SizeImportStringCaller;
inline void Size_exportString(const std::size_t& self, const StringImportCallback& importer)
{
  char buffer[16];
  sprintf(buffer, "%u", Unsigned(self));
  importer(buffer);
}
typedef ConstReferenceCaller1<std::size_t, const StringImportCallback&, Size_exportString> SizeExportStringCaller;

inline void Float_importString(float& self, const char* string)
{
  if(!string_parse_float(string, self))
  {
    self = 0;
  }
}
typedef ReferenceCaller1<float, const char*, Float_importString> FloatImportStringCaller;
inline void Float_exportString(const float& self, const StringImportCallback& importer)
{
  char buffer[16];
  sprintf(buffer, "%g", self);
  importer(buffer);
}
typedef ConstReferenceCaller1<float, const StringImportCallback&, Float_exportString> FloatExportStringCaller;

inline void Vector3_importString(Vector3& self, const char* string)
{
  if(!string_parse_vector3(string, self))
  {
    self = Vector3(0, 0, 0);
  }
}
typedef ReferenceCaller1<Vector3, const char*, Vector3_importString> Vector3ImportStringCaller;
inline void Vector3_exportString(const Vector3& self, const StringImportCallback& importer)
{
  char buffer[64];
  sprintf(buffer, "%g %g %g", self[0], self[1], self[2]);
  importer(buffer);
}
typedef ConstReferenceCaller1<Vector3, const StringImportCallback&, Vector3_exportString> Vector3ExportStringCaller;



template<typename FirstArgument, typename Caller, typename FirstConversion>
class ImportConvert1
{
public:
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    Caller::thunk(environment, FirstConversion(firstArgument));
  }
};


class BoolFromString
{
  bool m_value;
public:
  BoolFromString(const char* string)
  {
    Bool_importString(m_value, string);
  }
  operator bool() const
  {
    return m_value;
  }
};

inline void Bool_toString(const StringImportCallback& self, bool value)
{
  Bool_exportString(value, self);
}
typedef ConstReferenceCaller1<StringImportCallback, bool, Bool_toString> BoolToString;


template<typename Caller>
inline StringImportCallback makeBoolStringImportCallback(const Caller& caller)
{
  return StringImportCallback(caller.getEnvironment(), ImportConvert1<StringImportCallback::first_argument_type, Caller, BoolFromString>::thunk);
}

template<typename Caller>
inline StringExportCallback makeBoolStringExportCallback(const Caller& caller)
{
  return StringExportCallback(caller.getEnvironment(), ImportConvert1<StringExportCallback::first_argument_type, Caller, BoolToString>::thunk);
}


class IntFromString
{
  int m_value;
public:
  IntFromString(const char* string)
  {
    Int_importString(m_value, string);
  }
  operator int() const
  {
    return m_value;
  }
};

inline void Int_toString(const StringImportCallback& self, int value)
{
  Int_exportString(value, self);
}
typedef ConstReferenceCaller1<StringImportCallback, int, Int_toString> IntToString;


template<typename Caller>
inline StringImportCallback makeIntStringImportCallback(const Caller& caller)
{
  return StringImportCallback(caller.getEnvironment(), ImportConvert1<StringImportCallback::first_argument_type, Caller, IntFromString>::thunk);
}

template<typename Caller>
inline StringExportCallback makeIntStringExportCallback(const Caller& caller)
{
  return StringExportCallback(caller.getEnvironment(), ImportConvert1<StringExportCallback::first_argument_type, Caller, IntToString>::thunk);
}



class SizeFromString
{
  std::size_t m_value;
public:
  SizeFromString(const char* string)
  {
    Size_importString(m_value, string);
  }
  operator std::size_t() const
  {
    return m_value;
  }
};

inline void Size_toString(const StringImportCallback& self, std::size_t value)
{
  Size_exportString(value, self);
}
typedef ConstReferenceCaller1<StringImportCallback, std::size_t, Size_toString> SizeToString;


template<typename Caller>
inline StringImportCallback makeSizeStringImportCallback(const Caller& caller)
{
  return StringImportCallback(caller.getEnvironment(), ImportConvert1<StringImportCallback::first_argument_type, Caller, SizeFromString>::thunk);
}

template<typename Caller>
inline StringExportCallback makeSizeStringExportCallback(const Caller& caller)
{
  return StringExportCallback(caller.getEnvironment(), ImportConvert1<StringExportCallback::first_argument_type, Caller, SizeToString>::thunk);
}

#endif
