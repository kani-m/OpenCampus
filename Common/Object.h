#pragma once
#include <GL/glew.h>

// Object Data
class Object {
  // Vertex Array Object
  GLuint vao;
  
  // Vertex Buffer Object
  GLuint vbo;

  // Vertex Buffer Object of index
  GLuint ibo;
  
 public:
  // Vertex Attribute
  struct Vertex {
    // Position
    GLfloat position[3];

    // Color
    GLfloat color[3];

    // Normal
    GLfloat normal[3];
  };

  // Constructor
  //   size : dimension of positions of vertex
  //   vertexcount : number of vertices
  //   vertex : array contained vertex attributes
  //   indexcount : number of indexes of vertices
  //   index : array contained indexes of vertices
  Object (GLint size, GLsizei vertexcount, const Vertex *vertex,
	  GLsizei indexcount = 0, const GLuint* index = NULL) {
    // Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexcount * sizeof(Vertex), vertex, GL_STATIC_DRAW);

    // Enable to refer from in variable to binded VBO
    glVertexAttribPointer(0, size, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<Vertex*>(0)->position);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<Vertex*>(0)->color);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<Vertex*>(0)->normal);
    glEnableVertexAttribArray(2);
    
    // Create VBO of index
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexcount * sizeof(GLuint), index, GL_STATIC_DRAW);
  }

  // Destructor
  virtual ~Object () {
    // Destroy VAO
    glDeleteBuffers(1, &vao);

    // Destroy VBO
    glDeleteBuffers(1, &vbo);

    // Destroy VBO of index
    glDeleteBuffers(1, &ibo);
  }

 private:
  // Disable to copy by using copy constructor
  Object (const Object &o);

  // Disable to copy by using assignment statement
  Object& operator= (const Object& o);

 public:
  // Bind VAO
  void bind () const {
    // Designate drawn VAO
    glBindVertexArray(vao);
  }
};
