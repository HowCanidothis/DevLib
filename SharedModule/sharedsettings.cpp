#include "sharedsettings.h"

#include <QStandardPaths>

#include "WidgetsModule/Managers/widgetsdialogsmanager.h"

NetworkSettings::NetworkSettings()
{
    QVector<CommonDispatcher<>*> dispatchers {
        &ProxyHost.OnChange,
        &ProxyPassword.OnChange,
        &ProxyUserName.OnChange,
        &ProxyPort.OnChange
    };
    OnChanged.Subscribe(dispatchers);
}

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
    : TextComparatorApplicationPath("C:/Program Files/TortoiseGit/bin/TortoiseGitMerge.exe")
    , TempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
{
}

void PathSettings::Initialize(const QString& productName)
{
    auto productString = productName.isEmpty() ? qApp->applicationName() : productName;

    Q_ASSERT(!productString.isEmpty());

    TempDir.mkdir(productString);

    if(!TempDir.cd(productString)) {
        qCWarning(LC_UI) << "Unable to create temp directory";
    } else {
        LoggingDir = TempDir;
        LoggingDir.mkdir("Logging");
        LoggingDir.cd("Logging");
    }

    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + productString;
    UserDocumentsDir = QDir(path);
    if(!UserDocumentsDir.exists()){
        if(!UserDocumentsDir.mkpath(path)){
            qWarning(LC_UI()) << tr("Could not create directory %1").arg(path);
        }
    }
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

SharedSettings::SharedSettings()
    : OnChanged(500)
{
    QVector<CommonDispatcher<>*> dispatchers {
        &NetworkSettings.OnChanged
    };

    OnChanged.Subscribe(dispatchers);
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
