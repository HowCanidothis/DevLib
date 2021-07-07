#include "thread.h"

#include <QMutexLocker>
#include <QAbstractEventDispatcher>

#include "threadpool.h"
#include "threadfunction.h"

Thread::Thread(ThreadPool* pool)
    : m_pool(pool)
    , m_aboutToBeDestroyed(false)
{
    Q_ASSERT(m_pool != nullptr);
    start();
}

Thread::~Thread()
{
    m_aboutToBeDestroyed = true;
    while(isRunning()) {
        QMutexLocker locker(&m_taskMutex);
        m_taskCondition.wakeAll();
    }
}

void Thread::RunTask(ThreadTaskDesc* task)
{
    QMutexLocker locker(&m_taskMutex);
    m_task = task;
    m_taskCondition.wakeAll();
}

void Thread::run()
{
    {
    waitAgain:
        QMutexLocker locker(&m_taskMutex);
        if(m_aboutToBeDestroyed) {
            OnFinished();
            return;
        }
        while(m_task == nullptr) {
            m_taskCondition.wait(&m_taskMutex);
            if(m_aboutToBeDestroyed) {
                OnFinished();
                return;
            }
        }        
    }

    eventDispatcher()->processEvents(QEventLoop::AllEvents);
    
    m_task->Result.Resolve([this]{
        try
        {
            m_task->Task();
            return true;
        }
        catch (...)
        {
            return false;
        }
    });

    ThreadTaskDesc* nextTask = m_pool->takeTask();
    if(nextTask != nullptr) {
        m_task = nextTask;
    } else {
        m_task = nullptr;
        m_pool->markFree(this);
    }

    goto waitAgain;
}
