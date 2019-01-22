#include "dbmemorysector.h"
#include <QDataStream>
#include "DatabaseEngine/dbdatabase.h"

DB_BEGIN_NAMESPACE

DbMemoryFragment *DbMemorySector::findLast(DbMemoryFragment* head, DbMemoryFragment* tail) const
{
    forever {
        auto next = head->Next();
        if(next == tail)
            return head;
        head = next;
    }
}

DbMemorySector::DbMemorySector(const DbMemoryParams* params)
    : m_memoryParams(params)
    , m_alloc(nullptr)
{
    initialize();
}

void DbMemorySector::Load(QDataStream& stream)
{
    stream.readRawData(m_alloc.AsCharPtr(), m_memoryParams->GetSectorSize());
    // last is unused in full sectors
}

void DbMemorySector::Load(QDataStream& stream, qint32 bytes)
{
    stream.readRawData(m_alloc.AsCharPtr(), bytes);
    m_last = findLast(m_head, m_tail);
}

void DbMemorySector::Save(QDataStream& stream)
{
    stream.writeRawData(m_alloc.AsCharPtr(), m_memoryParams->GetSectorSize());
}

void DbMemorySector::SaveUsedMemoryOnly(QDataStream& stream)
{
    stream.writeRawData(m_alloc.AsCharPtr(), GetUsedMemorySize());
}

void DbMemorySector::UpdateSectorSize()
{
    ::free(m_alloc.AsPtr<void>());
    initialize();
}

bool DbMemorySector::UpdateSectorSize(const DbMemoryParams& old, Array<DbMemorySector*>& sectors)
{
    class MallocMemoryGuard
    {
        void* _ptr;
    public:
        MallocMemoryGuard(void* ptr)
            : _ptr(ptr)
        {}
        ~MallocMemoryGuard()
        {
            ::free(_ptr);
        }
    };

    MallocMemoryGuard oldAllocDeleter(m_alloc.AsPtr<void>());
    DbMemoryFragment* oldHead = m_head;
    DbMemoryFragment* oldTail = m_tail;

    initialize();

    sectors.Append(this);

    if(old.GetSectorSize() > m_memoryParams->GetSectorSize()) {
        DbMemoryFragment* dst_head = m_head;
        DbMemoryFragment* src_head = oldHead;
        qint32 bytes_to_copy=0;

        auto copy_mem = [&]() {
            memcpy(dst_head, src_head, bytes_to_copy);
            auto& lastFragment = dst_head->Offset(bytes_to_copy).As<DbMemoryFragment>();
            lastFragment.SetSize(m_memoryParams->GetSectorUsefullSize() - bytes_to_copy, true);
            src_head = oldHead;
        };

        for(; oldHead != oldTail; oldHead++) {
            if(oldHead->IsFree()) {
                continue;
            }
            qint32 tmp = bytes_to_copy + oldHead->GetSize();
            if(tmp < m_memoryParams->GetSectorUsefullSize())
                bytes_to_copy = tmp;
            else {
                if(!bytes_to_copy) {
                    return false;
                }

                copy_mem();
                DbMemorySector* additional_sector = new DbMemorySector(m_memoryParams);
                sectors.Append(additional_sector);
                dst_head = additional_sector->m_head;
            }
        }
        if(bytes_to_copy) {
            copy_mem();
        }
    }
    else { // old.getMaxSectorSize() < memory_set->getMaxSectorSize()
        DbMemoryFragment* fragment = findLast(oldHead, oldTail);
        fragment->SetSize(fragment->GetSize() + m_memoryParams->GetSectorSize() - old.GetSectorSize());

        memcpy(m_head, oldHead, old.GetSectorSize());
    }

    return true;
}

void DbMemorySector::Free(array_ptr_t address) const
{
    CastablePtr ptr(m_alloc + address);
    Q_ASSERT(!DbMemoryFragment::FromData(ptr)->IsFree());
    DbMemoryFragment::FromData(ptr)->SetFree(true);
}

Array<const DbMemoryFragment*> DbMemorySector::ToFragmentsArray()
{
    Array<const DbMemoryFragment*> result;
    for(DbMemoryFragment& fragment : *this) {
        result.Append(&fragment);
    }
    return result;
}

void DbMemorySector::initialize()
{
    m_alloc = (DbMemoryFragment*)::malloc(m_memoryParams->GetSectorSize());
    m_head = (m_alloc + m_memoryParams->GetSectorBegin()).AsPtr<DbMemoryFragment>();
    m_head->SetSize(m_memoryParams->GetSectorUsefullSize(), true);
    m_last = m_head;
    m_tail = (m_alloc + m_memoryParams->GetSectorEnd()).AsPtr<DbMemoryFragment>();

    m_alloc.As<qint32>() = 0;
}

DB_END_NAMESPACE
