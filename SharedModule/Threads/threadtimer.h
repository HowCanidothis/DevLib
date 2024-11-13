#ifndef THREADTIMER_H
#define THREADTIMER_H

#include <QTimer>

#include "SharedModule/declarations.h"
#include "SharedModule/array.h"
#include "SharedModule/smartpointersadapters.h"
#include "Promises/promise.h"
#include "SharedModule/External/qtlambdaconnections.h"

class ThreadTimerManager
{
    ThreadTimerManager();
    ~ThreadTimerManager();
public:
    static void SingleShot(qint32 msecs, const FAction& onTimeout);
    static void SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker);
    static void SingleShotDoMain(qint32 msecs, const FAction& onTimeout);

private:
    friend class ThreadTimer;
    static QTimer* createTimer(qint32 msecs);
    static QMetaObject::Connection addTimerConnection(QTimer* handle, const FAction& onTimeout);
    static void removeTimerConnection(const QMetaObject::Connection& connection);
    static ThreadTimerManager& getInstance();
    static AsyncResult deleteTimer(QTimer* timerHandle);
    void terminate();

private:
    ArrayPointers<class QTimer> m_timers;
    ScopedPointer<QThread> m_thread;
    ScopedPointer<QObject> m_threadWorker;
    bool m_isTerminated;
};

class ThreadTimer
{
public:
    ThreadTimer(qint32 msecs);
    ~ThreadTimer();

    static void SingleShot(qint32 msecs, const FAction& onTimeout) { ThreadTimerManager::SingleShot(msecs, onTimeout); }
    static void SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker) { ThreadTimerManager::SingleShotDoThreadWorker(msecs, onTimeout, threadWorker); }
    static void SingleShotDoMain(qint32 msecs, const FAction& onTimeout) { ThreadTimerManager::SingleShotDoMain(msecs, onTimeout); }

    QMetaObject::Connection OnTimeout(const FAction& action);
    QThread* GetThread() const { return ThreadTimerManager::getInstance().m_thread.get(); }

    QTimer* GetTimer() const { return m_handle; }

private:
    QTimer* m_handle;
    QtLambdaConnections m_connections;
};

#endif // THREADTIMER_H
