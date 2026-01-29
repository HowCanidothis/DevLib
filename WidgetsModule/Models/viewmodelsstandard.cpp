#include "viewmodelsstandard.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/measurementunitmanager.h>
#endif

void ModelsStandardRow::Set(qint32 i, const QVariant& data, qint32 role)
{
    if(!IsValid(i)) {
        resize(i + 1);
    }
    operator[](i)[role] = data;
}

QVariant ModelsStandardRow::Get(qint32 i, qint32 role) const
{
    return IsValid(i) ? at(i).value(role, QVariant()) : QVariant();
}

bool ModelsStandardRow::IsValid(qint32 i) const
{
    return i >= 0 && i < size();
}

ViewModelsStandardComponentsBuilder& ViewModelsStandardComponentsBuilder::AddColumn(qint32 i, const FTranslationHandler& title)
{
    Super::AddColumn(i, title, [i](const ModelsStandardRow& v) {
        return v.Get(i, Qt::DisplayRole);
    }, [i](const QVariant& data, ModelsStandardRow& v) {
        return [&]{ v.Set(i, data); };
    });
    return *this;
}

ViewModelsStandardComponentsBuilder& ViewModelsStandardComponentsBuilder::SetColumn(qint32 i, const FTranslationHandler& title)
{
    m_viewModel->ColumnComponents.ChangeComponent(Qt::DisplayRole, i).SetHeader([title]{
        return title();
    });
    return *this;
}

ViewModelsStandardComponentsBuilder& ViewModelsStandardComponentsBuilder::SetColumnsCount(qint32 count)
{
    auto currentCount = m_viewModel->ColumnComponents.GetColumnCount();
    if(count == currentCount) {
        return *this;
    }

    if(count > currentCount) {
        m_viewModel->beginInsertColumns(QModelIndex(), currentCount, count - 1);
        for(qint32 i(currentCount); i < count; ++i) {
            AddColumn(i, TR_NONE);
        }
        m_viewModel->endInsertColumns();
    } else {
        m_viewModel->beginRemoveColumns(QModelIndex(), count, currentCount - 1);
        for(qint32 i(count); i < currentCount; ++i) {
            m_viewModel->ColumnComponents.RemoveComponent(i);
        }
        m_viewModel->endInsertColumns();
    }

    return *this;
}

ViewModelsStandard::ViewModelsStandard(QObject* parent)
    : Super(parent)
{
}

ViewModelsStandardComponentsBuilder ViewModelsStandard::Builder()
{
    return ViewModelsStandardComponentsBuilder(this);
}

bool ViewModelsStandard::setData(const QModelIndex& index, const QVariant& v, qint32 role)
{
    if(role == Qt::CheckStateRole) {
        if(GetData() == nullptr || !GetData()->HasIndex(index.row())) {
            return false;
        }
        GetData()->Edit(index.row(), [&](ModelsStandardRow& row) {
            row.Set(index.column(), v, role);
        }, {index.column()});
        return true;
    }
    return Super::setData(index, v, role);
}

QVariant ViewModelsStandard::data(const QModelIndex& index, int role) const
{
    if(GetData() == nullptr) {
        return QVariant();
    }

    switch(role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
    case Qt::CheckStateRole:
    case Qt::DecorationRole:
    case Qt::SizeHintRole:
        if(GetData()->HasIndex(index.row())) {
            const auto& row = GetData()->At(index.row());
            if(index.column() >= 0 && index.column() < row.size()) {
                return row.at(index.column()).value(role);
            }
            return QVariant();
        }
        break;
    default: break;
    }

    return QVariant();
}
