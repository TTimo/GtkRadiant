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

#if !defined(INCLUDED_MATH_VECTOR_H)
#define INCLUDED_MATH_VECTOR_H

/// \file
/// \brief Vector data types and related operations.

#if 0

#define	lrint(dbl)		((int)((dbl) + 0.5))
#define	lrintf(flt)		((int)((flt) + 0.5))

#endif

#if defined (_MSC_VER)

inline int lrint (double flt)
{
  int i;

	_asm
	{
    fld flt
		fistp i
  };
			
	return i;
} 

#else // lrint is part of ISO C99

#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1

#define	__USE_ISOC9X	1
#define	__USE_ISOC99	1

#endif

#include <cmath>
#include <cstddef>
#include <float.h>
#include <algorithm>

//#include "debugging/debugging.h"

/// \brief Returns true if \p self is equal to other \p other within \p epsilon.
template<typename Element, typename OtherElement>
inline bool float_equal_epsilon(const Element& self, const OtherElement& other, const Element& epsilon)
{
  return fabs(other - self) < epsilon;
}

/// \brief Returns the value midway between \p self and \p other.
template<typename Element>
inline Element float_mid(const Element& self, const Element& other)
{
  return Element((self + other) * 0.5);
}

/// \brief Returns \p f rounded to the nearest integer. Note that this is not the same behaviour as casting from float to int.
template<typename Element>
inline int float_to_integer(const Element& f)
{
  return lrint(f);
}

/// \brief Returns \p f rounded to the nearest multiple of \p snap.
template<typename Element, typename OtherElement>
inline Element float_snapped(const Element& f, const OtherElement& snap)
{
  return Element(float_to_integer(f / snap) * snap);
}

/// \brief Returns true if \p f has no decimal fraction part.
template<typename Element>
inline bool float_is_integer(const Element& f)
{
  return f == Element(float_to_integer(f));
}

/// \brief Returns \p self modulated by the range [0, \p modulus)
/// \p self must be in the range [\p -modulus, \p modulus)
template<typename Element, typename ModulusElement>
inline Element float_mod_range(const Element& self, const ModulusElement& modulus)
{
  return Element((self < 0.0) ? self + modulus : self);
}

/// \brief Returns \p self modulated by the range [0, \p modulus)
template<typename Element, typename ModulusElement>
inline Element float_mod(const Element& self, const ModulusElement& modulus)
{
  return float_mod_range(Element(fmod(static_cast<double>(self), static_cast<double>(modulus))), modulus);
}


template<typename Element>
class BasicVector2
{
  Element m_elements[2];
public:
  BasicVector2()
  {
  }
  BasicVector2(const Element& x_, const Element& y_)
  {
    x() = x_;
    y() = y_;
  }

  Element& x()
  {
    return m_elements[0];
  }
  const Element& x() const
  {
    return m_elements[0];
  }
  Element& y()
  {
    return m_elements[1];
  }
  const Element& y() const
  {
    return m_elements[1];
  }

  const Element& operator[](std::size_t i) const
  {
    return m_elements[i];
  }
  Element& operator[](std::size_t i)
  {
    return m_elements[i];
  }
};


template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_added(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return BasicVector2<Element>(
    Element(self.x() + other.x()),
    Element(self.y() + other.y())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator+(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return vector2_added(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_add(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  self.x() += Element(other.x());
  self.y() += Element(other.y());
}
template<typename Element, typename OtherElement>
inline void operator+=(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  vector2_add(self, other);
}


template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_subtracted(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return BasicVector2<Element>(
    Element(self.x() - other.x()),
    Element(self.y() - other.y())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator-(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return vector2_subtracted(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_subtract(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  self.x() -= Element(other.x());
  self.y() -= lement(other.y());
}
template<typename Element, typename OtherElement>
inline void operator-=(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  vector2_subtract(self, other);
}


template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_scaled(const BasicVector2<Element>& self, OtherElement other)
{
  return BasicVector2<Element>(
    Element(self.x() * other),
    Element(self.y() * other)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator*(const BasicVector2<Element>& self, OtherElement other)
{
  return vector2_scaled(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_scale(BasicVector2<Element>& self, OtherElement other)
{
  self.x() *= Element(other);
  self.y() *= Element(other);
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector2<Element>& self, OtherElement other)
{
  vector2_scale(self, other);
}


template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_scaled(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return BasicVector2<Element>(
    Element(self.x() * other.x()),
    Element(self.y() * other.y())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator*(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return vector2_scaled(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_scale(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  self.x() *= Element(other.x());
  self.y() *= Element(other.y());
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  vector2_scale(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_divided(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return BasicVector2<Element>(
    Element(self.x() / other.x()),
    Element(self.y() / other.y())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator/(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return vector2_divided(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_divide(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  self.x() /= Element(other.x());
  self.y() /= Element(other.y());
}
template<typename Element, typename OtherElement>
inline void operator/=(BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  vector2_divide(self, other);
}


template<typename Element, typename OtherElement>
inline BasicVector2<Element> vector2_divided(const BasicVector2<Element>& self, OtherElement other)
{
  return BasicVector2<Element>(
    Element(self.x() / other),
    Element(self.y() / other)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector2<Element> operator/(const BasicVector2<Element>& self, OtherElement other)
{
  return vector2_divided(self, other);
}
template<typename Element, typename OtherElement>
inline void vector2_divide(BasicVector2<Element>& self, OtherElement other)
{
  self.x() /= Element(other);
  self.y() /= Element(other);
}
template<typename Element, typename OtherElement>
inline void operator/=(BasicVector2<Element>& self, OtherElement other)
{
  vector2_divide(self, other);
}

template<typename Element, typename OtherElement>
inline double vector2_dot(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return self.x() * other.x() + self.y() * other.y();
}

template<typename Element>
inline double vector2_length_squared(const BasicVector2<Element>& self)
{
  return vector2_dot(self, self);
}

template<typename Element>
inline double vector2_length(const BasicVector2<Element>& self)
{
  return sqrt(vector2_length_squared(self));
}

template<typename Element, typename OtherElement>
inline double vector2_cross(const BasicVector2<Element>& self, const BasicVector2<OtherElement>& other)
{
  return self.x() * other.y() - self.y() * other.x();
}

typedef BasicVector2<float> Vector2;

/// \brief A 3-element vector.
template<typename Element>
class BasicVector3
{
  Element m_elements[3];
public:

  BasicVector3()
  {
  }
  template<typename OtherElement>
  BasicVector3(const BasicVector3<OtherElement>& other)
  {
    x() = static_cast<Element>(other.x());
    y() = static_cast<Element>(other.y());
    z() = static_cast<Element>(other.z());
  }
  BasicVector3(const Element& x_, const Element& y_, const Element& z_)
  {
    x() = x_;
    y() = y_;
    z() = z_;
  }

  Element& x()
  {
    return m_elements[0];
  }
  const Element& x() const
  {
    return m_elements[0];
  }
  Element& y()
  {
    return m_elements[1];
  }
  const Element& y() const
  {
    return m_elements[1];
  }
  Element& z()
  {
    return m_elements[2];
  }
  const Element& z() const
  {
    return m_elements[2];
  }

  const Element& operator[](std::size_t i) const
  {
    return m_elements[i];
  }
  Element& operator[](std::size_t i)
  {
    return m_elements[i];
  }

  operator BasicVector2<Element>&()
  {
    return reinterpret_cast<BasicVector2<Element>&>(*this);
  }
  operator const BasicVector2<Element>&() const
  {
    return reinterpret_cast<const BasicVector2<Element>&>(*this);
  }
};

/// \brief A 3-element vector stored in single-precision floating-point.
typedef BasicVector3<float> Vector3;

const Vector3 g_vector3_identity(0, 0, 0);
const Vector3 g_vector3_max = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
const Vector3 g_vector3_axis_x(1, 0, 0);
const Vector3 g_vector3_axis_y(0, 1, 0);
const Vector3 g_vector3_axis_z(0, 0, 1);

const Vector3 g_vector3_axes[3] = { g_vector3_axis_x, g_vector3_axis_y, g_vector3_axis_z };

template<typename Element>
inline Element* vector3_to_array(BasicVector3<Element>& self)
{
  return reinterpret_cast<Element*>(&self);
}
template<typename Element>
inline const Element* vector3_to_array(const BasicVector3<Element>& self)
{
  return reinterpret_cast<const Element*>(&self);
}

template<typename Element, typename OtherElement>
inline void vector3_swap(BasicVector3<Element>& self, BasicVector3<OtherElement>& other)
{
  std::swap(self.x(), other.x());
  std::swap(self.y(), other.y());
  std::swap(self.z(), other.z());
}

template<typename Element, typename OtherElement>
inline bool vector3_equal(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return self.x() == other.x() && self.y() == other.y() && self.z() == other.z();
}
template<typename Element, typename OtherElement>
inline bool operator==(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return vector3_equal(self, other);
}
template<typename Element, typename OtherElement>
inline bool operator!=(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return !vector3_equal(self, other);
}


template<typename Element, typename OtherElement, typename Epsilon>
inline bool vector3_equal_epsilon(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other, Epsilon epsilon)
{
  return float_equal_epsilon(self.x(), other.x(), epsilon)
    && float_equal_epsilon(self.y(), other.y(), epsilon)
    && float_equal_epsilon(self.z(), other.z(), epsilon);
}



template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_added(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return BasicVector3<Element>(
    Element(self.x() + other.x()),
    Element(self.y() + other.y()),
    Element(self.z() + other.z())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator+(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return vector3_added(self, other);
}
template<typename Element, typename OtherElement>
inline void vector3_add(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  self.x() += static_cast<Element>(other.x());
  self.y() += static_cast<Element>(other.y());
  self.z() += static_cast<Element>(other.z());
}
template<typename Element, typename OtherElement>
inline void operator+=(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  vector3_add(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_subtracted(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return BasicVector3<Element>(
    Element(self.x() - other.x()),
    Element(self.y() - other.y()),
    Element(self.z() - other.z())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator-(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return vector3_subtracted(self, other);
}
template<typename Element, typename OtherElement>
inline void vector3_subtract(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  self.x() -= static_cast<Element>(other.x());
  self.y() -= static_cast<Element>(other.y());
  self.z() -= static_cast<Element>(other.z());
}
template<typename Element, typename OtherElement>
inline void operator-=(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  vector3_subtract(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_scaled(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return BasicVector3<Element>(
    Element(self.x() * other.x()),
    Element(self.y() * other.y()),
    Element(self.z() * other.z())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator*(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return vector3_scaled(self, other);
}
template<typename Element, typename OtherElement>
inline void vector3_scale(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  self.x() *= static_cast<Element>(other.x());
  self.y() *= static_cast<Element>(other.y());
  self.z() *= static_cast<Element>(other.z());
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  vector3_scale(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_scaled(const BasicVector3<Element>& self, const OtherElement& scale)
{
  return BasicVector3<Element>(
    Element(self.x() * scale),
    Element(self.y() * scale),
    Element(self.z() * scale)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator*(const BasicVector3<Element>& self, const OtherElement& scale)
{
  return vector3_scaled(self, scale);
}
template<typename Element, typename OtherElement>
inline void vector3_scale(BasicVector3<Element>& self, const OtherElement& scale)
{
  self.x() *= static_cast<Element>(scale);
  self.y() *= static_cast<Element>(scale);
  self.z() *= static_cast<Element>(scale);
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector3<Element>& self, const OtherElement& scale)
{
  vector3_scale(self, scale);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_divided(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return BasicVector3<Element>(
    Element(self.x() / other.x()),
    Element(self.y() / other.y()),
    Element(self.z() / other.z())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator/(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return vector3_divided(self, other);
}
template<typename Element, typename OtherElement>
inline void vector3_divide(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  self.x() /= static_cast<Element>(other.x());
  self.y() /= static_cast<Element>(other.y());
  self.z() /= static_cast<Element>(other.z());
}
template<typename Element, typename OtherElement>
inline void operator/=(BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  vector3_divide(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_divided(const BasicVector3<Element>& self, const OtherElement& divisor)
{
  return BasicVector3<Element>(
    Element(self.x() / divisor),
    Element(self.y() / divisor),
    Element(self.z() / divisor)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> operator/(const BasicVector3<Element>& self, const OtherElement& divisor)
{
  return vector3_divided(self, divisor);
}
template<typename Element, typename OtherElement>
inline void vector3_divide(BasicVector3<Element>& self, const OtherElement& divisor)
{
  self.x() /= static_cast<Element>(divisor);
  self.y() /= static_cast<Element>(divisor);
  self.z() /= static_cast<Element>(divisor);
}
template<typename Element, typename OtherElement>
inline void operator/=(BasicVector3<Element>& self, const OtherElement& divisor)
{
  vector3_divide(self, divisor);
}

template<typename Element, typename OtherElement>
inline double vector3_dot(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return self.x() * other.x() + self.y() * other.y() + self.z() * other.z();
}

template<typename Element>
inline BasicVector3<Element> vector3_mid(const BasicVector3<Element>& begin, const BasicVector3<Element>& end)
{
  return vector3_scaled(vector3_added(begin, end), 0.5);
}

template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_cross(const BasicVector3<Element>& self, const BasicVector3<OtherElement>& other)
{
  return BasicVector3<Element>(
    Element(self.y() * other.z() - self.z() * other.y()),
    Element(self.z() * other.x() - self.x() * other.z()),
    Element(self.x() * other.y() - self.y() * other.x())
  );
}

template<typename Element>
inline BasicVector3<Element> vector3_negated(const BasicVector3<Element>& self)
{
  return BasicVector3<Element>(-self.x(), -self.y(), -self.z()); 
}

template<typename Element>
inline void vector3_negate(BasicVector3<Element>& self)
{
  self = vector3_negated(self);
}

template<typename Element>
inline double vector3_length_squared(const BasicVector3<Element>& self)
{
  return vector3_dot(self, self);
}

template<typename Element>
inline double vector3_length(const BasicVector3<Element>& self)
{
  return sqrt(vector3_length_squared(self));
}

template<typename Element>
inline Element float_divided(Element f, Element other)
{
  //ASSERT_MESSAGE(other != 0, "float_divided: invalid divisor");
  return f / other;
}

template<typename Element>
inline BasicVector3<Element> vector3_normalised(const BasicVector3<Element>& self)
{
  return vector3_scaled(self, float_divided(1.0, vector3_length(self)));
}

template<typename Element>
inline void vector3_normalise(BasicVector3<Element>& self)
{
  self = vector3_normalised(self);
}


template<typename Element>
inline BasicVector3<Element> vector3_snapped(const BasicVector3<Element>& self)
{
  return BasicVector3<Element>(
    Element(float_to_integer(self.x())),
    Element(float_to_integer(self.y())),
    Element(float_to_integer(self.z()))
  );
}
template<typename Element>
inline void vector3_snap(BasicVector3<Element>& self)
{
  self = vector3_snapped(self);
}
template<typename Element, typename OtherElement>
inline BasicVector3<Element> vector3_snapped(const BasicVector3<Element>& self, const OtherElement& snap)
{
  return BasicVector3<Element>(
    Element(float_snapped(self.x(), snap)),
    Element(float_snapped(self.y(), snap)),
    Element(float_snapped(self.z(), snap))
  );
}
template<typename Element, typename OtherElement>
inline void vector3_snap(BasicVector3<Element>& self, const OtherElement& snap)
{
  self = vector3_snapped(self, snap);
}

inline Vector3 vector3_for_spherical(double theta, double phi)
{
  return Vector3(
    static_cast<float>(cos(theta) * cos(phi)),
    static_cast<float>(sin(theta) * cos(phi)),
    static_cast<float>(sin(phi))
  );
}


/// \brief A 4-element vector.
template<typename Element>
class BasicVector4
{
  Element m_elements[4];
public:

  BasicVector4()
  {
  }
  BasicVector4(Element x_, Element y_, Element z_, Element w_)
  {
    x() = x_;
    y() = y_;
    z() = z_;
    w() = w_;
  }
  BasicVector4(const BasicVector3<Element>& self, Element w_)
  {
    x() = self.x();
    y() = self.y();
    z() = self.z();
    w() = w_;
  }

  Element& x()
  {
    return m_elements[0];
  }
  Element x() const
  {
    return m_elements[0];
  }
  Element& y()
  {
    return m_elements[1];
  }
  Element y() const
  {
    return m_elements[1];
  }
  Element& z()
  {
    return m_elements[2];
  }
  Element z() const
  {
    return m_elements[2];
  }
  Element& w()
  {
    return m_elements[3];
  }
  Element w() const
  {
    return m_elements[3];
  }

  Element index(std::size_t i) const
  {
    return m_elements[i];
  }
  Element& index(std::size_t i)
  {
    return m_elements[i];
  }
  Element operator[](std::size_t i) const
  {
    return m_elements[i];
  }
  Element& operator[](std::size_t i)
  {
    return m_elements[i];
  }

  operator BasicVector3<Element>&()
  {
    return reinterpret_cast<BasicVector3<Element>&>(*this);
  }
  operator const BasicVector3<Element>&() const
  {
    return reinterpret_cast<const BasicVector3<Element>&>(*this);
  }

  bool operator==(const BasicVector4& other) const
  {
    return x() == other.x() && y() == other.y() && z() == other.z() && w() == other.w();
  }
};

/// \brief A 4-element vector stored in single-precision floating-point.
typedef BasicVector4<float> Vector4;


template<typename Element, typename OtherElement>
inline bool vector4_equal(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return self.x() == other.x() && self.y() == other.y() && self.z() == other.z() && self.w() == other.w();
}
template<typename Element, typename OtherElement>
inline bool operator==(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return vector4_equal(self, other);
}
template<typename Element, typename OtherElement>
inline bool operator!=(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return !vector4_equal(self, other);
}

template<typename Element, typename OtherElement>
inline bool vector4_equal_epsilon(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other, Element epsilon)
{
  return float_equal_epsilon(self.x(), other.x(), epsilon)
    && float_equal_epsilon(self.y(), other.y(), epsilon)
    && float_equal_epsilon(self.z(), other.z(), epsilon)
    && float_equal_epsilon(self.w(), other.w(), epsilon);
}

template<typename Element>
inline Element* vector4_to_array(BasicVector4<Element>& self)
{
  return reinterpret_cast<Element*>(&self);
}
template<typename Element>
inline const float* vector4_to_array(const BasicVector4<Element>& self)
{
  return reinterpret_cast<const Element*>(&self);
}

template<typename Element>
inline Vector3& vector4_to_vector3(BasicVector4<Element>& self)
{
  return reinterpret_cast<BasicVector3<Element>&>(self);
}
template<typename Element>
inline const Vector3& vector4_to_vector3(const BasicVector4<Element>& self)
{
  return reinterpret_cast<const BasicVector3<Element>&>(self);
}

template<typename Element, typename OtherElement>
inline BasicVector4<Element> vector4_added(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return BasicVector4<Element>(
    float(self.x() + other.x()),
    float(self.y() + other.y()),
    float(self.z() + other.z()),
    float(self.w() + other.w())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector4<Element> operator+(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return vector4_added(self, other);
}
template<typename Element, typename OtherElement>
inline void vector4_add(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  self.x() += static_cast<float>(other.x());
  self.y() += static_cast<float>(other.y());
  self.z() += static_cast<float>(other.z());
  self.w() += static_cast<float>(other.w());
}
template<typename Element, typename OtherElement>
inline void operator+=(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  vector4_add(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector4<Element> vector4_subtracted(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return BasicVector4<Element>(
    float(self.x() - other.x()),
    float(self.y() - other.y()),
    float(self.z() - other.z()),
    float(self.w() - other.w())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector4<Element> operator-(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return vector4_subtracted(self, other);
}
template<typename Element, typename OtherElement>
inline void vector4_subtract(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  self.x() -= static_cast<float>(other.x());
  self.y() -= static_cast<float>(other.y());
  self.z() -= static_cast<float>(other.z());
  self.w() -= static_cast<float>(other.w());
}
template<typename Element, typename OtherElement>
inline void operator-=(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  vector4_subtract(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector4<Element> vector4_scaled(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return BasicVector4<Element>(
    float(self.x() * other.x()),
    float(self.y() * other.y()),
    float(self.z() * other.z()),
    float(self.w() * other.w())
  );
}
template<typename Element, typename OtherElement>
inline BasicVector4<Element> operator*(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return vector4_scaled(self, other);
}
template<typename Element, typename OtherElement>
inline void vector4_scale(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  self.x() *= static_cast<float>(other.x());
  self.y() *= static_cast<float>(other.y());
  self.z() *= static_cast<float>(other.z());
  self.w() *= static_cast<float>(other.w());
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  vector4_scale(self, other);
}

template<typename Element, typename OtherElement>
inline BasicVector4<Element> vector4_scaled(const BasicVector4<Element>& self, OtherElement scale)
{
  return BasicVector4<Element>(
    float(self.x() * scale),
    float(self.y() * scale),
    float(self.z() * scale),
    float(self.w() * scale)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector4<Element> operator*(const BasicVector4<Element>& self, OtherElement scale)
{
  return vector4_scaled(self, scale);
}
template<typename Element, typename OtherElement>
inline void vector4_scale(BasicVector4<Element>& self, OtherElement scale)
{
  self.x() *= static_cast<float>(scale);
  self.y() *= static_cast<float>(scale);
  self.z() *= static_cast<float>(scale);
  self.w() *= static_cast<float>(scale);
}
template<typename Element, typename OtherElement>
inline void operator*=(BasicVector4<Element>& self, OtherElement scale)
{
  vector4_scale(self, scale);
}

template<typename Element, typename OtherElement>
inline BasicVector4<Element> vector4_divided(const BasicVector4<Element>& self, OtherElement divisor)
{
  return BasicVector4<Element>(
    float(self.x() / divisor),
    float(self.y() / divisor),
    float(self.z() / divisor),
    float(self.w() / divisor)
  );
}
template<typename Element, typename OtherElement>
inline BasicVector4<Element> operator/(const BasicVector4<Element>& self, OtherElement divisor)
{
  return vector4_divided(self, divisor);
}
template<typename Element, typename OtherElement>
inline void vector4_divide(BasicVector4<Element>& self, OtherElement divisor)
{
  self.x() /= divisor;
  self.y() /= divisor;
  self.z() /= divisor;
  self.w() /= divisor;
}
template<typename Element, typename OtherElement>
inline void operator/=(BasicVector4<Element>& self, OtherElement divisor)
{
  vector4_divide(self, divisor);
}

template<typename Element, typename OtherElement>
inline double vector4_dot(const BasicVector4<Element>& self, const BasicVector4<OtherElement>& other)
{
  return self.x() * other.x() + self.y() * other.y() + self.z() * other.z() + self.w() * other.w();
}

template<typename Element>
inline BasicVector3<Element> vector4_projected(const BasicVector4<Element>& self)
{
  return vector3_scaled(vector4_to_vector3(self), 1.0 / self[3]);
}

#endif
