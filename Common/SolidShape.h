#pragma once

// Object Data
#include "Shape.h"

// Draw object using triangles
class SolidShape : public Shape {
 public:
  // Constructor
  //   size : dimension of positions of vertex
  //   vertexcount : number of vertices
  //   vertex : array contained vertex attributes
  SolidShape (GLint size, GLsizei vertexcount, const Object::Vertex *vertex)
    : Shape(size, vertexcount, vertex) {
  }

  // Execute Drawing
  virtual void execute() const {
    // Draw object using triangles
    glDrawArrays(GL_TRIANGLES, 0, vertexcount);
  }
};
