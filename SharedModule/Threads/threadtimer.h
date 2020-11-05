#ifndef THREADTIMER_H
#define THREADTIMER_H

#include <QTimer>

#include "SharedModule/shared_decl.h"
#include "SharedModule/array.h"
#include "SharedModule/smartpointersadapters.h"

class ThreadTimerHandle
{
    friend class ThreadTimer;
    ThreadTimerHandle(QTimer* handle)
        : m_handle(handle)
    {}
public:
    ~ThreadTimerHandle();

    QTimer* GetTimer() const { return m_handle; }

private:
    QTimer* m_handle;
};
using ThreadTimerHandlePtr = SharedPointer<ThreadTimerHandle>;

class ThreadTimer
{
    ThreadTimer();
    ~ThreadTimer();
public:
    static void Initialize() { getInstance(); }

    static void SingleShot(qint32 msecs, const FAction& onTimeout);
    static void SingleShotDoThreadWorker(qint32 msecs, const FAction& onTimeout, QObject* threadWorker);
    static void SingleShotDoMain(qint32 msecs, const FAction& onTimeout);
    static ThreadTimerHandlePtr CreateTimer(qint32 msecs);
    static QMetaObject::Connection AddTimerConnection(const ThreadTimerHandlePtr& handle, const FAction& onTimeout);
    static void RemoveTimerConnection(const QMetaObject::Connection& connection);

private:
    friend class ThreadTimerHandle;
    static ThreadTimer& getInstance();
    static void deleteTimer(QTimer* timerHandle);

private:
    ArrayPointers<class QTimer> m_timers;
    ScopedPointer<QThread> m_thread;
    ScopedPointer<QObject> m_threadWorker;
};

#endif // THREADTIMER_H
