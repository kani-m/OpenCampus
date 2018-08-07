#ifndef __SAMPLE1_H__
#define __SAMPLE1_H__

#include <GLFW/glfw3.h>

void InitFog(void);
void InitLight(void);
void InitTexture(void);
void InitCone(void);
void InitCone2(void);
GLFWwindow* userInit(void);
void DrawPlane(void);
void DrawCone(void);
void DrawCube(void);
void display(GLFWwindow*);
void idle(void);
static void keyboard(GLFWwindow* const, int, int, int, int);
static void resize(GLFWwindow* const, int, int);
int shouldClose(GLFWwindow*);
void swapBuffers(GLFWwindow*);

#endif
