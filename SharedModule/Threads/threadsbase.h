#ifndef THREADSBASE_H
#define THREADSBASE_H

#include <functional>

#include <Qt>
#include <QThread>
#include <QCoreApplication>

#include "SharedModule/shared_decl.h"
#include "SharedModule/Threads/Promises/promise.h"

class _Export ThreadsBase
{
    ThreadsBase();
public:

    static bool IsTerminated();
    static AsyncResult DoMainWithResult(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMain(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMainAwait(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThreadWorker(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult DoQThreadWorkerWithResult(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task, EPriority priority = EPriority::High);
    static void TerminateAllAsyncTasks();
    static void AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task);
};

using ThreadHandlerNoThreadCheck = std::function<AsyncResult (const FAction& action)>;
using ThreadHandler = std::function<AsyncResult (const FAction& action)>;

const ThreadHandlerNoThreadCheck ThreadHandlerNoCheckMainLowPriority = [](const FAction& action) -> AsyncResult {
    return ThreadsBase::DoMainWithResult(action, Qt::LowEventPriority);
};

const ThreadHandler ThreadHandlerMain = [](const FAction& action) -> AsyncResult {
    if(QThread::currentThread() == QCoreApplication::instance()->thread()) {
        action();
        AsyncResult result;
        result.Resolve(true);
        return result;
    } else {
        return ThreadsBase::DoMainWithResult(action);
    }
};


#define THREAD_ASSERT_IS_THREAD(thread) Q_ASSERT(thread == QThread::currentThread());
#define THREAD_ASSERT_IS_NOT_THREAD(thread) Q_ASSERT(thread != QThread::currentThread());
#define THREAD_ASSERT_IS_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_THREAD(qApp->thread()); }
#define THREAD_ASSERT_IS_NOT_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_NOT_THREAD(qApp->thread()); }

#endif // THREADSBASE_H
