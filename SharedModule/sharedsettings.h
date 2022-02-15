#ifndef SHAREDSETTINGS_H
#define SHAREDSETTINGS_H

#include <PropertiesModule/internal.hpp>

struct NetworkSettings
{
    NetworkSettings(){}

    LocalPropertyString ProxyHost;
    LocalPropertyString ProxyPassword;
    LocalPropertyString ProxyUserName;
    LocalPropertyInt ProxyPort;

    void CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties);
};

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

    void Release();

private:
    ScopedPointer<class QtQSSReader> m_qssReader;
};

struct PathSettings
{
    PathSettings();

    LocalPropertyString UserDocumentsPath;
    QDir TempDir;

    QString GenerateLocalDataPath(const QString& fileName);

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
};

class SharedSettings : public Singletone<SharedSettings>
{
public:
    StyleSettings StyleSettings;
    PathSettings PathSettings;
    MetricsSettings MetricSettings;
    SaveLoadSettings SaveLoadSettings;
    LanguageSettings LanguageSettings;
    NetworkSettings NetworkSettings;

    void CreateGlobalProperties(QString prefix, PropertyFromLocalPropertyContainer& properties);

    void Release();
};

#endif // SHAREDSETTINGS_H
