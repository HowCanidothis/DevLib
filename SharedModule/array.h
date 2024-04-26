#ifndef ARRAY_H
#define ARRAY_H

#include <memory>
#include <type_traits>

#include "namingconvention.h"

template<class T>
class MiddleAlgoData {
public:
    MiddleAlgoData(count_t reserve)
        : _count(0)
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
        , _count(static_cast<count_t>(args.size()))
    {
        if(args.size()) {
            Alloc(static_cast<count_t>(args.size()), 0);
            MemMove(_first, args.begin(), static_cast<count_t>(args.size()));
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
            ::memmove(dst, src, count * sizeof(T)); //'if' can defend from unnecessary implicit calls from append(const T* v, count_t count)
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
#ifndef NO_QT
    static_assert(!QTypeInfo<T>::isComplex, "Using complex objects is restricted by code style use pointers instead");
#else
    static_assert(std::is_pod<T>::value, "Using complex objects is restricted by code style use pointers instead");
#endif
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
    ArrayCommon(ArrayCommon&& other)
        : _d(other._d)
    {
        other._d.reset(new MiddleAlgoData<T>());
    }
    virtual ~ArrayCommon()
    {

    }

    void Sort()
    {
        detachCopy();
        std::sort(Begin(), End());
    }

    template<typename Value, typename FComparator>
    iterator FindSortedByPredicate(const Value& value, FComparator predicate)
    {
        return std::lower_bound(Begin(), End(), value, predicate);
    }

    template<typename Value, typename FComparator>
    const_iterator FindSortedByPredicate(const Value& value, FComparator predicate) const
    {
        return std::lower_bound(Begin(), End(), value, predicate);
    }

    template<typename FComparator>
    bool ContainsSorted(const T& value, const FComparator& comparator) const
    {
        return this->FindSorted(value, comparator) != this->End();
    }

    bool ContainsSorted(const T& value) const
    {
        return this->FindSorted(value) != this->End();
    }

    iterator FindSorted(const T& value)
    {
        auto find = std::lower_bound(Begin(), End(), value);
        if(find != End()) {
            find = (*find == value) ? find : End();
        }
        return find;
    }

    template<typename FComparator>
    iterator FindSorted(const T& value, const FComparator& comparator)
    {
        return FindSortedByPredicate(value, comparator);
    }

    template<typename FComparator>
    const_iterator FindSorted(const T& value, const FComparator& comparator) const
    {
        return FindSortedByPredicate(value, comparator);
    }

    const_iterator FindSorted(const T& value) const
    {
        auto find = std::lower_bound(Begin(), End(), value);
        if(find != End()) {
            find = (*find == value) ? find : End();
        }
        return find;
    }

    count_t IndexOf(const T& value) const
    {
        auto foundIt = std::find(begin(), end(), value);
        if(foundIt == end()) {
            return -1;
        }
        return std::distance(begin(), foundIt);
    }

    void Merge(qint32 insertIndex, const ArrayCommon& other) {
        Resize(other.Size() + Size());
        auto moveRangeIt = End() - other.Size() - 1;
        auto moveRangeEnd = Begin() + insertIndex;
        auto moveRangeNewIt = End() - 1;
        for(;moveRangeIt >= moveRangeEnd; moveRangeIt--, moveRangeNewIt--) {
            *moveRangeNewIt = *moveRangeIt;
        }
        auto startInsertIt = other.Begin();
        auto startInsertNewIt = Begin() + insertIndex;
        for(;startInsertIt < other.end(); startInsertNewIt++, startInsertIt++) {
            *startInsertNewIt = *startInsertIt;
        }
    }
    template<typename FComparator>
    void InsertSortedUnique(const T &value, const FComparator& comparator)
    {
        auto it = std::lower_bound(Begin(), End(), value, comparator);
        if(it == End() || *it != value) {
            Insert(it, value);
        }
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
    void RemoveSorted(const T& value)
    {
        auto foundIt = std::lower_bound(Begin(), End(), value);
        if(foundIt != End()) {
            _d->Remove(foundIt);
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
    void Swap(ArrayCommon& other)
    {
        std::swap(other._d, _d);
    }
    void clear() { Clear(); }
    virtual void Clear()
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

    bool isEmpty() const { return IsEmpty(); }
    bool IsEmpty() const
    {
        return !Size();
    }
    count_t size() const { return Size(); }
    count_t Size() const
    {
        return _d->Size();
    }
    count_t Reserved() const
    {
        return _d->Reserved();
    }

    const T& at(count_t index) const { return At(index); }
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
    bool HasIndex(qint32 index) const
    {
        return index >= 0 && index < Size();
    }

    const_iterator Begin() const { return _d->First(); }
    const_iterator End() const { return _d->Last(); }
    const_iterator CBegin() const { return _d->First(); }
    const_iterator CEnd() const { return _d->Last(); }
    iterator begin() { return Begin(); }
    iterator end() { return End(); }
    const_iterator begin() const { return Begin(); }
    const_iterator end() const { return End(); }

    T& operator[](count_t index)
    {
        detachCopy();
        return this->At(index);
    }
    const T& operator[](count_t index) const
    {
        return this->At(index);
    }

    ArrayCommon& operator=(const ArrayCommon& another) {
        if(_d.use_count() == 1) {
            Clear();
        }
        _d = another._d;
        return *this;
    }

protected:
    SharedPtr<MiddleAlgoData<T> > _d;
    bool isShared() const { return _d.use_count() > 1; }

protected:
    void detachClear() {
        if(_d.use_count() > 1) {
            _d.reset(new MiddleAlgoData<T>(_d->Size()));
        } else {
            _d->Clear();
        }
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
    Array(ArrayCommon<T>&& other)
        : Super(other)
    {}

    const T* data() const { return this->begin(); }
    T* data() { return this->begin(); }
    count_t size() const { return this->Size(); }
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
        if(!Super::isShared()) {
            for(T* v : *this) {
                delete v;
            }
        }
    }

    void Clear() override
    {
        if(!Super::isShared()) {
            for(T* v : *this) {
                delete v;
            }
        }
        Super::Clear();
    }

    template<typename ... Args> void resizeAndAllocate(count_t size, Args ... args)
    {
        count_t old = this->Size();
        if(size < old) {
            for(T* ptr : adapters::range(this->Begin() + size, this->End())) {
                delete ptr;
            }
            this->Resize(size);
        }
        else if(size > old) {
            this->Resize(size);
            for(T*& ptr : adapters::range(this->Begin() + old, this->End())) {
                ptr = new T(args...);
            }
        }
    }
};


#endif // VECTOR_H
