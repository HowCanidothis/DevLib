#ifndef THREADCALCULATOR_H
#define THREADCALCULATOR_H

#include <QApplication>

#include "SharedModule/dispatcher.h"
#include "threadsbase.h"

template<class T>
struct ThreadCalculatorData
{
    using Calculator = std::function<T ()>;
    using Preparator = FAction;

    std::atomic_bool Destroyed = false;
    bool NeedRecalculate = false;
    bool Calculating = false;
    ThreadHandler Handler;
    Calculator CalculatorHandler = []{ return T(); };
    Preparator PreparatorHandler = []{};

    ThreadCalculatorData(const ThreadHandler& handler)
        : Handler(handler)
    {}
};

template<class T>
using ThreadCalculatorDataPtr = SharedPointer<ThreadCalculatorData<T>>;

template<class T>
class ThreadCalculator
{
public:
    ThreadCalculator(const ThreadHandler& threadHandler)
        : m_data(::make_shared<ThreadCalculatorData<T>>(threadHandler))
    {}
    ~ThreadCalculator()
    {
        SafeQuit();
    }

    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{})
    {
        m_data->Handler([this, calculator, preparator]{
            m_data->PreparatorHandler = preparator;
            m_data->CalculatorHandler = calculator;

            if(m_data->Calculating) {
                m_data->NeedRecalculate = true;
                return;
            }

            m_data->Calculating = true;
            m_data->PreparatorHandler();
            auto data = m_data;
            ThreadsBase::Async([this, data, calculator]{
                auto result = calculator();
                data->Handler([this, result, data]{
                    if(data->Destroyed) {
                        return;
                    }
                    data->Calculating = false;
                    if(data->NeedRecalculate) {
                        data->NeedRecalculate = false;
                        Calculate(data->CalculatorHandler, data->PreparatorHandler);
                    } else {
                        OnCalculated(result);
                    }
                });
            }, EPriority::Low);
        });
    }

    void SafeQuit()
    {
        m_data->Destroyed = true;
    }

    CommonDispatcher<const T&> OnCalculated;

protected:
    ThreadCalculatorDataPtr<T> m_data;
};

#endif // THREADCALCULATOR_H
