#ifndef THREADFUNCTION_H
#define THREADFUNCTION_H

#include <functional>

#include "SharedModule/declarations.h"
#include "SharedModule/Threads/Promises/promise.h"


class ThreadFunction
{
public:
    _Export static AsyncResult Async(const FAction& function, EPriority priority = EPriority::High);

private:
    friend class ThreadsBase;
    static class ThreadPool& threadPool();
};

#endif // THREADFUNCTION_H
