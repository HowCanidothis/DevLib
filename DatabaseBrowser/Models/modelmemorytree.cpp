#include "modelmemorytree.h"
#include <DatabaseEngine/Core/memory/dbmemory.h>

enum Type{
    Invalid,
    Sector,
    Fragment
};

ModelMemoryTree::ModelMemoryTree(const DbMemory* memory, QObject* parent)
    : QAbstractItemModel(parent)
    , m_memory(memory)
{

}

void ModelMemoryTree::Reset()
{
    m_sectorFragmentsCache.clear();
    emit layoutChanged();
}

QModelIndex ModelMemoryTree::index(int row, int column, const QModelIndex& parent) const
{
    return !hasIndex(row, column, parent) ? QModelIndex() :
    parent.isValid() ? createIndex(row, column, parent.row()) :
    createIndex(row, column, -1);
}

QModelIndex ModelMemoryTree::parent(const QModelIndex& child) const
{
    if(child.internalId() == -1) {
        return QModelIndex();
    }

    return index(child.internalId(), 0, QModelIndex());
}

int ModelMemoryTree::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid()) {
        return m_memory->Size();
    }
    if(!parent.parent().isValid()) {
        return sectorCount(parent.row());
    }
    return 0;
}

int ModelMemoryTree::columnCount(const QModelIndex&) const
{
    return Column_Count;
}

QVariant ModelMemoryTree::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        if(index.column() == 0) {
            return index.row();
        }
        if(!index.parent().isValid()) {
            return QVariant();
        }

        switch (index.column()) {
            case Column_IsFree: return asFragment(index)->GetFree();
            case Column_Location: return calculateFragmentLocation(index);
            case Column_Size: return asFragment(index)->GetSize();
            case Column_Reason: return QMetaEnum::fromType<DbMemoryFragment::Reason>().valueToKey(asFragment(index)->GetReason());
            case Column_Id: return asFragment(index)->GetID();
            default: return QVariant();
        }
        break;
    default:
        break;
    }

    return QVariant();
}

QVariant ModelMemoryTree::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case Column_Index: return "Index";
        case Column_Location: return "Location";
        case Column_IsFree: return "Free";
        case Column_Size: return "Size";
        case Column_Reason: return "Reason";
        case Column_Id: return "Alloc id";
        default:
            break;
        }
        break;
    default:
        break;
    }

    return QVariant();
}


qint32 ModelMemoryTree::sectorCount(qint32 j) const
{
    auto sector = m_memory->At(j);
    SectorsCountCache::iterator find = m_sectorFragmentsCache.find(sector);
    if(find != m_sectorFragmentsCache.end()){
        SectorFragmentsCache& sectorCachedInfo = find.value();
        return sectorCachedInfo.Fragments.Size();
    }

    auto fragments = sector->ToFragmentsArray();
    m_sectorFragmentsCache.insert(sector, SectorFragmentsCache(fragments));
    return fragments.Size();
}

const DbMemoryFragment* ModelMemoryTree::asFragment(const QModelIndex& i) const
{
    auto res = m_sectorFragmentsCache[m_memory->At(i.internalId())].Fragments[i.row()];
    return res;
}

qint32 ModelMemoryTree::calculateFragmentLocation(const QModelIndex& i) const
{
    return m_memory->At(i.internalId())->Offset(asFragment(i));
}
