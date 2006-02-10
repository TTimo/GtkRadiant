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

#include "stacktrace.h"
#include "stream/textstream.h"

#include "environment.h"

#if defined (WIN32) && defined (_MSC_VER)

#include "windows.h"
#include "winnt.h"
#include "dbghelp.h"

class Address
{
public:
  void* m_value;
  Address(void* value) : m_value(value)
  {
  }
};

/// \brief Writes an address \p p to \p ostream in hexadecimal form.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const Address& p)
{
  const std::size_t bufferSize = (sizeof(void*) * 2) + 1;
  char buf[bufferSize];
  ostream.write(buf, snprintf(buf, bufferSize, "%0p", p.m_value));
  return ostream;
}

class Offset
{
public:
  void* m_value;
  Offset(void* value) : m_value(value)
  {
  }
};

/// \brief Writes an address \p p to \p ostream in hexadecimal form.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const Offset& p)
{
  const std::size_t bufferSize = (sizeof(void*) * 2) + 1;
  char buf[bufferSize];
  ostream.write(buf, snprintf(buf, bufferSize, "%X", p.m_value));
  return ostream;
}

/// \brief Writes a WCHAR string \p s to \p ostream.
template<typename TextOutputStreamType>
inline TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const WCHAR* s)
{
  const std::size_t bufferSize = 1024;
  char buf[bufferSize];
  ostream.write(buf, snprintf(buf, bufferSize, "%ls", s));
  return ostream;
}

struct EnumerateSymbolsContext
{
  STACKFRAME& sf;
  TextOutputStream& outputStream;
  std::size_t count;
  EnumerateSymbolsContext(STACKFRAME& sf, TextOutputStream& outputStream) : sf(sf), outputStream(outputStream), count(0)
  {
  }
};

void write_symbol(PSYMBOL_INFO pSym, STACKFRAME& sf, TextOutputStream& outputStream, std::size_t& count)
{
  if ( pSym->Flags & SYMFLAG_PARAMETER )
  {
#if 0
    DWORD basicType;
    if ( SymGetTypeInfo( GetCurrentProcess(), pSym->ModBase, pSym->TypeIndex,
                        TI_GET_BASETYPE, &basicType ) )
    {
      int bleh = 0;
    }
    else
    {
      DWORD typeId;
      if(SymGetTypeInfo( GetCurrentProcess(), pSym->ModBase, pSym->TypeIndex,
                      TI_GET_TYPEID, &typeId ))
      {
        if ( SymGetTypeInfo( GetCurrentProcess(), pSym->ModBase, pSym->TypeIndex,
                            TI_GET_BASETYPE, &basicType ) )
        {
          int bleh = 0;
        }
        else
        {
          const char* FormatGetLastError();
          const char* error = FormatGetLastError();
          int bleh = 0;

          WCHAR* name;
          if(SymGetTypeInfo( GetCurrentProcess(), pSym->ModBase, typeId,
                          TI_GET_SYMNAME, &name ))
          {
            outputStream << name << " ";
            LocalFree(name);
            int bleh = 0;
          }
          else
          {
            const char* FormatGetLastError();
            const char* error = FormatGetLastError();
            int bleh = 0;
          }
        }
      }
      else
      {
        const char* FormatGetLastError();
        const char* error = FormatGetLastError();
        int bleh = 0;
      }
    }
#endif
    if(count != 0)
    {
      outputStream << ", ";
    }
    outputStream << pSym->Name;
    ++count;
  }
}

BOOL CALLBACK
EnumerateSymbolsCallback(
    PSYMBOL_INFO  pSymInfo,
    ULONG         SymbolSize,
    PVOID         UserContext )
{
  write_symbol( pSymInfo, ((EnumerateSymbolsContext*)UserContext)->sf, ((EnumerateSymbolsContext*)UserContext)->outputStream, ((EnumerateSymbolsContext*)UserContext)->count);


  return TRUE;
}

void write_stack_trace(PCONTEXT pContext, TextOutputStream& outputStream)
{
  HANDLE m_hProcess = GetCurrentProcess();
  DWORD dwMachineType = 0;

  CONTEXT context = *pContext;

  // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag
  if ( !SymInitialize( m_hProcess, (PSTR)environment_get_app_path(), TRUE ) )
  {
    return;
  }

  STACKFRAME sf;
  memset( &sf, 0, sizeof(sf) );

#ifdef _M_IX86
  // Initialize the STACKFRAME structure for the first call.  This is only
  // necessary for Intel CPUs, and isn't mentioned in the documentation.
  sf.AddrPC.Offset       = context.Eip;
  sf.AddrPC.Mode         = AddrModeFlat;
  sf.AddrStack.Offset    = context.Esp;
  sf.AddrStack.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset    = context.Ebp;
  sf.AddrFrame.Mode      = AddrModeFlat;

  dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif

  while ( 1 )
  {
    // Get the next stack frame
    if ( ! StackWalk(  dwMachineType,
                        m_hProcess,
                        GetCurrentThread(),
                        &sf,
                        &context,
                        0,
                        SymFunctionTableAccess,
                        SymGetModuleBase,
                        0 ) )
        break;

    if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
      break;                      // the frame is OK.  Bail if not.

    // Get the name of the function for this stack frame entry
    BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + MAX_SYM_NAME ];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
                    
    DWORD64 symDisplacement = 0;    // Displacement of the input address,
                                    // relative to the start of the symbol

    IMAGEHLP_MODULE module = { sizeof(IMAGEHLP_MODULE) };
    if(SymGetModuleInfo(m_hProcess, sf.AddrPC.Offset, &module))
    {
      outputStream << module.ModuleName << "!";

      if ( SymFromAddr(m_hProcess, sf.AddrPC.Offset, &symDisplacement, pSymbol))
      {
        char undecoratedName[MAX_SYM_NAME];
        UnDecorateSymbolName(pSymbol->Name, undecoratedName, MAX_SYM_NAME, UNDNAME_COMPLETE);

        outputStream << undecoratedName;

        outputStream << "(";
        // Use SymSetContext to get just the locals/params for this frame
        IMAGEHLP_STACK_FRAME imagehlpStackFrame;
        imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
        SymSetContext( m_hProcess, &imagehlpStackFrame, 0 );

        // Enumerate the locals/parameters
        EnumerateSymbolsContext context(sf, outputStream);
        SymEnumSymbols( m_hProcess, 0, 0, EnumerateSymbolsCallback, &context );
        outputStream << ")";

        outputStream << " + " << Offset(reinterpret_cast<void*>(symDisplacement));

        // Get the source line for this stack frame entry
        IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
        DWORD dwLineDisplacement;
        if ( SymGetLineFromAddr( m_hProcess, sf.AddrPC.Offset,
                                &dwLineDisplacement, &lineInfo ) )
        {
          outputStream << " " << lineInfo.FileName << " line " << Unsigned(lineInfo.LineNumber); 
        }
      }
      else
      {
        outputStream << Address(reinterpret_cast<void*>(sf.AddrPC.Offset));
      }
    }

    outputStream << "\n";
  }

  SymCleanup(m_hProcess);

  return;
}

void write_stack_trace(TextOutputStream& outputStream)
{
  __try{ RaiseException(0,0,0,0); } __except(write_stack_trace((GetExceptionInformation())->ContextRecord, outputStream), EXCEPTION_CONTINUE_EXECUTION) {}
}

#endif
