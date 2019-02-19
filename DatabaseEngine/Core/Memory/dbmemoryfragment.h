#ifndef DBMEMORYFRAGMENT_H
#define DBMEMORYFRAGMENT_H

#include "DatabaseEngine/db_decl.h"

DB_BEGIN_NAMESPACE

#pragma pack(1)
class DbMemoryFragment
{
    Q_GADGET
    friend class DatabaseMerger;
    Q_DISABLE_COPY(DbMemoryFragment)
public:
    enum Reason {
        ReasonTableFieldDeclaration,
        ReasonTableHeaderDeclaration,
        ReasonTableDeclaration,
        ReasonTableData,
        ReasonPointedRawData,
        ReasonTableProxyDeclaration,
    };
    Q_ENUM(Reason)

    void SetReason(Reason reason) { this->m_reason = reason; }
    void SetID(Reason reason, id_t id) { this->m_reason = reason; this->m_id = id; }
    void SetSize(qint32 size, bool free) { this->m_size = size; this->m_free = free; }
    void SetSize(qint32 size) { this->m_size = size; }
    void SetFree(bool flag) { this->m_free = flag; }
    id_t GetID() const { return m_id; }
    Reason GetReason() const { return (Reason)m_reason; }
    quint32 GetSize() const { return m_size; }
    quint32 GetDataSize() const { return m_size - sizeof(DbMemoryFragment); }
    bool GetFree() const { return IsFree(); }
    bool IsFree() const { return m_free; }
    bool HasEnoughMemory(qint32 size) const { return UsefullMemory() > size; }
    qint32 UsefullMemory() const { return m_size - sizeof(DbMemoryFragment); }
    static const DbMemoryFragment* FromData(const CastablePtr& data) { return data.AsPtr<DbMemoryFragment>() - 1; }
    static DbMemoryFragment* FromData(CastablePtr& data) { return data.AsPtr<DbMemoryFragment>() - 1; }
    const CastablePtr Data() const { return this + 1; }
    CastablePtr Data() { return this + 1; }
    DbMemoryFragment* Next() { return (CastablePtr(this) + (quint32)m_size).AsPtr<DbMemoryFragment>(); }
    const DbMemoryFragment* Next() const { return (CastablePtr(this) + (quint32)m_size).AsPtr<DbMemoryFragment>(); }
    CastablePtr Offset(quint32 size) { return CastablePtr(this) + size; }
    CastablePtr Offset(quint32 size) const { return CastablePtr(this) + size; }
    DbMemoryFragment* Split(qint32 bytes) {
        qint32 old_size = this->m_size;
        this->m_size = bytes + sizeof(DbMemoryFragment);
        this->m_free = false;
        DbMemoryFragment* res = this->Next();
        res->SetSize(old_size - this->m_size, true);
        return res;
    }
private:
    quint32 m_size:27; //MAX_SECTOR_SIZE ~ 134 Mbytes maximum for sector size
    quint32 m_free:1;
    quint32 m_reason:4; //MAX_REASONS maximum
    id_t m_id; // MAX_IDS maximum tables and MAX_IDS proxies supports
};
#pragma pack()

DB_END_NAMESPACE

#endif // DBMEMORYFRAGMENT_H
