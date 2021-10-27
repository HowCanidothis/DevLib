#ifndef MODELSVACABULARY_H
#define MODELSVACABULARY_H

#include <PropertiesModule/internal.hpp>

#include "modelstablebase.h"

template<class T>
class TModelsListBase;

using ModelsVacabularyRequest = CommonDispatcher<qint32>;

class ModelsVacabulary : public TModelsTableWrapper<QVector<QHash<Name, QVariant>>>
{
    using Super = TModelsTableWrapper<QVector<QHash<Name, QVariant>>>;
public:
    using HeaderDataValue = std::pair<Name, TranslatedStringPtr>;
    using HeaderData = QVector<HeaderDataValue>;

    ModelsVacabulary(const HeaderData& dictionary);

    template<class Property>
    DispatcherConnection CreatePropertyConnection(LocalPropertyOptional<Property>* property, const Name& name, ModelsVacabularyRequest* indexDispatcher)
    {
        return indexDispatcher->Connect(this, [this, name, property](qint32 index){
            if(!IsValidRow(index)) {
                return;
            }
            property->FromVariant(SelectValue(name, At(index)));
        });
    }

    static const QVariant& SelectValue(const Name& name, const QHash<Name, QVariant>& row);

    qint32 GetColumnsCount() const { return m_header.size(); }
    const std::pair<Name, TranslatedStringPtr>& GetHeader(qint32 column) const;

    static TModelsListBase<ModelsVacabulary>* CreateListModel(qint32 column, QObject* parent);

private:
    QVector<std::pair<Name, TranslatedStringPtr>> m_header;
};

using ModelsVacabularyPtr = SharedPointer<ModelsVacabulary>;

class ModelsVacabularyViewModel : public TModelsTableBase<ModelsVacabulary>
{
    using Super = TModelsTableBase<ModelsVacabulary>;
public:
    ModelsVacabularyViewModel(QObject* parent = nullptr);

    bool setData(const QModelIndex& index, const QVariant& value, qint32 role) override;
    QVariant data(const QModelIndex& index, qint32 role) const override;
    qint32 rowCount(const QModelIndex&) const override;
    qint32 columnCount(const QModelIndex&) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};

class ModelsVacabularyManager
{
    ModelsVacabularyManager();
public:
    struct ViewModelData
    {
        class ModelsFilterModelBase* SortedModel;
        QAbstractItemModel* SourceModel;
        DelayedCallObject Sorter;

        ViewModelData()
            : SortedModel(nullptr)
            , SourceModel(nullptr)
            , Sorter(1000)
        {}
    };
    using ViewModelDataPtr = SharedPointer<ViewModelData>;

    static ModelsVacabularyManager& GetInstance();

    void RegisterModel(const Name& modelName, const ModelsVacabularyPtr& vacabulary);
    const ModelsVacabularyPtr& GetModel(const Name& modelName);
    const ViewModelDataPtr& CreateViewModel(const Name& modelName, qint32 columnIndex);
    const ViewModelDataPtr& GetViewModel(const Name& modelName, qint32 column);
    class QCompleter* CreateCompleter(const Name& modelName, qint32 column, QObject* parent, ModelsVacabularyRequest* dispatcher);

private:
    QHash<Name, ModelsVacabularyPtr> m_models;
    QHash<Name, QHash<qint32, ViewModelDataPtr>> m_cache;
};

#endif // MODELSVACABULARY_H
