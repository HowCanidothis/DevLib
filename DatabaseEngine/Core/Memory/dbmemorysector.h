#ifndef DBMEMORYSECTOR_H
#define DBMEMORYSECTOR_H

#include "dbmemoryfragment.h"
#include "DatabaseEngine/Core/private_objects/dbarraybase.h"
#include <iterator>

#define MAX_IDS 65555
#define MAX_SECTOR_SIZE 134217727
#define MAX_REASONS 15

DB_BEGIN_NAMESPACE

class DbMemoryParams;

class DbMemorySector
{
    friend class DbMemory;
    Q_DISABLE_COPY(DbMemorySector)
public:
    DbMemorySector(const DbMemoryParams* params); //allocates memory
    ~DbMemorySector() { ::free(m_alloc.AsPtr<void>()); }

    void Load(QDataStream& stream);
    void Load(QDataStream& stream, qint32 bytes);
    void Save(QDataStream& stream);
    void SaveUsedMemoryOnly(QDataStream& stream);

    qint32 GetUsedMemorySize() const { return OffsetToLast() + sizeof(DbMemoryFragment); }

    void UpdateSectorSize();
    bool UpdateSectorSize(const DbMemoryParams& old, Array<DbMemorySector*>& sectors);

    DbMemoryFragment* Allocate(qint32 bytes) {
        if(m_last->HasEnoughMemory(bytes)) {
            DbMemoryFragment* allocated_in = m_last;
            m_last = m_last->Split(bytes);
            return allocated_in;
        }
        return nullptr;
    }

    qint32 OffsetToLast() const { return static_cast<qint32>(CastablePtr::Difference(m_last, m_alloc.AsPtr<void>())); }
    array_ptr_t Offset(CastablePtr ptr) const { return static_cast<array_ptr_t>(ptr.AsCharPtr() -  CastablePtr(m_alloc).AsCharPtr() + sizeof(DbMemoryFragment)); }
    const CastablePtr DataAt(array_ptr_t address) const { return CastablePtr(m_alloc + address); }
    CastablePtr DataAt(array_ptr_t address) { return CastablePtr(m_alloc + address); }
    void Free(array_ptr_t address) const;

    class MemoryFragmentIterator {
        DbMemoryFragment* m_ptr;
    public:
        typedef MemoryFragmentIterator It;
        typedef std::forward_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;
        typedef DbMemoryFragment value_type;
        typedef DbMemoryFragment* pointer;
        typedef DbMemoryFragment& reference;

        MemoryFragmentIterator(DbMemoryFragment *begin) : m_ptr(begin){}

        bool operator!=(const It& other) const{ return this->m_ptr != other.m_ptr; }
        It &operator++() { m_ptr = m_ptr->Next(); return *this; }
        It operator++(qint32) { It n(m_ptr); m_ptr = m_ptr->Next(); return n; }
        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }
        pointer value() const { return m_ptr; }
        bool operator==(const It& other) const { return !(operator !=(other)); }
    };

    MemoryFragmentIterator begin() const { return m_head; }
    MemoryFragmentIterator end() const { return m_tail; }

    Array<const DbMemoryFragment*> ToFragmentsArray();

private:
    DbMemoryFragment* findLast(DbMemoryFragment* m_head, DbMemoryFragment* m_tail) const;
    void initialize();

private:
    const DbMemoryParams* m_memoryParams;
    CastablePtr m_alloc;
    DbMemoryFragment* m_head;
    DbMemoryFragment* m_tail;
    DbMemoryFragment* m_last;
};

DB_END_NAMESPACE

#endif // DBMEMORYSECTOR_H
