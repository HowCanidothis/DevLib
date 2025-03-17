#include "threadsbase.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "SharedModule/External/qtinlineevent.h"
#include "ThreadFunction/threadfunction.h"
#include "ThreadFunction/threadpool.h"
#include "threads_declarations.h"
#include "SharedModule/ImportExport/importexport.h"

//#define ENABLE

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
        AsyncResult result;
        QMutexLocker locker(&m_mutex);
        if(m_isTerminated) {
            return result;
        }
        m_tasks.append({cdl, result, task});
        QtInlineEvent::Post(CDL, [this]{
            ProcessEvents();
        }, Qt::LowEventPriority);
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
        QMutexLocker locker(&m_mutex);
        m_tasks.clear();
    }

    void ProcessEvents()
    {
        if(m_isBlocked) {
            QtInlineEvent::Post(CDL, [this]{
                ProcessEvents();
            }, Qt::LowEventPriority);
            return;
        }

        if(m_isTerminated) {
            return;
        }

        if(!m_tasks.isEmpty()) {
            TaskInfo task;
            {
            QMutexLocker locker(&m_mutex);
                task = m_tasks.constFirst();
                m_tasks.pop_front();
            }
            try{
                task.Task();
                task.Result.Resolve(true);
            } catch (...) {
                task.Result.Resolve(false);
            }
        }
    }

private:
    bool m_isBlocked;
    bool m_isTerminated;
    QMutex m_mutex;
    QQueue<TaskInfo> m_tasks;
};

MainThreadTaskContainer tasks;

ThreadsBase::ThreadsBase()
{

}

void ThreadsBase::Initialize()
{
#ifdef ENABLE
//    tasks.ProcessEvents();
#endif
    ThreadFunction::threadPool();
    ImportExport::threadPool();
}

void ThreadsBase::ProcessUiOnly()
{
#ifdef ENABLE
    THREAD_ASSERT_IS_MAIN();
    tasks.Block();
    qApp->processEvents();
    tasks.Unblock();
#endif
}

void ThreadsBase::ProcessMainEvents()
{
#ifdef ENABLE
    tasks.Unblock();
#endif
    qApp->processEvents();
}

AsyncResult ThreadsBase::DoMainWithResult(const char* location, const FAction& task, Qt::EventPriority priority)
{
#ifdef ENABLE
    return tasks.Push(location, task);
#else
    return QtInlineEventWithResult::Post(location, task, priority);
#endif
}

void ThreadsBase::DoMain(const char* location, const FAction& task, Qt::EventPriority priority)
{
#ifdef ENABLE
    tasks.Push(location, task);
#else
    QtInlineEvent::Post(location, task, priority);
#endif
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
#ifdef ENABLE
        result += tasks.Push(location, task);
#else
        result += QtInlineEventWithResult::Post(location, task, priority);
#endif
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
#ifdef ENABLE
    tasks.Terminate();
#endif
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
