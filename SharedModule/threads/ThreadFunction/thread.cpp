#include "thread.h"

#include <QMutexLocker>

#include "threadpool.h"
#include "threadfunction.h"

Thread::Thread(ThreadPool* pool)
    : _pool(pool)
{
    Q_ASSERT(_pool != nullptr);
    start();
}

void Thread::RunTask(ThreadTaskDesc* task)
{
    QMutexLocker locker(&_taskMutex);
    _task = task;
    _taskCondition.wakeAll();
}

void Thread::run()
{
    {
    waitAgain:
        QMutexLocker locker(&_taskMutex);
        while(_task == nullptr) {
            _taskCondition.wait(&_taskMutex);
        }
    }

    _task->Task();
    _task->OnFinish();

    ThreadTaskDesc* nextTask = _pool->takeTask();
    if(nextTask != nullptr) {
        RunTask(nextTask);
    } else {
        _task = nullptr;
        _pool->markFree(this);
    }

    goto waitAgain;
}
