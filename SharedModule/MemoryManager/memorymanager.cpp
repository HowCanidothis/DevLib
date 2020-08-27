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

void MemoryManager::MakeMemoryReport()
{
    qCDebug(LC_SYSTEM) << "----------------------------MemoryReport------------------------";
    QMutexLocker locker(&m_mutex);
    QHashIterator<size_t,qint32> i(m_created);
    while(i.hasNext()){
        i.next();
        if(shouldBe(i.key())){
            qCDebug(LC_SYSTEM) << typeName(i.key()) << "constructed:" << i.value() << "destructed:" << m_destroyed.value(i.key());
        }
    }
    qCDebug(LC_SYSTEM) << "----------------------------------------------------------------";
}






