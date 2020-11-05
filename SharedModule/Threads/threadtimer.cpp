#include "threadtimer.h"

#include <QThread>
#include "SharedModule/Threads/threadsbase.h"

ThreadTimerHandle::~ThreadTimerHandle()
{
    ThreadTimer::deleteTimer(m_handle);
}

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
    Q_ASSERT(getInstance().m_thread->isRunning());

    FutureResult futureResult;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [msecs, onTimeout]{
        QTimer::singleShot(msecs, onTimeout);
    });
    futureResult.Wait();
}

ThreadTimerHandlePtr ThreadTimer::CreateTimer(qint32 msecs)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    FutureResult futureResult;
    QTimer* result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [msecs, &result]{
        auto* timer = new QTimer;
        getInstance().m_timers.InsertSortedUnique(timer);
        timer->start(msecs);
        result = timer;
    });
    futureResult.Wait();
    return SharedPointer<ThreadTimerHandle>(new ThreadTimerHandle(result));
}

void ThreadTimer::deleteTimer(QTimer* timerHandle)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    ThreadsBase::DoQThreadWorker(getInstance().m_threadWorker.get(), [timerHandle]{
        getInstance().m_timers.Remove(timerHandle);
        delete timerHandle;
    });
}

QMetaObject::Connection ThreadTimer::AddTimerConnection(const ThreadTimerHandlePtr& timerHandle, const FAction& onTimeout)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    FutureResult futureResult;
    QMetaObject::Connection result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [timerHandle, onTimeout, &result]{
        result = QObject::connect(timerHandle->GetTimer(), &QTimer::timeout, onTimeout);
    });
    futureResult.Wait();
    return result;
}

void ThreadTimer::RemoveTimerConnection(const QMetaObject::Connection& connection)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [connection]{
        QObject::disconnect(connection);
    });
}

ThreadTimer& ThreadTimer::getInstance()
{
    static ThreadTimer result;
    return result;
}
