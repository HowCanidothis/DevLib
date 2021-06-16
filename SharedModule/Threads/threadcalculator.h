#ifndef THREADCALCULATOR_H
#define THREADCALCULATOR_H

#include "SharedModule/dispatcher.h"
#include "threadsbase.h"

template<class T>
struct ThreadCalculatorData
{
    using Calculator = std::function<T ()>;
    using Preparator = FAction;
    using Releaser = FAction;

    std::atomic_bool Destroyed = false;
    bool NeedRecalculate = false;
    bool Calculating = false;
    ThreadHandler Handler;
    Calculator CalculatorHandler = []{ return T(); };
    Preparator PreparatorHandler = []{};
    Releaser ReleaserHandler = []{};

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
    template<typename ... Args> using CalculationData = SharedPointer<std::tuple<Args...>>;

    template<typename ... Args> CalculationData<Args...> PreparationData() { return ::make_shared<std::tuple<Args...>>();  }

    ThreadCalculator(const ThreadHandler& threadHandler)
        : m_data(::make_shared<ThreadCalculatorData<T>>(threadHandler))
    {}
    ~ThreadCalculator()
    {
        m_latestTask.Resolve(false);
        SafeQuit();
    }

    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                   const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        m_data->Handler([this, calculator, preparator, releaser]{
            m_data->PreparatorHandler = preparator;
            m_data->CalculatorHandler = calculator;
            m_data->ReleaserHandler = releaser;

            if(m_data->Calculating) {
                m_data->NeedRecalculate = true;
                return;
            }

            m_data->Calculating = true;
            m_data->PreparatorHandler();
            auto data = m_data;
            m_latestTask = ThreadsBase::Async([this, data, calculator]{
                auto result = calculator();
                data->Handler([this, result, data]{
                    if(data->Destroyed) {
                        return;
                    }
                    data->Calculating = false;
                    if(data->NeedRecalculate) {
                        data->NeedRecalculate = false;
                        Calculate(data->CalculatorHandler, data->PreparatorHandler, data->ReleaserHandler);
                    } else {
                        if(acceptResult()) {
                            OnCalculated(result);
                            onPostRecalculate();
                        }
                    }
                });
            }, EPriority::Low);
            m_latestTask.Then([data](bool){
                data->Handler([data]{
                    data->ReleaserHandler();
                });
            });
        });
    }

    void SafeQuit()
    {
        m_data->Destroyed = true;
    }

    CommonDispatcher<const T&> OnCalculated;

protected:
    virtual bool acceptResult() { return true; }
    virtual void onPostRecalculate() {}

protected:
    ThreadCalculatorDataPtr<T> m_data;
    AsyncResult m_latestTask;
};

#endif // THREADCALCULATOR_H
