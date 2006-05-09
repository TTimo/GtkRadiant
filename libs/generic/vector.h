
#if !defined(INCLUDED_VECTOR_H)
#define INCLUDED_VECTOR_H

#include <cstddef>

template <typename Element>
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

  Element* data()
  {
    return m_elements;
  }
  const Element* data() const
  {
    return m_elements;
  }
};

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

  Element* data()
  {
    return m_elements;
  }
  const Element* data() const
  {
    return m_elements;
  }
};

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
  Element& w()
  {
    return m_elements[3];
  }
  const Element& w() const
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

  Element* data()
  {
    return m_elements;
  }
  const Element* data() const
  {
    return m_elements;
  }
};

template<typename Element>
inline BasicVector3<Element> vector3_from_array(const Element* array)
{
  return BasicVector3<Element>(array[0], array[1], array[2]);
}

template<typename Element>
inline Element* vector3_to_array(BasicVector3<Element>& self)
{
  return self.data();
}
template<typename Element>
inline const Element* vector3_to_array(const BasicVector3<Element>& self)
{
  return self.data();
}

template<typename Element>
inline Element* vector4_to_array(BasicVector4<Element>& self)
{
  return self.data();
}
template<typename Element>
inline const Element* vector4_to_array(const BasicVector4<Element>& self)
{
  return self.data();
}

template<typename Element>
inline BasicVector3<Element>& vector4_to_vector3(BasicVector4<Element>& self)
{
  return *reinterpret_cast<BasicVector3<Element>*>(vector4_to_array(self));
}
template<typename Element>
inline const BasicVector3<Element>& vector4_to_vector3(const BasicVector4<Element>& self)
{
  return *reinterpret_cast<const BasicVector3<Element>*>(vector4_to_array(self));
}

/// \brief A 2-element vector stored in single-precision floating-point.
typedef BasicVector2<float> Vector2;

/// \brief A 3-element vector stored in single-precision floating-point.
typedef BasicVector3<float> Vector3;

/// \brief A 4-element vector stored in single-precision floating-point.
typedef BasicVector4<float> Vector4;


#endif
