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
};

struct LanguageSettings
{
    LanguageSettings();

    LocalPropertyLocale ApplicationLocale;
    LocalProperty<std::function<QString (const QLocale&, const QDateTime& dt)>> DateTimeToStringHandler;

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
        buffer.CloseSection();
    }

    void CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties);

    void Release();
};

#endif // SHAREDSETTINGS_H
