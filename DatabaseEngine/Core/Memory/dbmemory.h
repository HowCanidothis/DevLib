#ifndef DBMEMORY_H
#define DBMEMORY_H

#include <QScopedPointer>

#include <SharedModule/internal.hpp>

#include "dbmemorysector.h"
#include "DatabaseEngine/dbdatabase.h"

DB_BEGIN_NAMESPACE

class DbMemorySector;

class DbMemory : public Array<DbMemorySector*>
{
    Q_DISABLE_COPY(DbMemory)
    typedef Array<DbMemorySector*> Super;

    DbMemoryParams m_memoryParams;
public:
    DbMemory(qint32 sector_size) : m_memoryParams(sector_size) {}
    DbMemory() {}
    ~DbMemory();

    void Init();
    void DeleteAll();
    void Merge(DbMemory* other);

    CastablePtr Allocate(array_ptr_t& Offset, qint32 size);
    CastablePtr Allocate(DbMemoryFragment::Reason reason, id_t id, qint32 size);

    template<typename T, typename ... Args> T* Allocate(DbMemoryFragment::Reason reason, id_t id, Args ... args) {
        CastablePtr res = Allocate(reason, id, sizeof(T));
        new (res.AsPtr<void>()) T(args...);
        return res.AsPtr<T>();
    }


    DbAddressMap Vacuum();

    const CastablePtr Offset(array_ptr_t address) const;
    CastablePtr Offset(array_ptr_t address);
    void Free(CastablePtr object);
    void Free(array_ptr_t address);

    void SetSectorSize(qint32 new_size, bool keep_memory);

    void Load(QDataStream& stream, const DbMemoryParams& params);
    void Save(QDataStream& stream);

    void Synch(DbMemoryParams* params) const;
    count_t GetMemoryUsage() const;
private:
    DbMemoryFragment* allocate(qint32 size);

    quint32 sectorIndex(array_ptr_t p) const;
    void createSector();
};

DB_END_NAMESPACE

#endif // DBMEMORY_H
