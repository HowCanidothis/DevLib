#ifndef PROPERTYPROMISE_H
#define PROPERTYPROMISE_H

#include "propertiessystem.h"
#include "property.h"

class _Export PropertyPromiseBase
{
public:
    PropertyPromiseBase(const Name& name, qint32 contextIndex);
    PropertyPromiseBase(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex);

    void Subscribe(const Property::FOnChange& onChange);
    void InstallObserver(Dispatcher::Observer observer, const FAction& action) { GetProperty()->InstallObserver(observer, action); }
    void RemoveObserver(Dispatcher::Observer observer) { GetProperty()->RemoveObserver(observer); }
    Property* GetProperty() const { return m_getter(); }
    bool IsValid() const { return m_isValid(); }
protected:
    std::function<Property* ()> m_getter;
    std::function<bool ()> m_isValid;
    std::function<void (const QVariant&)> m_setter;
    Name m_propertyName;
};

template<class T>
class PropertyPromise : public PropertyPromiseBase
{
protected:
    typedef typename T::value_type value_type;

public:
    PropertyPromise(const Name& name, qint32 contextIndex)
        : PropertyPromiseBase(name, contextIndex)
    {}
    PropertyPromise(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex)
        : PropertyPromiseBase(name, onChange, contextIndex)
    {}

    T* GetProperty() const { return reinterpret_cast<T*>(m_getter()); }
    const value_type& Native() const { return *GetProperty(); }

    const value_type& operator->() const { return *GetProperty(); }
    operator const value_type& () const { return *GetProperty(); }
    PropertyPromise& operator=(const value_type& value) { m_setter(PropertyValueExtractorPrivate<value_type>::ExtractVariant(value)); return *this; }
};

template<class T>
PropertyPromise<T> PropertiesSystem::GetProperty(const Name& path, qint32 type)
{
    return PropertyPromise<T>(path, type);
}

template<class T>
PropertyPromise<T> PropertiesSystem::GetProperty(const Name& path, const FOnChange& onChange, qint32 type)
{
    return PropertyPromise<T>(path, onChange, type);
}

typedef PropertyPromiseBase PropertyPtr;
typedef PropertyPromise<IntProperty> IntPropertyPtr;
typedef PropertyPromise<UIntProperty> UIntPropertyPtr;
typedef PropertyPromise<BoolProperty> BoolPropertyPtr;
typedef PropertyPromise<FloatProperty> FloatPropertyPtr;
typedef PropertyPromise<DoubleProperty> DoublePropertyPtr;
typedef PropertyPromise<ByteArrayProperty> ByteArrayPropertyPtr;
typedef PropertyPromise<StringProperty> StringPropertyPtr;
typedef PropertyPromise<UrlListProperty> UrlListPropertyPtr;
typedef PropertyPromise<NamedUIntProperty> NamedUIntPropertyPtr;
template<class T> using PointerPropertyPtr = PropertyPromise<PointerProperty<T>>;

#ifdef QT_GUI_LIB
typedef PropertyPromise<ColorProperty> ColorPropertyPtr;
#endif

#endif // PROPERTYPROMISE_H
