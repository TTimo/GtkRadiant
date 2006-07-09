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

#ifdef __linux__
#include <execinfo.h>

void write_stack_trace(TextOutputStream& outputStream)
{
  const unsigned int MAX_SYMBOLS = 256;
  void* symbols[MAX_SYMBOLS];
    
  // get return addresses
  int symbol_count = backtrace(symbols, MAX_SYMBOLS);
  
  if(!symbol_count)
	return;
  
  // resolve and print names
  char** symbol_names = backtrace_symbols(symbols, symbol_count);	
  if(symbol_names)
  {
     for(int i = 0; (i < symbol_count); ++i)
        outputStream << symbol_names[i] << "\n";
        
     // not a memleak, see www.gnu.org/software/libc/manual (Debugging Support, Backtraces)
     free(symbol_names);
  }
}	
#endif

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
  STACKFRAME64& sf;
  TextOutputStream& outputStream;
  std::size_t count;
  EnumerateSymbolsContext(STACKFRAME64& sf, TextOutputStream& outputStream) : sf(sf), outputStream(outputStream), count(0)
  {
  }
};

void write_symbol(PSYMBOL_INFO pSym, STACKFRAME64& sf, TextOutputStream& outputStream, std::size_t& count)
{
#if 0
 if ( pSym->Flags & SYMFLAG_PARAMETER )
 {

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
    if(count != 0)
    {
      outputStream << ", ";
    }
    outputStream << pSym->Name;
    ++count;
  }
#endif
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

  STACKFRAME64 sf;
  memset( &sf, 0, sizeof(sf) );
  sf.AddrPC.Mode         = AddrModeFlat;
  sf.AddrStack.Mode      = AddrModeFlat;
  sf.AddrFrame.Mode      = AddrModeFlat;
  
#ifdef _M_IX86
  // Initialize the STACKFRAME structure for the first call.  This is only
  // necessary for Intel CPUs, and isn't mentioned in the documentation.
  sf.AddrPC.Offset       = context.Eip;
  sf.AddrStack.Offset    = context.Esp;
  sf.AddrFrame.Offset    = context.Ebp;
  
  dwMachineType = IMAGE_FILE_MACHINE_I386;
#elif _M_X64
  sf.AddrPC.Offset       = context.Rip;
  sf.AddrStack.Offset    = context.Rsp;
  
  // MSDN: x64:  The frame pointer is RBP or RDI. This value is not always used.
  // very funny, we'll try Rdi for now
  sf.AddrFrame.Offset    = context.Rdi;
  
  dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#endif

  const unsigned int max_sym_name = 1024;// should be enough

  while ( 1 )
  {
    // Get the next stack frame
    if ( ! StackWalk64( dwMachineType,
                        m_hProcess,
                        GetCurrentThread(),
                        &sf,
                        &context,
                        0,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        0 ) )
        break;

    if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
      break;                      // the frame is OK.  Bail if not.

    // Get the name of the function for this stack frame entry
    BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + max_sym_name ];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = max_sym_name;
                    
    DWORD64 symDisplacement = 0;    // Displacement of the input address,
                                    // relative to the start of the symbol

    IMAGEHLP_MODULE64 module = { sizeof(IMAGEHLP_MODULE64) };
    if(SymGetModuleInfo64(m_hProcess, sf.AddrPC.Offset, &module))
    {
      outputStream << module.ModuleName << "!";

      if ( SymFromAddr(m_hProcess, sf.AddrPC.Offset, &symDisplacement, pSymbol))
      {
        char undecoratedName[max_sym_name];
        UnDecorateSymbolName(pSymbol->Name, undecoratedName, max_sym_name, UNDNAME_COMPLETE);

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
        IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
        DWORD dwLineDisplacement;
        if ( SymGetLineFromAddr64( m_hProcess, sf.AddrPC.Offset,
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
