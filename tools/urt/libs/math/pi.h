
#if !defined(INCLUDED_MATH_PI_H)
#define INCLUDED_MATH_PI_H

/// \file
/// \brief Pi constants and degrees/radians conversion.

const double c_pi = 3.1415926535897932384626433832795;
const double c_half_pi = c_pi / 2;
const double c_2pi = 2 * c_pi;
const double c_inv_2pi = 1 / c_2pi;

const double c_DEG2RADMULT = c_pi / 180.0;
const double c_RAD2DEGMULT = 180.0 / c_pi;

inline double radians_to_degrees(double radians)
{
  return radians * c_RAD2DEGMULT;
}
inline double degrees_to_radians(double degrees)
{
  return degrees * c_DEG2RADMULT;
}

#endif
