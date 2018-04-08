#ifndef STACK_H
#define STACK_H
#include "shared_decl.h"
#include <memory>
#include <Qt>

template<class T>
class StackData
{
    T* _begin;
    count_t _count;
    count_t _reserved;
public:
    StackData(count_t reserve)
        : _reserved(reserve)
    {
        realloc();
    }
    StackData() : _begin(nullptr), _count(0), _reserved(0)
    {
    }
    StackData(std::initializer_list<T> args) : _begin(nullptr), _count(args.size()), _reserved(args.size()) {
        if(args.size()) {
            realloc();
            memcpy(begin(), args.begin(), args.size() * sizeof(T));
        }
    }
    ~StackData() { ::free(_begin);  }



    void resize(count_t count) {
        if(count > _reserved) {
            _reserved = count;
            realloc();
        }
        this->_count = count;
    }

    void clear() {
        _count = 0;
    }

    void dblRealloc() {
        if(_reserved > 1000000)
            _reserved += 5000000;
        else
            _reserved <<= 1;
        realloc();
    }
    void realloc() {
        _reserved = _reserved ? _reserved : defaultReserved();
        _begin = (T*)::realloc(_begin, _reserved * sizeof(T));

    }
    void push(const T& value) {
        if(_reserved == _count)
            dblRealloc();
        Q_ASSERT(_reserved != _count);
        *end() = value;
        _count++;
    }
    void pop() {
        _count--;
    }

    T* begin() const { return _begin; }
    T* end() const { return begin() + _count; }
    count_t size() const { return _count; }
    count_t defaultReserved() const { return 10; }

};

template<class T, template<typename> class SharedPtr = std::shared_ptr>
class Stack
{
protected:
    Q_STATIC_ASSERT_X(!QTypeInfo<T>::isComplex, "Using complex objects restricts by code style use pointers instead");
    SharedPtr<StackData<T>> d;
public:
    typedef T* iterator;
    typedef const T* const_iterator;
    Stack(count_t count) : Stack() { resize(count); }
    Stack() : d(new StackData<T>()) {}
    Stack(std::initializer_list<T> args) : d(new StackData<T>(args)) {}
    Stack(const Stack& other) : d(other.d) {}

    void push(const T& value) {
        detachCopy();
        d->push(value);
    }

    void append(const T& value) {
        push(value);
    }

    void pop() {
        detachCopy();
        d->pop();
    }

    bool isEmpty() const { return size() == 0; }

    const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    T& first() { Q_ASSERT(!isEmpty()); detachCopy(); return *begin(); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(end() - 1); }
    T& last() { Q_ASSERT(!isEmpty()); detachCopy(); return *(end() - 1); }

    const T& at(count_t index) const { Q_ASSERT(index >= 0 &&  index < size()); return *(begin() + index); }
    T& at(count_t index) {
        Q_ASSERT(index >= 0 &&  index < size());
        detachCopy();
        return *(begin() + index);
    }

//    void reserve(count_t count) {
//        if(count < size())
//    }

    void resize(count_t count) {
        detachCopy(); d->resize(count);
    }
    void clear() {
        detachClear(); d->clear();
    }

    T* data() { detachCopy(); return d->begin(); }
    const T* data() const { detachCopy(); return d->begin(); }

    count_t size() const { return d->size(); }

    iterator begin() {
        detachCopy(); return d->begin();
    }
    iterator end() {
        detachCopy(); return d->end();
    }

    const_iterator begin() const {
        return d->begin();
    }
    const_iterator end() const {
        return d->end();
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    T& operator[](qint32 index) { detachCopy(); return this->at(index); }
    const T& operator[](qint32 index) const { return this->at(index); }
protected:
    void detachClear() {
        if(d.use_count() > 1) d.reset(new StackData<T>(d->size()));
        else d->clear();
    }
    void detachCopy() {
        Q_ASSERT_X(!(d.use_count() > 1), "detachCopy", "restricted behavior");
    }
};

template<class T>
class StackPointers : public Stack<T*>
{
    typedef Stack<T*> Super;
public:
    StackPointers() : Super() {}
    StackPointers(count_t count) : StackPointers() {
        resize(count);
        for(T*& v : *this) {
            v = new T();
        }
    }
    ~StackPointers() {
        if(d.use_count() == 1) {
            for(T* v : *this)
                delete v;
        }
    }

    void clear() {
        if(d.use_count() == 1) {
            for(T* v : *this)
                delete v;
        }
        Super::clear();
    }

    template<typename ... Args> void resizeAndAllocate(qint32 size, Args ... args) {
        Q_ASSERT(d.use_count() == 1);
        qint32 old = this->size();
        if(size < old) {
            for(T* ptr : adapters::range(begin() + size, end()))
                delete ptr;
            resize(size);
        }
        else if(size > old){
            resize(size);
            for(T*& ptr : adapters::range(begin() + old, end()))
                ptr = new T(args...);
        }
    }
};

#endif // STACK_H
