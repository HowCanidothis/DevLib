#include "propertiesmodel.h"

#include "propertiessystem.h"
#include "property.h"
#include "propertiesscope.h"

PropertiesModel::PropertiesModel(QObject* parent)
    : PropertiesModel(PropertiesSystem::Global, parent)
{
}

PropertiesModel::PropertiesModel(const PropertiesScopeName& scope, QObject* parent)
    : QAbstractItemModel(parent)
    , Scope(scope)
{
    reset();

    FileName.Subscribe([this]{
        emit fileNameChanged();
    });

    Scope.Subscribe([this]{
        update();
        emit contextIndexChanged();
    });
}

void PropertiesModel::Change(const std::function<void ()>& handle)
{
    beginResetModel();

    handle();

    const auto& tree = PropertiesSystem::getOrCreateScope(Scope)->m_properties;

    reset(tree);

    endResetModel();
}

void PropertiesModel::update()
{
    beginResetModel();

    const auto& tree = PropertiesSystem::getOrCreateScope(Scope)->m_properties;

    reset(tree);

    endResetModel();
}

void PropertiesModel::forEachItem(QString& path,
                                  const Item* root,
                                  const std::function<void (const QString& path, const Item*)>& handle) const
{
    for(const Item* item : root->Childs) {
        if(item->Prop == nullptr) {
            QString pathCurrent = path + item->Name + "/";
            forEachItem(pathCurrent, item, handle);
        } else {
            handle(path, item);
        }
    }
}

void PropertiesModel::reset()
{
    m_root = new Item { "", nullptr, 0, nullptr, {} };
}

void PropertiesModel::reset(const QHash<Name, Property*>& tree)
{
    reset();

    QHash<QString, Item*> nodes;

    qint32 row=0;

    for(auto it(tree.begin()), e(tree.end()); it != e; it++, row++) {
        if(!it.value()->GetOptions().TestFlag(Property::Option_IsPresentable)){
            continue;
        }
        const Name& path = it.key();
        Item* current = m_root.data();
        QStringList paths = path.AsString().split('/', QString::SkipEmptyParts);
        QString currentPath;
        for(const QString& path : adapters::range(paths.begin(), paths.end() - 1)) {
            currentPath += path;
            auto find = nodes.find(currentPath);

            if(find == nodes.end()) {
                Item* parent = current;
                qint32 crow = current->Childs.Size();
                current = new Item { path, parent, crow, nullptr, {} };
                parent->Childs.Push(current);
                nodes.insert(currentPath, current);
            } else {
                current = find.value();
            }
        }

        //Last editable item
        qint32 crow = current->Childs.Size();
        Item* propertyItem = new Item { paths.last(), current, crow, nullptr, {} };
        current->Childs.Push(propertyItem);
        propertyItem->Prop = it.value();
    }
}

void PropertiesModel::Save(const QString& fileName) const
{
    PropertiesSystem::Save(fileName, Scope);
}

void PropertiesModel::Load(const QString& fileName)
{    
    PropertiesSystem::Load(fileName, Scope);
}

int PropertiesModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return asItem(parent)->Childs.Size();
    }
    return m_root->Childs.Size();
}

QVariant PropertiesModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    auto* item = asItem(index);

    if(index.column() == 0) {
        switch (role) {
            case Qt::DisplayRole: return item->Name;
            case Property::RoleHeaderItem: return item->Prop == nullptr;
            case Property::RoleQmlValue: return item->Prop != nullptr ? item->Prop->GetValueFromRole(role) : QVariant();
            default: break;
        }
    } else if(item->Prop != nullptr) {
        return item->Prop->GetValueFromRole(role);
    } else {
        if(role == Property::RoleHeaderItem) {
            return true;
        }
    }

    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    switch (role) {
    case Property::RoleQmlValue:
    case Qt::EditRole: {
        Item* item = asItem(index);
        if(auto prop = item->Prop) {
            if(prop->SetValue(value)) {
                emit dataChanged(index, index, { Property::RoleQmlValue, Qt::DisplayRole });
            }
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
        Item* item = asItem(parent)->Childs.At(row);
        return createIndex(row, column, item);
    }
    Item* item = m_root->Childs.At(row);
    return createIndex(row, column, item);
}

QModelIndex PropertiesModel::parent(const QModelIndex& child) const
{
    Item* node = asItem(child);
    if(node->Parent == m_root.data()) {
        return QModelIndex();
    }
    return createIndex(node->ParentRow, 0, node->Parent);
}

int PropertiesModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QHash<int, QByteArray> PropertiesModel::roleNames() const
{
    QHash<int, QByteArray> result;
    result[Property::RoleHeaderItem] = "headerItem";
    result[Property::RoleMinValue] = "minValue";
    result[Property::RoleMaxValue] = "maxValue";
    result[Property::RoleDelegateValue] = "delegateValue";
    result[Property::RoleDelegateData] = "delegateData";
    result[Qt::DisplayRole] = "name";
    result[Property::RoleQmlValue] = "value";
    return result;
}

PropertiesModel::Item* PropertiesModel::asItem(const QModelIndex& index) const
{
    return (Item*)index.internalPointer();
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex& index) const
{
    if(index.column()) {
        if(auto property = asItem(index)->Prop) {
            if(!property->GetOptions().TestFlag(Property::Option_IsReadOnly)) {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            }
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
