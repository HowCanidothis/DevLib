#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"

template<class T> class StateCalculator;

class StateProperty : public LocalPropertyBool
{
    using Super = LocalPropertyBool;
public:
    using Super::Super;

    void SetState(bool state);

    DispatcherConnections ConnectFromStateProperty(const char* location, const StateProperty& property);
    template<typename ... Properties>
    DispatcherConnections ConnectFromStateProperty(const char* location, const LocalPropertyBool& property, const Properties&... props)
    {
        DispatcherConnections result;
        auto update = [this, &property, &props...] {
            bool valid = true;
            for(bool prop : {property.Native(), props.Native()...}) {
                if(!prop) {
                    valid = false;
                    break;
                }
            }
            SetState(valid);
        };
        adapters::Combine([&](const auto& property){
            result += property.OnChanged.Connect(location, update);
        }, property, props...);
        update();
        return result;
    }

    template<typename ... Args>
    static DispatcherConnection OnFirstInvokePerformWhenEveryIsValid(const char* location, const FAction& handler, const Args&... stateProperties)
    {
        auto commutator = ::make_shared<WithDispatcherConnectionsSafe<LocalPropertyBoolCommutator>>(LocalPropertyBoolCommutator::And);
        commutator->ConnectFrom(location, stateProperties...).MakeSafe(commutator->Connections);

        return commutator->AsProperty().OnChanged.OnFirstInvoke([location, handler, commutator]{
            handler();
            ThreadsBase::DoMain(location, [commutator]{}); // Safe deletion
        });
    }
};

class StatePropertyBoolCommutator : public LocalPropertyBoolCommutatorBase<DispatchersCommutatorWithDirect>
{
    using Super = LocalPropertyBoolCommutatorBase<DispatchersCommutatorWithDirect>;
public:
    StatePropertyBoolCommutator(bool defaultState = true);

    void Update();
};

class StateParameters
{
public:
    StateParameters(bool valid = true);

    virtual void Initialize() final;
    bool IsInitialized() { return m_initializer == nullptr; }

    void Lock();
    void Unlock();
    void Reset();

    DispatchersCommutator OnChanged;
    StatePropertyBoolCommutator IsValid;
    LocalPropertyBool IsLocked;
    DispatcherConnectionsSafe Connections;

    virtual SmartPointerWatcherPtr Capture() { return nullptr; }

protected:
    virtual void onInitialized() {}

private:
    QVector<class IStateParameterBase*> m_parameters;

private:
    friend class IStateParameterBase;
    template<class T> friend class StateCalculator;
    std::atomic_int m_counter;
    LocalPropertyBool m_isValid;
    FAction m_initializer;
};

class CapturedStateParameters : public StateParameters
{
    using Super = StateParameters;
public:
    CapturedStateParameters(bool valid = true);

    LocalPropertyBool IsActive;
    SmartPointerWatcherPtr Capture() override { return m_used.Capture(); }
private:
    SmartPointer m_used;
};

class IStateParameterBase
{
public:
    IStateParameterBase(StateParameters* params);

    virtual bool IsInitialized() const = 0;

protected:
    friend class StateParameters;
    virtual void initialize(){}
};

template<class T>
class StateParameterBase : public IStateParameterBase
{
    using Super = IStateParameterBase;
public:
    template<typename ... Args>
    StateParameterBase(StateParameters* params, const FAction& locker, const FAction& unlocker, Args ... args)
        : Super(params)
        , InputValue(args...)
        , m_parameters(params)
    {
        params->IsLocked.Connect(CONNECTION_DEBUG_LOCATION, [locker, unlocker](bool locked){
            if(!locked) {
                unlocker();
            } else {
                locker();
            }
        });
        adapters::ResetThread(params->IsLocked);
    }

    bool IsInitialized() const override { return m_initializer == nullptr; }
    StateParameters* GetParameters() const { return m_parameters; }

    T InputValue;

private:
    void initialize() override final
    {
        Q_ASSERT(m_initializer != nullptr);
        m_initializer();
        m_initializer = nullptr;
    }

protected:
    StateParameters* m_parameters;
    FAction m_initializer;
};

template<class T>
class StateParameterProperty : public StateParameterBase<T>
{
    using Super = StateParameterBase<T>;
public:
    using value_type = typename T::value_type;
    template<typename ... Args>
    StateParameterProperty(StateParameters* params, Args ... args)
        : Super(params,
                [this]{ m_connections.clear(); },
                [this]{ m_immutableValue.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue).MakeSafe(m_connections); },
                args...)
        , m_setter([this](const value_type& value){
                    Q_ASSERT(!Super::m_parameters->IsLocked);
                    Super::InputValue.SetSilent(value);
                })
    {
        Super::m_initializer = [this]{
            m_setter = [this](const value_type& value){
                THREAD_ASSERT_IS_MAIN();
                Super::InputValue = value;
            };

            m_immutableValue.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue).MakeSafe(m_connections);
            auto handler = [this]{
                if(Super::m_parameters->IsLocked) {
                    Super::m_parameters->Reset();
                } else {
                    Super::m_parameters->OnChanged.Invoke();
                }
            };
            Super::InputValue.ConnectAction(CONNECTION_DEBUG_LOCATION, handler);
            handler();
        };
    }

    StateParameterProperty& operator=(const value_type& value)
    {
        m_setter(value);
        return *this;
    }

    operator T&() { THREAD_ASSERT_IS_MAIN(); return Super::InputValue; }
    const T& GetImmutableProperty() const { return m_immutableValue; }
    auto GetImmutable() const { return m_immutableValue.Native(); }
    template<typename T2>
    T2 GetImmutable(const T2& v) const { return m_immutableValue.ValueOr(v); }

private:
    template<class T2> friend struct Serializer;
    template<class T2> friend struct SerializerXml;
    T m_immutableValue;
    DispatcherConnectionsSafe m_connections;
    std::function<void (const value_type&)> m_setter;
};

template<class T> class StateParametersContainer;

template<class T>
class StateParameterImmutableData : public StateParameterBase<LocalPropertySharedPtr<T>>
{
    using Super = StateParameterBase<LocalPropertySharedPtr<T>>;
public:
    using TPtr = T;
    StateParameterImmutableData(StateParameters* params)
        : Super(params, [this]{
            if(Super::InputValue != nullptr) {
                m_lockedModel = Super::InputValue;
                m_lockedModel->Lock();
            }
        }, [this]{
            if(m_lockedModel != nullptr) {
                m_lockedModel->Unlock();
            }
        })
        , m_modelIsValid(false)
    {
        Super::m_initializer = [this]{
            Super::InputValue.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
                m_modelConnections.clear();
                if(Super::InputValue != nullptr) {
                    m_modelIsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue->IsValid).MakeSafe(m_modelConnections);
                } else {
                    m_modelIsValid = false;
                }
                Super::m_parameters->OnChanged.Invoke();
            });

            Super::m_parameters->IsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_modelIsValid);
        };
    }

private:
    friend class StateParametersContainer<T>;
    SharedPointer<T> m_lockedModel;
    LocalPropertyBool m_modelIsValid;
    DispatcherConnectionsSafe m_modelConnections;
};

enum InitializationWithLock {
    Lock
};

template<class T>
class StateParametersContainer : public StateParameters
{
    using Super = StateParameters;
public:
    using container_type = typename T::container_type;
    using TPtr = SharedPointer<T>;
    StateParametersContainer()
        : Super(false)
        , m_parameter(this)
    {
    }
    StateParametersContainer(const TPtr& initial)
        : Super(false)
        , m_parameter(this, initial)
    {
    }
    StateParametersContainer(const TPtr& initial, InitializationWithLock)
        : Super(false)
        , m_parameter(this)
    {
        SetLockedParameter(initial);
    }

    bool HasValue() const
    {
        return !IsNull();
    }

    bool IsNull() const
    {
        return m_parameter.InputValue == nullptr;
    }

    template<class T2>
    void SetParameter(const T2& data)
    {
        m_parameter.InputValue = data;
        adapters::ResetThread(GetProperty());
    }

    template<class T2>
    void SetLockedParameter(const T2& data)
    {
#ifdef QT_DEBUG
        m_lockConnections.clear();
#endif
        SetParameter(data);
#ifdef QT_DEBUG
        GetProperty().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, []{
            Q_ASSERT(false);
        }).MakeSafe(m_lockConnections);
        adapters::ResetThread(GetProperty());
#endif
    }

    const TPtr& GetLockedImmutableData() const
    {
        THREAD_ASSERT_IS_NOT_MAIN()
        return m_parameter.m_lockedModel;
    }

    const container_type& GetLockedImmutable() const
    {
        return GetLockedImmutableData()->GetData()->Native();
    }

    template<typename ... Dispatchers>
    DispatcherConnections AddValidatorOnChanged(const char* connectionInfo, const std::function<bool (const T& value)>& validator, const Dispatchers&... args)
    {
        auto dispatcher = GetProperty().DispatcherParamsOnChanged(CONNECTION_DEBUG_LOCATION).CreateDispatcher();
        return IsValid.ConnectFromDispatchers(connectionInfo, [this, validator, dispatcher]{
            if(GetProperty() == nullptr) {
                return false;
            }
            return validator(*GetProperty());
        }, *dispatcher, args...);
    }

    template<typename ... Dispatchers>
    DispatcherConnections AddValidator(const char* connectionInfo, const std::function<bool (const T& value)>& validator, const Dispatchers&... args)
    {
        return IsValid.ConnectFromDispatchers(connectionInfo, [this, validator]{
            if(GetProperty() == nullptr) {
                return false;
            }
            return validator(*GetProperty());
        }, args...);
    }

    LocalPropertySharedPtr<T>& GetProperty()
    {
        return m_parameter.InputValue;
    }

    const TPtr& GetInputData() const
    {
        THREAD_ASSERT_IS_MAIN()
        return m_parameter.InputValue;
    }

    const container_type& GetInput() const
    {
        return GetInputData()->GetData()->Native();
    }

    SmartPointerWatcherPtr Capture() { return m_captureHandler != nullptr ? m_captureHandler() : nullptr; }
    void SetCaptureHandler(const std::function<SmartPointerWatcherPtr()>& handler){ m_captureHandler = handler; }
private:
    std::function<SmartPointerWatcherPtr()> m_captureHandler;
    StateParameterImmutableData<T> m_parameter;
#ifdef QT_DEBUG
    DispatcherConnectionsSafe m_lockConnections;
#endif
};

template<class T> using StateParametersContainerPtr = SharedPointer<StateParametersContainer<T>>;
template<class T> using StateParametersContainerPtrInitialized = SharedPointerInitialized<StateParametersContainer<T>>;

inline uint qHash(const SharedPointer<StateParameters>& key, uint seed = 0) { return qHash(key.get(), seed); }

template<class T>
class StateCalculatorSwitcher
{
    template<class T2> friend class StateCalculator;
    StateCalculatorSwitcher(T* calculator)
        : m_pointer([calculator]{
            calculator->Enabled = true;
        }, [calculator]{
            calculator->Enabled = false;
        })
    {
        calculator->SetRecalculateOnEnabled(true);
    }
public:
    SmartPointerWatcherPtr Capture() { return m_pointer.Capture(); }

private:
    SmartPointer m_pointer;
};

template<class T>
class StateCalculator : public ThreadCalculator<T>
{
    using Super = ThreadCalculator<T>;
public:
    StateCalculator(bool recalculateOnEnabled = false)
        : Super(ThreadHandlerMain)
        , Enabled(false)
        , Valid(false)
        , m_dependenciesAreUpToDate(true)
        , m_recalculateOnEnabled(recalculateOnEnabled)
    {
        m_onChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_dependenciesAreUpToDate);

        Enabled.OnChanged += {this, [this]{
            THREAD_ASSERT_IS_MAIN();
            if(Enabled) {
                m_onChanged.OnDirectChanged += { this, [this]{
                    if(!Valid) {
                        return;
                    }
                    Q_ASSERT(m_calculator && m_preparator && m_releaser);
                    Valid.SetState(false);
                    Super::Cancel();
                }};
                m_onChanged += { this, [this]{
                    Valid.SetState(false);

                    DEBUG_PRINT_INFO_ACTION(this,
                        qDebug() << m_dependenciesAreUpToDate << (m_dependenciesAreUpToDate ? QString() : m_dependenciesAreUpToDate.ToString());
                    );

                    if(m_dependenciesAreUpToDate) {
                        Calculate(m_calculator, m_preparator, m_releaser);
                    } else {
                        OnCalculationRejected();
                    }
                }};
                if(m_recalculateOnEnabled) {
                    RequestRecalculate();
                }
            } else {
                m_onChanged -= this;
                m_onChanged.OnDirectChanged -= this;
                Valid.SetState(false);
            }
        }};

        adapters::ResetThread(m_dependenciesAreUpToDate.AsProperty(), Enabled);
    }

    SmartPointerWatcherPtr Capture()
    {
        if(m_switcher == nullptr) {
            m_switcher = new StateCalculatorSwitcher<StateCalculator>(this);
        }
        return m_switcher->Capture();
    }

    StateCalculator& SetRecalculateOnEnabled(bool recalculate)
    {
        m_recalculateOnEnabled = recalculate;
        return *this;
    }

    void RequestRecalculate() const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.Update();
    }

    void Cancel()
    {
        m_onChanged.OnDirectChanged();
    }

    void Disconnect(bool cancel = false)
    {
        THREAD_ASSERT_IS_MAIN();
        m_connections.clear();
        m_dependenciesAreUpToDate.Reset(false);
        m_stateParameters.clear();
        if(cancel) {
            Cancel();
        }
    }

    void SetCalculator(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                       const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        m_preparator = preparator;
        m_calculator = calculator;
        m_releaser = releaser;
    }

    template<typename ... Args, typename Function>
    void SetCalculatorWithParams(const char* connectionInfo, const Function& handler, Args... args)
    {
        Q_ASSERT(m_stateParameters.isEmpty());
        ConnectCombined(connectionInfo, args...);
        SetCalculatorBasedOnStateParameters([=]{
            return handler(args...);
        });
    }

    template<typename ... Args, typename Function>
    void SetCalculatorWithCaptureParams(const char* connectionInfo, const Function& handler, Args... args){
        SetCalculatorWithParams(connectionInfo, handler, args...);

        auto container = SmartPointerWatchersCreate();
        Enabled.ConnectAndCall(connectionInfo, [=](bool enable){
            if(!enable){
                container->clear();
                return ;
            }
            ([](SmartPointerWatchers& container, StateParameters* p){ container.append(p->Capture()); }(*container, args.get()), ...);
        });
    }

    void SetCalculatorBasedOnStateParameters(const typename ThreadCalculatorData<T>::Calculator& calculator)
    {
        auto params = m_stateParameters;
        m_preparator = [params]{
            for(const auto& parameters : params) {
                parameters->Lock();
            }
        };
        m_releaser = [params]{
            for(const auto& parameters : params) {
                parameters->Unlock();
            }

        };
        m_calculator = calculator;
    }

    template<class DoubleBuffer>
    void InitializeByDoubleBuffer(const char* connectionInfo, DoubleBuffer& buffer, bool enable = true)
    {
        OnCalculationRejected += { this, [&buffer]{
            buffer.EditData()->Clear();
        }};
        Super::OnCalculated += { this, [&buffer](const auto& container){
            buffer.EditData()->Set(container);
            buffer.EditData()->IsValid.SetState(true);
        }};
        buffer.EditData()->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        Enabled = enable;
        buffer.Enabled = true;
    }

    template<class DoubleBuffer>
    void ConnectDoubleBuffer(const char* connectionInfo, DoubleBuffer& buffer)
    {
        OnCalculationRejected.Connect(connectionInfo, [&buffer]{
            buffer.EditData()->Clear();
        });
        buffer.EditData()->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        buffer.Enabled = true;
    }

    template<class Buffer>
    void InitializeByBuffer(const char* connectionInfo, Buffer& buffer, bool enable = true)
    {
//        OnCalculationRejected += { this, [&buffer]{
//            buffer->Clear();
//        }};
        Super::OnCalculated += { this, [&buffer](const auto& container){
            buffer->Set(container);
            buffer->IsValid.SetState(true);
        }};
        buffer->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        Enabled = enable;
    }

    template<class Buffer>
    void ConnectBuffer(const char* connectionInfo, Buffer& buffer)
    {
//        OnCalculationRejected.Connect(connectionInfo, [&buffer]{
//            buffer->Clear();
//        });
        buffer->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const StateCalculator<T2>& calculator) const
    {
        THREAD_ASSERT_IS_MAIN();
        return Connect(connection, calculator.Valid);
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const LocalProperty<T2>& property) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.ConnectFrom(connection, nonConstProperty.OnChanged).MakeSafe(m_connections);
        return *this;
    }

    template<typename ... Args>
    const StateCalculator& ConnectCombined(const char* connection, const Args&... args) const
    {
        (Connect(connection, args), ...);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const SharedPointer<StateParameters>& params) const
    {
        if(!params->IsInitialized()) {
            params->Initialize();
        }
        Connect(connection, params->OnChanged);
        Connect(connection, params->m_isValid);
        Connect(connection, params->IsValid);
        m_stateParameters.insert(params);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const StatePropertyBoolCommutator& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.ConnectFrom(connection, dispatcher).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const StateProperty& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.ConnectFrom(connection, dispatcher).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const Dispatcher& onChanged) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_onChanged.ConnectFrom(connection, onChanged).MakeSafe(m_connections);
        return *this;
    }

    const StatePropertyBoolCommutator& GetDependenciesState() const { return m_dependenciesAreUpToDate; }

    mutable LocalPropertyBool Enabled;
    StateProperty Valid;
    Dispatcher OnCalculationRejected;

protected:
    void onPreRecalculate() override
    {
        OnCalculationRejected();
    }
    void onPostRecalculate() override
    {
        Valid.SetState(true);
    }
    bool acceptResult() override
    {
        return m_dependenciesAreUpToDate;
    }

private:
    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                   const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        Super::Calculate(calculator, preparator, releaser);
    }

private:
    typename ThreadCalculatorData<T>::Calculator m_calculator;
    typename ThreadCalculatorData<T>::Preparator m_preparator;
    typename ThreadCalculatorData<T>::Releaser m_releaser;
    mutable StatePropertyBoolCommutator m_dependenciesAreUpToDate;
    mutable DispatcherConnectionsSafe m_connections;
    mutable DispatchersCommutatorWithDirect m_onChanged;
    mutable QSet<SharedPointer<StateParameters>> m_stateParameters;
    bool m_recalculateOnEnabled;
    ScopedPointer<StateCalculatorSwitcher<StateCalculator>> m_switcher;
};

template<class T>
class StateImmutableData {
    using TPtr = SharedPointer<T>;
public:
    using container_type = typename T::container_type;
    using FHandler = std::function<container_type ()>;
    StateImmutableData(const TPtr& data = ::make_shared<T>())
        : m_lockCounter(0)
        , m_isDirty(false)
#ifndef QT_NO_DEBUG
        , m_internalEditing(false)
#endif
        , Enabled(m_calculator.Enabled)
        , IsValid(data->IsValid)
    {
        m_calculator.OnCalculated += { this, [this](bool){
            if(m_lockCounter != 0) {
                m_isDirty = true;
                if(m_lockCounter == 0) {
                    m_calculator.RequestRecalculate();
                }
                return;
            }

#ifndef QT_NO_DEBUG
        guards::LambdaGuard guard([this]{ m_internalEditing = false; }, [this]{ m_internalEditing = true; });
#endif
            if(m_handler != nullptr) {
                auto data = m_handler();
                m_data->Swap(data);
            } else {
                m_data->Clear();
            }
            m_data->IsValid.SetState(true);
        }};

//        m_calculator.OnCalculationRejected += { this, [this]{
//            if(m_lockCounter == 0) {
//    #ifndef QT_NO_DEBUG
//                guards::LambdaGuard guard([this]{ m_internalEditing = false; }, [this]{ m_internalEditing = true; });
//    #endif
//                m_data->Clear();
//            }
//        }};

        m_data = data;
#ifndef QT_NO_DEBUG
        m_data->OnChanged += { this, [this]{
            Q_ASSERT(m_internalEditing);
        }};
#endif
        m_data->IsValid.ConnectFromStateProperty(CONNECTION_DEBUG_LOCATION, m_calculator.Valid);

        adapters::SetThreadSafe(m_data->IsValid, m_data->OnChanged);
        adapters::ResetThread(m_calculator.Valid, m_calculator.OnCalculated);
    }

    guards::LambdaGuardPtr CreateLocker()
    {
        THREAD_ASSERT_IS_NOT_MAIN()
        return ::make_shared<guards::LambdaGuard>([this]{
            ThreadsBase::DoMainAwait(CONNECTION_DEBUG_LOCATION,[this]{ Unlock(); });
        }, [this]{
            ThreadsBase::DoMainAwait(CONNECTION_DEBUG_LOCATION,[this]{ Lock(); });
        });
    }

    void Lock() const
    {
        THREAD_ASSERT_IS_MAIN();
        ++m_lockCounter;
        Q_ASSERT(m_lockCounter >= 0);
    }

    void Unlock() const
    {
        --m_lockCounter;
        if(m_lockCounter == 0 && m_isDirty) {
            m_isDirty = false;
            m_calculator.RequestRecalculate();
        }
        Q_ASSERT(m_lockCounter >= 0);
    }

    void AttachSwap(const TPtr& data)
    {
        AttachSource(data, [data]{
            container_type result;
            result.swap(const_cast<container_type&>(data->Native()));
            return result;
        });
    }

    void AttachCopy(const TPtr& externalData)
    {
        if(externalData == nullptr) {
            AttachSource(nullptr);
            return;
        }
        AttachSource<T>(externalData, [externalData]{ return externalData->Clone(); });
    }

    void AttachSource(const std::function<void (StateCalculator<bool>&)>& connectorHandler, const FHandler& handler = nullptr)
    {
        m_calculator.Disconnect();
//        Q_ASSERT(handler != nullptr);
        m_handler = handler;
        if(m_handler){
            connectorHandler(m_calculator);
        }

        m_calculator.SetCalculator([]{
            return true;
        });

        m_calculator.RequestRecalculate();
    }

    template<class ExternalData>
    void AttachSource(const SharedPointer<ExternalData>& externalData, const FHandler& handler)
    {
        if(externalData == nullptr) {
            AttachSource(nullptr);
            return;
        }

        AttachSource([externalData](StateCalculator<bool>& calculator){
            calculator.Connect(CONNECTION_DEBUG_LOCATION, externalData->IsValid)
                    .Connect(CONNECTION_DEBUG_LOCATION, externalData->OnChanged);
        }, handler);
    }

    void Disconnect() {
        m_calculator.Disconnect(true);
    }

    const TPtr& GetData() const { return m_data; }
    qint32 GetLockCounter() const { return m_lockCounter; }

protected:
    TPtr m_data;
    FHandler m_handler;
    StateCalculator<bool> m_calculator;
    mutable std::atomic_int m_lockCounter;
    mutable std::atomic_bool m_isDirty;
#ifndef QT_NO_DEBUG
    bool m_internalEditing;
#endif

public:
    LocalPropertyBool& Enabled;
    StateProperty& IsValid;
};

template<class T> using StateParametersImmutableData = StateParametersContainer<StateImmutableData<T>>;
template<class T> using StateImmutableDataPtr = SharedPointer<StateImmutableData<T>>;

template<class T>
struct DefaultImmutableData
{
    static const StateImmutableDataPtr<T> Value;
};

#define IMPLEMENT_DEFAULT_IMMUTABLE_DATA(T) \
const StateImmutableDataPtr<T> DefaultImmutableData<T>::Value = []{ \
        auto result = ::make_shared<StateImmutableData<T>>(::make_shared<T>()); \
        result->IsValid.SetState(true); \
        return result; \
}();

template<class T, class T2, typename TPtr = SharedPointer<T>>
SharedPointer<StateParametersImmutableData<T>> StateParametersImmutableDataCreate(const SharedPointer<T2>& source, const std::function<void (StateCalculator<bool>&)>& connectorHandler, const typename StateImmutableData<T>::FHandler& handler = nullptr)
{
    auto result = ::make_shared<StateParametersImmutableData<T>>();
    result->InputValue = ::make_shared<T>();
    result->InputValue->AttachSource(source, connectorHandler, handler);
    result->InputValue = true;
    return result;
}

template<class T, class T2, typename TPtr = SharedPointer<T>>
SharedPointer<StateParametersImmutableData<T>> StateParametersImmutableDataCreate(const SharedPointer<T2>& source, const typename StateImmutableData<T>::FHandler& handler = nullptr)
{
    auto result = ::make_shared<StateParametersImmutableData<T>>();
    result->InputValue = ::make_shared<T>();
    result->InputValue->AttachSource(source, handler);
    result->InputValue = true;
    return result;
}

template<class T>
class IStateImmutableData
{
public:
    using TPtr = SharedPointer<T>;

    virtual void Swap(const TPtr& data) = 0;
    virtual TPtr Clone() const = 0;
    virtual void Clear() = 0;

    StateProperty IsValid;
    Dispatcher OnChanged;
};

template<class T>
class StateDoubleBufferData
{
public:
    using TPtr = SharedPointer<T>;
    StateDoubleBufferData(bool copy = false)
        : StateDoubleBufferData(::make_shared<T>(), ::make_shared<T>(), copy)
    {}

    using TPtr = SharedPointer<T>;
    StateDoubleBufferData(const TPtr& source, const TPtr& immutable, bool copy = false)
        : m_immutableData(::make_shared<StateImmutableData<T>>(immutable))
        , m_data(source)
        , Enabled(m_immutableData->Enabled)
    {
        Enabled.OnChanged += { this, [this, source, copy]{
            if(Enabled) {
                if(copy) {
                    m_immutableData->AttachCopy(source);
                } else {
                    m_immutableData->AttachSwap(source);
                }
            } else {
                m_immutableData->AttachCopy(nullptr);
            }
        }};
        adapters::ResetThread(Enabled);
    }

    void Reset()
    {
        m_data->IsValid.SetState(false);
    }

    void Set(const typename T::container_type& data)
    {
        m_data->Set(data);
        m_data->IsValid.SetState(true);
    }

    const TPtr& EditData() { return m_data; }
    const TPtr& EditData() const { return m_data; }
    const StateImmutableDataPtr<T>& GetImmutableData() const { return m_immutableData; }

private:
    StateImmutableDataPtr<T> m_immutableData;
    TPtr m_data;

public:
    LocalPropertyBool& Enabled;
};

class StateUpdateObjectResult
{
public:
    static AsyncResult GenerateUpdateResult(const char* location, const StateProperty* valid, Dispatcher* onDeleted)
    {
        AsyncResult updatedResult;
        auto* nonConst = const_cast<StateProperty*>(valid);
        if(*valid) {
            nonConst->OnChanged.OnFirstInvoke([location, updatedResult, valid, onDeleted]{
                generateUpdateResult(location, valid, onDeleted, updatedResult);
            });
        } else {
            generateUpdateResult(location, valid, onDeleted, updatedResult);
        }
        return updatedResult;
    }

private:
    static void generateUpdateResult(const char* location, const StateProperty* valid, Dispatcher* onDeleted, const AsyncResult& updatedResult)
    {
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        auto* nonConst = const_cast<StateProperty*>(valid);
        nonConst->OnChanged.OnFirstInvoke([updatedResult, connections]{
            connections->clear();
            updatedResult.Resolve(true);
        });
        onDeleted->Connect(location, [connections, updatedResult]{
            connections->clear();
            updatedResult.Resolve(false);
        }).MakeSafe(*connections);
    }
};

template<typename... Dispatchers>
SharedPointer<StateProperty> StatePropertyCreateFromDispatchers(const char* connection, const std::function<bool ()>& handler, Dispatcher& dispatcher, Dispatchers&... dispatchers)
{
    auto result = ::make_shared<WithDispatcherConnectionsSafe<StateProperty>>();
    result->ConnectFromDispatchers(connection, handler, dispatcher, dispatchers...).MakeSafe(result->Connections);
    return result;
}

template<typename... Props, typename FHandler>
SharedPointer<StateProperty> StatePropertyCreate(const char* connection, const FHandler& handler, Props&... properties)
{
    auto result = ::make_shared<WithDispatcherConnectionsSafe<StateProperty>>();
    result->ConnectFrom(connection, handler, properties...).MakeSafe(result->Connections);
    return result;
}


#endif // STATEPROPERTY_H
