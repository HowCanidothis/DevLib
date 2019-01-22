#ifndef DBTABLEHEADER_H
#define DBTABLEHEADER_H
#include "dbtablefield.h"

DB_BEGIN_NAMESPACE

struct MDbTableHeaderDeclaration;
class DbMemory;
class DbTableFieldDelegate;

class DbTableHeader
{
    template<class T> friend class MergeRuleT;
    friend class ModifyRule;
    friend class DatabaseMerger;
    Q_DISABLE_COPY(DbTableHeader)
    friend class DbDatabase;
    friend struct std::default_delete<DbTableHeader>;
    friend class DbTable;
    friend class ArrayPointers<DbTableHeader>;
    friend class DatabaseConverterSQL;


    DbMemory* m_memory;
    MDbTableHeaderDeclaration* m_declaration;
    Array<MDbTableField*> m_fields;
    ArrayPointers<DbTableFieldDelegate> m_fieldDelegates;
    Array<quint32> m_complexFieldsOffsets;
    QVector<DbDefaultValuesSetter> m_defaultValues;
    qint32 m_size;

    DbTableHeader(DbMemory* m_memory, MDbTableHeaderDeclaration* m_declaration);

    void updatePlainData(const DbAddressMap& addressMap);
    void edit(const class DbModificationRule& rule);

    void setUsage(qint32 GetUsage);
    void setBegin(MDbTableHeaderDeclaration* m_declaration);
    void setField(MDbTableField* field); //locates from pos
    void setEnd();

    void free();
    void reset(); // free only fields
    ~DbTableHeader();

    void addField(MDbTableField* field);
    void addPrimaryKey();
public:
    struct OutCreateConcreteTableDataHeaderNameGenerator{
        QString NameSpace;
        QString StructName;
        QString ConcreteTableDataName;
        QString ConcreteTableDataNameLowline;
        QStringList VirtualStructNames;
        QStringList VirtualConcreteTableDataNames;
        OutCreateConcreteTableDataHeaderNameGenerator(const DbTableHeader *th, const QString& NameSpace);
    };

    void CreateConcreteTableDataHeader(QTextStream &stream,const OutCreateConcreteTableDataHeaderNameGenerator *name_gen) const;

    id_t GetID() const;
    bool IsValid() const { return this->m_declaration; }

    void IncrementUsage();
    void DecrementUsage();
    bool IsUsed() const { return GetUsage() != -1; }
    qint32 GetUsage() const;
    void SetName(const QString& name);
    const DbString& GetName() const;

    template<class T> void AddField(const QString& name, const QString& defaultValue = QString()) {
        AddField(DbTableFieldTypeHelper<T>::Type, name, defaultValue);
    }
    void AddField(DbTableFieldType type, const QString& name, const QString& defaultValue);

    void Edit(qint32 i, DbTableFieldType type, const QString& name, const QString& defaultValue) { Edit(m_fields.At(i), type, name, defaultValue); }
    void Edit(MDbTableField* tableField, DbTableFieldType type, const QString& name, const QString& defaultValue);
    void Remove(MDbTableField* field);

    void InitializeDefaultValues(CastablePtr& row) const;
    qint32 GetSize() const { return m_size; } //sizeof(all fields)
    qint32 IndexOf(const QString& name) const;
    qint32 GetFieldsCount() const { return this->m_fields.Size(); }
    const DbTableFieldDelegate* GetFieldDelegate(qint32 index) const { return this->m_fieldDelegates[index]; }
    const Array<MDbTableField*>& GetFields() const { return m_fields; }
    const MDbTableField* GetField(const QString& name) const;
};

DB_END_NAMESPACE

#endif // DBHEADER_H
