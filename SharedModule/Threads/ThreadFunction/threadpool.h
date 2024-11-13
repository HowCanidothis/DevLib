#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <deque>
#include <QWaitCondition>
#include <QMutex>

#include "SharedModule/stack.h"
#include "SharedModule/Threads/Promises/promise.h"

class Thread;
struct ThreadTaskDesc;

class ThreadPool
{
public:
    ThreadPool(qint32 threadsCount = 4);
    virtual ~ThreadPool();

public:
    bool IsTerminated() const;
    void Terminate();
    // Note: ThreadTaskDesc is managed by ThreadPool and will be freed when task is performed
    AsyncResult PushTask(const FAction& function, EPriority priority);
    void Await();

private:
    friend class Thread;
    friend class ThreadFunction;

    ThreadTaskDesc* takeTask();
    void markFree(Thread* thread);

protected:
    StackPointers<Thread> m_threads;
    std::deque<Thread*> m_freeThreads; // TODO. Can be optimized in the future
    std::deque<ThreadTaskDesc*> m_tasks[(qint32)EPriority::Count];
    QMutex m_taskMutex;
    QWaitCondition m_awaitCondition;
};

#endif // THREADPOOL_H
