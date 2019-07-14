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

    static void DoMain(const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThread(class QThread* thread, const FAction& task, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Async(const FAction& task);
};

#endif // THREADSBASE_H
