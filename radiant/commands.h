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

#if !defined(INCLUDED_COMMANDS_H)
#define INCLUDED_COMMANDS_H

#include "gtkutil/accelerator.h"


const Accelerator& GlobalShortcuts_insert(const char* name, const Accelerator& accelerator);
void GlobalShortcuts_register(const char* name);
void GlobalShortcuts_reportUnregistered();

class CommandVisitor
{
public:
  virtual void visit(const char* name, Accelerator& accelerator) = 0;
};

void GlobalCommands_insert(const char* name, const Callback& callback, const Accelerator& accelerator = accelerator_null());
const Command& GlobalCommands_find(const char* name);

void GlobalToggles_insert(const char* name, const Callback& callback, const BoolExportCallback& exportCallback, const Accelerator& accelerator = accelerator_null());
const Toggle& GlobalToggles_find(const char* name);

void GlobalKeyEvents_insert(const char* name, const Accelerator& accelerator, const Callback& keyDown, const Callback& keyUp);
const KeyEvent& GlobalKeyEvents_find(const char* name);


void DoCommandListDlg();

void LoadCommandMap(const char* path);
void SaveCommandMap(const char* path);


#endif
