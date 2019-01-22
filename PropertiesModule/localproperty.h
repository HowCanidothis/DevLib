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
public:
    LocalPropertySet()
    {}
    LocalPropertySet(const QSet<T>& value)
        : m_value(value)
    {}

    bool IsEmpty() const { return m_value.isEmpty(); }
    qint32 Size() const { return m_value.size(); }
    bool IsContains(const T& value) const { return m_value.contains(value); }

    void Clear()
    {
        if(!m_value.isEmpty()) {
            m_value.clear();
            m_onChange();
        }
    }

    void SilentClear()
    {
        m_value.clear();
    }

    void SilentInsert(const T& value)
    {
        m_value.insert(value);
    }

    void SilentRemove(const T& value)
    {
        m_value.remove(value);
    }

    void Insert(const T& value)
    {
        auto find = m_value.find(value);
        if(find != m_value.end()) {
            m_value.insert(value);
            m_onChange();
        }
    }

    void Remove(const T& value)
    {
        auto find = m_value.find(value);
        if(find != m_value.end()) {
            m_value.erase(find);
            m_onChange();
        }
    }

    typename QSet<T>::const_iterator begin() const { return m_value.begin(); }
    typename QSet<T>::const_iterator end() const { return m_value.end(); }
};

#endif // LOCALPROPERTY_H
