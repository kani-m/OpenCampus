#pragma once
#include <memory>

// Object Data
#include "Object.h"

// Draw object
class Shape {
  std::shared_ptr<const Object> object;

 protected:
  // Number of vertices used by drawing
  const GLsizei vertexcount;

 public:
  // Constructor
  //   size : dimension of positions of vertex
  //   vertexcount : number of vertices
  //   vertex : array contained vertex attributes
  //   indexcount : number of indexes of vertices
  //   index : array contained indexes of vertices
  Shape (GLint size, GLsizei vertexcount, const Object::Vertex *vertex,
	 GLsizei indexcount = 0, const GLuint* index = NULL)
    : object(new Object(size, vertexcount, vertex, indexcount, index)),
    vertexcount(vertexcount) {
    }

  // Drawing
  void draw () const {
    // Bind VAO
    object->bind();

    // Execute Drawing
    execute();
  }

  // Execute Drawing
  virtual void execute() const {
    // Draw polygonal line
    glDrawArrays(GL_LINE_LOOP, 0, vertexcount);
  }
};
