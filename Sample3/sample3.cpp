#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
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
void DrawYoyo(GLfloat*, GLfloat*);
void DrawString(GLfloat*);

/*
  変数(プログラムの設定を保存しておくためのもの)の宣言
*/
int BAR;   // 棒
int SWING; // ブランコ本体

int    g_nAnim = GL_TRUE; // 一時停止しているかどうか
int    key_c   = 1;       // ブランコの状態
int    key_f   = 30;      // ブランコの動きの滑らかさ

double viewpoint_angle = 40.0; // ブランコを見るときの角度

glm::vec3     x = {  0.0f, 0.0f, 0.0f}; // Position of yoyo
glm::vec3     v = {  0.0f, 0.0f, 0.0f}; // Velocity of yoyo
glm::vec3 theta = {  0.0f, 0.0f, 0.0f}; // Rotation angle    of yoyo
glm::vec3 omega = {  0.0f, 0.0f, 0.0f}; // Angular  velocity of yoyo
float         t = 0.0f;             // Time
float        dt = pow(2.0f, -5.0f); // Time step

const float rInner =   5.0f; // Radius of inner cylinder of yoyo
const float rOuter =  25.0f; // Radius of outer cylinder of yoyo
const float      L = 400.0f; // Length of string
glm::vec3     fExt = glm::vec3(0.0f); // External force

static bool isDown  = true; // Up   down  flag, up   - false, down  - true
static bool isRight = true; // Left right flag, left - false, right - true

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
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

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
      isDown = !isDown;
    }
    if (isDown && x.x > 100.0f) {
      x.x = 100.0f;
    } else if (!isDown && x.x < 0.0f) {
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
  GLfloat g_aStringColor[3]   = {1.00, 0.00, 1.00}; // 紐の色
  GLfloat g_aYoyoColorInner[3] = {0.00, 1.00, 0.00}; // 座席の側面の色
  GLfloat g_aYoyoColorOuter[3]  = {0.00, 0.00, 1.00}; // 座席の上下面の色

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glRotated(viewpoint_angle, 1.0, 0.0, 0.0);
  
  glPushMatrix();
  // 全体をずらす
  glTranslated(-L/2.0f, 0.0, 0.0);
  
  // 棒を表示する
  DrawString(g_aStringColor);

  // ブランコ本体を表示する
  glTranslated(x.x, x.y, x.z);
  glRotated(glm::degrees(theta.z), 0.0, 0.0, 1.0);
  DrawYoyo(g_aYoyoColorInner, g_aYoyoColorOuter);

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
  /*
  double viewpoint_x = -150.0*sin(M_PI/180.0*viewpoint_angle); // 視点のx座標
  double viewpoint_y =    0.0;                                 // 視点のy座標
  double viewpoint_z =  150.0*cos(M_PI/180.0*viewpoint_angle); // 視点のz座標
  */
  double viewpoint_x =   0.0f;
  double viewpoint_y =   0.0f;
  double viewpoint_z = L*1.5f;
  
  // ウィンドウの大きさに合わせて物体の大きさや視点も変更する
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glm::mat4 Projection = glm::perspective(glm::radians(40.0),
					  (GLdouble)width/(GLdouble)height,
					  5.0, (GLdouble)L*2.0);
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

  if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE) {
    viewpoint_angle -= 10.0;
  } else if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE) {
    viewpoint_angle += 10.0;
  }
  
  const float  m = 100.0f; // Mass
  const float  g = 980.0f; // Gravitational acceleration
  fExt = static_cast<float>(glfwGetKey(window, GLFW_KEY_SPACE)) * (x.x > 0) * glm::vec3({-10.0f*m*g, 0.0f, 0.0f});
}

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

/*
  ヨーヨーの紐を描画するための関数
 */
void DrawString (GLfloat* g_aStringColor) {
  glPushMatrix();
  
  // ヨーヨーの紐を作成する
  glLineWidth(5.0);           // 紐の太さの設定をする
  glBegin(GL_LINES);
  glColor3fv(g_aStringColor); // 紐の色の設定をする
  glVertex3d(   0, 0, 0);
  glVertex3d( x.x, 0, 0);
  glEnd();

  glPopMatrix();
}

/*
  ヨーヨー本体を描画するための関数
*/
void DrawYoyo(GLfloat* g_aYoyoColorInner, GLfloat* g_aYoyoColorOuter) {
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

  glPushMatrix();
  glLineWidth(5.0);           // 紐の太さの設定をする
  glBegin(GL_LINES);
  glColor3f(1.0f, 0.0f, 0.0f); // 紐の色の設定をする
  glVertex3f(       0.0f, 0.0f, 0.0f);
  glVertex3f(rOuter*2.0f, 0.0f, 0.0f);
  glEnd();
  glPopMatrix();
  
  glPopMatrix();
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
  //const float dt =  pow(2.0f, -3.0f); // Time step
  const float  m = 100.0f; // Mass
  const float  g = 980.0f; // Gravitational acceleration
  const float  a = rInner; // Radius of inner cylinder
  
  //const glm::mat3 identity = glm::mat3(1.0f);
  
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

  //const float tau = 0.5f * (1.148f * 0.001f) * (M_PI*pow(rOuter, 2.0f) * 2.0f) * 1.2;
  float tau = 0.0f;
  
  // Runge-Kutta method
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func1 = [&f,&m,&tau](float t, glm::vec3 x, glm::vec3 v){ return (f+glm::vec3(-glm::sign(v.x)*tau*v.x*v.x, 0.0f, 0.0f))/m; };
  std::function<glm::vec3(float,glm::vec3,glm::vec3)> func2 = [          ](float t, glm::vec3 x, glm::vec3 v){ return v; };
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
  std::cout << (isDown ? "Down" : "Up") << " " << (isRight ? "Right" : "Left") << std::endl;
  std::cout << glm::to_string(fExt) << std::endl;
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
  if (x->x > L && isDown) {
    *v      = -(*v);
    isDown  = false;
    isRight = !isRight;
  } else if (x->x < 0 && !isDown) {
    *v      = -(*v);
    isDown  = true;
  } else if (v->x > 0 && !isDown) {
    isDown  = true;
  }
}
