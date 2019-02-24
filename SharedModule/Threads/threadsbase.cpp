#include "threadsbase.h"

#include <QThread>

#include "SharedModule/external/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::DoMain(const std::function<void ()>& function, Qt::EventPriority priority)
{
    QtInlineEvent::Post(function, priority);
}

void ThreadsBase::DoQThread(QThread* thread, const std::function<void ()>& function, Qt::EventPriority priority)
{
    QtInlineEvent::Post(function, thread, priority);
}

void ThreadsBase::Async(const std::function<void ()>& function)
{
    ThreadFunction::Async(function);
}
