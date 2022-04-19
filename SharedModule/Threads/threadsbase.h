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
    template<class T>
    static void FreeAtMainThread(SharedPointer<T>& ptr)
    {
        SharedPointer<SharedPointer<T>> pPtr(new SharedPointer<T>(ptr));
        ptr = nullptr;
        ThreadsBase::DoMain([pPtr]{});
    }
    static void DoMain(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMainAwait(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThreadWorker(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult DoQThreadWorkerWithResult(QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task, EPriority priority = EPriority::High);
    static void TerminateAllAsyncTasks();
    static void AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task);
};

#endif // THREADSBASE_H
