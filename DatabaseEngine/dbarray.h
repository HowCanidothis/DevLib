#ifndef DBARRAY_H
#define DBARRAY_H

#include <SharedGuiModule/internal.hpp>

#include "Core/Private/dbarraybase.h"

DB_BEGIN_NAMESPACE

template<class ContainerType>
class DbArray : public MDbArrayBase
{
    template<class T> friend struct DbArrayDelegate;
    ArrayData resize(qint32 size)
    {
        ArrayData res(this->reallocate(size, sizeof(value_type) * size));
        return res;
    }
public:
    typedef MDbArrayBase Super;
    typedef typename ContainerType::value_type value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    DbArray() : Super(0) {}
    DbArray(const ContainerType& ct) : Super(0) { this->Set(ct); }

    void Resize(qint32 count);

    const_iterator Data() const { return (const_iterator)Super::Data(); }
    iterator Data() { return (iterator)Super::Data(); }
    const_iterator begin() const { return this->Data(); }
    const_iterator end() const { return this->Data() + this->Size(); }
    iterator begin() { return this->Data(); }
    iterator end() { return this->Data() + this->Size(); }

    void SetFromPlainData(const QByteArray& data) { this->Set(data); }
    DbArray& operator=(const ContainerType& other) { this->Set(other); return *this; }
    bool operator==(const DbArray& other) const;
    bool operator!=(const DbArray& other) const { return !operator==(other); }
    bool operator==(const ContainerType& other) const;
    bool operator!=(const ContainerType& other) const { return !operator==(other); }
    bool operator<(const DbArray& other) const;
    bool operator<(const ContainerType& other) const;

    void ToNative(ContainerType& container) const;

    QString ToString() const;
    void FromString(const QString& string);
};

template<class ContainerType>
struct DbArrayDelegate {
    typedef DbArray<ContainerType> TargetArray;

    static void ToNative(ContainerType& container, const TargetArray& dbArray)
    {
        container.resize(dbArray.Size());
        memcpy(container.data(), dbArray.data(), sizeof(TargetArray::value_type) * dbArray.Size());
    }

    static bool LessThan(const TargetArray& f, const TargetArray& s) {
        return f.Size() < s.Size();
    }
    static bool LessThanT(const TargetArray& f, const ContainerType& s) {
        return f.Size() < s.size();
    }
    static QString ToString(const TargetArray& container) {
        static const char hex_pattern[] = "0123456789abcdef";
        auto toHex= [&](uint value) { return hex_pattern[value & 0xf]; };
        if(!container.Size())
            return QString();
        array_ptr_t bytes_count = sizeof(typename TargetArray::value_type) * container.Size();

        const uchar* data = (uchar*)container.Data();

        QString res(bytes_count * 2, Qt::Uninitialized);

        auto hexData = res.data();
        for (int i = 0, o = 0; i < container.Size(); ++i) {
            hexData[o++] = toHex(data[i] >> 4);
            hexData[o++] = toHex(data[i] & 0xf);
        }
        return res;
    }

    static void FromString(TargetArray& container, const QString& string) {
        auto fromHex = [](uint c) Q_DECL_NOTHROW
        {
            return ((c >= '0') && (c <= '9')) ? int(c - '0') :
                   ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10) :
                   /* otherwise */              -1;
        };

        MDbArrayBase::ArrayData a = container.resize(string.size() / 2);
        uchar* result = (uchar*)a.GetData() + a.GetSize() * sizeof(typename TargetArray::value_type);

        bool odd_digit = true;
        for (const QChar& c : adapters::reverse(string)) {
            uchar ch = c.toLatin1();
            int tmp = fromHex(ch);
            if (tmp == -1)
                continue;
            if (odd_digit) {
                --result;
                *result = tmp;
                odd_digit = false;
            } else {
                *result |= tmp << 4;
                odd_digit = true;
            }
        }
    }
};

template<class Type>
struct DbArrayDelegate<Array<Type>>
{
    typedef DbArray<Array<Type>> TargetArray;

    static void ToNative(Array<Type>& container, const TargetArray& dbArray)
    {
        container.Resize(dbArray.Size());
        memcpy(container.data(), dbArray.data(), sizeof(TargetArray::value_type) * dbArray.Size());
    }

    static QString ToString(const TargetArray& container)
    {
        QString result;
        for(const auto& value : container) {
            result += value.ToString() + ";";
        }
        result.resize(result.size() - 1);
        return result;
    }
    static void FromString(TargetArray& container, const QString& string)
    {
        QStringList splitted = string.split(";");
        container.Resize(splitted.size());
        auto it = container.begin();
        for(const QString& valueStr : splitted) {
            it->FromString(valueStr);
            it++;
        }
    }

    static bool LessThan(const TargetArray& f, const TargetArray& s)
    {
        return f.size() < s.size();
    }

    static bool LessThanT(const TargetArray& f, const Array<Type>& s)
    {
        return f.size() < s.size();
    }
};


template<>
struct DbArrayDelegate<Array<qint32>>
{
    typedef DbArray<Array<qint32>> TargetArray;

    static QString ToString(const TargetArray& container)
    {
        QString result;
        for(auto value : container) {
            result += QString::number(value) + ";";
        }
        result.resize(result.size() - 1);
        return result;
    }
    static void FromString(TargetArray& container, const QString& string)
    {
        QStringList splitted = string.split(";");
        container.Resize(splitted.size());
        auto it = container.begin();
        for(const QString& valueStr : splitted) {
            *it = valueStr.toDouble();
            it++;
        }
    }

    static bool LessThan(const TargetArray& f, const TargetArray& s)
    {
        return f.size() < s.size();
    }

    static bool LessThanT(const TargetArray& f, const Array<qint32>& s)
    {
        return f.size() < s.size();
    }
};

template<>
struct DbArrayDelegate<QString>
{
private:
    static int ucstrncmp_my(const QChar *a, const QChar *b, int l);
public:
    typedef DbArray<QString> TargetArray;
    static bool LessThan(const TargetArray& f, const TargetArray& s){
        if(f.GetAddress()){
            if(!s.GetAddress()) return false;
            MDbArrayBase::ArrayData a = f.GetArrayData(), b = s.GetArrayData();
            int l = qMin(a.GetSize(), b.GetSize());
            int cmp = ucstrncmp_my((QChar*)a.GetData(), (QChar*)b.GetData(), l);
            return (cmp ? cmp : (a.GetSize() - b.GetSize())) < 0;
        }
        else if(s.GetAddress()){
            return true;
        }
        return false;
    }

    static bool LessThanT(const TargetArray& f, const QString& s){
        if(f.GetAddress()){
            if(s.isEmpty()) return false;
            MDbArrayBase::ArrayData a = f.GetArrayData();
            int l = qMin(a.GetSize(), s.size());
            int cmp = ucstrncmp_my((QChar*)a.GetData(), (QChar*)s.data(), l);
            return (cmp ? cmp : (a.GetSize() - s.size())) < 0;
        }
        else if(s.size())
            return true;
        return false;
    }
    static QString ToString(const TargetArray& container) { return container.Get<QString>(); }
    static void FromString(TargetArray& container, const QString& string) { container = string; }
};

template<class ContainerType>
bool DbArray<ContainerType>::operator <(const ContainerType &other) const
{
    return DbArrayDelegate<ContainerType>::LessThanT(*this, other);
}

template<class ContainerType>
bool DbArray<ContainerType>::operator <(const DbArray<ContainerType> &other) const
{
    return DbArrayDelegate<ContainerType>::LessThan(*this, other);
}

template<class ContainerType>
void DbArray<ContainerType>::ToNative(ContainerType& container) const
{
    DbArrayDelegate<ContainerType>::ToNative(container, *this);
}

template<class ContainerType>
QString DbArray<ContainerType>::ToString() const
{
    return DbArrayDelegate<ContainerType>::ToString(*this);
}

template<class ContainerType>
void DbArray<ContainerType>::FromString(const QString& string)
{
    DbArrayDelegate<ContainerType>::FromString(*this, string);
}


template<class ContainerType>
void DbArray<ContainerType>::Resize(qint32 count)
{
    reallocate(count, sizeof(ContainerType::value_type));
}

template<class ContainerType>
bool DbArray<ContainerType>::operator==(const DbArray<ContainerType> &other) const
{
    ArrayData a = this->GetArrayData(), b = other.GetArrayData();
    if(a.GetSize() == b.GetSize()){
        const_iterator ab = this->begin();
        const_iterator bb = other.begin();
        const_iterator e = this->end();
        while (ab != e) {
            if(*ab != *bb)
                return false;
            ab++;
            bb++;
        }
        return true;
    }
    return false;
}

template<class ContainerType>
bool DbArray<ContainerType>::operator==(const ContainerType& b) const
{
    ArrayData a = this->GetArrayData();
    if(a.GetSize() == b.size()){
        const_iterator ab = this->begin();
        const_iterator bb = b.begin();
        const_iterator e = this->end();
        while (ab != e) {
            if(*ab != *bb)
                return false;
            ab++;
            bb++;
        }
        return true;
    }
    return false;
}

typedef DbArray<QString> DbString;
typedef DbArray<QByteArray> DbByteArray;
typedef DbArray<Array<qint32>> DbIntArray;
typedef DbArray<Array<Point3D>> DbPoint3DArray;

DB_END_NAMESPACE

#endif // DBARRAY_H
