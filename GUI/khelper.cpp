#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

#include <QObject>
#include <QVariantMap>
#include <QString>
#include "../src/config/UserConfig.hpp"

class Helper : public QObject
{
    Q_OBJECT

public:

    using QObject::QObject;


public Q_SLOTS:
    KAuth::ActionReply install(
        const QVariantMap &args
    );

private:
    KAuth::ActionReply doInstall(
        const QString &path, 
        const QString &name
    );

    KAuth::ActionReply doUninstall(
        const QString &name
    );
};



KAuth::ActionReply Helper::install(
    const QVariantMap &args
)
{

    QString targetAction = 
        args.value(
            QStringLiteral("action")
        ).toString();

    QString name =
        args.value(
            QStringLiteral("name")
        ).toString();


    if (targetAction == QStringLiteral("uninstall"))
    {
        return doUninstall(name);
    }


    QString path =
        args.value(
            QStringLiteral("path")
        ).toString();

    return doInstall(path, name);
}



KAuth::ActionReply Helper::doInstall(
    const QString &path, 
    const QString &name
)
{
    UltralightWebCursorM::UserConfig::instance()->load();

    bool ok =
        UltralightWebCursorM::UserConfig::instance()
        ->uploadTheme(
            path.toStdString(),
            name.toStdString()
        );


    if(!ok)
    {
        return KAuth::ActionReply::HelperErrorReply(1);
    }


    QVariantMap data;

    data.insert(
        QStringLiteral("success"),
        true
    );


    auto reply =
        KAuth::ActionReply::SuccessReply();

    reply.setData(data);

    return reply;
}



KAuth::ActionReply Helper::doUninstall(
    const QString &name
)
{
    UltralightWebCursorM::UserConfig::instance()->load();

    bool ok =
        UltralightWebCursorM::UserConfig::instance()
        ->removeTheme(
            name.toStdString()
        );


    if(!ok)
    {
        return KAuth::ActionReply::HelperErrorReply(2);
    }


    QVariantMap data;

    data.insert(
        QStringLiteral("success"),
        true
    );


    auto reply =
        KAuth::ActionReply::SuccessReply();

    reply.setData(data);

    return reply;
}



KAUTH_HELPER_MAIN(
    "org.ultralightwebcursor",
    Helper
)
#include "khelper.moc"
