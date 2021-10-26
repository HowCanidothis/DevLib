#include "modelsvacabulary.h"

#include <QCompleter>

#include "modelsfiltermodelbase.h"
#include "modelslistbase.h"

ModelsVacabulary::ModelsVacabulary(const HeaderData& dictionary)
    : m_header(dictionary)
{}

const QVariant& ModelsVacabulary::SelectValue(const Name& name, const QHash<Name, QVariant>& row)
{
    static QVariant result;
    auto foundIt = row.find(name);
    if(foundIt != row.end()) {
        return foundIt.value();
    }
    return result;
}

const std::pair<Name, TranslatedStringPtr>& ModelsVacabulary::GetHeader(qint32 column) const
{
    static std::pair<Name, TranslatedStringPtr> result = std::make_pair(Name(), ::make_shared<TranslatedString>([]{ return QString(); }));
    if(column < 0 || column >= m_header.size()) {
        return result;
    }
    return m_header.at(column);
}

TModelsListBase<ModelsVacabulary>* ModelsVacabulary::CreateListModel(qint32 column, QObject* parent)
{
    return new TModelsListBase<ModelsVacabulary>(parent, [column](const SharedPointer<ModelsVacabulary>& ptr, const QModelIndex& index, qint32 role){
        if(role == Qt::DisplayRole) {
            return ptr->SelectValue(ptr->GetHeader(column).first, ptr->At(index.row()));
        } else if(role == Qt::EditRole) {
            return QVariant(index.row());
        }
        return QVariant();
    }, [](const SharedPointer<ModelsVacabulary>& ptr){
        return ptr->GetSize();
    });
}

ModelsVacabularyViewModel::ModelsVacabularyViewModel()
{
    auto displayEditRoleHandlers = [this](qint32 row, qint32 column) -> QVariant {
        return GetData()->At(row)[GetData()->GetHeader(column).first];
    };

    m_roleDataHandlers.insert(Qt::EditRole, displayEditRoleHandlers);
    m_roleDataHandlers.insert(Qt::DisplayRole, displayEditRoleHandlers);

    m_roleHorizontalHeaderDataHandlers.insert(Qt::DisplayRole, [this](qint32 section){
        return GetData()->GetHeader(section).second->Native();
    });

    m_roleSetDataHandlers.insert(Qt::EditRole, [this](qint32 row, qint32 column, const QVariant& value) -> bool {
        return GetData()->EditWithCheck(row, [&](QHash<Name, QVariant>& row) -> FAction {
            const auto& key = GetData()->GetHeader(column).first;
            if(row[key] == value) {
                return nullptr;
            }
            return [&]{ row.insert(key, value); };
        }, { column });
});
}

bool ModelsVacabularyViewModel::setData(const QModelIndex& index, const QVariant& value, qint32 role)
{
    if(!index.isValid()) {
        return false;
    }

    if(isLastEditRow(index)) {
        if(role == Qt::EditRole) {
            QHash<Name, QVariant> newRow;
            newRow.insert(GetData()->GetHeader(index.column()).first, value);
            GetData()->Append(newRow);
            return true;
        }
        return false;
    }

    return Super::setData(index, value, role);
}

QVariant ModelsVacabularyViewModel::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    if(isLastEditRow(index)) {
        return QVariant();
    }

    return Super::data(index, role);
}

qint32 ModelsVacabularyViewModel::rowCount(const QModelIndex&) const
{
    return GetData() == nullptr ? 0 : (GetData()->GetSize() + 1);
}

qint32 ModelsVacabularyViewModel::columnCount(const QModelIndex&) const
{
    return GetData() != nullptr ? GetData()->GetColumnsCount() : 0;
}

Qt::ItemFlags ModelsVacabularyViewModel::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

ModelsVacabularyManager::ModelsVacabularyManager()
{}

void ModelsVacabularyManager::RegisterModel(const Name& modelName, const ModelsVacabularyPtr& vacabulary)
{
    Q_ASSERT(!m_models.contains(modelName));
    m_models.insert(modelName, vacabulary);
}

ModelsVacabularyManager& ModelsVacabularyManager::GetInstance()
{
    static ModelsVacabularyManager result;
    return result;
}

const ModelsVacabularyManager::ViewModelDataPtr& ModelsVacabularyManager::CreateViewModel(const Name& modelName, qint32 columnIndex)
{
    Q_ASSERT(m_models.contains(modelName));
    auto data = ::make_shared<ViewModelData>();
    auto* sortModel = new ModelsFilterModelBase(nullptr);
    sortModel->LessThan = [sortModel](const QModelIndex& f, const QModelIndex& s) {
        if(sortModel->IsLastEditRow(f)) {
            return sortModel->sortOrder() == Qt::AscendingOrder ? false : true;
        }
        if(sortModel->IsLastEditRow(s)) {
            return sortModel->sortOrder() == Qt::AscendingOrder ? true : false;
        }
        return sortModel->DefaultLessThan(f, s);
    };
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    data->SortedModel = sortModel;
    auto* listModel = m_models[modelName]->CreateListModel(columnIndex, nullptr);
    listModel->SetData(m_models[modelName]);
    data->SourceModel = listModel;
    data->SortedModel->setSourceModel(data->SourceModel);
    listModel->GetData()->OnChanged += { this, [sortModel]{
        sortModel->sort(0);
    }};

    return m_cache[modelName].insert(columnIndex, data).value();
}

const ModelsVacabularyManager::ViewModelDataPtr& ModelsVacabularyManager::GetViewModel(const Name& modelName, qint32 column)
{
    return m_cache[modelName][column];
}

QCompleter* ModelsVacabularyManager::CreateCompleter(const Name& modelName, qint32 column, QObject* parent, CommonDispatcher<qint32>* dispatcher)
{
    auto* completer = new QCompleter(parent);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionColumn(0);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setModel(m_cache[modelName][column]->SortedModel);
    completer->connect(completer, static_cast<void (QCompleter::*)(const QModelIndex&)>(&QCompleter::activated), [completer, this, modelName, column, dispatcher](const QModelIndex& index){
        dispatcher->Invoke(index.data(Qt::EditRole).toInt());
    });
    return completer;
}
