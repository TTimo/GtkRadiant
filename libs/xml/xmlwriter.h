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

#if !defined(INCLUDED_XML_XMLWRITER_H)
#define INCLUDED_XML_XMLWRITER_H

#include "convert.h"
#include <vector>
#include "xml/ixml.h"

class XMLEntityOutputStream
{
  SingleCharacterOutputStream m_ostream;
public:
  XMLEntityOutputStream(TextOutputStream& ostream)
    : m_ostream(ostream)
  {
  }
  void write(const char c)
  {
    m_ostream.write(c);
  }
  void writeEscaped(const char c)
  {
    switch(c)
    {
    case '<':
      write('&');
      write('l');
      write('t');
      write(';');
      break;
    case '>':
      write('&');
      write('g');
      write('t');
      write(';');
      break;
    case '"':
      write('&');
      write('q');
      write('u');
      write('o');
      write('t');
      write(';');
      break;
    case '&':
      write('&');
      write('a');
      write('m');
      write('p');
      write(';');
      break;
    default:
      write(c);
      break;
    }
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    const char*const end = buffer + length;
    for(const char* p = buffer; p != end; ++p)
    {
      writeEscaped(*p);
    }
    return length;
  }
};

template<typename T>
inline XMLEntityOutputStream& operator<<(XMLEntityOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}


class XMLStreamWriter : public XMLImporter, public XMLAttrVisitor
{
  class state_t
  {
  public:
    enum EState
    {
      eStartElement,
      eContent,
    };
    state_t()
      : m_state(eStartElement)
    {}
    EState m_state;
  };

  XMLEntityOutputStream m_ostream;
  std::vector<state_t> m_elements;

  void write_cdata(const char* buffer, std::size_t length)
  {
    m_ostream << ConvertLocaleToUTF8(StringRange(buffer, buffer + length));
  }
  void write_string(const char* string)
  {
    m_ostream << string;
  }
  void write_quoted_string(const char* string)
  {
    m_ostream.write('"');
    m_ostream << string;
    m_ostream.write('"');
  }
public:
  XMLStreamWriter(TextOutputStream& ostream)
    : m_ostream(ostream)
  {
    m_elements.push_back(state_t());
    m_elements.back().m_state = state_t::eContent;
    m_ostream.write('<');
    m_ostream.write('?');
    write_string("xml");
    visit("version", "1.0");
    m_ostream.write('?');
    m_ostream.write('>');
  }

  void pushElement(const XMLElement& element)
  {
    if(m_elements.back().m_state == state_t::eStartElement)
    {
      m_elements.back().m_state = state_t::eContent;
      m_ostream.write('>');
    }

    m_elements.push_back(state_t());

    m_ostream.write('<');
    write_string(element.name());
    element.forEachAttribute(*this);
  }
  void popElement(const char* name)
  {
    if(m_elements.back().m_state == state_t::eStartElement)
    {
      m_ostream.write('/');
      m_ostream.write('>');
    }
    else
    {
      m_ostream.write('<');
      m_ostream.write('/');
      write_string(name);
      m_ostream.write('>');
    }
  }
  std::size_t write(const char* data, std::size_t length)
  {
    if(m_elements.back().m_state == state_t::eStartElement)
    {
      m_elements.back().m_state = state_t::eContent;
      m_ostream.write('>');
    }
    write_cdata(data, length);
    return length;
  }

  void visit(const char* name, const char* value)
  {
    m_ostream.write(' ');
    write_string(name);
    m_ostream.write('=');
    write_quoted_string(value);
  }
};


#endif
