#pragma once
#include <array>
#include <GL/glew.h>

// Material data
struct Material {
  // Ambient reflection coefficient
  alignas(16) std::array<GLfloat, 3> ambient;
  
  // Diffuse reflection coefficient
  alignas(16) std::array<GLfloat, 3> diffuse;
  
  // Specular reflection coefficient
  alignas(16) std::array<GLfloat, 3> specular;
  
  // Shininess
  alignas( 4) GLfloat shininess;
};
