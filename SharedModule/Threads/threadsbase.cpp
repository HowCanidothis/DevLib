#include "threadsbase.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "SharedModule/External/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"
#include "ThreadFunction/threadpool.h"
#include "threads_declarations.h"
#include "SharedModule/ImportExport/importexport.h"

ThreadsBase::ThreadsBase()
{

}

AsyncResult ThreadsBase::DoMainWithResult(const char* location, const FAction& task, Qt::EventPriority priority)
{
    return QtInlineEventWithResult::Post(location, task, priority);
}

void ThreadsBase::DoMain(const char* location, const FAction& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(location, task, priority);
}

void ThreadsBase::DoMainAwait(const char* location, const FAction &task, Qt::EventPriority priority)
{
    if(QCoreApplication::instance() == nullptr) {
        return;
    }
    if(QThread::currentThread() == QCoreApplication::instance()->thread()) {
        task();
    } else {
        FutureResult result;
        result += QtInlineEventWithResult::Post(location, task, priority);
        result.Wait();
    }
}

bool ThreadsBase::IsTerminated()
{
    return ThreadFunction::threadPool().IsTerminated();
}

void ThreadsBase::TerminateAllAsyncTasks()
{
    ThreadFunction::threadPool().TerminateAll();
    ImportExport::threadPool().TerminateAll();
}
// Due to Qt arch we have to use threadWorker here
void ThreadsBase::DoQThreadWorker(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority)
{
    Q_ASSERT(!qobject_cast<QThread*>(threadObject));

    QtInlineEvent::Post(location, task, threadObject, priority);
}
// Due to Qt arch we have to use threadWorker here
AsyncResult ThreadsBase::DoQThreadWorkerWithResult(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority)
{
    Q_ASSERT(!qobject_cast<QThread*>(threadObject));

    return QtInlineEventWithResult::Post(location, task, threadObject, priority);
}

AsyncResult ThreadsBase::Async(const FAction& task, EPriority priority)
{
    return ThreadFunction::Async(task, priority);
}

void ThreadsBase::AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task)
{
    *result += Async([result, task]{
        task();
    });
}
