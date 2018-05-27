#ifndef PROPERTY_H
#define PROPERTY_H

#include "Shared/shared_decl.h"
#include "SharedGui/decl.h" // Vector3f

class Property {
protected:
    typedef std::function<void ()> FSetter;
    typedef std::function<void (const FSetter&)> FHandle;
    typedef std::function<void ()> FOnChange;
    typedef std::function<void (const QVariant& property, QVariant& new_value)> FValidator;

    FHandle _fHandle;
    FOnChange _fOnset;
    FValidator _fValidator;
    bool _bReadOnly;
public:
    Property(const QString& path);
    void SetValue(QVariant value);

    FHandle& Handle() { return _fHandle; }
    FValidator& Validator() { return _fValidator; }
    FOnChange& OnChange() { return _fOnset; }

    void Invoke() { _fHandle([]{}); }

    virtual bool IsTextFileName() const { return false; }

    void SetReadOnly(bool flag) { _bReadOnly = flag; }
    bool IsReadOnly() const { return _bReadOnly; }
    virtual QVariant GetMin() const { return 0; }
    virtual QVariant GetMax() const { return 0; }
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
        , _value(initial)
    {}

    T* ptr() { return &_value; }
    operator const T&() const { return _value; }
    operator T&() { return _value; }
protected:
    T _value;
};

template<class T>
class TProperty : public TPropertyBase<T>
{
public:
    TProperty(const QString& path, const T& initial, const T& min, const T& max)
        : TPropertyBase<T>(path, initial)
        , _min(min)
        , _max(max)
    {}
    virtual QVariant GetMin() const Q_DECL_OVERRIDE { return _min; }
    virtual QVariant GetMax() const Q_DECL_OVERRIDE { return _max; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return _value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = clamp((T)value.toDouble(), _min, _max); }
private:
    T _min;
    T _max;
};

template<>
class TProperty<bool> : public TPropertyBase<bool>
{
public:
    TProperty<bool>(const QString& path, bool initial)
        : TPropertyBase<bool>(path, initial)
    {}

    bool& operator=(bool value) { this->_value = value; return this->_value; }
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return _value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = value.toBool(); }
};

template<>
class TProperty<QString> : public TPropertyBase<QString>
{
public:
    TProperty<QString>(const QString& path, const QString& initial)
        : TPropertyBase<QString>(path, initial)
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE{ return _value; }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = value.toString(); }
};

class TextFileNameProperty : public TProperty<QString>
{
public:
    TextFileNameProperty(const QString& path, const QString& initial)
        : TProperty<QString>(path, initial)
    {}
    bool IsTextFileName() const Q_DECL_OVERRIDE { return true; }
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
    FloatProperty X;
    FloatProperty Y;
    FloatProperty Z;

    Vector3FProperty(const QString& path, const Vector3F& vector);
};

#endif // PROPERTY_H
