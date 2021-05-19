#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"

class StatePropertyBoolCommutator : public LocalProperty<bool>
{
    using Super = LocalProperty<bool>;
public:
    StatePropertyBoolCommutator(bool defaultState = false)
        : Super(defaultState)
        , m_defaultState(defaultState)
    {
        m_commutator += { this, [this]{
            if(value()) {
                m_setTrue.Call([this]{
                    SetValue(value());
                });
            } else {
                SetValue(false);
            }
        }};
    }

    void ClearProperties()
    {
        m_properties.clear();
    }

    void Update()
    {
        m_commutator.Invoke();
    }

    DispatcherConnections AddProperties(const QVector<LocalProperty<bool>*>& properties)
    {
        DispatcherConnections result;
        for(auto* property : properties) {
            if(*property == !m_defaultState) {
                SetValue(false);
            }
            result += m_commutator.ConnectFrom(property->OnChange);
        }
        m_properties += properties;
        return result;
    }

private:
    bool value() const
    {
        bool result = m_defaultState;
        bool oppositeState = !result;
        for(auto* property : m_properties) {
            if(*property == oppositeState) {
                result = oppositeState;
                break;
            }
        }
        return result;
    }

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

    void SetState(bool state)
    {
        Super::SetValue(state);
    }

    DispatcherConnections ConnectFromStateProperty(const StateProperty& property)
    {
        return Super::ConnectFrom(property, [this](bool valid) { return valid ? Super::m_value : valid; });
    }
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
        m_onChanged.Subscribe({ &m_dependenciesAreUpToDate.OnChange });
        Valid.ConnectFrom(m_dependenciesAreUpToDate, [this](bool valid){
            return !valid ? false : Valid.Native();
        });

        Enabled.OnChange += {this, [this, recalculateOnEnabled]{
            THREAD_ASSERT_IS_MAIN();
            if(Enabled) {
                m_onChanged += { this, [this, recalculateOnEnabled]{
                    Valid.SetState(false);
                    if(m_dependenciesAreUpToDate) {
                        Calculate(m_calculator, m_preparator);
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
        m_onChanged.Invoke();
    }

    void Disconnect()
    {
        THREAD_ASSERT_IS_MAIN();
        m_connections.clear();
        m_dependenciesAreUpToDate.ClearProperties();
    }

    void SetCalculator(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{})
    {
        m_preparator = preparator;
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
    const StateCalculator& Connect(const LocalProperty<T2>& property) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.Subscribe({ &nonConstProperty.OnChange }).MakeSafe(m_connections);
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
    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{})
    {
        Super::Calculate(calculator, preparator);
    }

private:
    typename ThreadCalculatorData<T>::Calculator m_calculator;
    typename ThreadCalculatorData<T>::Preparator m_preparator;
    mutable StatePropertyBoolCommutator m_dependenciesAreUpToDate;
    mutable DispatcherConnectionsSafe m_connections;
    mutable DispatchersCommutator m_onChanged;
};

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
        m_data->IsValid.ConnectFrom(m_calculator.Valid, [this](bool valid){
            return valid ? m_data->IsValid.Native() : false;
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

    void AttachCopy(const TPtr& externalData, const std::function<TPtr ()>& handler = nullptr)
    {
        m_calculator.Disconnect();

        if(externalData != nullptr) {
            m_handler = handler == nullptr ? [externalData]{ return externalData->Clone(); } : handler;
            m_calculator.Connect(externalData->IsValid).Connect(externalData->OnChanged);
        } else {
            m_handler = nullptr;
        }

        m_calculator.SetCalculator([]{
            return true;
        });

        m_calculator.RequestRecalculate();
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
        Enabled.OnChange += { this, [this, source]{
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

#endif // STATEPROPERTY_H
