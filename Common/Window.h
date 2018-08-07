#pragma once
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
  DC  - device coordinate
  NDC - normalized device coordinate
  WC  - world coordinate
 */

// Processing of window
class Window {
  // Handle of window
  GLFWwindow* const window;

  // Aspect ratio
  // GLfloat aspect;

  // Size of window
  GLfloat size[2];

  // Scale of device coordinate to world coordinate
  GLfloat scale;

  // Position of object on normalized device coordinate (clipping coordinate)
  GLfloat location[2];

  // Status of keyboard
  int keyStatus;
  
 public:
  // Constructor
  Window (int width = 640, int height = 480, const char* title = "Hello!")
    : window(glfwCreateWindow(width, height, title, NULL, NULL)),
    scale(100.0f), location{0, 0}, keyStatus(GLFW_RELEASE) {
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

    // Record this pointer of instance
    glfwSetWindowUserPointer(window, this);
    
    // Register processing at changing window size
    glfwSetWindowSizeCallback(window, resize);

    // Register processing at using mouse wheel
    glfwSetScrollCallback(window, wheel);

    // Register processing at using keyboard
    glfwSetKeyCallback(window, keyboard);
    
    // Initialize setting of opened window
    resize(window, width, height);
  }

  // Destructor
  virtual ~Window () {
    glfwDestroyWindow(window);
  }
  
  // Decide if window should be closed
  int shouldClose () const {
    return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
  }

  // Get events after swapping color buffer
  void swapBuffers () {
    // Swap color buffer
    glfwSwapBuffers(window);

    // Get some events
    /*
    if (keyStatus == GLFW_RELEASE) {
      glfwWaitEvents();
    } else {
      glfwPollEvents();
    }
    */
    glfwPollEvents();

    // Check status of keyboard
    if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE) {
      location[0] -= 2.0f / size[0];
    } else if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE) {
      location[0] += 2.0f / size[0];
    }
    
    if (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE) {
      location[1] -= 2.0f / size[1];
    } else if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE) {
      location[1] += 2.0f / size[1];
    }
    
    // Check status of left button of mouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE) {
      // Get position of mouse cursor on DC if left button of mouse is pressed
      double x, y;
      glfwGetCursorPos(window, &x, &y);

      // Compute position of mouse cursor on NDC
      location[0] =        static_cast<GLfloat>(x) * 2.0f / size[0] - 1.0f;
      location[1] = 1.0f - static_cast<GLfloat>(y) * 2.0f / size[1];
    }
  }

  // Get aspect ratio
  /*
  GLfloat getAspect () const {
    return aspect;
  }
  */

  // Get size of window
  const GLfloat* getSize () const {
    return size;
  }

  // Get scale of device coordinate to world coordinate
  GLfloat getScale () const {
    return scale;
  }

  // Get position
  const GLfloat* getLocation () const {
    return location;
  }

  // Set size of window
  void setSize (int width, int height) {
    glfwSetWindowSize(this->window, width, height);
    //resize(this->window, width, height);
  }

  // Set title of window
  void setTitle (const char* title) {
    glfwSetWindowTitle(this->window, title);
  }
  
  // Processing at changing window size
  static void resize (GLFWwindow* const window, int width, int height) {
    // Set whole window to viewport
    glViewport(0, 0, width, height);

    // Get this pointer of instance
    Window* const instance(static_cast<Window*>(glfwGetWindowUserPointer(window)));

    if (instance != NULL) {
      // Update aspect ratio of instance
      // instance->aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

      // Save size of opened window
      instance->size[0] = static_cast<GLfloat>(width);
      instance->size[1] = static_cast<GLfloat>(height);
    }
  }

  // Processing at using mouse wheel
  static void wheel (GLFWwindow* const window, double x, double y) {
    // Get this pointer of instance
    Window* const instance(static_cast<Window*>(glfwGetWindowUserPointer(window)));

    if (instance != NULL) {
      // Update scale of DC to WC
      instance->scale += static_cast<GLfloat>(y);
    }
  }

  // Processing at using keyboard
  static void keyboard (GLFWwindow* const window, int key, int scancode, int action, int mods) {
    // Get this pointer of instance
    Window* const instance(static_cast<Window*>(glfwGetWindowUserPointer(window)));

    if (instance != NULL) {
      // Save status of keyboard
      instance->keyStatus = action;
    }
  }
};
