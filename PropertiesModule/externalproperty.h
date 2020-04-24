#ifndef EXTERNALPROPERTY_H
#define EXTERNALPROPERTY_H

#include "propertypromise.h"

class ExternalPropertyProperty : public Property
{
    using Super = Property;
public:
    ExternalPropertyProperty(const Name& path, const Name& propertyPath, const PropertiesScopeName& propertyScope, Property::Options options = Property::Options_Default)
        : Super(path, options)
        , m_property(propertyPath, propertyScope)
    {}
    ExternalPropertyProperty(const Name& path, Property* property, Property::Options options = Property::Options_Default)
        : Super(path, options)
        , m_property(property)
    {}

protected:
    QVariant GetMin() const override { return m_property.GetProperty()->GetMin(); }
    QVariant GetMax() const override { return m_property.GetProperty()->GetMax(); }
    DelegateValue GetDelegateValue() const override { return m_property.GetProperty()->GetDelegateValue(); }
    const QVariant* GetDelegateData() const override { return m_property.GetProperty()->GetDelegateData(); }
    void SetDelegateData(const QVariant& value) override { m_property.GetProperty()->SetDelegateData(value); }
    QVariant getValue() const override { return m_property.GetProperty()->getValue(); }
    void setValueInternal(const QVariant& value) override {  m_property.GetProperty()->setValueInternal(value); }

private:
    PropertyPromiseBase m_property;
};

template<class T>
class TExternalPropertyBase : public Property
{
protected:
    typedef std::function<T ()> FGetter;
    typedef std::function<void (T value, T oldValue)> FSetter;

public:
    typedef T value_type;
    TExternalPropertyBase(const Name& path,const FGetter& getter, const FSetter& setter, Options options = Options_Default)
        : Property(path, options)
        , m_getter(getter)
        , m_setter(setter)
    {}
    TExternalPropertyBase(const Name& path, T& ref, Options options = Options_Default)
        : Property(path, options)
        , m_getter(defaultGetter(ref))
        , m_setter(defaultSetter(ref))
    {}

    operator T() const { return m_getter(); }

protected:
    FGetter defaultGetter(T& ref) { return [&ref]{ return ref; }; }
    FSetter defaultSetter(T& ref) { return [&ref](const T& value, const T&) { ref = value; }; }

protected:
    FGetter m_getter;
    FSetter m_setter;
};

template<class T>
class TExternalProperty : public TExternalPropertyBase<T>
{
    using Super = TExternalPropertyBase<T>;
public:
    using Super::Super;

protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return Super::m_getter(); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE {  Super::m_setter(value.value<T>(),  Super::m_getter()); }
};

template<class T>
class TExternalDecimalProperty : public TExternalProperty<T>
{
    typedef TExternalProperty<T> Super;
public:
    TExternalDecimalProperty(const Name& path,const typename Super::FGetter& getter, const typename Super::FSetter& setter, const T& min, const T& max)
        : Super(path, getter, setter)
        , m_min(min)
        , m_max(max)
    {}
    TExternalDecimalProperty(const Name& path,const typename Super::FGetter& getter, const typename Super::FSetter& setter)
        : Super(path, getter, setter)
        , m_min(0)
        , m_max(0)
    {}
    TExternalDecimalProperty(const Name& path, T& ref, const T& min, const T& max)
        : Super(path, this->defaultGetter(ref), this->defaultSetter(ref))
        , m_min(min)
        , m_max(max)
    {}
    TExternalDecimalProperty(const Name& path, T& ref)
        : Super(path, this->defaultGetter(ref), this->defaultSetter(ref))
        , m_min(0)
        , m_max(0)
    {}

    void SetMinMax(T min, T max)
    {
        m_min = min;
        m_max = max;
        T value = this->m_getter();
        if(value < m_min) {
            SetValue(m_min);
        }else if(value > m_max) {
            SetValue(m_max);
        }
    }

    T GetMinValue() const { return m_min; }
    T GetMaxValue() const { return m_max; }

    QVariant GetMin() const Q_DECL_OVERRIDE { return m_min; }
    QVariant GetMax() const Q_DECL_OVERRIDE { return m_max; }

    // Property interface
protected:
    T m_min;
    T m_max;
};

class ExternalNameProperty : public TExternalPropertyBase<Name>
{
    typedef TExternalPropertyBase<Name> Super;
public:
    ExternalNameProperty(const Name& path,const FGetter& getter, const FSetter& setter)
        : Super(path, getter, setter)
    {}
    ExternalNameProperty(const Name& path, Name& ref)
        : Super(path, defaultGetter(ref), defaultSetter(ref))
    {}
protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return m_getter().AsString(); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { m_setter(Name(value.toString()), m_getter()); }
};

class _Export ExternalNamedUIntProperty : public TExternalDecimalProperty<quint32>
{
    typedef TExternalDecimalProperty<quint32> Super;
public:
    using Super::Super;

    void SetNames(const QStringList& names);

    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateNamedUInt; }
    const QVariant* GetDelegateData() const Q_DECL_OVERRIDE{ return &m_names; }
protected:
    QVariant getDisplayValue() const Q_DECL_OVERRIDE { Q_ASSERT(!m_names.value<QStringList>().isEmpty()); return m_names.value<QStringList>().at(m_getter()); }

private:
    QVariant m_names;
};

// Externals
typedef TExternalProperty<QString> ExternalStringProperty;
typedef TExternalProperty<bool> ExternalBoolProperty;
typedef TExternalDecimalProperty<double> ExternalDoubleProperty;
typedef TExternalDecimalProperty<float> ExternalFloatProperty;
typedef TExternalDecimalProperty<qint32> ExternalIntProperty;
typedef TExternalDecimalProperty<quint32> ExternalUIntProperty;

class ExternalStdWStringProperty : public ExternalStringProperty
{
    typedef ExternalStringProperty Super;
public:
    ExternalStdWStringProperty(const Name& path, std::wstring& ref)
        : Super(path, [&ref]{ return QString::fromStdWString(ref); }, [&ref](const QString& value, const QString&){ ref = value.toStdWString(); } )
    {}
};

class ExternalTextFileNameProperty : public ExternalStringProperty
{
    typedef ExternalStringProperty Super;
public:
    ExternalTextFileNameProperty(const Name& path, QString& ref)
        : ExternalStringProperty(path, ref)
    {}
    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateFileName; }
};

#ifdef QT_GUI_LIB

#include <QColor>

class ExternalColorProperty : public TExternalProperty<QColor>
{
    typedef TExternalProperty<QColor> Super;
public:
    using Super::Super;

    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateColor; }
};

struct _Export ExternalVector3FProperty
{
    ExternalFloatProperty X;
    ExternalFloatProperty Y;
    ExternalFloatProperty Z;

    ExternalVector3FProperty(const QString& path, Vector3F& vector);

    void Subscribe(const Property::FOnChange& handle);
    void SetReadOnly(bool readOnly);
};

#endif // QT_GUI_LIB_LIB

#endif // EXTERNALPROPERTY_H
