#ifndef GPSSERVER_H
#define GPSSERVER_H

#include <NetworkModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

#include "decl.h"

class GPSServer : public NetworkServerBase
{
    Q_OBJECT
    typedef NetworkServerBase Super;
public:
    GPSServer(QObject* parent = nullptr);
    ~GPSServer() Q_DECL_OVERRIDE;

    // NetworkServerBase interface
protected:
    void onPackageRecieved(qintptr descriptor, const NetworkPackage& package) Q_DECL_OVERRIDE;
    void onConnectionLost(qintptr descriptor) Q_DECL_OVERRIDE;

public slots:
    void OnIncomedPackage(const GPSData& data);

private slots:
    void broadcast();

private:
    void checkPackage(const QByteArray& array);

private:
    QSet<quintptr> m_subscribedConnections;
    ScopedPointer<class QTimer> m_broadcastingTimer;
    IntProperty m_broadcastingInterval;
    IntProperty m_serverPort;
    StringProperty m_serverHost;
    NetworkPackage m_actualPackage;
};


#endif // GPSSERVER_H
