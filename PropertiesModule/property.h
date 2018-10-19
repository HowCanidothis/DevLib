#ifndef PROPERTY_H
#define PROPERTY_H

#include <QUrl>
#include <functional>

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/decl.h> // Vector3f

class _Export Property {
public:
    typedef std::function<void ()> FSetter;
    typedef std::function<void (const FSetter&)> FHandle;
    typedef std::function<void ()> FOnChange;
    typedef std::function<void (const QVariant& property, QVariant& new_value)> FValidator;

public:
    enum Option {
        Option_IsExportable = 0x1,// if property should be saved or loaded from file
        Option_IsPresentable = 0x2,// if property should be presented in properties model
        Option_IsReadOnly = 0x4, // if property cans be edited from gui

        Options_Default = Option_IsExportable | Option_IsPresentable,
        Options_InternalProperty = 0
    };
    DECL_FLAGS(Options, Option)

    enum DelegateValue {
        DelegateDefault,
        DelegateFileName,
        DelegatePositionXYZ,
        DelegateNamedUInt,

        DelegateUser
    };

    Property(const Name& path);
    virtual ~Property() {}
    bool SetValue(QVariant value);
    const Options& GetOptions() const { return _options; }
    Options& ChangeOptions() { return _options; }

    FHandle& Handler() { return _fHandle; }
    FValidator& Validator() { return _fValidator; }
    FOnChange& OnChange();

    void Subscribe(const FOnChange& onChange);

    void Invoke() { _fOnChange(); }

    virtual DelegateValue GetDelegateValue() const { return DelegateDefault; }
    virtual const QVariant* GetDelegateData() const { return nullptr; }
    virtual void SetDelegateData(const QVariant& value) { SetValue(value); }

    const QVariant& GetPreviousValue() const { return _previousValue; }
    QVariant GetValue() const { return getValue(); }
    virtual QVariant GetMin() const { return 0; }
    virtual QVariant GetMax() const { return 0; }

protected:
    friend class PropertiesSystem;
    friend class PropertiesModel;

    virtual QVariant getDisplayValue() const { return getValue(); }
    virtual QVariant getValue() const=0;
    virtual void setValueInternal(const QVariant&)=0;

    Q_DISABLE_COPY(Property)

protected:
    FHandle _fHandle;
    FOnChange _fOnChange;
    FValidator _fValidator;
    Options _options;
    QVariant _previousValue;
#ifdef DEBUG_BUILD
    bool _isSubscribed;
#endif
};

template<class T>
class TPropertyBase : public Property
{
    typedef TPropertyBase Super;
public:
    typedef T value_type;
    TPropertyBase(const Name& path, const T& initial)
        : Property(path)
        , _value(initial)
    {}

    const T& Native() const { return _value; }
    const T* Ptr() const { return &_value; }
    operator const T&() const { return _value; }

    template<class T2> const T2& Cast() const { return (const T2&)_value; }

protected:
    T _value;
};

template<class T>
class TStdPropertyBase : public TPropertyBase<T>
{
    typedef TPropertyBase<T> Super;
protected:
    TStdPropertyBase(const Name& path, const T& initial)
        : TPropertyBase<T>(path, initial)
    {}

    virtual QVariant getValue() const Q_DECL_OVERRIDE { return Super::_value; }
};

template<class T>
class TProperty : public TStdPropertyBase<T>
{
    typedef TStdPropertyBase<T> Super;
public:
    TProperty(const Name& path, const T& initial, const T& min, const T& max)
        : TStdPropertyBase<T>(path, initial)
        , _min(min)
        , _max(max)
    {}

    void SetMinMax(const T& min, const T& max)
    {
        _min = min;
        _max = max;
        if(Super::_value < _min) {
            SetValue(_min);
        }else if(Super::_value > _max) {
            SetValue(_max);
        }
    }

    const T& GetMinValue() const { return _min; }
    const T& GetMaxValue() const { return _max; }

    TProperty<T>& operator=(const T& value) { this->SetValue(value); return *this; }

    virtual QVariant GetMin() const Q_DECL_OVERRIDE { return _min; }
    virtual QVariant GetMax() const Q_DECL_OVERRIDE { return _max; }
protected:
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = clamp((T)value.toDouble(), _min, _max); }
protected:
    T _min;
    T _max;
};

template<>
class TProperty<bool> : public TStdPropertyBase<bool>
{
public:
    TProperty<bool>(const Name& path, bool initial)
        : TStdPropertyBase<bool>(path, initial)
    {}

    TProperty<bool>& operator=(bool value) { this->SetValue(value); return *this; }
protected:
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = value.toBool(); }
};

template<>
class TProperty<QString> : public TStdPropertyBase<QString>
{
public:
    TProperty<QString>(const Name& path, const QString& initial)
        : TStdPropertyBase<QString>(path, initial)
    {}

    TProperty<QString>& operator=(const QString& value) { this->SetValue(value); return *this; }

protected:
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { this->_value = value.toString(); }
};

template<>
class TProperty<QByteArray> : public TStdPropertyBase<QByteArray>
{
public:
    TProperty<QByteArray>(const Name& path, const QByteArray& initial)
        : TStdPropertyBase<QByteArray>(path, initial)
    {}

    TProperty<QByteArray>& operator=(const QByteArray& value) { this->SetValue(value); return *this; }
protected:
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE{ this->_value = value.toByteArray(); }
};

template<>
class TProperty<QUrl> : public TStdPropertyBase<QUrl>
{
public:
    TProperty<QUrl>(const Name& path, const QUrl& initial)
        : TStdPropertyBase<QUrl>(path, initial)
    {}

    TProperty<QUrl>& operator=(const QUrl& value) { this->SetValue(value); return *this; }
protected:
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE{ this->_value = value.toUrl(); }
};

// Extended
template<class T>
class PointerProperty : public TPropertyBase<T*>
{
    typedef TPropertyBase<T*> Super;
public:
    PointerProperty(const Name& path, T* initial)
        : Super(path, initial)
    {}

    PointerProperty<T>& operator=(T* ptr) { SetValue(reinterpret_cast<size_t>(ptr)); return *this; }

    // Property interface
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return reinterpret_cast<size_t>(Super::_value); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::_value = reinterpret_cast<T*>(value.toLongLong()); }
};

class TextFileNameProperty : public TProperty<QString>
{
public:
    TextFileNameProperty(const Name& path, const QString& initial)
        : TProperty<QString>(path, initial)
    {}
    virtual DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateFileName; }
};

class _Export NamedUIntProperty : public TProperty<quint32>
{
    typedef TProperty<quint32> Super;
public:
    NamedUIntProperty(const Name& path, const quint32& initial)
        : Super(path, initial, 0, 0)
    {}

    void SetNames(const QStringList& names);

    virtual DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateNamedUInt; }
    virtual const QVariant* GetDelegateData() const Q_DECL_OVERRIDE{ return &_names; }

protected:
    virtual QVariant getDisplayValue() const Q_DECL_OVERRIDE { return _names.value<QStringList>().at(Super::_value); }

private:
    QVariant _names;
};

class _Export UrlListProperty : public TPropertyBase<QList<QUrl>>
{
    typedef TPropertyBase<QList<QUrl>> Super;
public:
    UrlListProperty(const Name& path, qint32 maxCount = -1)
        : Super(path, {})
        , _maxCount(maxCount)
    {}

    void AddUniqueUrl(const QUrl& url);

    // Property interface
protected:
    virtual QVariant getValue() const Q_DECL_OVERRIDE { return QUrl::toStringList(Super::_value); }
    virtual void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::_value = QUrl::fromStringList(value.toStringList()); }

private:
    qint32 _maxCount;
};

// Internals
typedef TProperty<bool> BoolProperty;
typedef TProperty<double> DoubleProperty;
typedef TProperty<float> FloatProperty;
typedef TProperty<qint32> IntProperty;
typedef TProperty<quint32> UIntProperty;
typedef TProperty<QString> StringProperty;
typedef TProperty<QUrl> UrlProperty;
typedef TProperty<QByteArray> ByteArrayProperty;

class _Export Vector3FProperty
{
public:
    FloatProperty X;
    FloatProperty Y;
    FloatProperty Z;

    Vector3FProperty(const QString& path, const Vector3F& vector);
};

#endif // PROPERTY_H
