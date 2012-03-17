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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <stdlib.h>
#include <stdio.h>

#include "synapse.h"
#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#define USE_ECLASSMANAGER_DEFINE
#include "ieclass.h"
#define USE_SCRIPLIBTABLE_DEFINE
#include "iscriplib.h"
#define USE_VFSTABLE_DEFINE
#include "ifilesystem.h"

class CSynapseClientFGD : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientFGD() { }
virtual ~CSynapseClientFGD() { }
};

#endif // _PLUGIN_H_
