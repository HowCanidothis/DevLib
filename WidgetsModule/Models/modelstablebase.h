#ifndef MODELSTABLEBASE_H
#define MODELSTABLEBASE_H

#include <QAbstractTableModel>
#include <QMimeData>

#include <PropertiesModule/internal.hpp>
#include "WidgetsModule/Utils/iconsmanager.h"

class ModelsAbstractItemModel : public QAbstractItemModel
{
    using Super = QAbstractItemModel;
    using Super::Super;

    friend class ModelsWrapperBase;
    friend class ModelsTableWrapper;
    friend class ModelsTreeWrapper;
    static ModelsAbstractItemModel* Wrap(QAbstractItemModel* model) { return reinterpret_cast<ModelsAbstractItemModel*>(model); }
};

struct ModelsIconsContext
{
    IconsSvgIcon ErrorIcon;
    IconsSvgIcon WarningIcon;
    IconsSvgIcon InfoIcon;

private:
    friend class ViewModelsTableBase;
    ModelsIconsContext();
};

class ViewModelsTableColumnComponents
{
public:
    struct ColumnComponentData
    {
        using FSetterHandler = std::function<std::optional<bool> (const QModelIndex& index, const QVariant& data)>;
        using FGetterHandler = std::function<std::optional<QVariant> (const QModelIndex& index)>;
        using FGetHeaderHandler = std::function<std::optional<QVariant> ()>;
        FSetterHandler SetterHandler;
        FGetterHandler GetterHandler;
        FGetHeaderHandler GetHeaderHandler;

        ColumnComponentData()
            : SetterHandler([](const QModelIndex&, const QVariant&) { return false; })
            , GetterHandler([](const QModelIndex&) { return QVariant(); })
            , GetHeaderHandler([]() { return QVariant(); })
        {}

        ColumnComponentData(bool /*propagate*/)
            : SetterHandler([](const QModelIndex&, const QVariant&) { return std::nullopt; })
            , GetterHandler([](const QModelIndex&) { return std::nullopt; })
            , GetHeaderHandler([]{ return std::nullopt; })
        {}

        ColumnComponentData& SetSetter(const FSetterHandler& setter)
        {
            SetterHandler = setter;
            return *this;
        }
        ColumnComponentData& SetGetter(const FGetterHandler& setter)
        {
            GetterHandler = setter;
            return *this;
        }
        ColumnComponentData& SetHeader(const FGetHeaderHandler& setter)
        {
            GetHeaderHandler = setter;
            return *this;
        }
    };

    struct ColumnFlagsComponentData
    {
        std::function<std::optional<Qt::ItemFlags> (qint32 row)> GetFlagsHandler = [](qint32) { return std::nullopt; };
    };

    ViewModelsTableColumnComponents();

    void AddComponent(qint32 role /*Qt::ItemDataRole*/, qint32 column, const ColumnComponentData& columnData);
    void AddFlagsComponent(qint32 column, const ColumnFlagsComponentData& flagsColumnData);

    std::optional<bool> SetData(const QModelIndex& index, const QVariant& data, qint32 role);
    std::optional<QVariant> GetData(const QModelIndex& index, qint32 role) const;
    std::optional<QVariant> GetHeaderData(qint32 section, qint32 role) const;
    std::optional<Qt::ItemFlags> GetFlags(const QModelIndex& index) const;

    qint32 GetColumnCount() const;

private:
    bool callHandler(qint32 column, qint32 role, const std::function<void (const QVector<ColumnComponentData>&)>& onFound) const;
    bool callFlagsHandler(qint32 column, const std::function<void (const QVector<ColumnFlagsComponentData>& )>& onFound) const;

private:
    QMap<qint32, QHash<qint32, QVector<ColumnComponentData>>> m_columnComponents;
    QMap<qint32, QVector<ColumnFlagsComponentData>> m_columnFlagsComponents;
};

class ViewModelsTableBase : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    ViewModelsTableBase(QObject* parent = nullptr);
    ~ViewModelsTableBase();

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, qint32 role) const override;
    bool setData(const QModelIndex& index, const QVariant& data, qint32 role) override;
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override;
    qint32 columnCount(const QModelIndex& = QModelIndex()) const override;

    void RequestUpdateUi(qint32 left, qint32 right);

#ifdef UNITS_MODULE_LIB // TODO. Do not use it. Deprecated
    DispatcherConnection AttachTempDependence(const char* locationInfo, const class Measurement* unitName, int first, int last);
    void AttachDependence(const char* locationInfo, const Measurement* unitName, int first, int last);
#endif
    DispatcherConnection AttachTempDependence(const char* locationInfo, const Dispatcher* dispatcher, int first, int last);
    void AttachDependence(const char* locationInfo, const Dispatcher* dispatcher, int first, int last);

    const ModelsIconsContext& GetIconsContext() const { return m_iconsContext; }

    static Qt::ItemFlags StandardEditableFlags() { return StandardNonEditableFlags() | Qt::ItemIsEditable; }
    static Qt::ItemFlags StandardNonEditableFlags() { return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable; }

    Dispatcher OnModelChanged;

    ViewModelsTableColumnComponents ColumnComponents;

protected:
    ModelsIconsContext m_iconsContext;
    QHash<qint32, std::function<QVariant (qint32 row, qint32 column)>> m_roleDataHandlers;
    QHash<qint32, std::function<QVariant (qint32 column)>> m_roleHorizontalHeaderDataHandlers;
    QHash<qint32, std::function<QVariant (qint32 column)>> m_roleVerticalHeaderDataHandlers;
    QHash<qint32, std::function<bool (qint32 row, qint32 column, const QVariant&)>> m_roleSetDataHandlers;
    DispatcherConnectionsSafe m_connections;
    qint32 m_mostLeftColumnToUpdate;
    qint32 m_mostRightColumnToUpdate;
    DelayedCallObject m_update;
};

template<class T>
struct TypeHelper
{
    T& ToReference(T& value) { return value; }
    const T& ToReference(const T& value) { return value; }
};

template<class T>
struct TypeHelper<SharedPointer<T>>
{
    T& ToReference(SharedPointer<T>& value) { return *value; }
    const T& ToReference(const SharedPointer<T>& value) { return *value; }
};

template<class T>
struct TypeHelper<T*>
{
    T& ToReference(T*& value) { return *value; }
    const T& ToReference(const T*& value) { return *value; }
};

template<class T>
class TViewModelsTableBase : public ViewModelsTableBase
{
    using Super = ViewModelsTableBase;
public:
    using FInsertHandler = std::function<bool (int row, int count)>;
    using FCanDropMimeDataHandler = std::function<bool (const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)>;

    TViewModelsTableBase(QObject* parent)
        : Super(parent)
        , m_mimeTypesHandler([this]{ return Super::mimeTypes(); })
        , m_mimeDataHandler([this](const QModelIndexList& indexes){ return Super::mimeData(indexes); })
        , m_dropMimeDataHandler([this](const QMimeData* data, Qt::DropAction action, qint32 row, qint32 column, const QModelIndex& index){
            return Super::dropMimeData(data, action, row, column, index);
        })
        , m_canDropMimeDataHandler([this](const QMimeData* , Qt::DropAction , int , int , const QModelIndex& ){
            return true;
        })
        , m_insertHandler([this](qint32 row, qint32 count){
            const auto& data = GetData();
            Q_ASSERT(data != nullptr);
            data->Insert(row > data->GetSize() ? data->GetSize() : row, count);
            return true;
        })
    {}

    void SetCanDropMimeDataHandler(const FCanDropMimeDataHandler& handler)
    {
        m_canDropMimeDataHandler = handler;
    }

    void SetInsertionHandler(const FInsertHandler& insertHandler)
    {
        m_insertHandler = insertHandler;
    }

    void SetMimeDataHandlers(const std::function<QStringList ()>& mimeTypesHandler,
                             const std::function<QMimeData* (const QModelIndexList&)>& mimeDataHandler,
                             const std::function<bool (const QMimeData*, Qt::DropAction, qint32, qint32, const QModelIndex&)>& dropMimeDataHandler
                             )
    {
        m_mimeTypesHandler = mimeTypesHandler;
        m_mimeDataHandler = mimeDataHandler;
        m_dropMimeDataHandler = dropMimeDataHandler;
    }

    ~TViewModelsTableBase()
    {
        if(m_data != nullptr) {
            m_data->DisconnectModel(this);
        }
    }

    qint32 rowCount(const QModelIndex& index = QModelIndex()) const override
    {
        if(index.isValid()) {
            return 0;
        }
        return GetData() != nullptr ? GetData()->GetSize() : 0;
    }
    bool insertRows(int row, int count, const QModelIndex& = QModelIndex()) override
	{
        return m_insertHandler(row, count);
	}
    bool removeRows(int row, int count, const QModelIndex& = QModelIndex()) override
	{
        Q_ASSERT(GetData() != nullptr);
		QSet<qint32> indexs;
		while(count){
            indexs.insert(row + --count);
		}
		GetData()->Remove(indexs);
		return true;
	}

    QStringList mimeTypes() const override
    {
        return m_mimeTypesHandler();
    }

    QMimeData* mimeData(const QModelIndexList& indices) const override
    {
        return m_mimeDataHandler(indices);
    }

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                             int row, int column,
                                             const QModelIndex& parent) const
    {
        if(m_canDropMimeDataHandler(data, action, row, column, parent)) {
            return Super::canDropMimeData(data, action, row, column, parent);
        }
        return false;
    }

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, qint32 row, qint32 column, const QModelIndex& index) override
    {
        return m_dropMimeDataHandler(data, action, row, column, index);
    }
	
    void SetData(const SharedPointer<T>& data)
    {
        if(m_data == data) {
            return;
        }
        beginResetModel();
        if(m_data != nullptr) {
            m_data->DisconnectModel(this);
        }
        m_data = data;
        if(m_data != nullptr) {
            m_data->ConnectModel(this);
        }
        endResetModel();
        OnModelChanged();
    }
    const SharedPointer<T>& GetData() const { return m_data; }

protected:
    bool isLastEditRow(const QModelIndex& index) const
    {
        Q_ASSERT(GetData() != nullptr);
        return GetData()->GetSize() == index.row();
    }

private:
    SharedPointer<T> m_data;
    std::function<QStringList ()> m_mimeTypesHandler;
    std::function<QMimeData* (const QModelIndexList&)> m_mimeDataHandler;
    std::function<bool (const QMimeData*, Qt::DropAction, qint32, qint32, const QModelIndex&)> m_dropMimeDataHandler;
    FCanDropMimeDataHandler m_canDropMimeDataHandler;
    FInsertHandler m_insertHandler;
};

template<typename T>
class TViewModelsEditTable : public T
{
    using Super = T;
public:
    TViewModelsEditTable(QObject* parent)
        : Super(parent)
        , CreateDataHandler ([this](qint32, const QVariant&){ insertRows(rowCount()-1, 1); })
        , IsEditColumn      ([](qint32 column){ return true; })
        , DataHandler       ([](qint32, int ){ return QVariant();})
        , IsEditable(true)
        , m_isEditable(true)
    {
        setProperty(ExtraFieldsCountPropertyName, 1);

        IsEditable.Connect(CONNECTION_DEBUG_LOCATION, [this](bool editable){
            if(GetData() == nullptr) {
                m_isEditable = editable;
                return;
            }
            auto rows = Super::rowCount();
            if(editable) {
                beginInsertRows(QModelIndex(), rows, rows);
                m_isEditable = editable;
                setProperty(ExtraFieldsCountPropertyName, 1);
                endInsertRows();
            } else {
                beginRemoveRows(QModelIndex(), rows, rows);
                m_isEditable = editable;
                setProperty(ExtraFieldsCountPropertyName, 0);
                endRemoveRows();
            }
        });
    }

    std::function<void(qint32, const QVariant&)> CreateDataHandler;
    std::function<bool(qint32)> IsEditColumn;
    std::function<QVariant(qint32, int)> DataHandler;
    LocalPropertyBool IsEditable;

public:    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if(GetData() == nullptr) {
            return 0;
        }
        if(!m_isEditable) {
            return Super::rowCount(parent);
        }
        return Super::rowCount(parent) + 1;
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if(!m_isEditable) {
            return Super::data(index, role);
        }
        if(!index.isValid()){
            return QVariant();
        }
        if(isLastEditRow(index)){
            return DataHandler(index.column(), role);
        }
        return Super::data(index, role);
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
    {
        if(!m_isEditable) {
            return Super::setData(index, value, role);
        }

        if(!index.isValid()){
            return false;
        }
        if(isLastEditRow(index)){
            CreateDataHandler(index.column(), value);
        }
        return Super::setData(index, value, role);
    }

    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override
    {
        if(!m_isEditable) {
            return Super::flags(index);
        }

        if(!index.isValid()) {
            return Qt::ItemIsDropEnabled;
        }
        if(isLastEditRow(index)){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | (IsEditColumn(index.column()) ? Qt::ItemIsEditable : Qt::NoItemFlags);
        }
        return Super::flags(index);
    }

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
        if(!m_isEditable) {
            return Super::removeRows(row, count, parent);
        }

        if(row + count >= rowCount()){
            count = Super::rowCount() - row;
        }
        if(count <= 0){
            return false;
        }
        return Super::removeRows(row, count, parent);
    }

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
        if(!m_isEditable) {
            return Super::insertRows(row, count, parent);
        }

        auto existsCount = rowCount();
        if(row >= existsCount){
            row = existsCount - 1;
        }
        return Super::insertRows(row, count, parent);
    }

private:
    bool m_isEditable;
};

template<class T>
class TViewModelsDataInitializerComponent : public QObject
{
    using Super = QObject;
public:
    using value_type = typename T::value_type;

    TViewModelsDataInitializerComponent(TViewModelsTableBase<T>* model, const typename T::FDataInitializer& initializer)
        : Super(model)
    {
        model->SetInsertionHandler([model, initializer](qint32 row, qint32 count){
            const auto& data = model->GetData();
            Q_ASSERT(data != nullptr);
            data->Insert(row > data->GetSize() ? data->GetSize() : row, count, initializer);
            return true;
        });
    }
};

template<class T>
class TViewModelsDragAndDropComponent : public QObject
{
    using Super = QObject;
public:
    using value_type = typename T::value_type;

    struct WrappedObject
    {
        value_type Value;
        qint32 Row;

        WrappedObject(const value_type& value, qint32 row)
            : Value(value)
            , Row(row)
        {}

        WrappedObject()
            : Row(-1)
        {}

        template<class Buffer>
        void Serialize(Buffer& buffer)
        {
            buffer.OpenSection("MimeDataObject");
            buffer << buffer.Sect("Row", Row);
            buffer << Value;
            buffer.CloseSection();
        }
    };

    TViewModelsDragAndDropComponent(TViewModelsTableBase<T>* model)
        : Super(model)
        , m_model(model)
    {
        model->SetMimeDataHandlers([this]{
            return m_mimeTypes;
        }, [this, model](const QModelIndexList& indexes){
            auto* result = new QMimeData();

            if(model->GetData() == nullptr || m_mimeDataGetterType.isEmpty()) {
                return result;
            }

            Array<qint32> rows;
            for(const auto& index : indexes) {
                rows.InsertSortedUnique(index.row());
            }

            auto data = m_mimeDataGetter(rows);
            result->setText(data);
            result->setProperty("SourceModel", (size_t)model->GetData().get());

            result->setData(m_mimeDataGetterType, data);
            return result;
        }, [this, model](const QMimeData* data, Qt::DropAction action, qint32 row, qint32 column, const QModelIndex& index){
            if(model->GetData() == nullptr || m_mimeTypes.isEmpty()) {
                return false;
            }

            for(const QString& format : data->formats()) {
                auto foundIt = m_mimeDataSetters.find(Name(format));
                if(foundIt != m_mimeDataSetters.end()) {
                    auto objects = foundIt.value()(data->data(foundIt.key().AsString()));

                    auto* sourceModel = (T*)data->property("SourceModel").toLongLong();

                    if(objects.isEmpty()) {
                        return false;
                    }

                    QVector<qint32> toRemove;
                    if(sourceModel == model->GetData().get()) {
                        for(const auto& object : objects) {
                            toRemove.append(object.Row);
                        }
                    }

                    if(row < 0 || row >= model->GetData()->GetSize()) {
                        model->GetData()->Change([this, &objects](auto& native){
                            for(const auto& wrapped : objects) {
                                native.append(wrapped.Value);
                            }
                        });
                    } else {
                        for(auto& removed : toRemove) {
                            if(removed >= row) {
                                removed += objects.size();
                            }
                        }

                        auto it = objects.begin();
                        model->GetData()->Insert(row, objects.size(), [&it](qint32, typename T::value_type& ptr){
                            ptr = it->Value;
                            it++;
                        });
                    }

                    model->GetData()->RemoveFromVector(toRemove);
                    return true;
                }
            }

            return false;
        });
    }

    void SetDefaultMimeDataXml(const SerializerXmlVersion& version, const Name& mimeType,
                               const DescSerializationXMLWriteParams& writeParams = DescSerializationXMLWriteParams(),
                               const DescSerializationXMLReadParams& readParams = DescSerializationXMLReadParams())
    {
        SetMimeDataGetter(mimeType.AsString(), [this, version, writeParams, mimeType](const Array<qint32>& rows) -> QByteArray {
            QVector<WrappedObject> result;
            for(const auto& row : rows) {
                const auto& rowObject = m_model->GetData()->At(row);
                result.append(WrappedObject(rowObject, row));
            }
            return SerializeToXMLVersioned(version, "Objects", result, writeParams);
        });

        AddMimeDataSetter(mimeType, [version, readParams, mimeType](const QByteArray& data) -> QVector<WrappedObject> {
            QVector<WrappedObject> result;
            if(!DeSerializeFromXMLVersioned(version, "Objects", data, result, readParams)) {
                return QVector<WrappedObject>();
            }
            return result;
        });
    }

    void SetMimeDataGetter(const QString& mimeType, const std::function<QByteArray (const Array<qint32>& rows)>& getter)
    {
        Q_ASSERT(!mimeType.isEmpty());
        m_mimeDataGetterType = mimeType;
        m_mimeDataGetter = getter;
    }

    void AddMimeDataSetter(const Name& mimeType, const std::function<QVector<WrappedObject> (const QByteArray& data)>& setter)
    {
        m_mimeTypes.append(mimeType.AsString());
        m_mimeDataSetters.insert(mimeType, setter);
    }

private:
    TViewModelsTableBase<T>* m_model;
    QHash<Name, std::function<QVector<WrappedObject> (const QByteArray&)>> m_mimeDataSetters;
    QStringList m_mimeTypes;
    QString m_mimeDataGetterType;
    std::function<QByteArray (const Array<qint32>&)> m_mimeDataGetter;
};

template<class Wrapper>
struct ModelsTableBaseDecorator
{
    /// deprecated
    static QVariant GetModelData(const typename Wrapper::value_type& data, qint32 column, qint32 role = Qt::DisplayRole);
    static void Sort(const typename Wrapper::container_type& rows, qint32 column, Array<qint32>& indices);
    /// deprecated
    static bool SetModelData(const QVariant& value, typename Wrapper::value_type& data, qint32 column, qint32 role = Qt::DisplayRole);
    /// deprecated
    static Qt::ItemFlags GetFlags(const typename Wrapper::value_type&, qint32);
    /// deprecated
    static QVariant GetHeaderData(int section, Qt::Orientation orientation, qint32 role = Qt::DisplayRole);
};

template<class Wrapper>
struct ModelsTableDecoratorHelpers
{
    template<qint32 Column>
    static void Sort(const Wrapper& rows, Array<qint32>& indices)
    {
        std::sort(indices.begin(), indices.end(), [&](qint32 f, qint32 s){
            return rows.At(f).template Get<Column>() < rows.At(s).template Get<Column>();
        });
    }
};

template<class Wrapper>
class TViewModelsDecoratedTable : public TViewModelsTableBase<Wrapper>
{
    using Super = TViewModelsTableBase<Wrapper>;
public:
    using Super::Super;

    qint32 columnCount(const QModelIndex&  = QModelIndex()) const override
    {
        return std::max(Super::ColumnComponents.GetColumnCount(), (qint32)Wrapper::value_type::count);
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if(!index.isValid()) {
            return QVariant();
        }
        Q_ASSERT(Super::GetData() != nullptr);

        auto componentsResult = Super::ColumnComponents.GetData(index, role);
        if(componentsResult.has_value()) {
            return componentsResult.value();
        }

        return ModelsTableBaseDecorator<Wrapper>::GetModelData(Super::GetData()->At(index.row()), index.column(), role);
    }
    bool setData(const QModelIndex& index, const QVariant& data, qint32 role = Qt::DisplayRole) override
    {
        bool result = false;
        if(!index.isValid()) {
            return result;
        }

        Q_ASSERT(Super::GetData() != nullptr);

        auto componentsResult = Super::ColumnComponents.SetData(index, data, role);
        if(componentsResult.has_value()) {
            return componentsResult.value();
        }

        Super::GetData()->Edit(index.row(), [&](typename Wrapper::value_type& value){
            result = ModelsTableBaseDecorator<Wrapper>::SetModelData(data, value, index.column(), role);
        }, {index.column()});
        return result;
    }

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override 
	{
        auto componentsResult = Super::ColumnComponents.GetHeaderData(section, role);
        if(componentsResult.has_value()) {
            return componentsResult.value();
        }

		return ModelsTableBaseDecorator<Wrapper>::GetHeaderData(section, orientation, role);
	}
	
    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if(!index.isValid()) {
            return Qt::NoItemFlags;
        }

        auto componentsResult = Super::ColumnComponents.GetFlags(index);
        if(componentsResult.has_value()) {
            return componentsResult.value();
        }

        return ModelsTableBaseDecorator<Wrapper>::GetFlags(Super::GetData()->At(index.row()), index.column());
    }
};

template<class Wrapper>
class ModelsTableSearchComponent
{
    struct Cache
    {
        Array<qint32> SortedData;
        bool IsValid = false;
        Dispatcher ColumnListener;
    };

public:
    template<class T, qint32 Column>
    class iterator {
        qint32 m_row;
        ModelsTableSearchComponent* m_searchComponent;
        ModelsTableSearchComponent::Cache* m_cache;

        friend class ModelsTableSearchComponent;

        iterator(qint32 row, ModelsTableSearchComponent* searchComponent)
            : m_row(row)
            , m_searchComponent(searchComponent)
            , m_cache(&searchComponent->m_sortedColumns[Column])
        {

        }
    public:
        typedef iterator It;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;
        typedef T value_type;
        typedef const T* pointer;
        typedef const T& reference;

        bool operator!=(const It& other) const{ return this->m_row != other.m_row; }
        It& operator++()
        {
            ++m_row;
            return *this;
        }
        It operator++(qint32)
        {
            ++m_row;
            return *this;
        }
        It& operator--()
        {
            --m_row;
            return *this;
        }
        It operator+(qint32 index)
        {
            It n(*this);
            n.m_row += index;
            return n;
        }
        It& operator+=(qint32 index)
        {
            m_row += index;
            return *this;
        }
        friend size_t operator-(const It& first, const It& another)
        {
            return first.m_row - another.m_row;
        }
        reference operator*() const { Q_ASSERT(IsValid()); return m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]).template Get<Column>(); }
        pointer operator->() const { Q_ASSERT(IsValid()); return &m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]).template Get<Column>(); }
        reference value() const { return operator*(); }
        bool operator==(const It& other) const { return !(operator !=(other)); }
        bool IsValid() const { return m_row >= 0 && m_row < m_cache->SortedData.Size(); }
        qint32 GetRowIndex() const { return m_row; }
        const typename Wrapper::value_type& GetRow() const { Q_ASSERT(IsValid()); return m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]); }
        qint32 GetSourceRowIndex() const { Q_ASSERT(IsValid()); return m_cache->SortedData[m_row]; }
    };

    using value_type = typename Wrapper::value_type;
    using const_iterator = typename Wrapper::container_type::const_iterator;
    ModelsTableSearchComponent(const SharedPointer<Wrapper>& wrapper)
        : m_wrapper(wrapper)
    {
        auto invalidate = [this](const QSet<qint32>& columns){
            if(columns.isEmpty()) {
                for(qint32 i(0); i < value_type::count; i++) {
                    auto& cache = m_sortedColumns[i];
                    cache.IsValid = false;
                    cache.ColumnListener();
                }
            } else {
                for(qint32 i : columns) {
                    if(i<Wrapper::value_type::count){
                        auto& cache = m_sortedColumns[i];
                        cache.IsValid = false;
                        cache.ColumnListener();
                    }
                }
            }
        };

        m_wrapper->OnColumnsChanged.Connect(CONNECTION_DEBUG_LOCATION, invalidate).MakeSafe(m_connections);
    }

    QList<const value_type&> Select(const std::function<bool(const value_type&)>& where)
    {
        QList<const value_type&> result;
        for(const auto& row : *m_wrapper){
            if(where(row)){
                result.append(row);
            }
        }
        return result;
    }
    
    template<class T, qint32 Column>
    iterator<T, Column> FindEqualOrGreater(const T& value) const
    {
        return std::lower_bound(begin<T,Column>(), end<T,Column>(), value, [](const T& f, const T&s){
            return f < s;
        });
    }

    template<class T, qint32 Column>
    iterator<T, Column> FindEqualOrLower(const T& value) const
    {
        auto foundIt = FindEqualOrGreater<T,Column>(value);
        if(!foundIt.IsValid() || foundIt.value() > value) {
            --foundIt;
        }
        return foundIt;
    }

    void Sort(qint32 column) const
    {
        Q_ASSERT(column >= 0 && column < value_type::count);
        auto& cache = m_sortedColumns[column];
        if(!cache.IsValid) {
            ModelsTableBaseDecorator<Wrapper>::Sort(*m_wrapper, column, cache.SortedData);
            cache.IsValid = true;
        }
    }

    template<qint32 Column>
    DispatcherConnection Bind(const FAction& action) const
    {
        return m_sortedColumns[Column].ColumnListener.Connect(CONNECTION_DEBUG_LOCATION, action);
    }

    template<class T, qint32 Column>
    iterator<T, Column> begin() const
    {
        Sort(Column);
        return iterator<T, Column>(0, const_cast<ModelsTableSearchComponent*>(this));
    }

    template<class T, qint32 Column>
    iterator<T, Column> end() const
    {
        Sort(Column);
        return iterator<T, Column>(m_sortedColumns[Column].SortedData.Size(), const_cast<ModelsTableSearchComponent*>(this));
    }

    template<class T, qint32 Column>
    adapters::Range<iterator<T, Column>> range() const
    {
        return adapters::range(begin<T,Column>(), end<T,Column>());
    }

    const SharedPointer<Wrapper>& GetSource() const { return m_wrapper; }

private:
    template<class T, qint32> friend class iterator;
    SharedPointer<Wrapper> m_wrapper;
    mutable Cache m_sortedColumns[Wrapper::value_type::count];
    DispatcherConnectionsSafe m_connections;
};

namespace widget_models
{

template<qint32 Column>
struct VisitorHelper
{
    template<class Wrapper>
    static void Sort(const Wrapper& rows, Array<qint32>& indices)
    {
        std::sort(indices.begin(), indices.end(), [&](qint32 f, qint32 s){
            return rows.At(f).template Get<Column>() < rows.At(s).template Get<Column>();
        });
    }
};

template <std::size_t L, std::size_t U>
struct Visitor
{
    template <class Wrapper>
    static void Sort(const Wrapper& rows, Array<qint32>& indices, std::size_t idx)
    {
        static constexpr std::size_t MEDIAN = (U - L) / 2 + L;
        if (idx > MEDIAN)
            Visitor<MEDIAN, U>::Sort(rows, indices, idx);
        else if (idx < MEDIAN)
            Visitor<L, MEDIAN>::Sort(rows, indices, idx);
        else
            VisitorHelper<MEDIAN>::Sort(rows, indices);
    }
};

template <class Wrapper>
void sort(const Wrapper& rows, Array<qint32>& indices, std::size_t idx)
{
    assert(idx <= Wrapper::value_type::count);
    Visitor<0, Wrapper::value_type::count>::Sort(rows, indices, idx);
}

}

#endif // MODELSTABLEBASE_H
