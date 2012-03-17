/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#ifndef __QSYSPRINTF_H__
#define __QSYSPRINTF_H__

/*!
   this header is provided in libs/ in an attempt to provide a common API
   for all the diagnostic printing / fatal error situations

   this is oriented at synapse server targets ONLY
   synapse clients should not include this, as they are supposed to go
   through the function tables to report print diagnostics
   (or use Syn_Printf for situations where the func table may not be available)

   each server target implements that in it's own way. Radiant logs to
   a file and sends to the console, q3map prints to stdout and to the
   XML network stream, etc.
 */

#if defined( __cplusplus )
extern "C"
{
#endif

// NOTE: might want to switch to bits if needed
#define SYS_VRB 0 ///< verbose support (on/off)
#define SYS_STD 1 ///< standard print level - this is the default
#define SYS_WRN 2 ///< warnings
#define SYS_ERR 3 ///< error
#define SYS_NOCON 4 ///< no console, only print to the file (useful whenever Sys_Printf and output IS the problem)

/*!
   those are the real implementation
 */
void Sys_Printf_VA( const char *text, va_list args ); ///< matches PFN_SYN_PRINTF_VA prototype
void Sys_FPrintf_VA( int level, const char *text, va_list args );

/*!
   this is easy to call, wrappers around va_list version
 */
void Sys_Printf( const char *text, ... );
void Sys_FPrintf( int flag, const char *text, ... );

#if defined( __cplusplus )
};
#endif

#endif
