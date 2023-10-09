#include "threadshandler.h"

#include <QMutex>
#include <QThread>
#include <QCoreApplication>

#include "SharedModule/External/qtinlineevent.h"

class ThreadsHandlerData
{
public:
    ThreadsHandlerData()
        : Alive(true)
    {}

private:
    friend class ThreadsHandler;

    bool Alive;
    QMutex Mutex;
};

static thread_local QObject DefaultThreadObject;

ThreadsHandler::ThreadsHandler()
    : m_thread(&DefaultThreadObject)
{
    setDefaultEventLoopHandler();
}

ThreadsHandler::~ThreadsHandler()
{
}

void ThreadsHandler::Destroy()
{
    QMutexLocker locker(&m_data->Mutex);
    m_data->Alive = false;
}

void ThreadsHandler::MoveToThread(QObject* thread)
{
    m_thread = thread->thread();
    setDefaultEventLoopHandler();
}

void ThreadsHandler::MoveToThread(QObject* thread, const FEventLoopHandler& customEventLoopHandler)
{
    m_thread = thread;
    m_eventLoopHandler = customEventLoopHandler;
}

void ThreadsHandler::Call(const char* connectionInfo, const FAction& a) const
{
    if(QThread::currentThread() == m_thread) {
        a();
    } else {
        m_eventLoopHandler(connectionInfo, a);
    }
}

void ThreadsHandler::setDefaultEventLoopHandler()
{
    m_eventLoopHandler = [this](const char* c, const FAction& a){
        auto d = m_data;
        auto* e = new QtInlineEvent(c, [d, a] {
            {
                QMutexLocker m(&d->Mutex);
                if(!d->Alive) {
                    return;
                }
            }
            a();
        });
        QCoreApplication::postEvent(m_thread, e);
    };
}
