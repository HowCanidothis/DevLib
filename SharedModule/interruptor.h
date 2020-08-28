#ifndef INTERRUPTOR_H
#define INTERRUPTOR_H

#include "dispatcher.h"
#include "SharedModule/Threads/threadtimer.h"

class InterruptorData
{
    friend class Interruptor;
    Dispatcher OnInterrupted;
    std::atomic_bool m_interupted;
    InterruptorData* m_parentData;

    void interrupt()
    {
        m_interupted = true;
        OnInterrupted.Invoke();
    }
    void setParent(InterruptorData* data)
    {
        Q_ASSERT(m_parentData == nullptr);
        m_parentData = data;
        data->OnInterrupted += { this, [this]{ interrupt(); } };
    }

public:
    InterruptorData()
        : m_interupted(false)
        , m_parentData(nullptr)
    {
    }

    ~InterruptorData()
    {
        if(m_parentData != nullptr) {
            m_parentData->OnInterrupted -= this;
        }
    }
};

class Interruptor
{
public:
    Interruptor()
        : m_data(::make_shared<InterruptorData>())
        , OnInterrupted(m_data->OnInterrupted)
    {}


    void SetParent(const Interruptor& another) { m_data->setParent(another.m_data.get());  }
    void Interrupt() { m_data->interrupt(); }
    void Interrupt(qint32 msecs) { auto data = m_data; ThreadTimer::SingleShot(msecs, [data]{ data->interrupt(); }); }

    bool IsInterrupted() const { return m_data->m_interupted; }

private:
    SharedPointer<InterruptorData> m_data;

public:
    Dispatcher& OnInterrupted;
};


#endif // INTERUPTOR_H
