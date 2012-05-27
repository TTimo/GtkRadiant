
#if !defined(INCLUDED_SCRIPT_SCRIPTTOKENWRITER_H)
#define INCLUDED_SCRIPT_SCRIPTTOKENWRITER_H

#include "iscriplib.h"

class SimpleTokenWriter : public TokenWriter
{
public:
  SimpleTokenWriter(TextOutputStream& ostream)
    : m_ostream(ostream), m_separator('\n')
  {
  }
  ~SimpleTokenWriter()
  {
    writeSeparator();
  }
  void release()
  {
    delete this;
  }
  void nextLine()
  {
    m_separator = '\n';
  }
  void writeToken(const char* token)
  {
    ASSERT_MESSAGE(strchr(token, ' ') == 0, "token contains whitespace: ");
    writeSeparator();
    m_ostream << token;
  }
  void writeString(const char* string)
  {
    writeSeparator();
    m_ostream << '"' << string << '"';
  }
  void writeInteger(int i)
  {
    writeSeparator();
    m_ostream << i;
  }
  void writeUnsigned(std::size_t i)
  {
    writeSeparator();
    m_ostream << Unsigned(i);
  }
  void writeFloat(double f)
  {
    writeSeparator();
    m_ostream << Decimal(f);
  }

private:
  void writeSeparator()
  {
    m_ostream << m_separator;
    m_separator = ' ';
  }
  TextOutputStream& m_ostream;
  char m_separator;
};

inline TokenWriter& NewSimpleTokenWriter(TextOutputStream& ostream)
{
  return *(new SimpleTokenWriter(ostream));
}

#endif
