// We usually uses glm library for manipulating transformation matrix
#pragma once
#include <cmath>
#include <iostream>
#include <algorithm>
#include <GL/glew.h>

// Transformation matrix
class Matrix {
  // Elements of transformation matrix
  GLfloat matrix[16];

 public:
  // Default constructor
  Matrix () {}

  // Constructor for initializing with array
  //   a : array having 16 elements with GLfloat
  Matrix (const GLfloat* a) {
    std::copy(a, a + 16, matrix);
  }

  // Multiplication
  Matrix operator* (const Matrix& m) const {
    Matrix t;

    for (int i = 0; i < 16; ++i) {
      const int j(i & 3), k(i & ~3);

      t.matrix[i] =
	matrix[ 0 + j] * m.matrix[k + 0] +
	matrix[ 4 + j] * m.matrix[k + 1] +
	matrix[ 8 + j] * m.matrix[k + 2] +
	matrix[12 + j] * m.matrix[k + 3];
    }

    return t;
  }
  
  // Return array of transformation matrix
  const GLfloat* data () const {
    return matrix;
  }

  // Compute transformation matrix for normal vector
  void getNormalMatrix(GLfloat* m) const {
    m[0] = matrix[ 5]*matrix[10] - matrix[ 6]*matrix[ 9];
    m[1] = matrix[ 6]*matrix[ 8] - matrix[ 4]*matrix[10];
    m[2] = matrix[ 4]*matrix[ 9] - matrix[ 5]*matrix[ 8];
    m[3] = matrix[ 9]*matrix[ 2] - matrix[10]*matrix[ 1];
    m[4] = matrix[10]*matrix[ 0] - matrix[ 8]*matrix[ 2];
    m[5] = matrix[ 8]*matrix[ 1] - matrix[ 9]*matrix[ 0];
    m[6] = matrix[ 1]*matrix[ 6] - matrix[ 2]*matrix[ 5];
    m[7] = matrix[ 2]*matrix[ 4] - matrix[ 0]*matrix[ 6];
    m[8] = matrix[ 0]*matrix[ 5] - matrix[ 1]*matrix[ 4];
  }
  
  // Set identity matrix
  void loadIdentity () {
    std::fill(matrix, matrix + 16, 0.0f);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
  }

  // Generate identity matrix
  static Matrix identity () {
    Matrix t;
    t.loadIdentity();
    return t;
  }

  // Generate transformation matrix for translation with (x, y, z)
  static Matrix translate (GLfloat x, GLfloat y, GLfloat z) {
    Matrix t;

    t.loadIdentity();
    t.matrix[12] = x;
    t.matrix[13] = y;
    t.matrix[14] = z;

    return t;
  }

  // Generate transformation matrix for scaling with scale factor (x, y, z)
  static Matrix scale (GLfloat x, GLfloat y, GLfloat z) {
    Matrix t;

    t.loadIdentity();
    t.matrix[ 0] = x;
    t.matrix[ 5] = y;
    t.matrix[10] = z;

    return t;
  }

  // Generate transformation matrix for rotation with angle a and axis (x, y, z)
  static Matrix rotate (GLfloat a, GLfloat x, GLfloat y, GLfloat z) {
    Matrix t;
    const GLfloat d(sqrt(x*x + y*y + z*z));
    
    if (d > 0.0f) {
      const GLfloat l (x/d), m (y/d), n (z/d);
      const GLfloat l2(l*l), m2(m*m), n2(n*n);
      const GLfloat lm(l*m), mn(m*n), nl(n*l);

      const GLfloat c(cos(a)), s(sin(a)), c1(1.0f - c);

      t.loadIdentity();
      t.matrix[ 0] = (1.0f - l2)*c + l2;
      t.matrix[ 1] = lm*c1 + n*s;
      t.matrix[ 2] = nl*c1 - m*s;


      t.matrix[ 4] = lm*c1 - n*s;
      t.matrix[ 5] = (1.0f - m2)*c + m2;
      t.matrix[ 6] = mn*c1 + l*s;

      t.matrix[ 8] = nl*c1 + m*s;
      t.matrix[ 9] = mn*c1 - l*s;
      t.matrix[10] = (1.0f - n2)*c + n2;
    } else {
      std::cerr << "Error: Invalid argument." << std::endl;
      exit(1);
    }
    
    return t;
  }

  // Generate matrix for view transformation
  static Matrix lookat (GLfloat ex, GLfloat ey, GLfloat ez,   // position of view point
			GLfloat gx, GLfloat gy, GLfloat gz,   // position of target point
			GLfloat ux, GLfloat uy, GLfloat uz) { // vector of up direction
    // Transformation matrix for translation
    const Matrix tv(translate(-ex, -ey, -ez));

    // t-axis = e - g
    const GLfloat tx(ex - gx);
    const GLfloat ty(ey - gy);
    const GLfloat tz(ez - gz);

    // r-axis = u x t-axis
    const GLfloat rx(uy*tz - uz*ty);
    const GLfloat ry(uz*tx - ux*tz);
    const GLfloat rz(ux*ty - uy*tx);

    // s-axis = t-axis x r-axis
    const GLfloat sx(ty*rz - tz*ry);
    const GLfloat sy(tz*rx - tx*rz);
    const GLfloat sz(tx*ry - ty*rx);

    // Check norm of s-axis
    const GLfloat s2(sx*sx + sy*sy + sz*sz);
    if (s2 == 0.0f) {
      return tv;
    }

    // Transformation matrix for rotation
    Matrix rv;
    rv.loadIdentity();

    // Set elements after normalize r-axis
    const GLfloat r(sqrt(rx*rx + ry*ry + rz*rz));
    rv.matrix[ 0] = rx/r;
    rv.matrix[ 4] = ry/r;
    rv.matrix[ 8] = rz/r;

    // Set elements after normalize s-axis
    const GLfloat s(sqrt(s2));
    rv.matrix[ 1] = sx/s;
    rv.matrix[ 5] = sy/s;
    rv.matrix[ 9] = sz/s;

    // Set elements after normalize t-axis
    const GLfloat t(sqrt(tx*tx + ty*ty + tz*tz));
    rv.matrix[ 2] = tx/t;
    rv.matrix[ 6] = ty/t;
    rv.matrix[10] = tz/t;

    // Multiply rotation matrix to translation matrix
    return rv * tv;
  }

  // Generate transformation matrix for orthogonal projection
  static Matrix orthogonal (GLfloat   left, GLfloat right,
			    GLfloat bottom, GLfloat   top,
			    GLfloat  zNear, GLfloat  zFar) {
    Matrix t;
    const GLfloat dx(right - left);
    const GLfloat dy(top - bottom);
    const GLfloat dz(zFar - zNear);

    if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
      t.loadIdentity();
      t.matrix[ 0] =  2.0f / dx;
      t.matrix[ 5] =  2.0f / dy;
      t.matrix[10] = -2.0f / dz;

      t.matrix[12] = -(right + left) / dx;
      t.matrix[13] = -(top + bottom) / dy;
      t.matrix[14] = -(zFar + zNear) / dz;
    } else {
      std::cerr << "Error: Invalid argument." << std::endl;
      exit(1);
    }

    return t;
  }

  // Generate transformation matrix for perspective projection
  static Matrix frustum (GLfloat   left, GLfloat right,
			 GLfloat bottom, GLfloat   top,
			 GLfloat  zNear, GLfloat  zFar) {
    Matrix t;
    const GLfloat dx(right - left);
    const GLfloat dy(top - bottom);
    const GLfloat dz(zFar - zNear);

    if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
      t.loadIdentity();
      t.matrix[ 0] =  2.0f * zNear / dx;
      t.matrix[ 5] =  2.0f * zNear / dy;

      t.matrix[ 8] =  (right + left) / dx;
      t.matrix[ 9] =  (top + bottom) / dy;
      t.matrix[10] = -(zFar + zNear) / dz;

      t.matrix[11] = -1.0f;
      t.matrix[14] = -2.0f * zFar * zNear / dz;
      t.matrix[15] =  0.0f;
    } else {
      std::cerr << "Error: Invalid argument." << std::endl;
      exit(1);
    }

    return t;
  }

  // Generate transformation matrix for perspective projection using fovy and aspect ratio
  static Matrix perspective (GLfloat  fovy, GLfloat aspect,
			     GLfloat zNear, GLfloat   zFar) {
    Matrix t;
    const GLfloat dz(zFar - zNear);

    if (dz != 0.0f) {
      t.loadIdentity();
      t.matrix[ 5] =  1.0f / tan(fovy * 0.5f);
      t.matrix[ 0] =  t.matrix[5] / aspect;

      t.matrix[10] = -(zFar + zNear) / dz;
      t.matrix[11] = -1.0f;
      t.matrix[14] = -2.0f * zFar * zNear / dz;
      t.matrix[15] =  0.0f;
    } else {
      std::cerr << "Error: Invalid argument." << std::endl;
      exit(1);
    }

    return t;
  }
};
