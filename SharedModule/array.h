#ifndef ARRAY_H
#define ARRAY_H

#include <QTypeInfo>
#include <memory>
#include "decl.h"

template<class T>
class MiddleAlgoData {
public:
    MiddleAlgoData(qint32 reserve)
    {
        Alloc(reserve, 0);
    }
    MiddleAlgoData()
        : _begin(nullptr)
        , _end(nullptr)
        , _first(nullptr)
        , _count(0)
    {}
    MiddleAlgoData(std::initializer_list<T> args)
        : _begin(nullptr)
        , _end(nullptr)
        , _first(nullptr)
        , _count(args.size())
    {
        if(args.size()) {
            Alloc(args.size(), 0);
            MemMove(_first, args.begin(), args.size());
        }
    }
    ~MiddleAlgoData()
    {
        Free(Begin());
    }

    void Clear()
    {
        _count = 0;
        _first = _begin + (Reserved() >> 1);
    }
    void Alloc(count_t count, count_t offset)
    {
        _begin = (T*)::malloc(count * sizeof(T));
        _end = _begin + count;
        _first = _begin + offset;
    }
    //convenient func
    void Set(T* ptr, const T& value)
    {
        *ptr = value;
    }
    void Free(T* mem)
    {
        ::free(mem);
    }
    void MemMove(T* dst, const T* src, count_t count)
    {
        if(dst != src) {
            ::memmove(dst, src, count * sizeof(T)); //'if' cans defend from unnecessary implicit calls from append(const T* v, count_t count)
        }
    }
    void Realloc(count_t count, count_t offset)
    {
        count = count ? count : DefaultReserved();
        if(!Size()) {
            Free(Begin());
            Alloc(count, offset);
        }
        else {
            T* old_begin = Begin();
            T* old_first = First();
            Alloc(count, offset);
            AlignMem(old_first, offset);
            Free(old_begin);
        }
    }
    void DblRealloc()
    {
        count_t reserv = Reserved();
        if(reserv > 1000000) {
            Realloc(reserv += 5000000, 250000);
        }
        else if(reserv) {
            Realloc(reserv << 1, reserv >> 1);
        }
        else {
            Alloc(DefaultReserved(),DefaultReserved() >> 1);
        }
    }
    void Resize(count_t size)
    {
        if(size > (Reserved() - (First() - Begin()))) {
            Realloc(size, 0);
        }
        _count = size;
    }
    //align to center
    void AlignMem(T* oldf, count_t offset)
    {
        _first = _begin + offset;
        MemMove(First(), oldf, Size());
    }

    void EnsureLeft()
    {
        if(First() == Begin()) {
            DblRealloc();
        }
    }
    void EnsureRight()
    {
        if(Last() == End()) {
            DblRealloc();
        }
    }

    count_t Reserved() const { return _end - _begin; }
    count_t Size() const { return _count; }
    void Truncate(count_t newSize)
    {
        _count = newSize;
    }
    void Insert(T* before, const T& v)
    {
        if(before == Last()) {
            PushBack(v);
        }
        else if(before == First()) {
            PushFront(v);
        }
        else {
            ptrdiff_t toLeft = before - First();
            ptrdiff_t toRight = Last() - before;
            if(toRight < toLeft) {
                EnsureRight();
                MemMove(First() + toLeft + 1, First() + toLeft, toRight);
            }
            else {
                EnsureLeft();
                MemMove(First() - 1, First(), toLeft);
                --_first;
            }
            _count++;
            Set(_first + toLeft, v);
        }
    }
    void Append(const T* v, count_t count)
    {
        if(!Size()) { //empty
            if(Reserved() < count) {
                Realloc(count, 0);
            }
            else {
                _first = Begin();
            }
        }
        else if(End() - Last() < count) { //ensureRight return false
            Realloc(Reserved() + count, 0);
        }
        MemMove(_first + Size(), v, count);
        _count += count;
    }
    void Debug(const char* msg="")
    {
        qDebug() << msg;
        for(qint32 i : adapters::range(Begin(), End())) {
            qDebug() << i;
        }
    }
    void PushBack(const T& v)
    {
        EnsureRight(); Set(Last(), v);
        _count++;
    }
    void PushFront(const T& v)
    {
        EnsureLeft();
        Set(--_first, v);
        _count++;
    }
    void Remove(T* target)
    {
        if(target == Last() - 1) {
            PopBack();
        }
        else if(target == First()) {
            PopFront();
        }
        else {
            ptrdiff_t toLeft = target - First();
            ptrdiff_t toRight = Last() - target;
            if(toRight < toLeft)
                MemMove(target, target + 1, toRight);
            else {
                MemMove(First() + 1, First(), toLeft);
                ++_first;
            }
            _count--;
        }
    }
    void PopFront()
    {
        _first++;
        _count--;
    }
    void PopBack()
    {
        _count--;
    }
    bool IsEmpty() const { return !_count; }

    T* Begin() const { return _begin; }
    T* First() const { return _first; }
    T* Last() const { return _first + _count; }
    T* End() const { return _end; }

    static count_t DefaultReserved() { return 10; }

private:
    T* _begin;
    T* _end;
    T* _first;
    count_t _count;
};


template<class T,template<typename> class SharedPtr = std::shared_ptr>
class ArrayCommon
{
    Q_STATIC_ASSERT_X(!QTypeInfo<T>::isComplex, "Using complex objects restricts by code style use pointers instead");
public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;

    ArrayCommon()
        : _d(new MiddleAlgoData<T>())
    {}
    ArrayCommon(std::initializer_list<T> args)
        : _d(new MiddleAlgoData<T>(args))
    {}
    ArrayCommon(const ArrayCommon& other)
        : _d(other._d)
    {}

    void Sort()
    {
        detachCopy();
        std::sort(Begin(), End());
    }

    template<typename Value, typename Predicate>
    iterator FindSortedByPredicate(const Value& value, Predicate predicate)
    {
        return std::lower_bound(Begin(), End(), value, predicate);
    }

    template<typename Value, typename Predicate>
    const_iterator FindSortedByPredicate(const Value& value, Predicate predicate) const
    {
        return std::lower_bound(Begin(), End(), value, predicate);
    }

    bool ContainsSorted(const T& value) const
    {
        return this->FindSorted(value) != this->End();
    }

    iterator FindSorted(const T& value)
    {
        return std::lower_bound(Begin(), End(), value);
    }

    const_iterator FindSorted(const T& value) const
    {
        return std::lower_bound(Begin(), End(), value);
    }

    void InsertSortedUnique(const T& value)
    {
        auto it = std::lower_bound(Begin(), End(), value);
        if(it == End() || *it != value) {
            Insert(it, value);
        }
    }
    void InsertSortedMulty(const T& value)
    {
        auto it = std::lower_bound(Begin(), End(), value);
        Insert(it, value);
    }
    void Insert(iterator before, const T& value)
    {
        detachCopy();
        _d->Insert(before, value);
    }
    void Prepend(const T& value)
    {
        detachCopy();
        _d->PushFront(value);
    }
    void Append(const T* src, count_t count) {
        if(count) {
            detachCopy();
            _d->Append(src, count);
        }
    }
    void Append(const T& value)
    {
        detachCopy();
        _d->PushBack(value);
    }
    void Remove(const T& value)
    {
        auto rm = std::find_if(Begin(), End(), [value](const T& v) { return value == v; });
        if(rm != End()) {
            detachCopy();
            _d->Remove(rm);
        }
    }
    template<typename Predicate>
    void RemoveByPredicate(Predicate predicate)
    {
        auto e = std::remove_if(Begin(), End(), predicate);
        if(e != End()) {
            detachCopy();
            _d->Truncate(e - Begin());
        }
    }
    void RemoveByIndex(count_t index)
    {
        Q_ASSERT(index >= 0 && index < Size());
        detachCopy();
        _d->Remove(Begin() + index);
    }
    void Remove(iterator it)
    {
        Q_ASSERT(it != this->End());
        detachCopy();
        RemoveByIndex(std::distance(Begin(), it));
    }
    void Clear()
    {
        detachClear();
    }
    void Resize(count_t size)
    {
        detachCopy();
        _d->Resize(size);
    }
    void Reserve(count_t count)
    {
        Q_ASSERT(count > 0);
        if(count < Size()) {
            Resize(count);
        }
        else {
            _d->Realloc(count, 0);
        }
    }

    bool IsEmpty() const
    {
        return !Size();
    }
    count_t Size() const
    {
        return _d->Size();
    }
    count_t Reserved() const
    {
        return _d->Reserved();
    }

    T& At(count_t index)
    {
        detachCopy();
        return *(Begin() + index);
    }
    const T& At(count_t index) const
    {
        return *(Begin() + index);
    }
    T& Last()
    {
        Q_ASSERT(!IsEmpty());
        detachCopy();
        return *(End() - 1);
    }
    const T& Last() const
    {
        Q_ASSERT(!IsEmpty());
        return *(End() - 1);
    }
    T& First()
    {
        Q_ASSERT(!IsEmpty());
        detachCopy();
        return *Begin();
    }
    const T& First() const
    {
        Q_ASSERT(!IsEmpty());
        return *Begin();
    }

    iterator Begin()
    {
        detachCopy();
        return _d->First();
    }
    iterator End()
    {
        detachCopy();
        return _d->Last();
    }
    const_iterator Begin() const { return _d->First(); }
    const_iterator End() const { return _d->Last(); }
    const_iterator CBegin() const { return _d->First(); }
    const_iterator CEnd() const { return _d->Last(); }
    iterator begin() { return Begin(); }
    iterator end() { return End(); }
    const_iterator begin() const { return Begin(); }
    const_iterator end() const { return End(); }

    T& operator[](qint32 index)
    {
        detachCopy();
        return this->At(index);
    }
    const T& operator[](qint32 index) const
    {
        return this->At(index);
    }
protected:
    SharedPtr<MiddleAlgoData<T> > _d;

protected:
    void detachClear() {
        if(_d.use_count() > 1) _d.reset(new MiddleAlgoData<T>(_d->Size()));
        else _d->Clear();
    }
    void detachCopy() {
        Q_ASSERT_X(!(_d.use_count() > 1), "detachCopy", "restricted behavior");
    }
};

template<class T>
class Array : public ArrayCommon<T>
{
    typedef ArrayCommon<T> Super;
public:
    Array()
        : Super()
    {}
    Array(std::initializer_list<T> args)
        : Super(args)
    {}
    Array(const ArrayCommon<T>& other)
        : Super(other)
    {}
};

template<class T>
class ArrayPointers : public ArrayCommon<T*>
{
    typedef ArrayCommon<T*> Super;
public:
    ArrayPointers()
        : Super()
    {}
    ~ArrayPointers() {
        for(T* v : *this) {
            delete v;
        }
    }

    void Clear()
    {
        for(T* v : *this) {
            delete v;
        }
        Super::Clear();
    }

    template<typename ... Args> void resizeAndAllocate(qint32 size, Args ... args)
    {
        qint32 old = this->Size();
        if(size < old) {
            for(T* ptr : adapters::range(Begin() + size, End())) {
                delete ptr;
            }
            Resize(size);
        }
        else if(size > old) {
            Resize(size);
            for(T*& ptr : adapters::range(Begin() + old, End())) {
                ptr = new T(args...);
            }
        }
    }
};


#endif // VECTOR_H
