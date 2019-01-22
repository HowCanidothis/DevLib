#ifndef DBMODIFICATIONRULE_H
#define DBMODIFICATIONRULE_H

#include "dbtablefield.h"

class DbTableHeader;

struct DbCopyFieldDelegate
{
    QString Name;
    DbTableFieldType FieldType;
    void (*CopyFieldFunction)(const CastablePtr& source, CastablePtr& destination);
    qint32 SourceOffset;
    qint32 DestinationOffset;
    QString DefaultValue;
    DbDefaultValuesSetter DefaultValueSetter;

    DbCopyFieldDelegate();
    void Copy(CastablePtr& source, CastablePtr& destination) const;
};

class DbModificationRule
{
public:
    DbModificationRule(DbTableHeader* header);

    DbModificationRule& EditField(const MDbTableField* field, DbTableFieldType newType, const QString& fieldName, const QString& defaultValue = QString());
    template<class T> DbModificationRule& EditField(const MDbTableField* field, const QString& fieldName, const QString& defaultValue = QString());

    DbModificationRule& AddField(DbTableFieldType type, const QString& fieldName, const QString& defaultValue);
    template<class T> DbModificationRule& AddField(const QString& fieldName, const QString& defaultValue);

    DbModificationRule& RemoveField(const QString& fieldName);
    DbModificationRule& RemoveField(const MDbTableField* tableField);

    DbTableHeader* GetHeader() const { return m_header; }

private:
    friend class DbTableHeader;
    friend class DbTable;
    friend class DbDatabase;
    void initialize();
    void copyRow(const CastablePtr& source, const CastablePtr& destination) const;

private:
    DbTableHeader* m_header;
    QVector<DbCopyFieldDelegate> m_copyDelegates;
};

template<class T>
DbModificationRule& DbModificationRule::EditField(const MDbTableField* field, const QString& fieldName, const QString& defaultValue)
{
    return EditField(field, DbTableFieldTypeHelper<T>::Type, fieldName, defaultValue);
}

template<class T>
DbModificationRule& DbModificationRule::AddField(const QString& fieldName, const QString& defaultValue)
{
    return AddField(DbTableFieldTypeHelper<T>::Type, fieldName, defaultValue);
}

#endif // DBMODIFICATIONRULE_H
