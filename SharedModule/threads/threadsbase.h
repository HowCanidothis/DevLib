#ifndef THREADSBASE_H
#define THREADSBASE_H

#include <functional>

#include <Qt>

class ThreadsBase
{
    ThreadsBase();
public:

    static void DoMain(const std::function<void ()>& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void DoQThread(class QThread* thread, const std::function<void ()>& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void Async(const std::function<void ()>& function);
};

#endif // THREADSBASE_H
