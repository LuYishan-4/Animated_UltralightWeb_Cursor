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
};



KAuth::ActionReply Helper::install(
    const QVariantMap &args
)
{
    QString path =
        args.value(
            QStringLiteral("path")
        ).toString();


    QString name =
        args.value(
            QStringLiteral("name")
        ).toString();

    
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



KAUTH_HELPER_MAIN(
    "org.ultralightwebcursor",
    Helper
)
#include "khelper.moc"