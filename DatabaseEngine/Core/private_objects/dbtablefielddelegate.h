#ifndef DBTABLEFIELDDELEGATE_H
#define DBTABLEFIELDDELEGATE_H

#include "DatabaseEngine/db_decl.h"

DB_BEGIN_NAMESPACE

struct MDbTableField;

class DbTableFieldDelegate
{
    typedef bool (*Comparator)(const CastablePtr& f, const CastablePtr& s);
    typedef void (*Assigner)(void* f, const void* s);
    typedef QString (*StringSerializer)(const CastablePtr& f);

    Comparator m_comparator;
    Comparator m_searchComparator;
    Assigner m_assigner;
    StringSerializer m_stringSerializer;
    bool m_isComplex;
    quint16 m_size;
    quint16 m_offset;

public:
    DbTableFieldDelegate(const MDbTableField* field);
    DbTableFieldDelegate(quint16 m_offset, DbTableFieldType type);

    Comparator GetSearchComparator() const { return m_searchComparator; } // Use inc
    Comparator GetComparator() const { return m_comparator; }
    Assigner GetAssigner() const { return m_assigner; }
    StringSerializer GetStringSerializer() const { return m_stringSerializer; }
    bool IsComplex() const { return m_isComplex; }
    quint16 GetSize() const { return m_size; }
    quint16 GetOffset() const { return m_offset; }
};

DB_END_NAMESPACE

Q_DECLARE_TYPEINFO(DB_USE_NAMESPACE(DbTableFieldDelegate), Q_PRIMITIVE_TYPE);

#endif // DBTABLEFIELDDELEGATE_H
