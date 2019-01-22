#ifndef DBDATABASE_H
#define DBDATABASE_H

#include "dbarray.h"

DB_BEGIN_NAMESPACE
class DbMemory;
class DbTable;
class DbTableHeader;
class DbDatabaseContentHandler;
struct DbDatabaseHeader;

class DbMemoryParams
{
    array_ptr_t _totalMemorySize;
    qint32 _sectorSize;
    qint32 _sectorUsefullSize; //for safety prefer from buffer overrun
    qint32 _sectorEnd;

public:
    DbMemoryParams(){}
    DbMemoryParams(qint32 maxSectorSize);

    void SetTotalMemorySize(array_ptr_t size) { _totalMemorySize = size; }
    void SetSectorSize(qint32 sectorSize);

    array_ptr_t GetTotalMemorySize() const { return _totalMemorySize; }
    quint32 GetSectorSize() const { return _sectorSize; }
    qint32 GetSectorUsefullSize() const { return _sectorUsefullSize; }
    quint32 GetSectorEnd() const { return _sectorEnd; }
    quint32 GetSectorBegin() const;

private:
    qint32 calculateUsefullSize(qint32 sectorSize) const;
    qint32 calculateEnd(qint32 sectorSize) const;
};

struct DbDatabaseHeader
{
    qint32 db_key;
    qint32 major_version;
    qint32 minor_version;
    bool x64_architecture;
    qint32 last_header_id;
    qint32 last_table_id;
    DbMemoryParams properties;

    DbDatabaseHeader(qint32 max_sector_size);
    DbDatabaseHeader() {}

    void Reset();

    Q_DISABLE_COPY(DbDatabaseHeader)
};

class DbDatabase
{
    friend class DatabaseModifier;
    friend class DatabaseMerger;
protected:
    DbDatabase(qint32 sector_size);
    DbDatabase();

public:
    ~DbDatabase();
    static DbDatabase* CreateDatabase(const DbDatabaseProperties& properties);
    static DbDatabase* CreateDatabase(QDataStream& stream);

    DbTableHeader* AddHeader(const QString& name="");
    DbTable* AddTable(DbTableHeader* header, const QString& name="");
    const DbDatabaseContentHandler* GetContentHandler() const { return content_handler.data(); }

    bool Load(QDataStream& stream);
    void Save(QDataStream& stream);

    void Edit(class DbModificationRule& rule);

    void Vacuum();
    void RemoveAll();
    void RemoveHeader(DbTableHeader* header);
    void RemoveTable(DbTable* table);

    void MakeCurrent();
    bool IsCurrent() const;

    //Debug only
    const DbMemory* GetMemory() const { return memory.data(); }

private:
    void validateObjects();
    void invalidateObjects();
    void readMemory();
    void readProxies();
    void clear();

private:
    ArrayPointers<DbTableHeader> headers;
    ArrayPointers<DbTable> tables;
    ScopedPointer<DbMemory> memory;
    ScopedPointer<DbDatabaseHeader> _databaseHeader;
    ScopedPointer<DbDatabaseContentHandler> content_handler;
};

class DbDatabaseContentHandler {
public:
    DbTable* FindTable(const QString& table_name) const;
    DbTableHeader* FindHeader(const QString& header_name) const;

    adapters::Range<DbTable*const*> FindTables(const QString& table_name) const;
    adapters::Range<DbTableHeader*const*> FindHeaders(const QString& header_name) const;

    const Array<DbTable*>& GetTables() const { return this->m_tables.GetObjects(); }
    const Array<DbTableHeader*>& GetTableHeaders() const { return this->m_headers.GetObjects(); }

    template<class T>
    class ContentObject {
        Array<T*> m_objects;
    public:
        void Add(T* object) { m_objects.InsertSortedMulty(object); }
        void Remove(T* object) { m_objects.Remove(object); }
        void Change(T* object) { Remove(object); Add(object); }
        void Clear() { m_objects.Clear(); }
        const Array<T*>& GetObjects() const { return m_objects; }
    };

private:
    friend class DbDatabase;
    ContentObject<DbTable> m_tables;
    ContentObject<DbTableHeader> m_headers;
    void clear();
    DbDatabaseContentHandler() {}
};

DB_END_NAMESPACE

#endif // DBDATABASE_H
