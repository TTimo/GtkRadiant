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

#if !defined(INCLUDED_ISELECTION_H)
#define INCLUDED_ISELECTION_H

#include <cstddef>
#include "generic/constant.h"

class Renderer;
class View;

class Callback;

template<typename FirstArgument>
class Callback1;

class Selectable
{
public:
  STRING_CONSTANT(Name, "Selectable");

  virtual void setSelected(bool select) = 0;
  virtual bool isSelected() const = 0;
};

namespace scene
{
  class Instance;
};

class InstanceSelectionObserver
{
public:
  virtual void onSelectedChanged(scene::Instance& instance) = 0;
};

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
template<typename Element> class BasicVector4;
typedef BasicVector4<float> Vector4;
class Matrix4;
typedef Vector4 Quaternion;

typedef Callback1<const Selectable&> SelectionChangeCallback;

class SelectionSystem
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "selection");

  enum EMode
  {
    eEntity,
    ePrimitive,
    eComponent,
  };

  enum EComponentMode
  {
    eDefault,
    eVertex,
    eEdge,
    eFace,
  };

  enum EManipulatorMode
  {
    eTranslate,
    eRotate,
    eScale,
    eDrag,
    eClip,
  };

  virtual void SetMode(EMode mode) = 0;
  virtual EMode Mode() const = 0;
  virtual void SetComponentMode(EComponentMode mode) = 0;
  virtual EComponentMode ComponentMode() const = 0;
  virtual void SetManipulatorMode(EManipulatorMode mode) = 0;
  virtual EManipulatorMode ManipulatorMode() const = 0;

  virtual SelectionChangeCallback getObserver(EMode mode) = 0;
  virtual std::size_t countSelected() const = 0;
  virtual std::size_t countSelectedComponents() const = 0;
  virtual void onSelectedChanged(scene::Instance& instance, const Selectable& selectable) = 0;
  virtual void onComponentSelection(scene::Instance& instance, const Selectable& selectable) = 0;
  virtual scene::Instance& ultimateSelected() const = 0;
  virtual scene::Instance& penultimateSelected() const = 0;
  virtual void setSelectedAll(bool selected) = 0;
  virtual void setSelectedAllComponents(bool selected) = 0;

  class Visitor
  {
  public:
    virtual void visit(scene::Instance& instance) const = 0;
  };
  virtual void foreachSelected(const Visitor& visitor) const = 0;
  virtual void foreachSelectedComponent(const Visitor& visitor) const = 0;

  virtual void addSelectionChangeCallback(const SelectionChangeCallback& callback) = 0;

  virtual void NudgeManipulator(const Vector3& nudge, const Vector3& view) = 0;

  virtual void translateSelected(const Vector3& translation) = 0;
  virtual void rotateSelected(const Quaternion& rotation) = 0;
  virtual void scaleSelected(const Vector3& scaling) = 0;

  virtual void pivotChanged() const = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<SelectionSystem> GlobalSelectionModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<SelectionSystem> GlobalSelectionModuleRef;

inline SelectionSystem& GlobalSelectionSystem()
{
  return GlobalSelectionModule::getTable();
}


#endif
