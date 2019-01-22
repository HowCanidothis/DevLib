#include "dbmodelheadstree.h"

#include <DatabaseEngine/internal.hpp>

DbModelHeadsTree::DbModelHeadsTree(DbDatabase* db, QObject* parent)
    : QAbstractItemModel(parent)
    , m_database(db)
    , m_headers(db->GetContentHandler()->GetTableHeaders())
{
}

void DbModelHeadsTree::AddFieldToHeader(DbTableHeader* header, qint32 type, const QString& name, const QString& defaultValue)
{
    Q_ASSERT(header);
    emit layoutAboutToBeChanged();
    if(!header->IsUsed()) {
        header->AddField(type, name, defaultValue);
    } else {
        DbModificationRule modificationRule(header);
        modificationRule.AddField(type, name, defaultValue);
        m_database->Edit(modificationRule);
    }
    emit layoutChanged();
}

DbTableHeader*DbModelHeadsTree::AsHead(const QModelIndex& mi) const
{
    if(mi.parent().isValid()) {
        return nullptr;
    }
    return asHead(mi);
}

MDbTableField*DbModelHeadsTree::AsField(const QModelIndex& mi, DbTableHeader*& header) const
{
    if(!mi.parent().isValid()) {
        return nullptr;
    }
    header = asHead(mi.parent());
    return asField(mi);
}

void DbModelHeadsTree::AddTableHeader(const QString& name)
{
    qint32 size = m_headers.Size();
    beginInsertRows(QModelIndex(), size, size);
    m_database->AddHeader(name);
    endInsertRows();
}

void DbModelHeadsTree::EditTableField(DbTableHeader* header, MDbTableField* field, qint32 type, const QString& name, const QString& defaultValue)
{
    if(!header->IsUsed()) {
        header->Edit(field, type, name, defaultValue);
    } else {
        DbModificationRule modificationRule(header);
        modificationRule.EditField(field, type, name, defaultValue);
        m_database->Edit(modificationRule);
    }
}

void DbModelHeadsTree::EditTableHeader(DbTableHeader* header, const QString& name)
{
    emit layoutAboutToBeChanged();
    header->SetName(name);
    emit layoutChanged();
}

void DbModelHeadsTree::RemoveTableHead(const QModelIndex& current)
{
    Q_ASSERT(current.isValid() && !current.parent().isValid());
    DbTableHeader* th = asHead(current);
    beginRemoveRows(QModelIndex(), current.row(), current.row());
    m_database->RemoveHeader(th);
    endRemoveRows();
}

void DbModelHeadsTree::RemoveTableField(const QModelIndex& current)
{
    Q_ASSERT(current.isValid() && current.parent().isValid());
    QModelIndex parentIndex = parent(current);
    DbTableHeader* header = this->asHead(parentIndex);
    emit this->layoutAboutToBeChanged();
    if(!header->IsUsed())
        header->Remove(asField(current));
    else {
        DbModificationRule modificationRule(header);
        modificationRule.RemoveField(asField(current));
        m_database->Edit(modificationRule);
    }
    emit layoutChanged();
}

struct DbModelHeadsTreeRemoveDesc
{
    Stack<MDbTableField*> FieldsToRemove;
    bool IsRemoveHeader;
};

void DbModelHeadsTree::Remove(const QModelIndexList& indexes)
{
    QHash<DbTableHeader*, DbModelHeadsTreeRemoveDesc> removeTree;
    for(const auto& index : indexes) {
        Q_ASSERT(index.isValid());
        if(!index.parent().isValid()) {
            removeTree[asHead(index)].IsRemoveHeader = true;
        } else {
            removeTree[asHead(index.parent())].FieldsToRemove.Append(asField(index));
        }
    }

    auto it = removeTree.begin();
    auto end = removeTree.end();
    emit layoutAboutToBeChanged();
    for(; it != end; ++it) {
        DbTableHeader* header = it.key();
        if(it.value().IsRemoveHeader) {
            m_database->RemoveHeader(header);
        } else {
            if(!header->IsUsed()) {
                for(auto field : it.value().FieldsToRemove) {
                    if(field->IsPrimaryKey()) {
                        continue;
                    }
                    header->Remove(field);
                }
            } else {
                DbModificationRule modificationRule(header);
                for(MDbTableField* field : it.value().FieldsToRemove) {
                    if(field->IsPrimaryKey()) {
                        continue;
                    }
                    modificationRule.RemoveField(field);
                }
                m_database->Edit(modificationRule);
            }
        }
    }
    emit layoutChanged();
}

DbTableHeader* DbModelHeadsTree::asHead(const QModelIndex& mi) const
{
    return m_headers.At(mi.row());
}

MDbTableField* DbModelHeadsTree::asField(const QModelIndex& mi) const
{
    return m_headers.At(mi.internalId())->GetFields().At(mi.row());
}

QModelIndex DbModelHeadsTree::index(int row, int column, const QModelIndex& parent) const
{
    return !hasIndex(row, column, parent) ? QModelIndex() :
    parent.isValid() ? createIndex(row, column, parent.row()) :
    createIndex(row, column, -1);
}

QModelIndex DbModelHeadsTree::parent(const QModelIndex& child) const
{
    if(child.internalId() == -1) {
        return QModelIndex();
    }

    return index(child.internalId(), 0, QModelIndex());
}

int DbModelHeadsTree::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid()) {
        return m_headers.Size();
    }
    if(!parent.parent().isValid()) {
        return m_headers.At(parent.row())->GetFieldsCount();
    }
    return 0;
}

int DbModelHeadsTree::columnCount(const QModelIndex&) const
{
    return FC_Count;
}

QVariant DbModelHeadsTree::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        if(index.column() == 0) {
            return index.row();
        }
        if(!index.parent().isValid()) {
            switch (index.column()) {
                case TH_Id: return asHead(index)->GetID();
                case TH_Name: return asHead(index)->GetName().ToString();
                case TH_RowSize: return asHead(index)->GetSize();
                case TH_Usage: return asHead(index)->GetUsage();
                default: return QVariant();
            }
        }

        switch (index.column()) {
            case FC_Index: return asField(index)->Pos;
            case FC_Name: return asField(index)->Name.ToString();
            case FC_Offset: return asField(index)->Offset;
            case FC_Size: return asField(index)->Size;
            case FC_Type: return  QString(MDbTableField::TypeToString(asField(index)->Type)).remove(0, 18);
            case FC_DefaultValue: return asField(index)->DefaultValue.ToString();
            default: return QVariant();
        }
        break;
    default:
        break;
    }

    return QVariant();
}

QVariant DbModelHeadsTree::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case FC_Index: return "Id / Index";
        case FC_Type: return "Usage / Type";
        case FC_Name: return "Name";
        case FC_Offset: return "Offset";
        case FC_Size: return "Size";
        case FC_DefaultValue: return "Default value";
        default:
            break;
        }
    default:
        break;
    }

    return QVariant();
}

QMap<qint32, QVariant> DbModelHeadsTree::itemData(const QModelIndex &index) const
{
    QMap<qint32, QVariant> res;
    res[Qt::DisplayRole] = index.data();
    res[Qt::UserRole] = index.data(Qt::UserRole);
    return res;
}

Qt::ItemFlags DbModelHeadsTree::flags(const QModelIndex &index) const
{
    Qt::ItemFlags default_flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.column() == FC_Name && index.parent().isValid())
        default_flags |= Qt::ItemIsDragEnabled;
    return default_flags;
}
