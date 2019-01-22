#ifndef DBTABLEFIELD_H
#define DBTABLEFIELD_H

#include "dbarray.h"

DB_BEGIN_NAMESPACE

inline bool operator <(const QVector<Point3D>& p, const QVector<Point3D>& s){
    return p.size() < s.size();
}
inline bool operator <(const QVector<Point2D>& p, const QVector<Point2D>& s){
    return p.size() < s.size();
}

inline bool operator <(const QVector<Point3F>& p, const QVector<Point3F>& s){
    return p.size() < s.size();
}
inline bool operator <(const QVector<Point2F>& p, const QVector<Point2F>& s){
    return p.size() < s.size();
}

namespace DbPrivate
{
    template<class T>
    static void setFromVariant(const QVariant& variant, CastablePtr& value)
    {
        value.As<T>() = variant.toDouble();
    }

    template<class T>
    static void setFromVariantPOD(const QVariant& variant, CastablePtr& value)
    {
        value.As<T>().FromString(variant.toString());
    }

    template<class ContainerType>
    static void setFromVariantArray(const QVariant& variant, CastablePtr& value)
    {
        value.As<ContainerType>().FromString(variant.toString());
    }

    ////////////////////////////////////////////////////////////////////////////////

    template<class DbType>
    static QString toString(const DbType& value)
    {
        return value.ToString();
    }
}

#define DB_FOREACH_PRIMITIVE_FIELDS(DECL) \
    DECL(qint8,DbTableFieldType::Byte) \
    DECL(quint8,DbTableFieldType::UnsignedByte) \
    DECL(qint16,DbTableFieldType::Short) \
    DECL(quint16,DbTableFieldType::UnsignedShort) \
    DECL(qint32,DbTableFieldType::Long) \
    DECL(quint32,DbTableFieldType::UnsignedLong) \
    DECL(qint64,DbTableFieldType::LongLong) \
    DECL(quint64,DbTableFieldType::UnsignedLongLong) \
    DECL(double,DbTableFieldType::Double) \
    DECL(float,DbTableFieldType::Float) \
    DECL(bool,DbTableFieldType::Boolean)
#define DB_FOREACH_POD_FIELDS(DECL) \
    DECL(DateTime,DbTableFieldType::DateTime) \
    DECL(Point2D,DbTableFieldType::Point2D) \
    DECL(Point2F,DbTableFieldType::Point2F) \
    DECL(Point3D,DbTableFieldType::Point3D) \
    DECL(Point3F,DbTableFieldType::Point3F)
#define DB_FOREACH_COMPLEX_FIELDS(DECL) \
    DECL(QString,DbTableFieldType::Text) \
    DECL(QByteArray,DbTableFieldType::Blob) \
    DECL(Array<qint32>, DbTableFieldType::Longs) \
    DECL(Array<Point2D>,DbTableFieldType::Points2D) \
    DECL(Array<Point2F>,DbTableFieldType::Points2F) \
    DECL(Array<Point3D>,DbTableFieldType::Points3D) \
    DECL(Array<Point3F>,DbTableFieldType::Points3F)

template<typename T> struct DbTableFieldTypeHelper{ };

#define DECL_PRIMITIVE_FIELD_TYPE_HELPER(CppType, First) \
template<> struct DbTableFieldTypeHelper<CppType>{ \
    typedef CppType DbCppType; \
    enum{ \
        Type = First, \
        Size = sizeof(CppType), \
        isComplex = false \
    };\
    static const char* GetCppTypeString(){ return QT_STRINGIFY(CppType); } \
    static void SetFromVariant(const QVariant& variant, CastablePtr& value) { DbPrivate::setFromVariant<CppType>(variant, value); } \
    static const char* GetDbTypeString(){ return #First; } \
    static QString toString(const CastablePtr& value) { return QString::number(value.As<CppType>()); } \
    static bool less(const CastablePtr& f, const CastablePtr& s){ return f.As<CppType>() < s.As<CppType>(); } \
    static bool less_search(const CastablePtr& f, const CastablePtr& s){ return less(f,s); } \
    static void assign(void* dst, const void* src){ *(CppType*)dst = *(const CppType*)src; } \
};

DB_FOREACH_PRIMITIVE_FIELDS(DECL_PRIMITIVE_FIELD_TYPE_HELPER)
#define DECL_COMPLEX_FIELD_TYPE_HELPER(CppType, First) \
    template<> struct DbTableFieldTypeHelper<CppType>{ \
    typedef DbArray<CppType> DbCppType; \
    enum{ \
        Type = First, \
        Size = sizeof(MDbArrayBase), \
        isComplex = true \
    }; \
    static const char* GetCppTypeString(){ return QT_STRINGIFY(DbArray<CppType>); }  \
    static const char* valueName(){ return QT_STRINGIFY(CppType); } \
    static const char* GetDbTypeString(){ return #First; } \
    static void SetFromVariant(const QVariant& variant, CastablePtr& value) { DbPrivate::setFromVariantArray<DbCppType>(variant, value); } \
    static QString toString(const CastablePtr& value){ return DbPrivate::toString(value.As<DbCppType>()); } \
    static bool less(const CastablePtr& f, const CastablePtr& s){ return f.As<DbCppType>() < s.As<DbCppType>(); } \
    static bool less_search(const CastablePtr& f, const CastablePtr& s){ return f.As<DbCppType>() < s.As<CppType>(); } \
    static void assign(void* dst, const void* src){ *(DbCppType*)dst = *(const CppType*)src; } \
};
DB_FOREACH_COMPLEX_FIELDS(DECL_COMPLEX_FIELD_TYPE_HELPER)

#define DECL_SPECIAL_FIELDS_TYPE_HELPER(CppType,First) \
template<> struct DbTableFieldTypeHelper<CppType>{ \
    typedef CppType DbCppType; \
    enum{ \
        Type = First, \
        Size = sizeof(CppType), \
        isComplex = false \
    }; \
    static const char* GetCppTypeString(){ return QT_STRINGIFY(CppType); } \
    static const char* GetDbTypeString(){ return #First; } \
    static void SetFromVariant(const QVariant& variant, CastablePtr& value) { DbPrivate::setFromVariantPOD<CppType>(variant, value); } \
    static QString toString(const CastablePtr& f){ return DbPrivate::toString(f.As<DbCppType>()); } \
    static bool less(const CastablePtr& f, const CastablePtr& s){ return f.As<CppType>() < s.As<CppType>(); } \
    static bool less_search(const CastablePtr& f, const CastablePtr& s){ return less(f,s); } \
    static void assign(void* dst, const void* src){ *(CppType*)dst = *(const CppType*)src; } \
};
DB_FOREACH_POD_FIELDS(DECL_SPECIAL_FIELDS_TYPE_HELPER)

typedef std::function<void (CastablePtr& row)> DbDefaultValuesSetter;

struct MDbTableField : MDbObject
{
    qint16 Size;
    quint16 Offset;
    DbTableFieldType Type; // TODO. GETTER/SETTER
    qint32 Pos;
    DbString Name;
    DbString DefaultValue;

    DbDefaultValuesSetter CreateDefaultValueSetter() const;
    static DbDefaultValuesSetter CreateDefaultValueSetter(DbTableFieldType type, const QString& defaultValue);
    bool IsPrimaryKey() const { return m_isPrimaryKey; }
    void SetType(qint32 type);
    qint16 NextFieldOffset() const { return Offset + Size; }
    static const char* TypeToString(DbTableFieldType type);
    static size_t TypeSize(DbTableFieldType type);
    static DbTableFieldType TypeFromText(const QString& type_str);


private:
    void updatePlainData(const DbAddressMap& addressMap);
    bool m_isPrimaryKey;
    MDbTableField(DbTableFieldType Type, const QString& Name);
    ~MDbTableField(){}
    void Free();
    friend class DbTableHeader;
    friend class DbMemory;
    friend class DatabaseModifier;
};

DB_END_NAMESPACE

#endif // DBTABLEFIELD_H
