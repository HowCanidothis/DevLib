#ifndef SMARTPOINTERSADAPTERS_H
#define SMARTPOINTERSADAPTERS_H

#include <memory>
#include <functional>

template<typename T, typename deleter = std::default_delete<T>>
class ScopedPointer : public std::unique_ptr<T, deleter>
{
    typedef std::unique_ptr<T> Super;
public:
    using Super::Super;

    ScopedPointer& operator=(T* ptr) { this->reset(ptr); return *this; }

    T* get() { return Super::get(); }
    const T* get() const { return Super::get(); }

    T* data() { return Super::get(); }
    const T* data() const { return Super::get(); }

    const T* operator->() const { return Super::operator ->(); }
    T* operator->() { return Super::operator ->(); }
    const T& operator*() const { return Super::operator *(); }
    T& operator*() { return Super::operator *(); }

    bool isNull() const { return this->data() == nullptr; }
};

template<class T, typename ... Args>
ScopedPointer<T> make_scoped(Args ... args) { return ScopedPointer<T>(new T(args...)); }

template<class T, typename ... Args>
ScopedPointer<T> make_unique(Args ... args) { return ScopedPointer<T>(new T(args...)); }

template<typename T>
class SharedPointer : public std::shared_ptr<T>
{
    typedef std::shared_ptr<T> Super;
public:
    using Super::Super;

    SharedPointer& operator=(T* ptr) { this->reset(ptr); return *this; }
    SharedPointer& operator=(const std::shared_ptr<T>& ptr) { Super::operator=(ptr); return *this; }

    template<typename T2>
    const SharedPointer<T2>& Cast() const { return *reinterpret_cast<const SharedPointer<T2>*>(this); }
    template<typename T2>
    SharedPointer<T2>& Cast() { return *reinterpret_cast<SharedPointer<T2>*>(this); }
};

template<class T> using SP = SharedPointer<T>;

template<class T, typename DefaultDeleter, typename ... Args>
SharedPointer<T> make_shared(Args ... args) { return SharedPointer<T>(new T(args...), DefaultDeleter()); }

template<class T, typename ... Args>
SharedPointer<T> make_shared(Args ... args) { return SharedPointer<T>(new T(args...)); }

#define FIRST_DECLARE_POINTERS_BASE(type, sharedPointer, scopedPointer) \
using type##Ptr = sharedPointer<class type>; \
using type##UPtr = scopedPointer<type>

#define FIRST_DECLARE_POINTERS(type) \
FIRST_DECLARE_POINTERS_BASE(type, SharedPointer, ScopedPointer)

template<class T>
class SharedPointerInitialized : public SharedPointer<T>
{
    using Super = SharedPointer<T>;
public:
    SharedPointerInitialized()
        : Super(::make_shared<T>())
    {}
};

class SmartPointerWatcher
{
public:
    SmartPointerWatcher(const std::function<void ()>& disconnector);
    ~SmartPointerWatcher();

private:
    friend class SmartPointer;
    void disconnect();

private:
    std::function<void ()> m_disconnector;
};

using SmartPointerWatcherPtr = SharedPointer<SmartPointerWatcher>;
using SmartPointerWatchers = QVector<SmartPointerWatcherPtr>;

inline SharedPointer<SmartPointerWatcherPtr> SmartPointerWatcherCreate() { return ::make_shared<SmartPointerWatcherPtr>(); }
inline SharedPointer<SmartPointerWatchers> SmartPointerWatchersCreate() { return ::make_shared<SmartPointerWatchers>(); }

class SmartPointer
{
public:
    SmartPointer(const std::function<void ()>& onCaptured, const std::function<void ()>& onReleased);
    ~SmartPointer();

    SmartPointerWatcherPtr Capture();

private:
    std::function<void ()> m_onCaptured;
    std::function<void ()> m_onReleased;
    std::weak_ptr<SmartPointerWatcher> m_watcher;
};

#endif // SMARTPOINTERSADAPTERS_H
