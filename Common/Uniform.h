#pragma once
#include <memory>
#include <GL/glew.h>

// Uniform Buffer Object
template <typename T>
class Uniform {
  struct UniformBuffer {
    // Uniform Buffer Object
    GLuint ubo;

    // Size of uniform block
    GLsizeiptr blocksize;
    
    // Constructor
    //   data : data to be stored in uniform block
    //   count : number of uniform block
    UniformBuffer (const T* data, unsigned int count) {
      // Compute size of uniform block
      GLint alignment;
      glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
      blocksize = (((sizeof(T) - 1) / alignment) + 1) * alignment;
      
      // Create UBO
      glGenBuffers(1, &ubo);
      glBindBuffer(GL_UNIFORM_BUFFER, ubo);
      glBufferData(GL_UNIFORM_BUFFER, count * blocksize, NULL, GL_STATIC_DRAW);
      for (unsigned int i = 0; i < count; ++i) {
	glBufferSubData(GL_UNIFORM_BUFFER, i * blocksize, sizeof(T), data + i);
      }
    }

    // Destructor
    ~UniformBuffer () {
      // Destroy UBO
      glDeleteBuffers(1, &ubo);
    }
  };

  // Buffer object
  const std::shared_ptr<const UniformBuffer> buffer;

 public:
  // Constructor
  //   data : data to be stored in uniform block
  //   count : number of uniform block
  Uniform (const T* data = NULL, unsigned int count = 1)
    : buffer(new UniformBuffer(data, count)) {
  }

  // Destructor
  virtual ~Uniform () {
  }

  // Store data into UBO
  //   data : data to be stored in uniform block
  //   start : top position of uniform block storing data
  //   count : number of uniform block
  void set (const T* data, unsigned int start = 0, unsigned int count = 1) const {
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
    for (unsigned int i = 0; i < count; ++i) {
      glBufferSubData(GL_UNIFORM_BUFFER, (start + i) * buffer->blocksize, sizeof(T), data + i);
    }
  }

  // Use this UBO
  //   i : position of binding uniform block
  //   bp : bounding point
  void select (unsigned int i = 0, GLuint bp = 0) const {
    // Designate UBO configuring material
    glBindBufferRange(GL_UNIFORM_BUFFER, bp, buffer->ubo, i * buffer->blocksize, sizeof(T));
  }
};
