#ifndef THREADTIMER_H
#define THREADTIMER_H

#include <QTimer>

#include "SharedModule/shared_decl.h"
#include "SharedModule/array.h"

class ThreadTimer
{
    ThreadTimer();
    ~ThreadTimer();
public:
    using TimerHandle = void*;

    static ThreadTimer& GetInstance();

    static void SingleShot(qint32 msecs, const FAction& onTimeout);
    static TimerHandle CreateTimer(qint32 msecs);
    static void DeleteTimer(TimerHandle* timerHandle);
    static QMetaObject::Connection AddTimerConnection(TimerHandle handle, const FAction& onTimeout);
    static void RemoveTimerConnection(const QMetaObject::Connection& connection);

private:
    ArrayPointers<class QTimer> m_timers;
    ScopedPointer<QThread> m_thread;
    ScopedPointer<QObject> m_threadWorker;
};

#endif // THREADTIMER_H
