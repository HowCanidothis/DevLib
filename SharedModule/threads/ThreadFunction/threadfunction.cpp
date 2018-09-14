#include "threadfunction.h"

#include "threadpool.h"
#include "thread.h"

void ThreadFunction::Async(const FTask& function)
{
    threadPool().pushTask(new ThreadTaskDesc{ function, []{} });
}

void ThreadFunction::Async(const FTask& function, const FOnFinish& onFinish)
{
    threadPool().pushTask(new ThreadTaskDesc{ function, onFinish });
}

ThreadPool& ThreadFunction::threadPool()
{
    static ThreadPool res;
    return res;
}
