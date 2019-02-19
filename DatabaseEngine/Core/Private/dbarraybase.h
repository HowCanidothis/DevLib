#ifndef DBARRAYBASE_H
#define DBARRAYBASE_H
#include "DatabaseEngine/db_decl.h"

DB_BEGIN_NAMESPACE

class MDbArrayBase
{
public:
    class ArrayData{
        friend class MDbArrayBase;
        qint32& m_size;
        qint32* m_data;
    public:
        ArrayData(qint32* data) : m_size(*data), m_data(data + 1){}

        void* GetData() const { return m_data; }
        qint32 GetSize() const { return m_size; }
        void* GetPtr() const { return m_data - 1; }
    };

    MDbArrayBase()
        : m_address(0)
    {}

    void Free();

    void MoveAddress(array_ptr_t offset){ Q_ASSERT(m_address); this->m_address += offset; }
    void SetAddress(array_ptr_t address){ this->m_address = address; }
    array_ptr_t GetAddress() const { return this->m_address; }

    template<class T> void Set(const T& d);
    template<class T> T Get() const;

    bool IsEmpty() const { return Size() == 0; }
    void* Data() const { return GetArrayData().GetData();  }
    void* data() const { return Data(); }
    qint32 Size() const;
    qint32 size() const { return Size(); }
    ArrayData GetArrayData() const { return ArrayData(ptr()); }

    void updatePlainData(const DbAddressMap& addressMap);
protected:
    array_ptr_t m_address;
    MDbArrayBase(array_ptr_t address) : m_address(address){}
    friend class DbTable;
    friend class DbTableHeader;

    qint32* ptr() const;
    ArrayData reallocate(qint32 Size, qint32 elementSize); //reallocates only if this->size() < size. realoc to bytes_count. Return ptr();
    MDbArrayBase& operator=(const MDbArrayBase& other);

private:
    qint32* allocate(qint32 Size);
};

template<class T>
T MDbArrayBase::Get() const
{
    T res;
    if(!m_address) {
        return res;
    }
    qint32 this_size = this->Size();
    res.resize(this_size);
    memcpy(res.data(), this->Data(), sizeof(typename T::value_type) * this_size);
    return res;
}

template<class T>
void MDbArrayBase::Set(const T &d)
{
    ArrayData data(reallocate(d.size(), sizeof(typename T::value_type)));

    memcpy(data.m_data, d.data(), sizeof(typename T::value_type) * d.size());
}


DB_END_NAMESPACE
#endif // DBARRAYBASE_H
