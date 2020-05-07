#include "threadpool.h"
#include "thread.h"

#include <QMutexLocker>

ThreadPool::ThreadPool(qint32 threadsCount)
{
    while(threadsCount--) {
        auto* thread = new Thread(this);
        m_threads.Push(thread);
        m_freeThreads.push_back(thread);
    }
}

ThreadPool::~ThreadPool()
{

}

void ThreadPool::TerminateAll()
{
    m_threads.Clear();
    m_freeThreads.clear();
}

AsyncResult ThreadPool::PushTask(const FAction& function)
{
    auto* task = new ThreadTaskDesc { function };
    auto result = task->Result;
    QMutexLocker locker(&m_taskMutex);
    if(!m_freeThreads.empty()) {
        auto thread = m_freeThreads.front();
        m_freeThreads.pop_front();
        thread->RunTask(task);
    } else {
        m_tasks.push_back(task);
    }
    return result;
}

void ThreadPool::Await()
{
    QMutexLocker locker(&m_taskMutex);
    while(!m_tasks.empty()) {
        m_awaitCondition.wait(&m_taskMutex);
    }
}

ThreadTaskDesc* ThreadPool::takeTask()
{
    QMutexLocker locker(&m_taskMutex);
    if(m_tasks.empty()) {
        return nullptr;
    }    
    auto result = m_tasks.front();
    m_tasks.pop_front();
    return result;
}

void ThreadPool::markFree(Thread* thread)
{
    QMutexLocker locker(&m_taskMutex);
    m_freeThreads.push_back(thread);

    if(m_freeThreads.size() == m_threads.Size()) {
        m_awaitCondition.wakeAll();
    }
}
