#ifndef EXTERNALPROPERTY_H
#define EXTERNALPROPERTY_H

#include "property.h"

template<class T>
class TExternalPropertyBase : public Property
{
protected:
    typedef std::function<T ()> FGetter;
    typedef std::function<void (T value, T oldValue)> FSetter;
public:
    TExternalPropertyBase(const Name& path,const FGetter& getter, const FSetter& setter)
        : Property(path)
        , _getter(getter)
        , _setter(setter)
    {}

protected:
    FGetter defaultGetter(T& ref) { return [&ref]{ return ref; }; }
    FSetter defaultSetter(T& ref) { return [&ref](const T& value, const T&) { ref = value; }; }

protected:
    FGetter _getter;
    FSetter _setter;
};

template<class T>
class TExternalProperty : public TExternalPropertyBase<T>
{
    typedef TExternalPropertyBase<T> Super;
public:
    TExternalProperty(const Name& path,const FGetter& getter, const FSetter& setter, const T& min, const T& max)
        : Super(path, getter, setter)
        , _min(min)
        , _max(max)
    {}
    TExternalProperty(const Name &path, T& ref, const T &min, const T &max)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
        , _min(min)
        , _max(max)
    {}

    void SetMinMax(T min, T max)
    {
        _min = min;
        _max = max;
        T value = _getter();
        if(value < _min) {
            SetValue(_min);
        }else if(value > _max) {
            SetValue(_max);
        }
    }

    virtual QVariant GetMin() const Q_DECL_OVERRIDE { return _min; }
    virtual QVariant GetMax() const Q_DECL_OVERRIDE { return _max; }

    // Property interface
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return Super::_getter(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE {  Super::_setter(value.toDouble(),  Super::_getter()); }
    T _min;
    T _max;
};

template<>
class TExternalProperty<QString> : public TExternalPropertyBase<QString>
{
    typedef TExternalPropertyBase<QString> Super;
public:
    TExternalProperty(const Name& path,const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter)
    {}
    TExternalProperty(const Name& path, QString& ref)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return _getter(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { _setter(value.toString(), _getter()); }
};

template<>
class TExternalProperty<Name> : public TExternalPropertyBase<Name>
{
    typedef TExternalPropertyBase<Name> Super;
public:
    TExternalProperty(const Name& path,const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter)
    {}
    TExternalProperty(const Name& path, Name& ref)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
    {}
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return _getter().AsString(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { _setter(Name(value.toString()), _getter()); }
};

class ExternalStdWStringProperty : public TExternalProperty<QString>
{
    typedef TExternalProperty<QString> Super;
public:
    ExternalStdWStringProperty(const Name& path, std::wstring& ref)
        : Super(path, [&ref]{ return QString::fromStdWString(ref); }, [&ref](const QString& value, const QString&){ ref = value.toStdWString(); } )
    {}
};

template<>
class TExternalProperty<QByteArray> : public TExternalPropertyBase<QByteArray>
{
    typedef TExternalPropertyBase<QByteArray> Super;
public:
    TExternalProperty(const Name& path,const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter)
    {}
    TExternalProperty(const Name& path, QByteArray& ref)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
    {}

protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return _getter(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { _setter(value.toByteArray(), _getter()); }
};

template<>
class TExternalProperty<bool> : public TExternalPropertyBase<bool>
{
    typedef TExternalPropertyBase<bool> Super;
public:
    TExternalProperty(const Name& path, const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter)
    {}
    TExternalProperty(const Name& path, bool& ref)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
    {}

protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return _getter(); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { _setter(value.toBool(), _getter()); }
};

class _Export ExternalNamedUIntProperty : public TExternalProperty<quint32>
{
    typedef TExternalProperty<quint32> Super;
public:
    ExternalNamedUIntProperty(const Name& path, const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter, 0, 0)
    {}
    template<typename Enum>
    ExternalNamedUIntProperty(const Name& path, Enum& ref)
        : Super(path, defaultGetter(reinterpret_cast<quint32&>(ref)), defaultSetter(reinterpret_cast<quint32&>(ref)), 0, 0)
    {}

    void SetNames(const QStringList& names);

    virtual DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateNamedUInt; }
    virtual const QVariant* GetDelegateData() const Q_DECL_OVERRIDE{ return &_names; }
protected:
    virtual QVariant getDisplayValue() const Q_DECL_OVERRIDE { return _names.value<QStringList>().at(_getter()); }

private:
    QVariant _names;
};

class ExternalTextFileNameProperty : public TExternalProperty<QString>
{
public:
    ExternalTextFileNameProperty(const Name& path, QString& ref)
        : TExternalProperty(path, ref)
    {}
    virtual DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateFileName; }
};

// Externals
typedef TExternalProperty<bool> ExternalBoolProperty;
typedef TExternalProperty<double> ExternalDoubleProperty;
typedef TExternalProperty<float> ExternalFloatProperty;
typedef TExternalProperty<qint32> ExternalIntProperty;
typedef TExternalProperty<quint32> ExternalUIntProperty;
typedef TExternalProperty<Name> ExternalNameProperty;
typedef TExternalProperty<QString> ExternalStringProperty;
typedef TExternalProperty<QUrl> ExternalUrlProperty;
typedef TExternalProperty<QByteArray> ExternalByteArrayProperty;

struct _Export ExternalVector3FProperty
{
    ExternalFloatProperty X;
    ExternalFloatProperty Y;
    ExternalFloatProperty Z;

    ExternalVector3FProperty(const QString& path, Vector3F& vector);

    void Subscribe(const Property::FOnChange& handle);
    void SetReadOnly(bool readOnly);
};

#endif // EXTERNALPROPERTY_H
