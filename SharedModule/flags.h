#ifndef ATOMICFLAGS_H
#define ATOMICFLAGS_H

#include <atomic>

template<typename ValueType, typename Enum>
class Flags{
public:
    typedef Enum enum_type;

    inline int32_t ToInt() const
    {
        return _value;
    }

    constexpr Flags(int32_t i=0)
        : _value(i)
    {}
    constexpr Flags(Enum e)
        : _value(e)
    {}


    Flags& SetFlags(int32_t value)
    {
        _value = value;
        return *this;
    }

    Flags& AddFlags(int32_t flags)
    {
        _value |= flags;
        return *this;
    }
    Flags& RemoveFlags(int32_t flags)
    {
        _value &= ~flags;
        return *this;
    }
    bool TestFlagsAll(int32_t flags) const
    {
        return (_value & flags) == flags;
    }
    bool TestFlagsAtLeastOne(int32_t flags) const
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
    bool TestFlag(Enum flag) const
    {
        return _value & flag;
    }

    Flags& ChangeFromBoolean(int32_t flags, bool flag)
    {
        flag ? AddFlags(flags) : RemoveFlags(flags);
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

    Flags& operator =(const int32_t e)
    {
        this->_value = e;
        return *this;
    }

    operator int32_t() const
    {
        return this->_value;
    }

private:
    ValueType _value;
};

#define DECL_ATOMIC_FLAGS(flags, Enum) \
typedef Flags<std::atomic_int32_t, Enum> flags;

#define DECL_FLAGS(flags, Enum) \
typedef Flags<int32_t, Enum> flags;

#endif // ATOMICFLAGS_H
