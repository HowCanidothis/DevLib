#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include "property.h"

template<class T>
class LocalProperty
{
protected:
    Property::FOnChange m_onChange;
    T m_value;

public:
    LocalProperty()
    {}
    LocalProperty(const T& value)
        : m_value(value)
    {}

    void Invoke()
    {
        Q_ASSERT(m_onChange);
        m_onChange();
    }

    void Subscribe(const Property::FOnChange& onChange)
    {
        if(m_onChange) {
            auto oldOnChange = m_onChange;
            m_onChange = [onChange, oldOnChange]{
                oldOnChange();
                onChange();
            };
        } else {
            m_onChange = onChange;
        }
    }

    void SetValue(const T& value)
    {
        if(value != m_value) {
            m_value = value;
            m_onChange();
        }
    }
    const T& GetValue() const { return m_value; }

    LocalProperty& operator=(const T& value) { SetValue(value); return *this; }
    operator const T&() const { return m_value; }
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
            this->m_onChange();
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
            this->m_onChange();
        }
    }

    void Remove(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.erase(find);
            this->m_onChange();
        }
    }

    typename QSet<T>::const_iterator begin() const { return this->m_value.begin(); }
    typename QSet<T>::const_iterator end() const { return this->m_value.end(); }
};

#endif // LOCALPROPERTY_H
