#include "threadfunction.h"

#include "threadpool.h"
#include "thread.h"

AsyncResult ThreadFunction::Async(const FAction& function)
{
    auto desc = new ThreadTaskDesc{ function };
    auto result = desc->Result;
    threadPool().pushTask(desc);
    return result;
}

ThreadPool& ThreadFunction::threadPool()
{
    static ThreadPool res;
    return res;
}
