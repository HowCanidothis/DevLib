#include "modelsvocabularymanager.h"
#include <UnitsModule/internal.hpp>
#include "WidgetsModule/Models/viewmodelvacabulary.h"
#include "WidgetsModule/Models/modelslistbase.h"
#include "WidgetsModule/Models/modelsfiltermodelbase.h"

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
    auto* sortModel = new ViewModelsFilterModelBase(nullptr);
    data->SortedModel = sortModel;
    const auto& model = m_models[modelName];

    if(columnIndex == -1) {
        auto* sourceModel = new ModelsVocabularyViewModel(nullptr);
        sortModel->LessThan = [sortModel, sourceModel](const QModelIndex& f, const QModelIndex& s) {
            if(sourceModel->IsLastRow(f)) {
                return sortModel->sortOrder() == Qt::AscendingOrder ? false : true;
            }
            if(sourceModel->IsLastRow(s)) {
                return sortModel->sortOrder() == Qt::AscendingOrder ? true : false;
            }
            return sortModel->DefaultLessThan(f, s);
        };


#ifdef UNITS_MODULE_LIB
        qint32 i(0);
        for(const auto& header : model->GetHeader()) {
            if(header.Measurement != nullptr) {
                auto measurement = header.Measurement();
                sourceModel->SetterDelegates.insert(i, [measurement](QVariant& value){
                    if(value.isValid()) {
                        value = measurement->FromUnitToBase(value.toDouble());
                    }
                });
                sourceModel->GetterDelegates.insert(i, [measurement](QVariant& value){
                    if(value.isValid()) {
                        value = measurement->FromBaseToUnit(value.toDouble());
                    }
                });
                sourceModel->GetterDisplayDelegates.insert(i, [measurement](QVariant& value){
                    if(value.isValid()) {
                        value = measurement->FromBaseToUnitUi(value.toDouble());
                    }
                });
            }
            ++i;
        }
#endif
        sourceModel->SetData(model);
        data->SourceModel = sourceModel;
    } else {
        auto* listModel = model->CreateListModel(columnIndex, nullptr);
        listModel->SetData(model);
        data->SourceModel = listModel;
        auto* pData = data.get();
        sortModel->setDynamicSortFilter(false);
        listModel->GetData()->OnChanged += { this, [sortModel, pData]{
            pData->Sorter.Call(CONNECTION_DEBUG_LOCATION, [sortModel]{
                sortModel->sort(0, sortModel->sortOrder());
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
