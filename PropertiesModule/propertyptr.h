#ifndef PROPERTYPTR_H
#define PROPERTYPTR_H
#include "property.h"

template<class T>
class TPropertyPtrBase : public Property
{
public:
    TPropertyPtrBase(const QString& path, T* initial)
        : Property(path)
        , value(initial)
    {}
    operator const T&() const { return *value; }
    operator T&() { return *value; }
protected:
    T* value;
};

template<class T>
class TPropertyPtr : public TPropertyPtrBase<T>
{
public:
    TPropertyPtr(const QString& path, T* initial, const T& min, const T& max)
        : TPropertyPtrBase<T>(path, initial)
        , min(min)
        , max(max)
    {}
    virtual QVariant getMin() const Q_DECL_OVERRIDE { return min; }
    virtual QVariant getMax() const Q_DECL_OVERRIDE { return max; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return *value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->value = clamp((T)value.toDouble(), min, max); }
private:
    T min;
    T max;
};

template<>
class TPropertyPtr<bool> : public TPropertyPtrBase<bool>
{
public:
    TPropertyPtr<bool>(const QString& path, bool* initial)
        : TPropertyPtrBase<bool>(path, initial)
    {}

    bool& operator=(bool value) { *this->value = value; return *this->value; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return *value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->value = value.toBool(); }
};

template<>
class TPropertyPtr<QString> : public TPropertyPtrBase<QString>
{
public:
    TPropertyPtr(const QString& path, QString* initial)
        : TPropertyPtrBase<QString>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return *value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { *this->value = value.toString(); }
};

template<>
class TPropertyPtr<Name> : public TPropertyPtrBase<Name>
{
public:
    TPropertyPtr<Name>(const QString& path, Name* initial)
        : TPropertyPtrBase<Name>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return value->AsString(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->value->SetName(value.toString()); }
};

class TextFileNamePropertyPtr : public TPropertyPtr<QString>
{
public:
    TextFileNamePropertyPtr(const QString& path, QString* initial)
        : TPropertyPtr<QString>(path, initial)
    {}
    bool isTextFileName() const Q_DECL_OVERRIDE { return true; }
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
    FloatPropertyPtr x;
    FloatPropertyPtr y;
    FloatPropertyPtr z;

    Vector3FPropertyPtr(const QString& path, Vector3F* vector);
};

#endif // PROPERTYPTR_H
