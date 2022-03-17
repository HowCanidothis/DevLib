#ifndef SHAREDGUIMODULE_DECL_H
#define SHAREDGUIMODULE_DECL_H

#include <QMetaEnum>
#include <QLoggingCategory>

#include <functional>
#include <cmath>

#include "flags.h"

enum SerializationMode {
    SerializationMode_Default = 0x0,
    SerializationMode_InvokeProperties = 0x1,
    SerializationMode_MinMaxProperties = 0x2,
    SerializationMode_Sorted_Containers = 0x4,
    SerializationMode_Comparison = 0x8,
    SerializationMode_Merge_Containers = 0x10,
    SerializationMode_Cloning = 0x20,
    SerializationMode_UserDefined = 0x200
};
DECL_FLAGS(SerializationModes, SerializationMode);

class Nanosecs
{
public:
    Nanosecs(double nsecs)
        : _nsecs(nsecs)
    {}

    double TimesPerSecond() const;
    QString ToString(const QString& caption) const;

    operator double() const { return _nsecs; }

private:
    double _nsecs;
};

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

template<typename T>
T clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : (max < value) ? max : value;
}

template<class T>
T lerp(const T& a, const T& b, double t)
{
    return a + t * (b - a);
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

namespace sm_internal
{
    template<std::uint64_t B, unsigned char E, typename T>
    struct power_of
    {
        static constexpr T value = T(B) * power_of<B, E - 1, T>::value;
    };

    template<std::uint64_t B, typename T>
    struct power_of<B, 0, T>
    {
        static constexpr T value = T(1);
    };

    template<std::uint64_t B, unsigned char E, typename T>
    inline constexpr auto power_of_v = power_of<B, E, T>::value;
}

inline double epsilon(int decimals)
{
    return 1.0 / pow(10, decimals);
}

inline double round(double value, int decimals)
{
    return std::round(value * decimals) / decimals;
}

inline float round(float value, int decimals)
{
    return std::round(value * decimals) / decimals;
}

template<qint32 Decimals>
inline double round(double value)
{
    return round(value, sm_internal::power_of<10,Decimals,double>::value);
}

template<qint32 Decimals>
inline float round(float value)
{
    return round(value, sm_internal::power_of<10,Decimals,float>::value);
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

template <class T>
inline const T& make_const(T& container) { return container; }

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

namespace lq
{

template<class Container>
class Iterators : public QList<typename Container::const_iterator>
{
    using Super = QList<typename Container::const_iterator>;
public:
    using value_type = typename Container::value_type;

    struct const_iterator
    {
        using true_iterator = typename Super::const_iterator;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = qptrdiff;
        using value_type = typename Container::value_type;
        using pointer = value_type*;
        using reference = value_type&;

        const_iterator(const true_iterator& iterator)
            : m_it(iterator)
        {}
        const_iterator& operator++()
        {
            ++m_it;
            return *this;
        }

        const_iterator operator+(qint32 count) const
        {
            return const_iterator(m_it + count);
        }

        const value_type& operator*() const
        {
            return **m_it;
        }

        bool operator!=(const const_iterator& another) const
        {
            return m_it != another.m_it;
        }

    private:
        true_iterator m_it;
    };

    const_iterator begin() const { return const_iterator(Super::cbegin()); }
    const_iterator end() const { return const_iterator(Super::cend()); }
};

template<class Container>
bool Join(const Container& container, const std::function<void (const typename Container::value_type& value)>& selector,
          const std::function<void (const typename Container::value_type& value)>& lastSelector)
{
    if(container.begin() == container.end()) {
        return false;
    }
    for(const auto& value : adapters::range(container.begin(), container.end() - 1)) {
        selector(value);
    }
    lastSelector(*(container.end() - 1));

    return true;
}

template<class Container>
bool JoinIt(const Container& container, const std::function<void (const typename Container::const_iterator& value)>& selector,
          const std::function<void (const typename Container::const_iterator& value)>& lastSelector)
{
    if(container.begin() == container.end()) {
        return false;
    }
    for(auto it(container.cbegin()), e(container.cend() - 1); it != e; ++it) {
        selector(it);
    }
    lastSelector(container.end() - 1);

    return true;
}

template<class Container>
QString Join(const QChar& separator, const Container& container)
{
    QString result;
    Join(container, [&](const typename Container::value_type& value){
        result.append(value);
        result.append(separator);
    }, [&](const typename Container::value_type& value){
        result.append(value);
    });
    return result;
}

template<class Container>
QString Join(const QString& separator, const Container& container)
{
    QString result;
    Join(container, [&](const typename Container::value_type& value){
        result.append(value);
        result.append(separator);
    }, [&](const typename Container::value_type& value){
        result.append(value);
    });
    return result;
}

template<class T, class Container>
QList<T> Select(const Container& container, const std::function<T (const typename Container::value_type& value)>& selector)
{
    QList<T> result;
    for(const auto& value : container) {
        result.append(selector(value));
    }
    return result;
}

template<class T, class Container>
QList<T> SelectIt(const Container& container, const std::function<T (const typename Container::const_iterator& value)>& selector)
{
    QList<T> result;
    for(auto it(container.cbegin()), e(container.cend()); it != e; ++it) {
        result.append(selector(it));
    }
    return result;
}

template<class Container>
Iterators<Container> Where(const Container& container, const std::function<bool (const typename Container::value_type& value)>& selector)
{
    Iterators<Container> result;
    for(auto it(container.begin()), e(container.end()); it != e; ++it) {
        if(selector(*it)) {
            result.append(it);
        }
    }
    return result;
}
}

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

    void Reset(const FAction& release)
    {
        m_release = release;
    }

    ~LambdaGuard()
    {
        m_release();
    }
};

class BooleanGuard : public LambdaGuard
{
    using Super = LambdaGuard;
public:
    enum InvertedConstructor
    {
        Inverted
    };

    BooleanGuard(bool* value)
        : Super([value]{
            *value = false;
        }, [value]{
            *value = true;
        })
    {}

    BooleanGuard(bool* value, InvertedConstructor)
        : Super([value]{
            *value = true;
        }, [value]{
            *value = false;
        })
    {}
};

template<class Owner, typename BindFunc, typename ReleaseFunc>
CommonGuard<Owner, BindFunc, ReleaseFunc> make(Owner* owner, BindFunc bind, ReleaseFunc release)
{
    return CommonGuard<Owner, BindFunc, ReleaseFunc>(owner, bind, release);
}

}

template<class T>
class Singletone
{
public:
    Singletone()
    {
        Q_ASSERT(m_instance == nullptr);
        m_instance = reinterpret_cast<T*>(this);
    }
    virtual ~Singletone()
    {
        m_instance = nullptr;
    }

    static T& GetInstance()
    {
        Q_ASSERT(m_instance != nullptr);
        return *m_instance;
    }

private:
    static T* m_instance;
};

template<class T>
T* Singletone<T>::m_instance = nullptr;

template<class T>
class SingletoneGlobal
{
public:
    static T& GetInstance()
    {
        return m_instance;
    }

private:
    static T m_instance;
};

#define ENABLE_SINGLETONE_ACCESS(className) \
    template<class T> friend class SingletoneGlobal

template<class T>
T SingletoneGlobal<T>::m_instance;

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
