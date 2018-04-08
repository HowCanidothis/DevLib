#ifndef ARRAY_H
#define ARRAY_H

#include <QTypeInfo>
#include <memory>
#include "shared_decl.h"

template<class T>
class MiddleAlgoData {
public:
    MiddleAlgoData(qint32 reserve) {
        alloc(reserve, 0);
    }
    MiddleAlgoData() : _begin(nullptr), _end(nullptr), _first(nullptr), _count(0) {}
    MiddleAlgoData(std::initializer_list<T> args) : _begin(nullptr), _end(nullptr), _first(nullptr), _count(args.size()) {
        if(args.size()) {
            alloc(args.size(), 0);
            memMove(_first, args.begin(), args.size());
        }
    }
    ~MiddleAlgoData() { free(begin());  }

    void clear() { _count = 0; _first = _begin + (reserved() >> 1); }
    void alloc(count_t count, count_t offset) {
        _begin = (T*)malloc(count * sizeof(T));
        _end = _begin + count;
        _first = _begin + offset;
    }
    void set(T* ptr, const T& value) { *ptr = value; } //convenient func
    void free(T* mem) { ::free(mem); }
    void memMove(T* dst, const T* src, count_t count) {
        if(dst != src) ::memmove(dst, src, count * sizeof(T)); //'if' cans defend from unnecessary implicit calls from append(const T* v, count_t count)
    }
    void realloc(count_t count, count_t offset) {
        count = count ? count : defaultReserved();
        if(!size()) {
            free(begin());
            alloc(count, offset);
        }
        else {
            T* old_begin = begin();
            T* old_first = first();
            alloc(count, offset);
            alignMem(old_first, offset);
            free(old_begin);
        }
    }
    void dblRealloc() {
        count_t reserv = reserved();
        if(reserv > 1000000)
            realloc(reserv += 5000000, 250000);
        else
            realloc(reserv << 1, reserv >> 1);
    }
    void resize(count_t size) { if(size > (reserved() - (first() - begin()))) realloc(size, 0); _count = size; }
    void alignMem(T* oldf, count_t offset) { //align to center
        _first = _begin + offset;
        memMove(first(), oldf, size());
    }

    void ensureLeft(){ if(first() == begin()) dblRealloc(); }
    void ensureRight(){ if(last() == end())  dblRealloc(); }

    count_t reserved() const { return _end - _begin; }
    count_t size() const { return _count; }
    void truncate(count_t new_size) { _count = new_size; }
    void insert(T* before, const T& v) {
        if(before == last()) pushBack(v);
        else if(before == first()) pushFront(v);
        else {
            ptrdiff_t to_left = before - first();
            ptrdiff_t to_right = last() - before;
            if(to_right < to_left) {
                ensureRight();
                memMove(first() + to_left + 1, first() + to_left, to_right);
            }
            else {
                ensureLeft();
                memMove(first() - 1, first(), to_left);
                --_first;
            }
            _count++;
            set(_first + to_left, v);
        }
    }
    void append(const T* v, count_t count) {
        if(!size()) { //empty
            if(reserved() < count) realloc(count, 0);
            else _first = begin();
        }
        else if(end() - last() < count) { //ensureRight return false
            realloc(reserved() + count, 0);
        }
        memMove(_first + size(), v, count);
        _count += count;
    }
    void debug(const char *msg="") { qDebug() << msg; for(qint32 i : adapters::range(begin(), end())) qDebug() << i; }
    void pushBack(const T& v) { ensureRight(); set(last(), v); _count++; }
    void pushFront(const T& v) { ensureLeft(); set(--_first, v); _count++; }
    void remove(T* target) {
        if(target == last() - 1) popBack();
        else if(target == first()) popFront();
        else {
            ptrdiff_t to_left = target - first();
            ptrdiff_t to_right = last() - target;
            if(to_right < to_left)
                memMove(target, target + 1, to_right);
            else {
                memMove(first() + 1, first(), to_left);
                ++_first;
            }
            _count--;
        }
    }
    void popFront(){ _first++; _count--; }
    void popBack(){ _count--; }
    bool isEmpty() const { return !_count; }

    T* begin() const { return _begin; }
    T* first() const { return _first; }
    T* last() const { return _first + _count; }
    T* end() const { return _end; }

    static count_t defaultReserved() { return 10; }
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

    ArrayCommon() : d(new MiddleAlgoData<T>()) {}
    ArrayCommon(std::initializer_list<T> args) : d(new MiddleAlgoData<T>(args)) {}
    ArrayCommon(const ArrayCommon& other) : d(other.d) {}

    void sort() { detachCopy(); std::sort(begin(), end()); }

    template<typename Value, typename Predicate> iterator findSortedByPredicate(const Value& value, Predicate predicate) {
        return std::lower_bound(begin(), end(), value, predicate);
    }

    template<typename Value, typename Predicate> const_iterator findSortedByPredicate(const Value& value, Predicate predicate) const {
        return std::lower_bound(begin(), end(), value, predicate);
    }

    bool containsSorted(const T& value) const {
        return this->findSorted(value) != this->end();
    }

    iterator findSorted(const T& value) {
        return std::lower_bound(begin(), end(), value);
    }
    const_iterator findSorted(const T& value) const {
        return std::lower_bound(begin(), end(), value);
    }
    void insertSortedUnique(const T& value) {
        auto it = std::lower_bound(begin(), end(), value);
        if(it == end() || *it != value)
            insert(it, value);
    }
    void insertSortedMulty(const T& value) {
        auto it = std::lower_bound(begin(), end(), value);
        insert(it, value);
    }
    void insert(iterator before, const T& value) { detachCopy(); d->insert(before, value); }
    void append(const T* src, count_t count) {
        if(count) {
            detachCopy();
            d->append(src, count);
        }
    }
    void append(const T& value) { detachCopy(); d->pushBack(value); }
    void remove(const T& value) {
        auto rm = std::find_if(begin(), end(), [value](const T& v) { return value == v; });
        if(rm != end()){ detachCopy(); d->remove(rm); }
    }
    template<typename Predicate> void removeByPredicate(Predicate predicate) {
        auto e = std::remove_if(begin(), end(), predicate);
        if(e != end()){ detachCopy(); d->truncate(e - begin()); }
    }
    void removeByIndex(count_t index) { detachCopy(); Q_ASSERT(index >= 0 && index < size()); d->remove(begin() + index); }
    void remove(iterator it) { detachCopy(); Q_ASSERT(it != this->end()); removeByIndex(std::distance(begin(), it)); }
    void clear() { detachClear(); }
    void resize(count_t size) { detachCopy(); d->resize(size); }
    void reserve(count_t count) {
        Q_ASSERT(count > 0);
        if(count < size())
            resize(count);
        else
            d->realloc(count, 0);
    }

    bool isEmpty() const { return !size(); }
    count_t size() const { return d->size(); }
    count_t reserved() const { return d->reserved(); }

    T& at(count_t index) { detachCopy(); return *(begin() + index); }
    const T& at(count_t index) const { return *(begin() + index); }
    T& last() { detachCopy(); Q_ASSERT(!isEmpty()); return *(end() - 1); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(end() - 1);  }
    T& first() { detachCopy(); Q_ASSERT(!isEmpty()); return *begin(); }
    const T& first() const { Q_ASSERT(!isEmpty()); return *begin();  }

    iterator begin() { detachCopy(); return d->first(); }
    iterator end() { detachCopy(); return d->last(); }
    const_iterator begin() const { return d->first(); }
    const_iterator end() const { return d->last(); }
    const_iterator cbegin() const { return d->first(); }
    const_iterator cend() const { return d->last(); }

    T& operator[](qint32 index) { detachCopy(); return this->at(index); }
    const T& operator[](qint32 index) const { return this->at(index); }
protected:
    SharedPtr<MiddleAlgoData<T> > d;

protected:
    void detachClear() {
        if(d.use_count() > 1) d.reset(new MiddleAlgoData<T>(d->size()));
        else d->clear();
    }
    void detachCopy() {
        Q_ASSERT_X(!(d.use_count() > 1), "detachCopy", "restricted behavior");
    }
};

template<class T>
class Array : public ArrayCommon<T>
{
    typedef ArrayCommon<T> Super;
public:
    Array() : Super() {}
    Array(std::initializer_list<T> args) : Super(args) {}
    Array(const ArrayCommon<T>& other) : Super(other) {}
};

template<class T>
class ArrayPointers : public ArrayCommon<T*>
{
    typedef ArrayCommon<T*> Super;
public:
    ArrayPointers() : Super() {}
    ~ArrayPointers() {
        for(T* v : *this)
            delete v;
    }

    void clear() {
        for(T* v : *this)
            delete v;
        Super::clear();
    }

    template<typename ... Args> void resizeAndAllocate(qint32 size, Args ... args) {
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


#endif // VECTOR_H
