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

#if !defined (INCLUDED_EXPRESSION_H)
#define INCLUDED_EXPRESSION_H

#include <math/matrix.h>

template<typename Value>
class Literal
{
  Value m_value;
public:
  typedef Value value_type;

  Literal(const Value& value)
    : m_value(value)
  {
  }
  const value_type& eval() const
  {
    return m_value;
  }
};

template<typename Value>
inline Literal<Value> float_literal(const Value& value)
{
  return Literal<Value>(value);
}

template<typename Expression>
inline float float_for_expression(const Expression& expression)
{
  return expression.eval();
}


template<typename First, typename Second>
class ScalarDivided
{
  First first;
  Second second;
public:
  typedef typename First::value_type value_type;

  ScalarDivided(const First& first_, const Second& second_) : first(first_), second(second_)
  {
  }
  value_type eval() const
  {
    return static_cast<value_type>(first.eval() / second.eval());
  }
};

template<typename First, typename Second>
inline ScalarDivided<First, Second> float_divided(const First& first, const Second& second)
{
  return ScalarDivided<First, Second>(first, second);
}

template<typename First>
inline ScalarDivided<Literal<typename First::value_type>, First> float_reciprocal(const First& first)
{
  typedef typename First::value_type first_value_type;
  return ScalarDivided<Literal<first_value_type>, First>(float_literal(first_value_type(1.0)), first);
}

template<typename First>
class SquareRoot
{
  First first;
public:
  typedef typename First::value_type value_type;

  SquareRoot(const First& first_) : first(first_)
  {
  }
  value_type eval() const
  {
    return static_cast<value_type>(sqrt(first.eval()));
  }
};

template<typename First>
inline SquareRoot<First> float_square_root(const First& first)
{
  return SquareRoot<First>(first);
}


template<typename Element>
class BasicVector3Literal
{
  const BasicVector3<Element> m_value;
public:
  typedef Element value_type;
  typedef IntegralConstant<3> dimension;

  BasicVector3Literal(const BasicVector3<Element>& value)
    : m_value(value)
  {
  }
  const value_type& eval(unsigned int i) const
  {
    return m_value[i];
  }
};

template<typename Element>
inline BasicVector3Literal<Element> vector3_literal(const BasicVector3<Element>& value)
{
  return BasicVector3Literal<Element>(value);
}

typedef BasicVector3Literal<float> Vector3Literal;

template<typename Element>
class BasicVector3Identity
{
  const BasicVector3<Element>& m_value;
public:
  typedef Element value_type;
  typedef IntegralConstant<3> dimension;

  BasicVector3Identity(const BasicVector3<Element>& value)
    : m_value(value)
  {
  }
  const value_type& eval(unsigned int i) const
  {
    return m_value[i];
  }
};

template<typename Element>
inline BasicVector3Identity<Element> vector3_identity(const BasicVector3<Element>& value)
{
  return BasicVector3Identity<Element>(value);
}

typedef BasicVector3Identity<float> Vector3Identity;

template<typename Expression>
inline BasicVector3<typename Expression::value_type> vector3_for_expression(const Expression& expression)
{
  return Vector3(expression.eval(0), expression.eval(1), expression.eval(2));
}


template<typename Operation, typename First, typename Second>
class VectorScalar
{
  First first;
  Literal<typename Second::value_type> second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorScalar(const First& first_, const Second& second_)
    : first(first_), second(second_.eval())
  {
  }
  value_type eval(unsigned int i) const
  {
    return Operation::apply( first.eval(i), second.eval() );
  }
};



template<typename Operation, typename First, typename Second>
class VectorVector
{
  First first;
  Second second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorVector(const First& first_, const Second& second_)
    : first(first_), second(second_)
  {
  }
  value_type eval(unsigned int i) const
  {
    return Operation::apply(first.eval(i), second.eval(i));
  }
};

template<typename First, typename Second>
class Added
{
public:
  typedef First value_type;

  static value_type apply(const First& first, const Second& second)
  {
    return static_cast<value_type>(first + second);
  }
};

template<typename First, typename Second>
inline VectorVector<Added<typename First::value_type, typename Second::value_type>, First, Second>
vector_added(const First& first, const Second& second)
{
  typedef typename First::value_type first_value_type;
  typedef typename Second::value_type second_value_type;
  return VectorVector<Added<first_value_type, second_value_type>, First, Second>(first, second);
}

template<typename First, typename Second>
class Multiplied
{
public:
  typedef First value_type;

  static value_type apply(const First& first, const Second& second)
  {
    return static_cast<value_type>(first * second);
  }
};

template<typename First, typename Second>
inline VectorVector<Multiplied<typename First::value_type, typename Second::value_type>, First, Second>
vector_multiplied(const First& first, const Second& second)
{
  typedef typename First::value_type first_value_type;
  typedef typename Second::value_type second_value_type;
  return VectorVector<Multiplied<first_value_type, second_value_type>, First, Second>(first, second);
}

template<typename First, typename Second>
inline VectorScalar<Multiplied<typename First::value_type, typename Second::value_type>, First, Second>
vector_scaled(const First& first, const Second& second)
{
  typedef typename First::value_type first_value_type;
  typedef typename Second::value_type second_value_type;
  return VectorScalar<Multiplied<first_value_type, second_value_type>, First, Second>(first, second);
}

template<typename First>
class Negated
{
public:
  typedef First value_type;

  static value_type apply(const First& first)
  {
    return -first;
  }
};

template<typename First, typename Operation>
class VectorUnary
{
  First first;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorUnary(const First& first_) : first(first_)
  {
  }
  value_type eval(unsigned int i) const
  {
    return Operation::apply(first.eval(i));
  }
};

template<typename First>
inline VectorUnary<First, Negated<typename First::value_type> >
vector_negated(const First& first)
{
  typedef typename First::value_type first_value_type;
  return VectorUnary<First, Negated<first_value_type> >(first);
}

template<typename First, typename Second>
class VectorCross
{
  First first;
  Second second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorCross(const First& first_, const Second& second_)
    : first(first_), second(second_)
  {
  }
  value_type eval(unsigned int i) const
  {
    return first.eval((i+1)%3) * second.eval((i+2)%3) - first.eval((i+2)%3) * second.eval((i+1)%3);
  }
};

template<typename First, typename Second>
inline VectorCross<First, Second>
vector_cross(const First& first, const Second& second)
{
  return VectorCross<First, Second>(first, second);
}


template<typename First, typename Second>
class VectorDot
{
  First first;
  Second second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorDot(const First& first_, const Second& second_)
    : first(first_), second(second_)
  {
  }

  template<typename Index>
  struct eval_dot
  {
    static value_type apply(const First& first, const Second& second)
    {
      return static_cast<value_type>(
        first.eval(Index::VALUE) * second.eval(Index::VALUE)
        + eval_dot< IntegralConstant<Index::VALUE-1> >::apply(first, second)
      );
    }
  };

  template<>
  struct eval_dot< IntegralConstant<0> >
  {
    static value_type apply(const First& first, const Second& second)
    {
      return first.eval(0) * second.eval(0);
    }
  };

  value_type eval() const
  {
    return eval_dot< IntegralConstant<dimension::VALUE - 1> >::apply(first, second);
  }
};


template<typename First, typename Second>
inline VectorDot<First, Second> vector_dot(const First& first, const Second& second)
{
  return VectorDot<First, Second>(first, second);
}

template<typename First>
class VectorLengthSquared
{
  First first;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  VectorLengthSquared(const First& first_)
    : first(first_)
  {
  }

  static value_type squared(const value_type& value)
  {
    return value * value;
  }

  template<typename Index>
  struct eval_squared
  {
    static value_type apply(const First& first)
    {
      return static_cast<value_type>(
        squared(first.eval(Index::VALUE))
        + eval_squared< IntegralConstant<Index::VALUE - 1> >::apply(first)
      );
    }
  };

  template<>
  struct eval_squared< IntegralConstant<0> >
  {
    static value_type apply(const First& first)
    {
      return squared(first.eval(0));
    }
  };

  value_type eval() const
  {
    return eval_squared< IntegralConstant<dimension::VALUE - 1> >::apply(first);
  }
};

template<typename First>
inline VectorLengthSquared<First> vector_length_squared(const First& first)
{
  return VectorLengthSquared<First>(first);
}

template<typename First>
inline SquareRoot< VectorLengthSquared<First> > vector_length(const First& first)
{
  return float_square_root(vector_length_squared(first));
}

#if 1
template<typename First>
inline VectorScalar<
  Multiplied<typename First::value_type, typename First::value_type>,
  First,
  // multiple evaulations of subexpression
  ScalarDivided<
    Literal<typename First::value_type>,
    SquareRoot<
      VectorLengthSquared<First>
    >
  >
> vector_normalised(const First& first)
{
  typedef typename First::value_type first_value_type;
  return vector_scaled(first, float_reciprocal(vector_length(first)));
}
#else
template<typename First>
inline VectorScalar<
  Multiplied<typename First::value_type, typename First::value_type>,
  First,
  // single evaluation of subexpression
  Literal<typename First::value_type>
>
vector_normalised(const First& first)
{
  typedef typename First::value_type first_value_type;
  return vector_scaled(first, float_literal(static_cast<first_value_type>(first_value_type(1.0) / vector_length(first).eval())));
}
#endif


class Matrix4Literal
{
  const Matrix4 m_value;
public:
  typedef float value_type;
  typedef IntegralConstant<4> dimension0;
  typedef IntegralConstant<4> dimension1;

  Matrix4Literal(const Matrix4& value)
    : m_value(value)
  {
  }
  const value_type& eval(unsigned int r, unsigned int c) const
  {
    return m_value[r*4+c];
  }
};

inline Matrix4Literal matrix4_literal(const Matrix4& value)
{
  return Matrix4Literal(value);
}

class Matrix4Identity
{
  const Matrix4& m_value;
public:
  typedef float value_type;
  typedef IntegralConstant<4> dimension0;
  typedef IntegralConstant<4> dimension1;

  Matrix4Identity(const Matrix4& value)
    : m_value(value)
  {
  }
  const value_type& eval(unsigned int r, unsigned int c) const
  {
    return m_value[r*4+c];
  }
};

inline Matrix4Identity matrix4_identity(const Matrix4& value)
{
  return Matrix4Identity(value);
}

template<typename Expression>
inline Matrix4 matrix4_for_expression(const Expression& expression)
{
  return Matrix4(
    expression.eval(0, 0), expression.eval(0, 1), expression.eval(0, 2), expression.eval(0, 3),
    expression.eval(1, 0), expression.eval(1, 1), expression.eval(1, 2), expression.eval(1, 3),
    expression.eval(2, 0), expression.eval(2, 1), expression.eval(2, 2), expression.eval(2, 3),
    expression.eval(3, 0), expression.eval(3, 1), expression.eval(3, 2), expression.eval(3, 3)
  );
}

template<typename Expression>
inline Matrix4 matrix4_affine_for_expression(const Expression& expression)
{
  return Matrix4(
    expression.eval(0, 0), expression.eval(0, 1), expression.eval(0, 2), 0,
    expression.eval(1, 0), expression.eval(1, 1), expression.eval(1, 2), 0,
    expression.eval(2, 0), expression.eval(2, 1), expression.eval(2, 2), 0,
    expression.eval(3, 0), expression.eval(3, 1), expression.eval(3, 2), 1
  );
}


template<typename First, typename Second>
class PointMultiplied
{
  const First& first;
  const Second& second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension dimension;

  PointMultiplied(const First& first_, const Second& second_)
    : first(first_), second(second_)
  {
  }
  value_type eval(unsigned int i) const
  {
    return static_cast<value_type>(second.eval(0, i) * first.eval(0)
      + second.eval(1, i) * first.eval(1)
      + second.eval(2, i) * first.eval(2)
      + second.eval(3, i));
  }
};

template<typename First, typename Second>
inline PointMultiplied<First, Second> point_multiplied(const First& point, const Second& matrix)
{
  return PointMultiplied<First, Second>(point, matrix);
}

template<typename First, typename Second>
class Matrix4Multiplied
{
  const First& first;
  const Second& second;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension0 dimension0;
  typedef typename First::dimension1 dimension1;

  Matrix4Multiplied(const First& first_, const Second& second_)
    : first(first_), second(second_)
  {
  }

  value_type eval(unsigned int r, unsigned int c) const
  {
    return static_cast<value_type>(
      second.eval(r, 0) * first.eval(0, c)
      + second.eval(r, 1) * first.eval(1, c)
      + second.eval(r, 2) * first.eval(2, c)
      + second.eval(r, 3) * first.eval(3, c)
    );
  }
};

template<typename First, typename Second>
inline Matrix4Multiplied<First, Second> matrix4_multiplied(const First& first, const Second& second)
{
  return Matrix4Multiplied<First, Second>(first, second);
}

template<typename First>
class MatrixTransposed
{
  const First& first;
public:
  typedef typename First::value_type value_type;
  typedef typename First::dimension0 dimension0;
  typedef typename First::dimension1 dimension1;

  MatrixTransposed(const First& first_)
    : first(first_)
  {
  }

  value_type eval(unsigned int r, unsigned int c) const
  {
    return first.eval(c, r);
  }
};

template<typename First>
inline MatrixTransposed<First> matrix_transposed(const First& first)
{
  return MatrixTransposed<First>(first);
}

#endif
