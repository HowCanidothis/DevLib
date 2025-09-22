#ifndef TMEMORYMANAGER_H
#define TMEMORYMANAGER_H

#include <QHash>
#include <QString>
#include <QMutex>
#include <QSet>
#include <typeinfo>
#include <functional>

template<class T> class MemorySpy;

class MemoryManager
{
    struct ObjectInfo
    {
        void* Target;
        size_t Id;

        operator size_t() const { return (size_t)Target; }
    };

    QMutex m_mutex;
    size_t m_idGenerator = 0;
    QHash<size_t, QSet<ObjectInfo>> m_spies;
    QHash<size_t, const char*> m_dictionary;
    QHash<size_t, qint32> m_created;
    QHash<size_t, qint32> m_destroyed;
    QSet<size_t> m_tracedIds;
    qint32 shouldBe(size_t index);
    const char* typeName(size_t _type);
    void registerSpy(size_t key, void* spy, const char* name);
    void unregiterSpy(size_t key, void* spy);
public:
    static MemoryManager& GetInstance(){ static MemoryManager* manager = new MemoryManager(); return *manager; }
    void MakeMemoryReport();

    void TraceCreation(size_t id)
    {
        QMutexLocker locker(&m_mutex);
        m_tracedIds.insert(id);
    }

    template<class T>
    void TestObjects(const std::function<void (T*)>& handler)
    {
        QMutexLocker locker(&m_mutex);
        auto foundIt = m_spies.find(typeid(T).hash_code());
        if(foundIt != m_spies.end()) {
            for(const auto& spy : foundIt.value()) {
                auto* concreteSpy = reinterpret_cast<T*>(spy.Target);
                auto tSpy = dynamic_cast<T*>(concreteSpy);
                if(tSpy != nullptr) {
                    handler(tSpy);
                }
            }
        }

    }

    template<class T> friend class MemorySpy;
};

template<class T>
class MemorySpy
{
public:
    MemorySpy(){
        MemoryManager::GetInstance().registerSpy(typeid(T).hash_code(), this, typeid(T).name());
    }

    MemorySpy(const MemorySpy&)
        : MemorySpy()
    {
    }

    virtual ~MemorySpy(){
        MemoryManager::GetInstance().unregiterSpy(typeid(T).hash_code(), this);
    }
};

#ifdef SHARED_LIB_ENABLE_MEMORY_MANAGER
#define ATTACH_MEMORY_SPY(x) : MemorySpy<x>
#define ATTACH_MEMORY_SPY_2(x) , MemorySpy<x>
#else
#define ATTACH_MEMORY_SPY(x)
#define ATTACH_MEMORY_SPY_2(x)
#endif

#endif // TMEMORYMANAGER_H
