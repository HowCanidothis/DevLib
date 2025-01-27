#include "viewmodelsstandard.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/measurementunitmanager.h>
#endif

void ModelsStandardRow::Set(qint32 i, const QVariant& data)
{
    if(!IsValid(i)) {
        resize(i + 1);
    }
    operator[](i) = data;
}

QVariant ModelsStandardRow::Get(qint32 i) const
{
    return IsValid(i) ? at(i) : QVariant();
}

bool ModelsStandardRow::IsValid(qint32 i) const
{
    return i >= 0 && i < size();
}

void ViewModelsStandardComponentsBuilder::AddColumn(qint32 i, const FTranslationHandler& title)
{
    Super::AddColumn(i, title, [i](const ModelsStandardRow& v) {
        return v.Get(i);
    }, [i](const QVariant& data, ModelsStandardRow& v) {
        return [&]{ v.Set(i, data); };
    });
}

void ViewModelsStandardComponentsBuilder::SetColumn(qint32 i, const FTranslationHandler& title)
{
    m_viewModel->ColumnComponents.ChangeComponent(Qt::DisplayRole, i).SetHeader([title]{
        return title();
    });
}

void ViewModelsStandardComponentsBuilder::SetColumnsCount(qint32 count)
{
    auto currentCount = m_viewModel->ColumnComponents.GetColumnCount();
    if(count == currentCount) {
        return;
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
}

ViewModelsStandard::ViewModelsStandard(QObject* parent)
    : Super(parent)
{
}

ViewModelsStandardComponentsBuilder ViewModelsStandard::Builder()
{
    return ViewModelsStandardComponentsBuilder(this);
}

QVariant ViewModelsStandard::data(const QModelIndex& index, int role) const
{
    if(GetData() == nullptr || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    }

    if(GetData()->HasIndex(index.row())) {
        const auto& row = GetData()->At(index.row());
        if(index.column() >= 0 && index.column() < row.size()) {
            return row.at(index.column());
        }
        return QVariant();
    }
    return QVariant();
}
