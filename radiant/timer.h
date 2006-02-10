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

#if !defined (INCLUDED_TIMER_H)
#define INCLUDED_TIMER_H

#if 1

const int msec_per_sec = 1000;

class MillisecondTime
{
  unsigned int m_milliseconds;
public:
  MillisecondTime(unsigned int milliseconds)
    : m_milliseconds(milliseconds)
  {
  }
  MillisecondTime()
  {
  }
  static MillisecondTime current();

  unsigned int milliseconds_since(const MillisecondTime& other) const
  {
    return m_milliseconds - other.m_milliseconds;
  }
};

template<typename tick_type>
inline MillisecondTime time_from_ticks(tick_type tick_count, tick_type ticks_per_sec)
{
  return MillisecondTime(static_cast<unsigned int>(tick_count / static_cast<double>(ticks_per_sec / msec_per_sec)));
}

#else

const unsigned int usec_per_sec = 1000000;

class MillisecondTime
{
  unsigned int m_sec;
  unsigned int m_usec;
public:
  MillisecondTime(unsigned int sec, unsigned int usec)
    : m_sec(sec), m_usec(usec)
  {
  }
  MillisecondTime()
  {
  }
  staticMillisecondTime current();

  unsigned int milliseconds_since(const MillisecondTime& other) const
  {
    return static_cast<unsigned int>((m_sec * static_cast<double>(usec_per_sec) + m_usec)
      - (other.m_sec * static_cast<double>(usec_per_sec) + other.m_usec)) / 1000;
  }
};

template<typename tick_type>
inline MillisecondTime time_from_ticks(tick_type tick_count, tick_type ticks_per_sec)
{
  return MillisecondTime(static_cast<unsigned int>(tick_count / ticks_per_sec),
    static_cast<unsigned int>((tick_count % ticks_per_sec) * (usec_per_sec / static_cast<double>(ticks_per_sec))));
}

#endif

class Timer
{
  MillisecondTime m_start;

public:
  void start()
  {
    m_start = MillisecondTime::current();
  }
  unsigned int elapsed_msec()
  {
    return MillisecondTime::current().milliseconds_since(m_start);
  }
};

#endif
