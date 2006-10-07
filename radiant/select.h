/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

#if !defined(INCLUDED_SELECT_H)
#define INCLUDED_SELECT_H

#include "math/vector.h"

void Select_GetBounds(Vector3& mins, Vector3& maxs);
void Select_GetMid(Vector3& mid);

void Select_Delete();
void Select_Invert();
void Select_Inside();
void Select_Touching();
void Scene_ExpandSelectionToEntities();

void Selection_Flipx();
void Selection_Flipy();
void Selection_Flipz();
void Selection_Rotatex();
void Selection_Rotatey();
void Selection_Rotatez();


void Selection_MoveDown();
void Selection_MoveUp();

void Select_AllOfType();

void DoRotateDlg();
void DoScaleDlg();


void Select_SetShader(const char* shader);

class TextureProjection;
void Select_SetTexdef(const TextureProjection& projection);

class ContentsFlagsValue;
void Select_SetFlags(const ContentsFlagsValue& flags);

void Select_RotateTexture(float amt);
void Select_ScaleTexture(float x, float y);
void Select_ShiftTexture(float x, float y);
void Select_FitTexture(float horizontal = 1, float vertical = 1);
void FindReplaceTextures(const char* pFind, const char* pReplace, bool bSelected);

void HideSelected();
void Select_ShowAllHidden();

// updating workzone to a given brush (depends on current view)

void Selection_construct();
void Selection_destroy();


struct select_workzone_t
{
  // defines the boundaries of the current work area
  // is used to guess brushes and drop points third coordinate when creating from 2D view
  Vector3 d_work_min, d_work_max;

  select_workzone_t() :
    d_work_min(-64.0f,-64.0f,-64.0f),
    d_work_max( 64.0f, 64.0f, 64.0f)
  {
  }
};

const select_workzone_t& Select_getWorkZone();

#endif
