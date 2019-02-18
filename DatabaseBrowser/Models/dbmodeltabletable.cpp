#include "dbmodeltabletable.h"
#include <DatabaseEngine/Core/private_objects/dbtablefielddelegate.h>

DbModelTableTable::DbModelTableTable(DbTable* table, QObject *p)
    : QAbstractTableModel(p)
    , m_table(table)
{
}

void DbModelTableTable::Add(qint32 count)
{
    qint32 size = this->m_table->Size();
    beginInsertRows(QModelIndex(), size, size + count);
    m_table->Append(count);
    m_table->Update();
    endInsertRows();
}

void DbModelTableTable::Remove(const QModelIndexList& range)
{
    if(range.isEmpty()) {
        return;
    }

    Stack<DbTableRow> rowsToRemove;
    for(const auto& mi : range) {
        rowsToRemove.Append(getRow(mi));
    }

    beginRemoveRows(QModelIndex(), 0, m_table->Size() - 1);

    for(const auto& toRemove : rowsToRemove) {
        m_table->Remove(toRemove);
    }
    m_table->Update();

    endRemoveRows();
}

void DbModelTableTable::sort(int column, Qt::SortOrder)
{
    emit layoutAboutToBeChanged();
    m_table->Sort(column);
    emit layoutChanged();
}

int DbModelTableTable::rowCount(const QModelIndex&) const
{
    return m_table->Size();
}

int DbModelTableTable::columnCount(const QModelIndex&) const
{
    return m_table->GetHeader()->GetFieldsCount();
}

#define PRIMITIVE_SWITCH(CppType, Type) \
case Type: changeValue<CppType>(index) = value.toDouble(); emit dataChanged(index, index); return true;

#define COMPLEX_SWITCH(CppType, Type) \
case Type: changeValue<DbArray<CppType>>(index).FromString(value.toString()); emit dataChanged(index, index); return true;

#define POD_SWITCH(CppType, Type) \
case Type: changeValue<CppType>(index).FromString(value.toString()); emit dataChanged(index, index); return true;

bool DbModelTableTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) return false;
    switch (role) {
    case Qt::EditRole:
        switch (m_table->GetHeader()->GetFields()[index.column()]->Type) {
        DB_FOREACH_PRIMITIVE_FIELDS(PRIMITIVE_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(COMPLEX_SWITCH)
        DB_FOREACH_POD_FIELDS(POD_SWITCH)
        default: break;
        }
        break;
    default:
        break;
    }
    return false;
}

#undef PRIMITIVE_SWITCH
#define PRIMITIVE_SWITCH(CppType, Type) \
case Type: return getValue<CppType>(index);

#undef COMPLEX_SWITCH
#define COMPLEX_SWITCH(CppType, DbType) \
case DbType: return DbTableFieldTypeHelper<CppType>::ToString(&getValue<DbTableFieldTypeHelper<CppType>::DbCppType>(index));

#undef POD_SWITCH
#define POD_SWITCH(CppType, Type) \
case Type: return getValue<CppType>(index).ToString();


QVariant DbModelTableTable::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (m_table->GetHeader()->GetFields()[index.column()]->Type) {
        DB_FOREACH_PRIMITIVE_FIELDS(PRIMITIVE_SWITCH)
        DB_FOREACH_COMPLEX_FIELDS(COMPLEX_SWITCH)
        DB_FOREACH_POD_FIELDS(POD_SWITCH)
        default: break;
        }
    default:
        break;
    }
    return QVariant();
}

QVariant DbModelTableTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole: return m_table->GetHeader()->GetFields().At(section)->Name.ToString();
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags DbModelTableTable::flags(const QModelIndex &index) const
{
    return index.column() ?  (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

const DbTableRow DbModelTableTable::getRow(const QModelIndex& mi) const
{
    return m_table->RowAt(mi.row());
}
