#pragma once




#include <Ultralight/platform/GPUDriver.h>
#include <Ultralight/platform/Config.h>
#include "GPUDriverImpl.h"
#include <memory>


#define ENABLE_OFFSCREEN_GL 0


#if defined(_WIN32)
struct GLFWwindow; 
#else
class QOpenGLContext;
#endif

namespace ultralight {

class GPUContextGL {
protected:
  std::unique_ptr<ultralight::GPUDriverImpl> driver_;
  bool msaa_enabled_;
public:
  GPUContextGL(bool enable_vsync, bool enable_msaa);

  

  virtual ~GPUContextGL() {}

  virtual ultralight::GPUDriverImpl* driver() const { return driver_.get(); }

  virtual ultralight::FaceWinding face_winding() const { return ultralight::FaceWinding::CounterClockwise; }

  virtual void BeginDrawing() {}

  virtual void EndDrawing() {}

  virtual bool msaa_enabled() const { return msaa_enabled_; }

};

}  // namespace ultralight