#ifndef PROPERTYPROMISE_H
#define PROPERTYPROMISE_H

#include "propertiessystem.h"
#include "property.h"

class _Export PropertyPromiseBase
{
public:
    PropertyPromiseBase(const Name& name, qint32 contextIndex);
    PropertyPromiseBase(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex);

    Property* GetProperty() const { return _getter(); }
    bool IsValid() const { return _isValid(); }
protected:
    std::function<Property* ()> _getter;
    std::function<bool ()> _isValid;
    std::function<void (const QVariant&)> _setter;
};

template<class T>
class PropertyPromise : public PropertyPromiseBase
{
    typedef typename T::value_type value_type;
public:
    PropertyPromise(const Name& name, qint32 contextIndex)
        : PropertyPromiseBase(name, contextIndex)
    {}
    PropertyPromise(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex)
        : PropertyPromiseBase(name, onChange, contextIndex)
    {}

    T* GetProperty() const { return reinterpret_cast<T*>(_getter()); }
    const value_type& Native() const { return *GetProperty(); }

    const value_type& operator->() const { return *GetProperty(); }
    operator const value_type& () const { return *GetProperty(); }
    PropertyPromise& operator=(const value_type& value) { _setter(value); return *this; }
};

template<typename T>
struct PropertyPromisePrivate
{
    static QVariant ExtractVariant(const T& value) { return value; }
};

template<typename T>
struct PropertyPromisePrivate<T*>
{
    static QVariant ExtractVariant(T* ptr) { return reinterpret_cast<size_t>(ptr); }
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

template<class T> using PointerPropertyPtr = PropertyPromise<PointerProperty<T>>;

#endif // PROPERTYPROMISE_H
