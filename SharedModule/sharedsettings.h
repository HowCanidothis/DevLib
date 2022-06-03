#ifndef SHAREDSETTINGS_H
#define SHAREDSETTINGS_H

#include <PropertiesModule/internal.hpp>

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

#ifdef QT_GUI_LIB
struct StyleSettings
{
    StyleSettings();

    void InstallQSSReader(const QString& path, bool dynamic = true);

    LocalPropertyFilePath StylesQSSFile;

    LocalPropertyColor ShadowColor;

    LocalPropertyColor IconPrimaryColor;
    LocalPropertyColor IconSelectionColor;

    LocalPropertyColor DisabledTableCellColor;
    LocalPropertyColor EnabledTableCellColor;
    LocalPropertyColor DisabledTableCellTextColor;
    LocalPropertyColor EnabledTableCellTextColor;

    LocalPropertyColor DefaultRedColor;

    void Release();

private:
    ScopedPointer<class QtQSSReader> m_qssReader;
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

struct LanguageSettings
{
    LanguageSettings();

    LocalPropertyLocale ApplicationLocale;
    LocalProperty<std::function<QString (const QLocale&, const QDateTime& dt)>> DateTimeToStringHandler;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer.OpenSection("LanguageSettings");
        buffer << buffer.Sect("Locale", ApplicationLocale);
        buffer.CloseSection();
    }

    static QString DateTimeToString(const QDateTime& dt);
};

class SharedSettings : public Singletone<SharedSettings>
{
public:
    SharedSettings();

#ifdef QT_GUI_LIB
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
