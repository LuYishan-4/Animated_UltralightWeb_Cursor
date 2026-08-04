#include "GPUContextGL.h"
#include "GPUDriverGL.h"
#if defined(_WIN32)
  #include <glad/glad.h>
#else
  #include "opengl/glutils.h"
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

namespace ultralight {

GPUContextGL::GPUContextGL(bool enable_vsync, bool enable_msaa) : 
  msaa_enabled_(enable_msaa) {
  glfwSetErrorCallback(error_callback);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  if (enable_msaa) {
    // Request 4x MSAA for our window
    glfwWindowHint(GLFW_SAMPLES, 4);
  }

  // Make the window offscreen
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* win = glfwCreateWindow(10, 10, "", NULL, NULL);
  window_ = win;
  if (!window_)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window_);
#if defined(_WIN32)
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))exit(EXIT_FAILURE);
#else
#endif
  glfwSwapInterval(enable_vsync ? 1 : 0);

  int samples = 4;
  glGetIntegerv(GL_SAMPLES, &samples);
  if (!samples) {
    msaa_enabled_ = false;
  }

  if (msaa_enabled_) {
    glEnable(GL_MULTISAMPLE);
  }

  driver_.reset(new ultralight::GPUDriverGL(this));
}

}  // namespace ultralight
