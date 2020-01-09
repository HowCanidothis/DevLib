#ifndef SIMPLIFIEDPROTOCHOLQUERYBASE_H
#define SIMPLIFIEDPROTOCHOLQUERYBASE_H

#include <QDebug>

enum SimplifiedProtocholQueryType
{
    SimplifiedProtocholQuery_Error
};

struct SimplifiedProtocholQueryBase
{
    quint32 PackageType;

    SimplifiedProtocholQueryBase(quint32 packageType = SimplifiedProtocholQuery_Error)
        : PackageType(packageType)
    {}

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << (qint32&)PackageType;
    }

    friend QDebug operator<<(QDebug debug, const SimplifiedProtocholQueryBase& data)
    {
        debug << "SimplifiedProtochol Values" << "\n";
        debug << "(" << "\n";
        debug << "PackageType" << data.PackageType << "\n";
        debug << ")\n";
        return debug;
    }
};

#endif // SIMPLIFIEDPROTOCHOLQUERYBASE_H
