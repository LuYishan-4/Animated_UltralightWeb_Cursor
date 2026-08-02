#include "main_settings.hpp"

#include "SettingsBackend.hpp"

#include <QQmlContext>
#include <QQmlEngine>

#include <KLocalizedString>



K_PLUGIN_CLASS_WITH_JSON(UIKCM, "kcm_ultralightwebcursor.json")

UIKCM::UIKCM(
    QObject* parent,
    const KPluginMetaData& metaData
)
    : KQuickConfigModule(parent, metaData)
{
 

    m_backend = new SettingsBackend(this);

 

   setButtons(Help | Apply | Default);

    load();
}


UIKCM::~UIKCM() = default;
QObject* UIKCM::backend() const
{
    return m_backend;
}

void UIKCM::load()
{
    if(!m_backend)
        return;

    m_backend->reload();

    KQuickConfigModule::load();
}


void UIKCM::save()
{
    if(!m_backend)
        return;

    m_backend->save();

    KQuickConfigModule::save();
}


void UIKCM::defaults()
{
    if(!m_backend)
        return;

    m_backend->setEnabled(true);

    m_backend->setCursorWidth(128);
    m_backend->setCursorHeight(128);

    KQuickConfigModule::defaults();
}


#include "main_settings.moc"