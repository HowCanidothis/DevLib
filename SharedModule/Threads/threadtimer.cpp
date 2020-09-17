#include "threadtimer.h"

#include <QThread>
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

void ThreadTimer::SingleShotDoMain(qint32 msecs, const FAction& onTimeout)
{
    SingleShot(msecs, [onTimeout]{
        ThreadsBase::DoMain([onTimeout]{
            onTimeout();
        });
    });
}

void ThreadTimer::SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker)
{
    SingleShot(msecs, [onTimeout, threadWorker]{
        ThreadsBase::DoQThreadWorker(threadWorker, onTimeout);
    });
}

void ThreadTimer::SingleShot(qint32 msecs, const FAction& onTimeout)
{
    if(!GetInstance().m_thread->isRunning()) {
        onTimeout();
        return;
    }

    FutureResult futureResult;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(GetInstance().m_threadWorker.get(), [msecs, onTimeout]{
        QTimer::singleShot(msecs, onTimeout);
    });
    futureResult.Wait();
}

ThreadTimer::TimerHandle ThreadTimer::CreateTimer(qint32 msecs)
{
    if(!GetInstance().m_thread->isRunning()) {
        return nullptr;
    }

    FutureResult futureResult;
    ThreadTimer::TimerHandle result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(GetInstance().m_threadWorker.get(), [msecs, &result]{
        auto* timer = new QTimer;
        GetInstance().m_timers.InsertSortedUnique(timer);
        timer->start(msecs);
        result = timer;
    });
    futureResult.Wait();
    return result;
}

void ThreadTimer::DeleteTimer(TimerHandle* timerHandle)
{
    if(!GetInstance().m_thread->isRunning()) {
        return;
    }

    ThreadsBase::DoQThreadWorker(GetInstance().m_threadWorker.get(), [timerHandle]{
        auto* timer = reinterpret_cast<QTimer*>(timerHandle);
        timer->stop();
    });
}

QMetaObject::Connection ThreadTimer::AddTimerConnection(ThreadTimer::TimerHandle timerHandle, const FAction& onTimeout)
{
    if(!GetInstance().m_thread->isRunning()) {
        onTimeout();
        return QMetaObject::Connection();
    }

    FutureResult futureResult;
    QMetaObject::Connection result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(GetInstance().m_threadWorker.get(), [timerHandle, onTimeout, &result]{
        auto* timer = reinterpret_cast<QTimer*>(timerHandle);
        result = QObject::connect(timer, &QTimer::timeout, onTimeout);
    });
    futureResult.Wait();
    return result;
}

void ThreadTimer::RemoveTimerConnection(const QMetaObject::Connection& connection)
{
    if(!GetInstance().m_thread->isRunning()) {
        return;
    }

    ThreadsBase::DoQThreadWorkerWithResult(GetInstance().m_threadWorker.get(), [connection]{
        QObject::disconnect(connection);
    });
}

ThreadTimer& ThreadTimer::GetInstance()
{
    static ThreadTimer result;
    return result;
}
