#ifndef PROPERTIESMODEL_H
#define PROPERTIESMODEL_H
#include <QAbstractItemModel>
#include <functional>
#include "SharedModule/stack.h"

class Property;
class Name;

class PropertiesModel : public QAbstractItemModel
{
public:
    enum Role {
        RoleHeaderItem = Qt::UserRole,
        RoleMinValue,
        RoleMaxValue,
        RoleDelegateValue,
        RoleDelegateData
    };
    PropertiesModel(QObject* parent=0);

    void Update();

    void Save(const QString& fileName) const;
    void Load(const QString& fileName);

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
    friend class PropertiesModelInitializer;
    struct Item {
        QString Name;
        Item* Parent;
        qint32 ParentRow;
        Property* Prop;
        StackPointers<Item> Childs;
    };

    Item* asItem(const QModelIndex& index) const;
    void forEachItem(QString& path,
                     const Item* _root,
                     const std::function<void (const QString& path, const Item*)>& handle) const;
    void reset();
    void reset(const QHash<Name, Property*>& tree);

private:
    ScopedPointer<Item> _root;
};

#endif // PROPERTIESMODEL_H
