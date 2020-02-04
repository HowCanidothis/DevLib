#include "networkthreadpool.h"
#include "networkthread.h"

NetworkThreadPool::NetworkThreadPool(NetworkServerBase* server, qint32 threadsCount)
{
    Q_ASSERT(threadsCount > 0);
    while(threadsCount--) {
        m_threads.Append(new NetworkThread(server));
    }
}

NetworkThreadPool::~NetworkThreadPool()
{

}

bool NetworkThreadPool::IsRunning() const
{
    return m_threads.First()->isRunning();
}

void NetworkThreadPool::Start()
{
    for(auto* thread : m_threads) {
        thread->start();
    }
}

void NetworkThreadPool::Quit()
{
    for(auto* thread : m_threads) {
        thread->quit();
    }
}

void NetworkThreadPool::AddSocket(qintptr id)
{
    auto* associatedThread = findEmtiestThread();
    m_socketsMap[id] = associatedThread;
    associatedThread->AddSocket(id);
}

void NetworkThreadPool::Write(qintptr id, const NetworkPackage& package)
{
    auto foundIt = m_socketsMap.find(id);
    Q_ASSERT(foundIt != m_socketsMap.end());
    foundIt.value()->Write(id, package);
}

NetworkThread* NetworkThreadPool::findEmtiestThread() const
{
    NetworkThread* result = m_threads.First();
    for(auto* thread : adapters::range(m_threads.begin() + 1, m_threads.end())) {
        if(thread->GetConnectionsCount() < result->GetConnectionsCount()) {
            result = thread;
        }
    }
    return result;
}
