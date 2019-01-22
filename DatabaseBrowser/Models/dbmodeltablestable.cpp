#include "dbmodeltablestable.h"
#include <DatabaseEngine/internal.hpp>

DbModelTablesTable::DbModelTablesTable(DbDatabase *db, QObject *p)
    : QAbstractTableModel(p)
    , m_database(db)
    , m_tables(db->GetContentHandler()->GetTables())
{

}

void DbModelTablesTable::AddTable(DbTableHeader* head, const QString& tableName)
{
    qint32 size = m_tables.Size();
    beginInsertRows(QModelIndex(), size, size);
    m_database->AddTable(head, tableName);
    endInsertRows();
}

void DbModelTablesTable::RemoveTables(const QModelIndexList& modelIndexes)
{
    if(modelIndexes.isEmpty()) {
        return;
    }

    Stack<DbTable*> tablesToRemove;

    emit layoutAboutToBeChanged();
    for(const QModelIndex& mi : modelIndexes) {
        tablesToRemove.Append(GetTable(mi));
    }
    for(auto tableToRemove : tablesToRemove) {
        m_database->RemoveTable(tableToRemove);
    }

    emit layoutChanged();
}

DbTable*DbModelTablesTable::GetTable(const QModelIndex& index) const
{
    return m_tables.At(index.row());
}

int DbModelTablesTable::rowCount(const QModelIndex&) const
{
    return m_tables.Size();
}

int DbModelTablesTable::columnCount(const QModelIndex&) const
{
    return 7;
}

bool DbModelTablesTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) return false;
    if(role == Qt::EditRole){
        switch (index.column()) {
        case Name:
            m_tables.At(index.row())->SetName(value.toString()); 
            emit dataChanged(index, index);
            return true;
        case HeadName: m_tables.At(index.row())->SetHeaderName(value.toString()); return true;
        default:
            break;
        }
    }
    return false;
}

QVariant DbModelTablesTable::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case ID: return m_tables.At(index.row())->GetID();
        case HeadID: return m_tables.At(index.row())->GetHeader()->GetID();
        case Name: return m_tables.At(index.row())->GetName().ToString();
        case HeadName: return m_tables.At(index.row())->GetHeaderName().ToString();
        case LastID: return m_tables.At(index.row())->GetLastId();
        case RowCount: return m_tables.At(index.row())->Size();
        case MaxNop: return 0;
        default:
            break;
        }
    default:
        break;
    }
    return QVariant();
}

QVariant DbModelTablesTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case ID: return tr("Table id");
        case HeadID: return tr("Head id");
        case Name: return tr("Table name");
        case HeadName: return tr("Head name");
        case LastID: return tr("Last id");
        case RowCount: return tr("Row count");
        case MaxNop: return tr("Max N operation(synch field)");
        default:
            break;
        }
        break;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags DbModelTablesTable::flags(const QModelIndex& index) const
{
    return (index.column() == Name || index.column() == HeadName) ? (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled) : (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QMap<qint32, QVariant> DbModelTablesTable::itemData(const QModelIndex& index) const
{
    QMap<qint32, QVariant> res;
    res[Qt::DisplayRole] = index.data();
    res[Qt::UserRole] = index.data(Qt::UserRole);
    return res;
}

bool DbModelTablesTable::removeRows(int row, int count, const QModelIndex&)
{
    if(!count) return false;
    qint32 from = row;
    qint32 to = row + count;
    qint32 tomo = to - 1;
    beginRemoveRows(QModelIndex(), from, tomo);
    Stack<DbTable*> tablesToRemove;
    for(qint32 i(from); i < to; i++) {
        tablesToRemove.Append(m_tables.At(row));
    }
    /*
    for(DbTable* t : tablesToRemove)
        db->RemoveTable(t); */
    endRemoveRows();
    return true;
}
