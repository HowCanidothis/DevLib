#ifndef INTERRUPTOR_H
#define INTERRUPTOR_H

#include "dispatcher.h"
#include "name.h"
#include "SharedModule/Threads/threadtimer.h"

class InterruptorData
{
    friend class Interruptor;
    Dispatcher OnInterrupted;
    std::atomic_bool m_interupted;
    InterruptorData* m_parentData;

    void interrupt();
    void setParent(InterruptorData* data);

public:
    InterruptorData();

    ~InterruptorData();
};

class Interruptor
{
public:
    Interruptor();

    void SetParent(const Interruptor& another);
    void Interrupt() const;
    void Interrupt(qint32 msecs) const;

    bool IsInterrupted() const;

    const Dispatcher& OnInterrupted() const;

private:
    SharedPointer<InterruptorData> m_data;
};

struct AsyncRequest
{
    AsyncResult Result;
    Interruptor ResultInterruptor;
};

class AsyncRequestPending
{
public:
    AsyncRequestPending(const Name& processId, const QString& label = QObject::tr("Fetching Data..."));
    AsyncRequestPending();

    AsyncRequest SetRequest(const AsyncRequest& request);

private:
    AsyncRequest m_currentRequest;
    QString m_label;
    Name m_processId;
};

SharedPointer<AsyncRequestPending> AsyncRequestPendingCreate(const Name& processId, const QString& label = QObject::tr("Fetching Data..."))
{
    return ::make_shared<AsyncRequestPending>(processId, label);
}

SharedPointer<AsyncRequestPending> AsyncRequestPendingCreate()
{
    return ::make_shared<AsyncRequestPending>();
}

#endif // INTERUPTOR_H
