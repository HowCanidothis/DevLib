#include "dbdatabase.h"

#include <QDataStream>

#include "Core/private_objects/dbobjectsarraymanager.h"
#include "Core/memory/dbmemory.h"
#include "Core/memory/dbmemorydeclarations.h"
#include "dbmodificationrule.h"
#include "dbtableheader.h"
#include "dbtable.h"
#include "Core/dbcontext.h"

#define DB_KEY 0x12ff5600
#define DB_MINOR_VERSION 0
#define DB_MAJOR_VERSION 0

#define DB_MEMORY_SECTOR_LEFT_RESERVED (sizeof(qint32)) // Null data for Arrays
#define DB_MEMORY_SECTOR_RIGHT_RESERVED (sizeof(DbMemoryFragment))

DB_BEGIN_NAMESPACE

DbMemoryParams::DbMemoryParams(qint32 maxSectorSize)
    : _totalMemorySize(0)
    , _sectorSize(maxSectorSize)
    , _sectorUsefullSize(calculateUsefullSize(maxSectorSize))
    , _sectorEnd(calculateEnd(maxSectorSize))
{
    Q_ASSERT(_sectorUsefullSize > 0);
}

void DbMemoryParams::SetSectorSize(qint32 sectorSize)
{
    this->_sectorSize = sectorSize;
    _sectorUsefullSize = calculateUsefullSize(sectorSize);
    _sectorEnd = calculateEnd(sectorSize);
}

quint32 DbMemoryParams::GetSectorBegin() const
{
    return DB_MEMORY_SECTOR_LEFT_RESERVED;
}

qint32 DbMemoryParams::calculateUsefullSize(qint32 sectorSize) const
{
    return sectorSize - DB_MEMORY_SECTOR_RIGHT_RESERVED - DB_MEMORY_SECTOR_LEFT_RESERVED;
}

qint32 DbMemoryParams::calculateEnd(qint32 sectorSize) const
{
    return sectorSize - DB_MEMORY_SECTOR_RIGHT_RESERVED;
}

static constexpr bool isX64(){ return  sizeof(array_ptr_t) == sizeof(qint64); }

DbDatabaseHeader::DbDatabaseHeader(qint32 max_sector_size)
    : db_key(DB_KEY),
      major_version(DB_MAJOR_VERSION),
      minor_version(DB_MINOR_VERSION),
      x64_architecture(isX64()),
      last_header_id(0),
      last_table_id(0),
      properties(max_sector_size)
{}

void DbDatabaseHeader::Reset()
{
    last_header_id = 0;
    last_table_id = 0;
}

DbDatabase::DbDatabase(qint32 sector_size)
    : _databaseHeader(new DbDatabaseHeader(sector_size))
    , content_handler(new DbDatabaseContentHandler)
{}

DbDatabase::DbDatabase()
    : _databaseHeader(new DbDatabaseHeader)
    , content_handler(new DbDatabaseContentHandler)
{

}

void DbDatabase::validateObjects()
{
    DbObjectsArrayManager::ReduceArray(headers);
    DbObjectsArrayManager::ReduceArray(tables);
}

void DbDatabase::invalidateObjects()
{
    DbObjectsArrayManager::ExpandArray(headers, [&](){ return new DbTableHeader(memory.data(), nullptr); }, _databaseHeader->last_header_id);
    DbObjectsArrayManager::ExpandArray(tables, [&](){ return new DbTable(memory.data(), this, nullptr); }, _databaseHeader->last_table_id);
}

void DbDatabase::readMemory()
{
    invalidateObjects();

    auto proxiesMap = new Array<MDbTableProxyDeclaration*>[_databaseHeader->last_table_id + 1];

    for(DbMemorySector* s : *memory) {
        for(DbMemoryFragment& f : *s) {
            if(!f.GetFree()){
                switch (f.GetReason()) {
                case DbMemoryFragment::ReasonTableData: {
                    DbTable* table = tables.At(f.GetID());
                    table->appendRow(f.Data());
                    break;
                }
                case DbMemoryFragment::ReasonTableDeclaration: {
                    MDbTableDeclaration* td = f.Data().AsPtr<MDbTableDeclaration>();
                    DbTable* table = tables.At(f.GetID());
                    table->setDeclaration(td);
                    break;
                }
                case DbMemoryFragment::ReasonTableFieldDeclaration:{
                    MDbTableField* tf = f.Data().AsPtr<MDbTableField>();
                    DbTableHeader* header = headers.At(f.GetID());
                    header->setField(tf);
                    break;
                }
                case DbMemoryFragment::ReasonTableHeaderDeclaration:{
                    MDbTableHeaderDeclaration* tf = f.Data().AsPtr<MDbTableHeaderDeclaration>();
                    DbTableHeader* header = headers.At(f.GetID());
                    header->setBegin(tf);
                    break;
                }
                case DbMemoryFragment::ReasonTableProxyDeclaration:{
                    MDbTableProxyDeclaration* tf = f.Data().AsPtr<MDbTableProxyDeclaration>();
                    proxiesMap[f.GetID()].Append(tf);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    validateObjects();

    for(DbTableHeader* header : headers) {
        header->setEnd();
        content_handler->m_headers.Add(header);
    }
    for(DbTable* t : tables) {
        t->setHeader(*headers.FindSortedByPredicate(t->m_declaration->HeaderId, [](DbTableHeader* header, qint32 id){ return header->GetID() < id; }));
        content_handler->m_tables.Add(t);
    }

    for(qint32 i = 0; i < (_databaseHeader->last_table_id + 1); i++) {
        for(auto proxy : proxiesMap[i]) {
            DbTable* table = tables.At(i);
            table->setProxy(proxy);
        }
    }

    delete [] proxiesMap;
}

void DbDatabase::clear()
{
    content_handler->clear();
}

DbDatabase* DbDatabase::CreateDatabase(const DbDatabaseProperties& properties)
{
    DbDatabase* res = new DbDatabase(properties.SectorSize);
    DbMemory* memory = new DbMemory(properties.SectorSize);
    res->memory.reset(memory);
    res->MakeCurrent();

    memory->Init();

    return res;
}

DbDatabase* DbDatabase::CreateDatabase(QDataStream& stream)
{
    DbDatabase* res = new DbDatabase();
    DbMemory* memory = new DbMemory();
    res->memory.reset(memory);
    res->MakeCurrent();

    res->Load(stream);
    return res;
}

DbTableHeader* DbDatabase::AddHeader(const QString& name)
{
    MDbTableHeaderDeclaration* declaration = memory->Allocate<MDbTableHeaderDeclaration>(DbMemoryFragment::ReasonTableHeaderDeclaration, _databaseHeader->last_header_id++, name);
    DbTableHeader* res = new DbTableHeader(memory.data(), declaration);
    headers.Append(res);
    res->addPrimaryKey();
    content_handler->m_headers.Add(res);
    return res;
}

DbTable* DbDatabase::AddTable(DbTableHeader* header, const QString& name)
{
    MDbTableDeclaration* declaration = memory->Allocate<MDbTableDeclaration>(DbMemoryFragment::ReasonTableDeclaration, _databaseHeader->last_table_id++, name, header->GetID());
    DbTable* res = new DbTable(memory.data(), this, declaration);
    res->setHeader(header);
    header->IncrementUsage();
    tables.Append(res);
    content_handler->m_tables.Add(res);
    return res;
}

bool DbDatabase::Load(QDataStream &stream)
{
    clear();
    stream.readRawData((char*)_databaseHeader.data(), sizeof(DbDatabaseHeader));
    if(_databaseHeader->db_key != DB_KEY) {
        qCWarning(LC_UI) << "Unknown file format.";
        return false;
    }
    if(_databaseHeader->minor_version != DB_MINOR_VERSION || _databaseHeader->major_version != DB_MAJOR_VERSION)
    {
        qCWarning(LC_UI) << "Database version is out of date and not supported now";
        return false;
    }
    if(_databaseHeader->x64_architecture == !isX64())
    {
        qCWarning(LC_UI, "Database file is %s architecture but database is %s. Cannot open file", _databaseHeader->x64_architecture ? "x64" : "x32", isX64() ? "x64" : "x32") ;
        return false;
    }

    memory->Load(stream, _databaseHeader->properties);
    readMemory();

    for(DbTable* table : tables) {
        table->initializeProxies();
    }

    return true;
}

void DbDatabase::Save(QDataStream& stream)
{
    for(DbTable* table : tables) {
        table->SaveProxies();
    }

    memory->Synch(&_databaseHeader->properties);
    stream.writeRawData((const char*)_databaseHeader.data(), sizeof(DbDatabaseHeader));
    memory->Save(stream);
}

void DbDatabase::Edit(DbModificationRule& rule)
{
    rule.initialize();

    rule.GetHeader()->edit(rule);

    for(DbTable* table : tables) {
        if(table->GetHeader() == rule.GetHeader()) {
            table->edit(rule);
        }
    }
}

void DbDatabase::Vacuum()
{
    for(DbTable* table : tables) {
        table->SaveProxies();
    }

    tables.Clear();
    headers.Clear();
    content_handler->clear();

    auto addressMap = memory->Vacuum();

    readMemory();

    for(DbTable* table : tables) {
        table->updatePlainData(addressMap);
        table->initializeProxies();
    }

    for(DbTableHeader* header : headers) {
        header->updatePlainData(addressMap);
    }
}

void DbDatabase::RemoveAll()
{
    tables.Clear();
    headers.Clear();
    _databaseHeader->Reset();
    memory->DeleteAll();
    content_handler->clear();
}

void DbDatabase::RemoveHeader(DbTableHeader* header)
{
    Q_ASSERT(IsCurrent());

    tables.RemoveByPredicate([header, this](DbTable* table){
        if(table->GetHeader() == header) {
            content_handler->m_tables.Remove(table);
            table->free();
            delete table;
            return true;
        }
        return false;
    });

    headers.Remove(header);
    content_handler->m_headers.Remove(header);
    header->free();
    delete header;
}

void DbDatabase::RemoveTable(DbTable* tableToDelete)
{
    Q_ASSERT(content_handler->m_tables.GetObjects().ContainsSorted(tableToDelete));
    const_cast<DbTableHeader*>(tableToDelete->GetHeader())->DecrementUsage();
    tables.Remove(tableToDelete);
    content_handler->m_tables.Remove(tableToDelete);
    tableToDelete->free();
    delete tableToDelete;
}

void DbDatabase::MakeCurrent()
{
    context().Bind(this->memory.data());
}

bool DbDatabase::IsCurrent() const
{
    return context().GetMemory() == memory.data();
}

template<class T> static T* DbContentFindObject(const DbDatabaseContentHandler::ContentObject<T>& co, const QString& name)
{
    const Array<T*>& array = co.GetObjects();
    auto found = array.FindSortedByPredicate(name,
                 [](T* object, const QString& name) { return object->GetName() < name; });
    if(found == array.end() || (*found)->GetName() != name)
        return nullptr;
    return *found;
}

template<class T> static adapters::Range<T*const*> DbContentFindObjects(const DbDatabaseContentHandler::ContentObject<T>& co, const QString& name)
{
    const Array<T*>& array = co.GetObjects();
    auto begin = array.FindSortedByPredicate(name,
                 [](T* object, const QString& name) { return object->GetName() < name; });
    if(begin == array.end() || (*begin)->GetName() != name)
        return adapters::Range<T*const*>();
    auto it = begin + 1;
    while(it != array.end() && (*it)->GetName() == name)
        it++;
    return adapters::range(begin, it);
}


DbTable* DbDatabaseContentHandler::FindTable(const QString& table_name) const
{
    return DbContentFindObject(m_tables, table_name);
}

DbTableHeader*DbDatabaseContentHandler::FindHeader(const QString& header_name) const
{
    return DbContentFindObject(m_headers, header_name);
}

adapters::Range<DbTable*const*> DbDatabaseContentHandler::FindTables(const QString& table_name) const
{
    return DbContentFindObjects(m_tables, table_name);
}

adapters::Range<DbTableHeader*const*> DbDatabaseContentHandler::FindHeaders(const QString& header_name) const
{
    return DbContentFindObjects(m_headers, header_name);
}

void DbDatabaseContentHandler::clear()
{
    m_headers.Clear();
    m_tables.Clear();
}

DbDatabase::~DbDatabase()
{

}

DB_END_NAMESPACE
