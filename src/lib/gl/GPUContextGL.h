#pragma once
#include <Ultralight/platform/GPUDriver.h>
#include <Ultralight/platform/Config.h>
#include "GPUDriverImpl.h"
#include <memory>


#if defined(_WIN32)
  typedef struct GLFWwindow GLFWwindow;
#endif

#define ENABLE_OFFSCREEN_GL 0

namespace ultralight {

class GPUContextGL {
protected:
  std::unique_ptr<ultralight::GPUDriverImpl> driver_;
  #if defined(_WIN32)
    GLFWwindow* window_;
    GLFWwindow* active_window_ = nullptr;
  #endif
  bool msaa_enabled_;
public:
  GPUContextGL(bool enable_vsync, bool enable_msaa);

  virtual ~GPUContextGL() {}

  virtual ultralight::GPUDriverImpl* driver() const { return driver_.get(); }

  virtual ultralight::FaceWinding face_winding() const { return ultralight::FaceWinding::CounterClockwise; }

  virtual void BeginDrawing() {}

  virtual void EndDrawing() {}

  virtual bool msaa_enabled() const { return msaa_enabled_; }

  unsigned int getTextureId(uint32_t ultralight_texture_id) const;

  // An offscreen window dedicated to maintaining the OpenGL context.
  // All other windows created during lifetime of the app share this context.

  #if defined(_WIN32)
    virtual GLFWwindow* window() { return window_; }

    // FBOs are not shared across contexts in OpenGL 3.2 (AFAIK), we luckily
    // don't need to share them across multiple windows anyways so we temporarily
    // set the active GL context to the "active window" when creating FBOs.
    virtual void set_active_window(GLFWwindow* win) { active_window_ = win; }

    virtual GLFWwindow* active_window() { return active_window_; }
  #endif
};

}  // namespace ultralight