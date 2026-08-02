#pragma once

#include <KQuickConfigModule>
#include <KPluginFactory>


class SettingsBackend;


class UIKCM : public KQuickConfigModule
{
    Q_OBJECT

       Q_PROPERTY(
        QObject* backend
        READ backend
        CONSTANT
    )

public:

    UIKCM(
        QObject* parent,
        const KPluginMetaData& metaData
    );

    ~UIKCM() override;
    QObject* backend() const;

    void load() override;

    void save() override;

    void defaults() override;


private:

    SettingsBackend* m_backend = nullptr;

};