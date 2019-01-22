#ifndef DBTABLE_H
#define DBTABLE_H

#include "dbtableheader.h"

DB_BEGIN_NAMESPACE

class DbMemory;
class DbDatabase;
class DbTableHeader;
class DbTableProxy;
class DbTable;
struct MDbTableDeclaration;
struct MDbTableProxyDeclaration;

class DbTableRow : protected CastablePtr
{
    typedef CastablePtr Super;
public:
    DbTableRow(const CastablePtr& ptr)
        : CastablePtr(ptr.AsPtr<void>())
    {}

    template<class T> const T& As() const { return Super::As<T>(); }
    operator size_t() const { return Super::operator size_t();  }

private:
    friend class DbTableBase;
    friend class DbTable;
    friend class DbTableProxy;
};
Q_DECLARE_TYPEINFO(DbTableRow, Q_PRIMITIVE_TYPE);

class DbTableBase
{
    Q_DISABLE_COPY(DbTableBase)
protected:
    typedef QHash<CastablePtr, qint32> RowsMap;
    typedef Array<DbTableRow> Rows;
    typedef QSet<DbTableRow> RowsCache;

    Rows m_rows;
    RowsCache m_removeBuffer;
    friend class DbTable;
    DbTableBase() {}
    DbTableProxy* asProxy() const { return (DbTableProxy*)this; }
    RowsMap generateSourceMap() const;
public:
    typedef bool (*LessThan)(const DbTableRow& f, const DbTableRow& s);
    typedef const DbTableRow* const const_iterator;
    virtual ~DbTableBase();

    virtual void validate()=0;
    virtual bool IsChanged() const { return false; }

    const_iterator At(qint32 j) const {  return (m_rows.begin() + j); }
    const_iterator begin() const { return m_rows.begin(); }
    const_iterator end() const { return m_rows.end(); }

    qint32 Size() const { return m_rows.Size(); }
    bool IsEmpty() const { return m_rows.IsEmpty(); }
};

class DbTable : public DbTableBase
{
protected:
    friend class DbTableProxy;
    friend class DatabaseConverterSQL;
    friend class DatabaseMerger;
    friend class DbRowEditor;
    friend class DbDatabase;
    friend struct std::default_delete<DbTable>;
    friend class ArrayPointers<DbTable>;
    Array<DbTableBase*> m_proxies;
    MDbTableDeclaration* m_declaration;
    DbMemory* m_memory;
    DbDatabase* m_database;
    const DbTableHeader* m_header;
    DbTableBase* m_current;

    void initializeProxies();
    void updatePlainData(const DbAddressMap& addressMap);
    void edit(const class DbModificationRule& rule);
    CastablePtr edit(const DbTableRow& row, qint32 column);
    const CastablePtr get(DbTableRow row, qint32 column) const;
    void onRowAdded(const DbTableRow& row);
    void onRowRemoved(const DbTableRow& row);
    void onRowChanged(const DbTableRow& row, qint32 column);
    void appendRow(const DbTableRow& ptr) { m_rows.Append(ptr); }
    void setDeclaration(MDbTableDeclaration* declaration) { this->m_declaration = declaration; }
    void setHeader(const DbTableHeader* m_header);
    void setProxy(MDbTableProxyDeclaration* proxy_declaration);
    CastablePtr allocateRow();
    void free();

    DbTable(DbMemory* m_memory, DbDatabase* m_database, MDbTableDeclaration* m_declaration);
    ~DbTable();
protected:
    void validate();
public:

    void SaveProxies();

    DbTableRow Append();
    const_iterator Append(qint32 count);
    void Remove(const_iterator it) { Remove(*it); }
    void Remove(const DbTableRow& row);
    void Update() { m_current->validate(); }

    void SetHeaderName(const QString& name);
    const DbString& GetHeaderName() const;
    void SetName(const QString& name);
    const DbString& GetName() const;
    id_t GetID() const;
    id_t GetLastId() const;
    bool IsValid() { return m_declaration; }

    qint32 GetColumnsCount() const { return this->m_header->GetFieldsCount(); }
    const DbTableHeader* GetHeader() const { return this->m_header; }

    void Sort(qint32 column);
    template<class T> const_iterator Find(const T& value, qint32 column)
    {
        return find(const_cast<T*>(&value), column);
    }

    QString ToStringValue(qint32 row, qint32 column) const;

    const_iterator begin() const { return m_current->begin(); }
    const_iterator end() const { return m_current->end(); }
    const_iterator Change(qint32 row) const { return m_current->At(row); }
    const DbTableRow RowAt(qint32 row) const { return get(*(begin() + row), 0);  }
    template<class T> const T& At(const DbTableRow& row, qint32 column) const { return get(row, column).As<T>(); }
    template<class T> const T& At(qint32 row, qint32 column) const { return At<T>(begin() + row, column);  }
    template<class T> const T& At(const_iterator row, qint32 column) const { return get(*row, column).As<T>(); }
    template<class T> T& Change(const DbTableRow& row, qint32 column) { return edit(row, column).As<T>(); }
    template<class T> T& Change(qint32 row, qint32 column) { return Change<T>(begin() + row, column); }
    template<class T> T& Change(const_iterator row, qint32 column) { return edit(*row, column).As<T>(); }

private:
    const_iterator find(CastablePtr value, qint32 column);
};


template<typename T>
class TDbTable : public DbTable
{
public:

    template<typename ... Args> T* Append(Args...args) {
        return new (Append().data) T(args...);
    }
};

DB_END_NAMESPACE

#endif // DBTABLE_H
