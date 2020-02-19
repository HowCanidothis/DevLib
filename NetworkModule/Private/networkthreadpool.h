#ifndef NETWORKTHREADPOOL_H
#define NETWORKTHREADPOOL_H

class NetworkThread;

class NetworkThreadPool
{
public:
    NetworkThreadPool(class NetworkServerBase* server, qint32 threadsCount = 4);
    ~NetworkThreadPool();

    bool IsRunning() const;
    void Start();
    void Quit();

    void AddSocket(qintptr id);
    void Write(qintptr id, const class NetworkPackage& package);

private:
    NetworkThread* findEmtiestThread() const;

private:
    StackPointers<NetworkThread> m_threads;
    QHash<qintptr, NetworkThread*> m_socketsMap;
};

#endif // NETWORKTHREADPOOL_H
