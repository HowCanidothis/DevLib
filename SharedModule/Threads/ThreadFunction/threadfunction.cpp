#include "threadfunction.h"

#include "threadpool.h"
#include "thread.h"

AsyncResult ThreadFunction::Async(const FTask& function)
{
    auto desc = new ThreadTaskDesc{ function };
    threadPool().pushTask(desc);
    return desc->Result;
}

ThreadPool& ThreadFunction::threadPool()
{
    static ThreadPool res;
    return res;
}
