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
        , m_destroying(false)
    {}
    ~ThreadCalculatorBase()
    {
        safeQuit();
    }

    void Calculate()
    {
        m_threadHandler([this]{
            if(m_isCalculating) {
                m_needToRecalculate = true;
                return;
            }

            m_isCalculating = true;
            prepare();
            ThreadsBase::Async([this]{
                if(m_destroying) {
                    m_isCalculating = false;
                    return;
                }
                auto result = calculate();
                m_threadHandler([this, result]{
                    bool destroying = m_destroying;
                    m_isCalculating = false;
                    if(destroying) {
                        return;
                    }
                    if(m_needToRecalculate) {
                        m_needToRecalculate = false;
                        Calculate();
                    } else {
                        OnCalculated(result);
                    }
                });
            }, EPriority::Low);
        });
    }

    CommonDispatcher<const T&> OnCalculated;

protected:
    void safeQuit()
    {
        m_destroying = true;
        while(m_isCalculating) {
            qApp->processEvents();
        }
    }

    virtual T calculate() const = 0;
    virtual void prepare() {}

protected:
    ThreadHandler m_threadHandler;

private:
    std::atomic_bool m_isCalculating;
    std::atomic_bool m_needToRecalculate;
    std::atomic_bool m_destroying;
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
