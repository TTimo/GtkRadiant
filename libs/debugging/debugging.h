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

#if !defined(INCLUDED_DEBUGGING_DEBUGGING_H)
#define INCLUDED_DEBUGGING_DEBUGGING_H

/// \file
/// \brief Debugging macros for fatal error/assert messages.

#include "stream/textstream.h"
#include "warnings.h"
#include "generic/static.h"

#if defined(_MSC_VER) && defined(_M_IX86)
#define DEBUGGER_BREAKPOINT() __asm { int 3 }
#elif defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#define DEBUGGER_BREAKPOINT() __asm__ __volatile__ ("int $03")
#else
#include <signal.h>

#define DEBUGGER_BREAKPOINT() raise(SIGTRAP);
#endif


#define FILE_LINE __FILE__ ":" << __LINE__

#if defined(_DEBUG) || 1
#define DEBUG_ASSERTS
#endif

class DebugMessageHandler
{
public:
  virtual TextOutputStream& getOutputStream() = 0;
  virtual bool handleMessage() = 0;
};

class NullDebugMessageHandler : public NullOutputStream, public DebugMessageHandler
{
public:
  virtual TextOutputStream& getOutputStream()
  {
    return *this;
  }
  virtual bool handleMessage()
  {
    return false;
  }
};

class DefaultDebugMessageHandler : public DebugMessageHandler
{
public:
  virtual TextOutputStream& getOutputStream()
  {
    return globalErrorStream();
  }
  virtual bool handleMessage()
  {
#if defined(_DEBUG)
    return false; // send debug-break
#else
    return true;
#endif
  }
};

class DebugMessageHandlerRef : public DefaultDebugMessageHandler
{
  DebugMessageHandler* m_handler;
public:
  DebugMessageHandlerRef()
    : m_handler(this)
  {
  }
  void setHandler(DebugMessageHandler& handler)
  {
    m_handler = &handler;
  }
  DebugMessageHandler& getHandler()
  {
    return *m_handler;
  }
};

typedef Static<DebugMessageHandlerRef> GlobalDebugMessageHandler;

inline DebugMessageHandler& globalDebugMessageHandler()
{
  return GlobalDebugMessageHandler::instance().getHandler();
}

#if defined(DEBUG_ASSERTS)

/// \brief Sends a \p message to the current debug-message-handler text-output-stream if \p condition evaluates to false.
#define ASSERT_MESSAGE(condition, message)\
if(!(condition))\
{\
  globalDebugMessageHandler().getOutputStream() << FILE_LINE << "\nassertion failure: " << message << "\n";\
  if(!globalDebugMessageHandler().handleMessage()) { DEBUGGER_BREAKPOINT(); }\
} else\

/// \brief Sends a \p message to the current debug-message-handler text-output-stream.
#define ERROR_MESSAGE(message)\
globalDebugMessageHandler().getOutputStream() << FILE_LINE << "\nruntime error: " << message << "\n";\
if(!globalDebugMessageHandler().handleMessage()) { DEBUGGER_BREAKPOINT(); } else\

#define ASSERT_NOTNULL(ptr) ASSERT_MESSAGE(ptr != 0, "pointer \"" #ptr "\" is null")

#else

#define ASSERT_MESSAGE(condition, message)
#define ERROR_MESSAGE(message)
#define ASSERT_NOTNULL(ptr)

#endif

#endif
