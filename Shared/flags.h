#ifndef FLAGS_H
#define FLAGS_H

#include <Qt>

template<typename Enum>
class Flags{
    qint32 i;
public:
    inline qint32 toInt() const { return i; }
    typedef Enum enum_type;
    Q_CONSTEXPR Flags(qint32 _i=0): i(_i){}
    Q_CONSTEXPR Flags(Enum e) : i(e){}


    void setValue(qint32 f) { i = f; }

    void setFlags(qint32 f){ i |= f; }
    void unsetFlags(qint32 f) { i &= ~f; }
    bool testFlagsAll(qint32 fs) const { return (i & fs) == fs; }
    bool testFlagsAtLeastOne(qint32 fs) const { return (i & fs); }

    void setFlag(Enum f){ i |= f; }
    void unsetFlag(Enum f) { i &= ~f; }
    bool testFlag(Enum f) const { return i & f; }

    void changeFromBoolean(qint32 flags, bool flag) { flag ? setFlags(flags) : unsetFlags(flags); }

    Flags& operator |=(const Flags other) { this->i |= other.i; return *this; }
    Flags& operator &=(const Flags other) { this->i &= other.i; return *this; }
    Flags& operator ^=(const Flags other) { this->i ^= other.i; return *this; }

    operator qint32() const { return this->i; }
};

#define DECL_FLAGS(flags, Enum) \
typedef Flags<Enum> flags;


#endif // FLAGS

