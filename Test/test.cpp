/* 
  関数(コンピュータへの命令を機能ごとにまとめたもの)を呼ぶための準備 
*/
#include <cstdlib>
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Common/Window.h"
#include "Common/Shape.h"
#include "Common/ObjectData.h"

GLFWwindow* userInit(void);
void DrawCone(void);
void DrawCube(void);
void DrawPlane(void);
static void keyboard(GLFWwindow* const, int, int, int, int);
static void resize(GLFWwindow* const, int, int);
int shouldClose(GLFWwindow*);
void swapBuffers(GLFWwindow*);
void InitLight(void);
void InitTexture(void);
void InitFog(void);
void display(void);

float g_angle = 0.0f;
int g_nFogMode;
int g_nBlendMode;

/*
  プログラム本体
*/
int main(int argc, char *argv[]) {
  // プログラムを開始するための初期設定をする
  if (glfwInit() == GL_FALSE) {
    std::cerr << "Can't initialize GLFW" << std::endl;
    return 1;
  }
  atexit(glfwTerminate);
  
  // ウィンドウを作成する
  GLFWwindow* window;
  window = userInit();

  InitTexture();
  InitLight();
  InitFog();
  // 動作を開始する
  while (shouldClose(window) == GL_FALSE) {
    // Vanish the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    display();
    
    g_angle += 1.0f;
    if(g_angle > 360.0f){
      g_angle = 0.0f;
    }
    
    swapBuffers(window);
  }

  glfwDestroyWindow(window);
  
  return 0;
}

/*
  初期設定のための関数
*/
GLFWwindow* userInit(void) {
  // ウィンドウの幅と高さ
  int width  = 800;
  int height = 800;
  
  // ウィンドウを作成する
  GLFWwindow* window = glfwCreateWindow(width, height, "Blending-Fog", NULL, NULL);
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

  
  // 隠面消去(背面カリング)を有効にする
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // 隠面消去(デプスバッファ法)を有効にする
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  
  
  return window;
}

/*
  キーボード操作のための関数
*/
static void keyboard (GLFWwindow* const window, int key, int scancode, int action, int mods) {
  switch(key) {
  case GLFW_KEY_Q : 
    // プログラムを終了する
    exit(0);
    break;
  case GLFW_KEY_F :
    if (action == GLFW_PRESS) {
      // 霧のモードを変更する
      g_nFogMode = (g_nFogMode+1)%4;
      InitFog();
    }
    break;
  case GLFW_KEY_T :
    if (action == GLFW_PRESS) {
      // 物体の透明モードを変更する
      g_nBlendMode = (g_nBlendMode+1)%3;
      display();
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
  // ウィンドウの大きさに合わせて物体の大きさや視点も変更する
  glViewport(0, 0, width, height);
  
  glMatrixMode(GL_PROJECTION);
  glm::mat4 Projection = glm::perspective(glm::radians(60.0),
					  (double)width/(double)height,
					  1.0, 100.0);
  
  glLoadMatrixf(glm::value_ptr(Projection));

  /*
  float tmp[16];
  glGetFloatv(GL_PROJECTION_MATRIX, tmp);
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      std::cerr << tmp[4*i + j] << " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;
  */
  
  glMatrixMode(GL_MODELVIEW);
  glm::mat4 ModelView = glm::lookAt(glm::vec3(0.0, 1.5, 3.0),
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
  物体を表示するための関数
*/
void display(void) {
  float ConeDiff[] = {1.0, 1.0, 0.0, 1.0}; // コーンの色
  float CubeDiff[] = {0.0, 0.0, 1.0, 1.0}; // 立方体の色

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glRotated(g_angle, 0.0, 1.0, 0.0); // 全体を回転させる

  DrawPlane(); // 床を表示する

  // 物体の透明モードごとに表示方法を変える
  switch(g_nBlendMode){
  case 0:
    // コーンを表示する
    ConeDiff[3] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , ConeDiff);
    DrawCone();

    // 立方体を表示する
    CubeDiff[3] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , CubeDiff);
    DrawCube();
    break;
  case 1:
    // コーンを表示する
    ConeDiff[3] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , ConeDiff);
    DrawCone();

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 立方体を表示する
    CubeDiff[3] = 0.5; // 立方体を透明にする
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , CubeDiff);
    DrawCube();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    break;
  case 2:
    // 立方体を表示する
    CubeDiff[3] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , CubeDiff);
    DrawCube();

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // コーンを表示する
    ConeDiff[3] = 0.5; // コーンを透明にする
    glMaterialfv(GL_FRONT_AND_BACK , GL_DIFFUSE , ConeDiff);
    DrawCone();
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);    
    break;
  }
  glPopMatrix();
}

/*
  コーンを描画するための関数
*/
void DrawCone(void) {
  // 三角柱
  static const GLfloat vtx3[] = {
    +0.0f, 0.7f, +0.0f,
    -0.5f, 0.0f, +0.5f,
    +0.5f, 0.0f, +0.5f,
    +0.0f, 0.0f, -0.5f,
    -0.5f, 0.0f, +0.5f,
  };  
  
  glPushMatrix();
  glRotated(-90.0, 0.0, 1.0, 0.0); // コーンを回転させる
  // コーンを描画する
  glVertexPointer(3, GL_FLOAT, 0, vtx3);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
  glDisableClientState(GL_VERTEX_ARRAY);
  
  glPopMatrix();
}

/*
  立方体を描画するための関数
*/
void DrawCube(void) {
  static const GLfloat cube[] = {
    -0.5f, +0.5f, +0.5f,
    +0.5f, +0.5f, +0.5f,
    +0.5f, +0.5f, -0.5f,
    -0.5f, +0.5f, -0.5f,
    -0.5f, -0.5f, +0.5f,
    +0.5f, -0.5f, +0.5f,
    +0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
  };

  static const GLuint index[] = {
    // top
    0, 1, 3,
    1, 2, 3,
    // left
    0, 7, 4,
    0, 3, 7,
    // front
    0, 4, 1,
    1, 4, 5,
    // right
    1, 5, 2,
    2, 5, 6,
    // back
    2, 7, 3,
    2, 6, 7,
    // bottom
    4, 6, 5,
    4, 7, 6,
  };
  
  
  glPushMatrix();
  glTranslated(0.0, 0.5, -1.0); // 立方体を移動させる

  glVertexPointer(3, GL_FLOAT, 0, cube);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, index);
  glDisableClientState(GL_VERTEX_ARRAY);

  glPopMatrix();
}

/*
  霧の設定のための関数
*/
void InitFog() {
  GLint   g_aFogTable[3] = {GL_EXP, GL_EXP2, GL_LINEAR}; // 霧の種類
  GLfloat g_aFogColor[4] = {0.5, 0.5, 0.5, 1.0};         // 霧の色

  switch(g_nFogMode){
  case 0:
    // 霧をなくす
    glDisable(GL_FOG);
    break;
  case 1:
  case 2:
  case 3:
    // 霧の色、濃さ、広がりを決める
    glFogi(GL_FOG_MODE, g_aFogTable[g_nFogMode-1]);
    glFogfv(GL_FOG_COLOR, g_aFogColor);
    glFogf(GL_FOG_DENSITY, 0.35);
    glFogf(GL_FOG_START, 1.0);
    glFogf(GL_FOG_END, 5.0);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glEnable(GL_FOG);
    break;
  }
}

/*
  光の設定のための関数
*/
void InitLight(void) {
  float lightPosition[]  = {0.0, 3.0, 3.0, 1.0}; // 光の位置 
  float lightDiffuse[]   = {1.0, 1.0, 1.0, 1.0}; // 拡散光(上で決めた位置から来る光)の色
  float lightAmbient[]   = {0.0, 0.0, 0.0, 1.0}; // 環境光(全体にまんべんなく当たる光)の色
  float lightSpecular[]  = {1.0, 1.0, 1.0, 1.0}; // 鏡面光(反射する光)の色
  
  // 光の位置、色を決める
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
}

/*
  床の模様の設定
*/
void InitTexture(void) {
  GLuint  textureID;       // 作成する模様の番号
  GLubyte data[64][64][3]; // 模様そのもの
  int     i, j, c;

  // 市松模様を作成する
  for (i = 0; i < 64; i++) {
    for (j = 0; j < 64; j++) {
      c = (i&0x8)^(j&0x8);
      data[i][j][0] = c ? 255 : 0; // 赤色の量を決める
      data[i][j][1] = c ? 255 : 0; // 緑色の量を決める
      data[i][j][2] = c ? 255 : 0; // 青色の量を決める
    }
  }

  // 模様の初期設定をする
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

/*
  床を描画するための関数
*/
void DrawPlane(void) {
  float diff[] = {1.0, 1.0, 1.0, 1.0}; // 拡散光の反射係数
  float spec[] = {0.0, 0.0, 0.0, 1.0}; // 鏡面反射係数(ハイライトの強さ)
  
  // 模様を使用する
  if (glIsEnabled(GL_TEXTURE_2D) != GL_TRUE){
    glEnable(GL_TEXTURE_2D);
  }

  // 床の材質を設定する
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);

  // 床を描画する
  glBegin(GL_QUADS);
  glNormal3d(0.0, 1.0, 0.0);
  glTexCoord2d(0.0, 0.0); glVertex3d(-5.0, 0.0,-5.0);
  glTexCoord2d(0.0, 2.0); glVertex3d(-5.0, 0.0, 5.0);
  glTexCoord2d(2.0, 2.0); glVertex3d( 5.0, 0.0, 5.0);
  glTexCoord2d(2.0, 0.0); glVertex3d( 5.0, 0.0,-5.0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}
