#pragma once
#include <array>

// Transformation matrix
#include "Matrix.h"

// Vector
typedef std::array<GLfloat, 4> Vector;

// Multiplication of matrix and vector
//   m : Matrix
//   v : Vector
Vector operator* (const Matrix& m, const Vector& v) {
  Vector t;

  for (int i = 0; i < 4; ++i) {
    t[i] =
      m.data()[ 0 + i] * v[0] +
      m.data()[ 4 + i] * v[1] +
      m.data()[ 8 + i] * v[2] +
      m.data()[12 + i] * v[3];
  }

  return t;
}
