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

#include "expression.h"

Vector3 testAdded1(const Vector3& a, const Vector3& b)
{
  return vector3_added(a, vector3_added(a, b));
}

Vector3 testAdded2(const Vector3& a, const Vector3& b)
{
  return vector3_for_expression( vector_added( vector3_identity(a), vector_added( vector3_identity(a), vector3_identity(b) ) ) );
}

Vector3 testMultiplied1(const Vector3& a, const Vector3& b)
{
  return vector3_scaled(a, b);
}

Vector3 testMultiplied2(const Vector3& a, const Vector3& b)
{
  return vector3_for_expression( vector_multiplied( vector3_identity(a), vector3_identity(b) ) );
}

Vector3 testCross1(const Vector3& a, const Vector3& b)
{
  return vector3_cross(a, b);
}

Vector3 testCross2(const Vector3& a, const Vector3& b)
{
  return vector3_for_expression( vector_cross( vector3_identity(a), vector3_identity(b) ) );
}

double testDot1(const Vector3& a, const Vector3& b)
{
  return vector3_dot(a, b);
}

double testDot2(const Vector3& a, const Vector3& b)
{
  return float_for_expression( vector_dot( vector3_identity(a), vector3_identity(b) ) );
}

double testLength1(const Vector3& a)
{
  return vector3_length(a);
}

double testLength2(const Vector3& a)
{
  return float_for_expression( vector_length( vector3_identity(a) ) );
}

Vector3 testNormalised1(const Vector3& a)
{
  return vector3_normalised(a);
}

Vector3 testNormalised2(const Vector3& a)
{
  return vector3_for_expression( vector_normalised( vector3_identity(a) ) );
}

Vector3 testNegated1(const Vector3& a)
{
  return vector3_negated(a);
}

Vector3 testNegated2(const Vector3& a)
{
  return vector3_for_expression( vector_negated( vector3_identity(a) ) );
}

Vector3 testScaled1(const Vector3& a, const double& b)
{
  return vector3_scaled(a, b);
}

Vector3 testScaled2(const Vector3& a, const double& b)
{
  return vector3_for_expression( vector_scaled( vector3_identity(a), float_literal(b) ) );
}

Vector3 testMatrixMultiplied1(const Vector3& a, const Matrix4& b)
{
  return matrix4_transformed_point(b, vector3_added(a, Vector3(1, 0, 0)));
}

Vector3 testMatrixMultiplied2(const Vector3& a, const Matrix4& b)
{
  return vector3_for_expression(
    point_multiplied(
      vector_added(
        vector3_identity(a),
        vector3_literal(Vector3(1, 0, 0))
      ),
      matrix4_identity(b)
    )
  );
}

Matrix4 testMatrix4Multiplied1(const Matrix4& a, const Matrix4& b)
{
  return matrix4_multiplied_by_matrix4(a, matrix4_multiplied_by_matrix4(a, b));
}

Matrix4 testMatrix4Multiplied2(const Matrix4& a, const Matrix4& b)
{
  return matrix4_for_expression(
    matrix4_multiplied(
      matrix4_identity(a),
      matrix4_identity(b)
    )
  );
}

Matrix4 testMatrix4AffineMultiplied1(const Matrix4& a, const Matrix4& b)
{
  return matrix4_affine_multiplied_by_matrix4(a, b);
}

Matrix4 testMatrix4AffineMultiplied2(const Matrix4& a, const Matrix4& b)
{
  return matrix4_affine_for_expression(
    matrix4_multiplied(
      matrix4_identity(a),
      matrix4_identity(b)
    )
  );
}

Matrix4 testMatrix4MultipliedConstant1(const Matrix4& a)
{
  return matrix4_multiplied_by_matrix4(a, g_matrix4_identity);
}

Matrix4 testMatrix4MultipliedConstant2(const Matrix4& a)
{
  return matrix4_for_expression(
    matrix4_multiplied(
      matrix4_identity(a),
      matrix4_identity(g_matrix4_identity)
    )
  );
}
Matrix4 testMatrix4Transposed1(const Matrix4& a)
{
  return matrix4_transposed(a);
}

Matrix4 testMatrix4Transposed2(const Matrix4& a)
{
  return matrix4_for_expression( matrix_transposed( matrix4_identity(a) ) );
}

Vector3 testMulti1(const Matrix4& a, const Vector3& b, const Vector3& c)
{
  return vector3_added(matrix4_transformed_point(matrix4_transposed(a), b), c);
}

Vector3 testMulti2(const Matrix4& a, const Vector3& b, const Vector3& c)
{
  return vector3_for_expression(
    vector_added(
      point_multiplied(
        vector3_identity(b),
        matrix_transposed(matrix4_identity(a))
      ),
      vector3_identity(c)
    )
  );
}

template<typename Value, typename First, typename Second>
class TestBinaryFunction
{
  typedef Value(*Function)(const First&, const Second&);
  Function m_function;
public:

  TestBinaryFunction(Function function) : m_function(function)
  {
  }
  Value run(const First& first, const Second& second) const
  {
    return m_function(first, second);
  }
};

template<typename Value, typename First>
class TestUnaryFunction
{
  typedef Value(*Function)(const First&);
  Function m_function;
public:

  TestUnaryFunction(Function function) : m_function(function)
  {
  }
  Value run(const First& first) const
  {
    return m_function(first);
  }
};

class TestAll
{
public:
  TestAll()
  {
    Vector3 result1 = TestBinaryFunction<Vector3, Vector3, Vector3>(testAdded1).run(Vector3(0, 0, 0), Vector3(1, 1, 1));
    Vector3 result2 = TestBinaryFunction<Vector3, Vector3, Vector3>(testAdded2).run(Vector3(0, 0, 0), Vector3(1, 1, 1));
    Vector3 result3 = TestBinaryFunction<Vector3, Vector3, Vector3>(testMultiplied1).run(Vector3(1, 2, 3), Vector3(2, 1, 0.5f));
    Vector3 result4 = TestBinaryFunction<Vector3, Vector3, Vector3>(testMultiplied2).run(Vector3(1, 2, 3), Vector3(2, 1, 0.5f));
    Vector3 result5 = TestBinaryFunction<Vector3, Vector3, double>(testScaled1).run(Vector3(1, 2, 3), 2.0);
    Vector3 result6 = TestBinaryFunction<Vector3, Vector3, double>(testScaled2).run(Vector3(1, 2, 3), 2.0);
    Vector3 result7 = TestBinaryFunction<Vector3, Vector3, Matrix4>(testMatrixMultiplied1).run(Vector3(1, 2, 3), matrix4_rotation_for_x_degrees(90));
    Vector3 result8 = TestBinaryFunction<Vector3, Vector3, Matrix4>(testMatrixMultiplied2).run(Vector3(1, 2, 3), matrix4_rotation_for_x_degrees(90));
    Vector3 result9 = TestUnaryFunction<Vector3, Vector3>(testNormalised1).run(Vector3(1, 2, 3));
    Vector3 result10 = TestUnaryFunction<Vector3, Vector3>(testNormalised2).run(Vector3(1, 2, 3));
  }
} g_testAll;

