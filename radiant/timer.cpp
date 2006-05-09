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

#include "timer.h"


#if defined(WIN32)

#include <windows.h>

MillisecondTime MillisecondTime::current()
{
  static class Cached
  {
    LONGLONG m_frequency;
    LONGLONG m_base;
  public:
    Cached()
    {
      QueryPerformanceFrequency((LARGE_INTEGER *) &m_frequency);
      QueryPerformanceCounter((LARGE_INTEGER *) &m_base);
    }
    LONGLONG frequency()
    {
      return m_frequency;
    }
    LONGLONG base()
    {
      return m_base;
    }
  } cached;
    
  if(cached.frequency() > 0)
  {
    LONGLONG count;
    QueryPerformanceCounter((LARGE_INTEGER *) &count);
    return time_from_ticks(count - cached.base(), cached.frequency());
  }
  else
  {
#if 1
    return MillisecondTime();
#else
    return time_from_ticks(timeGetTime(), 1000);
#endif
  }
}




#elif defined(POSIX)

#include <time.h>
#include "sys/time.h"

MillisecondTime MillisecondTime::current()
{
  static class Cached
  {
    time_t m_base;
  public:
    Cached()
    {
      time(&m_base);
    }
    time_t base()
    {
      return m_base;
    }
  } cached;

  timeval time;
  gettimeofday(&time, 0);
  return MillisecondTime((time.tv_sec - cached.base()) * 1000 + time.tv_usec / 1000);
}



#else

#include <ctime>

MillisecondTime MillisecondTime::current()
{
  return time_from_ticks<std::clock_t>(std::clock(), CLOCKS_PER_SEC);
}



#endif
