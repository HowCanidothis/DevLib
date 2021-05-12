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
        THREAD_ASSERT_IS_MAIN();
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

        Enabled.OnChange += {this, [this, recalculateOnEnabled]{
            if(Enabled) {
                m_onChanged += { this, [this, recalculateOnEnabled]{
                    Valid.SetState(false);
                    if(m_dependenciesAreUpToDate) {
                        Calculate(m_calculator, m_preparator);
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
        m_dependenciesAreUpToDate.Update();
        m_onChanged.Invoke();
    }

    void Disconnect()
    {
        m_connections.clear();
        m_dependenciesAreUpToDate.ClearProperties();
    }

    void SetCalculator(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{})
    {
        m_preparator = preparator;
        m_calculator = calculator;
        RequestRecalculate();
    }

    template<class T2>
    const StateCalculator& Connect(const StateCalculator<T2>& calculator) const
    {
        auto& nonConstCalculator = const_cast<StateCalculator<T2>&>(calculator);
        m_dependenciesAreUpToDate.AddProperties({ &nonConstCalculator.Valid }).MakeSafe(m_connections);
        return *this;
    }

    template<class T2>
    const StateCalculator& Connect(const LocalProperty<T2>& property) const
    {
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.Subscribe({ &nonConstProperty.OnChange }).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(StateProperty& dispatcher) const
    {
        m_dependenciesAreUpToDate.AddProperties({ &dispatcher }).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(Dispatcher& onChanged) const
    {
        m_onChanged.Subscribe({&onChanged}).MakeSafe(m_connections);
        return *this;
    }

    const StatePropertyBoolCommutator& GetDependenciesState() const { return m_dependenciesAreUpToDate; }

    LocalPropertyBool Enabled;
    StateProperty Valid;

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

#endif // STATEPROPERTY_H
