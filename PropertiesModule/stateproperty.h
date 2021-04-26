#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"

class StateProperty : public LocalPropertyBool
{
    using Super = LocalPropertyBool;
public:
    using Super::Super;
};

template<class T>
class StateCalculator : public ThreadCalculator<T>
{
    using Super = ThreadCalculator<T>;
public:
    StateCalculator(const ThreadHandler& threadHandler)
        : Super(threadHandler)
        , Valid(true)
        , m_dependenciesAreUpToDate(true)
    {
        m_onChanged.Subscribe({ &m_dependenciesAreUpToDate.OnChange });
        m_onChanged += { this, [this]{
            if(m_dependenciesAreUpToDate) {
                Calculate(m_calculator, m_preparator);
            }
            Valid.SetValue(false);
        }};

        Super::OnCalculated += {this, [this](const T&){
            Valid.SetValue(true);
        }};
    }

    void SetCalculator(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{})
    {
        m_preparator = preparator;
        m_calculator = calculator;
    }

    template<class T2>
    StateCalculator& Connect(const StateCalculator<T2>& calculator)
    {
        auto& nonConstCalculator = const_cast<StateCalculator<T2>&>(calculator);
        m_dependenciesAreUpToDate.AddProperties({ &nonConstCalculator.Valid }).MakeSafe(m_connections);
        return *this;
    }

    template<class T2>
    StateCalculator& Connect(const LocalProperty<T2>& property)
    {
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.Subscribe({ &nonConstProperty.OnChange }).MakeSafe(m_connections);
        return *this;
    }

    StateCalculator& Connect(StateProperty& dispatcher)
    {
        m_dependenciesAreUpToDate.AddProperties({ &dispatcher }).MakeSafe(m_connections);
        return *this;
    }

    StateCalculator& Connect(Dispatcher& onChanged)
    {
        m_onChanged.Subscribe({&onChanged}).MakeSafe(m_connections);
        return *this;
    }

    StateProperty Valid;

protected:
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
    LocalPropertyBoolCommutator m_dependenciesAreUpToDate;
    DispatcherConnectionsSafe m_connections;
    DispatchersCommutator m_onChanged;
};

#endif // STATEPROPERTY_H
