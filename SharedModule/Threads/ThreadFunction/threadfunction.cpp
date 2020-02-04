#include "threadfunction.h"

#include "threadpool.h"
#include "thread.h"

AsyncResult ThreadFunction::Async(const FAction& function)
{
    return threadPool().PushTask(function);
}

ThreadPool& ThreadFunction::threadPool()
{
    static ThreadPool res;
    return res;
}
