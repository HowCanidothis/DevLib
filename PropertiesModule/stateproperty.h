#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"

class StateProperty : public LocalPropertyBool
{
    using Super = LocalPropertyBool;
public:
    using Super::Super;

    void SetState(bool state);

    DispatcherConnections ConnectFromStateProperty(const char* location, const StateProperty& property);
    DispatcherConnections ConnectFromDispatchers(const QVector<Dispatcher*>& dispatchers, qint32 delayMsecs);
    static DispatcherConnections PerformWhenEveryIsValid(const QVector<LocalPropertyBool*>& stateProperties, const FAction& handler, qint32 delayMsecs, bool once);
    static DispatcherConnections OnFirstInvokePerformWhenEveryIsValid(const QVector<LocalPropertyBool*>& stateProperties, const FAction& handler);
};

class StatePropertyBoolCommutator : public StateProperty
{
    using Super = StateProperty;
public:
    using FHandler = std::function<bool ()>;

    StatePropertyBoolCommutator(bool defaultState = false);

    void ClearProperties();
    void Update();

    DispatcherConnections AddProperties(const char* location, const QVector<LocalProperty<bool>*>& properties);
    DispatcherConnections AddProperty(const char* location, LocalProperty<bool>* property, bool inverted = false);
    DispatcherConnections AddHandler(const char* location, const FHandler& handler, const QVector<Dispatcher*>& dispatchers);

    QString ToString() const;

private:
    bool value() const;

private:
    DelayedCallObject m_setTrue;
    bool m_defaultState;
    Dispatcher m_commutator;
    ThreadHandlerNoThreadCheck m_threadHandler;
    QVector<FHandler> m_properties;
};

class StateParameters
{
public:
    StateParameters();

    void Lock();
    void Unlock();
    void Reset();

    DispatchersCommutator OnChanged;
    StatePropertyBoolCommutator IsValid;
    LocalPropertyBool IsLocked;

private:
    template<class T> friend class StateCalculator;
    std::atomic_int m_counter;
    LocalPropertyBool m_isValid;
};

template<class T>
class StateParameterBase
{
public:
    template<typename ... Args>
    StateParameterBase(StateParameters* params, const FAction& locker, const FAction& unlocker, Args ... args)
        : InputValue(args...)
        , m_parameters(params)
    {
        params->IsLocked.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [locker, unlocker, params]{
            if(!params->IsLocked) {
                unlocker();
            } else {
                locker();
            }
        });
    }

    StateParameters* GetParameters() const { return m_parameters; }

    T InputValue;

protected:
    StateParameters* m_parameters;
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
    {
        m_immutableValue.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue).MakeSafe(m_connections);
        Super::InputValue.ConnectAction(CONNECTION_DEBUG_LOCATION, [params]{
            if(params->IsLocked) {
                params->Reset();
            } else {
                params->OnChanged.Invoke();
            }
        });
    }

    StateParameterProperty& operator=(const value_type& value)
    {
        THREAD_ASSERT_IS_MAIN();
        Super::InputValue = value;
        return *this;
    }

    operator T&() { THREAD_ASSERT_IS_MAIN(); return Super::InputValue; }
    const T& GetImmutableProperty() const { return m_immutableValue; }
    const value_type& GetImmutable() const { return m_immutableValue; }

private:
    template<class T2> friend struct Serializer;
    template<class T2> friend struct SerializerXml;
    T m_immutableValue;
    DispatcherConnectionsSafe m_connections;
};

template<class T>
class StateParameterImmutableData : public StateParameterBase<LocalPropertySharedPtr<T>>
{
    using Super = StateParameterBase<LocalPropertySharedPtr<T>>;
public:
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
        Super::InputValue.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, params]{
            m_modelConnections.clear();
            if(Super::InputValue != nullptr) {
                m_modelIsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue->IsValid).MakeSafe(m_modelConnections);
            } else {
                m_modelIsValid = false;
            }
            params->OnChanged.Invoke();
        });

        params->IsValid.AddProperties(CONNECTION_DEBUG_LOCATION, { &m_modelIsValid });
    }

private:
    SharedPointer<T> m_lockedModel;
    LocalPropertyBool m_modelIsValid;
    DispatcherConnectionsSafe m_modelConnections;
};

template<class T>
class StateParametersContainer : public StateParameters
{
public:
    StateParametersContainer()
        : Parameter(this)
    {}

    StateParameterImmutableData<T> Parameter;
};
template<class T> using StateParametersContainerPtr = SharedPointer<StateParametersContainer<T>>;

inline uint qHash(const SharedPointer<StateParameters>& key, uint seed = 0) { return qHash(key.get(), seed); }

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
    {
        m_onChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_dependenciesAreUpToDate.OnChanged);
        Valid.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_dependenciesAreUpToDate, [this](bool valid){
            return !valid ? false : Valid.Native();
        });

        Enabled.OnChanged += {this, [this, recalculateOnEnabled]{
            THREAD_ASSERT_IS_MAIN();
            if(Enabled) {
                m_onDirectOnChanged += { this, [this]{
                    Cancel();
                }};
                m_onChanged += { this, [this, recalculateOnEnabled]{
                    Valid.SetState(false);
#ifdef QT_DEBUG
                    if(!ObjectName.isEmpty()) {
                        qDebug() << "Is able to calculate" << ObjectName << m_dependenciesAreUpToDate << (m_dependenciesAreUpToDate ? QString() : m_dependenciesAreUpToDate.ToString());
                    }
#endif
                    if(m_dependenciesAreUpToDate) {
                        Calculate(m_calculator, m_preparator, m_releaser);
                    } else {
                        OnCalculationRejected();
                    }
                }};
                if(recalculateOnEnabled) {
                    RequestRecalculate();
                }
            } else {
                m_onChanged -= this;
                m_onDirectOnChanged -= this;
            }
        }};
    }

    void RequestRecalculate() const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.Update();
    }

    void Disconnect()
    {
        THREAD_ASSERT_IS_MAIN();
        m_connections.clear();
        m_dependenciesAreUpToDate.ClearProperties();
        m_stateParameters.clear();
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
        SetCalculatorBasedOnStateParameters([=]{
            return handler(args...);
        });
        ConnectParameters(connectionInfo, args...);
    }

    void SetCalculatorBasedOnStateParameters(const typename ThreadCalculatorData<T>::Calculator& calculator)
    {
        m_preparator = [this]{
            for(const auto& parameters : m_stateParameters) {
                parameters->Lock();
            }
        };
        m_releaser = [this]{
            for(const auto& parameters : m_stateParameters) {
                parameters->Unlock();
            }

        };
        m_calculator = calculator;
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const StateCalculator<T2>& calculator) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstCalculator = const_cast<StateCalculator<T2>&>(calculator);
        m_onDirectOnChanged.ConnectFrom(connection, nonConstCalculator.Valid.OnChanged).MakeSafe(m_connections);
        m_dependenciesAreUpToDate.AddProperties(connection, { &nonConstCalculator.Valid }).MakeSafe(m_connections);
        return *this;
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const LocalProperty<T2>& property) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onDirectOnChanged.ConnectFrom(connection, nonConstProperty.OnChanged).MakeSafe(m_connections);
        m_onChanged.ConnectFrom(connection, nonConstProperty.OnChanged).MakeSafe(m_connections);
        return *this;
    }

    template<typename ... Args>
    const StateCalculator& ConnectParameters(const char* connection, Args... args) const
    {
        (Connect(connection, args), ...);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const SharedPointer<StateParameters>& params) const
    {
        Connect(connection, params->OnChanged);
        Connect(connection, params->m_isValid);
        Connect(connection, params->IsValid);
        m_stateParameters.insert(params);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const StateProperty& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_onDirectOnChanged.ConnectFrom(connection, dispatcher.OnChanged).MakeSafe(m_connections);
        m_dependenciesAreUpToDate.AddProperties(connection, { const_cast<StateProperty*>(&dispatcher) }).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const Dispatcher& onChanged) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_onDirectOnChanged.ConnectFrom(connection, onChanged).MakeSafe(m_connections);
        m_onChanged.ConnectFrom(connection, onChanged).MakeSafe(m_connections);
        return *this;
    }

    const StatePropertyBoolCommutator& GetDependenciesState() const { return m_dependenciesAreUpToDate; }

    mutable LocalPropertyBool Enabled;
    StateProperty Valid;
    Dispatcher OnCalculationRejected;
    QString ObjectName;

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
    mutable DispatchersCommutator m_onChanged;
    mutable Dispatcher m_onDirectOnChanged;
    mutable QSet<SharedPointer<StateParameters>> m_stateParameters;
};

template<class T>
class StateImmutableData {
    using TPtr = SharedPointer<T>;
public:
    StateImmutableData(const TPtr& data)
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

        m_calculator.OnCalculationRejected += { this, [this]{
            if(m_lockCounter == 0) {
    #ifndef QT_NO_DEBUG
                guards::LambdaGuard guard([this]{ m_internalEditing = false; }, [this]{ m_internalEditing = true; });
    #endif
                m_data->Clear();
            }
        }};

        m_data = data;
#ifndef QT_NO_DEBUG
        m_data->OnChanged += { this, [this]{
            Q_ASSERT(m_internalEditing);
        }};
#endif
        m_data->IsValid.ConnectFromStateProperty(CONNECTION_DEBUG_LOCATION, m_calculator.Valid);
    }

    guards::LambdaGuardPtr CreateLocker()
    {
        THREAD_ASSERT_IS_NOT_MAIN()
        return ::make_shared<guards::LambdaGuard>([this]{
            ThreadsBase::DoMainAwait([this]{ Unlock(); });
        }, [this]{
            ThreadsBase::DoMainAwait([this]{ Lock(); });
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
            ThreadsBase::DoMain([this]{
                m_calculator.RequestRecalculate();
            });
        }
        Q_ASSERT(m_lockCounter >= 0);
    }

    void AttachSwap(const TPtr& data)
    {
        AttachCopy(data, [data]{
            return data;
        });
    }

    void AttachCopy(const TPtr& externalData, const std::function<void (StateCalculator<bool>&)>& connectorHandler, const std::function<TPtr ()>& handler = nullptr)
    {
        m_calculator.Disconnect();

        if(externalData != nullptr) {
            m_handler = handler == nullptr ? [externalData]{ return externalData->Clone(); } : handler;
            connectorHandler(m_calculator);
        } else {
            m_handler = nullptr;
        }

        m_calculator.SetCalculator([]{
            return true;
        });

        m_calculator.RequestRecalculate();
    }

    void AttachCopy(const TPtr& externalData, const std::function<TPtr ()>& handler = nullptr)
    {
        AttachCopy(externalData, [externalData](StateCalculator<bool>& calculator){
            calculator.Connect(CONNECTION_DEBUG_LOCATION, externalData->IsValid)
                    .Connect(CONNECTION_DEBUG_LOCATION, externalData->OnChanged);
        }, handler);
    }

    const TPtr& GetData() const { return m_data; }

protected:
    TPtr m_data;
    std::function<TPtr ()> m_handler;
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
    }

    const TPtr& EditData() { return m_data; }
    const TPtr& GetData() const { return m_data; }
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
    static AsyncResult GenerateUpdateResult(const StateProperty* valid, Dispatcher* onDeleted)
    {
        AsyncResult updatedResult;
        auto* nonConst = const_cast<StateProperty*>(valid);
        if(*valid) {
            nonConst->OnChanged.OnFirstInvoke([updatedResult, valid, onDeleted]{
                generateUpdateResult(valid, onDeleted, updatedResult);
            });
        } else {
            generateUpdateResult(valid, onDeleted, updatedResult);
        }
        return updatedResult;
    }

private:
    static void generateUpdateResult(const StateProperty* valid, Dispatcher* onDeleted, const AsyncResult& updatedResult)
    {
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        auto* nonConst = const_cast<StateProperty*>(valid);
        nonConst->OnChanged.OnFirstInvoke([updatedResult, connections]{
            connections->clear();
            updatedResult.Resolve(true);
        });
        onDeleted->Connect(CONNECTION_DEBUG_LOCATION, [connections, updatedResult]{
            connections->clear();
            updatedResult.Resolve(false);
        }).MakeSafe(*connections);
    }
};

template<typename... Dispatchers>
SharedPointer<StateProperty> StatePropertyCreate(const char* connection, const std::function<bool ()>& handler, Dispatcher& dispatcher, Dispatchers&... dispatchers)
{
    auto result = ::make_shared<WithDispatchersConnectionsSafe<StateProperty>>();
    result->ConnectFrom(connection, handler, dispatchers...).MakeSafe(result->Connections);
    return result;
}


#endif // STATEPROPERTY_H
