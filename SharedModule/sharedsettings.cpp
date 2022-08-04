#include "sharedsettings.h"

#include <QStandardPaths>

NetworkSettings::NetworkSettings()
{
    OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, ProxyHost,
                        ProxyPassword,
                        ProxyUserName,
                        ProxyPort);
}

#ifdef QT_GUI_LIB

#include "WidgetsModule/Managers/widgetsdialogsmanager.h"

StyleSettings::StyleSettings()
    : DisabledTableCellColor("#d4d4d4")
    , EnabledTableCellColor(Qt::white)
    , DefaultRedColor("#ff3b30")
    , ShowFocusMinFrame(3, 0, 9)
{
    WidgetsDialogsManager::GetInstance().ShadowColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, ShadowColor);

    StylesQSSFile.OnChanged += { this, [this]{
        InstallQSSReader(StylesQSSFile);
    }};
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
#endif

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

static const QString DefaultDateTimeFormat = "MM/dd/yyyy hh:mm AP";

LanguageSettings::LanguageSettings()
    : DateTimeToStringHandler([](const QLocale& locale, const QDateTime& dt){
        if(locale.language() == QLocale::English){
            return locale.toString(dt, DefaultDateTimeFormat);
        }
        return locale.toString(dt, QLocale::FormatType::ShortFormat);
    })
{
}

QString LanguageSettings::DateTimeToString(const QDateTime& dt)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return settings.DateTimeToStringHandler.Native()(settings.ApplicationLocale, dt);
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
    OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, NetworkSettings.OnChanged);
}

MetricsSettings::MetricsSettings()
{
#ifdef QT_GUI_LIB
    WidgetsDialogsManager::GetInstance().ShadowBlurRadius.ConnectFrom(CONNECTION_DEBUG_LOCATION, ShadowBlurRadius);
#endif
}

void SharedSettings::Release()
{
#ifdef QT_GUI_LIB
    StyleSettings.Release();
#endif
}

void SharedSettings::CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties)
{
    if(prefix.isEmpty()) {
        prefix = "Settings/";
    }

    NetworkSettings.CreateGlobalProperties(prefix, properties);
}
