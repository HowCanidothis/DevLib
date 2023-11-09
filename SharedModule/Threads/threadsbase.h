#ifndef THREADSBASE_H
#define THREADSBASE_H

#include <functional>

#include <Qt>
#include <QThread>
#include <QCoreApplication>

#include "SharedModule/declarations.h"
#include "SharedModule/Threads/Promises/promise.h"

class _Export ThreadsBase
{
    ThreadsBase();
public:

    static bool IsTerminated();
    static AsyncResult DoMainWithResult(const char* location, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    template<class T>
    static void FreeAtMainThread(const SharedPointer<T>& ptr)
    {
        auto* pPtr = new SharedPointer<T>(ptr);
        ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[pPtr]{ delete pPtr; });
    }
    template<class T>
    static void FreeAtMainThread(SharedPointer<T>& ptr)
    {
        auto* pPtr = new SharedPointer<T>(ptr);
        ptr = nullptr;
        ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[pPtr]{ delete pPtr; });
    }
    template<class T>
    static void FreeAtMainThreadAwait(SharedPointer<T>& ptr)
    {
        auto* pPtr = new SharedPointer<T>(ptr);
        ptr = nullptr;
        ThreadsBase::DoMainAwait(CONNECTION_DEBUG_LOCATION,[pPtr]{ delete pPtr; });
    }
    static void DoMain(const char* location, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoMainAwait(const char* location, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThreadWorker(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult DoQThreadWorkerWithResult(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task, EPriority priority = EPriority::High);
    static void TerminateAllAsyncTasks();
    static void AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task);
};

#endif // THREADSBASE_H
