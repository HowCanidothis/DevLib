#ifndef DBMEMORYDECLARATIONS_H
#define DBMEMORYDECLARATIONS_H
#include "DatabaseEngine/dbarray.h"

DB_BEGIN_NAMESPACE

struct MDbTableHeaderDeclaration : MDbObject
{
    DbString Name;
    qint32 Usage;
    qint32 FieldsCount;

    MDbTableHeaderDeclaration(const QString& name) :
        Name(name),
        Usage(-1),
        FieldsCount(0)
    {}
    void Free() {
        Super::Free();
        Name.Free();
    }

    void updatePlainData(const DbAddressMap& addressMap)
    {
        Name.updatePlainData(addressMap);
    }
};

struct MDbTableDeclaration : MDbObject
{
    DbString Name;
    id_t HeaderId;
    quint32 LastId;

    void Free() {
        Super::Free();
        Name.Free();
    }
    MDbTableDeclaration(const QString& name, id_t header_id) :
        Name(name),
        HeaderId(header_id),
        LastId(0)
    {}

    void updatePlainData(const DbAddressMap& addressMap)
    {
        Name.updatePlainData(addressMap);
    }
};

struct MDbTableProxyDeclaration : MDbObject
{
    id_t ColumnIndex;
    DbIntArray RowsIndexesInSource;

    void Free() {
        Super::Free();
        RowsIndexesInSource.Free();
    }

    void updatePlainData(const DbAddressMap& addressMap)
    {
        RowsIndexesInSource.updatePlainData(addressMap);
    }
};

DB_END_NAMESPACE

#endif // DBMEMORYDATAHEADERS_H
