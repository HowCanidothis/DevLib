#ifndef ATOMICFLAGS_H
#define ATOMICFLAGS_H

#include <Qt>
#include <atomic>

template<typename Enum>
class AtomicFlags{
    std::atomic_int32_t i;
public:
    inline qint32 toInt() const { return i; }
    typedef Enum enum_type;
    constexpr AtomicFlags(qint32 i=0) { this->i = i; }
    constexpr AtomicFlags(Enum e) { i = e; }


    void setValue(qint32 f) { i = f; }

    void setFlags(qint32 f){ i |= f; }
    void unsetFlags(qint32 f) { i &= ~f; }
    bool testFlagsAll(qint32 fs) const { return (i & fs) == fs; }
    bool testFlagsAtLeastOne(qint32 fs) const { return (i & fs); }

    void switchFlag(Enum f) { i ^= f; }
    void setFlag(Enum f){ i |= f; }
    void unsetFlag(Enum f) { i &= ~f; }
    bool testFlag(Enum f) const { return i & f; }

    void changeFromBoolean(qint32 flags, bool flag) { flag ? setFlags(flags) : unsetFlags(flags); }

    AtomicFlags& operator |=(const AtomicFlags other) { this->i |= other.i; return *this; }
    AtomicFlags& operator &=(const AtomicFlags other) { this->i &= other.i; return *this; }
    AtomicFlags& operator ^=(const AtomicFlags other) { this->i ^= other.i; return *this; }

    AtomicFlags& operator =(const qint32 e) { this->i = e; return *this; }

    operator qint32() const { return this->i; }
};

#define DECL_ATOMIC_FLAGS(flags, Enum) \
typedef AtomicFlags<Enum> flags;

#endif // ATOMICFLAGS_H
