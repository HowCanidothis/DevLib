#ifndef THREADSBASE_H
#define THREADSBASE_H

#include <functional>

#include <Qt>

#include "SharedModule/shared_decl.h"
#include "SharedModule/Threads/Promises/promise.h"

class _Export ThreadsBase
{
    ThreadsBase();
public:

    static bool IsTerminated();
    static void DoMain(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMainAwait(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThreadWorker(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult DoQThreadWorkerWithResult(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task);
    static void TerminateAllAsyncTasks();
    static void AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task);
};

#define THREAD_ASSERT_IS_THREAD(thread) Q_ASSERT(thread == QThread::currentThread());
#define THREAD_ASSERT_IS_NOT_THREAD(thread) Q_ASSERT(thread != QThread::currentThread());
#define THREAD_ASSERT_IS_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_THREAD(qApp->thread()); }
#define THREAD_ASSERT_IS_NOT_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_NOT_THREAD(qApp->thread()); }

#endif // THREADSBASE_H
