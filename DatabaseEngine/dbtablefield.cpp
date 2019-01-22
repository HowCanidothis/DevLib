#include "dbtablefield.h"

DB_BEGIN_NAMESPACE

#define DECL_SWITCH(CppType, First) \
    else if(DbTableFieldTypeHelper<CppType>::GetDbTypeString() == type_str) { \
        return First; \
    }
DbTableFieldType MDbTableField::TypeFromText(const QString &type_str)
{
    if(type_str.isEmpty()) {
        return DbTableFieldType::NotStorable;
    }
    DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
    DB_FOREACH_POD_FIELDS(DECL_SWITCH)
    DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
    else {
        return DbTableFieldType::NotStorable;
    }
}

void MDbTableField::updatePlainData(const DbAddressMap& addressMap)
{
    Name.updatePlainData(addressMap);
    DefaultValue.updatePlainData(addressMap);
}

#undef DECL_SWITCH
#define DECL_SWITCH(CppType, DbType) \
    case DbType: { \
        DbTableFieldTypeHelper<CppType>::DbCppType wrappedValue; \
        DbTableFieldTypeHelper<CppType>::SetFromVariant(DefaultValue.ToString(), CastablePtr(&wrappedValue)); \
        return [this, wrappedValue](CastablePtr& row) { \
            new ((row + Offset).AsPtr<void>()) DbTableFieldTypeHelper<CppType>::DbCppType(wrappedValue); \
        }; \
    }
#define DECL_COMPLEX_SWITCH(CppType, DbType) \
    case DbType: return [this](CastablePtr& row) { \
        (row + Offset).As<DbArray<CppType>>().FromString(DefaultValue.ToString()); \
        };

DbDefaultValuesSetter MDbTableField::CreateDefaultValueSetter() const
{
    switch (Type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_COMPLEX_SWITCH)
        default:
            Q_ASSERT(false);
    }
    return DbDefaultValuesSetter();
}

#undef DECL_COMPLEX_SWITCH
#undef DECL_SWITCH
#define DECL_SWITCH(CppType, DbType) \
    case DbType: { \
        DbTableFieldTypeHelper<CppType>::DbCppType wrappedValue; \
        DbTableFieldTypeHelper<CppType>::SetFromVariant(defaultValue, CastablePtr(&wrappedValue)); \
        return [wrappedValue, defaultValue](CastablePtr& row) { \
            new ((row).AsPtr<void>()) DbTableFieldTypeHelper<CppType>::DbCppType(wrappedValue); \
        }; \
    }
#define DECL_COMPLEX_SWITCH(CppType, DbType) \
    case DbType: return [defaultValue](CastablePtr& row) { \
        (row).As<DbArray<CppType>>().FromString(defaultValue); \
        };

DbDefaultValuesSetter MDbTableField::CreateDefaultValueSetter(DbTableFieldType type, const QString& defaultValue)
{
    switch (type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_COMPLEX_SWITCH)
        default:
            Q_ASSERT(false);
    }
    return DbDefaultValuesSetter();
}

#undef DECL_SWITCH
#define DECL_SWITCH(CppType, First) \
    case First: return DbTableFieldTypeHelper<CppType>::GetDbTypeString(); break;

const char* MDbTableField::TypeToString(DbTableFieldType type) {
    switch (type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
        default: return "";
    }
}

#undef DECL_SWITCH
#define DECL_SWITCH(CppType, First) \
    case First: return DbTableFieldTypeHelper<CppType>::Size; break;
size_t MDbTableField::TypeSize(DbTableFieldType type)
{
    switch (type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
        default: Q_ASSERT(true);
    }
    return 0;
}

#undef DECL_SWITCH
#define DECL_SWITCH(CppType, First) \
    case First: Size = DbTableFieldTypeHelper<CppType>::Size; break;

void MDbTableField::SetType(qint32 type)
{
    switch (type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
        default: break;
    }
    Type = type;
}
#undef DECL_SWITCH

MDbTableField::MDbTableField(DbTableFieldType type, const QString& name)
    : Name(name)
    , m_isPrimaryKey(false)
{
    SetType(type);
}

void MDbTableField::Free()
{
    Super::Free();
    Name.Free();
    DefaultValue.Free();
}

DB_END_NAMESPACE
