#include "dbmodificationrule.h"

#include "dbtableheader.h"
#include "Core/Private/dbtablefielddelegate.h"

DbCopyFieldDelegate::DbCopyFieldDelegate()
    : CopyFieldFunction(nullptr)
    , DestinationOffset(-1)
    , DefaultValueSetter([](CastablePtr&){})
{

}

void DbCopyFieldDelegate::Copy(CastablePtr& source, CastablePtr& destination) const
{
    CopyFieldFunction(source, destination);
    DefaultValueSetter(destination);
    source += SourceOffset;
    destination += DestinationOffset;
}

template<class OldFieldType, class NewFieldType>
static void copyPODField(const CastablePtr& source, CastablePtr& destination)
{
    memcpy(destination.AsPtr<void>(), source.AsPtr<void>(), sizeof(NewFieldType));
}

template<class NewFieldType>
static void createField(const CastablePtr&, CastablePtr& destination)
{
    new (destination.AsPtr<void>()) NewFieldType();
}

template<class OldFieldType, class NewFieldType>
static void copyField(const CastablePtr& source, CastablePtr& destination)
{
    destination.As<NewFieldType>() = source.As<OldFieldType>();
}

template<>
static void copyField<MDbArrayBase, MDbArrayBase>(const CastablePtr& source, CastablePtr& destination)
{
    // We can do this because destination array is always invalid
    memcpy(destination.AsPtr<void>(), source.AsPtr<void>(), sizeof(MDbArrayBase));
}

static void dontCopyField(const CastablePtr&, CastablePtr&){}

template<class FieldType>
static void removeField(const CastablePtr&, CastablePtr&){}

template<>
static void removeField<MDbArrayBase>(const CastablePtr& source, CastablePtr&)
{
    const_cast<MDbArrayBase&>(source.As<MDbArrayBase>()).Free();
}

// Arrays transform into primitive type without copying, only free their data
#define DECL_SWITCH(CppType, DbType) \
template<> void copyField<MDbArrayBase, CppType>(const CastablePtr& source, CastablePtr& ) \
{ \
    const_cast<MDbArrayBase&>(source.As<MDbArrayBase>()).Free();\
} \
template<> void copyField<CppType, MDbArrayBase>(const CastablePtr&, CastablePtr& destination) \
{ \
    new (destination.AsPtr<void>()) MDbArrayBase(); \
}
DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
DB_FOREACH_POD_FIELDS(DECL_SWITCH)

#undef DECL_SWITCH
#define DECL_SWITCH(CppType, DbType) \
    case DbType:  \
        rule.CopyFieldFunction = &copyField<DbTableFieldTypeHelper<CppType>::DbCppType, NewCppType>; \
        rule.SourceOffset = DbTableFieldTypeHelper<CppType>::Size; \
    break;
#define DECL_POD_SWITCH(CppType, DbType) \
    case DbType:  \
        rule.CopyFieldFunction = &createField<NewCppType>; \
        rule.SourceOffset = DbTableFieldTypeHelper<CppType>::Size; \
    break;

template<class NewCppType>
static void CreateRule(qint32 oldFieldType, qint32 newFieldType, DbCopyFieldDelegate& rule)
{
    rule.FieldType = newFieldType;
    rule.DestinationOffset = sizeof(NewCppType);
    switch (oldFieldType) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_POD_SWITCH)
        default:
            rule.CopyFieldFunction = &copyField<MDbArrayBase, NewCppType>;
            rule.SourceOffset = sizeof(MDbArrayBase);
        break;
    }
}

#undef DECL_SWITCH
#undef DECL_POD_SWITCH

#define DECL_SWITCH(CppType, DbType) \
    case DbType:  \
        rule.CopyFieldFunction = &createField<NewCppType>; \
        rule.SourceOffset = DbTableFieldTypeHelper<CppType>::Size; \
    break;

#define DECL_POD_SWITCH(CppType, DbType) \
    case DbType:  \
        rule.CopyFieldFunction = &copyPODField<DbTableFieldTypeHelper<CppType>::DbCppType, NewCppType>; \
        rule.SourceOffset = DbTableFieldTypeHelper<CppType>::Size; \
    break;

template<class NewCppType>
static void CreatePODRule(qint32 oldFieldType, qint32 newFieldType, DbCopyFieldDelegate& rule)
{
    rule.FieldType = newFieldType;
    rule.DestinationOffset = sizeof(NewCppType);
    switch (oldFieldType) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_POD_SWITCH)
        default:
            rule.CopyFieldFunction = &copyField<MDbArrayBase, NewCppType>;
            rule.SourceOffset = sizeof(MDbArrayBase);
        break;
    }
}


DbModificationRule::DbModificationRule(DbTableHeader* header)
    : m_header(header)
{
    m_copyDelegates.resize(m_header->GetFieldsCount());
}

#define DECL_SWITCH_NEW_TYPE(CppType, DbType) \
    case DbType: CreateRule<DbTableFieldTypeHelper<CppType>::DbCppType>(oldType, newType, delegate); break;
#define DECL_SWITCH_NEW_POD_TYPE(CppType, DbType) \
    case DbType: CreatePODRule<DbTableFieldTypeHelper<CppType>::DbCppType>(oldType, newType, delegate); break;

DbModificationRule&DbModificationRule::EditField(const MDbTableField* field, DbTableFieldType newType, const QString& fieldName, const QString& defaultValue)
{
    Q_ASSERT(!field->IsPrimaryKey());
    auto indexOf = m_header->GetFields().IndexOf(const_cast<MDbTableField*>(field));
    Q_ASSERT(indexOf != -1);
    MDbTableField* tableField = m_header->GetFields().At(indexOf);
    Q_ASSERT(tableField != nullptr);
    auto oldType = tableField->Type;

    auto& delegate = m_copyDelegates[indexOf];

    switch (newType) {
    DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH_NEW_TYPE)
    DB_FOREACH_POD_FIELDS(DECL_SWITCH_NEW_POD_TYPE)
    default:
        CreateRule<MDbArrayBase>(oldType, newType, delegate);
        break;
    }

    delegate.Name = fieldName;
    delegate.DefaultValue = defaultValue;

    return *this;
}

#undef DECL_SWITCH
#undef DECL_SWITCH_NEW_TYPE

#define DECL_SWITCH(CppType, DbType) \
    case DbType: \
        addFieldDelegate.CopyFieldFunction = &createField<DbTableFieldTypeHelper<CppType>::DbCppType>; \
        addFieldDelegate.DefaultValueSetter = MDbTableField::CreateDefaultValueSetter(DbType, defaultValue); \
    break;


DbModificationRule& DbModificationRule::AddField(DbTableFieldType type, const QString& fieldName, const QString& defaultValue)
{
    DbCopyFieldDelegate addFieldDelegate;
    addFieldDelegate.FieldType = type;
    addFieldDelegate.Name = fieldName;
    addFieldDelegate.SourceOffset = 0;
    addFieldDelegate.DestinationOffset = static_cast<qint32>(MDbTableField::TypeSize(type));
    addFieldDelegate.DefaultValue = defaultValue;
    switch (type) {
    DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH);
    DB_FOREACH_POD_FIELDS(DECL_SWITCH);
    DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH);
    default:
        break;
    }
    m_copyDelegates.append(addFieldDelegate);
    return *this;
}

#undef DECL_SWITCH

DbModificationRule&DbModificationRule::RemoveField(const QString& fieldName)
{
    auto field = m_header->GetField(fieldName);
    Q_ASSERT(field != nullptr);
    return RemoveField(field);
}

DbModificationRule&DbModificationRule::RemoveField(const MDbTableField* field)
{
    auto indexOf = m_header->GetFields().IndexOf(const_cast<MDbTableField*>(field));
    Q_ASSERT(indexOf != -1 && !field->IsPrimaryKey());
    auto& delegate = m_copyDelegates[indexOf];
    delegate.DestinationOffset = 0;
    DbTableFieldDelegate fieldDelegate(field);
    if(fieldDelegate.IsComplex()) {
        delegate.CopyFieldFunction = &removeField<MDbArrayBase>;
    } else {
        delegate.CopyFieldFunction = &removeField<void>;
    }
    delegate.SourceOffset = field->Size;
    return *this;
}

#define DECL_SWITCH(CppType, DbType) \
    case DbType: \
    it->CopyFieldFunction = &copyField<DbTableFieldTypeHelper<CppType>::DbCppType, DbTableFieldTypeHelper<CppType>::DbCppType>; \
    it->SourceOffset = it->DestinationOffset = oldField->Size; \
    break;

void DbModificationRule::initialize()
{
    auto it = m_copyDelegates.begin();

    for(MDbTableField* oldField : m_header->GetFields()) {
        if(it->CopyFieldFunction == nullptr) {
            if(it->DestinationOffset != 0) {
                it->FieldType = oldField->Type;
                it->Name = oldField->Name.ToString();
                it->DefaultValue = oldField->DefaultValue.ToString();
                switch (oldField->Type) {
                DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
                DB_FOREACH_POD_FIELDS(DECL_SWITCH)
                DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
                default: Q_ASSERT(false);
                    break;
                }
            }
        }
        it++;
    }
}

#undef DECL_SWITCH

void DbModificationRule::copyRow(const CastablePtr& source,const CastablePtr& destination) const
{
    CastablePtr movableSource(source);
    CastablePtr movableDestination(destination);
    for(const DbCopyFieldDelegate& delegate : m_copyDelegates) {
        delegate.Copy(movableSource, movableDestination);
    }
}
