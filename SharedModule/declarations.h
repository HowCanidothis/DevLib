#ifndef SHAREDGUIMODULE_DECL_H
#define SHAREDGUIMODULE_DECL_H

#include <QMetaEnum>
#include <QDate>
#include <QDateTime>
#include <QLoggingCategory>

#include <functional>
#include <cmath>

#include "flags.h"
#include "debugobjectinfo.h"

#define THREAD_ASSERT_IS_THREAD(thread) Q_ASSERT(thread == QThread::currentThread());
#define THREAD_ASSERT_IS_NOT_THREAD(thread) Q_ASSERT(thread != QThread::currentThread());
#define THREAD_ASSERT_IS_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_THREAD(qApp->thread()); }
#define THREAD_ASSERT_IS_NOT_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_NOT_THREAD(qApp->thread()); }


struct Comparator
{
    template<class T>
    static bool Equal(const T& v1, const T& v2) { return v1 == v2; }
    template<class T>
    static bool NotEqual(const T& v1, const T& v2) { return !Equal(v1, v2); }
};

template<>
inline bool Comparator::Equal(const double& v1, const double& v2) { return (qIsNaN(v1) && qIsNaN(v2)) || qFuzzyCompare(v1, v2); }
template<>
inline bool Comparator::Equal(const float& v1, const float& v2) { return (qIsNaN(v1) && qIsNaN(v2)) || qFuzzyCompare(v1, v2); }

enum AdditionalStandardRoles {
    ObjectRole = Qt::UserRole,
    IdRole,
    MinLimitRole,
    MaxLimitRole,
    UnitRole,
    LastEditRowRole,
    SpanRole
};

#define FIRST_DECLARE(container) template<class, template<typename> class> class container;

const auto FInverseBool = [](bool b) { return !b; };
const auto FDirectBool = [](bool b) { return b; };
const auto FDirectBoolToInt = [](bool b) -> qint32 { return b ? 0xffffffff : 0; };
const auto FInverseBoolToInt = [](bool b) -> qint32 { return !b ? 0xffffffff : 0; };
const auto FIdleAction = []{};
using FAction = std::function<void ()>;
using FTranslationHandler = std::function<QString ()>;
#define TR(x, ...) [__VA_ARGS__]{ return x; }

#define DECLARE_GLOBAL(Type, name) \
extern const Type name;

#define IMPLEMENT_GLOBAL(Type, name, ...) \
const Type name(__VA_ARGS__);

#define DECLARE_GLOBAL_CHAR(name) \
    DECLARE_GLOBAL(char*, name)

#define IMPLEMENT_GLOBAL_CHAR(name, value) \
    const char* name = QT_STRINGIFY(value);

#define IMPLEMENT_GLOBAL_CHAR_1(name) \
    const char* name = QT_STRINGIFY(name);

template<class T>
struct Default
{
    static const T Value;
};

#define IMPLEMENT_DEFAULT_PTR(type) \
template<> const SharedPointer<type> Default<SharedPointer<type>>::Value;

#define IMPLEMENT_DEFAULT(type) \
template<> const type Default<type>::Value;

#define IMPLEMENT_DEFAULT_WITH_PARAMS(type, ...) \
template<> const type Default<type>::Value(__VA_ARGS__);

DECLARE_GLOBAL(FTranslationHandler, TR_NONE);
DECLARE_GLOBAL(QString, DASH);

#define toPointer(x) auto* p##x = x.get()

#if defined(QT_GUI_LIB) && !defined(FORCE_NO_UI)
    #define SHARED_LIB_ADD_UI
#endif

#define DELCARE_WITH_FIELD_BASE(FieldType, FieldName, Label) \
template<class T> \
class With##Label : public T \
{ \
    using Super = T; \
public: \
    using Super::Super; \
\
    T& ToBase() { return *this; } \
    const T& ToBase() const { return *this; } \
\
    FieldType FieldName; \
}
#define DECLARE_WITH_FIELD(FieldType, FieldName) DELCARE_WITH_FIELD_BASE(FieldType, FieldName, FieldType)


enum SerializationMode {
    SerializationMode_Default = 0x0,
    SerializationMode_InvokeProperties = 0x1,
    SerializationMode_MinMaxProperties = 0x2,
    SerializationMode_Sorted_Containers = 0x4,
    SerializationMode_Comparison = 0x8,
    SerializationMode_Merge_Containers = 0x10,
    SerializationMode_Cloning = 0x20,
    SerializationMode_Synchronization = 0x40,
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
    return qAbs(v1) < epsilon;
}

inline bool fuzzyIsNull(double v1, double epsilon = std::numeric_limits<double>().epsilon())
{
    return qAbs(v1) < epsilon;
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

inline bool IsInBounds(double value, double left, double right, double epsilon)
{
    if(fuzzyCompare(value, left, epsilon)) {
        return true;
    }
    if(fuzzyCompare(value, right, epsilon)) {
        return true;
    }
    auto minmax = std::minmax(left, right);
    return value > minmax.first && value < minmax.second;
}

template<class T>
inline bool IsInBounds(const T& value, const T& left, const T& right)
{
    auto minmax = std::minmax(left, right);
    return value >= minmax.first && value <= minmax.second;
}

template <typename FHandler, typename T, typename ... Args>
void Combine(const FHandler& handler, T& first, Args&... rest) {
    handler(first);

    if constexpr (sizeof...(rest) > 0) {
        Combine(handler, rest...);
    }
}

template <typename FHandler>
void Combine(const FHandler& handler)
{

}

template<class T>
T& ToReference(T& t)
{
    return t;
}

template<class T>
T& ToReference(SharedPointer<T>& tPtr)
{
    return *tPtr;
}

template<class T>
T& ToReference(ScopedPointer<T>& tPtr)
{
    return *tPtr;
}

template<class T>
T& ToReference(T*& t)
{
    return *t;
}

template<typename ... Args>
void Initialize(Args&... args)
{
    Combine([](auto& p){
        ToReference(p).Initialize();
    }, args...);
}

template<class IteratorType, class Struct>
inline void ForeachFieldOfStructByRef(const Struct& data, const std::function<void (IteratorType&)>& handler) {
    auto size = sizeof(data);
    qint32 count = size / sizeof(IteratorType);
    IteratorType* it = (IteratorType*)&data;
    while(count--) {
        handler(*it);
        it++;
    }
}

template<class IteratorType, class Struct>
inline void ForeachFieldOfStruct(const Struct& data, const std::function<void (const IteratorType&)>& handler) {
    auto size = sizeof(data);
    qint32 count = size / sizeof(IteratorType);
    IteratorType* it = (IteratorType*)&data;
    while(count--) {
        handler(*it);
        it++;
    }
}

template<typename FFunction, class T, class T2, typename ... Args>
bool Foreach(const FFunction& handler, T& c1, T2& c2, Args&... args)
{
    qint32 minSize = std::numeric_limits<qint32>().max();
    adapters::Combine([&](const auto& t){
        minSize = std::min(t.size(), minSize);
    }, c1, c2, args...);


    auto f = c1.begin();
    auto s = c2.begin();
    for(qint32 i(0); i < minSize; ++i) {
        handler(*f, *s, *(args.begin() + i)...);
        ++f;
        ++s;
    }

    return true;
}

template<typename FFunction, class T, class T2, typename ... Args>
bool ForeachConst(const FFunction& handler, const T& c1, const T2& c2, const Args&... args)
{
    qint32 minSize = std::numeric_limits<qint32>().max();
    adapters::Combine([&](const auto& t){
        minSize = std::min(t.size(), minSize);
    }, c1, c2, args...);


    auto f = c1.begin();
    auto s = c2.begin();
    for(qint32 i(0); i < minSize; ++i) {
        handler(*f, *s, *(args.begin() + i)...);
        ++f;
        ++s;
    }

    return true;
}

template<typename First, typename ... Args>
QVector<First> toVector(const First& first, Args... args) { return QVector<First>({ first, args... }); }

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

template<typename ORange, typename It = decltype(std::begin(std::declval<ORange>()))>
Range<It> withoutFirst(ORange&& originalRange) {
    auto b = std::begin(originalRange), e = std::end(originalRange);
    if(b == e) {
        return Range<It>(b, e);
    }
    return Range<It>(b + 1, e);
}

template<typename ORange, typename It = decltype(std::begin(std::declval<ORange>()))>
Range<It> withoutLast(ORange&& originalRange) {
    auto b = std::begin(originalRange), e = std::end(originalRange);
    if(b == e) {
        return Range<It>(b, e);
    }
    return Range<It>(b, e - 1);
}

template<typename Container, typename It = typename Container::const_iterator>
Range<It> range(const Container& container, qint32 startIndex, qint32 count) {
    return Range<It>(container.begin() + startIndex, container.begin() + startIndex + count);
}

template<typename Container, typename It = typename Container::const_iterator>
Range<It> range(Container& container, qint32 startIndex, qint32 count) {
    return Range<It>(container.begin() + startIndex, container.begin() + startIndex + count);
}

template<typename Container, typename It = typename Container::const_iterator>
Range<It> range(Container& container, qint32 startIndex) {
    return Range<It>(container.begin() + startIndex, container.begin() + startIndex + container.size() - startIndex);
}

}

template <class T>
inline const T& make_const(T& container) { return container; }

template <class T>
inline T make_copy(const T& container)
{
    T result;
    result.reserve(container.size());
    for(const auto& val : container){
        result.append(val);
    }
    return result;
}

template <class T>
inline T make_copy_if_not_detached(const T& container)
{
    if(container.isDetached()) {
        return container;
    }
    return make_copy(container);
}

template<typename Enum>
struct EnumHelper
{
    template<typename T> static T Validate(qint64 value);
    static QStringList GetNames();
};

template<>
struct EnumHelper<void>
{
    template<typename T> static T Validate(qint64 value) { return (T)::clamp(value, (qint64)T::First, (qint64)T::Last); }
    static QStringList GetNames();
};

using EnumHelperCommon = EnumHelper<void>;

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

    void Reset(const FAction& release)
    {
        m_release = release;
    }

    ~LambdaGuard()
    {
        m_release();
    }
};

using LambdaGuardPtr = SharedPointer<LambdaGuard>;

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
class TCachedExpression
{
public:
    using FEvaluator = std::function<T()>;
    TCachedExpression()
    {}
    TCachedExpression(nullptr_t)
    {}
    TCachedExpression(const std::function<T ()>& evaluator)
        : m_evaluator(evaluator)
        , m_initial(evaluator)
    {}

    bool operator!=(nullptr_t) const
    {
        return m_evaluator != nullptr;
    }

    void Reset()
    {
        m_evaluator = m_initial;
    }

    T operator()() const
    {
        auto result = m_evaluator();
        m_evaluator = [result]{ return result; };
        return result;
    }

private:
    mutable FEvaluator m_evaluator;
    FEvaluator m_initial;
};

using CachedExpressionDouble = TCachedExpression<double>;
using FMeasurementGetter = TCachedExpression<const class Measurement*>;

template<class T>
class Singletone
{
public:
    Singletone()
    {
        Q_ASSERT(m_instance == nullptr);
        m_instance = reinterpret_cast<T*>(this);
    }
    Singletone(T* instance)
    {
        Q_ASSERT(m_instance == nullptr);
        m_instance = instance;
    }
    virtual ~Singletone()
    {
        m_instance = nullptr;
    }

    static bool IsInitialized() { return m_instance != nullptr; }

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
_Export Q_DECLARE_LOGGING_CATEGORY(LC_CONSOLE)

#if !defined QT_NO_DEBUG || defined QT_PROFILE
#define SHOW_HIDDEN_FUNCTIONALITY
#endif

//========================================================DEBUG ONLY================================================

#ifdef _MSC_VER
    //if msvc compiler
#endif

template<class T>
T& GlobalSelfGetter(T& v) { return v; };

template<class T>
T* GlobalSelfGetterPointer(T& v) { return &v; };

#endif // SHARED_DECL_H
