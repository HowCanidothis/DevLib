#ifndef REGISTRYMANAGER_H
#define REGISTRYMANAGER_H

class RegistryManager
{
    RegistryManager();

public:
    bool Initialize(const QStringList& arguments, bool withAdminRights = false);

    SharedPointer<class QSettings> UninstallSettings(const QString& appName) const;
    SharedPointer<class QSettings> ProductSettings(const QString& appName) const;

    static RegistryManager& GetInstance();

private:
    QString m_uninstallPath;
    QString m_productsPath;
};

#endif // REGISTRYMANAGER_H
