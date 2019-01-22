#ifndef DBMODELTABLESTABLE_H
#define DBMODELTABLESTABLE_H
#include <QAbstractTableModel>

template<class T> class Array;

class DbModelTablesTable : public QAbstractTableModel
{
public:
    enum Columns{
        ID,
        HeadID,
        Name,
        HeadName,
        LastID,
        RowCount,
        MaxNop
    };

    DbModelTablesTable(class DbDatabase* db, QObject *p=0);

    void AddTable(class DbTableHeader* head, const QString& tableName);
    void RemoveTables(const QModelIndexList& modelIndexes);
    class DbTable* GetTable(const QModelIndex& index) const;
    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QMap<qint32, QVariant> itemData(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
private:
    DbDatabase* m_database;
    const Array<DbTable*>& m_tables;
};


#endif // DBMODELTABLESTABLE_H
