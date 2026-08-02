#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <filesystem>
#include <memory>

#include "../config/UserConfig.hpp"
#include "../config/CursorJSON.hpp"
#include "../lib/WebCall/WebCall.hpp"
#include "../header/UltralightHtmlEffect.hpp"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    auto appDir = std::filesystem::current_path();
    UltralightWebCursorM::g_sdkInitialPath = appDir / "sdk" / "ultralight-free-sdk-1.4.0-win-x64";
    UltralightWebCursorM::g_htmlInitialPath = appDir / "WebCursor";

    if (!UltralightWebCursorM::UserConfig::instance()->load()) {
        qCritical() << "Failed to load user config for Windows version.";
        return 1;
    }

    std::filesystem::path htmlPath(UltralightWebCursorM::UserConfigimp.html);
    UltralightWebCursorM::CursorJSON::instance()->load(htmlPath.parent_path().string());

    auto effect = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
    if (!effect->initialize(UltralightWebCursorM::UserConfigimp, UltralightWebCursorM::CursorJSONImp)) {
        qCritical() << "Failed to initialize Ultralight HTML effect for Windows version.";
        return 1;
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        effect->update();
        if (effect->hasNewFrame()) {
            qDebug() << "Windows effect frame ready:" << effect->width() << "x" << effect->height();
            effect->clearNewFrame();
        }
    });
    timer.start(16);

    qDebug() << "Windows cursor effect running.";
    return app.exec();
}
