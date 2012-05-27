
#if !defined(INCLUDED_GENERIC_ARRAYRANGE_H)
#define INCLUDED_GENERIC_ARRAYRANGE_H

/// \file
/// \brief Macros for automatically converting a compile-time-sized array to a range. 

template<typename Element>
struct ArrayRange
{
  typedef Element* Iterator;
  ArrayRange(Iterator _begin, Iterator _end)
    : begin(_begin), end(_end)
  {
  }
  Iterator begin;
  Iterator end;
};

template<typename Element>
inline ArrayRange<Element> makeArrayRange(Element* begin, Element* end)
{
  return ArrayRange<Element>(begin, end);
}

template<typename Element>
struct ArrayConstRange
{
  typedef const Element* Iterator;
  ArrayConstRange(Iterator _begin, Iterator _end)
    : begin(_begin), end(_end)
  {
  }
  Iterator begin;
  Iterator end;
};

template<typename Element>
inline ArrayConstRange<Element> makeArrayRange(const Element* begin, const Element* end)
{
  return ArrayConstRange<Element>(begin, end);
}

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#define ARRAY_END(array) (array + ARRAY_SIZE(array))
#define ARRAY_RANGE(array) (makeArrayRange(array, ARRAY_END(array)))


typedef ArrayConstRange<const char*> StringArrayRange;
#define STRING_ARRAY_RANGE(array) (StringArrayRange(array, ARRAY_END(array)))

#endif
