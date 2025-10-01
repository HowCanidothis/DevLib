#include "sharedsettings.h"

#ifdef SHARED_LIB_ADD_UI
#include <QApplication>
#include <QDoubleSpinBox>
#endif

#ifdef WIDGETS_MODULE_LIB
#include <WidgetsModule/internal.hpp>
#endif

#ifdef VISUALIZATION_MODULE_LIB
#include <VisualizationModule/internal.hpp>
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
    , DefaultRedColor("#e01936")
    , DefaultBlueColor("#3EA6FF")
    , DefaultGreenColor("#42d552")
    , DefaultOrangeColor("#ff9500")
    , DefaultGrayColor("#848894")
    , DefaultPurpleColor("#7B1FA2")
    , ShowFocusMinFrame(3, 0, 9)
    #ifdef QT_DEBUG
    , ShadersPath("C:/Work/wps-client/libs/Content/Shaders/")
    #else
    , ShadersPath(":/")
    #endif
#ifdef VISUALIZATION_MODULE_LIB
    , VisualSettings(::make_shared<VisualizationSettings>())
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
    , m_initialized(false)
{
}

void PathSettings::Terminate()
{
    if(m_initialized) {
        auto dir = TempDir;
#ifdef BUILD_MASTER
        dir.removeRecursively();
    }
#else
        if(dir.cd("sc")) {
            if(dir.cd(QString::number(qApp->applicationPid()))) {
                Q_ASSERT(dir.removeRecursively());
            }
        }
    }
#endif
}

void PathSettings::Initialize(const QString& productName)
{
    if(m_initialized) {
        return;
    }

    auto productString = productName.isEmpty() ? qApp->applicationName() : productName;

    Q_ASSERT(!productString.isEmpty());

    auto productStringWithPid = productString;

    TempDir.mkdir(productStringWithPid);

    if(!TempDir.cd(productStringWithPid)) {
        qCWarning(LC_CONSOLE) << "Unable to create temp directory";
    } else {
        m_initialized = true;
        LoggingDir = TempDir;
        LoggingDir.mkdir("Logging");
        LoggingDir.cd("Logging");
    }

    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + productString;
    UserDocumentsDir = QDir(path);
    if(!UserDocumentsDir.exists()){
        if(!UserDocumentsDir.mkpath(path)){
            qWarning(LC_CONSOLE) << tr("Could not create directory %1").arg(path);
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

        MonthFormat = "MMMM yyyy";
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

QString LanguageSettings::TimeToString(const QTime& timeValue)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return TimeToString(settings.ApplicationLocale, timeValue);
}

QDate LanguageSettings::DateFromVariant(const QVariant& data, const QString& dateFormat)
{
    if(data.type() == QVariant::String) {
        const auto& settings = SharedSettings::GetInstance().LanguageSettings;
        return settings.ApplicationLocale.Native().toDate(data.toString(), dateFormat.isEmpty() ? settings.DateFormat : dateFormat);
    }
    return data.toDate();
}

QString tryFixUpDateTime(const QString& input, const QString& format)
{
    if(format.size() == input.size()) {
        return input;
    }
    QString result;

    QString currentKey;
    QString currentDigit;
    auto inputIt = input.cbegin();
    qint32 currentDigitLettersCount = 0;

    auto update = [&]{
        if(currentKey == "AP") {
            while(inputIt != input.cend() && inputIt->isLetter()) {
                currentDigit.append(*inputIt);
                ++inputIt;
            }
            while(inputIt != input.cend() && !inputIt->isDigit()) {
                ++inputIt;
            }
            if(currentDigit.isEmpty()) {
                currentDigit = "AM";
            }
        } else {
            while(inputIt != input.cend() && inputIt->isDigit()) {
                currentDigit.append(*inputIt);
                ++inputIt;
            }
            while(inputIt != input.cend() && !inputIt->isDigit()) {
                ++inputIt;
            }
            auto adjustCount = currentDigitLettersCount - currentDigit.size();
            while(--adjustCount >= 0) {
                currentDigit.prepend('0');
            }
        }
        currentDigitLettersCount = 0;
        result.append(currentDigit);
        currentDigit = QString();
        currentKey = QString();
    };

    for(const auto& l : format) {
        if(l.isLetter()) {
            currentKey.append(l);
            ++currentDigitLettersCount;
        } else if(currentDigitLettersCount == 0) {
            result.append(l);
        } else {
            update();
            result.append(l);
        }
    }
    update();

    return result;
}

QString LanguageSettings::TimeToString(const QLocale& locale, const QTime& timeValue){
    static QString enFormat("%1:%2 %3");
    static QString ruFormat("%1:%2");
    QString result;
    if(!timeValue.isValid()) {
        return result;
    }

    auto formatString00 = [](qint32 count) {
        return (count < 10 ? "0" : "") + QString::number(count);
    };

    if(locale.language() == QLocale::English){
        auto cutHours = timeValue.hour() > 12 ? (timeValue.hour() - 12) : timeValue.hour();
        if(timeValue.hour() >= 12) {
            result = enFormat.arg(formatString00(cutHours), formatString00(timeValue.minute()), "PM");
        } else {
            result = enFormat.arg(formatString00(cutHours), formatString00(timeValue.minute()), "AM");
        }
    } else {
        result = ruFormat.arg(formatString00(timeValue.hour()), formatString00(timeValue.minute()));
    }

    return result;
}

QTime LanguageSettings::TimeFromVariant(const QVariant& v)
{
    if(v.type() == QVariant::String) {
        thread_local static QRegExp enFormat(R"((\d+):(\d+) (\w\w))");
        thread_local static QRegExp ruFormat(R"((\d+):(\d+))");

        auto string = v.toString();
        QLocale locale;
        if(locale.language() == QLocale::English) {
            if(enFormat.indexIn(string) != -1) {
                qint32 shift = enFormat.cap(3) == "PM" ? 12 : 0;
                return QTime(enFormat.cap(1).toInt() + shift, enFormat.cap(2).toInt());
            }
        }
        if(ruFormat.indexIn(string) != -1) {
            return QTime(ruFormat.cap(1).toInt(), ruFormat.cap(2).toInt());
        }
        return QTime();
    }
    return v.toTime();
}

QDateTime LanguageSettings::DateTimeFromVariant(const QVariant& v)
{
    if(v.type() == QVariant::String) {
        auto format = SharedSettings::GetInstance().LanguageSettings.DateTimeFormat.Native();
        QLocale locale;
        return locale.toDateTime(tryFixUpDateTime(v.toString(), format), format);
    }
    return v.toDateTime();
}

QString LanguageSettings::DateTimeToString(const QDateTime& dt)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return settings.DateTimeToStringHandler.Native()(settings.ApplicationLocale, dt);
}

QString LanguageSettings::DateToString(const QDate& dt, const QString& dateFormat)
{
    const auto& settings = SharedSettings::GetInstance().LanguageSettings;
    return settings.ApplicationLocale.Native().toString(dt, dateFormat.isEmpty() ? SharedSettings::GetInstance().LanguageSettings.DateFormat.Native() : dateFormat);
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
