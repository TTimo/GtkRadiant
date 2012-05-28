
#if !defined( INCLUDED_GENERIC_REFERENCE_H )
#define INCLUDED_GENERIC_REFERENCE_H

/// \file
/// \brief Wrappers to allow storing objects in templated containers using 'reference' semantics.

/// \brief A reference to a mutable object.
/// Has 'reference' semantics, except for \c 'operator==' and \c 'operator.'.
/// \param Type The type of the referenced object.
template<typename Type>
class Reference
{
Type* m_contained;
public:
explicit Reference( Type& contained ) : m_contained( &contained ){
}
Type& operator*() const {
	return *m_contained;
}
Type* operator->() const {
	return m_contained;
}
operator Type&() const
{
	return *m_contained;
}
Type& get() const {
	return *m_contained;
}
Type* get_pointer() const {
	return m_contained;
}
};

template<typename Type>
bool operator<( const Reference<Type>& self, const Reference<Type>& other ){
	return self.get() < other.get();
}
template<typename Type>
bool operator==( const Reference<Type>& self, const Reference<Type>& other ){
	return self.get() == other.get();
}

/// \brief construct a reference to a mutable object.
template<typename Type>
inline Reference<Type> makeReference( Type& value ){
	return Reference<Type>( value );
}

/// \brief A reference to a non-mutable object.
/// Has 'reference' semantics, except for \c 'operator==' and \c 'operator.'.
/// \param Type The type of the referenced object.
template<typename Type>
class ConstReference
{
const Type* m_contained;
public:
explicit ConstReference( const Type& contained ) : m_contained( &contained ){
}
const Type& operator*() const {
	return *m_contained;
}
const Type* operator->() const {
	return m_contained;
}
operator const Type&() const
{
	return *m_contained;
}
const Type& get() const {
	return *m_contained;
}
const Type* get_pointer() const {
	return m_contained;
}
};

template<typename Type>
bool operator<( const ConstReference<Type>& self, const ConstReference<Type>& other ){
	return self.get() < other.get();
}
template<typename Type>
bool operator==( const ConstReference<Type>& self, const ConstReference<Type>& other ){
	return self.get() == other.get();
}

/// \brief construct a reference to a non-mutable object.
template<typename Type>
inline ConstReference<Type> makeReference( const Type& value ){
	return ConstReference<Type>( value );
}


#endif
