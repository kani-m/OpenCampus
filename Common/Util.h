#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLuint createProgram (const char*, const char*);
GLuint loadProgram (const char*, const char*);
bool readShaderSource (const char*, std::vector<GLchar>&);
GLboolean printShaderInfoLog (GLuint, const char*);
GLboolean printProgramInfoLog (GLuint);

// Create program object
//   vsrc : Source code of vertex shader
//   fsrc : Source code of fragment shader
GLuint createProgram (const char* vsrc, const char* fsrc) {
  // Create empty program object
  const GLuint program(glCreateProgram());

  if (vsrc != NULL) {
    // Create shader object of vertex shader
    const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
    glShaderSource(vobj, 1, &vsrc, NULL);
    glCompileShader(vobj);

    // Embed shader object of vertex shader into program object
    if (printShaderInfoLog(vobj, "vertex shader")) {
      glAttachShader(program, vobj);
    }
    glDeleteShader(vobj);
  }

  if (fsrc != NULL) {
    // Create shader object of fragment shader
    const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
    glShaderSource(fobj, 1, &fsrc, NULL);
    glCompileShader(fobj);

    // Embed shader object of fragment shader into program object
    if (printShaderInfoLog(fobj, "fragment shader")) {
      glAttachShader(program, fobj);
    }
    glDeleteShader(fobj);
  }

  // Link program object
  glBindAttribLocation(program, 0, "position");
  glBindAttribLocation(program, 1, "color");
  glBindAttribLocation(program, 2, "normal");
  glBindFragDataLocation(program, 0, "fragment");
  glLinkProgram(program);

  // Return created program object
  if (printProgramInfoLog(program)) {
    return program;
  }

  // Return 0 if program object cannot be created
  glDeleteProgram(program);
  return 0;
}

// Create program object by reading source file of shader
//   vert : name of source file of vertex shader
//   frag : name of source file of fragment shader
GLuint loadProgram (const char* vert, const char* frag) {
  // Read source file of shader
  std::vector<GLchar> vsrc;
  const bool vstat(readShaderSource(vert, vsrc));
  std::vector<GLchar> fsrc;
  const bool fstat(readShaderSource(frag, fsrc));

  // Create program object
  return vstat && fstat ? createProgram(vsrc.data(), fsrc.data()) : 0;
}

// Return memory written source code of shader
//   name : name of source file of shader
//   buffer : text of source file
bool readShaderSource (const char* name, std::vector<GLchar>& buffer) {
  // Check if file name is NULL
  if (name == NULL) {
    return false;
  }

  // Open source file
  std::ifstream file(name, std::ios::binary);
  if (file.fail()) {
    // Cannot open
    std::cerr << "Error: Can't open source file: " << name << std::endl;
    return false;
  }

  // Get file size after seeking end of file
  file.seekg(0L, std::ios::end);
  GLsizei length = static_cast<GLsizei>(file.tellg());

  // Allocate memory to store file
  buffer.resize(length + 1);

  // Read from the top of file
  file.seekg(0L, std::ios::beg);
  file.read(buffer.data(), length);
  buffer[length] = '\0';

  if (file.fail()) {
    // Cannot read
    file.close();
    std::cerr << "Error: Could not read source file: " << name << std::endl;
    return false;
  }

  // Succeed reading
  file.close();
  return true;
}

// Display compilation result of shader object
//   shader : shader object
//   str : string pointing to line of compilation error
GLboolean printShaderInfoLog (GLuint shader, const char* str) {
  // Get compilation result
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    std::cerr << "Compile Error in " << str << std::endl;
  }

  // Get length of log at time of compiling shader
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1) {
    // Get content of log at time of compiling shader
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}

// Display linkage result of program object
//   program : program object
GLboolean printProgramInfoLog (GLuint program) {
  // Get linkage result
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    std::cerr << "Link Error." << std::endl;
  }
  
  // Get length of log at time of linking shader
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1) {
    // Get content of log at time of linking shader
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}
