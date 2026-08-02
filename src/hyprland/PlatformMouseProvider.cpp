#include "PlatformMouseProvider.hpp"

#include "../lib/X11MouseProvider.hpp"

#include <hyprutils/os/Process.hpp>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <sstream>
#include <string>

namespace {

std::string trim(std::string value)
{
    const auto isSpace = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };

    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) {
        return !isSpace(ch);
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](unsigned char ch) {
        return !isSpace(ch);
    }).base(), value.end());
    return value;
}

bool parseCursorPoint(const std::string& input, UltralightWebCursorM::MousePoint& out)
{
    const auto comma = input.find(',');
    if(comma == std::string::npos)
        return false;

    const std::string xText = trim(input.substr(0, comma));
    const std::string yText = trim(input.substr(comma + 1));
    if(xText.empty() || yText.empty())
        return false;

    char* end = nullptr;
    errno = 0;
    const long x = std::strtol(xText.c_str(), &end, 10);
    if(errno != 0 || end == xText.c_str() || *end != '\0')
        return false;

    errno = 0;
    const long y = std::strtol(yText.c_str(), &end, 10);
    if(errno != 0 || end == yText.c_str() || *end != '\0')
        return false;

    out.x = static_cast<int>(x);
    out.y = static_cast<int>(y);
    out.pressed = false;
    return true;
}

bool readHyprlandCursorPosition(UltralightWebCursorM::MousePoint& out)
{
    const char* signature = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");
    const char* display = std::getenv("WAYLAND_DISPLAY");
    if((!signature || !*signature) && (!display || !*display))
        return false;

    try {
        Hyprutils::OS::CProcess process("hyprctl", {"cursorpos"});
        if(!process.runSync())
            return false;

        const std::string output = trim(process.stdOut());
        if(output.empty())
            return false;

        return parseCursorPoint(output, out);
    } catch(...) {
        return false;
    }
}

}

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize()
{
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &PlatformMouseProvider::onTimer);
    timer_.start();
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback)
{
    callback_ = std::move(callback);
}

void PlatformMouseProvider::onTimer()
{
    if(!callback_)
        return;

    UltralightWebCursorM::MousePoint point;
    if(readHyprlandCursorPosition(point)) {
        callback_(point);
        return;
    }

    if(UltralightWebCursorM::readX11CursorPosition(point))
        callback_(point);
}
