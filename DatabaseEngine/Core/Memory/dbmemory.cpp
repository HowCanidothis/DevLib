#include "dbmemory.h"

DB_BEGIN_NAMESPACE

DbMemory::~DbMemory()
{
    for(auto sector : *this) {
        delete sector;
    }
}

void DbMemory::Init()
{
    createSector();
}

void DbMemory::DeleteAll()
{
    for(auto sector : *this) {
        delete sector;
    }
    Clear();
    Init();
}

void DbMemory::Merge(DbMemory* other)
{
    Q_ASSERT(other->m_memoryParams.GetSectorSize() == this->m_memoryParams.GetSectorSize());

    for(DbMemorySector *memorySector : *other) {
        memorySector->m_memoryParams = &this->m_memoryParams;
        this->Append(memorySector);
    }
    other->Clear();
}

DbMemoryFragment*DbMemory::allocate(qint32 size)
{
    Q_ASSERT(!IsEmpty());
    DbMemoryFragment* fragment;
    if(!(fragment = Last()->Allocate(size))) {
        createSector();
        fragment = Last()->Allocate(size);
    }
    Q_ASSERT(fragment);
    return fragment;
}

CastablePtr DbMemory::Allocate(array_ptr_t& offset, qint32 size)
{
    CastablePtr res = allocate(size);
    res.As<DbMemoryFragment>().SetReason(DbMemoryFragment::ReasonPointedRawData);
    offset = (this->Size() - 1) * m_memoryParams.GetSectorSize() + Last()->Offset(res);
    return res;
}

CastablePtr DbMemory::Allocate(DbMemoryFragment::Reason reason, id_t id, qint32 size)
{
    DbMemoryFragment* fragment = allocate(size);
    fragment->SetID(reason, id);
    return fragment->Data();
}

DbAddressMap DbMemory::Vacuum()
{
    DbAddressMap result;
    Array<DbMemorySector*> oldSectors = *this;
    this->Clear();
    createSector();
    array_ptr_t oldBaseSize = 0;
    for(DbMemorySector* sector : oldSectors) {
        for(DbMemoryFragment& fragment : *sector) {
            if(!fragment.IsFree()) {
                auto dataSize = fragment.GetDataSize();
                auto newFragment = Last()->Allocate(dataSize);
                if(newFragment == nullptr) {
                    createSector();
                    newFragment = Last()->Allocate(dataSize);
                    Q_ASSERT_X(newFragment != nullptr, "DbMemory::Vacuum()", "Too small sector size. Something goes wrong");
                }
                if(fragment.GetReason() == DbMemoryFragment::ReasonPointedRawData) {
                    auto oldOffset = sector->Offset(&fragment) + oldBaseSize;
                    auto newOffset = (Size() - 1) * m_memoryParams.GetSectorSize() + Last()->Offset(newFragment);
                    if(oldOffset != newOffset) {
                        result.insert(oldOffset, newOffset);
                    }
                }
                memcpy(newFragment, &fragment, fragment.GetSize());
            }
        }
        oldBaseSize += m_memoryParams.GetSectorSize();
    }

    for(auto sector : oldSectors) {
        delete sector;
    }
    return result;
}

const CastablePtr DbMemory::Offset(array_ptr_t address) const
{
    qint32 sector = sectorIndex(address);
    const DbMemorySector* s = this->At(sector);
    return s->DataAt(address - sector * m_memoryParams.GetSectorSize());
}

CastablePtr DbMemory::Offset(array_ptr_t address)
{
    qint32 sector = sectorIndex(address);
    Q_ASSERT_X(sector >= 0 && sector < this->Size(), "DbMemory::Free", "Incorrect address of Free() function");
    return this->At(sector)->DataAt(address - sector * m_memoryParams.GetSectorSize());
}

void DbMemory::Free(CastablePtr object)
{
    auto fragment = DbMemoryFragment::FromData(object);
    fragment->SetFree(true);
}

void DbMemory::Free(array_ptr_t address)
{
    qint32 sector = sectorIndex(address);
    Q_ASSERT_X(sector >= 0 && sector < this->Size(), "DbMemory::Free", "Incorrect address of Free() function");
    this->At(sector)->Free(address - sector * m_memoryParams.GetSectorSize());
}

void DbMemory::SetSectorSize(qint32 new_size, bool keep_memory)
{
    if(this->IsEmpty()) {
        m_memoryParams.SetSectorSize(new_size);
        return;
    }
    if(m_memoryParams.GetSectorSize() == new_size) {
        return;
    }

    DbMemoryParams old_set = m_memoryParams;
    m_memoryParams.SetSectorSize(new_size);
    if(!keep_memory) {
        for(DbMemorySector* sector : *this) {
            sector->UpdateSectorSize();
        }
    }
    else {
        Array<DbMemorySector*> old_array = *this;
        this->Clear();
        for(DbMemorySector* sector : old_array) {
            if(!sector->UpdateSectorSize(old_set, *this)) {
                this->DeleteAll(); // TODO. Try to keep old state
                break;
            }
        }
    }
}

void DbMemory::Load(QDataStream& stream, const DbMemoryParams& params)
{
    SetSectorSize(params.GetSectorSize(), false);

    qint32 sectors = params.GetTotalMemorySize() / m_memoryParams.GetSectorSize();
    qint32 lastSectorMemory = params.GetTotalMemorySize() - m_memoryParams.GetSectorSize() * sectors;
    if(lastSectorMemory > 0) { // need additional sector
        sectors++;
    }
    qint32 dif = this->Size() - sectors;
    this->Resize(sectors);
    auto e = this->end();
    if(dif > 0) {
        for(DbMemorySector* sector : adapters::range(e, e + dif)) {
            delete sector;
        }
    }
    else if(dif < 0) {
        for(DbMemorySector*& sector : adapters::range(e + dif, e)) {
            sector = new DbMemorySector(&m_memoryParams);
        }
    }

    for(DbMemorySector* sector : adapters::range(this->begin(), this->end() - 1)) {
        sector->Load(stream);
    }
    Last()->Load(stream, lastSectorMemory);
}

void DbMemory::Save(QDataStream& stream)
{
    for(DbMemorySector* sector : adapters::range(this->begin(), this->end() - 1))
        sector->Save(stream);
    Last()->SaveUsedMemoryOnly(stream);
}

void DbMemory::Synch(DbMemoryParams* params) const
{
    qint32 lastSectorSize = Last()->GetUsedMemorySize();
    params->SetTotalMemorySize(m_memoryParams.GetSectorSize() * (Size() - 1) + lastSectorSize);
}

count_t DbMemory::GetMemoryUsage() const
{
    return this->Size() * m_memoryParams.GetSectorSize();
}

quint32 DbMemory::sectorIndex(array_ptr_t p) const
{
    return p / m_memoryParams.GetSectorSize();
}

void DbMemory::createSector()
{
    DbMemorySector* res = new DbMemorySector(&m_memoryParams);
    this->Append(res);
}


DB_END_NAMESPACE
