#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <functional>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void rk4(float*, glm::vec3*, glm::vec3*, glm::vec3*, glm::vec3*);
GLFWwindow* userInit(void);
void idle(void);
void display(GLFWwindow*);
static void keyboard (GLFWwindow* const, int, int, int, int);
static void resize (GLFWwindow* const, int, int);
int shouldClose (GLFWwindow*);
void swapBuffers (GLFWwindow*);
void makeBar(GLfloat*, GLfloat*, GLfloat*);
void makeSwing(GLfloat*, GLfloat*, GLfloat*);

/*
  変数(プログラムの設定を保存しておくためのもの)の宣言
*/
int BAR;   // 棒
int SWING; // ブランコ本体

int    g_nAnim      = GL_TRUE;  // 一時停止しているかどうか
int    key_c  = 0;          // ブランコの状態
int    key_f  = 30;         // ブランコの動きの滑らかさ
double viewpoint_angle = 40.0; // ブランコを見るときの角度

glm::vec3     x = {  0.0f, 0.0f, 0.0f};
glm::vec3     v = {  0.0f, 0.0f, 0.0f};
glm::vec3 theta = {  0.0f, 0.0f, 0.0f};
glm::vec3 omega = {  0.0f, 0.0f, 0.0f};
    float     t = 0.0f;
    float    dt = pow(2.0f, -5.0f); // Time step
static bool flag = true; // Up down flag, up - false, down - true

int main() {
  // プログラムを開始するための初期設定をする
  if (glfwInit() == GL_FALSE) {
    std::cerr << "Can't initialize GLFW" << std::endl;
    return 1;
  }
  atexit(glfwTerminate);
  
  // ウィンドウを作成する
  GLFWwindow* window;
  window = userInit();

  // 動作を開始する
  while (shouldClose(window) == GL_FALSE) {
    if (g_nAnim) {
      idle();
    }
    display(window);
  }

  glfwDestroyWindow(window);
  
  return 0;
}

/*
  初期設定のための関数
*/
GLFWwindow* userInit(void) {
  GLfloat g_aBarColorX[3]   = {1.00, 0.00, 0.00}; // 棒の右と左の面の色
  GLfloat g_aBarColorY[3]   = {1.00, 1.00, 0.00}; // 棒の上と下の面の色
  GLfloat g_aBarColorZ[3]   = {1.00, 0.65, 0.00}; // 棒の手前と奥の面の色
  GLfloat g_aRopeColor[3]   = {1.00, 0.00, 1.00}; // 紐の色
  GLfloat g_aSeatColorXZ[3] = {0.00, 1.00, 0.00}; // 座席の側面の色
  GLfloat g_aSeatColorY[3]  = {0.00, 0.00, 1.00}; // 座席の上下面の色

  // ウィンドウの幅と高さ
  int width  = 600;
  int height = 600;

  // ウィンドウを作成する
  GLFWwindow* window = glfwCreateWindow(width, height, "Swing", NULL, NULL);
  if (window == NULL) {
    // Cannot create window
    std::cerr << "Can't create GLFW window." << std::endl;
    exit(1);
  }

  // Make created window processing target of OpenGL
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Can't initialize GLEW" << std::endl;
    exit(1);
  }
    
  // Wait timing for vertical synchronization
  glfwSwapInterval(1);
    
  // Register processing at changing window size
  glfwSetWindowSizeCallback(window, resize);

  // Register processing at using keyboard
  glfwSetKeyCallback(window, keyboard);
    
  // Initialize setting of opened window
  resize(window, width, height);

  // 背景の色を設定する
  glClearColor(0.9, 0.9, 0.9, 1.0);

  // ブランコの描画のための設定をする
  glShadeModel(GL_FLAT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);

  // ブランコを作成する
  BAR = glGenLists(2);
  SWING = BAR + 1;
  makeBar(g_aBarColorX, g_aBarColorY, g_aBarColorZ);
  makeSwing(g_aRopeColor, g_aSeatColorXZ, g_aSeatColorY);

  return window;
}

/*
  一定時間ごとに動作する関数
*/
void idle(void) { 
  static clock_t currentTime;        // 現在の時間
  static clock_t previousTime = 0.0; // 前の時間
  const float g = 9.8f;
  static bool flags[2] = {true, true};
  
  if(key_c == 0){
    const float T = 10.0f/(2.0f*M_PI);
    // 一定の量ずつ角度が増える
    x.x += v.x*dt;
    v.x += g*dt;
    theta.z += omega.z*dt;
    omega.z += g*dt*glm::sign(sin(t/T));
    t   += dt;
    
    flags[0] = flags[1];
    flags[1] = sin(t/T) > 0;
    if (flags[0] && !flags[1]) {
      v.x = -v.x;
      flag = !flag;
    }
    if (flag && x.x > 100.0f) {
      x.x = 100.0f;
    } else if (!flag && x.x < 0.0f) {
      x.x = 0.0f;
    }
  }else if(key_c == 1){
    // 物理法則に従って角度が増える
    //const clock_t start = clock();
    rk4(&t, &x, &v, &theta, &omega);
    //const clock_t end = clock();
    //std::cout << (double)(end - start)/CLOCKS_PER_SEC << std::endl;
  }

  // 決められた時間だけ待つ
  do{
    currentTime = clock();
  }while((double)(currentTime - previousTime)/CLOCKS_PER_SEC < 1/(double)key_f);
  previousTime = currentTime;
}

/*
  物体を表示するための関数
*/
void display(GLFWwindow* window) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  // 棒を表示する
  glCallList(BAR);

  // ブランコ本体を表示する
  glTranslated(-50.0, 0.0, 0.0);
  glTranslated(x.x, x.y, x.z);
  glRotated(theta.z, 0.0, 0.0, 1.0);
  glCallList(SWING);

  glPopMatrix();

  swapBuffers(window);
}

/*
  キーボード操作のための関数
*/
static void keyboard (GLFWwindow* const window, int key, int scancode, int action, int mods) {
  switch(key) {
  case GLFW_KEY_Q :
    if (action == GLFW_PRESS) {
      // プログラムを終了する
      exit(0);
    }
    break;
  case GLFW_KEY_A :
    if (action == GLFW_PRESS) {
      // 一時停止したり解除したりする
      g_nAnim = !g_nAnim;
    }
    break;
  case GLFW_KEY_C :
    if (action == GLFW_PRESS) {
      // ブランコの状態を変える
      key_c = (key_c == 0) ? 1 : 0;
    }
    break;  
  case GLFW_KEY_F :
    if (action == GLFW_PRESS) {
      if (mods == GLFW_MOD_SHIFT) {
	// ブランコの動きの滑らかさを上げる
	key_f += 5;
	if(key_f > 60){
	  key_f = 30;
	}
      } else {
	// ブランコの動きの滑らかさを下げる
	key_f -= 5;
	if(key_f <= 0){
	  key_f = 30;
	}
      }
      dt = 1/(float)key_f;
    }
    break;
  default:
    break;
  }
}

/*
  ウィンドウの大きさが変更されたときのための関数
*/
static void resize (GLFWwindow* const window, int width, int height) {
  double viewpoint_x = -150.0*sin(M_PI/180.0*viewpoint_angle); // 視点のx座標
  double viewpoint_y =    0.0;                               // 視点のy座標
  double viewpoint_z =  150.0*cos(M_PI/180.0*viewpoint_angle); // 視点のz座標
   
  // ウィンドウの大きさに合わせて物体の大きさや視点も変更する
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glm::mat4 Projection = glm::perspective(glm::radians(40.0),
					  (GLdouble)width/(GLdouble)height,
					  5.0, 1024.0);
  glLoadMatrixf(glm::value_ptr(Projection));
  
  glMatrixMode(GL_MODELVIEW);
  glm::mat4 ModelView = glm::lookAt(glm::vec3(viewpoint_x, viewpoint_y, viewpoint_z),
				    glm::vec3( 0.0, 0.0, 0.0),
				    glm::vec3(-1.0, 0.0, 0.0));
  glLoadMatrixf(glm::value_ptr(ModelView));
}

// Decide if window should be closed
int shouldClose (GLFWwindow* window) {
  return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
}

// Get events after swapping color buffer
void swapBuffers (GLFWwindow* window) {
  // Swap color buffer
  glfwSwapBuffers(window);
  
  // Get some events
  glfwPollEvents();
}

/*
  棒を作成するための関数
*/
void makeBar(GLfloat* g_aBarColorX, GLfloat* g_aBarColorY, GLfloat* g_aBarColorZ) {
  glNewList(BAR, GL_COMPILE);
  
  glPushMatrix();
  
  // 棒を作成する
  glBegin(GL_QUADS);
  glColor3fv(g_aBarColorX); // 右と左の面の色の設定をする
  // 右の面の作成をする
  glVertex3d(50, 33, 1);
  glVertex3d(50, 30, 1);
  glVertex3d(50, 30, -1);
  glVertex3d(50, 33, -1);
  // 左の面の作成をする
  glVertex3d(-50, 33, -1);
  glVertex3d(-50, 30, -1);
  glVertex3d(-50, 30,  1);
  glVertex3d(-50, 33,  1);

  glColor3fv(g_aBarColorY); // 上と下の面の色の設定をする
  // 上の面を作成する
  glVertex3d(-50, 33, -1);
  glVertex3d( 50, 33, -1);
  glVertex3d( 50, 33,  1);
  glVertex3d(-50, 33,  1);
  // 下の面を作成する
  glVertex3d(-50, 30, -1);
  glVertex3d( 50, 30, -1);
  glVertex3d( 50, 30,  1);
  glVertex3d(-50, 30,  1);

  glColor3fv(g_aBarColorZ); // 手前と奥の面の色の設定をする
  // 手前の面の作成をする
  glVertex3d(-50, 33, 1);
  glVertex3d(-50, 30, 1);
  glVertex3d( 50, 30, 1);
  glVertex3d( 50, 33, 1);
  // 奥の面の作成をする
  glVertex3d( 50, 33, -1);
  glVertex3d( 50, 30, -1);
  glVertex3d(-50, 30, -1);
  glVertex3d(-50, 33, -1);
  glEnd();

  glPopMatrix();

  glEndList();
}

/*
  ブランコ本体を作成するための関数
*/
void makeSwing(GLfloat* g_aRopeColor, GLfloat* g_aSeatColorXZ, GLfloat* g_aSeatColorY) {
  glNewList(SWING, GL_COMPILE);

  glPushMatrix();
 
  // ブランコの紐を作成する
  glLineWidth(5.0);         // 紐の太さの設定をする
  glBegin(GL_LINES);
  glColor3fv(g_aRopeColor); // 紐の色の設定をする
  // 手前の紐を作成する
  glVertex3d( 0,  30, 0);
  glVertex3d( 0,   0, 0);
  // 奥の紐を作成する
  glVertex3d( 0,   0, 0);
  glVertex3d( 0, -30, 0);
  glEnd();

  // ブランコの座席の側面を作成する
  glBegin(GL_QUAD_STRIP);
  glColor3fv(g_aSeatColorXZ);  // 座席の側面の色の設定をする
  // 座席の側面を作成する
  glVertex3d( 20, -0, -10);
  glVertex3d( 20, -3, -10);
  glVertex3d(-20, -0, -10);
  glVertex3d(-20, -3, -10);
  glVertex3d(-20, -0,  10);
  glVertex3d(-20, -3,  10);
  glVertex3d( 20, -0,  10);
  glVertex3d( 20, -3,  10);
  glVertex3d( 20, -0, -10);
  glVertex3d( 20, -3, -10);
  glEnd();

  // ブランコの座席の上下面を作成する
  glBegin(GL_QUADS);
  glColor3fv(g_aSeatColorY);  // 座席の上下面の色の設定をする
  // 座席の上の面を作成する
  glVertex3d(-20, -0, -10);
  glVertex3d( 20, -0, -10);
  glVertex3d( 20, -0,  10);
  glVertex3d(-20, -0,  10);
  // 座席の下の面を作成する
  glVertex3d(-20, -3, -10);
  glVertex3d( 20, -3, -10);
  glVertex3d( 20, -3,  10);
  glVertex3d(-20, -3,  10);
  glEnd();

  glPopMatrix();

  glEndList();
}

/*
  Matrix in GLM is column major order, so be careful in initializing matrix.
  glm::mat3 Matrix = {m11, m21, m31, m12, m22, m32, m13, m23, m33};
  We expect the matrix to form a line like
  m11 m12 m13
  m21 m22 m23
  m31 m32 m33
 */
void rk4 (float* t, glm::vec3* x, glm::vec3* v, glm::vec3* theta, glm::vec3* omega) {
  //const float dt =  pow(2.0f, -3.0f); // Time step
  const float  m =   0.1f; // Mass
  const float  g =   9.8f; // Gravitational acceleration
  const float  a =   0.03f; // Radius of inner cylinder
  const float  L = 100.0f; // Length of string
  
  //const glm::mat3 identity = glm::mat3(1.0f);
  
  // Tensor of moment of inertia
  const glm::mat3 I = {m*a*a/4.0f, 0.0f, 0.0f,
		       0.0f, m*a*a/4.0f, 0.0f,
		       0.0f, 0.0f, m*a*a/2.0f};

  // Vector of position for external force
  const glm::vec3 r1 = flag ? glm::vec3({0.0f, a, 0.0f}) : glm::vec3({0.0f, -a, 0.0f});
  
  // Matrix corresponding to cross product with r1
  const glm::mat3 A1 = { 0.0f, +r1.z, -r1.y,
			-r1.z,  0.0f, +r1.x,
			+r1.y, -r1.x,  0.0f};
  
  // Vector of external force
  const glm::vec3 f0 = {m*g, 0.0f, 0.0f};  // Gravity
  const glm::mat3 tmpMatrix = m*a*a*glm::mat3(1.0f) + I;
  const glm::vec3 f1 = A1*I*glm::inverse(tmpMatrix)*A1*f0/(a*a); // Tension
  //const glm::vec3 f2 = -glm::cross(r1, I*glm::inverse(tmpMatrix)*glm::cross(-r1, f0))/(a*a);
  const glm::vec3 f  = f0 + f1; // Sum of external force

  // Vector of sum of torque
  const glm::vec3 N = glm::cross(glm::vec3(0.0f), f0) + glm::cross(r1, f1);

  // Runge-Kutta method
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func1 = [&f,&m](float t, glm::vec3 x, glm::vec3 v){ return f/m; };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func2 = [     ](float t, glm::vec3 x, glm::vec3 v){ return v; };
  glm::vec3 v1 = func1( *t         ,  *x            ,  *v            );
  glm::vec3 x1 = func2( *t         ,  *x            ,  *v            );
  glm::vec3 v2 = func1((*t)+dt/2.0f, (*x)+x1*dt/2.0f, (*v)+v1*dt/2.0f);
  glm::vec3 x2 = func2((*t)+dt/2.0f, (*x)+x1*dt/2.0f, (*v)+v1*dt/2.0f);
  glm::vec3 v3 = func1((*t)+dt/2.0f, (*x)+x2*dt/2.0f, (*v)+v2*dt/2.0f);
  glm::vec3 x3 = func2((*t)+dt/2.0f, (*x)+x2*dt/2.0f, (*v)+v2*dt/2.0f);
  glm::vec3 v4 = func1((*t)+dt     , (*x)+x3*dt     , (*v)+v3*dt     );
  glm::vec3 x4 = func2((*t)+dt     , (*x)+x3*dt     , (*v)+v3*dt     );
  *v += dt*(v1 + 2.0f*v2 + 2.0f*v3 + v4)/6.0f;
  *x += dt*(x1 + 2.0f*x2 + 2.0f*x3 + x4)/6.0f;
  
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func3 = [&I,&N](float t, glm::vec3 theta, glm::vec3 omega){ return glm::inverse(I)*N; };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func4 = [     ](float t, glm::vec3 theta, glm::vec3 omega){ return omega; };
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

  *t     += dt;  
  
  // Kinetic energy
  const float K1 = 0.5f * m * dot(*v, *v); // Translational motion
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
  const glm::vec3 n = glm::normalize(*omega);
  const float K2 = 0.5f * glm::dot(n, I*(*omega)) * glm::length(*omega); // Rotational motion
  
  // Potential energy
  const float U = -glm::dot(f0, *x);

  // Mechanical energy
  const float E = K1 + K2 + U;

  // Debug print
  
  std::cout << std::noshowpos << std::defaultfloat << *t << std::endl;
  std::cout << "    x : " << glm::to_string(*x) << " " << 1.0f*g*(*t)*(*t)/3.0f << std::endl;
  std::cout << "    v : " << glm::to_string(*v) << " " << 2.0f*g*(*t)     /3.0f << std::endl;
  std::cout << "theta : " << glm::to_string(*theta) << " " << 1.0f*g*(*t)*(*t)/(3.0f*a) << std::endl;
  std::cout << "omega : " << glm::to_string(*omega) << " " << 2.0f*g*(*t)     /(3.0f*a) << std::endl;
  std::cout << std::endl;
  /*
  std::cout << "Energy" << std::endl;
  std::cout << std::scientific << std::showpos << std::setprecision(4)
	    << "K1 : " << K1 << " " <<  2.0f*m*g*g*(*t)*(*t)/9.0f << std::endl;
  std::cout << "K2 : " << K2 << " " <<  1.0f*m*g*g*(*t)*(*t)/9.0f <<std::endl;
  std::cout << " U : " << U  << " " << -1.0f*m*g*g*(*t)*(*t)/3.0f <<std::endl;
  std::cout << " E : " << E  << std::endl;
  std::cout << std::endl;
  */
  // Boundary condition
  if (x->x > L && flag) {
    *v   = -(*v);
    flag = false;
  } else if (x->x < 0 && !flag) {
    *v   = -(*v);
    flag = true;
  }
}
