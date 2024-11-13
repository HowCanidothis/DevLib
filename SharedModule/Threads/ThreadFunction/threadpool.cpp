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

bool ThreadPool::IsTerminated() const
{
    return m_threads.IsEmpty();
}

// Must be called on application close
void ThreadPool::Terminate()
{
    for(Thread* t : m_threads) {
        t->terminate();
    }
    m_threads.Clear();
    m_freeThreads.clear();
}

AsyncResult ThreadPool::PushTask(const FAction& function, EPriority priority)
{
    auto* task = new ThreadTaskDesc { function };
    auto result = task->Result;
    QMutexLocker locker(&m_taskMutex);
    if(!m_freeThreads.empty()) {
        auto thread = m_freeThreads.front();
        m_freeThreads.pop_front();
        thread->RunTask(task);
    } else {
        m_tasks[(qint32)priority].push_back(task);
    }
    return result;
}

void ThreadPool::Await()
{
    QMutexLocker locker(&m_taskMutex);
    while(!m_tasks[(qint32)EPriority::Low].empty() || !m_tasks[(qint32)EPriority::High].empty()) {
        m_awaitCondition.wait(&m_taskMutex);
    }
}

ThreadTaskDesc* ThreadPool::takeTask()
{
    QMutexLocker locker(&m_taskMutex);
    if(!m_tasks[(qint32)EPriority::High].empty()) {
        auto result = m_tasks[(qint32)EPriority::High].front();
        m_tasks[(qint32)EPriority::High].pop_front();
        return result;
    }    
    if(!m_tasks[(qint32)EPriority::Low].empty()) {
        auto result = m_tasks[(qint32)EPriority::Low].front();
        m_tasks[(qint32)EPriority::Low].pop_front();
        return result;
    }
    return nullptr;
}

void ThreadPool::markFree(Thread* thread)
{
    QMutexLocker locker(&m_taskMutex);
    m_freeThreads.push_back(thread);

    if(m_freeThreads.size() == m_threads.Size()) {
        m_awaitCondition.wakeAll();
    }
}
