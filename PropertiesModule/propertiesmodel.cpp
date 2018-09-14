#include "propertiesmodel.h"

#include "propertiessystem.h"
#include "property.h"

PropertiesModel::PropertiesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _contextIndex(PropertiesSystem::Global)
{
    reset();
}

PropertiesModel::PropertiesModel(qint32 contextIndex, QObject* parent)
    : QAbstractItemModel(parent)
    , _contextIndex(contextIndex)
{
    reset();
}

void PropertiesModel::Change(const std::function<void ()>& handle)
{
    beginResetModel();

    handle();

    const auto& tree = PropertiesSystem::context(_contextIndex);

    reset(tree);

    endResetModel();
}

void PropertiesModel::SetContextIndex(qint32 contextIndex)
{
    if(_contextIndex != contextIndex) {
        _contextIndex = contextIndex;

        update();

        emit contextIndexChanged();
    }
}

qint32 PropertiesModel::GetContextIndex() const
{
    return _contextIndex;
}

void PropertiesModel::SetFileName(const QString& fileName)
{
    if(_fileName != fileName) {
        _fileName = fileName;

        emit fileNameChanged();
    }
}

const QString& PropertiesModel::GetFileName() const
{
    return _fileName;
}

void PropertiesModel::update()
{
    beginResetModel();

    const auto& tree = PropertiesSystem::context(_contextIndex);

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
    _root = new Item { "", nullptr, 0, nullptr };
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
        Item* current = _root.data();
        QStringList paths = path.AsString().split('/', QString::SkipEmptyParts);
        for(const QString& path : adapters::range(paths.begin(), paths.end() - 1)) {
            auto find = nodes.find(path);

            if(find == nodes.end()) {
                Item* parent = current;
                qint32 crow = current->Childs.Size();
                current = new Item { path, parent, crow };
                parent->Childs.Push(current);
                nodes.insert(path, current);
            } else {
                current = find.value();
            }
        }

        //Last editable item
        qint32 crow = current->Childs.Size();
        Item* propertyItem = new Item { paths.last(), current, crow };
        current->Childs.Push(propertyItem);
        propertyItem->Prop = it.value();
    }
}

void PropertiesModel::Save(const QString& fileName) const
{
    PropertiesSystem::Save(fileName, _contextIndex);
}

void PropertiesModel::Load(const QString& fileName)
{    
    PropertiesSystem::Load(fileName, _contextIndex);
}

int PropertiesModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return asItem(parent)->Childs.Size();
    }
    return _root->Childs.Size();
}

QVariant PropertiesModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        auto item = asItem(index);
        if(item->Prop && index.column()) {
            return item->Prop->getDisplayValue();
        }
        if(!index.column()) {
            return item->Name;
        }
        break;
    }
    case Qt::EditRole: {
        auto item = asItem(index);
        if(item->Prop && index.column()) {
            return item->Prop->getValue();
        }
        if(!index.column()) {
            return item->Name;
        }
        break;
    }
    case RoleHeaderItem:
        return !asItem(index)->Prop;
    case RoleMinValue: {
        auto property = asItem(index)->Prop;
        Q_ASSERT(property);
        return property->GetMin();
    }
    case RoleMaxValue: {
        auto property = asItem(index)->Prop;
        Q_ASSERT(property);
        return property->GetMax();
    }
    case RoleDelegateValue: {
        if(index.column()) {
            auto property = asItem(index)->Prop;
            if(property) {
                return property->GetDelegateValue();
            }
        }
        return QVariant();
    }
    case RoleDelegateData: {
        if(index.column()) {
            auto property = asItem(index)->Prop;
            if(property) {
                auto delegateData = property->GetDelegateData();
                return (delegateData != nullptr) ? *delegateData : QVariant();
            }
        }
        return QVariant();
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
        if(auto prop = item->Prop) {
            prop->SetValue(value.toString());
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
    Item* item = _root->Childs.At(row);
    return createIndex(row, column, item);
}

QModelIndex PropertiesModel::parent(const QModelIndex& child) const
{
    Item* node = asItem(child);
    if(node->Parent == _root.data()) {
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
    result[RoleHeaderItem] = "headerItem";
    result[RoleMinValue] = "minValue";
    result[RoleMaxValue] = "maxValue";
    result[RoleDelegateValue] = "delegateValue";
    result[RoleDelegateData] = "delegateData";
    result[Qt::DisplayRole] = "text";
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
