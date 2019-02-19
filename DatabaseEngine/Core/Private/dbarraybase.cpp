#include "dbarraybase.h"
#include "DatabaseEngine/Core/dbcontext.h"
#include "DatabaseEngine/Core/memory/dbmemory.h"

DB_BEGIN_NAMESPACE

qint32* MDbArrayBase::allocate(qint32 size)
{
    return context().GetMemory()->Allocate(m_address,size).AsPtr<qint32>();
}

qint32* MDbArrayBase::ptr() const
{
    return context().GetMemory()->Offset(m_address).AsPtr<qint32>();
}

void MDbArrayBase::Free()
{
    if(m_address) {
        context().GetMemory()->Free(m_address);
    }
}

qint32 MDbArrayBase::Size() const
{
     auto arrayData = GetArrayData();
     return arrayData.GetSize();
}

void MDbArrayBase::updatePlainData(const DbAddressMap& addressMap)
{
    if(m_address != 0) {
        auto find = addressMap.find(m_address);
        if(find != addressMap.end()) {
            m_address = find.value();
        }
    }
}

MDbArrayBase& MDbArrayBase::operator=(const MDbArrayBase& other)
{
    m_address = other.m_address;
    return *this;
}

MDbArrayBase::ArrayData MDbArrayBase::reallocate(qint32 size, qint32 elementSize)
{
    ArrayData before(ptr());
    if(size <= before.m_size) {
        before.m_size = size;
        return before;
    }

    Free();
    allocate(elementSize * size + sizeof(qint32)); // size of array + memory for elements
    ArrayData after(ptr());
    after.m_size = size;
    return after;
}



DB_END_NAMESPACE
