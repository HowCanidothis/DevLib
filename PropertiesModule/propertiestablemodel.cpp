#include "propertiestablemodel.h"

#include "property.h"

PropertiesTableModel::PropertiesTableModel(QObject* parent)
    : Super(parent)
    , ContextIndex(PropertiesSystem::Empty)
{
    ContextIndex.Subscribe([this]{
        Q_ASSERT(m_initializationFunction != nullptr);
        Update();
    });
}

void PropertiesTableModel::Init(const PropertiesScopeName& scope, const FAction& initializationFunc, const FAction& onEveryChange)
{
    m_initializationFunction = initializationFunc;
    m_onEveryChange = onEveryChange;
    if(ContextIndex == scope) {
        Update();
    } else {
        ContextIndex = scope;
    }
}

void PropertiesTableModel::Update()
{
    beginResetModel();
    PropertiesSystem::Clear(ContextIndex);
    PropertiesSystem::Begin(ContextIndex);
    m_initializationFunction();
    m_headers.clear();
    m_rows.clear();

    QSet<Name> validHeaders;

    PropertiesSystem::ForeachProperty([this, &validHeaders](Property* property) {
        const auto& propertyName = property->GetPropertyName().AsString();
        auto splitted = propertyName.split("/");
        Q_ASSERT(splitted.size() == 2);
        auto baseName = Name(splitted.first());
        auto foundIt = m_rows.find(baseName);
        Name subName(splitted.last());
        if(foundIt != m_rows.end()) {
            foundIt.value().Properties.insert(subName, property);
        } else {
            Row newRow;
            newRow.Properties.insert(subName, property);
            m_rows.insert(baseName, newRow);
        }
        validHeaders.insert(subName);
    });

    for(const auto& name : validHeaders) {
        m_headers.append(name);
    }

    std::sort(m_headers.begin(), m_headers.end(), [](const Name& f, const Name& s){
        return f.AsString() < s.AsString();
    });

    if(m_onEveryChange != nullptr) {
        PropertiesSystem::Subscribe(m_onEveryChange);
    }
    PropertiesSystem::End();
    endResetModel();
}

int PropertiesTableModel::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid()) {
        return m_rows.size();
    }
    return 0;
}

int PropertiesTableModel::columnCount(const QModelIndex& parent) const
{
    if(!parent.isValid()) {
        return m_headers.size() + 1;
    }
    return 0;
}

QVariant PropertiesTableModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    if(index.column() == 0) {
        switch (role) {
            case Qt::DisplayRole: {
                return (m_rows.begin() + index.row()).key().AsString();
            }
        default: break;
        }
    } else {
        auto* property = asProperty(index);
        if(property != nullptr) {
            return property->GetValueFromRole(role);
        }
    }

    return QVariant();
}

Property* PropertiesTableModel::asProperty(const QModelIndex& index) const
{
    auto column = index.column() - 1;
    const auto& currentRow = (m_rows.begin() + index.row()).value();
    const auto& name = *(m_headers.begin() + column);
    auto foundIt = currentRow.Properties.find(name);
    if(foundIt != currentRow.Properties.end()) {
        return foundIt.value();
    }
    return nullptr;
}

Qt::ItemFlags PropertiesTableModel::flags(const QModelIndex& index) const
{
    if(index.column()) {
        if(auto property = asProperty(index)) {
            if(!property->GetOptions().TestFlag(Property::Option_IsReadOnly)) {
                auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
                if(property->GetValue().type() == QVariant::Bool) {
                    return Qt::ItemIsUserCheckable | flags;
                }
                return flags;
            }
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool PropertiesTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    switch (role) {
    case Qt::EditRole: {
        auto* property = asProperty(index);
        if(property != nullptr) {
            property->SetValue(value);
        }
        return true;
    }
    case Qt::CheckStateRole: {
        auto* property = asProperty(index);
        if(property != nullptr) {
            property->SetValue(value == Qt::Checked ? true : false);
        }
        return true;
    }
    default:
        break;
    }
    return false;
}

QVariant PropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == 0) {
                return tr("Name");
            }
            return (m_headers.begin() + section - 1)->AsString();
        }
    }

    return Super::headerData(section, orientation, role);
}
