#ifndef DBMODELMEMORYTREE_H
#define DBMODELMEMORYTREE_H

#include <QAbstractItemModel>

#include <SharedModule/internal.hpp>

class ModelMemoryTree : public QAbstractItemModel
{
public:
    enum Columns
    {
        Column_Index = 0,
        Column_Location,
        Column_IsFree,
        Column_Size,
        Column_Reason,
        Column_Id,
        Column_Count
    };

    ModelMemoryTree(const class DbMemory* memory, QObject* parent=nullptr);

public:
    void Reset();

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex& child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

private:
    struct SectorFragmentsCache
    {
        Array<const class DbMemoryFragment*> Fragments;

        SectorFragmentsCache(){}
        SectorFragmentsCache(Array<const DbMemoryFragment*> fragments)
            : Fragments(fragments)
        {}
    };
    typedef QHash<const class DbMemorySector*, SectorFragmentsCache> SectorsCountCache;

    const DbMemory* m_memory;
    mutable SectorsCountCache m_sectorFragmentsCache;

    qint32 sectorCount(qint32 j) const;
    const DbMemoryFragment* asFragment(const QModelIndex& i) const;
    qint32 calculateFragmentLocation(const QModelIndex& i) const;
};












#endif // DBVISUALMODEL_H
