#ifndef SMARTPOINTERSADAPTERS_H
#define SMARTPOINTERSADAPTERS_H

#include <memory>

template<typename T>
class ScopedPointer : public std::unique_ptr<T>
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

    template<typename T2>
    const SharedPointer<T2>& Cast() const { return *reinterpret_cast<const SharedPointer<T2>*>(this); }
    template<typename T2>
    SharedPointer<T2>& Cast() { return *reinterpret_cast<SharedPointer<T2>*>(this); }
};

template<class T, typename DefaultDeleter, typename ... Args>
SharedPointer<T> make_shared(Args ... args) { return SharedPointer<T>(new T(args...), DefaultDeleter()); }

template<class T, typename ... Args>
SharedPointer<T> make_shared(Args ... args) { return SharedPointer<T>(new T(args...)); }

#endif // SMARTPOINTERSADAPTERS_H
