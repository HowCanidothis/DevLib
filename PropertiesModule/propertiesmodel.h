#ifndef PROPERTIESMODEL_H
#define PROPERTIESMODEL_H
#include <QAbstractItemModel>

#include "Shared/stack.h"

#ifdef WORK
#include <functional>
#endif

class Property;
class Name;

class PropertiesModel : public QAbstractItemModel
{
    friend class PropertiesModelInitializer;
    struct Item {
        QString name;
        Item* parent;
        qint32 parent_row;
        Property* property;
        StackPointers<Item> childs;
    };
public:
    enum Role {
        RoleHeaderItem = Qt::UserRole,
        RoleMinValue,
        RoleMaxValue,
        RoleIsTextFileName
    };
    PropertiesModel(QObject* parent=0);

    void update();

    void save(const QString& file_name) const;
    void load(const QString& file_name);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& index, const QVariant& value, int role) Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& child) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;

private:
    Item* asItem(const QModelIndex& index) const;
    void forEachItem(QString& path,
                     const Item* root,
                     const std::function<void (const QString& path, const Item*)>& handle) const;
    void reset();
    void reset(const QHash<Name, Property*>& tree);
private:
    ScopedPointer<Item> root;
};

#endif // PROPERTIESMODEL_H
