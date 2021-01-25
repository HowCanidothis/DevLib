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
    static AsyncResult DoMainWithResult(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMain(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMainAwait(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThreadWorker(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult DoQThreadWorkerWithResult(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task, EPriority priority = EPriority::High);
    static void TerminateAllAsyncTasks();
    static void AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task);
};

template<class T>
DispatcherConnection Promise<T>::ThenMain(const typename PromiseData<T>::FCallback& handler) const {
    return m_data->then([handler](const T& value){
        ThreadsBase::DoMain([handler, value]{
            handler(value);
        });
    });
}

#define THREAD_ASSERT_IS_THREAD(thread) Q_ASSERT(thread == QThread::currentThread());
#define THREAD_ASSERT_IS_NOT_THREAD(thread) Q_ASSERT(thread != QThread::currentThread());
#define THREAD_ASSERT_IS_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_THREAD(qApp->thread()); }
#define THREAD_ASSERT_IS_NOT_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_NOT_THREAD(qApp->thread()); }

#endif // THREADSBASE_H
