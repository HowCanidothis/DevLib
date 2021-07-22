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
    class CurrentData
    {
    public:
        ThreadCalculatorDataPtr<T> Data;
        typename ThreadCalculatorData<T>::Calculator CurrentCalculator;
        typename ThreadCalculatorData<T>::Releaser CurrentReleaser;
        
        CurrentData(const ThreadCalculatorDataPtr<T>& data)
            : Data(data)
            , CurrentCalculator(data->CalculatorHandler)
            , CurrentReleaser(data->ReleaserHandler)
        {
            
        }
        
        ~CurrentData()
        {
            auto data = Data;
            auto currentCalculator = CurrentCalculator;
            auto currentReleaser = CurrentReleaser;
            data->Handler([data, currentCalculator, currentReleaser]{});
        }
        
    };
    
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
                bool resetedInThisThread = false;
                m_latestTask.Resolve([&resetedInThisThread]{ resetedInThisThread = true; return false; });
                if(!resetedInThisThread) {
                    m_data->NeedRecalculate = true;
                    return;
                }
                m_data->NeedRecalculate = false;
            }
            
            m_data->Calculating = true;
            m_data->PreparatorHandler();
            
            auto currentData = ::make_shared<CurrentData>(m_data);
            m_latestTask = ThreadsBase::Async([this, currentData]{
                auto result = currentData->CurrentCalculator();
                currentData->Data->Handler([this, result, currentData]{
                    const auto& data = currentData->Data;
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
            m_latestTask.Then([currentData](bool){
                currentData->Data->Handler([currentData]{
                    currentData->CurrentReleaser();
                });
            });
        });
    }
    
    bool IsCalculating() const {return m_data->Calculating; }
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
