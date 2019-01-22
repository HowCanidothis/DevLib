#ifndef DBCONTEXT_H
#define DBCONTEXT_H
#include "DatabaseEngine/db_decl.h"

DB_BEGIN_NAMESPACE

class DbMemory;

struct DbContext
{
private:
    DbMemory* m_memory=0;
public:
    bool IsValid() const { return m_memory; }
    void Bind(DbMemory *memory){ this->m_memory = memory; }
    DbMemory* GetMemory() const { return m_memory; }
};

inline DbContext& context() { static DbContext res; return res; }

DB_END_NAMESPACE

#endif // DBCONTEXT_H
