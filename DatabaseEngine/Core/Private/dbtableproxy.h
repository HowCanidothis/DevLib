#ifndef DBTABLEPROXY_H
#define DBTABLEPROXY_H
#include "DatabaseEngine/db_decl.h"
#include "DatabaseEngine/dbtable.h"

DB_BEGIN_NAMESPACE

struct MDbTableProxyDeclaration;
class DbTable;

class DbTableProxy : public DbTableBase
{
    enum StateFlags
    {
        State_SensetiveChanges = 0x1,
        State_NeedValidation = 0x2,
        State_WasChanged = 0x4,

        States_AfterChanges = State_SensetiveChanges | State_WasChanged,
        States_New = State_NeedValidation | State_WasChanged,
        States_Loaded = State_SensetiveChanges
    };
    DECL_FLAGS(State, StateFlags)

    friend class DbTable;

    DbTable* m_source;
    MDbTableProxyDeclaration* m_declaration;
    State m_state; //State
    RowsCache m_addBuffer;

    DbTableProxy(DbTable* table, MDbTableProxyDeclaration* m_declaration);

    void validate() Q_DECL_OVERRIDE;

    void save(const RowsMap& sourceMap);
    bool IsChanged() const Q_DECL_OVERRIDE { return m_state.TestFlag(State_WasChanged); }

    void initialize();
    void setDeclaration(MDbTableProxyDeclaration* declaration) { this->m_declaration = declaration; }
    void setSource(DbTable* table) { m_source = table; }

    void add(const DbTableRow& ptr) {
        if(m_state.TestFlag(State_SensetiveChanges)){
            m_state.AddFlag(State_NeedValidation);
            if(m_addBuffer.size() < 1000)
                m_addBuffer.insert(ptr);
            else
                m_state.RemoveFlag(State_SensetiveChanges);
        }
    }

    void remove(const DbTableRow& ptr) {
        if(m_state.TestFlag(State_SensetiveChanges)){
            m_state.AddFlag(State_NeedValidation);
            m_removeBuffer.insert(ptr);
            m_addBuffer.remove(ptr);
        }
    }

    void change(const DbTableRow& ptr) {
        if(m_state.TestFlag(State_SensetiveChanges)){
            m_removeBuffer.insert(ptr);
            add(ptr);
        }
    }

    void free();

    void updatePlainData(const DbAddressMap& addressMap);
};

DB_END_NAMESPACE

#endif //DBTABLEPROXY_H
