#ifndef TMEMORYMANAGER_H
#define TMEMORYMANAGER_H

#include <QHash>
#include <QString>
#include <QMutex>
#include <typeinfo>

template<class T> class MemorySpy;

class MemoryManager {
private:
    static QMutex& mutex()
    {
        static QMutex mutex;
        return mutex;
    };
    static QHash<size_t,const char*>& dictionary()
    {
        static QHash<size_t,const char*> ret;
        return ret;
    }
    static QHash<size_t,qint32>& created()
    {
        static QHash<size_t,qint32> ret;
        return ret;
    }
    static QHash<size_t,qint32>& destroyed()
    {
        static QHash<size_t,qint32> ret;
        return ret;
    }

    static qint32 shouldBe(size_t index);
    static const char* typeName(size_t _type);
public:

    static void MakeMemoryReport();

    template<class T> friend class MemorySpy;
};

template<class T>
class MemorySpy
{
public:
    MemorySpy(){
        MemoryManager::mutex().lock();
        size_t id = typeid(T).hash_code();
        if(!MemoryManager::dictionary().contains(id))
            MemoryManager::dictionary()[id] = typeid(T).name();
        MemoryManager::created()[id]++;
        MemoryManager::mutex().unlock();
    }

    MemorySpy(const MemorySpy&)
        : MemorySpy()
    {
    }

    virtual ~MemorySpy(){
        MemoryManager::mutex().lock();
        MemoryManager::destroyed()[typeid(T).hash_code()]++;
        MemoryManager::mutex().unlock();
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
