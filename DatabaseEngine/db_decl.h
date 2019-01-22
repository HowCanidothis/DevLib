#ifndef DB_DECL_H
#define DB_DECL_H

#include <QDateTime>

#include <SharedModule/internal.hpp>

typedef quint32 array_ptr_t;
typedef quint16 id_t;

typedef array_ptr_t old_address_t;
typedef array_ptr_t new_address_t;
typedef QHash<old_address_t, new_address_t> DbAddressMap;

// #define DB_NAMESPACE db

#ifdef DB_NAMESPACE
    #define DB_BEGIN_NAMESPACE namespace DB_NAMESPACE{
    #define DB_END_NAMESPACE }
    #define DB_USING_NAMESPACE using namespace DB_NAMESPACE;
    #define DB_USE_NAMESPACE(Type) DB_NAMESPACE::Type
#else
    #define DB_BEGIN_NAMESPACE
    #define DB_END_NAMESPACE
    #define DB_USING_NAMESPACE
    #define DB_USE_NAMESPACE(Type) Type
#endif

DB_BEGIN_NAMESPACE

struct MDbObject
{
    Q_DISABLE_COPY(MDbObject)
    typedef MDbObject Super;
    MDbObject(){}

    id_t GetID() const;

protected:
    void Free();
};

struct DbDatabaseProperties
{
    quint32 SectorSize = 1024 * 1024 *5;
};

struct DbTableFieldType{
    Q_GADGET
public:
    enum Type{
        NotStorable=-1,
        Boolean,
        Byte,
        UnsignedByte,
        Short,
        UnsignedShort,
        Long,
        UnsignedLong,
        LongLong,
        UnsignedLongLong,
        Float,
        Double,
        Text,
        Blob,
        Points2D,
        Points2F,
        Points3D,
        Points3F,
        DateTime,
        Point2D,
        Point2F,
        Point3D,
        Point3F,
        Longs,

        DefaultCount,
        User = 10000
    };
    Q_ENUM(Type)

    DbTableFieldType(){}
    DbTableFieldType(qint32 v) : m_integerType(v){}
    operator qint32&(){ return m_integerType; }
    operator qint32() const { return m_integerType; }
private:
    union{
        Type m_fieldType;
        qint32 m_integerType;
    };
};

DB_END_NAMESPACE

#endif // DB_DECL_H
