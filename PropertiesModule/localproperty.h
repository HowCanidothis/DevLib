#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include "property.h"

template<class T>
class LocalProperty
{
protected:
    T m_value;

public:
    LocalProperty()
    {}
    LocalProperty(const T& value)
        : m_value(value)
    {}

    void Invoke()
    {
        Q_ASSERT(!OnChange.IsEmpty() || m_subscribes != nullptr);
        OnChange.Invoke();
        m_subscribes();
    }

    void Subscribe(const FAction& subscribe)
    {
        if(m_subscribes == nullptr) {
            m_subscribes = subscribe;
        } else {
            auto oldHandle = m_subscribes;
            m_subscribes = [subscribe, oldHandle]{
                oldHandle();
                subscribe();
            };
        }
    }

    void SetValue(const T& value)
    {
        if(value != m_value) {
            m_value = value;
            Invoke();
        }
    }

    const T& Native() const { return m_value; }
    Dispatcher& GetDispatcher() { return OnChange; }

    bool operator!() const { return m_value == false; }
    LocalProperty& operator=(const T& value) { SetValue(value); return *this; }
    operator const T&() const { return m_value; }

    Dispatcher OnChange;

private:
    FAction m_subscribes;
};

template<class T>
class LocalPropertyPtr : public LocalProperty<T*>
{
    using Super = LocalProperty<T*>;
public:
    LocalPropertyPtr(T* initial)
        : Super(initial)
    {}

    bool operator!=(const T* another) const { return Super::m_value != another; }
    bool operator==(const T* another) const { return Super::m_value == another; }
    LocalPropertyPtr& operator=(T* value) { Super::SetValue(value); return *this; }
    operator const T*() const { return Super::m_value; }
    const T* operator->() const { return Super::m_value; }
    T* operator->() { return Super::m_value; }
};

template<class T>
class LocalPropertySet : public LocalProperty<QSet<T>>
{
    typedef LocalProperty<QSet<T>> Super;
public:
    LocalPropertySet()
    {}
    LocalPropertySet(const QSet<T>& value)
        : Super(value)
    {}

    bool IsEmpty() const { return this->m_value.isEmpty(); }
    qint32 Size() const { return this->m_value.size(); }
    bool IsContains(const T& value) const { return this->m_value.contains(value); }

    void Clear()
    {
        if(!this->m_value.isEmpty()) {
            this->m_value.clear();
            this->Invoke();
        }
    }

    void SilentClear()
    {
        this->m_value.clear();
    }

    void SilentInsert(const T& value)
    {
        this->m_value.insert(value);
    }

    void SilentRemove(const T& value)
    {
        this->m_value.remove(value);
    }

    void Insert(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.insert(value);
            this->Invoke();
        }
    }

    void Remove(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.erase(find);
            this->Invoke();
        }
    }

    typename QSet<T>::const_iterator begin() const { return this->m_value.begin(); }
    typename QSet<T>::const_iterator end() const { return this->m_value.end(); }
};

#endif // LOCALPROPERTY_H
