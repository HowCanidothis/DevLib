#include "threadtimer.h"

#include <QThread>
#include "SharedModule/Threads/threadsbase.h"

ThreadTimer::ThreadTimer(qint32 msecs)
    : m_handle(ThreadTimerManager::createTimer(msecs))
{

}

void ThreadTimerManager::terminate()
{
    if(!m_thread->isRunning()) {
        m_isTerminated = true;
        return;
    }

    FutureResult result;
    result += ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, m_threadWorker.get(), [this]{
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
    auto connection = ThreadTimerManager::addTimerConnection(m_handle, action);
    m_connections.Add(connection);
    return connection;
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
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this]{
        terminate();
    });

    ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, m_threadWorker.get(), [this]{
        m_singleShotTimer = new ThreadTimer(100);
        m_singleShotTimer->OnTimeout([this]{
            m_singleShotStack.remove_if([](auto& pair) {
                pair.first -= 100;
                if(pair.first <= 0) {
                    pair.second();
                    return true;
                }
                return false;
            });
        });
    });
}

ThreadTimerManager::~ThreadTimerManager()
{
    Q_ASSERT(m_isTerminated);
}

AsyncResult ThreadTimerManager::SingleShotDoMain(qint32 msecs, const FAction& onTimeout)
{
    auto result = SingleShot(msecs, []{});
    result.MoveToMain([onTimeout](qint8 ok) {
        if(ok) {
            onTimeout();
        }
        return ok;
    });
    return result;
}

void ThreadTimerManager::SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker)
{
    SingleShot(msecs, [onTimeout, threadWorker]{
        ThreadsBase::DoQThreadWorker(CONNECTION_DEBUG_LOCATION, threadWorker, onTimeout);
    });
}

AsyncResult ThreadTimerManager::SingleShot(qint32 msecs, const FAction& onTimeout)
{
    Q_ASSERT(getInstance().m_thread->isRunning());
    AsyncResult result;

    ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, getInstance().m_threadWorker.get(), [msecs, result]{
        ThreadTimerManager::getInstance().m_singleShotStack.push_back(std::make_pair(msecs, [result]{
            result.Resolve(true);
        }));
    });
    result.Then([onTimeout](bool){
        onTimeout();
    });
    return result;
}

void ThreadTimerManager::Terminate()
{
    getInstance().terminate();
}

QTimer* ThreadTimerManager::createTimer(qint32 msecs)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    QTimer* result;
    if(QThread::currentThread() == getInstance().m_thread.get()) {
            auto* timer = new QTimer;
            getInstance().m_timers.InsertSortedUnique(timer);
            timer->start(msecs);
            result = timer;
    } else {
        FutureResult futureResult;
        futureResult += ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, getInstance().m_threadWorker.get(), [msecs, &result]{
            auto* timer = new QTimer;
            getInstance().m_timers.InsertSortedUnique(timer);
            timer->start(msecs);
            result = timer;
        });
        futureResult.Wait();
    }

    return result;
}

AsyncResult ThreadTimerManager::deleteTimer(QTimer* timerHandle)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    return ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, getInstance().m_threadWorker.get(), [timerHandle]{
        getInstance().m_timers.Remove(timerHandle);
        delete timerHandle;
    });
}

QMetaObject::Connection ThreadTimerManager::addTimerConnection(QTimer* timerHandle, const FAction& onTimeout)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    QMetaObject::Connection result;
    if(QThread::currentThread() == getInstance().m_thread.get()) {
        result = QObject::connect(timerHandle, &QTimer::timeout, onTimeout);
    } else {
        FutureResult futureResult;
        futureResult += ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, getInstance().m_threadWorker.get(), [timerHandle, onTimeout, &result]{
            result = QObject::connect(timerHandle, &QTimer::timeout, onTimeout);
        });
        futureResult.Wait();
    }
    return result;
}

void ThreadTimerManager::removeTimerConnection(const QMetaObject::Connection& connection)
{
    Q_ASSERT(getInstance().m_thread->isRunning());

    ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, getInstance().m_threadWorker.get(), [connection]{
        QObject::disconnect(connection);
    });
}

ThreadTimerManager& ThreadTimerManager::getInstance()
{
    static ThreadTimerManager result;
    return result;
}
