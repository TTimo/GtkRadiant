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

#if !defined(INCLUDED_WINDOWOBSERVER_H)
#define INCLUDED_WINDOWOBSERVER_H

template<typename Enumeration> class BitFieldValue;
struct ModifierEnumeration;
typedef BitFieldValue<ModifierEnumeration> ModifierFlags;

template<typename Enumeration> class EnumeratedValue;
struct ButtonEnumeration;
typedef EnumeratedValue<ButtonEnumeration> ButtonIdentifier;


#include "generic/bitfield.h"

struct ModifierEnumeration
{
  enum Value
  {
    SHIFT = 0,
    CONTROL = 1,
    ALT = 2
  };
};

typedef BitFieldValue<ModifierEnumeration> ModifierFlags;

const ModifierFlags c_modifierNone;
const ModifierFlags c_modifierShift(ModifierEnumeration::SHIFT);
const ModifierFlags c_modifierControl(ModifierEnumeration::CONTROL);
const ModifierFlags c_modifierAlt(ModifierEnumeration::ALT);

#include "generic/enumeration.h"

struct ButtonEnumeration
{
  enum Value
  {
    INVALID = 0,
    LEFT = 1,
    MIDDLE = 3,
    RIGHT = 2
  };
};

typedef EnumeratedValue<ButtonEnumeration> ButtonIdentifier;

const ButtonIdentifier c_buttonInvalid(ButtonEnumeration::INVALID);
const ButtonIdentifier c_buttonLeft(ButtonEnumeration::LEFT);
const ButtonIdentifier c_buttonMiddle(ButtonEnumeration::MIDDLE);
const ButtonIdentifier c_buttonRight(ButtonEnumeration::RIGHT);


template<typename Element>
class BasicVector2;
typedef BasicVector2<float> Vector2;
typedef Vector2 WindowVector;

class WindowObserver
{
public:
  virtual void release() = 0;
  virtual void onSizeChanged(int width, int height) = 0;
  virtual void onMouseDown(const WindowVector& position, ButtonIdentifier button, ModifierFlags modifiers) = 0;
  virtual void onMouseUp(const WindowVector& position, ButtonIdentifier button, ModifierFlags modifiers) = 0;
  virtual void onMouseMotion(const WindowVector& position, ModifierFlags modifiers) = 0;
  virtual void onModifierDown(ModifierFlags modifier) = 0;
  virtual void onModifierUp(ModifierFlags modifier) = 0;
};

#endif
