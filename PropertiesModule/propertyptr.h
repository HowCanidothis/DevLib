#ifndef PROPERTYPTR_H
#define PROPERTYPTR_H
#include "property.h"

template<class T>
class TPropertyPtrBase : public Property
{
public:
    TPropertyPtrBase(const QString& path, T* initial)
        : Property(path)
        , _value(initial)
    {}
    operator const T&() const { return *_value; }
    operator T&() { return *_value; }
protected:
    T* _value;
};

template<class T>
class TPropertyPtr : public TPropertyPtrBase<T>
{
public:
    TPropertyPtr(const QString& path, T* initial, const T& min, const T& max)
        : TPropertyPtrBase<T>(path, initial)
        , _min(min)
        , _max(max)
    {}
    virtual QVariant GetMin() const Q_DECL_OVERRIDE { return _min; }
    virtual QVariant GetMax() const Q_DECL_OVERRIDE { return _max; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return *_value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->_value = clamp((T)value.toDouble(), _min, _max); }
private:
    T _min;
    T _max;
};

template<>
class TPropertyPtr<bool> : public TPropertyPtrBase<bool>
{
public:
    TPropertyPtr<bool>(const QString& path, bool* initial)
        : TPropertyPtrBase<bool>(path, initial)
    {}

    bool& operator=(bool value) { *this->_value = value; return *this->_value; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return *_value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->_value = value.toBool(); }
};

template<>
class TPropertyPtr<QString> : public TPropertyPtrBase<QString>
{
public:
    TPropertyPtr(const QString& path, QString* initial)
        : TPropertyPtrBase<QString>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return *_value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->_value = value.toString(); }
};

template<>
class TPropertyPtr<Name> : public TPropertyPtrBase<Name>
{
public:
    TPropertyPtr<Name>(const QString& path, Name* initial)
        : TPropertyPtrBase<Name>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return _value->AsString(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value->SetName(value.toString()); }
};

class TextFileNamePropertyPtr : public TPropertyPtr<QString>
{
public:
    TextFileNamePropertyPtr(const QString& path, QString* initial)
        : TPropertyPtr<QString>(path, initial)
    {}
    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateFileName; }
};

typedef TPropertyPtr<bool> BoolPropertyPtr;
typedef TPropertyPtr<double> DoublePropertyPtr;
typedef TPropertyPtr<float> FloatPropertyPtr;
typedef TPropertyPtr<qint32> IntPropertyPtr;
typedef TPropertyPtr<quint32> UIntPropertyPtr;
typedef TPropertyPtr<QString> StringPropertyPtr;
typedef TPropertyPtr<Name> NamePropertyPtr;

class Vector3FPropertyPtr
{
public:
    FloatPropertyPtr X;
    FloatPropertyPtr Y;
    FloatPropertyPtr Z;

    Vector3FPropertyPtr(const QString& path, Vector3F* vector);
};

#endif // PROPERTYPTR_H
