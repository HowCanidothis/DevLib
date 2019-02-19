#include "dbtableproxy.h"

#include "DatabaseEngine/dbtable.h"
#include "DatabaseEngine/dbtableheader.h"
#include "DatabaseEngine/Core/Private/dbtablefielddelegate.h"
#include "DatabaseEngine/Core/Memory/dbmemory.h"
#include "DatabaseEngine/Core/Memory/dbmemorydeclarations.h"

DB_BEGIN_NAMESPACE

DbTableProxy::DbTableProxy(DbTable* table, MDbTableProxyDeclaration* declaration)
    : m_source(table)
    , m_declaration(declaration)
    , m_state(States_New)
{
}

void DbTableProxy::validate()
{
    if(m_state.TestFlag(State_NeedValidation)) {
        qint32 column_index = this->m_declaration->ColumnIndex;
        const DbTableHeader* header = m_source->GetHeader();
        const DbTableFieldDelegate* c = header->GetFieldDelegate(column_index);
        quint16 offset = c->GetOffset();
        auto lessthan = [c,offset](const DbTableRow& f, const DbTableRow& s){ return c->GetComparator()((f + offset),(s + offset)); };

        if(m_state.TestFlag(State_SensetiveChanges)) {
            if(!m_removeBuffer.isEmpty())
                this->m_rows.RemoveByPredicate([&](DbTableRow& r){ return m_removeBuffer.contains(r); });
            if(!m_addBuffer.isEmpty()) {
                for(const DbTableRow& v : m_addBuffer)
                    m_rows.Insert(std::lower_bound(m_rows.begin(), m_rows.end(), v, lessthan), v);
            }
        }
        else {
            this->m_rows.Clear();
            for(const auto& row : m_source->m_rows) {
                this->m_rows.Append(row);
            }
            std::sort(m_rows.begin(), m_rows.end(), lessthan);
        }
        m_removeBuffer.clear();
        m_addBuffer.clear();
        m_state.SetFlags(States_AfterChanges);
    }
}

void DbTableProxy::save(const RowsMap& sourceMap)
{
    auto& indexesArray = m_declaration->RowsIndexesInSource;
    indexesArray.Resize(m_rows.Size());
    auto it = indexesArray.Data();
    for(auto row : m_rows) {
        auto find = sourceMap.find(row);
        Q_ASSERT(find != sourceMap.end());
        *it = find.value();
        it++;
    }
    m_state.SetFlags(States_Loaded);
}

void DbTableProxy::initialize()
{
    auto& proxyIndexes = m_declaration->RowsIndexesInSource;
    if(proxyIndexes.IsEmpty()) {
        m_state.SetFlags(States_New);
    } else {
        for(qint32 index : proxyIndexes) {
            this->m_rows.Append(m_source->m_rows[index]);
        }
        m_state.SetFlags(States_Loaded);
    }
}

void DbTableProxy::free()
{
    m_declaration->Free();
}

void DbTableProxy::updatePlainData(const DbAddressMap& addressMap)
{
    m_declaration->updatePlainData(addressMap);
}

DB_END_NAMESPACE
