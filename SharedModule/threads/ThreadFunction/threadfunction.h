#ifndef THREADFUNCTION_H
#define THREADFUNCTION_H

#include <functional>

#include "SharedModule/shared_decl.h"

typedef std::function<void ()> FTask;
typedef std::function<void ()> FOnFinish;

class ThreadFunction
{
public:
    _Export static void Async(const FTask& function);
    _Export static void Async(const FTask& function, const FOnFinish& onFinish);

private:
    static class ThreadPool& threadPool();
};

#endif // THREADFUNCTION_H
