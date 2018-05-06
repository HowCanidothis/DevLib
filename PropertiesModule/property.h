#ifndef PROPERTY_H
#define PROPERTY_H

#include "Shared/shared_decl.h"
#include "SharedGui/gt_decl.h" // Vector3f

class Property {
protected:
    typedef std::function<void ()> FSetter;
    typedef std::function<void (const FSetter&)> FHandle;
    typedef std::function<void ()> FOnChange;
    typedef std::function<void (const QVariant& property, QVariant& new_value)> FValidator;

    FHandle fhandle;
    FOnChange fonset;
    FValidator fvalidator;
    bool read_only;
public:
    Property(const QString& path);
    void setValue(QVariant value);

    FHandle& handle() { return fhandle; }
    FValidator& validator() { return fvalidator; }
    FOnChange& onChange() { return fonset; }

    void invoke() { fhandle([]{}); }

    virtual bool isTextFileName() const { return false; }

    void setReadOnly(bool flag) { read_only = flag; }
    bool isReadOnly() const { return read_only; }
    virtual QVariant getMin() const { return 0; }
    virtual QVariant getMax() const { return 0; }
protected:
    friend class PropertiesSystem;
    friend class PropertiesModel;

    virtual QVariant getValue() const=0;
    virtual void setValueInternal(const QVariant&)=0;
};

template<class T>
class TPropertyBase : public Property
{
public:
    TPropertyBase(const QString& path, const T& initial)
        : Property(path)
        , value(initial)
    {}

    T* ptr() { return &value; }
    operator const T&() const { return value; }
    operator T&() { return value; }
protected:
    T value;
};

template<class T>
class TProperty : public TPropertyBase<T>
{
public:
    TProperty(const QString& path, const T& initial, const T& min, const T& max)
        : TPropertyBase<T>(path, initial)
        , min(min)
        , max(max)
    {}
    virtual QVariant getMin() const Q_DECL_OVERRIDE { return min; }
    virtual QVariant getMax() const Q_DECL_OVERRIDE { return max; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->value = clamp((T)value.toDouble(), min, max); }
private:
    T min;
    T max;
};

template<>
class TProperty<bool> : public TPropertyBase<bool>
{
public:
    TProperty<bool>(const QString& path, bool initial)
        : TPropertyBase<bool>(path, initial)
    {}

    bool& operator=(bool value) { this->value = value; return this->value; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->value = value.toBool(); }
};

template<>
class TProperty<QString> : public TPropertyBase<QString>
{
public:
    TProperty<QString>(const QString& path, const QString& initial)
        : TPropertyBase<QString>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->value = value.toString(); }
};

class TextFileNameProperty : public TProperty<QString>
{
public:
    TextFileNameProperty(const QString& path, const QString& initial)
        : TProperty<QString>(path, initial)
    {}
    bool isTextFileName() const Q_DECL_OVERRIDE { return true; }
};

typedef TProperty<bool> BoolProperty;
typedef TProperty<double> DoubleProperty;
typedef TProperty<float> FloatProperty;
typedef TProperty<qint32> IntProperty;
typedef TProperty<quint32> UIntProperty;
typedef TProperty<QString> StringProperty;

class Vector3FProperty
{
public:
    FloatProperty x;
    FloatProperty y;
    FloatProperty z;

    Vector3FProperty(const QString& path, const Vector3F& vector);
};

#endif // PROPERTY_H
