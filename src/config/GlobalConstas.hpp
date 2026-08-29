#pragma once
#include <string>

enum class BuildType { Kwin, X11, Windows };

class GloablContast {
private:
  GloablContast() = default;

public:
#if defined(BUILD_TYPE_WINDOWS)
  inline static const BuildType buildType = BuildType::Windows;
#elif defined(BUILD_TYPE_KWIN)
  inline static const BuildType buildType = BuildType::Kwin;
#else
  inline static const BuildType buildType = BuildType::X11;
#endif

  inline static const std::string Version = "1.0.2";
};
