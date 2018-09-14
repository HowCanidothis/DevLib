#ifndef SHAREDGUIMODULE_DECL_H
#define SHAREDGUIMODULE_DECL_H

#include <QMetaEnum>
#include <QLoggingCategory>

#ifndef STATIC_LINK
#if defined(LIBRARY)
#  define _Export Q_DECL_EXPORT
#else
#  define _Export Q_DECL_IMPORT
#endif
#else
#  define _Export
#endif

typedef qint32 count_t;

class QTextStream;

class _Export DirBinder
{
    QString old_path;
public:
    explicit DirBinder(const QString& dir);
    ~DirBinder();
};

template<typename T>
T clamp(const T& value, const T& min, const T& max) {
    return std::min(std::max(value, min), max);
}

namespace adapters {

template<typename It>
class Range
{
    It b, e;
public:
    Range() : b(e) {}
    Range(It b, qint32 count) : b(b), e(b + count) {}
    Range(It b, It e) : b(b), e(e) {}
    qint32 size() const { return std::distance(b,e); }
    It begin() const { return b; }
    It end() const { return e; }
};

template<typename ORange, typename OIt = decltype(std::begin(std::declval<ORange>())), typename It = std::reverse_iterator<OIt>>
Range<It> reverse(ORange && originalRange) {
    return Range<It>(It(std::end(originalRange)), It(std::begin(originalRange)));
}

template<typename It>
Range<It> reverse(It begin, It end) {
    return reverse(Range<It>(begin, end));
}

template<typename It>
Range<It> range(It begin, It end) {
    return Range<It>(begin, end);
}


}

struct CastablePtr;
struct ConstCastablePtr{
    const void* data;
    ConstCastablePtr(const void* d) : data(d) {}

    template<class T> const T* asPtr() const { return (T*)data; }
    template<class T> const T& as() const { return *(T*)data; }
    const char *asCharPtr() const { return asPtr<const char>(); }

    ConstCastablePtr operator+(quint16 v) const { return this->asCharPtr() + v; }
    ConstCastablePtr operator+(quint32 v) const { return this->asCharPtr() + v; }
    ConstCastablePtr operator+(quint64 v) const { return this->asCharPtr() + v; }

    CastablePtr* constCastPtr() const { return (CastablePtr*)this; }
    const CastablePtr& constCast() const { return *(CastablePtr*)this; }

    static size_t difference(const void* p1, const void* p2) {
        return (const char*)p1 - (const char*)p2;
    }
};
Q_DECLARE_TYPEINFO(ConstCastablePtr, Q_PRIMITIVE_TYPE);

enum Sides {
    Left,
    Right,
    Bottom,
    Top
};

namespace guards {

template<class Owner, typename BindFunc, typename ReleaseFunc>
class CommonGuard
{
    Owner* _owner;
    ReleaseFunc _release;
public:
    CommonGuard(Owner* owner, BindFunc bind, ReleaseFunc release) Q_DECL_NOEXCEPT
        : _owner(owner)
        , _release(release)
    {
        (_owner->*bind)();
    }
    ~CommonGuard()
    {
        (_owner->*_release)();
    }
};

template<class Owner, typename BindFunc, typename ReleaseFunc>
CommonGuard<Owner, BindFunc, ReleaseFunc> make(Owner* owner, BindFunc bind, ReleaseFunc release)
{
    return CommonGuard<Owner, BindFunc, ReleaseFunc>(owner, bind, release);
}

}

struct CastablePtr{
    void* data;
    CastablePtr(void* d) : data(d) {}

    template<class T> T& as() const { return *asPtr<T>(); }
    template<class T> T* asPtr() const { return (T*)data; }
    char* asCharPtr() const { return asPtr<char>(); }

    CastablePtr operator+(quint16 v) const { return this->asCharPtr() + v; }
    CastablePtr operator+(quint32 v) const { return this->asCharPtr() + v; }
    CastablePtr operator+(quint64 v) const { return this->asCharPtr() + v; }

    ConstCastablePtr* toConstPtr() const { return (ConstCastablePtr*)this; }
    const ConstCastablePtr& toConst() const { return *(ConstCastablePtr*)this; }

    operator size_t() const { return (size_t)this->data; }
};

Q_DECLARE_TYPEINFO(CastablePtr, Q_PRIMITIVE_TYPE);

_Export Q_DECLARE_LOGGING_CATEGORY(LC_UI)
_Export Q_DECLARE_LOGGING_CATEGORY(LC_SYSTEM)

#if !defined QT_NO_DEBUG || defined QT_PROFILE
#define SHOW_HIDDEN_FUNCTIONALITY
#endif

//========================================================DEBUG ONLY================================================

#ifdef _MSC_VER
    //if msvc compiler
#endif

#define FIRST_DECLARE(container) template<class, template<typename> class> class container;

#endif // SHARED_DECL_H
