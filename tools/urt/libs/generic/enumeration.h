
#if !defined( INCLUDED_GENERIC_ENUMERATION_H )
#define INCLUDED_GENERIC_ENUMERATION_H

/// \file
/// \brief Type safe enumeration.

/// \brief An enumerated value.
///
/// - Can be forward-declared when the definition of Enumeration is unknown.
/// - Can only be constructed from valid enumerated values.
/// - Can only be compared with others of the same type.
///
/// \param Enumeration A type that contains an enum \c Value of the allowed values of the enumeration.
template<typename Enumeration>
class EnumeratedValue : public Enumeration
{
typename Enumeration::Value m_value;
public:
explicit EnumeratedValue( typename Enumeration::Value value ) : m_value( value ){
}
typename Enumeration::Value get() const {
	return m_value;
}
};

template<typename Enumeration>
inline bool operator==( EnumeratedValue<Enumeration> self, EnumeratedValue<Enumeration> other ){
	return self.get() == other.get();
}
template<typename Enumeration>
inline bool operator!=( EnumeratedValue<Enumeration> self, EnumeratedValue<Enumeration> other ){
	return !operator==( self, other );
}

#endif
