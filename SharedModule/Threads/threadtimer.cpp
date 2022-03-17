#include "threadtimer.h"

#include <QThread>
#include "SharedModule/Threads/threadsbase.h"

ThreadTimer::ThreadTimer(qint32 msecs)
    : m_handle(ThreadTimerManager::createTimer(msecs))
{

}

void ThreadTimerManager::Terminate()
{
    getInstance().terminate();
}

void ThreadTimerManager::terminate()
{
    FutureResult result;
    result += ThreadsBase::DoQThreadWorkerWithResult(m_threadWorker.get(), [this]{
        m_timers.Clear();
        m_threadWorker = nullptr;
    });
    result.Wait();
    m_thread->quit();
    m_thread->wait();
    m_isTerminated = true;
}

QMetaObject::Connection ThreadTimer::OnTimeout(const FAction& action)
{
    return ThreadTimerManager::addTimerConnection(m_handle, action);
}

ThreadTimer::~ThreadTimer()
{
    if(ThreadTimerManager::getInstance().m_isTerminated) {
        return;
    }
    FutureResult result;
    result += ThreadTimerManager::deleteTimer(m_handle);
    result.Wait();
}

ThreadTimerManager::ThreadTimerManager()
    : m_thread(new QThread())
    , m_threadWorker(new QObject())
    , m_isTerminated(false)
{
    m_thread->start();
    m_threadWorker->moveToThread(m_thread.get());
}

ThreadTimerManager::~ThreadTimerManager()
{
    Q_ASSERT(m_isTerminated);
}

void ThreadTimerManager::SingleShotDoMain(qint32 msecs, const FAction& onTimeout)
{
    SingleShot(msecs, [onTimeout]{
        ThreadsBase::DoMain([onTimeout]{
            onTimeout();
        });
    });
}

void ThreadTimerManager::SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker)
{
    SingleShot(msecs, [onTimeout, threadWorker]{
        ThreadsBase::DoQThreadWorker(threadWorker, onTimeout);
    });
}

void ThreadTimerManager::SingleShot(qint32 msecs, const FAction& onTimeout)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    FutureResult futureResult;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [msecs, onTimeout]{
        QTimer::singleShot(msecs, onTimeout);
    });
    futureResult.Wait();
}

QTimer* ThreadTimerManager::createTimer(qint32 msecs)
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
    return result;
}

AsyncResult ThreadTimerManager::deleteTimer(QTimer* timerHandle)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    return ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [timerHandle]{
        getInstance().m_timers.Remove(timerHandle);
        delete timerHandle;
    });
}

QMetaObject::Connection ThreadTimerManager::addTimerConnection(QTimer* timerHandle, const FAction& onTimeout)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    FutureResult futureResult;
    QMetaObject::Connection result;
    futureResult += ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [timerHandle, onTimeout, &result]{
        result = QObject::connect(timerHandle, &QTimer::timeout, onTimeout);
    });
    futureResult.Wait();
    return result;
}

void ThreadTimerManager::removeTimerConnection(const QMetaObject::Connection& connection)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    ThreadsBase::DoQThreadWorkerWithResult(getInstance().m_threadWorker.get(), [connection]{
        QObject::disconnect(connection);
    });
}

ThreadTimerManager& ThreadTimerManager::getInstance()
{
    static ThreadTimerManager result;
    return result;
}
