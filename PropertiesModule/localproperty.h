#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include <limits>

#include "property.h"
#include "externalproperty.h"

template<class T, class StorageType = T>
class LocalProperty
{
    using FSetter = std::function<void ()>;
    using FSetterHandler = std::function<void (const FSetter&)>;
    using FValidator = std::function<T (const T&)>;
protected:
    StorageType m_value;
    FSetterHandler m_setterHandler;
    FValidator m_validator;

public:
    using value_type = T;

    LocalProperty()
        : m_setterHandler([](const FSetter& setter){
            setter();
        })
        , m_validator([](const T& value){ return value; })
    {}
    LocalProperty(const T& value)
        : m_value(value)
        , m_setterHandler([](const FSetter& setter){
            setter();
        })
        , m_validator([](const T& value){ return value; })
    {}
    ~LocalProperty()
    {

    }

    void Invoke()
    {
        m_setterHandler([this]{
            if(m_subscribes != nullptr) {
                m_subscribes();
            }
            OnChange.Invoke();
        });
    }

    void Subscribe(const FAction& subscribe) const
    {
        if(m_subscribes == nullptr) {
            m_subscribes = subscribe;
        } else {
            auto oldHandle = m_subscribes;
            m_subscribes = [subscribe, oldHandle]{
                oldHandle();
                subscribe();
            };
        }
    }

    void SetValidator(const FValidator& validator)
    {
        m_validator = validator;
    }

    void SetAndSubscribe(const FAction& subscribe) const
    {
        Subscribe(subscribe);
        subscribe();
    }

    void SetSetterHandler(const FSetterHandler& handler) {
        m_setterHandler = handler;
    }

    void SetValue(const T& value)
    {
        auto validatedValue = m_validator(value);
        if(validatedValue != m_value) {
            m_setterHandler([validatedValue, this]{
                m_value = validatedValue;
                Invoke();
            });
        }
    }

    DispatcherConnection ConnectFrom(const LocalProperty& another)
    {
        *this = another.Native();
        auto& nonConst = const_cast<LocalProperty&>(another);
        return nonConst.OnChange.Connect(this, [this, &another]{
            *this = another.Native();
        });
    }

    DispatcherConnection ConnectTo(LocalProperty& another)
    {
        another = Native();
        return OnChange.Connect(this, [this, &another]{
            another = Native();
        });
    }

    template<class T2, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>>
    DispatcherConnections ConnectBoth(LocalProperty<T2>& another, const Evaluator& anotherEvaluator, const ThisEvaluator& thisEvaluator, const QVector<Dispatcher*>& dispatchers = {})
    {
        DispatcherConnections result;
        another = anotherEvaluator(Native());
        auto sync = ::make_shared<std::atomic_bool>(false);
        result += another.OnChange.Connect(this, [this, thisEvaluator, &another, sync]{
            if(!*sync) {
                *sync = true;
                *this = thisEvaluator(another);
                *sync = false;
            }
        });
        result += OnChange.Connect(this, [this, anotherEvaluator, &another, sync]{
            if(!*sync) {
                *sync = true;
                another = anotherEvaluator(*this);
                *sync = false;
            }
        });
        for(auto* dispatcher : dispatchers) {
            result += dispatcher->Connect(this, [this, anotherEvaluator, &another, sync]{
                if(!*sync) {
                    *sync = true;
                    another = anotherEvaluator(*this);
                    *sync = false;
                }
            });
        }
        return result;
    }

    StorageType& EditSilent() { return m_value; }
    const StorageType& Native() const { return m_value; }
    Dispatcher& GetDispatcher() { return OnChange; }

    bool operator!() const { return m_value == false; }
    bool operator!=(const T& value) const { return m_value != value; }
    bool operator==(const T& value) const { return m_value == value; }
    LocalProperty& operator=(const T& value) { SetValue(value); return *this; }
    operator const T&() const { return m_value; }

    Dispatcher OnChange;

private:
    template<class T2> friend struct Serializer;
    mutable FAction m_subscribes;
};

using LocalPropertyDoubleEvaluator = std::function<double (double)>;

template<class T>
class LocalPropertyLimitedDecimal : public LocalProperty<T>
{
    using Super = LocalProperty<T>;
public:
    LocalPropertyLimitedDecimal(const T& value = 0, const T& min = (std::numeric_limits<T>::lowest)(), const T& max = (std::numeric_limits<T>::max)())
        : Super(::clamp(value, min, max))
        , m_min(min)
        , m_max(max)
    {}

    void SetMinMax(const T& min, const T& max)
    {
        if(!qFuzzyCompare((double)m_max,max) || !qFuzzyCompare((double)m_min, min)) {
            m_min = min;
            m_max = max;
            SetValue(Super::m_value);
            OnMinMaxChanged();
        }
    }

    void SetValue(const T& value)
    {
        auto validatedValue = Super::m_validator(value);
        validatedValue = applyMinMax(validatedValue);
        if(!qFuzzyCompare(double(validatedValue), double(Super::m_value))) {
            Super::m_setterHandler([validatedValue, this]{
                Super::m_value = validatedValue;
                Super::Invoke();
            });
        }
    }

    LocalPropertyLimitedDecimal& operator-=(const T& value) { SetValue(Super::Native() - value); return *this; }
    LocalPropertyLimitedDecimal& operator+=(const T& value) { SetValue(Super::Native() + value); return *this; }
    LocalPropertyLimitedDecimal& operator=(const T& value) { SetValue(value); return *this; }

    const T& GetMin() const { return m_min; }
    const T& GetMax() const { return m_max; }

    Dispatcher OnMinMaxChanged;

private:
    T applyMinMax(const T& value)
    {
        return ::clamp(value, m_min, m_max);
    }

private:
    template<class T2> friend struct Serializer;
    T m_min;
    T m_max;
};

using LocalPropertyInt = LocalPropertyLimitedDecimal<qint32>;
using LocalPropertyUInt = LocalPropertyLimitedDecimal<quint32>;
using LocalPropertyDouble = LocalPropertyLimitedDecimal<double>;
using LocalPropertyFloat = LocalPropertyLimitedDecimal<float>;

template<typename Enum>
class LocalPropertySequentialEnum : public LocalPropertyInt
{
    using Super = LocalPropertyInt;
public:
    LocalPropertySequentialEnum()
        : Super((qint32)Enum::First,(qint32)Enum::First,(qint32)Enum::Last)
    {}
    LocalPropertySequentialEnum(Enum initial)
        : Super((qint32)initial,(qint32)Enum::First,(qint32)Enum::Last)
    {}

    LocalPropertySequentialEnum& operator=(qint32 value) { SetValue(value); return *this; }
    LocalPropertySequentialEnum& operator=(Enum value) { return operator=((qint32)value); }
    bool operator==(Enum value) const { return Super::m_value == (qint32)value; }
    bool operator!=(Enum value) const { return Super::m_value != (qint32)value; }

    Enum Native() const { return (Enum)Super::m_value; }

    QStringList GetNames() { return EnumHelper<Enum>::GetNames(); }
};

template<class T>
class LocalPropertyPtr : public LocalProperty<T*>
{
    using Super = LocalProperty<T*>;
public:
    LocalPropertyPtr(T* initial)
        : Super(initial)
    {}

    bool operator!=(const T* another) const { return Super::m_value != another; }
    bool operator==(const T* another) const { return Super::m_value == another; }
    LocalPropertyPtr& operator=(T* value) { Super::SetValue(value); return *this; }
    operator const T*() const { return Super::m_value; }
    const T* operator->() const { return Super::m_value; }
    T* operator->() { return Super::m_value; }
};

template<class T>
class LocalPropertySharedPtr : public LocalProperty<SharedPointer<T>>
{
    using Super = LocalProperty<SharedPointer<T>>;
public:
    LocalPropertySharedPtr(T* initial = nullptr)
        : Super(SharedPointer<T>(initial))
    {}

    const T* get() const { return Super::m_value.get(); }
    T* get() { return Super::m_value.get(); }

    void SetValue(T* value)
    {
        if(value != Super::m_value.get()) {
            Super::m_setterHandler([value, this]{
                Super::m_value = value;
                Super::Invoke();
            });
        }
    }

    bool operator!=(const T* another) const { return Super::m_value.get() != another; }
    bool operator==(const T* another) const { return Super::m_value.get() == another; }
    LocalPropertySharedPtr& operator=(T* value) { SetValue(value); return *this; }
    LocalPropertySharedPtr& operator=(const SharedPointer<T>& value) { Super::SetValue(value); return *this; }
    operator const LocalPropertySharedPtr&() const { return Super::m_value; }
    const T* operator->() const { return Super::m_value.get(); }
    T* operator->() { return Super::m_value.get(); }
    T& operator*() { return *Super::m_value; }
    const T& operator*() const { return *Super::m_value; }
};

template<class T>
class LocalPropertySet : public LocalProperty<QSet<T>>
{
    using ContainerType = QSet<T>;
    typedef LocalProperty<QSet<T>> Super;
public:
    LocalPropertySet()
    {}
    LocalPropertySet(const QSet<T>& value)
        : Super(value)
    {}

    bool IsEmpty() const { return this->m_value.isEmpty(); }
    qint32 Size() const { return this->m_value.size(); }
    bool IsContains(const T& value) const { return this->m_value.contains(value); }

    void Clear()
    {
        if(!this->m_value.isEmpty()) {
            this->m_value.clear();
            this->Invoke();
        }
    }

    void Insert(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.insert(value);
            this->Invoke();
        }
    }

    void Remove(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.erase(find);
            this->Invoke();
        }
    }

    typename QSet<T>::const_iterator begin() const { return this->m_value.begin(); }
    typename QSet<T>::const_iterator end() const { return this->m_value.end(); }
};

template<class T>
class LocalPropertyVector : public LocalProperty<QVector<T>>
{
    using ContainerType = QVector<T>;
    using Super = LocalProperty<ContainerType>;
public:
    LocalPropertyVector()
    {}
    LocalPropertyVector(const ContainerType& value)
        : Super(value)
    {}

    bool IsEmpty() const { return this->m_value.isEmpty(); }
    qint32 Size() const { return this->m_value.size(); }
    bool IsContains(const T& value) const { return this->m_value.contains(value); }

    void Clear()
    {
        if(!this->m_value.isEmpty()) {
            this->m_value.clear();
            this->Invoke();
        }
    }

    void Append(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.append(value);
            this->Invoke();
        }
    }

    typename ContainerType::const_iterator begin() const { return this->m_value.begin(); }
    typename ContainerType::const_iterator end() const { return this->m_value.end(); }
};

#define DECLARE_LOCAL_PROPERTY_TYPE(propertyName, baseType) \
class propertyName : public baseType \
{ \
    using Super = baseType; \
public: \
    using Super::Super; \
    propertyName& operator=(const Super::value_type& value) { Super::SetValue(value); return *this; } \
};

DECLARE_LOCAL_PROPERTY_TYPE(LocalPropertyFilePath, LocalProperty<QString>)
using LocalPropertyBool = LocalProperty<bool>;
using LocalPropertyColor = LocalProperty<QColor>;
using LocalPropertyString = LocalProperty<QString>;

using PropertyFromLocalPropertyContainer = QVector<SharedPointer<Property>>;

struct PropertyFromLocalProperty
{
    template<typename Enum>
    static SharedPointer<Property> Create(const Name& name, LocalPropertySequentialEnum<Enum>& enumProperty);
    template<class T>
    static SharedPointer<Property> Create(const Name& name, T& localProperty);
    template<class T>
    inline static SharedPointer<Property> Create(const QString& name, T& localProperty) { return Create(Name(name), localProperty); }
};

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyFilePath& localProperty)
{
    return ::make_shared<ExternalFileNameProperty>(
                name,
                [&localProperty] { return localProperty.Native(); },
                [&localProperty](const QString& value, const QString&) { localProperty = value; }
    );
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<bool>& localProperty)
{
    return ::make_shared<ExternalBoolProperty>(
                name,
                [&localProperty] { return localProperty.Native(); },
                [&localProperty](double value, double) { localProperty = value; }
    );
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyLimitedDecimal<double>& localProperty)
{
    return ::make_shared<ExternalDoubleProperty>(
                name,
                [&localProperty] { return localProperty.Native(); },
                [&localProperty](double value, double) { localProperty = value; },
                localProperty.GetMin(),
                localProperty.GetMax()
    );
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyLimitedDecimal<float>& localProperty)
{
    return ::make_shared<ExternalFloatProperty>(
                name,
                [&localProperty] { return localProperty.Native(); },
                [&localProperty](double value, double) { localProperty = value; },
                localProperty.GetMin(),
                localProperty.GetMax()
    );
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<QColor>& localProperty)
{
    return ::make_shared<ExternalColorProperty>(
                name,
                [&localProperty] { return localProperty.Native(); },
                [&localProperty](const QColor& value, const QColor&) { localProperty = value; }
    );
}

template<typename Enum>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertySequentialEnum<Enum>& localProperty)
{
    auto result = ::make_shared<ExternalNamedUIntProperty>(
                name,
                [&localProperty]() -> quint32 { return localProperty; },
                [&localProperty](quint32 value, quint32) { localProperty = value; },
                localProperty.GetMin(),
                localProperty.GetMax()
    );
    result->SetNames(localProperty.GetNames());
    return std::move(result);
}

#endif // LOCALPROPERTY_H
