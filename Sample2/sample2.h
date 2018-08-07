#ifndef __SAMPLE2_H__
#define __SAMPLE2_H__

#include <GLFW/glfw3.h>

void makeBar(GLfloat*, GLfloat*, GLfloat*);
void makeSwing(GLfloat*, GLfloat*, GLfloat*);
GLFWwindow* userInit(void);
void display(GLFWwindow*);
void idle(void);
static void keyboard(GLFWwindow* const, int, int, int, int);
static void resize(GLFWwindow* const, int, int);
int shouldClose(GLFWwindow*);
void swapBuffers(GLFWwindow*);

#endif
