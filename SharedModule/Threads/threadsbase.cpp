#include "threadsbase.h"

#include <QThread>

#include "SharedModule/external/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::DoMain(const FTask& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, priority);
}

void ThreadsBase::DoQThread(QThread* thread, const FTask& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, thread, priority);
}

AsyncResult ThreadsBase::Async(const FTask& task)
{
    return ThreadFunction::Async(task);
}
