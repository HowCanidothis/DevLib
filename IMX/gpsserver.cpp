#include "gpsserver.h"

#include <QTimer>

#include <SharedModule/internal.hpp>

GPSServer::GPSServer(QObject* parent)
    : Super(parent)
    , m_broadcastingTimer(new QTimer)
    , m_broadcastingInterval("Server/BroadcastingInterval", 1000, 100, 10000)
    , m_serverPort("Server/Port", 55555, 0, 0xffff)
    , m_serverHost("Server/Host", QHostAddress(QHostAddress::LocalHost).toString())
{
    connect(m_broadcastingTimer.data(), SIGNAL(timeout()), this, SLOT(broadcast()));
    m_broadcastingTimer->start(m_broadcastingInterval);

    m_broadcastingInterval.Subscribe([this]{
        m_broadcastingTimer->start(m_broadcastingInterval);
    });

    m_serverHost.Subscribe([this]{
        SetHost(m_serverHost);
        if(IsRunning()) {
            StartServer();
        }
    });

    m_serverPort.Subscribe([this]{
        SetHostPort(m_serverPort);
        if(IsRunning()) {
            StartServer();
        }
    });
}

GPSServer::~GPSServer()
{

}

void GPSServer::onPackageRecieved(qintptr descriptor, const NetworkPackage& package)
{
    auto baseQuery = package.Read<GPSQueryBase>();
    switch (baseQuery.PackageType) {
    case GPSQueryBase::PackageType_GPSSubsrcibe:
        m_subscribedConnections.insert(descriptor);
        break;
    case GPSQueryBase::PackageType_GPSState: {
            if(m_actualPackage.IsEmpty()) {
                GPSQueryMessage message;
                message.Message = "No valid data";
                NetworkPackage package;
                package.Pack(message);

                write(descriptor, package);
            } else {
                write(descriptor, m_actualPackage);
            }
        }
        break;
    case GPSQueryBase::PackageType_Message: {
        auto message = package.Read<GPSQueryMessage>();
        qCritical() << "Incoming message" << message.Message;
        break;
    }
    default:
        qCritical() << "Broken package recieved" << package.GetData();
        break;
    };
}

void GPSServer::onConnectionLost(qintptr descriptor)
{
    m_subscribedConnections.remove(descriptor);
}

void GPSServer::OnIncomedPackage(const GPSData& data)
{
    GPSQueryGPSData query;
    query.Data = data;    
    m_actualPackage.Pack(query);
}

void GPSServer::broadcast()
{
    if(m_subscribedConnections.isEmpty()) {
        return;
    }

    for(qintptr descriptor : m_subscribedConnections) {
        write(descriptor, m_actualPackage);
    }
}
