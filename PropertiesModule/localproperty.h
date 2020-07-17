#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include "property.h"

template<class T>
class LocalProperty
{
    typedef std::function<void ()> FSetter;
    typedef std::function<void (const FSetter&)> FSetterHandler;
protected:
    T m_value;
    FSetterHandler m_setterHandler;

public:
    LocalProperty()
        : m_setterHandler([](const FSetter& setter){
            setter();
        })
    {}
    LocalProperty(const T& value)
        : m_value(value)
        , m_setterHandler([](const FSetter& setter){
            setter();
        })
    {}

    void Invoke()
    {
        m_setterHandler([this]{
            OnChange.Invoke();
            if(m_subscribes != nullptr) {
                m_subscribes();
            }
        });
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

    void SetSetterHandler(const FSetterHandler& handler) {
        m_setterHandler = handler;
    }

    void SetValue(const T& value)
    {
        if(value != m_value) {
            m_setterHandler([value, this]{
                m_value = value;
                Invoke();
            });
        }
    }

    const T& Native() const { return m_value; }
    Dispatcher& GetDispatcher() { return OnChange; }

    bool operator!() const { return m_value == false; }
    bool operator!=(const T& value) const { return m_value != value; }
    bool operator==(const T& value) const { return m_value == value; }
    LocalProperty& operator=(const T& value) { SetValue(value); return *this; }
    operator const T&() const { return m_value; }

    Dispatcher OnChange;

private:
    FAction m_subscribes;
};

template<class T>
class LocalPropertyLimitedDecimal : public LocalProperty<T>
{
    using Super = LocalProperty<T>;
public:
    LocalPropertyLimitedDecimal(const T& value = 0, const T& min = std::numeric_limits<T>::lowest(), const T& max = std::numeric_limits<T>::max())
        : Super(::clamp(value, min, max))
        , m_min(min)
        , m_max(max)
    {}

    void SetMinMax(const T& min, const T& max)
    {
        m_min = min;
        m_max = max;
        SetValue(validateValue(Super::m_value));
    }

    void SetValue(const T& value)
    {
        auto validatedValue = validateValue(value);
        if(!qFuzzyCompare(double(validatedValue), double(Super::m_value))) {
            m_setterHandler([validatedValue, this]{
                Super::m_value = validatedValue;
                Invoke();
            });
        }
    }

    LocalPropertyLimitedDecimal& operator=(const T& value) { SetValue(value); return *this; }

    const T& GetMin() const { return m_min; }
    const T& GetMax() const { return m_max; }

private:
    T validateValue(const T& value)
    {
        return ::clamp(value, m_min, m_max);
    }

private:
    T m_min;
    T m_max;
};

using LocalPropertyInt = LocalPropertyLimitedDecimal<qint32>;
using LocalPropertyUInt = LocalPropertyLimitedDecimal<quint32>;
using LocalPropertyDouble = LocalPropertyLimitedDecimal<double>;
using LocalPropertyFloat = LocalPropertyLimitedDecimal<float>;


class LocalPropertyNamedUint : public LocalPropertyUInt
{
    using Super = LocalPropertyUInt;
public:
    LocalPropertyNamedUint()
        : Super(0, 0, 0)
    {}

    void Initialize(quint32 initial, const QStringList& names)
    {
        m_value = initial;
        m_names = names;
        SetMinMax(0, m_names.size());
    }
    const QStringList& GetNames() const { return m_names; }
    template<class T> T Cast() const { return (T)Native(); }

    LocalPropertyNamedUint& operator=(quint32 value) { SetValue(value); return *this; }

private:
    QStringList m_names;
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
