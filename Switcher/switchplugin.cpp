#include "switch.h"
#include "switchplugin.h"

#include <QtPlugin>

SwitchPlugin::SwitchPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void SwitchPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool SwitchPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *SwitchPlugin::createWidget(QWidget *parent)
{
    return new Switch(parent);
}

QString SwitchPlugin::name() const
{
    return QLatin1String("Switch");
}

QString SwitchPlugin::group() const
{
    return QLatin1String("Buttons");
}

QIcon SwitchPlugin::icon() const
{
    return QIcon();
}

QString SwitchPlugin::toolTip() const
{
    return QLatin1String("");
}

QString SwitchPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool SwitchPlugin::isContainer() const
{
    return false;
}

QString SwitchPlugin::domXml() const
{
    return QLatin1String("<widget class=\"Switch\" name=\"switch\">\n</widget>\n");
}

QString SwitchPlugin::includeFile() const
{
    return QLatin1String("switch.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(switchplugin, SwitchPlugin)
#endif // QT_VERSION < 0x050000
