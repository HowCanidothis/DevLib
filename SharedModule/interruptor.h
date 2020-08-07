#ifndef INTERRUPTOR_H
#define INTERRUPTOR_H

#include "dispatcher.h"
#include "SharedModule/Threads/threadtimer.h"

class Interruptor
{
public:
    Interruptor()
        : OnInterrupted(::make_shared<Dispatcher>())
        , m_interupted(::make_shared<std::atomic_bool>(false))
    {}

    void Interrupt() { *m_interupted = true; OnInterrupted->Invoke(); }
    void Interrupt(qint32 msecs) { ThreadTimer::SingleShot(msecs, [this]{ Interrupt(); }); }

    bool IsInterrupted() const { return *m_interupted; }

    SharedPointer<Dispatcher> OnInterrupted;

private:
    SharedPointer<std::atomic_bool> m_interupted;
};

#endif // INTERUPTOR_H
