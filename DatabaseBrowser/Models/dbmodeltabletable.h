#ifndef DBMODELTABLETABLE_H
#define DBMODELTABLETABLE_H
#include <QAbstractTableModel>
#include <DatabaseEngine/internal.hpp>

class DbModelTableTable : public QAbstractTableModel
{
    DbTable* m_table;
public:
    DbModelTableTable(DbTable* table, QObject *p=0);

    void Add(qint32 count);
    void Remove(const QModelIndexList& range);

    // QAbstractItemModel interface
public:
    virtual void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    template<class T> T& changeValue(const QModelIndex& mi) const
    {
        auto row = m_table->RowAt(mi.row());
        return m_table->Change<T>(row, mi.column());
    }
    template<class T> const T& getValue(const QModelIndex& mi) const
    {
        return m_table->At<T>(mi.row(),mi.column());
    }
    const DbTableRow getRow(const QModelIndex& mi) const;
};

#endif // DBMODELTABLETABLE_H
