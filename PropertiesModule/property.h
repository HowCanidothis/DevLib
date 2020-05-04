#ifndef PROPERTY_H
#define PROPERTY_H

#include <QUrl>

#include <functional>

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/decl.h> // Vector3f

template<typename T>
struct PropertyValueExtractorPrivate
{
    static QVariant ExtractVariant(const T& value) { return value; }
};

template<typename T>
struct PropertyValueExtractorPrivate<T*>
{
    static QVariant ExtractVariant(T* ptr) { return QVariant((qint64)(ptr)); }
};

template<typename T>
struct PropertyValueExtractorPrivate<QList<T>>
{
    static QVariant ExtractVariant(const QList<T>& value) { return TextConverter<QList<T>>::ToText(value); }
};

template<typename Key, typename Value>
struct PropertyValueExtractorPrivate<QHash<Key, Value>>
{
    static QVariant ExtractVariant(const QHash<Key, Value>& value) { return TextConverter<QHash<Key, Value>>::ToText(value); }
};

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
        Options_ReadOnlyPresentable = Option_IsPresentable | Option_IsReadOnly,
        Options_InternalProperty = 0
    };
    DECL_FLAGS(Options, Option)

    enum DelegateValue {
        DelegateDefault,
        DelegateFileName,
        DelegatePositionXYZ,
        DelegateNamedUInt,
        DelegateRect,
        DelegateColor,

        DelegateUser
    };

    enum PropertyRole {
        RoleHeaderItem = Qt::UserRole, // Using for delegating headers
        RoleMinValue,
        RoleMaxValue,
        RoleDelegateValue,
        RoleDelegateData,
    };

    Property(const Name& path, Options options);
    virtual ~Property() {}
    bool SetValue(QVariant value);
    const Options& GetOptions() const { return m_options; }
    Options& ChangeOptions() { return m_options; }

    FHandle& Handler() { return m_fHandle; }
    FValidator& Validator() { return m_fValidator; }

    void Subscribe(const FOnChange& onChange);
    void Invoke();
    void InstallObserver(Dispatcher::Observer observer, const FAction& action) { m_onChangeDispatcher += {observer, action}; }
    void RemoveObserver(Dispatcher::Observer observer) { m_onChangeDispatcher -= observer; }
    Dispatcher& GetDispatcher() { return m_onChangeDispatcher; }

    virtual DelegateValue GetDelegateValue() const { return DelegateDefault; }
    virtual const QVariant* GetDelegateData() const { return nullptr; }
    virtual void SetDelegateData(const QVariant& value) { SetValue(value); }

    const QVariant& GetPreviousValue() const { return m_previousValue; }
    QVariant GetValue() const { return getValue(); }
    const Name& GetPropertyName() const { return m_propertyName; }
    virtual QVariant GetMin() const { return 0; }
    virtual QVariant GetMax() const { return 0; }
    SharedPointer<class ExternalPropertyProperty> Clone(const Name& newName) const;

    QVariant GetValueFromRole(int role) const;

protected:
    friend class ExternalPropertyProperty;
    friend class PropertiesSystem;
    friend class PropertiesScope;
    friend class PropertiesModel;

    virtual QVariant getDisplayValue() const { return getValue(); }
    virtual QVariant getValue() const=0;
    virtual void setValueInternal(const QVariant&)=0;

    Q_DISABLE_COPY(Property)

protected:
    Dispatcher m_onChangeDispatcher;
    FHandle m_fHandle;
    FOnChange m_fOnChange;
    FValidator m_fValidator;
    Options m_options;
    QVariant m_previousValue;
    Name m_propertyName;
#ifdef DEBUG_BUILD
    bool m_isSubscribed;
#endif
};

template<class T>
class TPropertyBase : public Property
{
    typedef TPropertyBase Super;
public:
    typedef T value_type;
    TPropertyBase(const Name& path, const T& initial, Options options = Options_Default)
        : Property(path, options)
        , m_value(initial)
    {}

    // Avoid invoking. Sometimes it's helpfull
    void SetDirect(const T& value) { m_value = value; }

    const T& Native() const { return m_value; }
    const T* Ptr() const { return &m_value; }
    operator const T&() const { return m_value; }

    bool operator!=(const T& another) const { return m_value != another; }
    TPropertyBase<T>& operator=(const T& value) { this->SetValue(value); return *this; }

    template<class T2> const T2& Cast() const { return (const T2&)m_value; }

protected:
    T m_value;
};

template<class T>
class TProperty : public TPropertyBase<T>
{
    typedef TPropertyBase<T> Super;
public:
    TProperty(const Name& path, const T& initial, Property::Options options = Property::Options_Default)
        : TPropertyBase<T>(path, initial, options)
    {}

    TProperty<T>& operator=(const T& value) { this->SetValue(value); return *this; }

protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return Super::m_value; }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = value.value<T>(); }
};

template<class T>
class TDecimalProperty : public TProperty<T>
{
    typedef TProperty<T> Super;
public:
    TDecimalProperty(const Name& path, const T& initial, const T& min = -std::numeric_limits<T>::max(), const T& max = std::numeric_limits<T>::max(), Property::Options options = Property::Options_Default)
        : Super(path, initial, options)
        , m_min(min)
        , m_max(max)
    {
        Super::Validator() = [this](const QVariant&, QVariant& value) {
            value = ::clamp(value.value<T>(), m_min, m_max);
        };
    }

    void SetMinMax(const T& min, const T& max)
    {
        m_min = min;
        m_max = max;
        if(Super::m_value < m_min) {
            SetValue(m_min);
        }else if(Super::m_value > m_max) {
            SetValue(m_max);
        }
    }

    const T& GetMinValue() const { return m_min; }
    const T& GetMaxValue() const { return m_max; }

    TDecimalProperty<T>& operator+=(const T& value) { this->SetValue(Super::m_value + value); return *this; }
    TDecimalProperty<T>& operator-=(const T& value) { this->SetValue(Super::m_value - value); return *this; }
    TDecimalProperty<T>& operator=(const T& value) { this->SetValue(value); return *this; }

    QVariant GetMin() const Q_DECL_OVERRIDE { return m_min; }
    QVariant GetMax() const Q_DECL_OVERRIDE { return m_max; }

protected:
    T m_min;
    T m_max;
};


// Extended
template<class T>
class PointerProperty : public TPropertyBase<T*>
{
    typedef TPropertyBase<T*> Super;
public:
    PointerProperty(const Name& path, T* initial)
        : Super(path, initial, Super::Options_InternalProperty)
    {
    }

    T* operator->() { return this->Native(); }
    const T* operator->() const { return this->Native(); }
    PointerProperty<T>& operator=(T* ptr) { this->SetValue(reinterpret_cast<qulonglong>(ptr)); return *this; }

    // Property interface
protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return PropertyValueExtractorPrivate<typename Super::value_type>::ExtractVariant(Super::m_value); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = reinterpret_cast<T*>(value.toLongLong()); }
};

class FileNameProperty : public TProperty<QString>
{
public:
    FileNameProperty(const Name& path, const QString& initial, Options options = Options_Default)
        : TProperty<QString>(path, initial, options)
    {}
    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateFileName; }
};

class _Export NamedUIntProperty : public TDecimalProperty<quint32>
{
    typedef TDecimalProperty<quint32> Super;
public:
    NamedUIntProperty(const Name& path, const quint32& initial, Options options = Options_Default)
        : Super(path, initial, 0, 0, options)
    {}

    void SetNames(const QStringList& names);

    NamedUIntProperty& operator=(quint32 value)
    {
        SetValue(value);
        return *this;
    }

    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateNamedUInt; }
    const QVariant* GetDelegateData() const Q_DECL_OVERRIDE{ return &m_names; }

protected:
    QVariant getDisplayValue() const Q_DECL_OVERRIDE { Q_ASSERT(!m_names.value<QStringList>().isEmpty()); return m_names.value<QStringList>().at(Super::m_value); }

private:
    QVariant m_names;
};

class _Export UrlListProperty : public TPropertyBase<QList<QUrl>>
{
    typedef TPropertyBase<QList<QUrl>> Super;
public:
    UrlListProperty(const Name& path, qint32 maxCount = -1, Options options = Options_Default)
        : Super(path, {}, options)
        , m_maxCount(maxCount)
    {}

    void AddUniqueUrl(const QUrl& url);

    // Property interface
protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return QUrl::toStringList(Super::m_value); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = QUrl::fromStringList(value.toStringList()); }

private:
    qint32 m_maxCount;
};

template<class Key, class Value>
class _Export HashProperty : public TPropertyBase<QHash<Key, Value>>
{
    using Super = TPropertyBase<QHash<Key, Value>>;

public:
    HashProperty(const Name& path, Property::Options options = Super::Options_Default)
        : Super(path, {}, options)
    {}

    void Insert(const Key& key, const Value& value)
    {
        QHash<Key, Value>& hash = Super::m_value;
        auto foundIt = hash.find(key);
        if(foundIt != hash.end()) {
            if(*foundIt != value) {
                *foundIt = value;
                Super::Invoke();
            }
        } else {
            Super::m_previousValue = getValue();
            hash.insert(key, value);
            Super::Invoke();
        }
    }

    void Remove(const Key& key)
    {
        auto& hash = Super::m_value;
        auto foundIt = hash.find(key);
        if(foundIt != hash.end()) {
            Super::m_previousValue = getValue();
            hash.erase(foundIt);
            Super::Invoke();
        }
    }

    void Clear()
    {
        if(!Super::m_value.isEmpty()) {
            Super::m_previousValue = getValue();
            Super::m_value.clear();
            Super::Invoke();
        }
    }

    void insert(const Key& key, const Value& value) { Insert(key, value); }
    void remove(const Key& key) { Remove(key); }


    bool contains(const Key& key) const { return Super::m_value.contains(key); }
    qint32 size() const { return Super::m_value.size(); }
    typename QHash<Key, Value>::const_iterator find(const Key& key) const { return Super::m_value.find(key); }
    typename QHash<Key, Value>::const_iterator begin() const { return Super::m_value.begin(); }
    typename QHash<Key, Value>::const_iterator end() const { return Super::m_value.end(); }

    HashProperty& operator=(const QHash<Key, Value>& another)
    {
        if(Super::m_value != another) {
            Super::m_previousValue = getValue();
            Super::m_value = another;
            Super::Invoke();
        }

        return *this;
    }

protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return TextConverter<typename Super::value_type>::ToText(Super::m_value); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = TextConverter<typename Super::value_type>::FromText(value.toString()); }
};

template<class Key>
class _Export SetProperty : public TPropertyBase<QSet<Key>>
{
    using Super = TPropertyBase<QSet<Key>>;

public:
    SetProperty(const Name& path, Property::Options options = Super::Options_Default)
        : Super(path, {}, options)
    {}

    void Insert(const Key& key)
    {
        auto& hash = Super::m_value;
        auto foundIt = hash.find(key);
        if(foundIt == hash.end()) {
            Super::m_previousValue = getValue();
            Super::m_value.insert(key);
            Super::Invoke();
        }
    }

    void Remove(const Key& key)
    {
        auto& hash = Super::m_value;
        auto foundIt = hash.find(key);
        if(foundIt != hash.end()) {
            Super::m_previousValue = getValue();
            hash.erase(foundIt);
            Super::Invoke();
        }
    }

    void Clear()
    {
        auto& hash = Super::m_value;
        if(!hash.isEmpty()) {
            Super::m_previousValue = getValue();
            hash.clear();
            Super::Invoke();
        }
    }

    SetProperty& operator=(const QSet<Key>& another)
    {
        if(Super::m_value != another) {
            Super::m_previousValue = getValue();
            Super::m_value = another;
            Super::Invoke();
        }

        return *this;
    }

    void insert(const Key& key) { Insert(key); }
    void remove(const Key& key) { Remove(key); }

    bool contains(const Key& key) const { return Super::m_value.contains(key); }
    qint32 size() const { return Super::m_value.size(); }
    typename QSet<Key>::const_iterator find(const Key& key) const { return Super::m_value.find(key); }
    typename QSet<Key>::const_iterator begin() const { return Super::m_value.begin(); }
    typename QSet<Key>::const_iterator end() const { return Super::m_value.end(); }

    QSet<Key> GetPreviousValue() const { return TextConverter<typename Super::value_type>::FromText(Super::m_previousValue.toString()); }

protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return TextConverter<typename Super::value_type>::ToText(Super::m_value); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = TextConverter<typename Super::value_type>::FromText(value.toString()); }
};

template<class Key>
class _Export ListProperty : public TPropertyBase<QList<Key>>
{
    using Super = TPropertyBase<QList<Key>>;

public:
    ListProperty(const Name& path, Property::Options options = Super::Options_Default)
        : Super(path, {}, options)
    {}

    void Add(const Key& value)
    {
        Super::m_value.append(value);
        Super::Invoke();
    }

    void Clear()
    {
        if(!Super::m_value.isEmpty()) {
            Super::m_value.clear();
            Super::Invoke();
        }
    }

    void Set(const QVector<Key>& values)
    {
        Super::m_value = values;
        Super::Invoke();
    }

    ListProperty& operator=(const QList<Key>& another)
    {
        if(Super::m_value != another) {
            Super::m_value = another;
            Super::Invoke();
        }

        return *this;
    }

protected:
    QVariant getValue() const Q_DECL_OVERRIDE { return TextConverter<typename Super::value_type>::ToText(Super::m_value); }
    void setValueInternal(const QVariant& value) Q_DECL_OVERRIDE { Super::m_value = TextConverter<typename Super::value_type>::FromText(value.toString()); }
};

class _Export PropertiesDialogGeometryProperty : protected TProperty<QByteArray>
{
    typedef TProperty<QByteArray> Super;

public:
    PropertiesDialogGeometryProperty(const QString& name)
        : Super(Name("PropertiesDialogGeometry/" + name), QByteArray(), Option_IsExportable)
    {
    }
};

// Internals
typedef TProperty<bool> BoolProperty;
typedef TDecimalProperty<double> DoubleProperty;
typedef TDecimalProperty<float> FloatProperty;
typedef TDecimalProperty<qint32> IntProperty;
typedef TDecimalProperty<quint32> UIntProperty;
typedef TProperty<QString> StringProperty;
typedef TProperty<QUrl> UrlProperty;
typedef TProperty<QByteArray> ByteArrayProperty;

#ifdef QT_GUI_LIB
#include <QColor>

#include <SharedGuiModule/internal.hpp>

class ColorProperty : public TProperty<QColor>
{
    typedef TProperty<QColor> Super;
public:
    ColorProperty(const Name& name, const QColor& initial, Options options = Options_Default)
        : Super(name, initial, options)
    {}

    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateColor; }

    ColorProperty& operator=(const QColor& color) { SetValue(color); return *this; }
};

class RectProperty : public TProperty<Rect>
{
    typedef TProperty<Rect> Super;
public:
    RectProperty(const Name& name, const Rect& initial, Options options = Options_Default)
        : Super(name, initial, options)
    {}

    DelegateValue GetDelegateValue() const Q_DECL_OVERRIDE { return DelegateRect; }
};

class _Export Vector3FProperty
{
public:
    FloatProperty X;
    FloatProperty Y;
    FloatProperty Z;

    Vector3FProperty(const QString& path, const Vector3F& vector, Property::Options options = Property::Options_Default);
};

#endif // QT_GUI_LIB_LIB

#endif // PROPERTY_H
