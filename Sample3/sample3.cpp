/*
  sample3
  内容 : ヨーヨーのシミュレーションプログラム
  キーボード操作 :
  q - 終了
  a - 一時停止
  c - ヨーヨーの状態の切り替え
  f - 動きの滑らかさを下げる
  F - 動きの滑らかさを上げる
*/

/* 
  関数(コンピュータへの命令を機能ごとにまとめたもの)を呼ぶための準備 
*/
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "sample3.h"

/*
  変数(プログラムの設定を保存しておくためのもの)の宣言
*/
int    g_nAnim         = GL_FALSE; // 一時停止しているかどうか
int    g_nYoyoMode     = 0;        // ヨーヨーの状態
int    g_nFramerate    = 30;       // ヨーヨーの動きの滑らかさ
double viewpoint_angle = 0.0;      // ヨーヨーを見るときの角度

const float   aGra = 980.0f; // 重力加速度 [cm/s^2]
const float  mYoyo = 100.0f; // ヨーヨーの質量 [g]
const float rInner =   5.0f; // ヨーヨーの内側の半径(紐を巻きつける部分) [cm]
const float rOuter =  25.0f; // ヨーヨーの外側の半径(実際に見える部分) [cm]
const float   lStr = 400.0f; // ヨーヨーの紐の長さ [cm]
// ヨーヨーを上に引く力 [g cm/s^2]
glm::vec3    fPull = glm::vec3({-10.0f*mYoyo*aGra, 0.0f, 0.0f});

const GLfloat g_aStringColor[3] = {1.00, 0.00, 1.00}; // 紐の色
const GLfloat g_aYoyoColor[3]   = {0.00, 1.00, 0.00}; // ヨーヨーの色

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
    if (!g_nAnim) {
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
  // ウィンドウの幅と高さ
  int width  = 600;
  int height = 600;

  // ウィンドウを作成する
  GLFWwindow* window = glfwCreateWindow(width, height, "Yoyo", NULL, NULL);
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

  // ヨーヨーの描画のための設定をする
  glShadeModel(GL_FLAT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  return window;
}

/*
  計算用変数の宣言
 */
bool isDown  = true;  // Up   down  flag, up   - false, down  - true
bool isRight = true;  // Left right flag, left - false, right - true

glm::vec3   pos = {0.0f, -rInner, 0.0f}; // Position of yoyo
glm::vec3   vel = {0.0f,    0.0f, 0.0f}; // Velocity of yoyo
glm::vec3 theta = {0.0f,    0.0f, 0.0f}; // Rotation angle    of yoyo
glm::vec3 omega = {0.0f,    0.0f, 0.0f}; // Angular  velocity of yoyo
float         t =  0.0f;                 // Time
float        dt =  pow(2.0f, -5.0f);     // Time step
glm::vec3  fExt = glm::vec3(0.0f);       // Force to pull

/*
  一定時間ごとに動作する関数
*/
void idle(void) { 
  static clock_t currentTime;        // 現在の時間
  static clock_t previousTime = 0.0; // 前の時間
  
  if (g_nYoyoMode == 0) {
    const float T      = 20.0f;       // ヨーヨーの周期
    const float dl     = 4.0f*lStr/T; // ヨーヨーの位置の刻み幅
    const float dtheta = 8.0f*M_PI/T; // ヨーヨーの角度の刻み幅

    // 運動のフラグの設定
    if (t < T/4.0f) {
      isDown  = true;
      isRight = true;
    } else if (t < T/2.0f) {
      isDown  = false;
      isRight = false;
    } else if (t < T/4.0f*3.0f) {
      isDown  = true;
      isRight = false;
    } else if (t < T) {
      isDown  = false;
      isRight = true;
    }
    
    // 一定の量ずつ運動する
    pos.x   += isDown  ? dl*dt   : -dl*dt;
    pos.y    = isRight ? -rInner : rInner;
    vel.x    = isDown  ? dl      : -dl;
    theta.z += !(isDown^isRight) ? dtheta*dt : -dtheta*dt;
    omega.z  = !(isDown^isRight) ? dtheta    : -dtheta;
    t       += dt;

    if (t > T) {
      t       = 0;
      pos.x   = 0.0f;
      theta.z = 0.0f;
    }
  } else if (g_nYoyoMode == 1 || g_nYoyoMode == 2) {
    // 物理法則に従って運動する
    rk4(t, pos, vel, theta, omega);
  }

  // 決められた時間だけ待つ
  do {
    currentTime = clock();
  } while ((double)(currentTime - previousTime)/CLOCKS_PER_SEC < 1/(double)g_nFramerate);
  previousTime = currentTime;
}

/*
  物体を表示するための関数
*/
void display(GLFWwindow* window) {
  const GLfloat g_aYoyoColorInner[3] = {0.00, 1.00, 0.00}; // ヨーヨーの内側の色
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  // 視点の角度に合わせて回転させる
  glRotated(viewpoint_angle, 1.0, 0.0, 0.0);
  
  glPushMatrix();
  // 全体をずらす
  glTranslated(-lStr/2.0f, 0.0, 0.0);
  
  // 紐を表示する
  drawString(g_aStringColor);

  // ヨーヨー本体を表示する
  glTranslated(pos.x, pos.y, pos.z);
  glRotated(glm::degrees(theta.z), 0.0, 0.0, 1.0);
  drawYoyo(g_aYoyoColorInner, g_aYoyoColor);

  glPopMatrix();
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
      // ヨーヨーの状態を変える
      g_nYoyoMode = (g_nYoyoMode + 1) % 3;

      // 変数を初期化する
      isDown  = true;  // Up   down  flag, up   - false, down  - true
      isRight = true;  // Left right flag, left - false, right - true
      
      pos   = {0.0f, -rInner, 0.0f}; // Position of yoyo
      vel   = {0.0f,    0.0f, 0.0f}; // Velocity of yoyo
      theta = {0.0f,    0.0f, 0.0f}; // Rotation angle    of yoyo
      omega = {0.0f,    0.0f, 0.0f}; // Angular  velocity of yoyo
      t     =  0.0f;                 // Time
    }
    break;  
  case GLFW_KEY_F :
    if (action == GLFW_PRESS) {
      if (mods == GLFW_MOD_SHIFT) {
	// ヨーヨーの動きの滑らかさを上げる
	g_nFramerate += 5;
	if(g_nFramerate > 60){
	  g_nFramerate = 60;
	}
      } else {
	// ヨーヨーの動きの滑らかさを下げる
	g_nFramerate -= 5;
	if(g_nFramerate < 5){
	  g_nFramerate = 5;
	}
      }
      dt = 1/(float)g_nFramerate;
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
  double viewpoint_x =      0.0f; // 視点のx座標
  double viewpoint_y =      0.0f; // 視点のy座標
  double viewpoint_z = lStr*1.5f; // 視点のz座標
  
  // ウィンドウの大きさに合わせて物体の大きさや視点も変更する
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glm::mat4 Projection = glm::perspective(glm::radians(40.0),
					  (GLdouble)width/(GLdouble)height,
					  5.0, (GLdouble)lStr*2.0);
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

  // 矢印キーを押すと，10度ずつ視点が回転する
  if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE) {
    viewpoint_angle -= 10.0;
  } else if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE) {
    viewpoint_angle += 10.0;
  }

  // ヨーヨーを上に持ち上げる力を設定する(空気抵抗ありの状態のみ有効)
  fExt = static_cast<float>(glfwGetKey(window, GLFW_KEY_SPACE)) * (pos.x > 0) * (g_nYoyoMode == 2) * fPull;
}

/*
  ヨーヨーの紐を描画するための関数
 */
void drawString (const GLfloat* g_aStringColor) {
  glPushMatrix();
  
  // ヨーヨーの紐を作成する
  glLineWidth(5.0);           // 紐の太さの設定をする
  glBegin(GL_LINES);
  glColor3fv(g_aStringColor); // 紐の色の設定をする
  glVertex3f(  0.0f, 0.0f, 0.0f);
  glVertex3f( pos.x, 0.0f, 0.0f);
  glEnd();

  glPopMatrix();
}

/*
  ヨーヨー本体を描画するための関数
*/
void drawYoyo(const GLfloat* g_aYoyoColorInner, const GLfloat* g_aYoyoColorOuter) {
  static Cylinder c0(rInner, 2.0f);
  static Cylinder c1(rOuter, 8.0f);
  static Cylinder c2(rOuter, 8.0f);
  
  glPushMatrix();
  glRotated(90.0, 1.0, 0.0, 0.0);

  // ヨーヨーを描画する
  glPushMatrix();
  glColor3fv(g_aYoyoColorInner); // ヨーヨーの色の設定をする
  glVertexPointer(3, GL_FLOAT, 0, c0.getVertex().data());
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(c0.getIndex().size()),
		 GL_UNSIGNED_INT, c0.getIndex().data());
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();

  glPushMatrix();
  glTranslated(0.0f, 1.0f, 0.0f);
  glColor3fv(g_aYoyoColorOuter); // ヨーヨーの色の設定をする
  glVertexPointer(3, GL_FLOAT, 0, c1.getVertex().data());
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(c1.getIndex().size()),
		 GL_UNSIGNED_INT, c1.getIndex().data());
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();

  glPushMatrix();
  glTranslated(0.0f, -1.0f, 0.0f);
  glColor3fv(g_aYoyoColorOuter); // ヨーヨーの色の設定をする
  glVertexPointer(3, GL_FLOAT, 0, c2.getVertex().data());
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(c2.getIndex().size()),
		 GL_UNSIGNED_INT, c2.getIndex().data());
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();

  // ヨーヨーの回転運動をわかりやすくする補助線を描画する
  glPushMatrix();
  glLineWidth(5.0); // 紐の太さの設定をする
  glBegin(GL_LINES);
  const float tmp = rOuter*2.0f;
  if (omega.z > 0) {
    glColor3f(1.0f, 0.0f, 0.0f); // 紐の色の設定をする
    glVertex3f(tmp                       , 0.0f, -tmp                       );
    glVertex3f(tmp                       , 0.0f,  tmp                       );
    glVertex3f(tmp - 10.0f*sin( M_PI/4.0), 0.0f, -tmp + 10.0f*cos( M_PI/4.0));
    glVertex3f(tmp                       , 0.0f, -tmp                       );
    glVertex3f(tmp - 10.0f*sin(-M_PI/4.0), 0.0f, -tmp + 10.0f*cos(-M_PI/4.0));
    glVertex3f(tmp                       , 0.0f, -tmp                       );
  } else if (omega.z < 0) {
    glColor3f(0.0f, 0.0f, 1.0f); // 紐の色の設定をする
    glVertex3f(tmp                       , 0.0f, -tmp                       );
    glVertex3f(tmp                       , 0.0f,  tmp                       );
    glVertex3f(tmp - 10.0f*sin( M_PI/4.0), 0.0f,  tmp - 10.0f*cos( M_PI/4.0));
    glVertex3f(tmp                       , 0.0f,  tmp                       );
    glVertex3f(tmp - 10.0f*sin(-M_PI/4.0), 0.0f,  tmp - 10.0f*cos(-M_PI/4.0));
    glVertex3f(tmp                       , 0.0f,  tmp                       );
  }
  glEnd();
  glPopMatrix();
  
  glPopMatrix();
}

/*
  運動をシミュレーションするための関数
 */
void rk4 (float& t, glm::vec3& pos, glm::vec3& vel, glm::vec3& theta, glm::vec3& omega) {
  /*
    We adopt cgs units system in program
    - Length : cm
    - Mass   : g
    - Time   : s,
    in other words, carry out scaling.

    Why?
    -> To avoid numerical computation error and get better numerical solution.
    Theoretical physics adopts SI units system, length (m), mass (kg) and time (s),
    but values of parameter used in practical problem become smaller in SI.
    In addition, algorithm of numerical computation usually pick 
    time step dt as value smaller than 1 for better solution,
    so other parameters need not become smaller to avoid that
    computation result with dt gets closer to order (of magnitude) of error.
  */
  const float m = mYoyo;  // Mass
  const float g = aGra;   // Gravitational acceleration
  const float a = rInner; // Radius of inner cylinder
  
  //const glm::mat3 identity = glm::mat3(1.0f);

  /*
    Matrix in GLM is column major order, so be careful in initializing matrix.
    glm::mat3 Matrix = {m11, m21, m31, m12, m22, m32, m13, m23, m33};
    We expect the matrix to form a line like
    m11 m12 m13
    m21 m22 m23
    m31 m32 m33
  */
  // Tensor of moment of inertia
  const glm::mat3 I = {m*a*a/4.0f, 0.0f, 0.0f,
		       0.0f, m*a*a/4.0f, 0.0f,
		       0.0f, 0.0f, m*a*a/2.0f};

  // Vector of position for external force
  const glm::vec3 r1 = isRight ? glm::vec3({0.0f, a, 0.0f}) : glm::vec3({0.0f, -a, 0.0f});
  
  // Matrix corresponding to cross product with r1
  const glm::mat3 A1 = { 0.0f, +r1.z, -r1.y,
			-r1.z,  0.0f, +r1.x,
			+r1.y, -r1.x,  0.0f};
  
  // Vector of external force
  const glm::vec3 f0 = {m*g, 0.0f, 0.0f};  // Gravity
  const glm::mat3 tmpMatrix = m*a*a*glm::mat3(1.0f) + I;
  const glm::vec3 f1 = A1*I*glm::inverse(tmpMatrix)*A1*f0/(a*a) + fExt; // Tension
  //const glm::vec3 f1 = -glm::cross(r1, I*glm::inverse(tmpMatrix)*glm::cross(-r1, f0))/(a*a);
  const glm::vec3 f  = f0 + f1; // Sum of external force

  // Vector of sum of torque
  const glm::vec3 N = glm::cross(glm::vec3(0.0f), f0) + glm::cross(r1, f1);

  const float tau = (g_nYoyoMode == 2) * 0.5f * (1.148f * 0.001f) * (M_PI*pow(rOuter, 2.0f) * 2.0f) * 1.2;
  
  // Runge-Kutta method
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func1 =
    [&f,&m,&tau] (float t, glm::vec3 pos, glm::vec3 vel) {
    return (f + glm::vec3(-glm::sign(vel.x)*tau*vel.x*vel.x, 0.0f, 0.0f))/m; };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func2 =
    [          ] (float t, glm::vec3 pos, glm::vec3 vel) { return vel; };
  
  glm::vec3 vel1 = func1(t          , pos               , vel               );
  glm::vec3 pos1 = func2(t          , pos               , vel               );
  glm::vec3 vel2 = func1(t + dt/2.0f, pos + pos1*dt/2.0f, vel + vel1*dt/2.0f);
  glm::vec3 pos2 = func2(t + dt/2.0f, pos + pos1*dt/2.0f, vel + vel1*dt/2.0f);
  glm::vec3 vel3 = func1(t + dt/2.0f, pos + pos2*dt/2.0f, vel + vel2*dt/2.0f);
  glm::vec3 pos3 = func2(t + dt/2.0f, pos + pos2*dt/2.0f, vel + vel2*dt/2.0f);
  glm::vec3 vel4 = func1(t + dt     , pos + pos3*dt     , vel + vel3*dt     );
  glm::vec3 pos4 = func2(t + dt     , pos + pos3*dt     , vel + vel3*dt     );
  vel   += dt*(vel1 + 2.0f*vel2 + 2.0f*vel3 + vel4)/6.0f;
  pos   += dt*(pos1 + 2.0f*pos2 + 2.0f*pos3 + pos4)/6.0f;
  pos.y  = isRight ? -rInner : rInner;
  
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func3 = [&I,&N](float t, glm::vec3 theta, glm::vec3 omega){ return glm::inverse(I)*N; };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func4 = [     ](float t, glm::vec3 theta, glm::vec3 omega){ return omega; };
  glm::vec3 omega1 = func3(t          , theta                 , omega                 );
  glm::vec3 theta1 = func4(t          , theta                 , omega                 );
  glm::vec3 omega2 = func3(t + dt/2.0f, theta + theta1*dt/2.0f, omega + omega1*dt/2.0f);
  glm::vec3 theta2 = func4(t + dt/2.0f, theta + theta1*dt/2.0f, omega + omega1*dt/2.0f);
  glm::vec3 omega3 = func3(t + dt/2.0f, theta + theta2*dt/2.0f, omega + omega2*dt/2.0f);
  glm::vec3 theta3 = func4(t + dt/2.0f, theta + theta2*dt/2.0f, omega + omega2*dt/2.0f);
  glm::vec3 omega4 = func3(t + dt     , theta + theta3*dt     , omega + omega3*dt     );
  glm::vec3 theta4 = func4(t + dt     , theta + theta3*dt     , omega + omega3*dt     );
  omega += dt*(omega1 + 2.0f*omega2 + 2.0f*omega3 + omega4)/6.0f;
  theta += dt*(theta1 + 2.0f*theta2 + 2.0f*theta3 + theta4)/6.0f;

  t     += dt;  
  
  // Debug print
  /*
  std::cout << std::noshowpos << std::defaultfloat << t << std::endl;
  std::cout << (isDown ? "Down" : "Up") << " " << (isRight ? "Right" : "Left") << std::endl;
  std::cout << glm::to_string(fExt) << std::endl;
  std::cout << "    x : " << glm::to_string(pos) << " " << 1.0f*g*t*t/3.0f << std::endl;
  std::cout << "    v : " << glm::to_string(vel) << " " << 2.0f*g*t  /3.0f << std::endl;
  std::cout << "theta : " << glm::to_string(theta) << " " << 1.0f*g*t*t/(3.0f*a) << std::endl;
  std::cout << "omega : " << glm::to_string(omega) << " " << 2.0f*g*t  /(3.0f*a) << std::endl;
  std::cout << std::endl;
  */
  
  // Boundary condition
  if (pos.x > lStr && isDown) {
    vel     = -vel;
    isDown  = false;
    isRight = !isRight;
    /*
    isNadir = true;
    thetaNadir = isRight ? glm::vec3(0.0f) : glm::vec3({0.0f, 0.0f, M_PI});
    omegaNadir = *omega;
    */
  } else if (pos.x < 0 && !isDown) {
    vel    = -vel;
    isDown = true;
  } else if (vel.x > 0 && !isDown) {
    isDown = true;
  } else if (vel.x < 0 && isDown) {
    isDown = false; 
  }
}
