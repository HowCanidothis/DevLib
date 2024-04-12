#ifndef MODELSLISTBASE_H
#define MODELSLISTBASE_H

#include <QAbstractListModel>

#include "modelstablebase.h"
#include "WidgetsModule/widgetsdeclarations.h"

template<class T>
class TViewModelsListBase : public TViewModelsTableBase<T>
{
    using Super = TViewModelsTableBase<T>;
    using FValueExtractor = std::function<QVariant (const SharedPointer<T>&, const QModelIndex&, int)>;
    using FCountExtractor = std::function<qint32 (const SharedPointer<T>&)>;
public:
    TViewModelsListBase(QObject* parent, const FValueExtractor& extractor, const FCountExtractor& countExtractor)
        : Super(parent)
        , m_extractor(extractor)
        , m_countExtractor(countExtractor)
    {}
    TViewModelsListBase(QObject* parent)
        : Super(parent)
        , m_extractor([](const SharedPointer<T>&, const QModelIndex&, int){ return QVariant(); })
        , m_countExtractor([](const SharedPointer<T>&){ return 0; })
    {}

    // QAbstractItemModel interface☺
public:
    int rowCount(const QModelIndex& index = QModelIndex()) const override
    {
        if(index.isValid()) {
            return 0;
        }
        if(this->GetData() == nullptr) {
            return 0;
        }
        return m_countExtractor(this->GetData());
    }

    int columnCount(const QModelIndex& index = QModelIndex()) const override
    {
        if(index.isValid()) {
            return 0;
        }
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid() || this->GetData() == nullptr) {
            return QVariant();
        }

        return m_extractor(this->GetData(), index, role);
    }

private:
    FValueExtractor m_extractor;
    FCountExtractor m_countExtractor;
};

class ModelsStandardListModel : public TModelsTableWrapper<ModelsStandardListModelContainer>
{
    using Super = TModelsTableWrapper<ModelsStandardListModelContainer>;
public:
    using container_type = ModelsStandardListModelContainer;

    template<class ... Args>
    DispatcherConnection AddTrigger(CommonDispatcher<Args...>* dispatcher, const std::function<void (container_type&)>& handler, const std::function<bool ()>& apply = []{ return true; })
    {
        auto applyHandler = [apply, handler, this]{
            if(apply()) {
                Change(handler);
            }
        };
        auto result = dispatcher->Connect(CONNECTION_DEBUG_LOCATION, applyHandler);
        applyHandler();
        return result;
    }

    template<class Enum>
    void SetEnum(const std::function<void (ModelsStandardListModelContainer&)>& handler = [](ModelsStandardListModelContainer&){})
    {
        m_enumConnections.clear();
        AddTrigger(&TranslatorManager::GetInstance().OnLanguageChanged, [handler, this](container_type& native){
            setEnum<Enum>(native, handler);
        }).MakeSafe(m_enumConnections);
    }

    template<class Enum, typename ... Dispatchers>
    void SetEnum(const std::function<void (ModelsStandardListModelContainer&)>& handler = [](ModelsStandardListModelContainer&){}, Dispatchers&... dispatchers)
    {
        m_enumConnections.clear();
        auto commutator = ::make_shared<DispatchersCommutator>();
        commutator->ConnectFrom(CONNECTION_DEBUG_LOCATION, TranslatorManager::GetInstance().OnLanguageChanged, dispatchers...).MakeSafe(m_enumConnections);
        AddTrigger(commutator.get(), [this, handler, commutator](container_type& native){
            setEnum<Enum>(native, handler);
        });
    }

    template<class Enum>
    void SetEnumCategorized(const ViewModelsCategoriesContainer<Enum>& categories)
    {
        m_enumConnections.clear();
        AddTrigger(&TranslatorManager::GetInstance().OnLanguageChanged, [categories](container_type& native){
            native.clear();
            const auto& names = TranslatorManager::GetInstance().GetEnumNames<Enum>();
            QFont bold;
            bold.setBold(true);
            for(const auto& category : categories) {
                if(category.first != nullptr) {
                    ModelsStandardListModel::value_type categoryData;
                    categoryData.insert(Qt::DisplayRole, category.first());
                    categoryData.insert(Qt::FontRole, bold);
                    categoryData.insert(Qt::BackgroundRole, SharedSettings::GetInstance().StyleSettings.DisabledTableCellColor.Native());
                    categoryData.insert(Qt::ForegroundRole, SharedSettings::GetInstance().StyleSettings.DisabledTableCellTextColor.Native());
                    categoryData.insert(IdRole, -1);
                    native.append(categoryData);
                }

                for(const auto& index : category.second) {
                    ModelsStandardListModel::value_type data;
                    const auto& label = names.at((qint32)index);
                    data.insert(Qt::DisplayRole, label);
                    data.insert(Qt::EditRole, label);
                    data.insert(IdRole, (qint32)index);
                    native.append(data);
                }
                native.append(ModelsStandardListModel::value_type());
            }
        }).MakeSafe(m_enumConnections);
    }

private:
    void fillContainerWithEnum(container_type& container, const QStringList& fillWith, qint32 startsWith, qint32 endsWith);
    template<class Enum>
    void setEnum(container_type& container, const std::function<void (ModelsStandardListModelContainer&)>& handler)
    {
        container.clear();
        auto names = TranslatorManager::GetNames<Enum>();
        fillContainerWithEnum(container, names, (qint32)Enum::First, (qint32)Enum::Last);
        handler(container);
    }

private:
    DispatcherConnectionsSafe m_enumConnections;
};
using ModelsStandardListModelPtr = SharedPointer<ModelsStandardListModel>;

class ViewModelsStandardListModel : public TViewModelsTableBase<ModelsStandardListModel>
{
    Q_OBJECT
    using Super = TViewModelsTableBase<ModelsStandardListModel>;
public:
    using Super::Super;

    QVariant data(const QModelIndex& index, int role) const override;

    int columnCount(const QModelIndex& index = QModelIndex()) const override;

    template<class Enum, typename ... Dispatchers>
    static ViewModelsStandardListModel* CreateEnumViewModel(QObject* parent, const std::function<void (ModelsStandardListModelContainer&)>& extraFieldsHandler = [](ModelsStandardListModelContainer&){}, Dispatchers&... dispatchers)
    {
        auto* result = new ViewModelsStandardListModel(parent);
        auto model = ::make_shared<ModelsStandardListModel>();
        model->SetEnum<Enum>(extraFieldsHandler, dispatchers...);
        result->SetData(model);
        return result;
    }

    template<class Enum>
    static ViewModelsStandardListModel* CreateCategorizedEnumViewModel(QObject* parent, const ViewModelsCategoriesContainer<Enum>& categories)
    {
        auto* result = new ViewModelsStandardListModel(parent);
        ViewModelsTableColumnComponents::ColumnFlagsComponentData flagsComponent;
        flagsComponent.GetFlagsHandler = [result](qint32 row) -> std::optional<Qt::ItemFlags> {
            if(result->GetData()->At(row).value(IdRole, -1) == -1) {
                return Qt::ItemIsEnabled;
            }
            return std::nullopt;
        };
        result->ColumnComponents.AddFlagsComponent(0, flagsComponent);
        auto model = ::make_shared<ModelsStandardListModel>();
        model->SetEnumCategorized<Enum>(categories);
        result->SetData(model);
        return result;
    }
};

#endif // MODELSLISTBASE_H
