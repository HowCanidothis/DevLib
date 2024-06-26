#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "SharedModule/smartpointersadapters.h"
#include "SharedModule/dispatcher.h"
#include "threadfunction.h"

struct ThreadTaskDesc
{
    FAction Task;
    AsyncResult Result;

    ThreadTaskDesc(){}
    ThreadTaskDesc(const FAction& action)
        : Task(action)
    {}
};

class Thread : public QThread
{
public:
    Thread(class ThreadPool* pool);
    ~Thread();
    AsyncResult RunTask(ThreadTaskDesc* task);

    Dispatcher OnFinished;

    // QThread interface
protected:
    virtual void run() Q_DECL_OVERRIDE;

private:
    class ThreadPool* m_pool;
    ScopedPointer<ThreadTaskDesc> m_task;
    QMutex m_taskMutex;
    QWaitCondition m_taskCondition;
    std::atomic_bool m_aboutToBeDestroyed;
};



#endif // THREAD_H
