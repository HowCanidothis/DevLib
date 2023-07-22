#include "sharedsettings.h"

#ifdef SHARED_LIB_ADD_UI
#include <QApplication>
#include <QDoubleSpinBox>
#endif

#ifdef WIDGETS_MODULE_LIB
#include <WidgetsModule/internal.hpp>
#endif

#include <QStandardPaths>

NetworkSettings::NetworkSettings()
{
    OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, ProxyHost,
                        ProxyPassword,
                        ProxyUserName,
                        ProxyPort);
}

#ifdef SHARED_LIB_ADD_UI

StyleSettings::StyleSettings()
    : DisabledTableCellColor("#d4d4d4")
    , EnabledTableCellColor(Qt::white)
    , DefaultRedColor("#ff3b30")
    , ShowFocusMinFrame(3, 0, 9)
    #ifdef QT_DEBUG
    , ShadersPath("C:/Work/wps-client/libs/Content/Shaders/")
    #else
    , ShadersPath(":/")
    #endif   
{
    ShadersPath.SetValidator([](const QString& path){ return path.isEmpty() ? ":/" : path; });
#ifdef WIDGETS_MODULE_LIB
    WidgetsDialogsManager::GetInstance().ShadowColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, ShadowColor);
#endif
    StylesQSSFile.OnChanged += { this, [this]{
        InstallQSSReader(StylesQSSFile);
    }};
}

StyleSettings::~StyleSettings()
{

}

void StyleSettings::InstallQSSReader(const QString& path, bool dynamic)
{
#ifdef WIDGETS_MODULE_LIB
    if(m_qssReader == nullptr) {
        m_qssReader = new QtQSSReader();
    }
    m_qssReader->SetEnableObserver(dynamic);
    m_qssReader->Install(path);
#endif
}

void StyleSettings::Release()
{
#ifdef WIDGETS_MODULE_LIB
    m_qssReader = nullptr;
#endif
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
    : ApplicationLocale(QLocale::system())
    , DateTimeToStringHandler([this](const QLocale& locale, const QDateTime& dt){
        return locale.toString(dt, DateTimeFormat);
    })
{
    ApplicationLocale.ConnectBoth(CONNECTION_DEBUG_LOCATION, LocaleIndex, [](const QLocale& locale){
        if(locale.language() == QLocale::English) {
            return LocaleType::English;
        }
        return LocaleType::Russian;
    }, [](qint32 localeType){
        QLocale result;
        if(localeType == (qint32)LocaleType::English) {
            result = QLocale(QLocale::English);
        } else {
            result = QLocale(QLocale::Russian);
        }
        return result;
    });

    ApplicationLocale.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QLocale& locale){
        ApplicationLocale.EditSilent().setNumberOptions(QLocale::OmitGroupSeparator);
        QLocale::setDefault(locale);

        if(locale.language() == QLocale::English){
            TimeFormat = "hh:mm AP";
            DateFormat = "MM/dd/yyyy";
            DateTimeFormat = DefaultDateTimeFormat;
            return;
        }

        TimeFormat = locale.timeFormat(QLocale::FormatType::ShortFormat);
        DateFormat = locale.dateFormat(QLocale::FormatType::ShortFormat);
        DateTimeFormat = locale.dateTimeFormat(QLocale::FormatType::ShortFormat);
    });
}

void LanguageSettings::Initialize()
{
#ifdef SHARED_LIB_ADD_UI
    ApplicationLocale.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [&]{
        for(auto* w : qApp->topLevelWidgets()) {
            auto spinboxes = w->findChildren<QDoubleSpinBox*>();
            for(auto* s : spinboxes) {
                s->setLocale(ApplicationLocale);
            }
        }
    });
#endif
}

QTime LanguageSettings::TimeFromVariant(const QVariant& data)
{
    return data.toTime();
}

QDate LanguageSettings::DateFromVariant(const QVariant& data)
{
    return data.toDate();
}

QDateTime LanguageSettings::DateTimeFromVariant(const QVariant& data)
{
    return data.toDateTime();
}

QString LanguageSettings::DateTimeToString(const QDateTime& dt)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return settings.DateTimeToStringHandler.Native()(settings.ApplicationLocale, dt);
}

QString LanguageSettings::DoubleToString(double v, qint32 precision)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return settings.ApplicationLocale.Native().toString(v, 'f', precision);
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
#ifdef WIDGETS_MODULE_LIB
    WidgetsDialogsManager::GetInstance().ShadowBlurRadius.ConnectFrom(CONNECTION_DEBUG_LOCATION, ShadowBlurRadius);
#endif
}

void SharedSettings::Release()
{
#ifdef SHARED_LIB_ADD_UI
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
