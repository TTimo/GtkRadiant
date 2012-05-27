
#if !defined(INCLUDED_STREAM_TEXTFILESTREAM_H)
#define INCLUDED_STREAM_TEXTFILESTREAM_H

#include "itextstream.h"
#include <stdio.h>

/// \brief A wrapper around a file input stream opened for reading in text mode. Similar to std::ifstream.
class TextFileInputStream : public TextInputStream
{
  FILE* m_file;
public:
  TextFileInputStream(const char* name)
  {
    m_file = name[0] == '\0' ? 0 : fopen(name, "rt");
  }
  ~TextFileInputStream()
  {
    if(!failed())
      fclose(m_file);
  }

  bool failed() const
  {
    return m_file == 0;
  }

  std::size_t read(char* buffer, std::size_t length)
  {
    return fread(buffer, 1, length, m_file);
  }
};

/// \brief A wrapper around a file input stream opened for writing in text mode. Similar to std::ofstream.
class TextFileOutputStream : public TextOutputStream
{
  FILE* m_file;
public:
  TextFileOutputStream(const char* name)
  {
    m_file = name[0] == '\0' ? 0 : fopen(name, "wt");
  }
  ~TextFileOutputStream()
  {
    if(!failed())
      fclose(m_file);
  }

  bool failed() const
  {
    return m_file == 0;
  }

  std::size_t write(const char* buffer, std::size_t length)
  {
    return fwrite(buffer, 1, length, m_file);
  }
};

template<typename T>
inline TextFileOutputStream& operator<<(TextFileOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}


#endif
