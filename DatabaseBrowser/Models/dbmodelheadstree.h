#ifndef DBMODELHEADSTABLE_H
#define DBMODELHEADSTABLE_H
#include <QAbstractItemModel>

template<class T> class Array;

class DbModelHeadsTree : public QAbstractItemModel
{
public:
    enum TableHeaderColumns
    {
        TH_Id = 1,
        TH_Name,
        TH_RowSize,
        TH_Usage
    };

    enum FieldColumns
    {
        FC_Index = 1,
        FC_Name,
        FC_Size,
        FC_Type,
        FC_Offset,
        FC_DefaultValue,
        FC_Count,
    };

    DbModelHeadsTree(class DbDatabase* db, QObject *parent=0);

    void AddFieldToHeader(class DbTableHeader* header, qint32 type, const QString& name, const QString& defaultValue);
    void AddTableHeader(const QString& name);
    void EditTableField(DbTableHeader* header, struct MDbTableField* field, qint32 type, const QString& name, const QString& defaultValue);
    void EditTableHeader(DbTableHeader* header, const QString& name);
    void RemoveTableHead(const QModelIndex& current);
    void RemoveTableField(const QModelIndex& current);
    void Remove(const QModelIndexList& indexes);

    class DbTableHeader* AsHead(const QModelIndex& mi) const;
    struct MDbTableField* AsField(const QModelIndex& mi, DbTableHeader*& header) const;

    // QAbstractItemModel interface
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex& child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    virtual QMap<qint32, QVariant> itemData(const QModelIndex& index) const;

private:
    DbTableHeader* asHead(const QModelIndex& mi) const;
    struct MDbTableField* asField(const QModelIndex& mi) const;

private:
    DbDatabase* m_database;
    const Array<DbTableHeader*>& m_headers;
};







#endif // DBHEADSTABLEMODEL_H
