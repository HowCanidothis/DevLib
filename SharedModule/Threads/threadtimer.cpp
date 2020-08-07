#include "threadtimer.h"

#include <QThread>
#include <QTimer>

#include "SharedModule/Threads/threadsbase.h"

ThreadTimer::ThreadTimer()
    : m_thread(new QThread())
    , m_threadWorker(new QObject())
{
    m_thread->start();
    m_threadWorker->moveToThread(m_thread.get());
}

ThreadTimer::~ThreadTimer()
{
    FutureResult result;
    result += ThreadsBase::DoQThreadWorkerWithResult(m_threadWorker.get(), [this]{
        m_timers.Clear();
        m_threadWorker = nullptr;
    });
    result.Wait();
    m_thread->quit();
    m_thread->wait();
}

void ThreadTimer::SingleShot(qint32 msecs, const FAction& onTimeout)
{
    FutureResult futureResult;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [msecs, onTimeout]{
        QTimer::singleShot(msecs, onTimeout);
    });
    futureResult.Wait();
}

ThreadTimer::TimerHandle ThreadTimer::CreateTimer(qint32 msecs)
{
    FutureResult futureResult;
    ThreadTimer::TimerHandle result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [msecs, &result]{
        auto* timer = new QTimer;
        getInstance().m_timers.InsertSortedUnique(timer);
        timer->start(msecs);
        result = timer;
    });
    futureResult.Wait();
    return result;
}

void ThreadTimer::DeleteTimer(TimerHandle* timerHandle)
{
    ThreadsBase::DoQThreadWorker(getInstance().m_threadWorker.get(), [timerHandle]{
        auto* timer = reinterpret_cast<QTimer*>(timerHandle);
        timer->stop();
    });
}

QMetaObject::Connection ThreadTimer::AddTimerConnection(ThreadTimer::TimerHandle timerHandle, const FAction& onTimeout)
{
    FutureResult futureResult;
    QMetaObject::Connection result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [timerHandle, onTimeout, &result]{
        auto* timer = reinterpret_cast<QTimer*>(timerHandle);
        result = QObject::connect(timer, &QTimer::timeout, onTimeout);
    });
    futureResult.Wait();
    return result;
}

void ThreadTimer::RemoveTimerConnection(const QMetaObject::Connection& connection)
{
    ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [connection]{
        QObject::disconnect(connection);
    });
}

ThreadTimer& ThreadTimer::getInstance()
{
    static ThreadTimer result;
    return result;
}
