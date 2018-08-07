#pragma once

// Object Data
#include "Shape.h"

// Draw object using index
class ShapeIndex : public Shape {
 protected:
  // Number of used vertices
  const GLsizei indexcount;

 public:
  // Constructor
  //   size : dimension of positions of vertex
  //   vertexcount : number of vertices
  //   vertex : array contained vertex attributes
  //   indexcount : number of indexes of vertices
  //   index : array contained indexes of vertices
  ShapeIndex (GLint size, GLsizei vertexcount, const Object::Vertex *vertex,
	      GLsizei indexcount, const GLuint* index)
    : Shape(size, vertexcount, vertex, indexcount, index),
    indexcount(indexcount) {
    }
  
  // Execute Drawing
  virtual void execute() const {
    // Draw line segment group
    glDrawElements(GL_LINES, indexcount, GL_UNSIGNED_INT, 0);
  }
};
