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
    OnFinished.SetAutoThreadSafe();
    start();
}

Thread::~Thread()
{
    m_aboutToBeDestroyed = true;
    {
        QMutexLocker locker(&m_taskMutex);
        m_taskCondition.wakeAll();
    }

    while (isRunning()) {
        {
            QMutexLocker locker(&m_taskMutex);
            m_taskCondition.wakeAll();
        }
        ThreadsBase::ProcessMainEvents();
        QThread::yieldCurrentThread();
    }

//    wait();
}

AsyncResult Thread::RunTask(ThreadTaskDesc* task)
{
    if(m_aboutToBeDestroyed) {
        return AsyncError();
    }
    QMutexLocker locker(&m_taskMutex);
    m_task = task;
    m_taskCondition.wakeAll();
    return task->Result;
}

void Thread::run()
{
    while (true) {
        ThreadTaskDesc* currentTask = nullptr;

        {
            QMutexLocker locker(&m_taskMutex);

            if (m_aboutToBeDestroyed) {
                OnFinished();
                return;
            }

            while (m_task == nullptr) {
                if (m_aboutToBeDestroyed) {
                    OnFinished();
                    return;
                }
                m_taskCondition.wait(&m_taskMutex);
            }
            currentTask = m_task.get();
        }

        eventDispatcher()->processEvents(QEventLoop::AllEvents);

        currentTask->Result.Resolve([currentTask]{
            try {
                currentTask->Task();
                return true;
            } catch (...) {
                currentTask->Result.SetException(std::current_exception());
                return false;
            }
        });

        {
            QMutexLocker locker(&m_taskMutex);
            ThreadTaskDesc* nextTask = m_pool->takeTask();
            if (nextTask != nullptr) {
                m_task = nextTask;
            } else {
                m_task = nullptr;
                m_pool->markFree(this);
            }

            if (m_aboutToBeDestroyed) {
                OnFinished();
                return;
            }
        }
    }
}
