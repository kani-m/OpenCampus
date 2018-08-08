/*
  sample1
  内容 : CG(コンピュータグラフィックス)の入門プログラム
  キーボード操作 :
  q - 終了
  a - 一時停止
  f - 霧のモード切り替え
  t - 物体の透明モード切り替え
*/

/* 
  関数(コンピュータへの命令を機能ごとにまとめたもの)を呼ぶための準備 
*/
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "sample1.h"

/*
  変数(プログラムの設定を保存しておくためのもの)の宣言
*/
double g_angle      = 0.0;      // 全体の角度
int    g_nAnim      = GL_TRUE;  // 一時停止しているかどうか
int    g_nFogMode   = 0;        // 霧のモード
int    g_nBlendMode = 0;        // 物体の透明モード
int    g_nShadeMode = 0;        // 陰影処理のモード
int    g_nConeMode  = 0;        // コーンのメッシュの切り方
int    g_nPartition = 32;       // コーンのメッシュの分割数
int    g_nMesh      = GL_FALSE; // メッシュの更新があるかどうか

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

  // 動作を開始する
  int g_nOldConeMode = g_nConeMode;
  while (shouldClose(window) == GL_FALSE) {
    if (g_nAnim) {
      idle();
    } else {
      // 一時停止中に限り、コーンのメッシュの切り方を変更する
      if (g_nOldConeMode != g_nConeMode || g_nMesh) {
	if (g_nConeMode) {
	  InitCone2();
	} else {
	  InitCone();
	}
	g_nOldConeMode = g_nConeMode;
	g_nMesh        = GL_FALSE;
      }
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

  InitLight();   // 光の設定をする
  InitTexture(); // 床の模様の設定をする
  InitFog();     // 霧の設定をする

  InitCone();    // コーンの形状の設定をする
  
  return window;
}

/*
  一定時間ごとに動作する関数
*/
void idle(void) {
  // 全体の角度を変えて再表示する
  g_angle += 1.0;
  if (g_angle >= 360.0){
    g_angle = 0.0;
  }
}

/*
  物体を表示するための関数
*/
void display(GLFWwindow* window) {
  float ConeDiff[] = {1.0, 1.0, 0.0, 1.0}; // コーンの色
  float CubeDiff[] = {0.0, 0.0, 1.0, 1.0}; // 立方体の色

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (g_nShadeMode) {
    glShadeModel(GL_SMOOTH);
  } else {
    glShadeModel(GL_FLAT);
  }
  
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
      // コーンのメッシュの切り方を変更する
      g_nConeMode = !g_nConeMode;
    }
    break;
  case GLFW_KEY_F :
    if (action == GLFW_PRESS) {
      // 霧のモードを変更する
      g_nFogMode = (g_nFogMode+1)%4;
      InitFog();
      display(window);
    }
    break;
  case GLFW_KEY_N :
    if (action == GLFW_PRESS) {
      // メッシュの分割数を変更する
      if (mods == GLFW_MOD_SHIFT) {
	g_nPartition *= 2;
	if (g_nPartition > 128) {
	  g_nPartition = 128;
	}
      } else {
	g_nPartition /= 2;
	if (g_nPartition < 4) {
	  g_nPartition = 4;
	}
      }
      g_nMesh = GL_TRUE;
    }
    break;
  case GLFW_KEY_S :
    if (action == GLFW_PRESS) {
      // 陰影処理のモードを変更する
      g_nShadeMode = !g_nShadeMode;
      display(window);
    }
    break;
  case GLFW_KEY_T :
    if (action == GLFW_PRESS) {
      // 物体の透明モードを変更する
      g_nBlendMode = (g_nBlendMode+1)%3;
      display(window);
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
					  (GLdouble)width/(GLdouble)height,
					  1.0, 100.0);
  glLoadMatrixf(glm::value_ptr(Projection));
  
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
  float lightPosition[]  = {0.0, 3.0, 1.0, 1.0}; // 光の位置 
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

/*
  コーンの形状を初期化するための関数
  - 円錐の部分を三角形で雑に近似するプラン
 */
std::vector<GLuint>    solidConeIndex;  // 描画される各面の頂点番号
std::vector<glm::vec3> solidConeVertex; // 描画される各面の頂点座標
std::vector<glm::vec3> solidConeNormal; // 描画される各面の頂点法線
void InitCone(void) {
  const int     N(g_nPartition);   // 分割数
  const GLfloat r(0.5f); // 円錐の底面の半径
  const GLfloat h(0.7f); // 円錐の高さ

  solidConeIndex.clear();
  solidConeVertex.clear();
  solidConeNormal.clear();
  
  std::vector<glm::vec3> solidConeVertexCoord; // 頂点座標
  solidConeVertexCoord.emplace_back((glm::vec3){0.0f, h, 0.0f});    // 円錐の頂点
  solidConeVertexCoord.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f}); // 円錐の底面の中心

  std::vector<glm::vec3> solidConeVertexNormal; // 頂点法線
  // 法線の初期化
  solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});
  solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});
  
  for (int i = 0; i <= N; ++i) {
    // 円錐の底面の円周をN分割した点
    GLfloat t = 2.0f * M_PI * static_cast<GLfloat>(i) /  static_cast<GLfloat>(N);
    GLfloat x = r*sin(t);
    GLfloat y = 0.0f;
    GLfloat z = r*cos(t);

    glm::vec3 v = {x, y, z};
    solidConeVertexCoord.emplace_back(v);
    solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});
  }

  std::vector<GLuint> solidConeVertexIndex;  // 頂点番号
  // 円錐を近似するN角錐の各面における頂点座標と頂点番号を計算する
  for (int i = 0; i < N; ++i) {
    solidConeVertex.emplace_back(solidConeVertexCoord[          0]);
    solidConeVertex.emplace_back(solidConeVertexCoord[ i   %N + 2]);
    solidConeVertex.emplace_back(solidConeVertexCoord[(i+1)%N + 2]);

    solidConeVertexIndex.emplace_back(          0);
    solidConeVertexIndex.emplace_back( i   %N + 2);
    solidConeVertexIndex.emplace_back((i+1)%N + 2);
    
    solidConeIndex.emplace_back(3*i + 0);
    solidConeIndex.emplace_back(3*i + 1);
    solidConeIndex.emplace_back(3*i + 2);
  }
  // 円錐の底面を近似するN角形の各面における頂点座標と頂点番号を計算する
  for (int i = 0; i < N; ++i) {
    solidConeVertex.emplace_back(solidConeVertexCoord[          1]);
    solidConeVertex.emplace_back(solidConeVertexCoord[(i+1)%N + 2]);
    solidConeVertex.emplace_back(solidConeVertexCoord[ i   %N + 2]);

    solidConeVertexIndex.emplace_back(          1);
    solidConeVertexIndex.emplace_back((i+1)%N + 2);
    solidConeVertexIndex.emplace_back( i   %N + 2);
    
    solidConeIndex.emplace_back(3*(i+N) + 0);
    solidConeIndex.emplace_back(3*(i+N) + 1);
    solidConeIndex.emplace_back(3*(i+N) + 2);
  }

  // 各面法線に対し、面積での重み付け平均を行い、頂点法線を計算する
  // Normal_vS = Sum_{v in S} Area(S)*Normal_S
  // Normal_v  = Normalize(Normal_vS)
  int size = solidConeIndex.size();
  for (int i = 0; i < size; i += 3) {
    glm::vec3 p = solidConeVertex[solidConeIndex[i  ]];
    glm::vec3 u = solidConeVertex[solidConeIndex[i+1]];
    glm::vec3 v = solidConeVertex[solidConeIndex[i+2]];

    glm::vec3 S = glm::cross(u - p, v - p); // 絶対値が平行四辺形の面積
    glm::vec3 n = glm::normalize(S);        // 面法線
    solidConeVertexNormal[solidConeVertexIndex[i  ]] += glm::abs(S)*n;
    solidConeVertexNormal[solidConeVertexIndex[i+1]] += glm::abs(S)*n;
    solidConeVertexNormal[solidConeVertexIndex[i+2]] += glm::abs(S)*n;
  }
  size = solidConeVertexNormal.size();
  for (int i = 0; i < size; ++i) {
    solidConeVertexNormal[i] = glm::normalize(solidConeVertexNormal[i]);
    std::cerr << i << " : " << glm::to_string(solidConeVertexNormal[i]) << std::endl;
  }
  
  // 描画される各面の頂点法線を求める
  const float smoothingAngle(M_PI / 4); // スムージング角
  const float threshold(cos(smoothingAngle));
  int vCnt = 0; int sCnt = 0;
  size = solidConeIndex.size();
  for (int i = 0; i < size; i += 3) {
    // そのまま頂点法線を代入する
    /*
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i  ]]);
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i+1]]);
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i+2]]);
    */
    // スムージング角を使い、面法線か頂点法線か選択する
    glm::vec3 p = solidConeVertex[solidConeIndex[i  ]];
    glm::vec3 u = solidConeVertex[solidConeIndex[i+1]];
    glm::vec3 v = solidConeVertex[solidConeIndex[i+2]];

    glm::vec3 n   = glm::normalize(glm::cross(u - p, v - p)); // 面法線
    glm::vec3 nv0 = solidConeVertexNormal[solidConeVertexIndex[i  ]];
    glm::vec3 nv1 = solidConeVertexNormal[solidConeVertexIndex[i+1]];
    glm::vec3 nv2 = solidConeVertexNormal[solidConeVertexIndex[i+2]];
    
    if (threshold > glm::dot(n, nv0)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv0);
      vCnt++;
    }

    if (threshold > glm::dot(n, nv1)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv1);
      vCnt++;
    }

    if (threshold > glm::dot(n, nv2)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv2);
      vCnt++;
    }
  }
  std::cerr << "SurfaceNormal : " << sCnt << std::endl;
  std::cerr << "VertexNormal  : " << vCnt << std::endl;
}

/*
  コーンの形状を初期化するための関数
  - 円錐の部分をメッシュで切ってちゃんと近似するプラン
 */
void InitCone2(void) {
  const int     slices(g_nPartition);    // 円周の分割数
  const int     stacks(g_nPartition/2);  // 高さの分割数
  const GLfloat radius(0.5f);            // 円錐の底面の半径
  const GLfloat height(0.7f);            // 円錐の高さ

  solidConeIndex.clear();
  solidConeVertex.clear();
  solidConeNormal.clear();
  
  std::vector<glm::vec3> solidConeVertexCoord; // 頂点座標
  solidConeVertexCoord.emplace_back((glm::vec3){0.0f, height, 0.0f}); // 円錐の頂点
  solidConeVertexCoord.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});   // 円錐の底面の中心
  
  std::vector<glm::vec3> solidConeVertexNormal; // 頂点法線
  // 法線の初期化
  solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});
  solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});

  for (int j = 1; j <= stacks; ++j) {
    GLfloat s = static_cast<float>(stacks - j) / static_cast<float>(stacks);
    GLfloat y =      s *height;
    GLfloat r = (1 - s)*radius;
    for (int i = 0; i < slices; ++i) {
      // 部分的な円錐の底面の円周をN分割した点
      GLfloat t = static_cast<GLfloat>(i) /  static_cast<GLfloat>(slices);
      GLfloat x = r*sin(2.0f * M_PI * t);
      GLfloat z = r*cos(2.0f * M_PI * t);
      
      glm::vec3 v = {x, y, z};
      solidConeVertexCoord.emplace_back(v);
      solidConeVertexNormal.emplace_back((glm::vec3){0.0f, 0.0f, 0.0f});
    }
  }

  std::vector<GLuint> solidConeVertexIndex;  // 頂点番号
  // 円錐の頂点付近を近似するN角錐の各面における頂点座標と頂点番号を計算する
  for (int i = 0; i < slices; ++i) {
    solidConeVertex.emplace_back(solidConeVertexCoord[               0]);
    solidConeVertex.emplace_back(solidConeVertexCoord[ i   %slices + 2]);
    solidConeVertex.emplace_back(solidConeVertexCoord[(i+1)%slices + 2]);

    solidConeVertexIndex.emplace_back(               0);
    solidConeVertexIndex.emplace_back( i   %slices + 2);
    solidConeVertexIndex.emplace_back((i+1)%slices + 2);
    
    solidConeIndex.emplace_back(3*i + 0);
    solidConeIndex.emplace_back(3*i + 1);
    solidConeIndex.emplace_back(3*i + 2);
  }
  // 円錐の大部分を近似するメッシュの各面における頂点座標と頂点番号を計算する
  for (int j = 1; j < stacks; ++j) {
    int k = (j - 1)*slices + 2;
    for (int i = 0; i < slices; ++i) {
      GLuint k0 = k  +  i;
      GLuint k1 = k  + (i + 1)%slices;
      GLuint k2 = k0 + slices;
      GLuint k3 = k1 + slices;

      // 左下の三角形
      solidConeVertex.emplace_back(solidConeVertexCoord[k0]);
      solidConeVertex.emplace_back(solidConeVertexCoord[k2]);
      solidConeVertex.emplace_back(solidConeVertexCoord[k3]);
      
      solidConeVertexIndex.emplace_back(k0);
      solidConeVertexIndex.emplace_back(k2);
      solidConeVertexIndex.emplace_back(k3);
      
      solidConeIndex.emplace_back(6*i + 0 + (j - 1)*3*2*slices + 3*slices);
      solidConeIndex.emplace_back(6*i + 1 + (j - 1)*3*2*slices + 3*slices);
      solidConeIndex.emplace_back(6*i + 2 + (j - 1)*3*2*slices + 3*slices);

      // 右上の三角形
      solidConeVertex.emplace_back(solidConeVertexCoord[k0]);
      solidConeVertex.emplace_back(solidConeVertexCoord[k3]);
      solidConeVertex.emplace_back(solidConeVertexCoord[k1]);
      
      solidConeVertexIndex.emplace_back(k0);
      solidConeVertexIndex.emplace_back(k3);
      solidConeVertexIndex.emplace_back(k1);
      
      solidConeIndex.emplace_back(6*i + 3 + (j - 1)*3*2*slices + 3*slices);
      solidConeIndex.emplace_back(6*i + 4 + (j - 1)*3*2*slices + 3*slices);
      solidConeIndex.emplace_back(6*i + 5 + (j - 1)*3*2*slices + 3*slices);      
    }
  }
  // 円錐の底面を近似するN角形の各面における頂点座標と頂点番号を計算する
  int kCircle = (stacks - 1)*slices + 2;
  for (int i = 0; i < slices; ++i) {
    GLuint k0 = kCircle +  i;
    GLuint k1 = kCircle + (i + 1)%slices;

    solidConeVertex.emplace_back(solidConeVertexCoord[ 1]);
    solidConeVertex.emplace_back(solidConeVertexCoord[k1]);
    solidConeVertex.emplace_back(solidConeVertexCoord[k0]);

    solidConeVertexIndex.emplace_back( 1);
    solidConeVertexIndex.emplace_back(k1);
    solidConeVertexIndex.emplace_back(k0);
    
    solidConeIndex.emplace_back(3*i + 0 + (stacks - 1)*3*2*slices + 3*slices);
    solidConeIndex.emplace_back(3*i + 1 + (stacks - 1)*3*2*slices + 3*slices);
    solidConeIndex.emplace_back(3*i + 2 + (stacks - 1)*3*2*slices + 3*slices);
  }

  // 各面法線に対し、面積での重み付け平均を行い、頂点法線を計算する
  // Normal_vS = Sum_{v in S} Area(S)*Normal_S
  // Normal_v  = Normalize(Normal_vS)
  int size = solidConeIndex.size();
  for (int i = 0; i < size; i += 3) {
    glm::vec3 p = solidConeVertex[solidConeIndex[i  ]];
    glm::vec3 u = solidConeVertex[solidConeIndex[i+1]];
    glm::vec3 v = solidConeVertex[solidConeIndex[i+2]];

    glm::vec3 S = glm::cross(u - p, v - p); // 絶対値が平行四辺形の面積
    glm::vec3 n = glm::normalize(S);        // 面法線
    solidConeVertexNormal[solidConeVertexIndex[i  ]] += glm::abs(S)*n;
    solidConeVertexNormal[solidConeVertexIndex[i+1]] += glm::abs(S)*n;
    solidConeVertexNormal[solidConeVertexIndex[i+2]] += glm::abs(S)*n;
  }
  size = solidConeVertexNormal.size();
  for (int i = 0; i < size; ++i) {
    solidConeVertexNormal[i] = glm::normalize(solidConeVertexNormal[i]);
    std::cerr << i << " : " << glm::to_string(solidConeVertexNormal[i]) << std::endl;
  }
  
  // 描画される各面の頂点法線を求める
  const float smoothingAngle(M_PI / 4); // スムージング角
  const float threshold(cos(smoothingAngle));
  int vCnt = 0; int sCnt = 0;
  size = solidConeIndex.size();
  for (int i = 0; i < size; i += 3) {
    // そのまま頂点法線を代入する
    /*
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i  ]]);
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i+1]]);
    solidConeNormal.emplace_back(solidConeVertexNormal[solidConeVertexIndex[i+2]]);
    */
    // スムージング角を使い、面法線か頂点法線か選択する
    glm::vec3 p = solidConeVertex[solidConeIndex[i  ]];
    glm::vec3 u = solidConeVertex[solidConeIndex[i+1]];
    glm::vec3 v = solidConeVertex[solidConeIndex[i+2]];

    glm::vec3 n   = glm::normalize(glm::cross(u - p, v - p)); // 面法線
    glm::vec3 nv0 = solidConeVertexNormal[solidConeVertexIndex[i  ]];
    glm::vec3 nv1 = solidConeVertexNormal[solidConeVertexIndex[i+1]];
    glm::vec3 nv2 = solidConeVertexNormal[solidConeVertexIndex[i+2]];
    
    if (threshold > glm::dot(n, nv0)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv0);
      vCnt++;
    }

    if (threshold > glm::dot(n, nv1)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv1);
      vCnt++;
    }

    if (threshold > glm::dot(n, nv2)) {
      solidConeNormal.emplace_back(n);
      sCnt++;
    } else {
      solidConeNormal.emplace_back(nv2);
      vCnt++;
    }
  }
  std::cerr << "SurfaceNormal : " << sCnt << std::endl;
  std::cerr << "VertexNormal  : " << vCnt << std::endl;
}

/*
  コーンを描画するための関数
*/
void DrawCone(void) {  
  glPushMatrix();

  float theta = -90.0;
  glTranslated(0.0, 0.7, 0.0);
  glRotated(theta, 0.0, 0.0, 1.0); // コーンを回転させる

  // コーンを描画する  
  glVertexPointer(3, GL_FLOAT, 0, solidConeVertex.data());
  glNormalPointer(   GL_FLOAT, 0, solidConeNormal.data());
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(solidConeIndex.size()),
		 GL_UNSIGNED_INT, solidConeIndex.data());
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  
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
  glTranslated(0.0, 0.5, 1.5); // 立方体を移動させる
  // 立方体を描画する
  glVertexPointer(3, GL_FLOAT, 0, cube);
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, index);
  glDisableClientState(GL_VERTEX_ARRAY);

  glPopMatrix();
}
