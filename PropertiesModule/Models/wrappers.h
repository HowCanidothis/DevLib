#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <QAbstractItemModel>

#include <PropertiesModule/internal.hpp>

class ModelsWrapperBase
{
public:
    ModelsWrapperBase()
        : m_inScope(false)
    {
        OnAboutToBeReseted.Connect(CONNECTION_DEBUG_LOCATION, [this] { testInScope(); });
        OnAboutToBeUpdated.Connect(CONNECTION_DEBUG_LOCATION, [this] { testInScope(); });
        OnReseted.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnUpdated.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnRowsRemoved.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnRowsInserted.Connect(CONNECTION_DEBUG_LOCATION, [this] (qint32, qint32) { testOutScope(); });
    }

    virtual ~ModelsWrapperBase()
    {
        OnAboutToBeDestroyed();
    }

#ifdef SHARED_LIB_ADD_UI
    virtual void ConnectModel(class QAbstractItemModel* model);
    virtual void DisconnectModel(QAbstractItemModel* model);
#endif

    bool IsInScope() const { return m_inScope; }

    Dispatcher OnAboutToBeReseted;
    Dispatcher OnReseted;
    Dispatcher OnAboutToBeUpdated;
    Dispatcher OnUpdated;
    Dispatcher OnRowsRemoved;
    CommonDispatcher<qint32, qint32> OnRowsInserted;
    Dispatcher OnAboutToBeDestroyed;
    Dispatcher OnChanged;
    CommonDispatcher<QSet<qint32>> OnColumnsChanged;
    CommonDispatcher<qint32, qint32> OnAboutToChangeRows;
    CommonDispatcher<qint32, qint32, const QSet<qint32>&> OnRowsChanged;

protected:
    void testInScope()
    {
        Q_ASSERT(!m_inScope);
        m_inScope = true;
    }

    void testOutScope()
    {
        Q_ASSERT(m_inScope);
        m_inScope = false;
    }

private:
    bool m_inScope;
};

class ModelsTreeWrapper : public ModelsWrapperBase
{
    using Super = ModelsWrapperBase;
public:
    ModelsTreeWrapper()
    {
        OnAboutToInsertRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32,qint32, ModelsTreeItemBase*){ testInScope(); });
        OnAboutToRemoveRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32,qint32, ModelsTreeItemBase*){ testInScope(); });
    }

#ifdef SHARED_LIB_ADD_UI
    void ConnectModel(QAbstractItemModel* model) override;
    void DisconnectModel(QAbstractItemModel* model) override;
#endif

    CommonDispatcher<qint32,qint32,class ModelsTreeItemBase*> OnAboutToRemoveRows;
    CommonDispatcher<qint32,qint32,ModelsTreeItemBase*> OnAboutToInsertRows;

    CommonDispatcher<size_t, ModelsTreeItemBase*,QVector<int>> OnTreeValueChanged;

    virtual ModelsTreeItemBase* GetRoot() = 0;
};

class ModelsTableWrapper : public ModelsWrapperBase
{
    using Super = ModelsWrapperBase;
public:
    ModelsTableWrapper()
    {
        OnAboutToInsertRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32,qint32){ testInScope(); });
        OnAboutToRemoveRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32,qint32){ testInScope(); });
    }

#ifdef SHARED_LIB_ADD_UI
    void ConnectModel(QAbstractItemModel* model) override;
    void DisconnectModel(QAbstractItemModel* model) override;
#endif

    CommonDispatcher<qint32,qint32> OnAboutToRemoveRows;
    CommonDispatcher<qint32,qint32> OnAboutToInsertRows;
    CommonDispatcher<qint32,qint32> OnValueChanged;
};

template<class Container>
class TModelsTableWrapper : protected Container, public ModelsTableWrapper
{
    using Super = Container;
public:
    using container_type = Container;
    using value_type = typename Super::value_type;
    using Super::Super;
    using FDataInitializer = std::function<void (qint32 index, value_type& data)>;

    TModelsTableWrapper<Container>& BaseWrapper() { return *this; }

    void Set(const Super& another)
    {
        Change([&another](Super& data){ data = another; });
    }

    void Swap(Super& another)
    {
        OnAboutToBeReseted();
        Super::swap(another);
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    void Clear() {
        OnAboutToBeReseted();
        Super::clear();
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    bool IsValidRow(qint32 row) const
    {
        return row >= 0 && row < GetSize();
    }

    qint32 Find(const value_type& value) const
    {
        return Find([&value](const value_type& current){ return value == current; });
    }

    qint32 Find(const std::function<bool (const value_type&)>& searchPredicate) const
    {
        auto it = Super::begin();
        while(it != Super::end()) {
            if(searchPredicate(*it)) {
                return std::distance(Super::begin(), it);
            }
            it++;
        }
        return -1;
    }

    virtual void Prepend(const value_type& part)
    {
        OnAboutToInsertRows(0, 0);
        Super::prepend(part);
        OnRowsInserted(0, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    virtual void Append(const value_type& part)
    {
        auto size = GetSize();
        OnAboutToInsertRows(size, size);
        Super::append(part);
        OnRowsInserted(size, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    void InsertSorted(const value_type& value, const std::function<bool (const value_type& f, const value_type& s )>& lessThan = [](const value_type& f, const value_type& s ){ return f < s; })
    {
        auto insertTo = std::lower_bound(begin(), end(), value, lessThan);
        auto index = std::distance(begin(), insertTo);
        OnAboutToInsertRows(index, index);
        Super::insert(index, value);
        OnRowsInserted(index, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    template<class T2, typename F = bool (*)(const value_type&, const T2&)>
    typename Super::const_iterator FindSorted(const T2& value, const F& lessThan) const
    {
        return std::lower_bound(begin(), end(), value, lessThan);
    }

	void Insert(int index, int count, const value_type& part = value_type())
	{
		OnAboutToInsertRows(index, index + count - 1);
        Super::insert(index, count, part);
        OnRowsInserted(index, count);
        OnChanged();
        OnColumnsChanged({});
	}

    void Insert(int index, qint32 count, const FDataInitializer& dataInitializer)
    {
        OnAboutToInsertRows(index, index + count - 1);
        Super::insert(index, count, value_type());
        qint32 i = index;
        for(auto& value : adapters::range(Super::begin() + index, Super::begin() + index + count)) {
            dataInitializer(i, value);
        }
        OnRowsInserted(index, count);
        OnChanged();
        OnColumnsChanged({});
    }
	
    void Insert(int index, const value_type& part)
    {
        OnAboutToInsertRows(index, index);
        Super::insert(index, part);
        OnRowsInserted(index, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    void UpdateUi(const FAction& handler)
    {
        OnAboutToBeUpdated();
        handler();
        OnUpdated();
    }

    void Update(const FAction& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeUpdated();
        handler();
        OnUpdated();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Change(const std::function<void (Container&)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeReseted();
        handler(EditSilent());
        OnReseted();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Change(const FAction& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeReseted();
        handler();
        OnReseted();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void RemoveFromVector(const QVector<qint32>& vectorIndexes)
    {
        QSet<qint32> toRemove;
        for(auto index : vectorIndexes) {
            toRemove.insert(index);
        }
        Remove(toRemove);
    }

    void Clone(const QVector<value_type>& toClone, const std::function<void (container_type& container, value_type& cloned)>& handler)
    {
        QVector<value_type> cloned;

        SerializeCopyObject(toClone, cloned, SerializationMode_Cloning);

        Change([&](container_type& native){
            for(auto& value : cloned) {
                handler(native, value);
            }
        });
    }

    void Remove(const QSet<qint32>& indexes)
    {
        if(indexes.isEmpty()) {
            return;
        }

        OnAboutToBeReseted();
        qint32 currentIndex = 0;
        auto endIt = std::remove_if(Super::begin(), Super::end(), [&currentIndex, &indexes](const value_type&){
            if(indexes.contains(currentIndex++)) {
                return true;
            }
            return false;
        });
        Super::resize(std::distance(Super::begin(), endIt));
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    void Edit(qint32 index, qint32 count, const std::function<void (int, value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToChangeRows(index, count);
        for(int i=0; i<count; ++i){
            handler(i, Super::operator[](index + i));
        }
        OnRowsChanged(index, count, affectedColumns);
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Edit(qint32 index, const std::function<void (value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToChangeRows(index, 1);
        handler(Super::operator[](index));
        OnRowsChanged(index, 1, affectedColumns);
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void EditLast(const std::function<void (value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        if(IsEmpty()) {
            return;
        }
        auto index = GetSize() - 1;
        OnAboutToChangeRows(index, 1);
        handler(Super::operator[](index));
        OnRowsChanged(index, 1, affectedColumns);
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    bool EditWithCheck(qint32 index, const std::function<FAction (value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        auto updateFunction = handler(Super::operator[](index));
        if(updateFunction != nullptr) {
            OnAboutToChangeRows(index, 1);
            updateFunction();
            OnRowsChanged(index, 1, affectedColumns);
            OnChanged();
            OnColumnsChanged(affectedColumns);
            return true;
        }
        return false;
    }

    qint32 IndexOf(const value_type& value) const
    {
        return Super::indexOf(value);
    }

    Super& EditSilent() { return *this; }
    const Super& Native() const { return *this; }

    const value_type& First() const { return Super::first(); }
    const value_type& Last() const { return Super::last(); }
    bool IsEmpty() const { return Super::isEmpty(); }
    qint32 GetSize() const { return Super::size(); }

    bool AtSafe(qint32 index, const value_type*& value) const
    {
        if(index >= 0 && index < GetSize()) {
            value = &At(index);
            return true;
        }
        return false;
    }

    const value_type& At(qint32 index) const { return Super::at(index); }
    const value_type& operator[](qint32 index) const { return Super::operator[](index); }

    typename Super::const_iterator begin() const { return Super::begin(); }
    typename Super::const_iterator end() const { return Super::end(); }

    template<typename ... Dispatchers>
    DispatcherConnections Connect(const char* connectionInfo, const std::function<void(const Container&)>& handler, Dispatchers&... dispatchers){
        DispatcherConnections ret;
        ret += OnChanged.ConnectCombined(connectionInfo, [this, handler]{ handler(*this); }, dispatchers...);
        return ret;
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCall(const char* connectionInfo, const std::function<void(const Container&)>& handler, Dispatchers&... dispatchers){
        auto ret = Connect(connectionInfo, handler, dispatchers...);
        handler(*this);
        return ret;
    }
};

template<class Container>
class TStatedModelsTableWrapper : public TModelsTableWrapper<Container>
{
    using Super = TModelsTableWrapper<Container>;
public:
    TStatedModelsTableWrapper()
        : IsValid(true)
    {}

    void Swap(const SharedPointer<TStatedModelsTableWrapper>& data)
    {
        Super::Swap(*data);
    }

    void Swap(Container& data)
    {
        Super::Swap(data);
    }

    SharedPointer<TStatedModelsTableWrapper> Clone() const
    {
        auto result = ::make_shared<TStatedModelsTableWrapper>();
        result->EditSilent() = *this;
        return result;
    }

    StateProperty IsValid;
};

using ModelsTableWrapperPtr = SharedPointer<ModelsTableWrapper>;
using ModelsTreeWrapperPtr = SharedPointer<ModelsTreeWrapper>;

#endif // WRAPPERS_H
