#include "registrymanager.h"

#include <QSettings>

#ifdef WIN64
#include <windows.h>
#include <shellapi.h>
#endif

RegistryManager::RegistryManager()
{

}

bool RegistryManager::Initialize(const QStringList& arguments, bool withAdminRights)
{
    if(!withAdminRights) {
        m_uninstallPath = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        return true;
    }
    m_uninstallPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    static const QString testRightsName("HasRights");
    auto settings = UninstallSettings("");
    settings->setValue(testRightsName, true);
    if(!settings->value(testRightsName).toBool()) {
#ifdef WIN64
        ::ShellExecuteA(0, "runas", QCoreApplication::applicationFilePath().replace('/', '\\').toUtf8().constData(), arguments.join(" ").toUtf8().constData(), 0, SW_SHOWNORMAL);
#endif
        return false;
    }
    settings->remove(testRightsName);
    return true;
}

SharedPointer<QSettings> RegistryManager::UninstallSettings(const QString& appName) const
{
    if(appName.isEmpty()) {
        return ::make_shared<QSettings>(m_uninstallPath, QSettings::NativeFormat);
    }
    return ::make_shared<QSettings>(QString("%1\\%2").arg(m_uninstallPath, appName), QSettings::NativeFormat);
}

RegistryManager& RegistryManager::GetInstance()
{
    static RegistryManager registry;
    return registry;
}
