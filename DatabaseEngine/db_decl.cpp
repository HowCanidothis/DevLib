#include "db_decl.h"
#include "Core/dbcontext.h"
#include "Core/memory/dbmemory.h"
DB_BEGIN_NAMESPACE

void MDbObject::Free()
{
    context().GetMemory()->Free(this);
}

id_t MDbObject::GetID() const
{
    return DbMemoryFragment::FromData(this)->GetID();
}

DB_END_NAMESPACE
