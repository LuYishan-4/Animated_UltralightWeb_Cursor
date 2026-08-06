#include "GPUContextGL.h"
#include "GPUDriverGL.h"

#if defined(_WIN32)
  #include <glad/glad.h>
  #ifndef GLFW_INCLUDE_NONE
    #define GLFW_INCLUDE_NONE
  #endif
  #include <GLFW/glfw3.h>
#else
  #include "opengl/glutils.h"
#endif

#include <iostream>

#if defined(_WIN32)
void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}
#endif

namespace ultralight {

GPUContextGL::GPUContextGL(bool enable_vsync, bool enable_msaa) : 
  msaa_enabled_(enable_msaa) {

#if defined(_WIN32)
  glfwSetErrorCallback(error_callback);

  if (enable_msaa) {
    glfwWindowHint(GLFW_SAMPLES, 4);
  }

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* win = glfwCreateWindow(10, 10, "", NULL, NULL);
  window_ = win;
  if (!window_)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window_);
  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    exit(EXIT_FAILURE);
  }
  
  glfwSwapInterval(enable_vsync ? 1 : 0);

#else
  (void)enable_vsync; 
#endif
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
