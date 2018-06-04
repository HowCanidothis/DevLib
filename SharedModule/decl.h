#ifndef SHAREDGUIMODULE_DECL_H
#define SHAREDGUIMODULE_DECL_H

#include <QMetaEnum>

#include <QDebug>

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

#define NO_EXCEPT noexcept

class QTextStream;

class DirBinder
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

namespace adapters{

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

template <typename T, typename Cleanup = QScopedPointerDeleter<T> >
class ScopedPointer : public QScopedPointer<T, Cleanup>
{
    typedef QScopedPointer<T,Cleanup> Super;
public:
    ScopedPointer(T* p = nullptr) : Super(p) {}

    ScopedPointer& operator=(T* ptr) { reset(ptr); return *this; }

    T* data() { return Super::data(); }
    const T* data() const { return Super::data(); }

    const T* operator->() const { return Super::operator ->(); }
    T* operator->() { return Super::operator ->(); }
    const T& operator*() const { return Super::operator *(); }
    T& operator*() { return Super::operator *(); }
};

template<class T>
class SharedPointer : public QSharedPointer<T>
{
    typedef QSharedPointer<T> Super;
public:
    SharedPointer(T* p=nullptr) : Super(p) {}
};

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

class Messager
{
    const char* _label;
    QByteArray _location;
public:
    Messager(const char* lbl, const char* file, qint32 line);

    void Error(const char* fmt, ...) const;
    void Warning(const char* fmt, ...) const;
    void Info(const char* fmt, ...) const;

    QDebug Error() const;
    QDebug Warning() const;
    QDebug Info() const;
};

#define tDebug \
    (qDebug().nospace() << "   Loc: [" << __FILE__ << ":" << __LINE__ << "] ").space() << QString::asprintf

#define LOGOUT Messager log(__FUNCTION__, __FILE__, __LINE__)

#define ASSURE(expression, message, what) \
    if(!expression) { \
        tDebug << __FUNCTION__ << message << ": " << what; \
    }

//========================================================DEBUG ONLY================================================

#ifdef _MSC_VER
    //if msvc compiler
#endif





#endif // SHARED_DECL_H
