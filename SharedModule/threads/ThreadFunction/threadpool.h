#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <deque>
#include <QWaitCondition>
#include <QMutex>

class Thread;
struct ThreadTaskDesc;

class ThreadPool
{
    enum Threads{
        ThreadsCount = 4
    };
public:
    ThreadPool();

private:
    friend class Thread;
    friend class ThreadFunction;
    void pushTask(ThreadTaskDesc* task);
    void await();

    ThreadTaskDesc* takeTask();
    void markFree(Thread* thread);

private:
    Thread* _threads[ThreadsCount];
    std::deque<Thread*> _freeThreads; // TODO. Can be optimized in the future
    std::deque<ThreadTaskDesc*> _tasks;
    QMutex _taskMutex;
    QWaitCondition _awaitCondition;
};

#endif // THREADPOOL_H
