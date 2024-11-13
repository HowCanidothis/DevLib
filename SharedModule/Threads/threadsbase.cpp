#include "threadsbase.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "SharedModule/External/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"
#include "ThreadFunction/threadpool.h"
#include "threads_declarations.h"
#include "SharedModule/ImportExport/importexport.h"

struct TaskInfo
{
    const char* Location;
    AsyncResult Result;
    FAction Task;
};

class MainThreadTaskContainer
{
public:
    MainThreadTaskContainer()
        : m_isBlocked(false)
        , m_isTerminated(false)
    {}

    AsyncResult Push(const char* cdl, const FAction& task)
    {
        m_mutex.lock();
        AsyncResult result;
        m_tasks.push({cdl, result, task });
        m_mutex.unlock();
        return result;
    }

    void Block()
    {
        m_isBlocked = true;
    }

    void Unblock()
    {
        m_isBlocked = false;
    }

    void Terminate()
    {
        m_isTerminated = true;
    }

    void ProcessEvents()
    {
        if(m_isTerminated) {
            return;
        }

        if(!m_isBlocked) {
            while(m_tasks.size() != 0) {

                TaskInfo task;
                {
                    QMutexLocker locker(&m_mutex);
                    task = m_tasks.front();
                    m_tasks.pop();
                }
                try{
                    task.Task();
                    task.Result.Resolve(true);
                } catch (...) {
                    task.Result.Resolve(false);
                }

            }
        }

        QtInlineEvent::Post(CDL, [this]{
            ProcessEvents();
        }, Qt::LowEventPriority);
    }

private:
    bool m_isBlocked;
    bool m_isTerminated;
    QMutex m_mutex;
    std::queue<TaskInfo> m_tasks;
};

MainThreadTaskContainer tasks;

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::Initialize()
{
    tasks.ProcessEvents();
    ThreadFunction::threadPool();
    ImportExport::threadPool();
}

void ThreadsBase::ProcessUiOnly()
{
    tasks.Block();
    qApp->processEvents();
    tasks.Unblock();
}

AsyncResult ThreadsBase::DoMainWithResult(const char* location, const FAction& task, Qt::EventPriority priority)
{
    return tasks.Push(location, task);
//    return QtInlineEventWithResult::Post(location, task, priority);
}

void ThreadsBase::DoMain(const char* location, const FAction& task, Qt::EventPriority priority)
{
    tasks.Push(location, task);
//    QtInlineEvent::Post(location, task, priority);
}

void ThreadsBase::DoMainAwait(const char* location, const FAction &task, Qt::EventPriority priority)
{
    if(QCoreApplication::instance() == nullptr) {
        return;
    }
    if(QThread::currentThread() == qApp->thread()) {
        task();
    } else {
        FutureResult result;
        result += tasks.Push(location, task);
        result.Wait();
    }
}

bool ThreadsBase::IsTerminated()
{
    return ThreadFunction::threadPool().IsTerminated();
}

void ThreadsBase::Terminate()
{
    ThreadFunction::threadPool().Terminate();
    ImportExport::threadPool().Terminate();
    tasks.Terminate();
}
// Due to Qt arch we have to use threadWorker here
void ThreadsBase::DoQThreadWorker(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority)
{
    Q_ASSERT(!qobject_cast<QThread*>(threadObject));

    QtInlineEvent::Post(location, task, threadObject, priority);
}
// Due to Qt arch we have to use threadWorker here
AsyncResult ThreadsBase::DoQThreadWorkerWithResult(const char* location, QObject* threadObject, const FAction& task, Qt::EventPriority priority)
{
    Q_ASSERT(!qobject_cast<QThread*>(threadObject));

    return QtInlineEventWithResult::Post(location, task, threadObject, priority);
}

AsyncResult ThreadsBase::Async(const FAction& task, EPriority priority)
{
    return ThreadFunction::Async(task, priority);
}

void ThreadsBase::AsyncSemaphore(const SharedPointer<FutureResult>& result, const FAction& task)
{
    *result += Async([result, task]{
        task();
    });
}
