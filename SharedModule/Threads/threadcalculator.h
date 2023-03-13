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
    bool Canceled = false;
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

        
        CurrentData(const ThreadCalculatorDataPtr<T>& data)
            : Data(data)
            , CurrentCalculator(data->CalculatorHandler)
            , m_currentReleaser(data->ReleaserHandler)
            , m_released(false)
        {
            
        }
        
        ~CurrentData()
        {
            Q_ASSERT(m_released);
            auto data = Data;
            auto currentCalculator = CurrentCalculator;
            auto currentReleaser = m_currentReleaser;
            data->Handler([data, currentCalculator, currentReleaser]{});
        }

        void Release() {
            if(!m_released) {
                m_released = true;
                m_currentReleaser();
            }
        }

    private:
        typename ThreadCalculatorData<T>::Releaser m_currentReleaser;
        bool m_released;
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

    void Cancel()
    {
        m_data->Handler([this]{
            m_data->Canceled = true;
        });
    }

    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                   const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        m_data->Handler([this, calculator, preparator, releaser]{
            if(m_prevData != nullptr) {
                m_prevData->Release();
            }
            m_data->PreparatorHandler = preparator;
            m_data->CalculatorHandler = calculator;
            m_data->ReleaserHandler = releaser;
            
            if(m_data->Calculating && !m_latestTask.IsResolved()) {
                bool resetedInThisThread = false;
                m_latestTask.Resolve([&resetedInThisThread]{ resetedInThisThread = true; return false; });
                if(!resetedInThisThread) {
                    m_data->NeedRecalculate = true;
                    return;
                }
                m_data->NeedRecalculate = false;
            }
            
            m_data->Canceled = false;
            m_data->Calculating = true;
            m_data->PreparatorHandler();
            onPreRecalculate();
            
            auto currentData = m_prevData = ::make_shared<CurrentData>(m_data);
            m_latestTask = ThreadsBase::Async([this, currentData]{
                auto result = currentData->CurrentCalculator();
                currentData->Data->Handler([this, result, currentData]{
                    currentData->Release();
                    const auto& data = currentData->Data;
                    if(data->Destroyed) {
                        return;
                    }
                    data->Calculating = false;
                    if(data->NeedRecalculate) {
                        data->NeedRecalculate = false;
                        Calculate(data->CalculatorHandler, data->PreparatorHandler, data->ReleaserHandler);
                    } else if(!data->Canceled){
                        if(acceptResult()) {
                            OnCalculated(result);
                            onPostRecalculate();
                        }
                    }
                });
            }, EPriority::Low);
            m_latestTask.Then([currentData](bool){
                currentData->Data->Handler([currentData]{
                    currentData->Release();
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
    virtual void onPreRecalculate() {}

protected:
    ThreadCalculatorDataPtr<T> m_data;
    SharedPointer<CurrentData> m_prevData;
    AsyncResult m_latestTask;
};

#endif // THREADCALCULATOR_H
