#pragma once

// Object using index
#include "ShapeIndex.h"

// Draw object utilizing triangle by using index
class SolidShapeIndex : public ShapeIndex {
 public:
  // Constructor
  //   size : dimension of positions of vertex
  //   vertexcount : number of vertices
  //   vertex : array contained vertex attributes
  //   indexcount : number of indexes of vertices
  //   index : array contained indexes of vertices
  SolidShapeIndex (GLint size, GLsizei vertexcount, const Object::Vertex *vertex,
		   GLsizei indexcount, const GLuint* index)
    : ShapeIndex(size, vertexcount, vertex, indexcount, index) {
  }
  
  // Execute Drawing
  virtual void execute() const {
    // Draw triangles
    glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, 0);
  }
};
