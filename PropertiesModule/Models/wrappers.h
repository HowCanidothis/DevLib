#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <QAbstractItemModel>

#include <PropertiesModule/internal.hpp>

class ModelsWrapperBase
{
public:
    ModelsWrapperBase()
        : IsValid(true)
        , m_resetViewModelOnReset(true)
        , m_inScope(false)
    {
        OnAboutToBeReset.Connect(CONNECTION_DEBUG_LOCATION, [this] { testInScope(); });
        OnAboutToBeUpdated.Connect(CONNECTION_DEBUG_LOCATION, [this] { testInScope(); });
        OnReset.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnUpdated.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnRowsRemoved.Connect(CONNECTION_DEBUG_LOCATION, [this] { testOutScope(); });
        OnRowsInserted.Connect(CONNECTION_DEBUG_LOCATION, [this] (qint32, qint32) { testOutScope(); });
        adapters::ResetThread(OnAboutToBeReset, OnAboutToBeUpdated, OnReset, OnUpdated, OnRowsRemoved, OnRowsInserted);
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

    Dispatcher OnAboutToBeReset;
    Dispatcher OnReset;
    Dispatcher OnAboutToBeUpdated;
    Dispatcher OnUpdated;
    Dispatcher OnRowsRemoved;
    CommonDispatcher<qint32, qint32> OnRowsInserted;
    Dispatcher OnAboutToBeDestroyed;
    Dispatcher OnChanged;
    StateProperty IsValid;
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

protected:
    bool m_resetViewModelOnReset;

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

        adapters::ResetThread(OnAboutToInsertRows, OnAboutToRemoveRows);
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

        adapters::ResetThread(OnAboutToInsertRows, OnAboutToRemoveRows);
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
    using const_iterator = typename Super::const_iterator;
    using Super::Super;
    using FDataInitializer = std::function<void (qint32 index, value_type& data)>;

    TModelsTableWrapper<Container>& BaseWrapper() { return *this; }

    void Set(const Super& another)
    {
        if(another.size() == GetSize()) {
            m_resetViewModelOnReset = false;
        }
        OnAboutToBeReset();
        EditSilent() = another;
        OnReset();
        m_resetViewModelOnReset = true;
        OnChanged();
        OnColumnsChanged({});
    }

    void Swap(Super& another)
    {
        if(another.size() == GetSize()) {
            m_resetViewModelOnReset = false;
        }
        OnAboutToBeReset();
        Super::swap(another);
        OnReset();
        m_resetViewModelOnReset = true;
        OnChanged();
        OnColumnsChanged({});
    }

    void Clear() {
        if(Super::isEmpty()) {
            return;
        }
        OnAboutToBeReset();
        Super::clear();
        OnReset();
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
    typename const_iterator FindSorted(const T2& value, const F& lessThan) const
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
            dataInitializer(i++, value);
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
        OnAboutToBeReset();
        handler(EditSilent());
        OnReset();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Change(const FAction& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeReset();
        handler();
        OnReset();
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

        OnAboutToBeReset();
        qint32 currentIndex = 0;
        auto endIt = std::remove_if(Super::begin(), Super::end(), [&currentIndex, &indexes](const value_type&){
            if(indexes.contains(currentIndex++)) {
                return true;
            }
            return false;
        });
        Super::resize(std::distance(Super::begin(), endIt));
        OnReset();
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
    bool IsLastIndex(qint32 index) const { return (Super::size() - 1) == index; }
    bool HasIndex(qint32 index) const { return index >= 0 && index < Super::size(); }

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

    typename const_iterator begin() const { return Super::begin(); }
    typename const_iterator end() const { return Super::end(); }

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

    DispatcherConnection ConnectAction(const char* locationInfo, const FAction& action) const
    {
        return OnChanged.Connect(locationInfo, action);
    }

    Container Clone() const
    {
        return *this;
    }
};

template<class T> using TModelsTableWrapperPtr = SharedPointer<TModelsTableWrapper<T>>;

using ModelsTableWrapperPtr = SharedPointer<ModelsTableWrapper>;
using ModelsTreeWrapperPtr = SharedPointer<ModelsTreeWrapper>;

template<class T>
class ModelsTableView
{
    using TPtr = SharedPointer<T>;
public:
    ModelsTableView(const TPtr& data, qint32 start, qint32 count)
        : m_data(data)
        , m_offset(start)
        , m_last(start + count)
    {
        OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_data->OnChanged).MakeSafe(m_connections);
    }

    Dispatcher OnChanged;

    typename T::const_iterator At(qint32 index) const { return index >= m_data->GetSize() ? m_data->end() : m_data->begin() + index;  }
    typename T::const_iterator begin() const { return At(m_offset);  }
    typename T::const_iterator end() const { return m_data->begin() + std::min(m_last, m_data->GetSize()); }

    bool HasValue(qint32 index) const { return At(index) != m_data->end(); }
    qint32 GetSize() const { return std::distance(begin(), end()); }

private:
    TPtr m_data;
    DispatcherConnectionsSafe m_connections;
    qint32 m_offset;
    qint32 m_last;
};


template<class T>
struct LambdaValueWrapper
{

};

template<class T>
struct LambdaValueWrapper<T&>
{
    LambdaValueWrapper(T& value)
        : m_value(value)
    {}

    T& GetValue() const { return m_value; }
    operator T&() const { return m_value; }

private:
    T& m_value;
};

template<class T>
struct LambdaValueWrapper<T*>
{
    LambdaValueWrapper(T* value)
        : m_value(value)
    {}

    T* GetValue() const { return m_value; }
    operator T*() const { return m_value; }

private:
    T* m_value;
};

#define DECLARE_MODEL_BY_TYPE(TypeName) \
    using TypeName##Container = QVector<TypeName>; \
    using TypeName##Model = TModelsTableWrapper<TypeName##Container>; \
    using TypeName##ModelPtr = SharedPointer<TypeName##Model>; \
    using TypeName##ModelPtrInitialized = SharedPointerInitialized<TypeName##Model>; \
    using TypeName##ImmutableModel = StateImmutableData<TypeName##Model>; \
    using TypeName##ImmutableModelPtr = SharedPointer<StateImmutableData<TypeName##Model>>; \
    using TypeName##StateParameter = StateParametersContainer<StateImmutableData<TypeName##Model>>; \
    using TypeName##StateParameterPtr = StateParametersContainerPtr<StateImmutableData<TypeName##Model>>; \
    using TypeName##StateParameterPtrInitialized = StateParametersContainerPtrInitialized<StateImmutableData<TypeName##Model>>; \
    using TypeName##ProxyStateParameter = StateParametersProxy<TypeName##StateParameter>; \
    using TypeName##ProxyStateParameterPtr = SP<TypeName##ProxyStateParameter>;

#define DECLARE_MODEL_TYPENAME(ModelName) \
    using ModelName##Ptr = SharedPointer<ModelName>; \
    using ModelName##PtrInitialized = SharedPointerInitialized<ModelName>; \
    using ModelName##ImmutableModel = StateImmutableData<ModelName>; \
    using ModelName##ImmutableModelPtr = SharedPointer<StateImmutableData<ModelName>>; \
    using ModelName##StateParameter = StateParametersContainer<StateImmutableData<ModelName>>; \
    using ModelName##StateParameterPtr = StateParametersContainerPtr<StateImmutableData<ModelName>>; \
    using ModelName##StateParameterPtrInitialized = StateParametersContainerPtrInitialized<StateImmutableData<ModelName>>; \
    using ModelName##ProxyStateParameter = StateParametersProxy<ModelName##StateParameter>; \
    using ModelName##ProxyStateParameterPtr = SP<ModelName##ProxyStateParameter>;

#define DECLARE_MODEL(ModelName) \
    class ModelName; \
    DECLARE_MODEL_TYPENAME(ModelName)


template<class ModelType>
class StateChainedContainerCalculator
{
public:
    using container_type = typename ModelType::container_type;
    using model_type = ModelType;
    using state_parameter_type = StateParametersContainer<StateImmutableData<ModelType>>;

    StateChainedContainerCalculator()
    {
        Result->SetLockedParameter(m_calculatorResult.GetImmutableData(), SPCO());
    }

    void Initialize(const char* cdl)
    {
        m_calculator.OnCalculationRejected += { this, [this]{
            m_calculatorResult.EditData()->Clear();
        }};
        m_calculator.OnCalculated += { this, [this](const auto& container){
            Result->GetChainData()->EditErrors().Remove(nullptr);
            m_calculatorResult.EditData()->Set(container);
            m_calculatorResult.EditData()->IsValid.SetState(true);
        }};
        m_calculator.OnExceptionCaught += { this, [this](const Exception& ex) {
            Result->GetChainData()->EditErrors().Add(ex.GetError(), nullptr);
        }};
        m_calculatorResult.EditData()->IsValid.ConnectFromStateProperty(cdl, m_calculator.Valid);
        m_calculatorResult.Enabled.ConnectFrom(cdl, m_calculator.Enabled);
    }

    template<typename FFunction, typename ... Args>
    void Initialize(const char* cdl, const FFunction& handler, const Args&... args)
    {
        InitializeWithId(cdl, Name(), handler, args...);
    }

    template<typename FFunction, typename ... Args>
    void InitializeWithId(const char* cdl, const Name& processId, const FFunction& handler, const Args&... args)
    {
        Initialize(cdl);
        SetCalculator(cdl, processId, handler, args...);
    }

    template<typename FFunction, typename ... Args>
    void SetCalculator(const char* cdl, const Name& processId, const FFunction& handler, const Args&... args)
    {
        m_calculator.SetCalculatorWithParams(cdl, handler, args...);
        Result->ConnectChain(cdl, SPCCO().SetProcessId(processId).AddCapturers(m_calculator.GetCapturer()), args...);
    }

    void UnsetCalculator(const char* cdl)
    {
        m_calculator.Disconnect(cdl);
        Result->GetChainData()->SetCaptureHandler([](StateParametersChainData&){});
    }

    StateCalculator<container_type>& GetCalculator() { return m_calculator; }
    Interruptor GetInterruptor() { return m_calculator.GetInterruptor(); }
    SmartPointerWatcherPtr Capture()
    {
        if(!Result->IsInitialized()) {
            Result->Initialize();
        }
        return Result->GetChainData()->Capture();
    }

    SharedPointerInitialized<state_parameter_type> Result;

protected:
    StateCalculator<container_type> m_calculator;
    StateDoubleBufferData<model_type> m_calculatorResult;
};

template<class Property>
class StateChainedValueCalculator
{
public:
    class ResultParameter : public StateParameters
    {
    public:
        ResultParameter()
            : Value(this)
        {}

        StateParameterProperty<LocalPropertyOptional<Property>> Value;
    };

    using value_type = typename Property::value_type;

    StateChainedValueCalculator()
    {
        Result->SetChained(SPCO());
    }

    template<typename FFunction, typename ... Args>
    void Initialize(const char* cdl, const FFunction& handler, const Args&... args)
    {
        InitializeWithId(cdl, Name(), handler, args...);
    }

    template<typename FFunction, typename ... Args>
    void InitializeWithId(const char* cdl, const Name& processId, const FFunction& handler, const Args&... args)
    {
        m_calculator.SetCalculatorWithParams(cdl, handler, args...);
        m_calculator.OnCalculated += {this, [this](const auto& v) {
            Result->GetChainData()->EditErrors().Remove(nullptr);
            Result->Value.InputValue = v;
        }};
        m_calculator.OnExceptionCaught += { this, [this](const Exception& ex) {
            Result->GetChainData()->EditErrors().Add(ex.GetError(), nullptr);
        }};
        m_calculator.OnCalculationRejected += { this, [this]{
            Result->Value.InputValue = std::nullopt;
        }};
        Result->IsValid.ConnectFrom(CDL, m_calculator.Valid).MakeSafe(m_connections);
        Result->ConnectChain(cdl, SPCCO().SetProcessId(processId).AddCapturers(m_calculator.GetCapturer()), args...);
    }

    SharedPointerInitialized<ResultParameter> Result;

    StateCalculator<std::optional<value_type>>& GetCalculator() { return m_calculator; }
    Interruptor GetInterruptor() { return m_calculator.GetInterruptor(); }

protected:
    StateCalculator<std::optional<value_type>> m_calculator;
    DispatcherConnectionsSafe m_connections;
};

template<class Result>
class StateChainedComplexCalculator
{
public:
    template<typename FFunction, typename ... Args>
    void SetCalculator(const char* cdl, const Name& processId, const FFunction& handler, const Args&... args)
    {
        m_calculator.SetCalculatorWithParams(cdl, handler, args...);
        for(const auto& parameter : m_parameters) {
            parameter->ConnectChain(cdl, SPCCO().SetProcessId(processId).AddCapturers(m_calculator.GetCapturer()), args...);
        }
    }

    void Initialize()
    {
        Q_ASSERT(!m_initialized);
        m_calculator.OnCalculated += { this, [this](const auto&){
            for(const auto& parameter : m_parameters) {
                parameter->GetChainData()->EditErrors().Remove(nullptr);
            }
        }};
        m_calculator.OnExceptionCaught += { this, [this](const Exception& ex){
            for(const auto& parameter : m_parameters) {
                parameter->GetChainData()->EditErrors().Add(ex.GetError(), nullptr);
            }
        }};
        m_initialized = true;
    }

    template<typename FFunction, typename ... Args>
    void SetCalculator(const char* cdl, const FFunction& handler, const Args&... args)
    {
        Q_ASSERT(m_initialized);
        SetCalculator(cdl, Name(), handler, args...);
    }

    void UnsetCalculator(const char* cdl)
    {
        m_calculator.Disconnect(cdl);
        for(const auto& parameter : m_parameters) {
            parameter->GetChainData()->SetCaptureHandler([](StateParametersChainData&){});
        }
    }

    template<class ImmutableModel, class Model = typename ImmutableModel::model_type>
    void RegisterOutput(const char* cdl, const StateParametersContainerPtr<ImmutableModel>& parameter, StateDoubleBufferData<Model>& dbuffer, const std::function<const typename Model::container_type& (const Result&)>& getter)
    {
        Q_ASSERT(parameter->GetInputData() == dbuffer.GetImmutableData());
        m_calculator.OnCalculationRejected.Connect(CDL, [&dbuffer]{
            dbuffer.EditData()->Clear();
        });
        m_calculator.OnCalculated.Connect(cdl, [&dbuffer, cdl, getter](const Result& result){
            auto r = getter(result);
            dbuffer.EditData()->Set(r);
            dbuffer.EditData()->IsValid.SetState(true);
        });
        dbuffer.GetImmutableData()->Enabled.ConnectFrom(cdl, m_calculator.Enabled);
        dbuffer.EditData()->IsValid.ConnectFromStateProperty(CDL, m_calculator.Valid);
        m_parameters.append(parameter);
    }

    template<class DBuffer, class ImmutableModel, class Model = typename ImmutableModel::model_type>
    void RegisterOutput(const char* cdl, const StateParametersContainerPtr<ImmutableModel>& parameter, DBuffer& dbuffer, const std::function<void (const Result&)>& onDone, const FAction& onRejected)
    {
        Q_ASSERT(parameter->GetInputData() == dbuffer.GetImmutableData());
        m_calculator.OnCalculationRejected.Connect(CDL, onRejected);
        m_calculator.OnCalculated.Connect(cdl, onDone);
        dbuffer.GetImmutableData()->Enabled.ConnectFrom(cdl, m_calculator.Enabled);
        dbuffer.EditData()->IsValid.ConnectFromStateProperty(CDL, m_calculator.Valid);
        m_parameters.append(parameter);
    }

    template<class Model>
    void RegisterOutput(const char* cdl, SP<Model>& parameter, const std::function<const typename Model::container_type& (const Result&)>& getter)
    {
        RegisterOutput(cdl, [&parameter, getter](const Result& result){
            auto r = getter(result);
            parameter->Set(r);
            parameter->IsValid.SetState(true);
        },[&parameter]{
            parameter->IsValid.SetState(false);
        }, parameter->IsValid);
    }

    template<class T>
    void RegisterOutput(const char* cdl, const SP<T>& parameter, const std::function<void (T&, const Result&)>& onDone)
    {
        m_calculator.OnCalculated.Connect(CDL, [onDone, parameter](const Result& result) {
            onDone(*parameter, result);
        });
        parameter->IsValid.ConnectFromStateProperty(CDL, m_calculator.Valid);
        m_parameters.append(parameter);
    }

    void RegisterOutput(const char* cdl, const std::function<void (const Result&)>& onDone, const FAction& onRejected = []{})
    {
        m_calculator.OnCalculationRejected.Connect(CDL, onRejected);
        m_calculator.OnCalculated.Connect(CDL, onDone);
    }

    void RegisterOutput(const char* cdl, const std::function<void (const Result&)>& onDone, const FAction& onRejected, StateProperty& stateProperty)
    {
        m_calculator.OnCalculationRejected.Connect(CDL, onRejected);
        m_calculator.OnCalculated.Connect(CDL, onDone);
        stateProperty.ConnectFromStateProperty(CDL, m_calculator.Valid);
    }

    SmartPointerWatcherPtr Capture() { return m_calculator.Capture(); }
    Interruptor GetInterruptor() { return m_calculator.GetInterruptor(); }

private:
    StateCalculator<Result> m_calculator;
    QVector<SP<StateParameters>> m_parameters;
#ifdef QT_DEBUG
    bool m_initialized = false;
#endif
};

#endif // WRAPPERS_H
