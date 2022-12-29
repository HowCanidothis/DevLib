#ifndef MODELSVOCABULARYMANAGER_H
#define MODELSVOCABULARYMANAGER_H

#include <PropertiesModule/internal.hpp>

class ModelsVocabularyManager
{
    ModelsVocabularyManager();
public:
    struct ViewModelData
    {
        class ViewModelsFilterModelBase* SortedModel;
        class QAbstractItemModel* SourceModel;
        DelayedCallObject Sorter;

        ViewModelData()
            : SortedModel(nullptr)
            , SourceModel(nullptr)
            , Sorter(1000)
        {}
    };
    using ViewModelDataPtr = SharedPointer<ViewModelData>;

    static ModelsVocabularyManager& GetInstance();

    void RegisterModel(const Name& modelName, const ModelsVocabularyPtr& vacabulary);
    const ModelsVocabularyPtr& GetModel(const Name& modelName);
    const ViewModelDataPtr& CreateViewModel(const Name& modelName, qint32 columnIndex);
    const ViewModelDataPtr& GetViewModel(const Name& modelName, qint32 column);

    LocalPropertyLocale Locale;

private:
    QHash<Name, ModelsVocabularyPtr> m_models;
    QHash<Name, QHash<qint32, ViewModelDataPtr>> m_cache;
};

#endif // MODELSVOCABULARYMANAGER_H
