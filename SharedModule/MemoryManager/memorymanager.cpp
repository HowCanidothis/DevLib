#include "MemoryManager.h"
#include "SharedModule/internal.hpp"

qint32 MemoryManager::shouldBe(size_t index)
{
    return MemoryManager::created()[index] - MemoryManager::destroyed()[index];
}

const char *MemoryManager::typeName(size_t _type)
{
    return MemoryManager::dictionary()[_type];
}

void MemoryManager::MakeMemoryReport()
{
    qCDebug(LC_SYSTEM) << "----------------------------MemoryReport------------------------";
    QHashIterator<size_t,qint32> i(created());
    while(i.hasNext()){
        i.next();
        if(shouldBe(i.key())){
            qCDebug(LC_SYSTEM) << typeName(i.key()) << "constructed:" << i.value() << "destructed:" << destroyed().value(i.key());
        }
    }
    qCDebug(LC_SYSTEM) << "----------------------------------------------------------------";
}






