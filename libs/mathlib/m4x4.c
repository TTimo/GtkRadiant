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

#include "mathlib.h"

const m4x4_t g_m4x4_identity = {
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1,
};

void m4x4_identity(m4x4_t matrix)
{
  matrix[1] = matrix[2] = matrix[3] =
  matrix[4] = matrix[6] = matrix[7] =
  matrix[8] = matrix[9] = matrix[11] =
  matrix[12] = matrix[13] = matrix[14] = 0;

  matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;
}

m4x4Handedness_t m4x4_handedness(const m4x4_t matrix)
{
  vec3_t cross;
  CrossProduct(matrix+0, matrix+4, cross);
  return (DotProduct(matrix+8, cross) < 0) ? eLeftHanded : eRightHanded;
}

void m4x4_assign(m4x4_t matrix, const m4x4_t other)
{
  M4X4_COPY(matrix, other);
}

void m4x4_translation_for_vec3(m4x4_t matrix, const vec3_t translation)
{
  matrix[1] = matrix[2] = matrix[3] =
  matrix[4] = matrix[6] = matrix[7] =
  matrix[8] = matrix[9] = matrix[11] = 0;

  matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;

  matrix[12] = translation[0];
  matrix[13] = translation[1];
  matrix[14] = translation[2];
}

/*
clockwise rotation around X, Y, Z, facing along axis
 1  0   0    cy 0  sy   cz  sz 0
 0  cx  sx   0  1  0   -sz  cz 0
 0 -sx  cx  -sy 0  cy   0   0  1

rows of Z by cols of Y
 cy*cz -sy*cz+sz -sy*sz+cz
-sz*cy -sz*sy+cz 

  .. or something like that..

final rotation is Z * Y * X
 cy*cz -sx*-sy*cz+cx*sz  cx*-sy*sz+sx*cz
-cy*sz  sx*sy*sz+cx*cz  -cx*-sy*sz+sx*cz
 sy    -sx*cy            cx*cy
*/

/* transposed
|  cy.cz + 0.sz + sy.0            cy.-sz + 0 .cz +  sy.0          cy.0  + 0 .0  +   sy.1       |
|  sx.sy.cz + cx.sz + -sx.cy.0    sx.sy.-sz + cx.cz + -sx.cy.0    sx.sy.0  + cx.0  + -sx.cy.1  |
| -cx.sy.cz + sx.sz +  cx.cy.0   -cx.sy.-sz + sx.cz +  cx.cy.0   -cx.sy.0  + 0 .0  +  cx.cy.1  |
*/
void m4x4_rotation_for_vec3(m4x4_t matrix, const vec3_t euler, eulerOrder_t order)
{
  double cx, sx, cy, sy, cz, sz;
    
  cx = cos(DEG2RAD(euler[0]));
  sx = sin(DEG2RAD(euler[0]));
  cy = cos(DEG2RAD(euler[1]));
  sy = sin(DEG2RAD(euler[1]));
  cz = cos(DEG2RAD(euler[2]));
  sz = sin(DEG2RAD(euler[2]));

  switch(order)
  {
  case eXYZ:

#if 1

    {
      matrix[0]  = (vec_t)(cy*cz);
      matrix[1]  = (vec_t)(cy*sz);
      matrix[2]  = (vec_t)-sy;
      matrix[4]  = (vec_t)(sx*sy*cz + cx*-sz);
      matrix[5]  = (vec_t)(sx*sy*sz + cx*cz);
      matrix[6]  = (vec_t)(sx*cy);
      matrix[8]  = (vec_t)(cx*sy*cz + sx*sz);
      matrix[9]  = (vec_t)(cx*sy*sz + -sx*cz);
      matrix[10] = (vec_t)(cx*cy);
    }

    matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
    matrix[15] =  1;

#else

    m4x4_identity(matrix);
    matrix[5] =(vec_t) cx; matrix[6] =(vec_t) sx;
    matrix[9] =(vec_t)-sx; matrix[10]=(vec_t) cx;

    {
      m4x4_t temp;
      m4x4_identity(temp);
      temp[0] =(vec_t) cy; temp[2] =(vec_t)-sy;
      temp[8] =(vec_t) sy; temp[10]=(vec_t) cy;
      m4x4_premultiply_by_m4x4(matrix, temp);
      m4x4_identity(temp);
      temp[0] =(vec_t) cz; temp[1] =(vec_t) sz;
      temp[4] =(vec_t)-sz; temp[5] =(vec_t) cz;
      m4x4_premultiply_by_m4x4(matrix, temp);
    }
#endif

    break;

  case eYZX:
    m4x4_identity(matrix);
    matrix[0] =(vec_t) cy; matrix[2] =(vec_t)-sy;
    matrix[8] =(vec_t) sy; matrix[10]=(vec_t) cy;

    {
      m4x4_t temp;
      m4x4_identity(temp);
      temp[5] =(vec_t) cx; temp[6] =(vec_t) sx;
      temp[9] =(vec_t)-sx; temp[10]=(vec_t) cx;
      m4x4_premultiply_by_m4x4(matrix, temp);
      m4x4_identity(temp);
      temp[0] =(vec_t) cz; temp[1] =(vec_t) sz;
      temp[4] =(vec_t)-sz; temp[5] =(vec_t) cz;
      m4x4_premultiply_by_m4x4(matrix, temp);
    }
    break;

  case eZXY:
    m4x4_identity(matrix);
    matrix[0] =(vec_t) cz; matrix[1] =(vec_t) sz;
    matrix[4] =(vec_t)-sz; matrix[5] =(vec_t) cz;

    {
      m4x4_t temp;
      m4x4_identity(temp);
      temp[5] =(vec_t) cx; temp[6] =(vec_t) sx;
      temp[9] =(vec_t)-sx; temp[10]=(vec_t) cx;
      m4x4_premultiply_by_m4x4(matrix, temp);
      m4x4_identity(temp);
      temp[0] =(vec_t) cy; temp[2] =(vec_t)-sy;
      temp[8] =(vec_t) sy; temp[10]=(vec_t) cy;
      m4x4_premultiply_by_m4x4(matrix, temp);
    }
    break;

  case eXZY:
    m4x4_identity(matrix);
    matrix[5] =(vec_t) cx; matrix[6] =(vec_t) sx;
    matrix[9] =(vec_t)-sx; matrix[10]=(vec_t) cx;

    {
      m4x4_t temp;
      m4x4_identity(temp);
      temp[0] =(vec_t) cz; temp[1] =(vec_t) sz;
      temp[4] =(vec_t)-sz; temp[5] =(vec_t) cz;
      m4x4_premultiply_by_m4x4(matrix, temp);
      m4x4_identity(temp);
      temp[0] =(vec_t) cy; temp[2] =(vec_t)-sy;
      temp[8] =(vec_t) sy; temp[10]=(vec_t) cy;
      m4x4_premultiply_by_m4x4(matrix, temp);
    }
    break;

  case eYXZ:

/* transposed
|  cy.cz + sx.sy.-sz + -cx.sy.0   0.cz + cx.-sz + sx.0   sy.cz + -sx.cy.-sz + cx.cy.0 |
|  cy.sz + sx.sy.cz + -cx.sy.0    0.sz + cx.cz + sx.0    sy.sz + -sx.cy.cz + cx.cy.0  |
|  cy.0 + sx.sy.0 + -cx.sy.1      0.0 + cx.0 + sx.1      sy.0 + -sx.cy.0 + cx.cy.1    |
*/

#if 1

  {
    matrix[0]  = (vec_t)(cy*cz + sx*sy*-sz);
    matrix[1]  = (vec_t)(cy*sz + sx*sy*cz);
    matrix[2]  = (vec_t)(-cx*sy);
    matrix[4]  = (vec_t)(cx*-sz);
    matrix[5]  = (vec_t)(cx*cz);
    matrix[6]  = (vec_t)(sx);
    matrix[8]  = (vec_t)(sy*cz + -sx*cy*-sz);
    matrix[9]  = (vec_t)(sy*sz + -sx*cy*cz);
    matrix[10] = (vec_t)(cx*cy);
  }

  matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
  matrix[15] =  1;

#else

  m4x4_identity(matrix);
  matrix[0] =(vec_t) cy; matrix[2] =(vec_t)-sy;
  matrix[8] =(vec_t) sy; matrix[10]=(vec_t) cy;

  {
    m4x4_t temp;
    m4x4_identity(temp);
    temp[5] =(vec_t) cx; temp[6] =(vec_t) sx;
    temp[9] =(vec_t)-sx; temp[10]=(vec_t) cx;
    m4x4_premultiply_by_m4x4(matrix, temp);
    m4x4_identity(temp);
    temp[0] =(vec_t) cz; temp[1] =(vec_t) sz;
    temp[4] =(vec_t)-sz; temp[5] =(vec_t) cz;
    m4x4_premultiply_by_m4x4(matrix, temp);
  }
#endif
  break;

  case eZYX:
#if 1

  {
    matrix[0]  = (vec_t)(cy*cz);
    matrix[4]  = (vec_t)(cy*-sz);
    matrix[8]  = (vec_t)sy;
    matrix[1]  = (vec_t)(sx*sy*cz + cx*sz);
    matrix[5]  = (vec_t)(sx*sy*-sz + cx*cz);
    matrix[9]  = (vec_t)(-sx*cy);
    matrix[2]  = (vec_t)(cx*-sy*cz + sx*sz);
    matrix[6]  = (vec_t)(cx*-sy*-sz + sx*cz);
    matrix[10] = (vec_t)(cx*cy);
  }

  matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
  matrix[15] =  1;

#else

  m4x4_identity(matrix);
  matrix[0] =(vec_t) cz; matrix[1] =(vec_t) sz;
  matrix[4] =(vec_t)-sz; matrix[5] =(vec_t) cz;
  {
    m4x4_t temp;
    m4x4_identity(temp);
    temp[0] =(vec_t) cy; temp[2] =(vec_t)-sy;
    temp[8] =(vec_t) sy; temp[10]=(vec_t) cy;
    m4x4_premultiply_by_m4x4(matrix, temp);
    m4x4_identity(temp);
    temp[5] =(vec_t) cx; temp[6] =(vec_t) sx;
    temp[9] =(vec_t)-sx; temp[10]=(vec_t) cx;
    m4x4_premultiply_by_m4x4(matrix, temp);
  }

#endif
  break;

  }
}

void m4x4_scale_for_vec3(m4x4_t matrix, const vec3_t scale)
{
  matrix[1] = matrix[2] = matrix[3] =
  matrix[4] = matrix[6] = matrix[7] =
  matrix[8] = matrix[9] = matrix[11] =
  matrix[12] = matrix[13] = matrix[14] = 0;

  matrix[15] = 1;

  matrix[0] = scale[0];
  matrix[5] = scale[1];
  matrix[10] = scale[2];
}

void m4x4_rotation_for_quat(m4x4_t matrix, const vec4_t quat)
{
#if 0
  const double xx = quat[0] * quat[0];
  const double xy = quat[0] * quat[1];
  const double xz = quat[0] * quat[2];
  const double xw = quat[0] * quat[3];

  const double yy = quat[1] * quat[1];
  const double yz = quat[1] * quat[2];
  const double yw = quat[1] * quat[3];

  const double zz = quat[2] * quat[2];
  const double zw = quat[2] * quat[3];

  matrix[0]  = 1 - 2 * ( yy + zz );
  matrix[4]  =     2 * ( xy - zw );
  matrix[8]  =     2 * ( xz + yw );

  matrix[1]  =     2 * ( xy + zw );
  matrix[5]  = 1 - 2 * ( xx + zz );
  matrix[9]  =     2 * ( yz - xw );

  matrix[2]  =     2 * ( xz - yw );
  matrix[6]  =     2 * ( yz + xw );
  matrix[10] = 1 - 2 * ( xx + yy );
#else
  const double x2 = quat[0] + quat[0];
  const double y2 = quat[1] + quat[1]; 
  const double z2 = quat[2] + quat[2];
  const double xx = quat[0] * x2;
  const double xy = quat[0] * y2;
  const double xz = quat[0] * z2;
  const double yy = quat[1] * y2;
  const double yz = quat[1] * z2;
  const double zz = quat[2] * z2;
  const double wx = quat[3] * x2;
  const double wy = quat[3] * y2;
  const double wz = quat[3] * z2;

  matrix[0] = (vec_t)( 1.0 - (yy + zz) );
  matrix[4] = (vec_t)(xy - wz);
  matrix[8] = (vec_t)(xz + wy);

  matrix[1] = (vec_t)(xy + wz);
  matrix[5] = (vec_t)( 1.0 - (xx + zz) );
  matrix[9] = (vec_t)(yz - wx);

  matrix[2] = (vec_t)(xz - wy);
  matrix[6] = (vec_t)(yz + wx);
  matrix[10] = (vec_t)( 1.0 - (xx + yy) );
#endif

  matrix[3]  = matrix[7] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0;
  matrix[15] = 1;
}

void m4x4_rotation_for_axisangle(m4x4_t matrix, const vec3_t axis, double angle)
{
  vec4_t quat;
  quat_for_axisangle(quat, axis, angle);
  m4x4_rotation_for_quat(matrix, quat);
}

void m4x4_frustum(m4x4_t matrix,
		      vec_t left, vec_t right,
		      vec_t bottom, vec_t top,
		      vec_t nearval, vec_t farval)
{
   matrix[0] = (vec_t)( (2*nearval) / (right-left) );
   matrix[1] = 0;
   matrix[2] = 0;
   matrix[3] = 0;

   matrix[4] = 0;
   matrix[5] = (vec_t)( (2*nearval) / (top-bottom) );
   matrix[6] = 0;
   matrix[7] = 0;

   matrix[8] = (vec_t)( (right+left) / (right-left) );
   matrix[9] = (vec_t)( (top+bottom) / (top-bottom) );
   matrix[10] = (vec_t)( -(farval+nearval) / (farval-nearval) );
   matrix[11] =-1;

   matrix[12] = 0;
   matrix[13] = 0;
   matrix[14] = (vec_t)( -(2*farval*nearval) / (farval-nearval) );
   matrix[15] = 0;
}


void m4x4_get_translation_vec3(const m4x4_t matrix, vec3_t translation)
{
  translation[0] = matrix[12];
	translation[1] = matrix[13];
	translation[2] = matrix[14];
}

void m4x4_get_rotation_vec3(const m4x4_t matrix, vec3_t euler, eulerOrder_t order)
{
  double a, ca;

  switch(order)
  {
  case eXYZ:
    a = asin(-matrix[2]);
    ca = cos(a);
    euler[1] = (vec_t)RAD2DEG(a);  /* Calculate Y-axis angle */    

    if (fabs(ca) > 0.005) /* Gimbal lock? */
    {
      /* No, so get Z-axis angle */
      euler[2] = (vec_t)RAD2DEG(atan2(matrix[1] / ca, matrix[0]/ ca));

      /* Get X-axis angle */
      euler[0] = (vec_t)RAD2DEG(atan2(matrix[6] / ca, matrix[10] / ca));
    }
    else /* Gimbal lock has occurred */
    {
      /* Set Z-axis angle to zero */
      euler[2]  = 0;

      /* And calculate X-axis angle */
      euler[0] = (vec_t)RAD2DEG(atan2(-matrix[9], matrix[5]));
    }
    break;
  case eYZX:
    /* NOT IMPLEMENTED */
    break;
  case eZXY:
    /* NOT IMPLEMENTED */
    break;
  case eXZY:
    /* NOT IMPLEMENTED */
    break;
  case eYXZ:
    a = asin(matrix[6]);
    ca = cos(a);
    euler[0] = (vec_t)RAD2DEG(a);  /* Calculate X-axis angle */    

    if (fabs(ca) > 0.005) /* Gimbal lock? */
    {
      /* No, so get Y-axis angle */
      euler[1] = (vec_t)RAD2DEG(atan2(-matrix[2] / ca, matrix[10]/ ca));

      /* Get Z-axis angle */
      euler[2] = (vec_t)RAD2DEG(atan2(-matrix[4] / ca, matrix[5] / ca));
    }
    else /* Gimbal lock has occurred */
    {
      /* Set Z-axis angle to zero */
      euler[2]  = 0;

      /* And calculate Y-axis angle */
      euler[1] = (vec_t)RAD2DEG(atan2(matrix[8], matrix[0]));
    }
    break;
  case eZYX:
    a = asin(matrix[8]);
    ca = cos(a);
    euler[1] = (vec_t)RAD2DEG(a);  /* Calculate Y-axis angle */    

    if (fabs(ca) > 0.005) /* Gimbal lock? */
    {
      /* No, so get X-axis angle */
      euler[0] = (vec_t)RAD2DEG(atan2(-matrix[9] / ca, matrix[10]/ ca));

      /* Get Z-axis angle */
      euler[2] = (vec_t)RAD2DEG(atan2(-matrix[4] / ca, matrix[0] / ca));
    }
    else /* Gimbal lock has occurred */
    {
      /* Set X-axis angle to zero */
      euler[0]  = 0;

      /* And calculate Z-axis angle */
      euler[2] = (vec_t)RAD2DEG(atan2(matrix[1], matrix[5]));
    }
    break;
  }

  /* return only positive angles in [0,360] */
  if (euler[0] < 0) euler[0] += 360;
  if (euler[1] < 0) euler[1] += 360;
  if (euler[2] < 0) euler[2] += 360;
}

void m4x4_get_scale_vec3(const m4x4_t matrix, vec3_t scale)
{
  scale[0] = VectorLength(matrix+0);
  scale[1] = VectorLength(matrix+4);
  scale[2] = VectorLength(matrix+8);
}

void m4x4_get_transform_vec3(const m4x4_t matrix, vec3_t translation, vec3_t euler, eulerOrder_t order, vec3_t scale)
{
  m4x4_t normalised;
  m4x4_assign(normalised, matrix);
  scale[0] = VectorNormalize(normalised+0, normalised+0);
  scale[1] = VectorNormalize(normalised+4, normalised+4);
  scale[2] = VectorNormalize(normalised+8, normalised+8);
  if(m4x4_handedness(normalised) == eLeftHanded)
  {
    VectorNegate(normalised+0, normalised+0);
    VectorNegate(normalised+4, normalised+4);
    VectorNegate(normalised+8, normalised+8);
    scale[0] = -scale[0];
    scale[1] = -scale[1];
    scale[2] = -scale[2];
  }
  m4x4_get_rotation_vec3(normalised, euler, order);
  m4x4_get_translation_vec3(matrix, translation);
}

void m4x4_translate_by_vec3(m4x4_t matrix, const vec3_t translation)
{
  m4x4_t temp;
  m4x4_translation_for_vec3(temp, translation);
  m4x4_multiply_by_m4x4(matrix, temp);
}

void m4x4_rotate_by_vec3(m4x4_t matrix, const vec3_t euler, eulerOrder_t order)
{
  m4x4_t temp;
  m4x4_rotation_for_vec3(temp, euler, order);
  m4x4_multiply_by_m4x4(matrix, temp);
}

void m4x4_scale_by_vec3(m4x4_t matrix, const vec3_t scale)
{
  m4x4_t temp;
  m4x4_scale_for_vec3(temp, scale);
  m4x4_multiply_by_m4x4(matrix, temp);
}

void m4x4_rotate_by_quat(m4x4_t matrix, const vec4_t rotation)
{
  m4x4_t temp;
  m4x4_rotation_for_quat(temp, rotation);
  m4x4_multiply_by_m4x4(matrix, temp);
}

void m4x4_rotate_by_axisangle(m4x4_t matrix, const vec3_t axis, double angle)
{
  m4x4_t temp;
  m4x4_rotation_for_axisangle(temp, axis, angle);
  m4x4_multiply_by_m4x4(matrix, temp);
}

void m4x4_transform_by_vec3(m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale)
{
  m4x4_translate_by_vec3(matrix, translation);
  m4x4_rotate_by_vec3(matrix, euler, order);
  m4x4_scale_by_vec3(matrix, scale);
}

void m4x4_pivoted_rotate_by_vec3(m4x4_t matrix, const vec3_t euler, eulerOrder_t order, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;
  VectorNegate(pivotpoint, vec3_temp);

  m4x4_translate_by_vec3(matrix, pivotpoint);
  m4x4_rotate_by_vec3(matrix, euler, order);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

void m4x4_pivoted_scale_by_vec3(m4x4_t matrix, const vec3_t scale, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;
  VectorNegate(pivotpoint, vec3_temp);

  m4x4_translate_by_vec3(matrix, pivotpoint);
  m4x4_scale_by_vec3(matrix, scale);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

void m4x4_pivoted_transform_by_vec3(m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;

  VectorAdd(pivotpoint, translation, vec3_temp);
  m4x4_translate_by_vec3(matrix, vec3_temp);
  m4x4_rotate_by_vec3(matrix, euler, order);
  m4x4_scale_by_vec3(matrix, scale);
  VectorNegate(pivotpoint, vec3_temp);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

void m4x4_pivoted_transform_by_rotation(m4x4_t matrix, const vec3_t translation, const m4x4_t rotation, const vec3_t scale, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;

  VectorAdd(pivotpoint, translation, vec3_temp);
  m4x4_translate_by_vec3(matrix, vec3_temp);
  m4x4_multiply_by_m4x4(matrix, rotation);
  m4x4_scale_by_vec3(matrix, scale);
  VectorNegate(pivotpoint, vec3_temp);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

void m4x4_pivoted_rotate_by_quat(m4x4_t matrix, const vec4_t rotation, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;
  VectorNegate(pivotpoint, vec3_temp);

  m4x4_translate_by_vec3(matrix, pivotpoint);
  m4x4_rotate_by_quat(matrix, rotation);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

void m4x4_pivoted_rotate_by_axisangle(m4x4_t matrix, const vec3_t axis, double angle, const vec3_t pivotpoint)
{
  vec3_t vec3_temp;
  VectorNegate(pivotpoint, vec3_temp);

  m4x4_translate_by_vec3(matrix, pivotpoint);
  m4x4_rotate_by_axisangle(matrix, axis, angle);
  m4x4_translate_by_vec3(matrix, vec3_temp);
}

/*
A = A.B

A0 = B0 * A0 + B1 * A4 + B2 * A8 + B3 * A12
A4 = B4 * A0 + B5 * A4 + B6 * A8 + B7 * A12
A8 = B8 * A0 + B9 * A4 + B10* A8 + B11* A12
A12= B12* A0 + B13* A4 + B14* A8 + B15* A12

A1 = B0 * A1 + B1 * A5 + B2 * A9 + B3 * A13
A5 = B4 * A1 + B5 * A5 + B6 * A9 + B7 * A13
A9 = B8 * A1 + B9 * A5 + B10* A9 + B11* A13
A13= B12* A1 + B13* A5 + B14* A9 + B15* A13

A2 = B0 * A2 + B1 * A6 + B2 * A10+ B3 * A14
A6 = B4 * A2 + B5 * A6 + B6 * A10+ B7 * A14
A10= B8 * A2 + B9 * A6 + B10* A10+ B11* A14
A14= B12* A2 + B13* A6 + B14* A10+ B15* A14

A3 = B0 * A3 + B1 * A7 + B2 * A11+ B3 * A15
A7 = B4 * A3 + B5 * A7 + B6 * A11+ B7 * A15
A11= B8 * A3 + B9 * A7 + B10* A11+ B11* A15
A15= B12* A3 + B13* A7 + B14* A11+ B15* A15
*/

void m4x4_multiply_by_m4x4(m4x4_t dst, const m4x4_t src)
{
	vec_t dst0, dst1, dst2, dst3;

#if 1

  dst0 = src[0] * dst[0] + src[1] * dst[4] + src[2] * dst[8] + src[3] * dst[12];
  dst1 = src[4] * dst[0] + src[5] * dst[4] + src[6] * dst[8] + src[7] * dst[12];
  dst2 = src[8] * dst[0] + src[9] * dst[4] + src[10]* dst[8] + src[11]* dst[12];
  dst3 = src[12]* dst[0] + src[13]* dst[4] + src[14]* dst[8] + src[15]* dst[12];
  dst[0] = dst0; dst[4] = dst1; dst[8] = dst2; dst[12]= dst3;

  dst0 = src[0] * dst[1] + src[1] * dst[5] + src[2] * dst[9] + src[3] * dst[13];
  dst1 = src[4] * dst[1] + src[5] * dst[5] + src[6] * dst[9] + src[7] * dst[13];
  dst2 = src[8] * dst[1] + src[9] * dst[5] + src[10]* dst[9] + src[11]* dst[13];
  dst3 = src[12]* dst[1] + src[13]* dst[5] + src[14]* dst[9] + src[15]* dst[13];
  dst[1] = dst0; dst[5] = dst1; dst[9] = dst2; dst[13]= dst3;

  dst0 = src[0] * dst[2] + src[1] * dst[6] + src[2] * dst[10]+ src[3] * dst[14];
  dst1 = src[4] * dst[2] + src[5] * dst[6] + src[6] * dst[10]+ src[7] * dst[14];
  dst2 = src[8] * dst[2] + src[9] * dst[6] + src[10]* dst[10]+ src[11]* dst[14];
  dst3 = src[12]* dst[2] + src[13]* dst[6] + src[14]* dst[10]+ src[15]* dst[14];
  dst[2] = dst0; dst[6] = dst1; dst[10]= dst2; dst[14]= dst3;

  dst0 = src[0] * dst[3] + src[1] * dst[7] + src[2] * dst[11]+ src[3] * dst[15];
  dst1 = src[4] * dst[3] + src[5] * dst[7] + src[6] * dst[11]+ src[7] * dst[15];
  dst2 = src[8] * dst[3] + src[9] * dst[7] + src[10]* dst[11]+ src[11]* dst[15];
  dst3 = src[12]* dst[3] + src[13]* dst[7] + src[14]* dst[11]+ src[15]* dst[15];
  dst[3] = dst0; dst[7] = dst1; dst[11]= dst2; dst[15]= dst3;

#else

  vec_t * p = dst;
	for(int i=0;i<4;i++)
	{
		dst1 =  src[0]  * p[0];
		dst1 += src[1]  * p[4];
		dst1 += src[2]  * p[8];
		dst1 += src[3]  * p[12];
		dst2 =  src[4]  * p[0];
		dst2 += src[5]  * p[4];
		dst2 += src[6]  * p[8];
		dst2 += src[7]  * p[12];
		dst3 =  src[8]  * p[0];
		dst3 += src[9]  * p[4];
		dst3 += src[10] * p[8];
		dst3 += src[11] * p[12];
		dst4 =  src[12] * p[0];
		dst4 += src[13] * p[4];
		dst4 += src[14] * p[8];
		dst4 += src[15] * p[12];

		p[0] = dst1;
		p[4] = dst2;
		p[8] = dst3;
		p[12] = dst4;
    p++;
	}

#endif
}

/*
A = B.A

A0 = A0 * B0 + A1 * B4 + A2 * B8 + A3 * B12
A1 = A0 * B1 + A1 * B5 + A2 * B9 + A3 * B13
A2 = A0 * B2 + A1 * B6 + A2 * B10+ A3 * B14
A3 = A0 * B3 + A1 * B7 + A2 * B11+ A3 * B15

A4 = A4 * B0 + A5 * B4 + A6 * B8 + A7 * B12
A5 = A4 * B1 + A5 * B5 + A6 * B9 + A7 * B13
A6 = A4 * B2 + A5 * B6 + A6 * B10+ A7 * B14
A7 = A4 * B3 + A5 * B7 + A6 * B11+ A7 * B15

A8 = A8 * B0 + A9 * B4 + A10* B8 + A11* B12
A9 = A8 * B1 + A9 * B5 + A10* B9 + A11* B13
A10= A8 * B2 + A9 * B6 + A10* B10+ A11* B14
A11= A8 * B3 + A9 * B7 + A10* B11+ A11* B15

A12= A12* B0 + A13* B4 + A14* B8 + A15* B12
A13= A12* B1 + A13* B5 + A14* B9 + A15* B13
A14= A12* B2 + A13* B6 + A14* B10+ A15* B14
A15= A12* B3 + A13* B7 + A14* B11+ A15* B15
*/

void m4x4_premultiply_by_m4x4(m4x4_t dst, const m4x4_t src)
{
	vec_t dst0, dst1, dst2, dst3;

#if 1

  dst0 = dst[0] * src[0] + dst[1] * src[4] + dst[2] * src[8] + dst[3] * src[12];
  dst1 = dst[0] * src[1] + dst[1] * src[5] + dst[2] * src[9] + dst[3] * src[13];
  dst2 = dst[0] * src[2] + dst[1] * src[6] + dst[2] * src[10]+ dst[3] * src[14];
  dst3 = dst[0] * src[3] + dst[1] * src[7] + dst[2] * src[11]+ dst[3] * src[15];
  dst[0] = dst0; dst[1] = dst1; dst[2] = dst2; dst[3]= dst3;

  dst0 = dst[4] * src[0] + dst[5] * src[4] + dst[6] * src[8] + dst[7] * src[12];
  dst1 = dst[4] * src[1] + dst[5] * src[5] + dst[6] * src[9] + dst[7] * src[13];
  dst2 = dst[4] * src[2] + dst[5] * src[6] + dst[6] * src[10]+ dst[7] * src[14];
  dst3 = dst[4] * src[3] + dst[5] * src[7] + dst[6] * src[11]+ dst[7] * src[15];
  dst[4] = dst0; dst[5] = dst1; dst[6] = dst2; dst[7]= dst3;

  dst0 = dst[8] * src[0] + dst[9] * src[4] + dst[10]* src[8] + dst[11]* src[12];
  dst1 = dst[8] * src[1] + dst[9] * src[5] + dst[10]* src[9] + dst[11]* src[13];
  dst2 = dst[8] * src[2] + dst[9] * src[6] + dst[10]* src[10]+ dst[11]* src[14];
  dst3 = dst[8] * src[3] + dst[9] * src[7] + dst[10]* src[11]+ dst[11]* src[15];
  dst[8] = dst0; dst[9] = dst1; dst[10] = dst2; dst[11]= dst3;

  dst0 = dst[12]* src[0] + dst[13]* src[4] + dst[14]* src[8] + dst[15]* src[12];
  dst1 = dst[12]* src[1] + dst[13]* src[5] + dst[14]* src[9] + dst[15]* src[13];
  dst2 = dst[12]* src[2] + dst[13]* src[6] + dst[14]* src[10]+ dst[15]* src[14];
  dst3 = dst[12]* src[3] + dst[13]* src[7] + dst[14]* src[11]+ dst[15]* src[15];
  dst[12] = dst0; dst[13] = dst1; dst[14] = dst2; dst[15]= dst3;

#else

  vec_t* p = dst;
	for(int i=0;i<4;i++)
	{
		dst1 =  src[0]  * p[0];
		dst2 =  src[1]  * p[0];
		dst3 =  src[2]  * p[0];
		dst4 =  src[3]  * p[0];
		dst1 += src[4]  * p[1];
		dst2 += src[5]  * p[1];
		dst3 += src[6]  * p[1];
		dst4 += src[7]  * p[1];
		dst1 += src[8]  * p[2];
		dst2 += src[9]  * p[2];
		dst4 += src[11] * p[2];
		dst3 += src[10] * p[2];
		dst1 += src[12] * p[3];
		dst2 += src[13] * p[3];
		dst3 += src[14] * p[3];
		dst4 += src[15] * p[3];

		*p++ = dst1;
		*p++ = dst2;
		*p++ = dst3;
		*p++ = dst4;
	}

#endif
}

void m4x4_orthogonal_multiply_by_m4x4(m4x4_t dst, const m4x4_t src)
{
	vec_t dst0, dst1, dst2, dst3;

  dst0 = src[0] * dst[0] + src[1] * dst[4] + src[2] * dst[8];
  dst1 = src[4] * dst[0] + src[5] * dst[4] + src[6] * dst[8];
  dst2 = src[8] * dst[0] + src[9] * dst[4] + src[10]* dst[8];
  dst3 = src[12]* dst[0] + src[13]* dst[4] + src[14]* dst[8] + dst[12];
  dst[0] = dst0; dst[4] = dst1; dst[8] = dst2; dst[12]= dst3;

  dst0 = src[0] * dst[1] + src[1] * dst[5] + src[2] * dst[9];
  dst1 = src[4] * dst[1] + src[5] * dst[5] + src[6] * dst[9];
  dst2 = src[8] * dst[1] + src[9] * dst[5] + src[10]* dst[9];
  dst3 = src[12]* dst[1] + src[13]* dst[5] + src[14]* dst[9] + dst[13];
  dst[1] = dst0; dst[5] = dst1; dst[9] = dst2; dst[13]= dst3;

  dst0 = src[0] * dst[2] + src[1] * dst[6] + src[2] * dst[10];
  dst1 = src[4] * dst[2] + src[5] * dst[6] + src[6] * dst[10];
  dst2 = src[8] * dst[2] + src[9] * dst[6] + src[10]* dst[10];
  dst3 = src[12]* dst[2] + src[13]* dst[6] + src[14]* dst[10]+ dst[14];
  dst[2] = dst0; dst[6] = dst1; dst[10]= dst2; dst[14]= dst3;
}

void m4x4_orthogonal_premultiply_by_m4x4(m4x4_t dst, const m4x4_t src)
{
	vec_t dst0, dst1, dst2;

  dst0 = dst[0] * src[0] + dst[1] * src[4] + dst[2] * src[8];
  dst1 = dst[0] * src[1] + dst[1] * src[5] + dst[2] * src[9];
  dst2 = dst[0] * src[2] + dst[1] * src[6] + dst[2] * src[10];
  dst[0] = dst0; dst[1] = dst1; dst[2] = dst2;

  dst0 = dst[4] * src[0] + dst[5] * src[4] + dst[6] * src[8];
  dst1 = dst[4] * src[1] + dst[5] * src[5] + dst[6] * src[9];
  dst2 = dst[4] * src[2] + dst[5] * src[6] + dst[6] * src[10];
  dst[4] = dst0; dst[5] = dst1; dst[6] = dst2;

  dst0 = dst[8] * src[0] + dst[9] * src[4] + dst[10]* src[8];
  dst1 = dst[8] * src[1] + dst[9] * src[5] + dst[10]* src[9];
  dst2 = dst[8] * src[2] + dst[9] * src[6] + dst[10]* src[10];
  dst[8] = dst0; dst[9] = dst1; dst[10] = dst2;

  dst0 = dst[12]* src[0] + dst[13]* src[4] + dst[14]* src[8] + dst[15]* src[12];
  dst1 = dst[12]* src[1] + dst[13]* src[5] + dst[14]* src[9] + dst[15]* src[13];
  dst2 = dst[12]* src[2] + dst[13]* src[6] + dst[14]* src[10]+ dst[15]* src[14];
  dst[12] = dst0; dst[13] = dst1; dst[14] = dst2;
}

void m4x4_transform_point(const m4x4_t matrix, vec3_t point)
{
  float out1, out2, out3;

	out1 =  matrix[0]  * point[0] + matrix[4]  * point[1] + matrix[8]  * point[2] + matrix[12];
	out2 =  matrix[1]  * point[0] + matrix[5]  * point[1] + matrix[9]  * point[2] + matrix[13];
	out3 =  matrix[2]  * point[0] + matrix[6]  * point[1] + matrix[10] * point[2] + matrix[14];

	point[0] = out1;
	point[1] = out2;
	point[2] = out3;
}

void m4x4_transform_normal(const m4x4_t matrix, vec3_t normal)
{
  float out1, out2, out3;

	out1 =  matrix[0]  * normal[0] + matrix[4]  * normal[1] + matrix[8]  * normal[2];
	out2 =  matrix[1]  * normal[0] + matrix[5]  * normal[1] + matrix[9]  * normal[2];
	out3 =  matrix[2]  * normal[0] + matrix[6]  * normal[1] + matrix[10] * normal[2];

	normal[0] = out1;
	normal[1] = out2;
	normal[2] = out3;
}

void m4x4_transform_vec4(const m4x4_t matrix, vec4_t vector)
{
  float out1, out2, out3, out4;

	out1 =  matrix[0]  * vector[0] + matrix[4]  * vector[1] + matrix[8]  * vector[2] + matrix[12] * vector[3];
	out2 =  matrix[1]  * vector[0] + matrix[5]  * vector[1] + matrix[9]  * vector[2] + matrix[13] * vector[3];
	out3 =  matrix[2]  * vector[0] + matrix[6]  * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[3];
	out4 =  matrix[3]  * vector[0] + matrix[7]  * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[3];

	vector[0] = out1;
	vector[1] = out2;
	vector[2] = out3;
  vector[3] = out4;
}

#define CLIP_X_LT_W(p) ((p)[0] < (p)[3])
#define CLIP_X_GT_W(p) ((p)[0] > -(p)[3])
#define CLIP_Y_LT_W(p) ((p)[1] < (p)[3])
#define CLIP_Y_GT_W(p) ((p)[1] > -(p)[3])
#define CLIP_Z_LT_W(p) ((p)[2] < (p)[3])
#define CLIP_Z_GT_W(p) ((p)[2] > -(p)[3])

clipmask_t homogenous_clip_point(const vec4_t clipped)
{
  clipmask_t result = CLIP_FAIL;
  if(CLIP_X_LT_W(clipped)) result &= ~CLIP_LT_X; // X < W
  if(CLIP_X_GT_W(clipped)) result &= ~CLIP_GT_X; // X > -W
  if(CLIP_Y_LT_W(clipped)) result &= ~CLIP_LT_Y; // Y < W
  if(CLIP_Y_GT_W(clipped)) result &= ~CLIP_GT_Y; // Y > -W
  if(CLIP_Z_LT_W(clipped)) result &= ~CLIP_LT_Z; // Z < W
  if(CLIP_Z_GT_W(clipped)) result &= ~CLIP_GT_Z; // Z > -W
  return result;
}

clipmask_t m4x4_clip_point(const m4x4_t matrix, const vec3_t point, vec4_t clipped)
{
  clipped[0] = point[0];
  clipped[1] = point[1];
  clipped[2] = point[2];
  clipped[3] = 1;
  m4x4_transform_vec4(matrix, clipped);
  return homogenous_clip_point(clipped);
}


unsigned int homogenous_clip_triangle(vec4_t clipped[9])
{
  vec4_t buffer[9];
  unsigned int rcount = 3;
  unsigned int wcount = 0;
  vec_t const* rptr = clipped[0];
  vec_t* wptr = buffer[0];
  const vec_t* p0;
  const vec_t* p1;
  unsigned char b0, b1;

  unsigned int i;
  double scale;

  p0 = rptr;
  b0 = CLIP_X_LT_W(p0);
  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_X_LT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[0] - p0[3]) / (wptr[3] - wptr[0]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  rcount = wcount;
  wcount = 0;
  rptr = buffer[0];
  wptr = clipped[0];
  p0 = rptr;
  b0 = CLIP_X_GT_W(p0);

  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_X_GT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[0] + p0[3]) / (-wptr[3] - wptr[0]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  rcount = wcount;
  wcount = 0;
  rptr = clipped[0];
  wptr = buffer[0];
  p0 = rptr;
  b0 = CLIP_Y_LT_W(p0);

  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_Y_LT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[1] - p0[3]) / (wptr[3] - wptr[1]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  rcount = wcount;
  wcount = 0;
  rptr = buffer[0];
  wptr = clipped[0];
  p0 = rptr;
  b0 = CLIP_Y_GT_W(p0);

  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_Y_GT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[1] + p0[3]) / (-wptr[3] - wptr[1]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  rcount = wcount;
  wcount = 0;
  rptr = clipped[0];
  wptr = buffer[0];
  p0 = rptr;
  b0 = CLIP_Z_LT_W(p0);

  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_Z_LT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[2] - p0[3]) / (wptr[3] - wptr[2]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  rcount = wcount;
  wcount = 0;
  rptr = buffer[0];
  wptr = clipped[0];
  p0 = rptr;
  b0 = CLIP_Z_GT_W(p0);

  for(i=0; i<rcount; ++i)
  {
    p1 = (i+1 != rcount) ? p0 + 4 : rptr;
    b1 = CLIP_Z_GT_W(p1);
    if(b0 ^ b1)
    {
			wptr[0] = p1[0] - p0[0];
			wptr[1] = p1[1] - p0[1];
			wptr[2] = p1[2] - p0[2];
			wptr[3] = p1[3] - p0[3];

      scale = (p0[2] + p0[3]) / (-wptr[3] - wptr[2]);

			wptr[0] = (vec_t)(p0[0] + scale*(wptr[0]));
			wptr[1] = (vec_t)(p0[1] + scale*(wptr[1]));
			wptr[2] = (vec_t)(p0[2] + scale*(wptr[2]));
			wptr[3] = (vec_t)(p0[3] + scale*(wptr[3]));

      wptr += 4;
      ++wcount;
    }

    if(b1)
    {
      wptr[0] = p1[0];
      wptr[1] = p1[1];
      wptr[2] = p1[2];
      wptr[3] = p1[3];

      wptr += 4;
      ++wcount;
    }

    p0 = p1;
    b0 = b1;
  }

  return wcount;
}

unsigned int m4x4_clip_triangle(const m4x4_t matrix, const vec3_t p0, const vec3_t p1, const vec3_t p2, vec4_t clipped[9])
{
  clipped[0][0] = p0[0];
  clipped[0][1] = p0[1];
  clipped[0][2] = p0[2];
  clipped[0][3] = 1;
  clipped[1][0] = p1[0];
  clipped[1][1] = p1[1];
  clipped[1][2] = p1[2];
  clipped[1][3] = 1;
  clipped[2][0] = p2[0];
  clipped[2][1] = p2[1];
  clipped[2][2] = p2[2];
  clipped[2][3] = 1;

  m4x4_transform_vec4(matrix, clipped[0]);
  m4x4_transform_vec4(matrix, clipped[1]);
  m4x4_transform_vec4(matrix, clipped[2]);

  return homogenous_clip_triangle(clipped);
}

unsigned int homogenous_clip_line(vec4_t clipped[2])
{
  vec4_t clip;
  double scale;
  const vec_t* const p0 = clipped[0];
  const vec_t* const p1 = clipped[1];

  // early out
  {
    clipmask_t mask0 = homogenous_clip_point(clipped[0]);
    clipmask_t mask1 = homogenous_clip_point(clipped[1]);

    if((mask0 | mask1) == CLIP_PASS) // both points passed all planes
      return 2;

    if(mask0 & mask1) // both points failed any one plane
      return 0;
  }

  {
    const unsigned int index = CLIP_X_LT_W(p0);
    if(index ^ CLIP_X_LT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[0] - p0[3]) / (clip[3] - clip[0]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  {
    const unsigned int index = CLIP_X_GT_W(p0);
    if(index ^ CLIP_X_GT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[0] + p0[3]) / (-clip[3] - clip[0]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  {
    const unsigned int index = CLIP_Y_LT_W(p0);
    if(index ^ CLIP_Y_LT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[1] - p0[3]) / (clip[3] - clip[1]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  {
    const unsigned int index = CLIP_Y_GT_W(p0);
    if(index ^ CLIP_Y_GT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[1] + p0[3]) / (-clip[3] - clip[1]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  {
    const unsigned int index = CLIP_Z_LT_W(p0);
    if(index ^ CLIP_Z_LT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[2] - p0[3]) / (clip[3] - clip[2]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  {
    const unsigned int index = CLIP_Z_GT_W(p0);
    if(index ^ CLIP_Z_GT_W(p1))
    {
		  clip[0] = p1[0] - p0[0];
		  clip[1] = p1[1] - p0[1];
		  clip[2] = p1[2] - p0[2];
		  clip[3] = p1[3] - p0[3];

      scale = (p0[2] + p0[3]) / (-clip[3] - clip[2]);

		  clip[0] = (vec_t)(p0[0] + scale*(clip[0]));
		  clip[1] = (vec_t)(p0[1] + scale*(clip[1]));
		  clip[2] = (vec_t)(p0[2] + scale*(clip[2]));
		  clip[3] = (vec_t)(p0[3] + scale*(clip[3]));

      clipped[index][0] = clip[0];
      clipped[index][1] = clip[1];
      clipped[index][2] = clip[2];
      clipped[index][3] = clip[3];
    }
    else if(index == 0)
      return 0;
  }

  return 2;
}

unsigned int m4x4_clip_line(const m4x4_t matrix, const vec3_t p0, const vec3_t p1, vec4_t clipped[2])
{
  clipped[0][0] = p0[0];
  clipped[0][1] = p0[1];
  clipped[0][2] = p0[2];
  clipped[0][3] = 1;
  clipped[1][0] = p1[0];
  clipped[1][1] = p1[1];
  clipped[1][2] = p1[2];
  clipped[1][3] = 1;

  m4x4_transform_vec4(matrix, clipped[0]);
  m4x4_transform_vec4(matrix, clipped[1]);

  return homogenous_clip_line(clipped);
}

void m4x4_transpose(m4x4_t matrix)
{
	int i, j;
	float temp, *p1, *p2;

  for (i=1; i<4; i++) {
    for (j=0; j<i; j++) {
      p1 = matrix+(j*4+i);
      p2 = matrix+(i*4+j);
      temp = *p1;
      *p1=*p2;
      *p2=temp;
    }
  }
}

/* adapted from Graphics Gems 2
 invert a 3d matrix (4x3) */
int m4x4_orthogonal_invert(m4x4_t matrix)
{
  m4x4_t temp;
  vec_t* src = temp;

  m4x4_assign(src, matrix);
  
  /* Calculate the determinant of upper left 3x3 submatrix and
  * determine if the matrix is singular.
  */
  {
#if 0
  float pos = 0.0f;
  float neg = 0.0f;
  float det = src[0] * src[5] * src[10];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = src[1] * src[6] * src[8];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = src[2] * src[4] * src[9];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = -src[2] * src[5] * src[8];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = -src[1] * src[4] * src[10];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = -src[0] * src[6] * src[9];
  if (det >= 0.0) pos += det; else neg += det;
  
  det = pos + neg;
#elif 0
  float det
    = (src[0] * src[5] * src[10])
    + (src[1] * src[6] * src[8])
    + (src[2] * src[4] * src[9])
    - (src[2] * src[5] * src[8])
    - (src[1] * src[4] * src[10])  
    - (src[0] * src[6] * src[9]);
#else
  float det
    = src[0] * ( src[5]*src[10] - src[9]*src[6] )
    - src[1] * ( src[4]*src[10] - src[8]*src[6] )
    + src[2] * ( src[4]*src[9] - src[8]*src[5] );

#endif
  
  if (det*det < 1e-25)
    return 1;
  
  det = 1.0f / det;
  matrix[0] = (  (src[5]*src[10]- src[6]*src[9] )*det);
  matrix[1] = (- (src[1]*src[10]- src[2]*src[9] )*det);
  matrix[2] = (  (src[1]*src[6] - src[2]*src[5] )*det);
  matrix[4] = (- (src[4]*src[10]- src[6]*src[8] )*det);
  matrix[5] = (  (src[0]*src[10]- src[2]*src[8] )*det);
  matrix[6] = (- (src[0]*src[6] - src[2]*src[4] )*det);
  matrix[8] = (  (src[4]*src[9] - src[5]*src[8] )*det);
  matrix[9] = (- (src[0]*src[9] - src[1]*src[8] )*det);
  matrix[10]= (  (src[0]*src[5] - src[1]*src[4] )*det);
  }
  
  /* Do the translation part */
  matrix[12] = - (src[12] * matrix[0] +
    src[13] * matrix[4] +
    src[14] * matrix[8]);
  matrix[13] = - (src[12] * matrix[1] +
    src[13] * matrix[5] +
    src[14] * matrix[9]);
  matrix[14] = - (src[12] * matrix[2] +
    src[13] * matrix[6] +
    src[14] * matrix[10]);
  
  return 0;
}

void quat_identity(vec4_t quat)
{
  quat[0] = quat[1] = quat[2] = 0;
  quat[3] = 1;
}

void quat_multiply_by_quat(vec4_t quat, const vec4_t other)
{
  const vec_t x = quat[3]*other[0] + quat[0]*other[3] + quat[1]*other[2] - quat[2]*other[1];
  const vec_t y = quat[3]*other[1] + quat[1]*other[3] + quat[2]*other[0] - quat[0]*other[2];
  const vec_t z = quat[3]*other[2] + quat[2]*other[3] + quat[0]*other[1] - quat[1]*other[0];
  const vec_t w = quat[3]*other[3] - quat[0]*other[0] - quat[1]*other[1] - quat[2]*other[2];
  quat[0] = x;
  quat[1] = y;
  quat[2] = z;
  quat[3] = w;
}

void quat_conjugate(vec4_t quat)
{
  VectorNegate(quat, quat);
}

//! quaternion from two unit vectors
void quat_for_unit_vectors(vec4_t quat, const vec3_t from, const vec3_t to)
{
  CrossProduct(from, to, quat);
  quat[3] = DotProduct(from, to);
}

void quat_normalise(vec4_t quat)
{
  const vec_t n = 1 / ( quat[0] * quat[0] +  quat[1] * quat[1] +  quat[2] * quat[2] +  quat[3] *  quat[3] );
  quat[0] *= n;
  quat[1] *= n;
  quat[2] *= n;
  quat[3] *= n;
}

void quat_for_axisangle(vec4_t quat, const vec3_t axis, double angle)
{
  angle *= 0.5;

  quat[3] = (float)sin(angle);

  quat[0] = axis[0] * quat[3];
  quat[1] = axis[1] * quat[3];
  quat[2] = axis[2] * quat[3];
  quat[3] = (float)cos(angle);
}

void m3x3_multiply_by_m3x3(m3x3_t matrix, const m3x3_t matrix_src)
{
  float *pDest = matrix;
	float out1, out2, out3;
  int i;

	for(i=0;i<3;i++)
	{
		out1 =  matrix_src[0] * pDest[0];
		out1 += matrix_src[1] * pDest[3];
		out1 += matrix_src[2] * pDest[6];
		out2 =  matrix_src[3] * pDest[0];
		out2 += matrix_src[4] * pDest[3];
		out2 += matrix_src[5] * pDest[6];
		out3 =  matrix_src[6] * pDest[0];
		out3 += matrix_src[7] * pDest[3];
		out3 += matrix_src[8] * pDest[6];

		pDest[0] = out1;
		pDest[3] = out2;
		pDest[6] = out3;

    pDest++;
	}
}

void m3x3_transform_vec3(const m3x3_t matrix, vec3_t vector)
{
  float out1, out2, out3;

	out1 =  matrix[0]  * vector[0];
	out1 += matrix[3]  * vector[1];
	out1 += matrix[6]  * vector[2];
	out2 =  matrix[1]  * vector[0];
	out2 += matrix[4]  * vector[1];
	out2 += matrix[7]  * vector[2];
	out3 =  matrix[2]  * vector[0];
	out3 += matrix[5]  * vector[1];
	out3 += matrix[8] * vector[2];

	vector[0] = out1;
	vector[1] = out2;
	vector[2] = out3;
}

float m3_det( m3x3_t mat )
{
  float det;
  
  det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
    - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
    + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );
  
  return( det );
}

int m3_inverse( m3x3_t mr, m3x3_t ma )
{
  float det = m3_det( ma );
 
  if (det == 0 )
  {
    return 1;
  }

  
  mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
  mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
  mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;
  
  mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
  mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
  mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;
  
  mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
  mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
  mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;

  return 0;
}

void m4_submat( m4x4_t mr, m3x3_t mb, int i, int j )
{
  int ti, tj, idst, jdst;
  
  for ( ti = 0; ti < 4; ti++ )
  {
    if ( ti < i )
      idst = ti;
    else
      if ( ti > i )
        idst = ti-1;
      
      for ( tj = 0; tj < 4; tj++ )
      {
        if ( tj < j )
          jdst = tj;
        else
          if ( tj > j )
            jdst = tj-1;
          
          if ( ti != i && tj != j )
            mb[idst*3 + jdst] = mr[ti*4 + tj ];
      }
  }
}

float m4_det( m4x4_t mr )
{
  float  det, result = 0, i = 1;
  m3x3_t msub3;
  int     n;
  
  for ( n = 0; n < 4; n++, i *= -1 )
  {
    m4_submat( mr, msub3, 0, n );
    
    det     = m3_det( msub3 );
    result += mr[n] * det * i;
  }
  
  return result;
}

int m4x4_invert(m4x4_t matrix)
{
  float  mdet = m4_det( matrix );
  m3x3_t mtemp;
  int     i, j, sign;
  m4x4_t m4x4_temp;
  
#if 0
  if ( fabs( mdet ) < 0.0000000001 )
    return 1;
#endif

  m4x4_assign(m4x4_temp, matrix);
  
  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 4; j++ )
    {
      sign = 1 - ( (i +j) % 2 ) * 2;
      
      m4_submat( m4x4_temp, mtemp, i, j );
      
      matrix[i+j*4] = ( m3_det( mtemp ) * sign ) / mdet; /*  FIXME: try using * inverse det and see if speed/accuracy are good enough */
    }
    
  return 0;
}
#if 0
void m4x4_solve_ge(m4x4_t matrix, vec4_t x)
{
  int indx[4];
  int c,r;
  int i;
  int best;
  float scale[4];
  float f, pivot;
  float aug[4];
  float recip, ratio;
  float* p;

  for(r=0; r<4; r++)
  {
    aug[r] = 0;
    indx[r] = r;
  }

  for (r=0; r<4; r++)
  {
    scale[r] = 0;
    for (c=0; c<4; c++, p++)
    {
      if (fabs(*p) > scale[r])
      {
        scale[r] = (float)fabs(*p);
      }
    }
  }

  for (c=0; c<3; c++)
  {
    pivot = 0;
    for (r=c; r<4; r++)
    {
      f = (float)fabs(matrix[(indx[r]<<2)+c]) / scale[indx[r]];
      if (f > pivot)
      {
        pivot = f;
        best = r;
      }
    }

    i = indx[c];
    indx[c] = indx[best];
    indx[best] = i;

    recip = 1 / matrix[(indx[c]<<2)+c];

    for (r=c+1; r<4; r++)
    {
      p = matrix + (indx[r]<<2);
      ratio = p[c] * recip;

      for (i=c+1; i<4; i++)
        p[i] -= ratio * matrix[(indx[c]<<2)+i];
      aug[indx[r]] -= ratio * aug[indx[c]];
    }
  }

  x[indx[3]] = aug[indx[3]] / matrix[(indx[3]<<2)+3];
  for(r=2; r>=0; r--)
  {
    f = aug[indx[r]];
    p = matrix + (indx[r]<<2);
    recip = 1 / p[r];
    for(c=(r+1); c<4; c++)
    {
      f -= (p[c] * x[indx[c]]);
    }
    x[indx[r]] = f * recip;
  }
}
#endif

#define N 3

int matrix_solve_ge(vec_t* matrix, vec_t* aug, vec3_t x)
{
  int indx[N];
  int c,r;
  int i;
  int best;
  float scale[N];
  float f, pivot;
  float ratio;
  float* p;

  for(r=0; r<N; r++)
  {
    indx[r] = r;
  }

  for (r=0; r<N; r++)
  {
    p = matrix+r;
    scale[r] = 0;
    for (c=0; c<N; c++, p++)
    {
      if (fabs(*p) > scale[r])
      {
        scale[r] = (float)fabs(*p);
      }
    }
  }

  for (c=0; c<N; c++)
  {
    pivot = 0;
    best = -1;
    for (r=c; r<N; r++)
    {
      f = (float)fabs(matrix[(indx[r]*N)+c]) / scale[indx[r]];
      if (f > pivot)
      {
        pivot = f;
        best = r;
      }
    }

    if(best == -1) return 1;

    i = indx[c];
    indx[c] = indx[best];
    indx[best] = i;

    for (r=c+1; r<N; r++)
    {
      p = matrix + (indx[r]*N);
      ratio = p[c] / matrix[(indx[c]*N)+c];

      for (i=c+1; i<N; i++) p[i] -= ratio * matrix[(indx[c]*N)+i];
      aug[indx[r]] -= ratio * aug[indx[c]];
    }
  }

  x[N-1] = aug[indx[N-1]] / matrix[(indx[N-1]*N)+N-1];
  for(r=1; r>=0; r--)
  {
    f = aug[indx[r]];
    p = matrix + (indx[r]*N);
    for(c=(r+1); c<N; c++) f -= (p[c] * x[c]);
    x[r] = f / p[r];
  }
  return 0;
}

#ifdef YOU_WANT_IT_TO_BORK
 /* Gaussian elimination */
  for(i=0;i<4;i++)
  {
    for(j=(i+1);j<4;j++)
    {
      ratio = matrix[j][i] / matrix[i][i];
      for(count=i;count<n;count++) {
        matrix[j][count] -= (ratio * matrix[i][count]);
      }
      b[j] -= (ratio * b[i]);
    }
  }

  /* Back substitution */
  x[n-1] = b[n-1] / matrix[n-1][n-1];
  for(i=(n-2);i>=0;i--)
  {
    temp = b[i];
    for(j=(i+1);j<n;j++)
    {
      temp -= (matrix[i][j] * x[j]);
    }
    x[i] = temp / matrix[i][i];
  }
#endif

int plane_intersect_planes(const vec4_t plane1, const vec4_t plane2, const vec4_t plane3, vec3_t intersection)
{
  m3x3_t planes;
  vec3_t b;
  VectorCopy(plane1, planes+0);
  b[0] = plane1[3];
  VectorCopy(plane2, planes+3);
  b[1] = plane2[3];
  VectorCopy(plane3, planes+6);
  b[2] = plane3[3];

  return matrix_solve_ge(planes, b, intersection);
}
