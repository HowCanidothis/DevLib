#ifndef PROPERTIESMODEL_H
#define PROPERTIESMODEL_H
#include <QAbstractItemModel>
#include <functional>
#include "SharedModule/internal.hpp"

#include "localproperty.h"
#include "propertiessystem.h"

class Property;
class Name;

class _Export PropertiesModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(qint32 contextIndex MEMBER ContextIndex NOTIFY contextIndexChanged)
    Q_PROPERTY(QString fileName MEMBER FileName NOTIFY fileNameChanged)
public:
    PropertiesModel(QObject* parent=0);
    PropertiesModel(qint32 contextIndex, QObject* parent=0);

    LocalProperty<properties_context_index_t> ContextIndex;
    LocalProperty<QString> FileName;

Q_SIGNALS:
    void fileNameChanged();
    void contextIndexChanged();

public:
    void Change(const std::function<void ()>& handle);

    Q_SLOT void Save(const QString& fileName) const;
    Q_SLOT void Load(const QString& fileName);

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

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

private:
    Q_SLOT void update();

private:
    struct Item {
        QString Name;
        Item* Parent;
        qint32 ParentRow;
        Property* Prop;
        StackPointers<Item> Childs;
    };

    Item* asItem(const QModelIndex& index) const;
    void forEachItem(QString& path,
                     const Item* root,
                     const std::function<void (const QString& path, const Item*)>& handle) const;
    void reset();
    void reset(const QHash<Name, Property*>& tree);

private:
    ScopedPointer<Item> m_root;
};

#endif // PROPERTIESMODEL_H
