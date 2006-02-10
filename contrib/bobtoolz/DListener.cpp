/*
BobToolz plugin for GtkRadiant
Copyright (C) 2001 Gordon Biggans

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// DListener.cpp: implementation of the DListener class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DListener.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DListener::DListener()
{
	refCount = 1;
	m_bHooked = FALSE;
}

DListener::~DListener()
{
	UnRegister();
}

void DListener::Register()
{
	g_MessageTable.m_pfnHookWindow( this );
	m_bHooked = TRUE;
}

void DListener::UnRegister()
{
	if(m_bHooked)
	{
		g_MessageTable.m_pfnUnHookWindow( this );
		m_bHooked = FALSE;
	}
}

bool DListener::OnMouseMove(unsigned int nFlags, double x, double y)
{
	if(!parent->UpdatePath())
		delete parent;

	return FALSE;
}

bool DListener::OnLButtonDown(unsigned int nFlags, double x, double y)
{
	return FALSE;
}

bool DListener::OnLButtonUp(unsigned int nFlags, double x, double y)
{
	return FALSE;
}

bool DListener::OnRButtonDown(unsigned int nFlags, double x, double y)
{
	return FALSE;
}

bool DListener::OnRButtonUp(unsigned int nFlags, double x, double y)
{
	return FALSE;
}

bool DListener::OnMButtonDown(unsigned int nFlags, double x, double y)
{
	return FALSE;
}

bool DListener::OnMButtonUp(unsigned int nFlags, double x, double y)
{
	return FALSE;
}
