#ifndef SHAREDSETTINGS_H
#define SHAREDSETTINGS_H

#include "localproperty.h"

struct NetworkSettings
{
    NetworkSettings();

    LocalPropertyString ProxyHost;
    LocalPropertyString ProxyPassword;
    LocalPropertyString ProxyUserName;
    LocalPropertyInt ProxyPort;

    DispatchersCommutator OnChanged;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("NetworkSettings");
        buffer << buffer.Sect("ProxyHost", ProxyHost);
        buffer << buffer.Sect("ProxyPassword", ProxyPassword);
        buffer << buffer.Sect("ProxyUserName", ProxyUserName);
        buffer << buffer.Sect("ProxyPort", ProxyPort);
        buffer.CloseSection();
    }

    void CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties);
};

#ifdef SHARED_LIB_ADD_UI
struct StyleSettings
{
    StyleSettings();
    ~StyleSettings();

    void InstallQSSReader(const QString& path, bool dynamic = true);

    LocalPropertyFilePath StylesQSSFile;
    LocalPropertyColor ShadowColor;

    LocalPropertyColor IconPrimaryColor;
    LocalPropertyColor IconSecondaryColor;
    LocalPropertyColor IconSelectionColor;
    LocalPropertyColor IconSecondarySelectionColor;
    LocalPropertyColor IconDisabledColor;
    LocalPropertyColor IconSecondaryDisabledColor;

    LocalPropertyColor DisabledTableCellColor;
    LocalPropertyColor EnabledTableCellColor;
    LocalPropertyColor DisabledTableCellTextColor;
    LocalPropertyColor EnabledTableCellTextColor;

    LocalPropertyColor ErrorLinkColor;
    LocalPropertyColor WarningLinkColor;
    LocalPropertyColor ErrorColor;
    LocalPropertyColor WarningColor;
    LocalPropertyColor PlaceHolderColor;

    LocalPropertyColor DefaultRedColor;
    LocalPropertyColor DefaultBlueColor;
    LocalPropertyColor DefaultGreenColor;
    LocalPropertyColor DefaultOrangeColor;
    LocalPropertyColor DefaultGrayColor;
    LocalPropertyColor DefaultPurpleColor;

    LocalPropertyInt ShowFocusMinFrame;
    LocalPropertyFilePath ShadersPath;

    void Release();

private:
#ifdef WIDGETS_MODULE_LIB
    ScopedPointer<class QtQSSReader> m_qssReader;
#endif
};
#endif

struct PathSettings
{
    PathSettings();

    LocalPropertyFilePath TextComparatorApplicationPath;
    QDir UserDocumentsDir;
    QDir TempDir;
    QDir LoggingDir;


    void Initialize(const QString& productName);

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("PathSettings");
        buffer << buffer.Sect("TextComparatorApplicationPath", TextComparatorApplicationPath);
        buffer.CloseSection();
    }

    Q_DECLARE_TR_FUNCTIONS(PathSettings)
};

struct MetricsSettings
{
    MetricsSettings();

    LocalPropertyDouble ShadowBlurRadius;
};

struct SaveLoadSettings
{
    SaveLoadSettings();

    LocalPropertyInt AutoSaveInMinutes;
    LocalPropertyBool AutoSaveEnabled;
    LocalPropertyBool ApplicationRestoreStateAfterRerun;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("LanguageSettings");
        buffer << buffer.Sect("AutoSaveEveryMinutes", AutoSaveInMinutes);
        buffer << buffer.Sect("AutoSaveEnabled", AutoSaveEnabled);
        buffer << buffer.Sect("ApplicationRestoreStateAfterRerun", ApplicationRestoreStateAfterRerun);
        buffer.CloseSection();
    }
};

enum class LocaleType {
    English,
    Russian,
    First = English,
    Last = Russian,
};

template<>
struct EnumHelper<LocaleType>
{
    static QStringList GetNames() { return { tr("EN"), tr("RU")}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

struct LanguageSettings
{
    LanguageSettings();

    LocalPropertySequentialEnum<LocaleType> LocaleIndex;
    LocalPropertyLocale ApplicationLocale;

    LocalPropertyString TimeFormat;
    LocalPropertyString DateFormat;
    LocalPropertyString DateTimeFormat;

    LocalProperty<std::function<QString (const QLocale&, const QDateTime& dt)>> DateTimeToStringHandler;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("LanguageSettings");
        buffer << buffer.Sect("Locale", ApplicationLocale);
        buffer.CloseSection();
    }

    void Initialize();

    static QTime TimeFromVariant(const QVariant& data);
    static QDate DateFromVariant(const QVariant& data);
    static QDateTime DateTimeFromVariant(const QVariant& data);
    static QString DateTimeToString(const QDateTime& dt);
    static QString DoubleToString(double v, qint32 precision);
};

class SharedSettings : public Singletone<SharedSettings>
{
public:
    SharedSettings();

#ifdef SHARED_LIB_ADD_UI
    StyleSettings StyleSettings;
#endif
    PathSettings PathSettings;
    MetricsSettings MetricSettings;
    SaveLoadSettings SaveLoadSettings;
    LanguageSettings LanguageSettings;
    NetworkSettings NetworkSettings;

    DispatchersCommutator OnChanged;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("Settings");
        buffer << NetworkSettings;
        buffer << PathSettings;
        buffer << SaveLoadSettings;
        buffer << LanguageSettings;
        buffer.CloseSection();
    }

    void CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties);

    void Release();
};

#endif // SHAREDSETTINGS_H
