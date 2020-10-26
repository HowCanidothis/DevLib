#ifndef THREADCALCULATOR_H
#define THREADCALCULATOR_H

#include <QApplication>

#include "SharedModule/dispatcher.h"
#include "threadsbase.h"

using ThreadHandler = std::function<void (const FAction& action)>;

const ThreadHandler ThreadHandlerMain = [](const FAction& action) {
    if(QThread::currentThread() == qApp->thread()) {
        action();
    } else {
        ThreadsBase::DoMain(action);
    }
};

template<class T>
class ThreadCalculatorBase
{
public:
    ThreadCalculatorBase(const ThreadHandler& threadHandler)
        : m_isCalculating(false)
        , m_needToRecalculate(false)
        , m_threadHandler(threadHandler)
    {}
    ~ThreadCalculatorBase()
    {
        while(m_isCalculating) {
            m_needToRecalculate = false;
            qApp->processEvents();
        }
    }

    void Calculate()
    {
        m_threadHandler([this]{
            if(m_isCalculating) {
                m_needToRecalculate = true;
                return;
            }

            m_isCalculating = true;
            ThreadsBase::Async([this]{
                auto result = calculate();
                m_threadHandler([this, result]{
                    m_isCalculating = false;
                    if(m_needToRecalculate) {
                        m_needToRecalculate = false;
                        Calculate();
                    } else {
                        OnCalculated(result);
                    }
                });
            });
        });
    }

    CommonDispatcher<const T&> OnCalculated;

protected:
    virtual T calculate() const = 0;

protected:
    ThreadHandler m_threadHandler;

private:
    std::atomic_bool m_isCalculating;
    std::atomic_bool m_needToRecalculate;

};

template<class T>
class ThreadCalculatorLambda : public ThreadCalculatorBase<T>
{
    using Super = ThreadCalculatorBase<T>;
public:
    using Calculator = std::function<T ()>;
    ThreadCalculatorLambda(const ThreadHandler& threadHandler)
        : Super(threadHandler)
        , m_calculator([]{ return T(); })
    {}

    void Calculate(const Calculator& calculator)
    {
        {
            QMutexLocker locker(&m_mutex);
            m_calculator = calculator;
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

private:
    mutable QMutex m_mutex;
    Calculator m_calculator;
};

#endif // THREADCALCULATOR_H
