#include "memorymanager.h"
#include "SharedModule/internal.hpp"

qint32 MemoryManager::shouldBe(size_t index)
{
    return m_created[index] - m_destroyed[index];
}

const char *MemoryManager::typeName(size_t _type)
{
    return m_dictionary[_type];
}

void MemoryManager::registerSpy(size_t key, void* spy, const char* name)
{
#ifdef ENABLE_MEMORY_MANAGER
    bool traced;
    {
        QMutexLocker locker(&m_mutex);
        if(!m_dictionary.contains(key)) {
            m_dictionary[key] = name;
        }
        m_created[key]++;
        traced = m_tracedIds.contains(m_idGenerator);
        m_spies[key].insert({ spy, m_idGenerator++ });
    }
    Q_ASSERT(!traced);
#endif
}

void MemoryManager::unregiterSpy(size_t key, void* spy)
{
#ifdef ENABLE_MEMORY_MANAGER
    QMutexLocker locker(&m_mutex);
    m_destroyed[key]++;
    m_spies[key].remove({ spy, 0 });
#endif
}

void MemoryManager::MakeMemoryReport()
{
#ifdef ENABLE_MEMORY_MANAGER
    qCDebug(LC_SYSTEM) << "----------------------------MemoryReport------------------------";
    QMutexLocker locker(&m_mutex);
    QHashIterator<size_t,qint32> i(m_created);
    while(i.hasNext()){
        i.next();
        if(shouldBe(i.key())){
            QString idsString("{");
            const auto& spies = m_spies[i.key()];
            if(spies.size() < 20) {
                for(const auto& spy : spies) {
                    idsString += QString::number(spy.Id) + ", ";
                }
            }
            idsString += "}";
            qCDebug(LC_SYSTEM) << typeName(i.key()) << "constructed:" << i.value() << "destructed:" << m_destroyed.value(i.key()) << idsString;
        }
    }
    qCDebug(LC_SYSTEM) << "----------------------------------------------------------------";
#endif
}






