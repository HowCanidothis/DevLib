#ifndef THREADSHANDLER_H
#define THREADSHANDLER_H

#include "SharedModule/declarations.h"
#include "SharedModule/smartpointersadapters.h"

class ThreadsHandler
{
public:
    using FEventLoopHandler = std::function<void (const char*, const FAction&)>;
    ThreadsHandler();
    ~ThreadsHandler();

    void Destroy();
    void MoveToThread(QObject* thread);
    void MoveToThread(QObject* thread, const FEventLoopHandler& customEventLoopHandler);

    void Call(const char* connectionInfo, const FAction& a) const;

private:
    void setDefaultEventLoopHandler();

private:
    QObject* m_thread;
    FEventLoopHandler m_eventLoopHandler;
    SharedPointer<class ThreadsHandlerData> m_data;
};

#endif // THREADSHANDLER_H
