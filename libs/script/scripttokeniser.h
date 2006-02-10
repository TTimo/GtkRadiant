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

#if !defined(INCLUDED_SCRIPT_SCRIPTTOKENISER_H)
#define INCLUDED_SCRIPT_SCRIPTTOKENISER_H

#include "iscriplib.h"

class ScriptTokeniser : public Tokeniser
{
  enum CharType
  {
    eWhitespace,
    eCharToken,
    eNewline,
    eCharQuote,
    eCharSolidus,
    eCharStar,
    eCharSpecial,
  };

  typedef bool (ScriptTokeniser::*Tokenise)(char c);

  Tokenise m_stack[3];
  Tokenise* m_state;
  SingleCharacterInputStream<TextInputStream> m_istream;
  std::size_t m_scriptline;
  std::size_t m_scriptcolumn;

  char m_token[MAXTOKEN];
  char* m_write;

  char m_current;
  bool m_eof;
  bool m_crossline;
  bool m_unget;
  bool m_emit;

  bool m_special;

  CharType charType(const char c)
  {
    switch(c)
    {
    case '\n': return eNewline;
    case '"': return eCharQuote;
    case '/': return eCharSolidus;
    case '*': return eCharStar;
    case '{': case '(': case '}': case ')': case '[': case ']': case ',': case ':': return (m_special) ? eCharSpecial : eCharToken;
    }
    
    if(c > 32)
    {
      return eCharToken;
    }
    return eWhitespace;
  }

  Tokenise state()
  {
    return *m_state;
  }
  void push(Tokenise state)
  {
    ASSERT_MESSAGE(m_state != m_stack + 2, "token parser: illegal stack push");
    *(++m_state) = state;
  }
  void pop()
  {
    ASSERT_MESSAGE(m_state != m_stack, "token parser: illegal stack pop");
    --m_state;
  }
  void add(const char c)
  {
    if(m_write < m_token + MAXTOKEN - 1)
    {
      *m_write++ = c;
    }
  }
  void remove()
  {
    ASSERT_MESSAGE(m_write > m_token, "no char to remove");
    --m_write;
  }

  bool tokeniseDefault(char c)
  {
    switch(charType(c))
    {
    case eNewline:
      if(!m_crossline)
      {
        globalErrorStream() << Unsigned(getLine()) << ":" << Unsigned(getColumn()) << ": unexpected end-of-line before token\n";
        return false;
      }
      break;
    case eCharToken:
    case eCharStar:
      push(Tokenise(&ScriptTokeniser::tokeniseToken));
      add(c);
      break;
    case eCharSpecial:
      push(Tokenise(&ScriptTokeniser::tokeniseSpecial));
      add(c);
      break;
    case eCharQuote:
      push(Tokenise(&ScriptTokeniser::tokeniseQuotedToken));
      break;
    case eCharSolidus:
      push(Tokenise(&ScriptTokeniser::tokeniseSolidus));
      break;
    default:
      break;
    }
    return true;
  }
  bool tokeniseToken(char c)
  {
    switch(charType(c))
    {
    case eNewline:
    case eWhitespace:
    case eCharQuote:
    case eCharSpecial:
      pop();
      m_emit = true; // emit token
      break;
    case eCharSolidus:
#if 0 //SPoG: ignore comments in the middle of tokens. 
      push(Tokenise(&ScriptTokeniser::tokeniseSolidus));
      break;
#endif
    case eCharToken:
    case eCharStar:
      add(c);
      break;
    default:
      break;
    }
    return true;
  }
  bool tokeniseQuotedToken(char c)
  {
    switch(charType(c))
    {
    case eNewline:
      if(m_crossline)
      {
        globalErrorStream() << Unsigned(getLine()) << ":" << Unsigned(getColumn()) << ": unexpected end-of-line in quoted token\n";
        return false;
      }
      break;
    case eWhitespace:
    case eCharToken:
    case eCharSolidus:
    case eCharStar:
    case eCharSpecial:
      add(c);
      break;
    case eCharQuote:
      pop();
      push(Tokenise(&ScriptTokeniser::tokeniseEndQuote));
      break;
    default:
      break;
    }
    return true;
  }
  bool tokeniseSolidus(char c)
  {
    switch(charType(c))
    {
    case eNewline:
    case eWhitespace:
    case eCharQuote:
    case eCharSpecial:
      pop();
      add('/');
      m_emit = true; // emit single slash
      break;
    case eCharToken:
      pop();
      add('/');
      add(c);
      break;
    case eCharSolidus:
      pop();
      push(Tokenise(&ScriptTokeniser::tokeniseComment));
      break; // dont emit single slash
    case eCharStar:
      pop();
      push(Tokenise(&ScriptTokeniser::tokeniseBlockComment));
      break; // dont emit single slash
    default:
      break;
    }
    return true;
  }
  bool tokeniseComment(char c)
  {
    if(c == '\n')
    {
      pop();
      if(state() == Tokenise(&ScriptTokeniser::tokeniseToken))
      {
        pop();
        m_emit = true; // emit token immediatly preceding comment
      }
    }
    return true;
  }
  bool tokeniseBlockComment(char c)
  {
    if(c == '*')
    {
      pop();
      push(Tokenise(&ScriptTokeniser::tokeniseEndBlockComment));
    }
    return true;
  }
  bool tokeniseEndBlockComment(char c)
  {
    switch(c)
    {
    case '/':
      pop();
      if(state() == Tokenise(&ScriptTokeniser::tokeniseToken))
      {
        pop();
        m_emit = true; // emit token immediatly preceding comment
      }
      break; // dont emit comment
    case '*':
      break; // no state change
    default:
      pop();
      push(Tokenise(&ScriptTokeniser::tokeniseBlockComment));
      break;
    }
    return true;
  }
  bool tokeniseEndQuote(char c)
  {
    pop();
    m_emit = true; // emit quoted token
    return true;
  }
  bool tokeniseSpecial(char c)
  {
    pop();
    m_emit = true; // emit single-character token
    return true;
  }

  /// Returns true if a token was successfully parsed.
  bool tokenise()
  {
    m_write = m_token;
    while(!eof())
    {
      char c = m_current;

      if(!((*this).*state())(c))
      {
        // parse error
        m_eof = true;
        return false;
      }
      if(m_emit)
      {
        m_emit = false;
        return true;
      }

      if(c == '\n')
      {
        ++m_scriptline;
        m_scriptcolumn = 1;
      }
      else
      {
        ++m_scriptcolumn;
      }

      m_eof = !m_istream.readChar(m_current);
    }
    return m_write != m_token;
  }

  const char* fillToken()
  {
    if(!tokenise())
    {
      return 0;
    }

    add('\0');
    return m_token;
  }

  bool eof()
  {
    return m_eof;
  }

public:
  ScriptTokeniser(TextInputStream& istream, bool special)
    : m_state(m_stack),
    m_istream(istream),
    m_scriptline(1),
    m_scriptcolumn(1),
    m_crossline(false),
    m_unget(false),
    m_emit(false),
    m_special(special)
  {
    m_stack[0] = Tokenise(&ScriptTokeniser::tokeniseDefault);
    m_eof = !m_istream.readChar(m_current);
    m_token[MAXTOKEN - 1] = '\0';
  }
  void release()
  {
    delete this;
  }
  void nextLine()
  {
    m_crossline = true;
  }
  const char* getToken()
  {
    if(m_unget)
    {
      m_unget = false;
      return m_token;
    }
    
    return fillToken();
  }
  void ungetToken()
  {
    ASSERT_MESSAGE(!m_unget, "can't unget more than one token");
    m_unget = true;
  }
  std::size_t getLine() const
  {
    return m_scriptline;
  }
  std::size_t getColumn() const
  {
    return m_scriptcolumn;
  }
};


inline Tokeniser& NewScriptTokeniser(TextInputStream& istream)
{
  return *(new ScriptTokeniser(istream, true));
}

inline Tokeniser& NewSimpleTokeniser(TextInputStream& istream)
{
  return *(new ScriptTokeniser(istream, false));
}

#endif
