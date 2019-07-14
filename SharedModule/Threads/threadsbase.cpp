#include "threadsbase.h"

#include <QThread>

#include "SharedModule/External/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::DoMain(const FAction& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, priority);
}

void ThreadsBase::DoQThread(QThread* thread, const FAction& task, Qt::EventPriority priority)
{
    QtInlineEvent::Post(task, thread, priority);
}

AsyncResult ThreadsBase::Async(const FAction& task)
{
    return ThreadFunction::Async(task);
}
