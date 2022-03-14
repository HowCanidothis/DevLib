#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"

class StatePropertyBoolCommutator : public LocalProperty<bool>
{
    using Super = LocalProperty<bool>;
public:
    StatePropertyBoolCommutator(bool defaultState = false);

    void ClearProperties();
    void Update();

    DispatcherConnections AddProperties(const QVector<LocalProperty<bool>*>& properties);

    QString ToString() const;

private:
    bool value() const;

private:
    DelayedCallObject m_setTrue;
    bool m_defaultState;
    Dispatcher m_commutator;
    ThreadHandlerNoThreadCheck m_threadHandler;
    QVector<LocalProperty<bool>*> m_properties;
};

class StateProperty : public LocalPropertyBool
{
    using Super = LocalPropertyBool;
public:
    using Super::Super;

    void SetState(bool state);

    DispatcherConnections ConnectFromStateProperty(const StateProperty& property);
    DispatcherConnections ConnectFromDispatchers(const QVector<Dispatcher*>& dispatchers, qint32 delayMsecs);
};

class StateParameters
{
public:
    StateParameters();

    void Lock();
    void Unlock();

    LocalPropertyBool IsLocked;

private:
    std::atomic_int m_counter;
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
        params->IsLocked.OnChanged.ConnectAndCall(this, [locker, unlocker, params]{
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
class StateParameter : public StateParameterBase<T>
{
    using Super = StateParameterBase<T>;
public:
    using value_type = typename T::value_type;
    template<typename ... Args>
    StateParameter(StateParameters* params, Args ... args)
        : Super(params,
                [this]{ m_connection = m_immutableValue.ConnectFrom(Super::InputValue).MakeSafe(); },
                [this]{ m_connection = DispatcherConnectionSafePtr(); },
                args...)
    {

    }

    StateParameter& operator=(const value_type& value)
    {
        Super::InputValue = value;
        return *this;
    }

    const value_type& GetImmutable() const { return m_immutableValue; }

private:
    T m_immutableValue;
    DispatcherConnectionSafePtr m_connection;
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
    {
        m_onChanged.Subscribe({ &m_dependenciesAreUpToDate.OnChanged });
        Valid.ConnectFrom(m_dependenciesAreUpToDate, [this](bool valid){
            return !valid ? false : Valid.Native();
        });

        Enabled.OnChanged += {this, [this, recalculateOnEnabled]{
            THREAD_ASSERT_IS_MAIN();
            if(Enabled) {
                m_onChanged += { this, [this, recalculateOnEnabled]{
                    Valid.SetState(false);
#ifdef QT_DEBUG
                    if(!ObjectName.isEmpty()) {
                        qDebug() << "Is able to calculate" << ObjectName << m_dependenciesAreUpToDate << (m_dependenciesAreUpToDate ? "" : m_dependenciesAreUpToDate.ToString());
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

    void SetCalculatorBasedOnStateParameters(const typename ThreadCalculatorData<T>::Calculator& calculator)
    {
        m_preparator = [this]{
            for(auto* parameters : m_stateParameters) {
                parameters->Lock();
            }
        };
        m_releaser = [this]{
            for(auto* parameters : m_stateParameters) {
                parameters->Unlock();
            }
        };
        m_calculator = calculator;
    }

    template<class T2>
    const StateCalculator& Connect(const StateCalculator<T2>& calculator) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstCalculator = const_cast<StateCalculator<T2>&>(calculator);
        m_dependenciesAreUpToDate.AddProperties({ &nonConstCalculator.Valid }).MakeSafe(m_connections);
        return *this;
    }

    template<class T2>
    const StateCalculator& Connect(const StateParameterBase<T2>& stateParameter) const;

    template<class T2>
    const StateCalculator& Connect(const LocalProperty<T2>& property) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.Subscribe({ &nonConstProperty.OnChanged }).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(StateProperty& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.AddProperties({ &dispatcher }).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(Dispatcher& onChanged) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_onChanged.Subscribe({&onChanged}).MakeSafe(m_connections);
        return *this;
    }

    const StatePropertyBoolCommutator& GetDependenciesState() const { return m_dependenciesAreUpToDate; }

    mutable LocalPropertyBool Enabled;
    StateProperty Valid;
    Dispatcher OnCalculationRejected;
    QString ObjectName;

protected:
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
    mutable QSet<StateParameters*> m_stateParameters;
};

template<class T>
struct StateCalculatorConnectionHelper
{
    template<class T2>
    static void ConnectStateParameter(const StateCalculator<T2>& calculator, StateParameterBase<T>& parameter)
    {
        calculator.Connect(parameter.InputValue.OnChanged);
    }
};

template<class T> template<class T2>
const StateCalculator<T>& StateCalculator<T>::Connect(const StateParameterBase<T2>& stateParameter) const
{
    THREAD_ASSERT_IS_MAIN();
    StateCalculatorConnectionHelper<T2>::ConnectStateParameter<T>(*this, const_cast<StateParameterBase<T2>&>(stateParameter));
    m_stateParameters.insert(stateParameter.GetParameters());
    return *this;
}

template<class T>
class StateImmutableData {
    using TPtr = SharedPointer<T>;
public:
    class UnLocker : public guards::LambdaGuard
    {
        using Super = guards::LambdaGuard;
    public:
        UnLocker(const StateImmutableData* table) noexcept
            : Super([table]{ table->Unlock(); }, []{} )
        {}
    };

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
        m_data->IsValid.ConnectFromStateProperty(m_calculator.Valid);
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
            calculator.Connect(externalData->IsValid).Connect(externalData->OnChanged);
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
    StateDoubleBufferData(const TPtr& source, const TPtr& immutable)
        : m_immutableData(::make_shared<StateImmutableData<T>>(immutable))
        , m_data(source)
        , Enabled(m_immutableData->Enabled)
    {
        Enabled.OnChanged += { this, [this, source]{
            if(Enabled) {
                m_immutableData->AttachSwap(source);
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
        onDeleted->Connect(nullptr, [connections, updatedResult]{
            connections->clear();
            updatedResult.Resolve(false);
        }).MakeSafe(*connections);
    }
};

#endif // STATEPROPERTY_H
