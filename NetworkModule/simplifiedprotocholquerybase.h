#ifndef SIMPLIFIEDPROTOCHOLQUERYBASE_H
#define SIMPLIFIEDPROTOCHOLQUERYBASE_H

#include <QDebug>

enum SimplifiedProtocholQueryType
{
    SimplifiedProtocholQuery_Unknown = -1,
    SimplifiedProtocholQuery_Error,

    SimplifiedProtocholQuery_Last = 100,
};

struct SimplifiedProtocholQueryBase
{
    quint32 PackageType;

    SimplifiedProtocholQueryBase(quint32 packageType = SimplifiedProtocholQuery_Unknown)
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

struct SimplifiedProtocholErrorQuery : SimplifiedProtocholQueryBase
{
    using Super = SimplifiedProtocholQueryBase;

    Name ErrorId;

    SimplifiedProtocholErrorQuery()
        : Super(SimplifiedProtocholQuery_Error)
    {}

    static NetworkPackage CreatePackage(const Name& errorId)
    {
        SimplifiedProtocholErrorQuery errorQuery;
        errorQuery.ErrorId = errorId;
        NetworkPackage result;
        result.Pack(errorQuery);
        return result;
    }

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        Super::Serialize(buffer);
        buffer << ErrorId;
    }
};

#endif // SIMPLIFIEDPROTOCHOLQUERYBASE_H
