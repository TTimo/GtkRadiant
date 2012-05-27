
#if !defined(INCLUDED_GENERIC_BITFIELD_H)
#define INCLUDED_GENERIC_BITFIELD_H

/// \file
/// \brief Type safe bitfield.

/// \brief A bit-field value.
///
/// - Can be forward-declared when the definition of Enumeration is unknown.
/// - Can only be constructed from valid enumerated values.
/// - Can only be compared and combined with others of the same type.
///
/// \param Enumeration A type that contains an enum \c Value of the bits that can be set in this field.
template<typename Enumeration>
class BitFieldValue : public Enumeration
{
  unsigned m_value;
protected:
  explicit BitFieldValue(unsigned value) : m_value(value)
  {
  }
public:
  BitFieldValue() : m_value(0)
  {
  }
  explicit BitFieldValue(typename Enumeration::Value value) : m_value(1 << value)
  {
  }
  unsigned get() const
  {
    return m_value;
  }
};

template<typename Enumeration>
class BitFieldValueUnsafe : public BitFieldValue<Enumeration>
{
public:
  explicit BitFieldValueUnsafe(unsigned value) : BitFieldValue<Enumeration>(value)
  {
  }
};

template<typename Enumeration>
inline bool operator==(BitFieldValue<Enumeration> self, BitFieldValue<Enumeration> other)
{
  return self.get() == other.get();
}
template<typename Enumeration>
inline bool operator!=(BitFieldValue<Enumeration> self, BitFieldValue<Enumeration> other)
{
  return !operator==(self, other);
}

template<typename Enumeration>
inline BitFieldValue<Enumeration> operator|(BitFieldValue<Enumeration> self, BitFieldValue<Enumeration> other)
{
  return BitFieldValueUnsafe<Enumeration>(self.get() | other.get());
}
template<typename Enumeration>
inline BitFieldValue<Enumeration>& operator|=(BitFieldValue<Enumeration>& self, BitFieldValue<Enumeration> other)
{
  return self = self | other;
}
template<typename Enumeration>
inline BitFieldValue<Enumeration> operator&(BitFieldValue<Enumeration> self, BitFieldValue<Enumeration> other)
{
  return BitFieldValueUnsafe<Enumeration>(self.get() & other.get());
}
template<typename Enumeration>
inline BitFieldValue<Enumeration>& operator&=(BitFieldValue<Enumeration>& self, BitFieldValue<Enumeration> other)
{
  return self = self & other;
}
template<typename Enumeration>
inline BitFieldValue<Enumeration> operator~(BitFieldValue<Enumeration> self)
{
  return BitFieldValueUnsafe<Enumeration>(~self.get());
}



inline unsigned int bitfield_enable(unsigned int bitfield, unsigned int mask)
{
  return bitfield | mask;
}
inline unsigned int bitfield_disable(unsigned int bitfield, unsigned int mask)
{
  return bitfield & ~mask;
}
inline bool bitfield_enabled(unsigned int bitfield, unsigned int mask)
{
  return (bitfield & mask) != 0;
}

template<typename Enumeration>
inline BitFieldValue<Enumeration> bitfield_enable(BitFieldValue<Enumeration> bitfield, BitFieldValue<Enumeration> mask)
{
  return bitfield | mask;
}
template<typename Enumeration>
inline BitFieldValue<Enumeration> bitfield_disable(BitFieldValue<Enumeration> bitfield, BitFieldValue<Enumeration> mask)
{
  return bitfield & ~mask;
}
template<typename Enumeration>
inline bool bitfield_enabled(BitFieldValue<Enumeration> bitfield, BitFieldValue<Enumeration> mask)
{
  return (bitfield & mask).get() != 0;
}

#endif
