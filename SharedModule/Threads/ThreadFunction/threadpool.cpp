#include "threadpool.h"
#include "thread.h"

#include <QMutexLocker>

ThreadPool::ThreadPool()
{
    for(auto& thread : _threads) {
        thread = new Thread(this);
        _freeThreads.push_back(thread);
    }
}

void ThreadPool::pushTask(ThreadTaskDesc* task)
{
    QMutexLocker locker(&_taskMutex);
    if(!_freeThreads.empty()) {
        auto thread = _freeThreads.front();
        _freeThreads.pop_front();
        thread->RunTask(task);
    } else {
        _tasks.push_back(task);
    }
}

void ThreadPool::await()
{
    QMutexLocker locker(&_taskMutex);
    while(!_tasks.empty()) {
        _awaitCondition.wait(&_taskMutex);
    }
}

ThreadTaskDesc* ThreadPool::takeTask()
{
    if(_tasks.empty()) {
        return nullptr;
    }
    QMutexLocker locker(&_taskMutex);
    auto result = _tasks.front();
    _tasks.pop_front();
    return result;
}

void ThreadPool::markFree(Thread* thread)
{
    QMutexLocker locker(&_taskMutex);
    _freeThreads.push_back(thread);

    if(_freeThreads.size() == ThreadsCount) {
        _awaitCondition.wakeAll();
    }
}
