#include "math_utilities.h"

// System includes
#include "math.h"

/*
 * Returns atan2(y, x) where
 *  0 is x == 1, y == 0,
 *  Pi/2 is x == 0, y == 1
 *  Pi is x == -1, y == 0
 *
 *  Angles where y < 0 are rotated by Pi until they are in the region above.
 * */
double atan2upperHalfPlane(double y, double x)
{
  double phi = atan2(y, x);
  while (phi < 0) {
    phi += M_PI;
  }
  return phi;
}

/*
 * Returns atan2(y, x) as defined by
 *  http://en.cppreference.com/w/cpp/numeric/math/atan2
 * except always a positive angle.
 *
 * 0 is x == 1, y == 0
 * Pi/2 is x == 0, y == 1
 * Pi is x == -1, y == 0
 * 3 Pi/2 is x == 0, y == -1
 *  and so on.
 * */
double atan2Positive(double y, double x)
{
  double phi = atan2(y, x);
  while (phi < 0) {
    phi += 2 * M_PI;
  }
  return phi;
}
