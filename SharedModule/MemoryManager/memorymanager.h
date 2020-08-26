#ifndef TMEMORYMANAGER_H
#define TMEMORYMANAGER_H

#include <QHash>
#include <QString>
#include <QMutex>
#include <typeinfo>

template<class T> class MemorySpy;

class MemoryManager
{
    QMutex m_mutex;
    QHash<size_t, const char*> m_dictionary;
    QHash<size_t, qint32> m_created;
    QHash<size_t, qint32> m_destroyed;
    qint32 shouldBe(size_t index);
    const char* typeName(size_t _type);
public:
    static MemoryManager& GetInstance(){ static MemoryManager* manager = new MemoryManager(); return *manager; }
    void MakeMemoryReport();

    template<class T> friend class MemorySpy;
};

template<class T>
class MemorySpy
{
public:
    MemorySpy(){
        auto& m = MemoryManager::GetInstance();
        m.m_mutex.lock();
        size_t id = typeid(T).hash_code();
        if(!m.m_dictionary.contains(id)) {
            m.m_dictionary[id] = typeid(T).name();
        }
        m.m_created[id]++;
        m.m_mutex.unlock();
    }

    MemorySpy(const MemorySpy&)
        : MemorySpy()
    {
    }

    virtual ~MemorySpy(){
        auto& m = MemoryManager::GetInstance();
        m.m_mutex.lock();
        m.m_destroyed[typeid(T).hash_code()]++;
        m.m_mutex.unlock();
    }
};

#ifdef QT_PROFILE
#define ATTACH_MEMORY_SPY(x) : MemorySpy<x>
#define ATTACH_MEMORY_SPY_2(x) , MemorySpy<x>
#else
#define ATTACH_MEMORY_SPY(x)
#define ATTACH_MEMORY_SPY_2(x)
#endif

#endif // TMEMORYMANAGER_H
