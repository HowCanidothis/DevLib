#include "threadsbase.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "SharedModule/External/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"
#include "ThreadFunction/threadpool.h"

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::DoMain(const FAction& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, priority);
}

// TODO. potentially unsafe due to thread pool limitations
void ThreadsBase::DoMainAwait(const FAction &task, Qt::EventPriority priority)
{
    QMutex mutex;
    QWaitCondition waitCondition;
    std::atomic_bool done = false;

    QtInlineEvent::Post([task, &done, &waitCondition]{
        task();
        done = true;
        waitCondition.wakeAll();
    }, priority);

    QMutexLocker locker(&mutex);
    while(!done) { // from spurious wakeups
        waitCondition.wait(&mutex);
    }
}

void ThreadsBase::TerminateAllAsyncTasks()
{
    ThreadFunction::threadPool().TerminateAll();
}

void ThreadsBase::DoQThread(QThread* thread, const FAction& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, thread, priority);
}

AsyncResult ThreadsBase::Async(const FAction& task)
{
    return ThreadFunction::Async(task);
}
