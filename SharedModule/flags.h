#ifndef ATOMICFLAGS_H
#define ATOMICFLAGS_H

#include <QDebug>

#include <atomic>

template<class T>
struct FlagsHelpers
{
    static T& Remove(T& value, T flags)
    {
        value &= ~flags;
        return value;
    }

    static T& Add(T& value, T flags)
    {
        value |= flags;
        return value;
    }

    static T& ChangeFromBoolean(bool add, T& value, T flags)
    {
        return add ? Add(value, flags) : Remove(value, flags);
    }
};

using IntFlagsHelpers = FlagsHelpers<qint32>;
using LongFlagsHelpers = FlagsHelpers<qint64>;

template<typename ValueType, typename Enum>
class Flags{
public:
    typedef Enum enum_type;

    inline ValueType ToInt() const
    {
        return _value;
    }

    constexpr Flags(ValueType i=0)
        : _value(i)
    {}
    constexpr Flags(Enum e)
        : _value(e)
    {}


    Flags& SetFlags(ValueType value)
    {
        _value = value;
        return *this;
    }

    Flags& AddFlags(ValueType flags)
    {
        _value |= flags;
        return *this;
    }
    Flags& RemoveFlags(ValueType flags)
    {
        _value &= ~flags;
        return *this;
    }
    bool TestFlagsAll(ValueType flags) const
    {
        return (_value & flags) == flags;
    }
    bool TestFlagsAtLeastOne(ValueType flags) const
    {
        return (_value & flags);
    }

    void SwitchFlag(Enum flag)
    {
        _value ^= flag;
    }
    Flags& AddFlag(Enum flag)
    {
        _value |= flag;
        return *this;
    }
    Flags& RemoveFlag(Enum flag)
    {
        _value &= ~flag;
        return *this;
    }
    Flags RemovedFlag(ValueType flags) const
    {
        return _value & ~flags;
    }
    bool TestFlag(Enum flag) const
    {
        return _value & flag;
    }
    bool TestFlag(qint32 flag) const
    {
        return _value & flag;
    }

    void Foreach(const std::function<void (ValueType flag)>& handler) const
    {
        ValueType it = 1;
        while(it != 0) {
            if((_value & it) != 0) {
                handler(it);
            }
            it <<= 1;
        }
    }

    void ToString(QString& str) const
    {
        ValueType it = 1;
        while(it != 0) {
            str += ((_value & it) != 0) ? "1" : "0";
            it <<= 1;
        }
    }

    QString ToString() const
    {
        QString res; ToString(res); return res;
    }

    Flags& ChangeFromBoolean(bool flag, ValueType flags)
    {
        FlagsHelpers<ValueType>::ChangeFromBoolean(flag, _value, flags);
        return *this;
    }

    Flags& operator |=(const Flags other)
    {
        this->_value |= other._value;
        return *this;
    }
    Flags& operator &=(const Flags other)
    {
        this->_value &= other._value;
        return *this;
    }
    Flags& operator ^=(const Flags other)
    {
        this->_value ^= other._value;
        return *this;
    }

    Flags& operator =(const ValueType e)
    {
        this->_value = e;
        return *this;
    }

    operator ValueType&()
    {
        return this->_value;
    }

    operator ValueType() const
    {
        return this->_value;
    }

    friend QDebug operator<<(QDebug debug, const Flags& data)
    {
        return debug << data._value;
    }

private:
    ValueType _value;
};

#define DECL_ATOMIC_FLAGS(flags, Enum) \
typedef Flags<std::atomic_int32_t, Enum> flags;

#define DECL_FLAGS(flags, Enum) \
typedef Flags<int32_t, Enum> flags;

#define DECL_FLAGS_64(flags, Enum) \
typedef Flags<int64_t, Enum> flags;

#endif // ATOMICFLAGS_H
