#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "SharedModule/smartpointersadapters.h"
#include "threadfunction.h"

struct ThreadTaskDesc
{
    FTask Task;
    FOnFinish OnFinish;
};

class Thread : public QThread
{
public:
    Thread(class ThreadPool* pool);
    void RunTask(ThreadTaskDesc* task);

    // QThread interface
protected:
    virtual void run() Q_DECL_OVERRIDE;

private:
    class ThreadPool* _pool;
    ScopedPointer<ThreadTaskDesc> _task;
    QMutex _taskMutex;
    QWaitCondition _taskCondition;
};



#endif // THREAD_H
