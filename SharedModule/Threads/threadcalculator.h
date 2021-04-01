#ifndef THREADCALCULATOR_H
#define THREADCALCULATOR_H

#include <QApplication>

#include "SharedModule/dispatcher.h"
#include "threadsbase.h"

struct ThreadCalculatorData
{
    std::atomic_bool Destroyed = false;
    bool NeedRecalculate = false;
    bool Calculating = false;
    ThreadHandler Handler;

    ThreadCalculatorData(const ThreadHandler& handler)
        : Handler(handler)
    {}
};

using ThreadCalculatorDataPtr = SharedPointer<ThreadCalculatorData>;

template<class T>
class ThreadCalculatorBase
{
public:
    ThreadCalculatorBase(const ThreadHandler& threadHandler)
        : m_data(::make_shared<ThreadCalculatorData>(threadHandler))
    {}
    ~ThreadCalculatorBase()
    {
        SafeQuit();
    }

    void Calculate()
    {
        m_data->Handler([this]{
            if(m_data->Calculating) {
                m_data->NeedRecalculate = true;
                return;
            }

            m_data->Calculating = true;
            prepare();
            auto data = m_data;
            ThreadsBase::Async([this, data]{
                auto result = calculate();
                data->Handler([this, result, data]{
                    if(data->Destroyed) {
                        return;
                    }
                    data->Calculating = false;
                    if(data->NeedRecalculate) {
                        data->NeedRecalculate = false;
                        Calculate();
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
    virtual T calculate() const = 0;
    virtual void prepare() {}

protected:

    ThreadCalculatorDataPtr m_data;
};

template<class T>
class ThreadCalculatorLambda : public ThreadCalculatorBase<T>
{
    using Super = ThreadCalculatorBase<T>;
public:
    using Calculator = std::function<T ()>;
    using Preparator = FAction;
    ThreadCalculatorLambda(const ThreadHandler& threadHandler)
        : Super(threadHandler)
        , m_calculator([]{ return T(); })
    {}

    void Calculate(const Calculator& calculator, const Preparator& preparator = []{})
    {
        {
            QMutexLocker locker(&m_mutex);
            m_calculator = calculator;
            m_preparator = preparator;
        }
        Super::Calculate();
    }

protected:
    T calculate() const override
    {
        Calculator calculator;
        {
            QMutexLocker locker(&m_mutex);
            calculator = m_calculator;
        }
        return calculator();
    }
    void prepare() override
    {
        Preparator preparator;
        {
            QMutexLocker locker(&m_mutex);
            preparator = m_preparator;
        }
        preparator();
    }

private:
    mutable QMutex m_mutex;
    Calculator m_calculator;
    Preparator m_preparator;
};

#endif // THREADCALCULATOR_H
