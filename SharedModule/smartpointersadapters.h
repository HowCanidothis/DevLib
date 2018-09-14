#ifndef SMARTPOINTERSADAPTERS_H
#define SMARTPOINTERSADAPTERS_H

#include <memory>

template<typename T>
class ScopedPointer : public std::unique_ptr<T>
{
    typedef std::unique_ptr<T> Super;
public:
    using Super::Super;

    ScopedPointer& operator=(T* ptr) { reset(ptr); return *this; }

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

template<class T>
ScopedPointer<T> make_scoped(T* ptr) { return ScopedPointer<T>(ptr); }
template<class T>
ScopedPointer<T> make_unique(T* ptr) { return ScopedPointer<T>(ptr); }

template<typename T>
class SharedPointer : public std::shared_ptr<T>
{
    typedef std::shared_ptr<T> Super;
public:
    using Super::Super;
};

template<class T>
SharedPointer<T> make_shared(T* ptr) { return SharedPointer<T>(ptr); }

#endif // SMARTPOINTERSADAPTERS_H
