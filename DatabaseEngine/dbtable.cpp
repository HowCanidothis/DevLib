#include "dbtable.h"
#include "dbtableheader.h"
#include "dbmodificationrule.h"
#include "Core/Memory/dbmemorydeclarations.h"
#include "Core/Memory/dbmemory.h"
#include "Core/Private/dbtableproxy.h"
#include "Core/Private/dbarraybase.h"
#include "Core/Private/dbtablefielddelegate.h"

DB_BEGIN_NAMESPACE

void DbTable::onRowAdded(const DbTableRow& row)
{
    for(DbTableBase* base : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(base) {
            DbTableProxy* proxy = base->asProxy();
            proxy->add(row);
        }
    }
}

void DbTable::onRowRemoved(const DbTableRow& row)
{
    for(DbTableBase* base : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(base) {
            DbTableProxy* proxy = base->asProxy();
            proxy->remove(row);
        }
    }
}

void DbTable::onRowChanged(const DbTableRow& row, qint32 column)
{
    if(m_proxies[column])
        m_proxies[column]->asProxy()->change(row);
}

void DbTable::initializeProxies()
{
    for(DbTableBase*& proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(proxy != nullptr) {
            proxy->asProxy()->initialize();
        }
    }
}

void DbTable::updatePlainData(const DbAddressMap& addressMap)
{
    m_declaration->updatePlainData(addressMap);
    for(DbTableBase*& proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(proxy != nullptr) {
            proxy->asProxy()->updatePlainData(addressMap);
        }
    }

    for(const DbTableRow& r : m_rows) {
        for(quint32 offset : m_header->m_complexFieldsOffsets) {
            (r + offset).AsPtr<MDbArrayBase>()->updatePlainData(addressMap);
        }
    }
}

void DbTable::edit(const DbModificationRule& rule)
{
    for(DbTableBase*& t : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(t != nullptr) {
            CastablePtr(t).As<DbTableProxy>().free();
            delete t;
        }
    }

    Q_ASSERT(m_header == rule.GetHeader());
    setHeader(m_header); // Reset proxies

    Rows oldRows = m_rows;
    m_rows.Clear();
    for(const DbTableRow& row : oldRows) {
        qint32 need_bytes = m_header->GetSize();
        CastablePtr newRow = m_memory->Allocate(DbMemoryFragment::ReasonTableData, GetID(),need_bytes);
        rule.copyRow(row, newRow);
        m_memory->Free(row);
        m_rows.Append(newRow);
    }
}

CastablePtr DbTable::edit(const DbTableRow& row, qint32 column)
{
    Q_ASSERT(column);
    CastablePtr res = row + m_header->GetFieldDelegate(column)->GetOffset();
    onRowChanged(row, column);
    return res;
}

const CastablePtr DbTable::get(DbTableRow row, qint32 column) const
{
    return row + m_header->GetFieldDelegate(column)->GetOffset();
}

void DbTable::setHeader(const DbTableHeader* header)
{
    this->m_header = header;
    m_proxies.Resize(this->GetColumnsCount());
    m_proxies.First() = this;
    m_current = this;
    for(DbTableBase*& t : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        t = nullptr;
    }
}

void DbTable::setProxy(MDbTableProxyDeclaration* proxy_declaration)
{
    DbTableBase*& proxy = m_proxies[proxy_declaration->ColumnIndex];
    proxy = new DbTableProxy(this, proxy_declaration);
}

CastablePtr DbTable::allocateRow()
{
    qint32 need_bytes = m_header->GetSize();
    CastablePtr allocated = m_memory->Allocate(DbMemoryFragment::ReasonTableData, GetID(),need_bytes);
    m_header->InitializeDefaultValues(allocated);
    return allocated;
}

void DbTable::free()
{
    m_header->m_declaration->Usage--;

    if(!m_header->m_complexFieldsOffsets.IsEmpty()) {
        for(const DbTableRow& r : m_rows) {
            for(quint32 offset : m_header->m_complexFieldsOffsets) {
                (r + offset).AsPtr<MDbArrayBase>()->Free();
            }
            m_memory->Free(r);
        }
    } else {
        for(const DbTableRow& row : this->m_rows) {
            m_memory->Free(row);
        }
    }

    for(DbTableBase*& proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(proxy != nullptr) {
            static_cast<DbTableProxy*>(proxy)->free();
        }
    }

    this->m_declaration->Free();
}

DbTable::DbTable(DbMemory* memory, DbDatabase* database, MDbTableDeclaration* declaration) :
    m_declaration(declaration),
    m_memory(memory),
    m_database(database),
    m_current(this)
{
}

DbTable::~DbTable() {
    if(m_declaration != nullptr) {
        for(DbTableBase* proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
            delete proxy;
        }
    }
}

DbTableRow DbTable::Append()
{
    auto allocated = allocateRow();
    allocated.As<qint32>() = m_declaration->LastId++;
    for(DbTableBase* proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(proxy != nullptr) {
            proxy->asProxy()->add(allocated);
        }
    }
    m_rows.Append(allocated);
    return allocated;
}

DbTableBase::const_iterator DbTable::Append(qint32 count)
{
    qint32 tmp = count;
    while(tmp--) {
        auto allocated = allocateRow();
        allocated.As<qint32>() = m_declaration->LastId++;
        for(DbTableBase* proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
            if(proxy != nullptr) {
                proxy->asProxy()->add(allocated);
            }
        }
        m_rows.Append(allocated);
    }
    return m_rows.end() - count;
}

void DbTable::Remove(const DbTableRow& row)
{
    for(DbTableBase* proxy : adapters::range(m_proxies.begin() + 1, m_proxies.end())) {
        if(proxy != nullptr) {
            proxy->asProxy()->remove(row);
        }
    }
    m_removeBuffer.insert(row);
}

void DbTable::SetHeaderName(const QString& name)
{
    const_cast<DbTableHeader*>(m_header)->SetName(name);
}

const DbString&DbTable::GetHeaderName() const
{
    return m_header->GetName();
}

void DbTable::SetName(const QString& name)
{
    m_declaration->Name = name;
}

const DbString& DbTable::GetName() const
{
    return m_declaration->Name;
}

id_t DbTable::GetID() const
{
    return m_declaration->GetID();
}

id_t DbTable::GetLastId() const
{
    return m_declaration->LastId;
}

void DbTable::Sort(qint32 column)
{
    auto& proxyTable = m_proxies[column];
    if(proxyTable == nullptr) {
        MDbTableProxyDeclaration* declaration = m_memory->Allocate<MDbTableProxyDeclaration>(DbMemoryFragment::ReasonTableProxyDeclaration, this->m_declaration->GetID());
        declaration->ColumnIndex = column;
        proxyTable = new DbTableProxy(this, declaration);
    }
    proxyTable->validate();
    m_current = proxyTable;
}

QString DbTable::ToStringValue(qint32 row, qint32 column) const
{
    auto stringSerializer = m_header->GetFieldDelegate(column)->GetStringSerializer();
    return stringSerializer(get(*(begin() + row),column).AsPtr<void>());
}

DbTableBase::const_iterator DbTable::find(CastablePtr value, qint32 column)
{
    Sort(column);
    auto proxyTable = m_proxies[column];
    const DbTableFieldDelegate* fieldDelegate = m_header->GetFieldDelegate(column);
    quint16 offset = fieldDelegate->GetOffset();
    auto lessthan = [fieldDelegate,offset](const DbTableRow& f, const DbTableRow& s) {
        return fieldDelegate->GetSearchComparator()(f + offset, s.AsPtr<void>());
    };

    auto result = proxyTable->m_rows.FindSortedByPredicate(value, lessthan);

    return reinterpret_cast<const_iterator>(result);
}

void DbTable::validate()
{
    if(!m_removeBuffer.isEmpty()) {
        const Array<quint32> & offsets = this->m_header->m_complexFieldsOffsets;
        this->m_rows.RemoveByPredicate([&](DbTableRow &r){
            if(m_removeBuffer.contains(r)) {
                for(quint32 offset : offsets) {
                    (r + offset).AsPtr<MDbArrayBase>()->Free();
                }
                m_memory->Free(r.AsPtr<void>());
                return true;
            }
            return false;
        });
        m_removeBuffer.clear();
    }
}

void DbTable::SaveProxies()
{
    Array<DbTableProxy*> changedProxies;
    for(DbTableBase* proxy : m_proxies) {
        if(proxy != nullptr) {
            proxy->validate();
            if(proxy->IsChanged()) {
                changedProxies.Append(reinterpret_cast<DbTableProxy*>(proxy));
            }
        }
    }

    if(!changedProxies.IsEmpty()) {
        auto rowsMap = generateSourceMap();
        for(DbTableProxy* proxy : changedProxies) {
            proxy->save(rowsMap);
        }
    }
}

DbTableBase::RowsMap DbTableBase::generateSourceMap() const
{
    RowsMap result;
    qint32 rowCounter = 0;
    for(auto row : m_rows) {
        result.insert(row, rowCounter++);
    }
    return result;
}

DbTableBase::~DbTableBase()
{

}

DB_END_NAMESPACE


