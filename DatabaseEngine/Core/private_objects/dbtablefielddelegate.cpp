#include "dbtablefielddelegate.h"
#include "DatabaseEngine/dbtablefield.h"

DB_BEGIN_NAMESPACE

DbTableFieldDelegate::DbTableFieldDelegate(const MDbTableField* field) : DbTableFieldDelegate(field->Offset, field->Type)
{

}

#define DECL_SWITCH(CppType, First) \
    case First: m_comparator = DbTableFieldTypeHelper<CppType>::less; \
                m_searchComparator = DbTableFieldTypeHelper<CppType>::less_search; \
                m_assigner = DbTableFieldTypeHelper<CppType>::assign; \
                m_stringSerializer = DbTableFieldTypeHelper<CppType>::toString; \
                m_isComplex = DbTableFieldTypeHelper<CppType>::isComplex; \
                m_size = DbTableFieldTypeHelper<CppType>::Size; \
                m_offset = offset; \
    break;
DbTableFieldDelegate::DbTableFieldDelegate(quint16 offset, DbTableFieldType type)
{
    switch (type) {
        DB_FOREACH_PRIMITIVE_FIELDS(DECL_SWITCH)
        DB_FOREACH_POD_FIELDS(DECL_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH)
        default: Q_ASSERT_X(false, "TableHead", "Unable add unknown field");
    }
}

DB_END_NAMESPACE
