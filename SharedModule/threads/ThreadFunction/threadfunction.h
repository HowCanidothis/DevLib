#ifndef THREADFUNCTION_H
#define THREADFUNCTION_H

#include <functional>

#include "SharedModule/shared_decl.h"
#include "SharedModule/Threads/Promises/promise.h"

typedef std::function<void ()> FTask;

class ThreadFunction
{
public:
    _Export static AsyncResult* Async(const FTask& function);

private:
    static class ThreadPool& threadPool();
};

#endif // THREADFUNCTION_H
