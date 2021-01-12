#ifndef SHAREDGUIMODULE_DECL_H
#define SHAREDGUIMODULE_DECL_H

#include <QMetaEnum>
#include <QLoggingCategory>

#include <functional>
#include <cmath>

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
typedef std::function<void ()> FAction;

class QTextStream;

enum class EPriority {
    High,
    Low,
    Count
};

class _Export DirBinder
{
    QString old_path;
public:
    explicit DirBinder(const QString& dir);
    ~DirBinder();
};

template<typename T>
T clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : (max < value) ? max : value;
}

inline double sign(double value)
{
    if(value < 0.0) {
        return -1.0;
    }
    if(value > 0.0) {
        return 1.0;
    }
    return 0.0;
}

inline bool fuzzyCompare(double v1, double v2, double epsilon = std::numeric_limits<double>().epsilon())
{
    return qAbs(v1 - v2) < epsilon;
}

inline bool fuzzyCompare(float v1, float v2, float epsilon = std::numeric_limits<float>().epsilon())
{
    return qAbs(v1 - v2) < epsilon;
}

inline bool fuzzyIsNull(float v1, float epsilon = std::numeric_limits<float>().epsilon())
{
    return qAbs(v1 - 0.f) < epsilon;
}

inline bool fuzzyIsNull(double v1, double epsilon = std::numeric_limits<double>().epsilon())
{
    return qAbs(v1 - 0.0) < epsilon;
}

inline double round(double value, int decimals)
{
    return std::round(value * decimals) / decimals;
}

inline float round(float value, int decimals)
{
    return std::round(value * decimals) / decimals;
}

inline QString dToStr(double value, qint32 precision = 2)
{
    return QString::number(value, 'd', precision);
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
Range<It> reverse(ORange&& originalRange) {
    return Range<It>(It(std::end(originalRange)), It(std::begin(originalRange)));
}

template<typename It, typename OIt = std::reverse_iterator<It>>
Range<OIt> reverse(It begin, It end) {
    return reverse(Range<It>(begin, end));
}

template<typename It>
Range<It> range(It begin, It end) {
    return Range<It>(begin, end);
}


}

template<typename Enum>
struct EnumHelper
{
    static QStringList GetNames();
};

enum Sides {
    Left,
    Right,
    Bottom,
    Top,
    Sides_Count
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

class LambdaGuard
{
    FAction m_release;
public:
    LambdaGuard(const FAction& release, const FAction& bind = nullptr)
        : m_release(release)
    {
        if(bind != nullptr) {
            bind();
        }
    }

    ~LambdaGuard()
    {
        m_release();
    }
};

template<class Owner, typename BindFunc, typename ReleaseFunc>
CommonGuard<Owner, BindFunc, ReleaseFunc> make(Owner* owner, BindFunc bind, ReleaseFunc release)
{
    return CommonGuard<Owner, BindFunc, ReleaseFunc>(owner, bind, release);
}

}

struct CastablePtr{
public:
    CastablePtr(const void* d) : m_data(d) {}

    template<class T> const T& As() const { return *AsPtr<T>(); }
    template<class T> const T* AsPtr() const { return (T*)m_data; }
    const char* AsCharPtr() const { return AsPtr<char>(); }

    template<class T> T& As() { return *AsPtr<T>(); }
    template<class T> T* AsPtr() { return (T*)m_data; }
    char* AsCharPtr() { return AsPtr<char>(); }

    void operator+=(qint32 bytes) { m_data = this->AsCharPtr() + bytes; }
    CastablePtr operator+(quint16 bytes) const { return this->AsCharPtr() + bytes; }
    CastablePtr operator+(quint32 bytes) const { return this->AsCharPtr() + bytes; }
    CastablePtr operator+(quint64 bytes) const { return this->AsCharPtr() + bytes; }

    operator size_t() const { return (size_t)this->m_data; }

    static size_t Difference(const void* p1, const void* p2) {
        return (const char*)p1 - (const char*)p2;
    }

private:
    const void* m_data;
};
Q_DECLARE_TYPEINFO(CastablePtr, Q_PRIMITIVE_TYPE);

template<class T>
class NamedClassReferenceWrapper
{
    T& m_reference;
public:
    NamedClassReferenceWrapper(T& reference)
        : m_reference(reference)
    {}

    operator qint32() const { return m_reference; }
};

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
