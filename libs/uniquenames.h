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

#if !defined(INCLUDED_UNIQUENAMES_H)
#define INCLUDED_UNIQUENAMES_H

#include "debugging/debugging.h"
#include <map>
#include "string/string.h"
#include "generic/static.h"

#if 1
class Postfix
{
  unsigned int m_value;
public:
  Postfix(const char* postfix) : m_value(atoi(postfix))
  {
  }
  unsigned int number() const
  {
    return m_value;
  }
  void write(char* buffer)
  {
    sprintf(buffer, "%u", m_value);
  }
  Postfix& operator++()
  { 
    ++m_value;
    return *this;
  }
  bool operator<(const Postfix& other) const
  {
    return m_value < other.m_value;
  }
  bool operator==(const Postfix& other) const
  {
    return m_value == other.m_value;
  }
  bool operator!=(const Postfix& other) const
  {
    return !operator==(other);
  }
};

#else
class Postfix
{
  std::pair<unsigned int, unsigned int> m_value;
public:
  Postfix(unsigned int number, unsigned int leading_zeros)
    : m_value(leading_zeros, number)
  {
  }
  Postfix(const char* postfix)
    : m_value(number_count_leading_zeros(postfix), atoi(postfix))
  {
  }
  unsigned int number() const
  {
    return m_value.second;
  }
  unsigned int leading_zeros() const
  {
    return m_value.first;
  }
  void write(char* buffer)
  {
    for(unsigned int count = 0; count < m_value.first; ++count, ++buffer)
      *buffer = '0';
    sprintf(buffer, "%u", m_value.second);
  }
  Postfix& operator++()
  { 
    ++m_value.second;
    if(m_value.first != 0 && m_value.second % 10 == 0)
      --m_value.first;
    return *this;
  }
  bool operator<(const Postfix& other) const
  {
    return m_value < other.m_value;
  }
  bool operator==(const Postfix& other) const
  {
    return m_value == other.m_value;
  }
  bool operator!=(const Postfix& other) const
  {
    return !operator==(other);
  }
};

#endif

typedef std::pair<CopiedString, Postfix> name_t;

inline void name_write(char* buffer, name_t name)
{
  strcpy(buffer, name.first.c_str());
  name.second.write(buffer + strlen(name.first.c_str()));
}

inline name_t name_read(const char* name)
{
  const char* end = name + strlen(name);
  for(const char* p = end; end != name; --p)
  {
    if(strrchr("1234567890", *p) == NULL)
      break;
    end = p;
  }

  return name_t(CopiedString(StringRange(name, end)), Postfix(end));
}


class PostFixes
{
  typedef std::map<Postfix, unsigned int> postfixes_t;
  postfixes_t m_postfixes;

  Postfix find_first_empty() const
  {
    Postfix postfix("1");
    for(postfixes_t::const_iterator i = m_postfixes.find(postfix); i != m_postfixes.end(); ++i, ++postfix)
    {
      if((*i).first != postfix)
      {
        break;
      }
    }
    return postfix;
  }

public:
  Postfix make_unique(Postfix postfix) const
  {
    postfixes_t::const_iterator i = m_postfixes.find(postfix);
    if(i == m_postfixes.end())
    {
      return postfix;
    }
    else
    {
      return find_first_empty();
    }
  }

  void insert(Postfix postfix)
  {
    postfixes_t::iterator i = m_postfixes.find(postfix);
    if(i == m_postfixes.end())
    {
      m_postfixes.insert(postfixes_t::value_type(postfix, 1));
    }
    else
    {
      ++(*i).second;
    }
  }

  void erase(Postfix postfix)
  {
    postfixes_t::iterator i = m_postfixes.find(postfix);
    if(i == m_postfixes.end())
    {
      // error
    }
    else
    {
      if(--(*i).second == 0)
        m_postfixes.erase(i);
    }
  }

  bool empty() const
  {
    return m_postfixes.empty();
  }
};


class UniqueNames
{
  typedef std::map<CopiedString, PostFixes> names_t;
  names_t m_names;
public:
  name_t make_unique(const name_t& name) const
  {
    names_t::const_iterator i = m_names.find(name.first);
    if(i == m_names.end())
    {
      return name;
    }
    else
    {
      return name_t(name.first, (*i).second.make_unique(name.second));
    }
  }

  void insert(const name_t& name)
  {
    m_names[name.first].insert(name.second);
  }

  void erase(const name_t& name)
  {
    names_t::iterator i = m_names.find(name.first);
    if(i == m_names.end())
    {
      ASSERT_MESSAGE(true, "erase: name not found");
    }
    else
    {
      (*i).second.erase(name.second);
      if((*i).second.empty())
        m_names.erase(i);
    }
  }

  bool empty() const
  {
    return m_names.empty();
  }
};



#if 0

#undef ERROR_MESSAGE
#define ERROR_MESSAGE(message)

class TestUniqueName
{
  void name_check_equal(const name_t& name, const char* string, unsigned int postfix)
  {
    ASSERT_MESSAGE(strcmp(name.first.c_str(), string) == 0
      && name.second.number() == postfix,
      "test failed!");
  }
  void test_refcount()
  {
    Names names;

    names.insert(name_t("func_bleh_", "100"));
    names.insert(name_t("func_bleh_", "100"));
    names.insert(name_t("func_bleh_", "100"));


    names.erase(name_t("func_bleh_", "100"));
    names.erase(name_t("func_bleh_", "100"));
    names.erase(name_t("func_bleh_", "100"));

    ASSERT_MESSAGE(names.empty(), "test failed!");
  }

  void test_make_unique()
  {
    Names names;

    {
      name_t name(names.make_unique(name_t("func_bleh_", "01")));
      name_check_equal(name, "func_bleh_", 1);
      names.insert(name);
    }
    {
      name_t name(names.make_unique(name_t("func_bleh_", "04")));
      name_check_equal(name, "func_bleh_", 4);
      names.insert(name);
    }
    {
      name_t name(names.make_unique(name_t("func_bleh_", "04")));
      name_check_equal(name, "func_bleh_", 2);
      names.insert(name);
    }
    {
      name_t name(names.make_unique(name_t("func_bleh_", "1")));
      name_check_equal(name, "func_bleh_", 3);
      names.insert(name);
    }
    {
      name_t name(names.make_unique(name_t("func_bleh_", "2")));
      name_check_equal(name, "func_bleh_", 5);
      names.insert(name);
    }
    {
      name_t name(names.make_unique(name_t("func_bleh_", "3")));
      name_check_equal(name, "func_bleh_", 6);
      names.insert(name);
    }

    names.erase(name_t("func_bleh_", "1"));
    names.erase(name_t("func_bleh_", "2"));
    names.erase(name_t("func_bleh_", "3"));
    names.erase(name_t("func_bleh_", "4"));
    names.erase(name_t("func_bleh_", "5"));
    names.erase(name_t("func_bleh_", "6"));

    ASSERT_MESSAGE(names.empty(), "test failed!");
  }
public:
  TestUniqueName()
  {
    test_refcount();
    test_make_unique();
  }
};

const TestUniqueName g_testuniquename;

#endif


#endif
