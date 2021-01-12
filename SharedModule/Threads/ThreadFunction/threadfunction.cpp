#include "threadfunction.h"

#include "threadpool.h"
#include "thread.h"

AsyncResult ThreadFunction::Async(const FAction& function, EPriority priority)
{
    return threadPool().PushTask(function, priority);
}

ThreadPool& ThreadFunction::threadPool()
{
    static ThreadPool res;
    return res;
}
