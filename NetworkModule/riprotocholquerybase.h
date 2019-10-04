#ifndef RIPROTOCHOLQUERYBASE_H
#define RIPROTOCHOLQUERYBASE_H

#include <QDebug>

enum RIProtocholQueryType
{
    RIProtocholQuery_Error
};

struct RIProtocholQueryBase
{
    quint16 From;
    quint16 To;
    quint16 ResponseId;
    quint16 RequestId;
    quint32 PackageType;

    RIProtocholQueryBase(quint32 packageType = RIProtocholQuery_Error)
        : PackageType(packageType)
    {}

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << From;
        buffer << To;
        buffer << ResponseId;
        buffer << RequestId;
        buffer << (qint32&)PackageType;
    }

    friend QDebug operator<<(QDebug debug, const RIProtocholQueryBase& data)
    {
        debug << "GPSData Values" << "\n";
        debug << "(" << "\n";
        debug << "From" << data.From << "\n";
        debug << "To" << data.To << "\n";
        debug << "RequestId" << data.RequestId << "\n";
        debug << "ResponseId" << data.ResponseId << "\n";
        debug << ")\n";
        return debug;
    }
};

#endif // PACKAGES_H
