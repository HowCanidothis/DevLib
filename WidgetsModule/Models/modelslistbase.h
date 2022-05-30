#ifndef MODELSLISTBASE_H
#define MODELSLISTBASE_H

#include <QAbstractListModel>

#include "modelstablebase.h"

template<class T>
class TViewModelsListBase : public TViewModelsTableBase<T>
{
    using Super = TViewModelsTableBase<T>;
    using ValueExtractor = std::function<QVariant (const SharedPointer<T>&, const QModelIndex&, int)>;
    using CountExtractor = std::function<qint32 (const SharedPointer<T>&)>;
public:
    TViewModelsListBase(QObject* parent, const ValueExtractor& extractor, const CountExtractor& countExtractor)
        : Super(parent)
        , m_extractor(extractor)
        , m_countExtractor(countExtractor)
    {}
    TViewModelsListBase(QObject* parent)
        : Super(parent)
        , m_extractor([](const SharedPointer<T>&, const QModelIndex&, int){ return QVariant(); })
        , m_countExtractor([](const SharedPointer<T>&){ return 0; })
    {}

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& index = QModelIndex()) const override
    {
        Q_UNUSED(index);
        if(GetData() == nullptr) {
            return 0;
        }
        return m_countExtractor(GetData());
    }

    int columnCount(const QModelIndex& index = QModelIndex()) const override
    {
        Q_UNUSED(index);
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid() || GetData() == nullptr) {
            return QVariant();
        }

        return m_extractor(GetData(), index, role);
    }

private:
    ValueExtractor m_extractor;
    CountExtractor m_countExtractor;
};

class ModelsStandardListModel : public TModelsTableWrapper<QVector<QHash<qint32, QVariant>>>
{
    using Super = TModelsTableWrapper<QVector<QHash<qint32, QVariant>>>;
public:
    using container_type = QVector<QHash<qint32, QVariant>>;

    template<class ... Args>
    DispatcherConnection AddTrigger(CommonDispatcher<Args...>* dispatcher, const std::function<void (container_type&)>& handler, const std::function<bool ()>& apply = []{ return true; })
    {
        auto applyHandler = [apply, handler, this]{
            if(apply()) {
                Change(handler);
            }
        };
        auto result = dispatcher->Connect(this, applyHandler);
        applyHandler();
        return result;
    }

    template<class Enum>
    void SetEnum(const std::function<void (qint32 i, container_type::value_type&)>& handler = [](container_type&){})
    {
        AddTrigger(&TranslatorManager::GetInstance().OnLanguageChanged, [handler](container_type& native){
            native.clear();
            auto names = TranslatorManager::GetNames<Enum>();
            for(qint32 i((qint32)Enum::First), e((qint32)Enum::Last); i <= e; i++) {
                ModelsStandardListModel::value_type data;
                data.insert(Qt::DisplayRole, names.at(i));
                data.insert(Qt::EditRole, i);
                handler(i, data);
                native.append(data);
            }
        });
    }
};
using ModelsStandardListModelPtr = SharedPointer<ModelsStandardListModel>;

class ViewModelsStandardListModel : public TViewModelsTableBase<ModelsStandardListModel>
{
    using Super = TViewModelsTableBase<ModelsStandardListModel>;
public:
    using Super::Super;

    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid() || GetData() == nullptr) {
            return QVariant();
        }

        return GetData()->At(index.row()).value(role, QVariant());
    }

    int rowCount(const QModelIndex& index = QModelIndex()) const override
    {
        if(index.isValid()) {
            return 0;
        }
        if(GetData() == nullptr) {
            return 0;
        }
        return GetData()->GetSize();
    }

    int columnCount(const QModelIndex& index = QModelIndex()) const override
    {
        if(index.isValid()) {
            return 0;
        }
        return 1;
    }

    template<class Enum>
    static ViewModelsStandardListModel* CreateEnumModel(QObject* parent, const std::function<void (qint32, ModelsStandardListModel::value_type&)>& extraFieldsHandler = [](ModelsStandardListModel::container_type&){})
    {
        auto* result = new ViewModelsStandardListModel(parent);
        auto model = ::make_shared<ModelsStandardListModel>();
        model->SetEnum<Enum>(extraFieldsHandler);
        result->SetData(model);
        return result;
    }
};

#endif // MODELSLISTBASE_H
