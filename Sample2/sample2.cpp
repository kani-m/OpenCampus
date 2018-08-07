/*
  sample2
  内容 : ブランコのシミュレーションプログラム
  キーボード操作 :
  q - 終了
  c - ブランコの状態の切り替え
  t - ブランコの速度を上げる
  T - ブランコの速度を下げる
  f - 動きの滑らかさを下げる
  F - 動きの滑らかさを上げる
*/

/* 
  関数(コンピュータへの命令を機能ごとにまとめたもの)を呼ぶための準備 
*/
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "sample2.h"

/*
  定数
*/
#define INIT_ANGLE 60.0 // 最初のブランコの角度

/*
  変数(プログラムの設定を保存しておくためのもの)の宣言
*/
int BAR;   // 棒
int SWING; // ブランコ本体

double angle1 = INIT_ANGLE; // ブランコの角度
double key_tt = 2.0;        // ブランコの周期(端から行って戻ってくるまでの時間)
int    key_c  = 0;          // ブランコの状態
int    key_f  = 30;         // ブランコの動きの滑らかさ

double viewpoint_angle = 40.0; // ブランコを見るときの角度

/*
  プログラム本体
*/
int main(int argc, char** argv) {
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
    idle();
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
  static double  p = 0.0;            // 1往復中の割合
  static clock_t currentTime;        // 現在の時間
  static clock_t previousTime = 0.0; // 前の時間

  // ブランコの角度を決める
  if(p >= 1.0){
    p = 0.0;
  }
  p += 1/(key_tt*key_f);

  if(key_c == 0){
    // 一定の量ずつ角度が増える
    if(p <= 0.5){
      angle1 =  INIT_ANGLE - 4*INIT_ANGLE*p;
    }
    else{
      angle1 = -INIT_ANGLE + 4*INIT_ANGLE*(p-0.5);
    }
  }else if(key_c == 1){
    // 物理法則に従って角度が増える
    angle1 = INIT_ANGLE * cos(2*M_PI*p);
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
  glTranslated(0.0, 30*(1-cos(M_PI/180*angle1)), -30*sin(M_PI/180*angle1));
  glRotated(angle1, 1.0, 0.0, 0.0);
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
  case GLFW_KEY_C :
    if (action == GLFW_PRESS) {
      // ブランコの状態を変える
      key_c = (key_c == 0) ? 1 : 0;
    }
    break;  
  case GLFW_KEY_T :
    if (action == GLFW_PRESS) {
      if (mods == GLFW_MOD_SHIFT) {
	// ブランコの周期を長くする(ブランコの速度を下げる)
	key_tt += 0.5;
	if(key_tt > 10.0){
	  key_tt = 2.0;
	}
      } else {
	// ブランコの周期を短かくする(ブランコの速度を上げる)
	key_tt -= 0.5;
	if(key_tt <= 0){
	  key_tt = 2.0;
	}
      }
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
  double viewpoint_x = 150.0*sin(M_PI/180*viewpoint_angle); // 視点のx座標
  double viewpoint_y = 0.0;                                 // 視点のy座標
  double viewpoint_z = 150.0*cos(M_PI/180*viewpoint_angle); // 視点のz座標
   
  // ウィンドウの大きさに合わせて物体の大きさや視点も変更する
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glm::mat4 Projection = glm::perspective(glm::radians(40.0),
					  (GLdouble)width/(GLdouble)height,
					  5.0, 1024.0);
  glLoadMatrixf(glm::value_ptr(Projection));
  
  glMatrixMode(GL_MODELVIEW);
  glm::mat4 ModelView = glm::lookAt(glm::vec3(viewpoint_x, viewpoint_y, viewpoint_z),
				    glm::vec3(0.0, 0.0, 0.0),
				    glm::vec3(0.0, 1.0, 0.0));
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
  glVertex3d( 20,  30, 0);
  glVertex3d( 20, -30, 0);
  // 奥の紐を作成する
  glVertex3d(-20,  30, 0);
  glVertex3d(-20, -30, 0);
  glEnd();

  // ブランコの座席の側面を作成する
  glBegin(GL_QUAD_STRIP);
  glColor3fv(g_aSeatColorXZ);  // 座席の側面の色の設定をする
  // 座席の側面を作成する
  glVertex3d( 20, -30, -10);
  glVertex3d( 20, -33, -10);
  glVertex3d(-20, -30, -10);
  glVertex3d(-20, -33, -10);
  glVertex3d(-20, -30,  10);
  glVertex3d(-20, -33,  10);
  glVertex3d( 20, -30,  10);
  glVertex3d( 20, -33,  10);
  glVertex3d( 20, -30, -10);
  glVertex3d( 20, -33, -10);
  glEnd();

  // ブランコの座席の上下面を作成する
  glBegin(GL_QUADS);
  glColor3fv(g_aSeatColorY);  // 座席の上下面の色の設定をする
  // 座席の上の面を作成する
  glVertex3d(-20, -30, -10);
  glVertex3d( 20, -30, -10);
  glVertex3d( 20, -30,  10);
  glVertex3d(-20, -30,  10);
  // 座席の下の面を作成する
  glVertex3d(-20, -33, -10);
  glVertex3d( 20, -33, -10);
  glVertex3d( 20, -33,  10);
  glVertex3d(-20, -33,  10);
  glEnd();
 
  glPopMatrix();

  glEndList();
}

