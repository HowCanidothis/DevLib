#include "modelsvacabulary.h"

#include <QCompleter>

#include "modelsfiltermodelbase.h"
#include "modelslistbase.h"

ModelsVocabulary::ModelsVocabulary(const HeaderData& dictionary)
    : m_header(dictionary)
{}

const QVariant& ModelsVocabulary::SelectValue(const Name& name, const QHash<Name, QVariant>& row)
{
    static QVariant result;
    auto foundIt = row.find(name);
    if(foundIt != row.end()) {
        return foundIt.value();
    }
    return result;
}

const std::pair<Name, TranslatedStringPtr>& ModelsVocabulary::GetHeader(qint32 column) const
{
    static std::pair<Name, TranslatedStringPtr> result = std::make_pair(Name(), ::make_shared<TranslatedString>([]{ return QString(); }));
    if(column < 0 || column >= m_header.size()) {
        return result;
    }
    return m_header.at(column);
}

TModelsListBase<ModelsVocabulary>* ModelsVocabulary::CreateListModel(qint32 column, QObject* parent)
{
    return new TModelsListBase<ModelsVocabulary>(parent, [column](const SharedPointer<ModelsVocabulary>& ptr, const QModelIndex& index, qint32 role){
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            return ptr->SelectValue(ptr->GetHeader(column).first, ptr->At(index.row()));
        } else if(role == Qt::UserRole) {
            return QVariant(index.row());
        }
        return QVariant();
    }, [](const SharedPointer<ModelsVocabulary>& ptr){
        return ptr->GetSize();
    });
}

ModelsVocabularyViewModel::ModelsVocabularyViewModel(QObject* parent)
    : Super(parent)
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

bool ModelsVocabularyViewModel::setData(const QModelIndex& index, const QVariant& value, qint32 role)
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

QVariant ModelsVocabularyViewModel::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    if(isLastEditRow(index)) {
        return QVariant();
    }

    return Super::data(index, role);
}

qint32 ModelsVocabularyViewModel::rowCount(const QModelIndex&) const
{
    return GetData() == nullptr ? 0 : (GetData()->GetSize() + 1);
}

qint32 ModelsVocabularyViewModel::columnCount(const QModelIndex&) const
{
    return GetData() != nullptr ? GetData()->GetColumnsCount() : 0;
}

Qt::ItemFlags ModelsVocabularyViewModel::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

ModelsVocabularyManager::ModelsVocabularyManager()
{}

void ModelsVocabularyManager::RegisterModel(const Name& modelName, const ModelsVocabularyPtr& vacabulary)
{
    Q_ASSERT(!m_models.contains(modelName));
    m_models.insert(modelName, vacabulary);
}

ModelsVocabularyManager& ModelsVocabularyManager::GetInstance()
{
    static ModelsVocabularyManager result;
    return result;
}

const ModelsVocabularyPtr& ModelsVocabularyManager::GetModel(const Name& modelName)
{
    Q_ASSERT(m_models.contains(modelName));
    return m_models[modelName];
}

const ModelsVocabularyManager::ViewModelDataPtr& ModelsVocabularyManager::CreateViewModel(const Name& modelName, qint32 columnIndex)
{
    Q_ASSERT(m_models.contains(modelName));
    auto data = ::make_shared<ViewModelData>();
    auto* sortModel = new ModelsFilterModelBase(nullptr);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    data->SortedModel = sortModel;
    const auto& model = m_models[modelName];

    if(columnIndex == -1) {
        sortModel->LessThan = [sortModel](const QModelIndex& f, const QModelIndex& s) {
            if(sortModel->IsLastEditRow(f)) {
                return sortModel->sortOrder() == Qt::AscendingOrder ? false : true;
            }
            if(sortModel->IsLastEditRow(s)) {
                return sortModel->sortOrder() == Qt::AscendingOrder ? true : false;
            }
            return sortModel->DefaultLessThan(f, s);
        };

        auto* sourceModel = new ModelsVocabularyViewModel(nullptr);
        sourceModel->SetData(model);
        data->SourceModel = sourceModel;
    } else {

        auto* listModel = model->CreateListModel(columnIndex, nullptr);
        listModel->SetData(model);
        data->SourceModel = listModel;
        auto* pData = data.get();
        listModel->GetData()->OnChanged += { this, [sortModel, pData]{
            pData->Sorter.Call([sortModel]{
                sortModel->sort(0);
            });
        }};
    }
    data->SortedModel->setSourceModel(data->SourceModel);


    return m_cache[modelName].insert(columnIndex, data).value();
}

const ModelsVocabularyManager::ViewModelDataPtr& ModelsVocabularyManager::GetViewModel(const Name& modelName, qint32 column)
{
    return m_cache[modelName][column];
}

QCompleter* ModelsVocabularyManager::CreateCompleter(const Name& modelName, qint32 column, QObject* parent, ModelsVocabularyRequest* dispatcher)
{
    auto* completer = new QCompleter(parent);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionColumn(0);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setModel(m_cache[modelName][column]->SortedModel);
    completer->connect(completer, static_cast<void (QCompleter::*)(const QModelIndex&)>(&QCompleter::activated), [modelName, dispatcher](const QModelIndex& index){
        dispatcher->Invoke(index.data(Qt::UserRole).toInt());
    });
    return completer;
}
