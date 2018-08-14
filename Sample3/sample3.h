#ifndef __SAMPLE3_H__
#define __SAMPLE3_H__

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void drawYoyo(const GLfloat*, const GLfloat*);
void drawString(const GLfloat*);
void rk4(float&, glm::vec3&, glm::vec3&, glm::vec3&, glm::vec3&);
void rk4_nadir(float*, glm::vec3*, glm::vec3*, glm::vec3*, glm::vec3*);
void computeEnergy(float*, glm::vec3*, glm::vec3*, glm::vec3*, glm::vec3*);
GLFWwindow* userInit(void);
void idle(void);
void display(GLFWwindow*);
static void keyboard (GLFWwindow* const, int, int, int, int);
static void resize (GLFWwindow* const, int, int);
int shouldClose (GLFWwindow*);
void swapBuffers (GLFWwindow*);

bool isNadir = false; // The lowest point flag
glm::vec3 thetaNadir = {0.0f, 0.0f, 0.0f}; // Rotation angle    of yoyo
glm::vec3 omegaNadir = {0.0f, 0.0f, 0.0f}; // Angular  velocity of yoyo


/*
  円柱クラス
 */
class Cylinder {
  GLfloat r; // 底面の半径
  GLfloat h; // 円柱の高さ
  GLuint  N; // 分割数
  
  std::vector<GLuint>    index;  // 描画される各面の頂点番号
  std::vector<glm::vec3> vertex; // 描画される各面の頂点座標
  std::vector<glm::vec3> normal; // 描画される各面の頂点法線

public:
  Cylinder (GLfloat r, GLfloat h, GLuint N = 16)
    : r(r), h(h), N(N) {
    createMesh();
  }

  const std::vector<GLuint>& getIndex (void) {
    return index;
  }
  const std::vector<glm::vec3>& getVertex (void) {
    return vertex;
  }
  const std::vector<glm::vec3>& getNormal (void) {
    return normal;
  }
  
  void createMesh (void) {
    index.clear();
    vertex.clear();
    normal.clear();

    std::vector<glm::vec3> vertexCoord; // 頂点座標
    vertexCoord.emplace_back((glm::vec3){0.0f,  h/2.0f, 0.0f}); // 円柱の頂面の中心
    vertexCoord.emplace_back((glm::vec3){0.0f, -h/2.0f, 0.0f}); // 円柱の底面の中心

    // 円柱の頂面
    for (int i = 0; i < N; ++i) {
      // 円柱の頂面の円周をN分割した点
      GLfloat t = 2.0f * M_PI * static_cast<GLfloat>(i) /  static_cast<GLfloat>(N);
      GLfloat x = r*sin(t);
      GLfloat y = h/2.0f;
      GLfloat z = r*cos(t);

      glm::vec3 v = {x, y, z};
      vertexCoord.emplace_back(v);
    }

    // 円柱の底面
    for (int i = 0; i < N; ++i) {
      // 円柱の底面の円周をN分割した点
      GLfloat t = 2.0f * M_PI * static_cast<GLfloat>(i) /  static_cast<GLfloat>(N);
      GLfloat x =  r*sin(t);
      GLfloat y = -h/2.0f;
      GLfloat z =  r*cos(t);

      glm::vec3 v = {x, y, z};
      vertexCoord.emplace_back(v);
    }
    
    // 円柱の頂面を近似するN角形の各面における頂点座標，頂点法線と頂点番号を計算する
    for (int i = 0; i < N; ++i) {
      vertex.emplace_back(vertexCoord[          0]);
      vertex.emplace_back(vertexCoord[ i   %N + 2]);
      vertex.emplace_back(vertexCoord[(i+1)%N + 2]);

      normal.emplace_back((glm::vec3){0.0f,  1.0f, 0.0f});
      normal.emplace_back((glm::vec3){0.0f,  1.0f, 0.0f});
      normal.emplace_back((glm::vec3){0.0f,  1.0f, 0.0f});
      
      index.emplace_back(3*(i) + 0);
      index.emplace_back(3*(i) + 1);
      index.emplace_back(3*(i) + 2);
    }

    // 円柱の底面を近似するN角形の各面における頂点座標，頂点法線と頂点番号を計算する
    for (int i = 0; i < N; ++i) {
      vertex.emplace_back(vertexCoord[           1]);
      vertex.emplace_back(vertexCoord[(i+1)%N + 10]);
      vertex.emplace_back(vertexCoord[ i   %N + 10]);

      normal.emplace_back((glm::vec3){0.0f, -1.0f, 0.0f});
      normal.emplace_back((glm::vec3){0.0f, -1.0f, 0.0f});
      normal.emplace_back((glm::vec3){0.0f, -1.0f, 0.0f});
      
      index.emplace_back(3*(i+N) + 0);
      index.emplace_back(3*(i+N) + 1);
      index.emplace_back(3*(i+N) + 2);
    }

    // 円柱の側面を近似するメッシュの各面における頂点座標と頂点番号を計算する
    for (int i = 0; i < N; ++i) {
      int    k  = 2;
      GLuint k0 = k  +  i;
      GLuint k1 = k  + (i + 1)%N;
      GLuint k2 = k0 +  N;
      GLuint k3 = k1 +  N;

      glm::vec3 v0 = vertexCoord[k0];
      glm::vec3 v1 = vertexCoord[k1];
      glm::vec3 v2 = vertexCoord[k2];
      glm::vec3 v3 = vertexCoord[k3];
      
      // 左下の三角形
      vertex.emplace_back(v0);
      vertex.emplace_back(v2);
      vertex.emplace_back(v3);

      normal.emplace_back((glm::vec3){v0.x, 0.0f, v0.z});
      normal.emplace_back((glm::vec3){v2.x, 0.0f, v2.z});
      normal.emplace_back((glm::vec3){v3.x, 0.0f, v3.z});
      
      index.emplace_back(6*i + 0 + 3*2*N);
      index.emplace_back(6*i + 1 + 3*2*N);
      index.emplace_back(6*i + 2 + 3*2*N);

      // 右上の三角形
      vertex.emplace_back(v0);
      vertex.emplace_back(v3);
      vertex.emplace_back(v1);

      normal.emplace_back((glm::vec3){v0.x, 0.0f, v0.z});
      normal.emplace_back((glm::vec3){v3.x, 0.0f, v3.z});
      normal.emplace_back((glm::vec3){v1.x, 0.0f, v1.z});
      
      index.emplace_back(6*i + 3 + 3*2*N);
      index.emplace_back(6*i + 4 + 3*2*N);
      index.emplace_back(6*i + 5 + 3*2*N);      
    } 
  }
};

void computeEnergy (float* t, glm::vec3* pos, glm::vec3* vel, glm::vec3* theta, glm::vec3* omega) {
  // Kinetic energy
  //const float K1 = 0.5f * m * dot(*vel, *vel); // Translational motion
  /*
    omega - Angular velocity around an axis of rotation
    L - Angular momentum around the axis of rotation
    I - Tensor of moment of inertia
    -> L = I * omega

    <a, b> := dot(a, b)
    n - Unit vector having same direction of omega
    |omega| - Magnitude of omega
    |I| - Moment of inertia around the axis of rotation
    <n, L> = <n, I|omega|n> = <n, In>|omega| = |I||omega|

    K - Kinetic energy of rotational motion
    -> K = 0.5*|I|*|omega|^2
   */
  /*
  const glm::vec3 n = glm::normalize(*omega);
  const float K2 = 0.5f * glm::dot(n, I*(*omega)) * glm::length(*omega); // Rotational motion
  
  // Potential energy
  const float U = -glm::dot(f0, *pos);

  // Mechanical energy
  const float E = K1 + K2 + U;

  std::cout << "Energy" << std::endl;
  std::cout << std::scientific << std::showpos << std::setprecision(4)
	    << "K1 : " << K1 << " " <<  2.0f*m*g*g*(*t)*(*t)/9.0f << std::endl;
  std::cout << "K2 : " << K2 << " " <<  1.0f*m*g*g*(*t)*(*t)/9.0f <<std::endl;
  std::cout << " U : " << U  << " " << -1.0f*m*g*g*(*t)*(*t)/3.0f <<std::endl;
  std::cout << " E : " << E  << std::endl;
  std::cout << std::endl;
  */
}

void rk4_nadir (float* t, glm::vec3* x, glm::vec3* v, glm::vec3* theta, glm::vec3* omega) {
  /*
  const float  m = 100.0f; // Mass
  const float  g = 980.0f; // Gravitational acceleration
  const float  a = rInner; // Radius of inner cylinder
  
  // Tensor of moment of inertia around barycenter
  const glm::mat3 Ig = {m*a*a/4.0f, 0.0f, 0.0f,
		       0.0f, m*a*a/4.0f, 0.0f,
		       0.0f, 0.0f, m*a*a/2.0f};

  // Vector of position from center of rotation axis to barycenter
  const glm::vec3 r1 = glm::vec3({a*sin(theta->z), -a*cos(theta->z), 0.0f});
  
  // Matrix corresponding to cross product with r1
  const glm::mat3 A1 = { 0.0f, +r1.z, -r1.y,
			-r1.z,  0.0f, +r1.x,
			+r1.y, -r1.x,  0.0f};

  // Tensor of moment of inertia around center of rotation axis
  const glm::vec3 ap = -r1;
  const glm::mat3 Ip = {ap.y*ap.y + ap.z*ap.z, -ap.x*ap.y, -ap.x*ap.z,
			-ap.x*ap.y, ap.x*ap.x + ap.z*ap.z, -ap.y*ap.z,
			-ap.x*ap.z, -ap.y*ap.z, ap.x*ap.x + ap.y*ap.y};
  const glm::mat3 I = Ig + m*Ip;
  
  // Vector of external force
  const glm::vec3 f0 = {m*g, 0.0f, 0.0f};  // Gravity
  //const glm::mat3 tmpMatrix = m*a*a*glm::mat3(1.0f) + Ig;
  //const glm::vec3 f1 = A1*I*glm::inverse(tmpMatrix)*A1*f0/(a*a) + fExt; // Tension
  //const glm::vec3 f1 = -glm::cross(ap, Ig*glm::inverse(tmpMatrix)*glm::cross(-ap, f0))/(a*a) + fExt;
  //const glm::vec3 f  = f0 + f1; // Sum of external force

  // Vector of sum of torque
  //const glm::vec3 N = glm::cross(glm::vec3(0.0f), f1) + glm::cross(r1, f0);

  //const float tau = 0.5f * (1.148f * 0.001f) * (M_PI*pow(rOuter, 2.0f) * 2.0f) * 1.2;
  //float tau = 0.0f;
  
  // Runge-Kutta method  
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func3 = [&I,&a,&f0](float t, glm::vec3 theta, glm::vec3 omega){ return glm::inverse(I)*glm::cross(glm::vec3({a*sin(theta.z), -a*cos(theta.z), 0.0f}), f0); };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func4 = [         ](float t, glm::vec3 theta, glm::vec3 omega){ return omega; };
  glm::vec3 omega1 = func3( *t         ,  *theta                ,  *omega                );
  glm::vec3 theta1 = func4( *t         ,  *theta                ,  *omega                );
  glm::vec3 omega2 = func3((*t)+dt/2.0f, (*theta)+theta1*dt/2.0f, (*omega)+omega1*dt/2.0f);
  glm::vec3 theta2 = func4((*t)+dt/2.0f, (*theta)+theta1*dt/2.0f, (*omega)+omega1*dt/2.0f);
  glm::vec3 omega3 = func3((*t)+dt/2.0f, (*theta)+theta2*dt/2.0f, (*omega)+omega2*dt/2.0f);
  glm::vec3 theta3 = func4((*t)+dt/2.0f, (*theta)+theta2*dt/2.0f, (*omega)+omega2*dt/2.0f);
  glm::vec3 omega4 = func3((*t)+dt     , (*theta)+theta3*dt     , (*omega)+omega3*dt     );
  glm::vec3 theta4 = func4((*t)+dt     , (*theta)+theta3*dt     , (*omega)+omega3*dt     );
  *omega += dt*(omega1 + 2.0f*omega2 + 2.0f*omega3 + omega4)/6.0f;
  *theta += dt*(theta1 + 2.0f*theta2 + 2.0f*theta3 + theta4)/6.0f;
  const glm::vec3 dtheta = dt*(theta1 + 2.0f*theta2 + 2.0f*theta3 + theta4)/6.0f;
  theta->z = fmin(M_PI, fmax(0.0f, theta->z));
  
  const glm::mat4 rot = glm::rotate(glm::mat4(), dtheta.z, glm::vec3({0.0f, 0.0f, 1.0f}));
  
  *x = glm::vec3({L, 0.0f, 0.0f}) + (rot*glm::vec4(r1, 1.0f)).xyz;
  const float vNorm = glm::length(*v);
  *v = glm::vec3({vNorm*cos(theta->z), vNorm*sin(theta->z), 0.0f});
  
  *t     += dt;

  std::cout << std::noshowpos << std::defaultfloat << *t << std::endl;
  std::cout << (isDown ? "Down" : "Up") << " " << (isRight ? "Right" : "Left") << " " << (isNadir ? "Nadir" : "") << std::endl;
  std::cout << "    x : " << glm::to_string(*x) << " " << 1.0f*g*(*t)*(*t)/3.0f << std::endl;
  std::cout << "    v : " << glm::to_string(*v) << " " << 2.0f*g*(*t)     /3.0f << std::endl;
  std::cout << "theta : " << glm::to_string(*theta) << " " << 1.0f*g*(*t)*(*t)/(3.0f*a) << std::endl;
  std::cout << "omega : " << glm::to_string(*omega) << " " << 2.0f*g*(*t)     /(3.0f*a) << std::endl;
  std::cout << std::endl;
  
  isRight = (glm::sign(cos(theta->z)) > 0);
  if (theta->z < 0.0f || theta->z > M_PI) {
    isDown  = false;
    isNadir = false;
  }
  */
}

#endif
