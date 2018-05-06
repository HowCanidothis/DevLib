#include "propertiesmodel.h"

#include "propertiessystem.h"
#include "property.h"
#include <QSettings>

PropertiesModel::PropertiesModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    reset();
}

void PropertiesModel::update()
{
    layoutAboutToBeChanged();

    const auto& tree = PropertiesSystem::context();

    reset(tree);

    layoutChanged();
}

void PropertiesModel::forEachItem(QString& path,
                                  const Item* root,
                                  const std::function<void (const QString& path, const Item*)>& handle) const
{
    for(const Item* item : root->childs) {
        if(item->property == nullptr) {
            QString path_current = path + item->name + "/";
            forEachItem(path_current, item, handle);
        } else {
            handle(path, item);
        }
    }
}

void PropertiesModel::reset()
{
    root = new Item { "", nullptr, 0, nullptr };
}

void PropertiesModel::reset(const QHash<Name, Property*>& tree)
{
    reset();

    QHash<QString, Item*> nodes;

    qint32 row=0;

    for(auto it(tree.begin()), e(tree.end()); it != e; it++, row++) {
        const Name& path = it.key();
        Item* current = root.data();
        QStringList paths = path.asString().split('/', QString::SkipEmptyParts);
        for(const QString& path : adapters::range(paths.begin(), paths.end() - 1)) {
            auto find = nodes.find(path);

            if(find == nodes.end()) {
                Item* parent = current;
                qint32 crow = current->childs.size();
                current = new Item { path, parent, crow };
                parent->childs.push(current);
                nodes.insert(path, current);
            } else {
                current = find.value();
            }
        }

        //Last editable item
        qint32 crow = current->childs.size();
        Item* property_item = new Item { paths.last(), current, crow };
        current->childs.push(property_item);
        property_item->property = it.value();
    }
}

void PropertiesModel::save(const QString& file_name) const
{
    Q_ASSERT(!file_name.isEmpty());
    QSettings settings(file_name, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    QString path;
    forEachItem(path, root.data(), [&settings](const QString& path, const Item* item) {
        settings.setValue(path + item->name, item->property->getValue());
    });
}

void PropertiesModel::load(const QString& file_name)
{
    Q_ASSERT(!file_name.isEmpty());
    LOGOUT;
    QSettings settings(file_name, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    beginResetModel();

    const auto& tree = PropertiesSystem::context();

    for(const QString& key : settings.allKeys()) {
        auto find = tree.find(Name(key));
        if(find == tree.end()) {
            log.warning() << "unknown property" << key;
        } else {
            find.value()->setValue(settings.value(key));
        }
    }

    endResetModel();
}

int PropertiesModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return asItem(parent)->childs.size();
    }
    return root->childs.size();
}

QVariant PropertiesModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        auto item = asItem(index);
        if(item->property && index.column()) {
            return item->property->getValue();
        }
        if(!index.column())
            return item->name;
    }
    case RoleHeaderItem:
        return !asItem(index)->property;
    case RoleMinValue: {
        auto property = asItem(index)->property;
        Q_ASSERT(property);
        return property->getMin();
    }
    case RoleMaxValue: {
        auto property = asItem(index)->property;
        Q_ASSERT(property);
        return property->getMax();
    }
    case RoleIsTextFileName: {
        if(index.column()) {
            auto property = asItem(index)->property;
            if(property) {
                return property->isTextFileName();
            }
        }
        return false;
    }
    default:
        break;
    }
    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    switch (role) {
    case Qt::EditRole: {
        Item* item = asItem(index);
        if(auto prop = item->property) {
            prop->setValue(value.toString());
        }
        return true;
    }
    default:
        break;
    }
    return false;
}

QVariant PropertiesModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

QModelIndex PropertiesModel::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(parent.isValid()) {
        Item* item = asItem(parent)->childs.at(row);
        return createIndex(row, column, item);
    }
    Item* item = root->childs.at(row);
    return createIndex(row, column, item);
}

QModelIndex PropertiesModel::parent(const QModelIndex& child) const
{
    Item* node = asItem(child);
    if(node->parent == root.data())
        return QModelIndex();
    return createIndex(node->parent_row, 0, node->parent);
}

int PropertiesModel::columnCount(const QModelIndex&) const
{
    return 2;
}

PropertiesModel::Item* PropertiesModel::asItem(const QModelIndex& index) const
{
    return (Item*)index.internalPointer();
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex& index) const
{
    if(index.column()) {
        if(auto property = asItem(index)->property) {
            if(!property->isReadOnly())
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
