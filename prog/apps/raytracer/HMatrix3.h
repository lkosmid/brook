#ifndef HMATRIX3_H
#define HMATRIX3_H

#include "Types.h"
#include <math.h>

#define FWD 0
#define BACK 1

class HMatrix3 {
public:
  HMatrix3();
  ~HMatrix3();

  void setIdentity();
  Tuple3f apply(const Tuple3f &p);
  static HMatrix3 multiply(const HMatrix3 &m1, const HMatrix3 &m2);
  void translate(float tx, float ty, float tz, int dir);
  void scale(float sx, float sy, float sz, int dir);
  void rotatex(float theta, int dir);
  void rotatey(float theta, int dir);
  void rotatez(float theta, int dir);
  void rotate(const Vector3 &ax, float theta, int dir);
  static HMatrix3 Transpose(HMatrix3 M);

  float data[4][4];
};

#endif /* HMATRIX3_H */
