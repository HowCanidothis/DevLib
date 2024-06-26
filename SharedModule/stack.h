#ifndef STACK_H
#define STACK_H

#include <memory>
#include <type_traits>

#include "namingconvention.h"
#include "SharedModule/declarations.h"

template<class T>
class StackData
{
    T* _begin;
    count_t _reserved;
    count_t _count;

public:
    StackData(count_t reserve)
        : _begin(nullptr)
        , _reserved(reserve)
        , _count(0)
    {
        Realloc();
    }
    StackData()
        : _begin(nullptr)
        , _reserved(0)
        , _count(0)
    {}
    StackData(std::initializer_list<T> args)
        : _begin(nullptr)
        , _reserved(args.size())
        , _count(static_cast<count_t>(args.size()))
    {
        if(args.size()) {
            Realloc();
            memcpy(Begin(), args.begin(), args.size() * sizeof(T));
        }
    }
    ~StackData()
    {
        ::free(_begin);
    }

    void Resize(count_t count)
    {
        if(count > _reserved) {
            _reserved = count;
            Realloc();
        }
        this->_count = count;
    }
    void Clear()
    {
        _count = 0;
    }
    void DblRealloc()
    {
        if(_reserved > 1000000) {
            _reserved += 5000000;
        } else {
            _reserved <<= 1;
        }
        Realloc();
    }
    void Realloc()
    {
        _reserved = _reserved ? _reserved : DefaultReserved();
        _begin = (T*)::realloc(_begin, _reserved * sizeof(T));

    }
    void Push(const T& value)
    {
        if(_reserved == _count) {
            DblRealloc();
        }
        Q_ASSERT(_reserved != _count);
        *End() = value;
        _count++;
    }
    void Pop()
    {
        _count--;
    }

    T* Begin() const { return _begin; }
    T* End() const { return Begin() + _count; }
    count_t Size() const { return _count; }
    count_t DefaultReserved() const { return 10; }

};

template<class T, template<typename> class SharedPtr = std::shared_ptr>
class Stack
{
protected:
#ifndef NO_QT
    static_assert(!QTypeInfo<T>::isComplex, "Using complex objects is restricted by code style use pointers instead");
#else
    static_assert(std::is_pod<T>::value, "Using complex objects is restricted by code style use pointers instead");
#endif
    SharedPtr<StackData<T>> d;
public:
    typedef T* iterator;
    typedef const T* const_iterator;
    Stack(count_t count)
        : Stack()
    {
        Resize(count);
    }
    Stack()
        : d(new StackData<T>())
    {}
    Stack(std::initializer_list<T> args)
        : d(new StackData<T>(args))
    {}
    Stack(const Stack& other)
        : d(other.d)
    {}

    void Push(const T& value)
    {
        detachCopy();
        d->Push(value);
    }

    void Append(const T& value)
    {
        Push(value);
    }

    void Pop()
    {
        detachCopy();
        d->Pop();
    }

    bool IsEmpty() const { return Size() == 0; }

    const T& First() const
    {
        Q_ASSERT(!IsEmpty());
        return *Begin();
    }
    T& First()
    {
        Q_ASSERT(!IsEmpty());
        detachCopy();
        return *Begin();
    }
    const T& Last() const
    {
        Q_ASSERT(!IsEmpty());
        return *(End() - 1);
    }
    T& Last()
    {
        Q_ASSERT(!IsEmpty());
        detachCopy();
        return *(End() - 1);
    }

    const T& At(count_t index) const
    {
        Q_ASSERT(index >= 0 &&  index < Size());
        return *(Begin() + index);
    }
    T& At(count_t index)
    {
        Q_ASSERT(index >= 0 &&  index < Size());
        detachCopy();
        return *(Begin() + index);
    }

    void Swap(Stack& other)
    {
        std::swap(other.d, d);
    }

    void Resize(count_t count)
    {
        detachCopy();
        d->Resize(count);
    }
    void Clear()
    {
        detachClear();
        d->Clear();
    }

    T* data()
    {
        detachCopy();
        return d->Begin();
    }
    const T* data() const
    {
        detachCopy();
        return d->Begin();
    }

    iterator Begin()
    {
        detachCopy();
        return d->Begin();
    }
    iterator End()
    {
        detachCopy();
        return d->End();
    }

    count_t Size() const { return d->Size(); }

    iterator begin() { return Begin(); }
    iterator end() { return End(); }
    const_iterator begin() const { return Begin(); }
    const_iterator end() const { return End(); }

    const_iterator Begin() const { return d->Begin(); }
    const_iterator End() const { return d->End(); }
    const_iterator CBegin() const { return Begin(); }
    const_iterator CEnd() const { return End(); }

    T& operator[](count_t index) { detachCopy(); return this->At(index); }
    const T& operator[](count_t index) const { return this->At(index); }
protected:
    void detachClear() {
        if(d.use_count() > 1) d.reset(new StackData<T>(d->Size()));
        else d->Clear();
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
        this->Resize(count);
        for(T*& v : *this) {
            v = new T();
        }
    }
    ~StackPointers() {
        if(this->d.use_count() == 1) {
            for(T* v : *this)
                delete v;
        }
    }

    void Swap(StackPointers& other)
    {
        std::swap(other.d, this->d);
    }

    void Clear() {
        if(this->d.use_count() == 1) {
            for(T* v : *this)
                delete v;
        }
        Super::Clear();
    }

    template<typename ... Args> void ResizeAndAllocate(count_t size, Args ... args) {
        Q_ASSERT(this->d.use_count() == 1);
        count_t old = this->Size();
        if(size < old) {
            for(T* ptr : adapters::Range<typename Super::const_iterator>(this->Begin() + size, this->End()))
                delete ptr;
            this->Resize(size);
        }
        else if(size > old){
            this->Resize(size);
            for(T*& ptr : adapters::Range<typename Super::const_iterator>(this->Begin() + old, this->End()))
                ptr = new T(args...);
        }
    }
};

#endif // STACK_H
