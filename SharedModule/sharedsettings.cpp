#include "sharedsettings.h"

#include <QStandardPaths>

#include "WidgetsModule/Managers/widgetsdialogsmanager.h"

StyleSettings::StyleSettings()
    : DisabledTableCellColor(QColor("#d4d4d4"))
{
    WidgetsDialogsManager::GetInstance().ShadowColor.ConnectFrom(ShadowColor);

    StylesQSSFile.OnChange += { this, [this]{
        InstallQSSReader(StylesQSSFile);
    }};
}

void NetworkSettings::CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties)
{
    prefix += "NetworkSettings/";
    properties += PropertyFromLocalProperty::Create(prefix + "ProxyUserName", ProxyUserName);
    properties += PropertyFromLocalProperty::Create(prefix + "ProxyPassword", ProxyPassword);
    properties += PropertyFromLocalProperty::Create(prefix + "ProxyPort", ProxyPort);
    properties += PropertyFromLocalProperty::Create(prefix + "ProxyHost", ProxyHost);
}

PathSettings::PathSettings()
{
    auto productString = qApp->applicationName();

    Q_ASSERT(!productString.isEmpty());

    if(!TempDir.mkpath(QString("%1/Logging").arg(productString))) {
        qCWarning(LC_UI) << "Unable to create temp directory";
    } else {
        TempDir.cd(productString);
    }

    auto pathList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    pathList << productString;
    UserDocumentsPath = pathList.join("/");
    QDir dir(UserDocumentsPath);
    if(!dir.exists()){
        if(!dir.mkpath(UserDocumentsPath)){
            qWarning(LC_UI()) << tr("Could not create directory %1").arg(UserDocumentsPath.Native());
        }
    }
}

QString PathSettings::GenerateLocalDataPath(const QString& fileName)
{
    return QString("%1/%2").arg(UserDocumentsPath,fileName);
}

LanguageSettings::LanguageSettings()
{
}

void StyleSettings::InstallQSSReader(const QString& path, bool dynamic)
{
    if(m_qssReader == nullptr) {
        m_qssReader = new QtQSSReader();
    }
    m_qssReader->SetEnableObserver(dynamic);
    m_qssReader->Install(path);
}

void StyleSettings::Release()
{
    m_qssReader = nullptr;
}

SaveLoadSettings::SaveLoadSettings()
    : AutoSaveInMinutes(2, 1, 120)
    , AutoSaveEnabled(true)
    , ApplicationRestoreStateAfterRerun(true)
{

}

MetricsSettings::MetricsSettings()
{
    WidgetsDialogsManager::GetInstance().ShadowBlurRadius.ConnectFrom(ShadowBlurRadius);
}

void SharedSettings::Release()
{
    StyleSettings.Release();
}

void SharedSettings::CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties)
{
    if(prefix.isEmpty()) {
        prefix = "Settings/";
    }

    NetworkSettings.CreateGlobalProperties(prefix, properties);
}
