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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// interface for BSP frontends plugins
//

// DONE:	- change BSP menu to Q3Build menu ?
// DONE:    - detect when Q3Build dies ?
// DELAYED: - hotkeys !
// SUCCESS: - try again getting feedback from Q3Build

#ifndef __IBSPFRONTEND_H_
#define __IBSPFRONTEND_H_

// define a GUID for this interface so plugins can access and reference it
// {8ED6A480-BA5E-11d3-A3E3-0004AC96D4C3}
static const GUID QERPlugBSPFrontendTable_GUID =
{ 0x8ed6a480, 0xba5e, 0x11d3, { 0xa3, 0xe3, 0x0, 0x4, 0xac, 0x96, 0xd4, 0xc3 } };

// ask the plugin about the items to show up in the BSP menu
typedef char * ( WINAPI * PFN_GETBSPMENU )();
// dispatch a BSP menu command
typedef void ( WINAPI * PFN_DISPATCHBSPCOMMAND )( char * );
// this one gets called after a monitoring loop ends
// 0: all good
// 1: timed out / Radiant didn't get the connection
// 2: got a connection, compilation ended with an error
typedef void ( WINAPI * PFN_ENDLISTEN )( int status );

struct _QERPlugBSPFrontendTable
{
	int m_nSize;
	PFN_GETBSPMENU m_pfnGetBSPMenu;
	PFN_DISPATCHBSPCOMMAND m_pfnDispatchBSPCommand;
	PFN_ENDLISTEN m_pfnEndListen;
};

// interface provided by Radiant to the plugin
// {A2CCF366-BA60-11d3-A3E3-0004AC96D4C3}
static const GUID QERAppBSPFrontendTable_GUID =
{ 0xa2ccf366, 0xba60, 0x11d3, { 0xa3, 0xe3, 0x0, 0x4, 0xac, 0x96, 0xd4, 0xc3 } };

typedef char * ( WINAPI * PFN_GETMAPNAME )();
typedef void ( WINAPI * PFN_LISTEN )();
typedef void ( WINAPI * PFN_SLEEP )();

struct _QERAppBSPFrontendTable
{
	int m_nSize;
	PFN_GETMAPNAME m_pfnGetMapName;
	PFN_LISTEN m_pfnListen;
	PFN_SLEEP m_pfnSleep;
	//++timo TODO: needs a hook to reset the debug window (in regular mode it's done at startup of the BSP operation)
};

#endif
