#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include <limits>
#include <optional>
#include "property.h"
#include "externalproperty.h"

namespace adapters {
template<typename ... Args>
void ResetThread(const Args&... args);

template<typename ... Args>
void SetThreadSafe(Args&... args);
}

template<class value_type>
struct LocalPropertyDescInitializationParams
{
    LocalPropertyDescInitializationParams(const std::optional<value_type>& initial = std::nullopt)
        : InitialValue(initial)
    {}

    LocalPropertyDescInitializationParams& SetMin(const value_type& value) { Min = value; return *this; }
    LocalPropertyDescInitializationParams& SetMax(const value_type& value) { Max = value; return *this; }
    LocalPropertyDescInitializationParams& SetMinMax(const value_type& min, const value_type& max) { Min = min; Max = max; return *this; }

    std::optional<value_type> InitialValue;
    std::optional<value_type> Min;
    std::optional<value_type> Max;
};

using LocalPropertyDoubleParams = LocalPropertyDescInitializationParams<double>;
using LocalPropertyIntParams = LocalPropertyDescInitializationParams<qint32>;

DispatcherConnections LocalPropertiesConnectBoth(const char* debugLocation, const QVector<Dispatcher*>& dispatchers1, const FAction& evaluator1, const QVector<Dispatcher*>& dispatchers2, const FAction& evaluator2);

class LocalPropertyBool;

template<class T>
class LocalPropertyValidatorBuilder
{
public:
    using value_type = typename T::value_type;
    using FValidator = std::function<void (T*, value_type&)>;
    LocalPropertyValidatorBuilder()
    {}

    LocalPropertyValidatorBuilder& Add(const FValidator& validator) {
        m_validators.append(validator);
        return *this;
    }

    LocalPropertyValidatorBuilder& SetEpsilonEquality(const value_type& eps) {
        m_validators.append([eps](T* property, value_type& value){
            value = fuzzyCompare(property->Native(), value, eps) ? property->Native() : value;
        });
        return *this;
    }

    const LocalPropertyValidatorBuilder& Install(T* property) const
    {
        Q_ASSERT(!m_validators.isEmpty());
        auto validators = m_validators;
        property->SetValidator([property, validators](const value_type& value) -> value_type {
            auto validated = value;
            for(const auto& validator : validators) {
                validator(property, validated);
            }
            return validated;
        });
        return *this;
    }

private:
    QVector<FValidator> m_validators;
};

template<class T>
class LocalProperty
{
    using FSetter = std::function<void ()>;
    using FSetterHandler = std::function<void (const FSetter&)>;

public:
    using FValidator = std::function<T (const T&)>;
    using value_type = T;

protected:
    T m_value;
    FSetterHandler m_setterHandler;
    FValidator m_validator;

public:
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
    virtual ~LocalProperty()
    {

    }

    bool IsValueValid() const { return true; }

    void Invoke()
    {
        m_setterHandler([this]{
            if(m_subscribes != nullptr) {
                m_subscribes();
            }
            OnChanged.Invoke();
        });
    }

    bool Change(const std::function<bool (T& type)>& changeHandler)
    {
        if(changeHandler(m_value)) {
            Invoke();
            return true;
        }
        return false;
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

    void Subscribe(const std::function<void (const T&)>& handler) const
    {
        Subscribe([this, handler]{
            handler(*this);
        });
    }

    void SetAndSubscribe(const std::function<void (const T&)>& handler) const
    {
        Subscribe(handler);
        handler(*this);
    }

    static LocalPropertyValidatorBuilder<LocalProperty<T>> CreateValidatorBuilder()
    {
        return LocalPropertyValidatorBuilder<LocalProperty<T>>();
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
        m_setterHandler([value, this]{
            auto validatedValue = m_validator(value);
            validate(validatedValue);
            if(NotEqual(validatedValue, m_value)) {
                m_value = validatedValue;
                Invoke();
            }
        });
    }

    bool TrySetValue(const T& value)
    {
        auto validatedValue = m_validator(value);
        validate(validatedValue);
        if(NotEqual(validatedValue, m_value)) {
            m_setterHandler([validatedValue, this]{
                m_value = validatedValue;
                Invoke();
            });
            return true;
        }
        return false;
    }

    void Set(const T& value, bool silent)
    {
        if(silent) {
            SetSilentWithValidators(value);
        } else {
            SetValue(value);
        }
    }

    void SetSilentWithValidators(const T& value)
    {
        auto validatedValue = m_validator(value);
        validate(validatedValue);
        m_value = validatedValue;
    }

    void SetValueForceInvoke(const T& value)
    {
        m_setterHandler([value, this]{
            auto validatedValue = m_validator(value);
            validate(validatedValue);
            m_value = validatedValue;
            Invoke();
        });
    }

    template<typename ... Dispatchers>
    DispatcherConnections Connect(const char* connectionInfo, const std::function<void (const T&)>& handler, Dispatchers&... dispatchers) const
    {
        return OnChanged.ConnectCombined(connectionInfo, [handler, this]{ handler(*this); }, dispatchers...);
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCall(const char* connectionInfo, const std::function<void (const T&)>& handler, Dispatchers&... dispatchers) const
    {
        return OnChanged.ConnectAndCallCombined(connectionInfo, [handler, this]{ handler(*this); }, dispatchers...);
    }

    template<typename ... Args, typename First, typename Function>
    DispatcherConnections ConnectFrom(const char* locationInfo, const Function& handler, const First& first, const Args&... args)
    {
        DispatcherConnections connections;
        auto update = [locationInfo, this, handler, &first, &args...]{
            this->operator=(handler(first.Native(), args.Native()...));
        };
        adapters::Combine([&](const auto& property){
            connections += property.ConnectAction(locationInfo, update);
        }, first, args...);
        update();
        return connections;
    }

    template<typename... Args, typename First, typename... Dispatchers>
    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<T ()>& thisEvaluator, const First& first, Dispatchers&... dispatchers)
    {
        DispatcherConnections result;
        *this = thisEvaluator();
        auto onChange = [this, thisEvaluator, locationInfo]{
            *this = thisEvaluator();
        };
        adapters::Combine([&](const auto& property){
            result += property.ConnectAction(locationInfo, onChange);
        }, first, dispatchers...);
        return result;
    }

    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<T ()>& thisEvaluator, const QVector<Dispatcher*>& dispatchers)
    {
        DispatcherConnections result;
        *this = thisEvaluator();
        auto onChange = [this, thisEvaluator, locationInfo]{
            *this = thisEvaluator();
        };
        for(auto* dispatcher : dispatchers) {
            result += dispatcher->Connect(locationInfo, onChange);
        }
        return result;
    }

    DispatcherConnection ConnectFrom(const char* locationInfo, const LocalProperty& another)
    {
        *this = another.Native();
        return another.OnChanged.Connect(locationInfo, [this, &another, locationInfo]{
            *this = another.Native();
        });
    }

    template<class Property>
    DispatcherConnection ConnectTo(const char* locationInfo, Property& another)
    {
        another = Native();
        return OnChanged.Connect(locationInfo, [this, &another, locationInfo]{
            another = Native();
        });
    }

    template<class T2, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>, typename... Dispatchers>
    DispatcherConnections ConnectTo(const char* locationInfo, LocalProperty<T2>& another, const Evaluator& anotherEvaluator, Dispatchers&... dispatchers)
    {
        return OnChanged.ConnectAndCallCombined(locationInfo, [this, anotherEvaluator, &another, locationInfo] {
            another = anotherEvaluator(Native());
        }, dispatchers...);
    }

    template<class Property, typename T2 = typename Property::value_type, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>, typename... Dispatchers>
    DispatcherConnections ConnectBoth(const char* locationInfo, Property& another, const Evaluator& anotherEvaluator = [](const T& v) { return v; }, const ThisEvaluator& thisEvaluator = [](const T2& v) { return v; }, Dispatchers&... dispatchers)
    {
        DispatcherConnections result;
        another = anotherEvaluator(Native());
        auto sync = ::make_shared<std::atomic_bool>(false);
        result += another.OnChanged.Connect(locationInfo, [this, thisEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                *this = thisEvaluator(another);
                *sync = false;
            }
        });
        result += OnChanged.ConnectCombined(locationInfo, [this, anotherEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                another = anotherEvaluator(*this);
                *sync = false;
            }
        }, dispatchers...);
        return result;
    }

    T& EditSilent() { return m_value; }
    const T& Native() const { return m_value; }
    Dispatcher& GetDispatcher() const { return OnChanged; }

    void SetSilent(const T& another)
    {
        m_value = another;
    }

    DispatcherConnection ConnectAction(const char* locationInfo, const FAction& action) const
    {
        return OnChanged.Connect(locationInfo, action);
    }

    bool operator<(const T& value) const { return m_value < value; }
    bool operator!() const { return m_value == false; }
    bool operator!=(const T& value) const { return m_value != value; }
    bool operator==(const T& value) const { return m_value == value; }
    LocalProperty& operator=(const T& value) { SetValue(value); return *this; }
    operator const T&() const { return m_value; }

    mutable Dispatcher OnChanged;

protected:
    virtual void validate(T&) const {}

private:
    template<class T2> friend struct Serializer;
    template<class T2> friend struct SerializerXml;
    mutable FAction m_subscribes;
};

using LocalPropertyDoubleEvaluator = std::function<double (double)>;

template<class T>
class LocalPropertyLimitedDecimal : public LocalProperty<T>
{
    using Super = LocalProperty<T>;
public:
    LocalPropertyLimitedDecimal(const LocalPropertyDescInitializationParams<T>& params)
        : LocalPropertyLimitedDecimal(params.InitialValue.has_value() ? params.InitialValue.value() : 0,
                                      params.Min.has_value() ? params.Min.value() : (std::numeric_limits<T>::lowest)(),
                                      params.Max.has_value() ? params.Max.value() : (std::numeric_limits<T>::max)())
    {}
    LocalPropertyLimitedDecimal(const T& value = 0, const T& min = (std::numeric_limits<T>::lowest)(), const T& max = (std::numeric_limits<T>::max)())
        : Super(::clamp(value, min, max))
        , m_min(min)
        , m_max(max)
    {}

    void SetMinMax(const T& min, const T& max)
    {
        if(NotEqual(m_max, max) || NotEqual(m_min, min)) {
            m_min = min;
            m_max = max;
            OnMinMaxChanged();
            Super::SetValue(Super::m_value);
        }
    }

    LocalPropertyLimitedDecimal& FlagRemove(const T& flags)
    {
        T result = Super::Native();
        *this = FlagsHelpers<T>::Add(result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& FlagAdd(const T& flags)
    {
        T result = Super::Native();
        *this = FlagsHelpers<T>::Remove(result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& FlagChange(bool add, const T& flags)
    {
        T result = Super::Native();
        *this = FlagsHelpers<T>::ChangeFromBoolean(add, result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& operator-=(const T& value) { Super::SetValue(Super::Native() - value); return *this; }
    LocalPropertyLimitedDecimal& operator+=(const T& value) { Super::SetValue(Super::Native() + value); return *this; }
    LocalPropertyLimitedDecimal& operator/=(const T& value) { Super::SetValue(Super::Native() / value); return *this; }
    LocalPropertyLimitedDecimal& operator*=(const T& value) { Super::SetValue(Super::Native() * value); return *this; }
    LocalPropertyLimitedDecimal& operator=(const T& value) { Super::SetValue(value); return *this; }

    typename T operator&(const T& value) const { return Super::m_value & value; }
    typename T operator^(const T& value) const { return Super::m_value ^ value; }
    typename T operator|(const T& value) const { return Super::m_value | value; }
    typename T operator~() const { return ~Super::m_value; }

    const T& GetMin() const { return m_min; }
    const T& GetMax() const { return m_max; }

    Dispatcher OnMinMaxChanged;

private:
    T applyMinMax(const T& value) const
    {
        return ::clamp(value, m_min, m_max);
    }
    void validate(T& value) const override
    {
        value = applyMinMax(value);
    }

private:
    template<class T2> friend struct Serializer;
    T m_min;
    T m_max;
};

using LocalPropertyInt64 = LocalPropertyLimitedDecimal<qint64>;
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
        : LocalPropertySequentialEnum(Enum::First)
    {}
    LocalPropertySequentialEnum(Enum initial)
        : Super((qint32)initial,(qint32)Enum::First,(qint32)Enum::Last)
    {}

    void SetSilentWithValidators(const Enum& value)
    {
        Super::SetSilentWithValidators((qint32)value);
    }

    void SetSilentWithValidators(qint32 value)
    {
        Super::SetSilentWithValidators(value);
    }

    void SetSilent(const value_type& value)
    {
        Super::SetSilent(value);
    }

    void SetSilent(const Enum& another)
    {
        Super::m_value = (qint32)another;
    }

    FAction SetterFromString(const QString& value);
    LocalPropertySequentialEnum& operator=(const QString& value)
    {
        auto setter = SetterFromString(value);
        if(setter != nullptr) {
            setter();
        }
        return *this;
    }
    LocalPropertySequentialEnum& operator=(qint32 value) { SetValue(value); return *this; }
    LocalPropertySequentialEnum& operator=(Enum value) { return operator=((qint32)value); }
    bool operator==(qint32 value) const { return Super::m_value == value; }
    bool operator==(Enum value) const { return Super::m_value == (qint32)value; }
    bool operator!=(Enum value) const { return Super::m_value != (qint32)value; }

    Enum Native() const { return (Enum)Super::m_value; }
    int Value() const { return Super::m_value; }

    QString GetName() const
    {
        Q_ASSERT(0<= Super::m_value && Super::m_value < GetNames().size());
        return GetNames().at(Super::m_value);
    }

    QStringList GetNames() const;
};

template<class T>
class LocalPropertyPtr : public LocalProperty<T*>
{
    using Super = LocalProperty<T*>;
public:
    LocalPropertyPtr(T* initial = nullptr)
        : Super(initial)
    {}

    bool operator!=(T* another) const { return Super::m_value != another; }
    bool operator==(T* another) const { return Super::m_value == another; }
    LocalPropertyPtr& operator=(T* value) { Super::SetValue(value); return *this; }
    operator const T*() const { return Super::m_value; }
    const T* operator->() const { return Super::m_value; }
    T* operator->() { return Super::m_value; }
};

template<class T> class LocalPropertySharedPtrDispatcherHelper;

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

    bool operator!=(const SharedPointer<T>& another) const { return Super::m_value != another; }
    bool operator==(const SharedPointer<T>& another) const { return Super::m_value == another; }
    bool operator!=(const T* another) const { return Super::m_value.get() != another; }
    bool operator==(const T* another) const { return Super::m_value.get() == another; }
    LocalPropertySharedPtr& operator=(T* value) { SetValue(value); return *this; }
    LocalPropertySharedPtr& operator=(const SharedPointer<T>& value) { Super::SetValue(value); return *this; }
    operator const LocalPropertySharedPtr&() const { return Super::m_value; }
    const T* operator->() const { return Super::m_value.get(); }
    T* operator->() { return Super::m_value.get(); }
    T& operator*() { return *Super::m_value; }
    const T& operator*() const { return *Super::m_value; }

    using FOnValidHandler = std::function<bool (const T&)>;
    using FOnInvalidHandler = FAction;
    using FCreateDispatcherHandler = std::function<void (const T&, Dispatcher*, DispatcherConnectionsSafe&)>;

    struct CreateDispatcherParams
    {
        DelayedCallObjectParams DelayedCallParams;
        const char* ConnectionInfo;
        FCreateDispatcherHandler Handler;

        CreateDispatcherParams(const char* connectionInfo, LocalPropertySharedPtrDispatcherHelper<T>* helper)
            : DelayedCallParams(-1, ThreadHandlerNoCheckMainLowPriority)
            , ConnectionInfo(connectionInfo)
            , Handler([](const T&, Dispatcher*, DispatcherConnectionsSafe&){})
            , m_helper(helper)
        {}

        CreateDispatcherParams& SetCommutator(const DelayedCallObjectParams& params)
        {
            DelayedCallParams = params;
            return *this;
        }

        CreateDispatcherParams& SetHandler(const FCreateDispatcherHandler& handler)
        {
            Handler = handler;
            return *this;
        }

        CreateDispatcherParams& SetOnChangedHandler()
        {
            Handler = [this](const T& value, Dispatcher* dispatcher, DispatcherConnectionsSafe& connections) {
                dispatcher->ConnectFrom(CONNECTION_DEBUG_LOCATION, value.OnChanged).MakeSafe(connections);
            };
            return *this;
        }

        template<typename ... Dispatchers>
        DispatcherConnections ConnectAndCall(const FOnValidHandler& handler, const FOnInvalidHandler& reset, Dispatchers&... dispatchers) const
        {
            return m_helper->ConnectAndCall(*this, handler, reset, dispatchers...);
        }

        SharedPointer<Dispatcher> CreateDispatcher() const
        {
            return m_helper->CreateDispatcher(*this);
        }

    private:
        LocalPropertySharedPtrDispatcherHelper<T>* m_helper;
    };

    CreateDispatcherParams DispatcherParams(const char* connectionInfo)
    {
        if(m_dispatcher == nullptr) {
            m_dispatcher = ::make_scoped<LocalPropertySharedPtrDispatcherHelper<T>>(this);
        }

        return CreateDispatcherParams(connectionInfo, m_dispatcher.get());
    }

    CreateDispatcherParams DispatcherParamsOnChanged(const char* connectionInfo)
    {
        return DispatcherParams(connectionInfo).SetOnChangedHandler();
    }

private:
    ScopedPointer<LocalPropertySharedPtrDispatcherHelper<T>> m_dispatcher;
};

template<class T>
class LocalPropertySharedPtrDispatcherHelper
{
public:
    using FOnValidHandler = std::function<bool (const T&)>;
    using FOnInvalidHandler = FAction;
    using FCreateDispatcherHandler = std::function<void (const T&, Dispatcher*, DispatcherConnectionsSafe&)>;
    using CreateDispatcherParams = typename LocalPropertySharedPtr<T>::CreateDispatcherParams;

    LocalPropertySharedPtrDispatcherHelper(LocalPropertySharedPtr<T>* property)
        : m_property(property)
    {}

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCall(const CreateDispatcherParams& params, const FOnValidHandler& handler, const FOnInvalidHandler& reset, Dispatchers&... dispatchers) const
    {
        auto dispatcher = CreateDispatcher(params);
        return { dispatcher->ConnectAndCallCombined(params.ConnectionInfo, [this, dispatcher, handler, reset]{
            if(*m_property == nullptr) {
                reset();
                return;
            }
            if(!handler(**m_property)) {
                reset();
            }
        }, dispatchers...)};
    }

    SharedPointer<Dispatcher> CreateDispatcher(const CreateDispatcherParams& params) const
    {
        SharedPointer<Dispatcher> result;
        DispatcherConnectionsSafe* resultConnections;
        if(params.DelayedCallParams.DelayMsecs == -1) {
            auto cr = ::make_shared<WithDispatcherConnectionsSafe<Dispatcher>>();
            result = cr;
            resultConnections = &cr->Connections;
        } else {
            auto cr = ::make_shared<WithDispatcherConnectionsSafe<DispatchersCommutator>>(params.DelayedCallParams);
            result = cr;
            resultConnections = &cr->Connections;
        }
        auto* pResult = result.get();
        auto connections = DispatcherConnectionsSafeCreate();
        m_property->OnChanged.ConnectAndCallCombined(params.ConnectionInfo, [this, pResult, params, connections]{
            connections->clear();
            if(*m_property != nullptr) {
                params.Handler(**m_property, pResult, *connections);
            }
            pResult->Invoke();
        }).MakeSafe(*resultConnections);
        return result;
    }

private:
    LocalPropertySharedPtr<T>* m_property;
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
    typename QSet<T>::const_iterator Find(const T& value) const { return this->m_value.find(value); }//TODO

    LocalPropertySet& operator=(const QSet<T>& another)
    {
        if(IsEmpty() && another.isEmpty()) {
            return *this;
        }

        this->m_value = another;
        this->Invoke();
        return *this;
    }

    void Clear()
    {
        if(!this->m_value.isEmpty()) {
            this->m_value.clear();
            this->Invoke();
        }
    }

    bool Insert(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find == this->m_value.end()) {
            this->m_value.insert(value);
            this->Invoke();
            return true;
        }
        return false;
    }

    bool Remove(const T& value)
    {
        auto find = this->m_value.find(value);
        if(find != this->m_value.end()) {
            this->m_value.erase(find);
            this->Invoke();
            return true;
        }
        return false;
    }

    typename QSet<T>::const_iterator begin() const { return this->m_value.begin(); }
    typename QSet<T>::const_iterator end() const { return this->m_value.end(); }
};

class LocalPropertyBool : public LocalProperty<bool>
{
    using Super = LocalProperty<bool>;
public:
    LocalPropertyBool(bool state = false);
    LocalPropertyBool& operator=(bool value) { Super::SetValue(value); return *this; }
};

template<class DispatcherType>
class LocalPropertyIntCommutatorBase
{
public:
    enum Mode
    {
        And,
        Or
    };

    LocalPropertyIntCommutatorBase(Mode defaultState = Or, const DelayedCallObjectParams& params = DelayedCallObjectParams())
        : m_commutator(params)
    {
        if(defaultState == Or) {
            m_defaultState = 0;
            m_operation = [](qint32 v1, qint32 v2) { return v1 | v2; };
        } else {
            m_defaultState = 0xffffffff;
            m_operation = [](qint32 v1, qint32 v2) { return v1 & v2; };
        }
        m_commutator += { this, [this]{
            Update();
        }};
        adapters::ResetThread(m_commutator);
    }

    void Reset(bool invoke = true)
    {
        m_handlers.clear();
        if(invoke) {
            m_property = m_defaultState;
        }
    }
    void Update()
    {
        m_property = value();
    }
    template<typename Function, typename ... Args>
    DispatcherConnections ConnectFromProperties(const char* locationInfo, const Function& handler, const Args& ... args)
    {
        auto result = connectFromProperties(locationInfo, handler, args...);
        m_commutator.Invoke();
        return result;
    }
    template<typename ... Args>
    DispatcherConnections ConnectFrom(const char* locationInfo, const LocalPropertyInt& another, const Args& ... args) {
        auto result = connectFrom(locationInfo, another, args...);
        m_commutator.Invoke();
        return result;
    }
    DispatcherConnections ConnectFrom(const char* locationInfo, const QVector<const LocalPropertyInt*>& properties){
        auto result = connectFrom(locationInfo, properties);
        m_commutator.Invoke();
        return result;
    }
    template<typename ... Args>
    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<qint32()>& thisEvaluator, const Args&... args)
    {
        auto result = connectFromDispatchers(locationInfo, thisEvaluator, args...);
        m_commutator.Invoke();
        return result;
    }
    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<qint32()>& thisEvaluator, const QVector<Dispatcher*>& dispatchers);

    template<typename ... Dispatchers>
    DispatcherConnections Connect(const char* connectionInfo, const std::function<void (const bool&)>& handler, Dispatchers&... dispatchers) const
    {
        return m_property.Connect(connectionInfo, handler, dispatchers...);
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCall(const char* connectionInfo, const std::function<void (const bool&)>& handler, Dispatchers&... dispatchers) const
    {
        return m_property.ConnectAndCall(connectionInfo, handler, dispatchers...);
    }
    DispatcherConnection ConnectAction(const char* locationInfo, const FAction& action) const
    {
        return m_property.ConnectAction(locationInfo, action);
    }

    bool operator!() const { return !m_property; }
    //bool operator<(const bool& value) const { return Super::operator <(value); }
    bool operator!=(const bool& value) const { return m_property != value; }
    bool operator==(const bool& value) const { return m_property == value; }
    operator qint32() const { return m_property; }

    qint32 Native() const { return m_property; }
    operator const LocalPropertyInt&() const { return m_property; }
    operator const LocalProperty<qint32>&() const { return m_property; }

    const LocalPropertyInt& AsProperty() const { return m_property; }
    Flags<qint32, Mode> AsFlags() const { return Flags<qint32, Mode>(m_property); }

    QString ToString() const
    {
        QString result;
        for(const auto& handler : m_handlers) {
            Flags<qint32, Mode> f(handler());
            f.ToString(result);
            result += " ";
        }
        return result;
    }

protected:
    template<typename Function, typename ... Args>
    DispatcherConnections connectFromProperties(const char* locationInfo, const Function& handler, const Args& ... args)
    {
        DispatcherConnections result;
        m_handlers.append([this, locationInfo, handler, &args...]{
            return handler(args.Native()...);
        });
        adapters::Combine([&](const auto& property){
            result += m_commutator.ConnectFrom(locationInfo, property);
        }, args...);
        return result;
    }

    template<typename ... Args>
    DispatcherConnections connectFrom(const char* locationInfo, const LocalPropertyInt& another, const Args& ... args) {
        DispatcherConnections result;
        adapters::Combine([&](const LocalPropertyInt& p){
            result += connectFromProperties(locationInfo, [](qint32 i) { return i; }, p);
        }, another, args...);
        return result;
    }
    DispatcherConnections connectFrom(const char* locationInfo, const QVector<const LocalPropertyInt*>& properties){
        DispatcherConnections result;
        for(const auto* property : properties){
            result += connectFromProperties(locationInfo, [](qint32 i) { return i; }, *property);
        }
        return result;
    }
    template<typename ... Args>
    DispatcherConnections connectFromDispatchers(const char* locationInfo, const std::function<qint32()>& thisEvaluator, const Args&... args)
    {
        DispatcherConnections result;
        m_handlers.append(thisEvaluator);
        adapters::Combine([&](const auto& property){
            result += m_commutator.ConnectFrom(locationInfo, property);
        }, args...);
        return result;
    }

    qint32 value() const
    {
        qint32 result = m_defaultState;
        for(const auto& handler : ::make_const(m_handlers)) {
            result = m_operation(result, handler());
        }
        return result;
    }

protected:
    LocalPropertyInt m_property;
    DispatcherType m_commutator;
    QVector<std::function<qint32()>> m_handlers;
    qint32 m_defaultState;
    std::function<qint32 (qint32, qint32)> m_operation;
};

using LocalPropertyIntCommutator = LocalPropertyIntCommutatorBase<DispatchersCommutator>;

template<class DispatcherType>
class LocalPropertyBoolCommutatorBase
{
public:
    enum Mode
    {
        And,
        Or
    };

    LocalPropertyBoolCommutatorBase(Mode defaultState = Or, const DelayedCallObjectParams& params = DelayedCallObjectParams())
        : m_commutator(params)
        , m_defaultState(defaultState == Or ? false : true)
    {
        m_commutator += { this, [this]{
            Update();
        }};
        adapters::ResetThread(m_commutator);
    }

    void Reset(bool invoke = true)
    {
        m_handlers.clear();
        if(invoke) {
            m_property = m_defaultState;
        }
    }
    void Update()
    {
        m_property = value();
    }
    template<typename Function, typename ... Args>
    DispatcherConnections ConnectFromProperties(const char* locationInfo, const Function& handler, const Args& ... args)
    {
        auto result = connectFromProperties(locationInfo, handler, args...);
        m_commutator.Invoke();
        return result;
    }
    template<typename ... Args>
    DispatcherConnections ConnectFrom(const char* locationInfo, const LocalPropertyBool& another, const Args& ... args) {
        auto result = connectFrom(locationInfo, another, args...);
        m_commutator.Invoke();
        return result;
    }
    DispatcherConnections ConnectFrom(const char* locationInfo, const QVector<const LocalPropertyBool*>& properties){
        auto result = connectFrom(locationInfo, properties);
        m_commutator.Invoke();
        return result;
    }
    template<typename ... Args>
    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<bool()>& thisEvaluator, const Args&... args)
    {
        auto result = connectFromDispatchers(locationInfo, thisEvaluator, args...);
        m_commutator.Invoke();
        return result;
    }
    DispatcherConnections ConnectFromDispatchers(const char* locationInfo, const std::function<bool()>& thisEvaluator, const QVector<Dispatcher*>& dispatchers);

    template<typename ... Dispatchers>
    DispatcherConnections Connect(const char* connectionInfo, const std::function<void (const bool&)>& handler, Dispatchers&... dispatchers) const
    {
        return m_property.Connect(connectionInfo, handler, dispatchers...);
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCall(const char* connectionInfo, const std::function<void (const bool&)>& handler, Dispatchers&... dispatchers) const
    {
        return m_property.ConnectAndCall(connectionInfo, handler, dispatchers...);
    }
    DispatcherConnection ConnectAction(const char* locationInfo, const FAction& action) const
    {
        return m_property.ConnectAction(locationInfo, action);
    }

    bool operator!() const { return !m_property; }
    //bool operator<(const bool& value) const { return Super::operator <(value); }
    bool operator!=(const bool& value) const { return m_property != value; }
    bool operator==(const bool& value) const { return m_property == value; }
    operator bool() const { return m_property; }

    bool Native() const { return m_property; }
    operator const LocalPropertyBool&() const { return m_property; }
    operator const LocalProperty<bool>&() const { return m_property; }

    const LocalPropertyBool& AsProperty() const { return m_property; }

    QString ToString() const
    {
        QString result;
        for(const auto& handler : m_handlers) {
            result += handler() ? "true " : "false ";
        }
        return result;
    }

protected:
    template<typename Function, typename ... Args>
    DispatcherConnections connectFromProperties(const char* locationInfo, const Function& handler, const Args& ... args)
    {
        DispatcherConnections result;
        m_handlers.append([this, locationInfo, handler, &args...]{
            return handler(args.Native()...);
        });
        adapters::Combine([&](const auto& property){
            result += m_commutator.ConnectFrom(locationInfo, property);
        }, args...);
        return result;
    }

    template<typename ... Args>
    DispatcherConnections connectFrom(const char* locationInfo, const LocalPropertyBool& another, const Args& ... args) {
        DispatcherConnections result;
        adapters::Combine([&](const LocalPropertyBool& p){
            result += connectFromProperties(locationInfo, FDirectBool, p);
        }, another, args...);
        return result;
    }
    DispatcherConnections connectFrom(const char* locationInfo, const QVector<const LocalPropertyBool*>& properties){
        DispatcherConnections result;
        for(const auto* property : properties){
            result += connectFromProperties(locationInfo, FDirectBool, *property);
        }
        return result;
    }
    template<typename ... Args>
    DispatcherConnections connectFromDispatchers(const char* locationInfo, const std::function<bool()>& thisEvaluator, const Args&... args)
    {
        DispatcherConnections result;
        m_handlers.append(thisEvaluator);
        adapters::Combine([&](const auto& property){
            result += m_commutator.ConnectFrom(locationInfo, property);
        }, args...);
        return result;
    }

    bool value() const
    {
        bool result = m_defaultState;
        bool oppositeState = !result;
        for(const auto& handler : ::make_const(m_handlers)) {
            if(handler() == oppositeState) {
                result = oppositeState;
                break;
            }
        }
        return result;
    }

protected:
    LocalPropertyBool m_property;
    DispatcherType m_commutator;
    QVector<std::function<bool()>> m_handlers;
    bool m_defaultState;
};

using LocalPropertyBoolCommutator = LocalPropertyBoolCommutatorBase<DispatchersCommutator>;

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
        this->m_value.append(value);
        this->Invoke();
    }

    LocalPropertyVector& operator=(const QVector<T>& another)
    {
        if(IsEmpty() && another.isEmpty()) {
            return *this;
        }

        this->m_value = another;
        this->Invoke();
        return *this;
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
using LocalPropertyColor = LocalProperty<QColor>;
using LocalPropertyString = LocalProperty<QString>;
using LocalPropertyLocale = LocalProperty<QLocale>;
using LocalPropertyName = LocalProperty<Name>;

Q_DECLARE_METATYPE(SharedPointer<LocalPropertyBool>)

#include <QDateTime>
class LocalPropertyDate : public LocalProperty<QDate>
{
    using Super = LocalProperty<QDate>;
public:
    LocalPropertyDate(const QDate& value = QDate(), const QDate& min = QDate(), const QDate& max = QDate());

    void SetMinMax(const QDate& min, const QDate& max);

    LocalPropertyDate& operator-=(const QDate& value) { SetValue(QDate::fromJulianDay(Super::Native().toJulianDay() - value.toJulianDay())); return *this; }
    LocalPropertyDate& operator+=(const QDate& value) { SetValue(QDate::fromJulianDay(Super::Native().toJulianDay() - value.toJulianDay())); return *this; }
    LocalPropertyDate& operator=(const QDate& value) { SetValue(value); return *this; }

    const QDate& GetMin() const { return m_min; }
    const QDate& GetMax() const { return m_max; }

    QDate GetMinValid() const { return ValidatedMin(m_min); }
    QDate GetMaxValid() const { return ValidatedMax(m_max); }

    static QDate ValidatedMin(const QDate& min) { return !min.isValid() ? QDate::currentDate().addYears(-1000) : min; }
    static QDate ValidatedMax(const QDate& max) { return !max.isValid() ? QDate::currentDate().addYears(1000) : max; }

    Dispatcher OnMinMaxChanged;

private:
    static QDate applyRange(const QDate& cur, const QDate& min, const QDate& max);

    QDate applyMinMax(const QDate& value) const;
    void validate(QDate& value) const override;

private:
    template<class T2> friend struct Serializer;
    QDate m_min;
    QDate m_max;
};
class LocalPropertyTime : public LocalProperty<QTime>
{
    using Super = LocalProperty<QTime>;
public:
    LocalPropertyTime(const QTime& value = QTime(), const QTime& min = QTime(), const QTime& max = QTime());

    void SetMinMax(const QTime& min, const QTime& max);

    LocalPropertyTime& operator-=(const QTime& value) { SetValue(Super::Native().addMSecs(-value.msecsSinceStartOfDay())); return *this; }
    LocalPropertyTime& operator+=(const QTime& value) { SetValue(Super::Native().addMSecs(value.msecsSinceStartOfDay())); return *this; }
    LocalPropertyTime& operator=(const QTime& value) { SetValue(value); return *this; }

    const QTime& GetMin() const { return m_min; }
    const QTime& GetMax() const { return m_max; }

    QTime GetMinValid() const { return ValidatedMin(m_min); }
    QTime GetMaxValid() const { return ValidatedMax(m_max); }

    static QTime ValidatedMin(const QTime& min) { return !min.isValid() ? QTime(0,0) : min; }
    static QTime ValidatedMax(const QTime& max) { return !max.isValid() ? QTime(23,59,59,999) : max; }

    Dispatcher OnMinMaxChanged;

private:
    static QTime applyRange(const QTime& cur, const QTime& min, const QTime& max);

    QTime applyMinMax(const QTime& value) const;
    void validate(QTime& value) const override;

private:
    template<class T2> friend struct Serializer;
    QTime m_min;
    QTime m_max;
};

class LocalPropertyDateTime : public LocalProperty<QDateTime>
{
    using Super = LocalProperty<QDateTime>;
public:
    LocalPropertyDateTime(const QDateTime& value = QDateTime(), const QDateTime& min = QDateTime(), const QDateTime& max = QDateTime());

    void SetMinMax(const QDateTime& min, const QDateTime& max);

    LocalPropertyDateTime& operator-=(const QDateTime& value) { SetValue(Super::Native().addMSecs(-value.toMSecsSinceEpoch())); return *this; }
    LocalPropertyDateTime& operator+=(const QDateTime& value) { SetValue(Super::Native().addMSecs(value.toMSecsSinceEpoch())); return *this; }
    LocalPropertyDateTime& operator=(const QDateTime& value) { SetValue(value); return *this; }
    operator const QDateTime& () const { return Super::m_value; }
    bool IsRealTime() const { return !m_value.isValid(); }

    const QDateTime& GetMin() const { return m_min; }
    const QDateTime& GetMax() const { return m_max; }

    QTime GetTime() const { return m_value.time(); }
    QDate GetDate() const { return m_value.date(); }
    QDateTime GetMinValid() const { return ValidatedMin(m_min); }
    QDateTime GetMaxValid() const { return ValidatedMax(m_max); }

    static QDateTime ValidatedMin(const QDateTime& min) { return !min.isValid() ? QDateTime::currentDateTime().addYears(-1000) : min; }
    static QDateTime ValidatedMax(const QDateTime& max) { return !max.isValid() ? QDateTime::currentDateTime().addYears(1000) : max; }

    Dispatcher OnMinMaxChanged;

private:
    static QDateTime applyRange(const QDateTime& cur, const QDateTime& min, const QDateTime& max);

    void validate(QDateTime& value) const override;

private:
    template<class T2> friend struct Serializer;
    QDateTime m_min;
    QDateTime m_max;
};


class LocalPropertyDateTimeRange
{
public:
    template<class DateOrTimeProperty>
    static DispatcherConnections Bind(DateOrTimeProperty* start, DateOrTimeProperty* end)
    {
        DispatcherConnections result;
        auto updateMinMax = [start, end]{
            end->SetMinMax(*start, end->GetMax());
            start->SetMinMax(start->GetMin(), *end);
        };

        result += start->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateMinMax);
        result += end->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateMinMax);

        updateMinMax();

        return result;
    }

    template<class Date>
    static bool IsSequential(const Date& dt1, const Date& dt2)
    {
        enum RangeFlags {
            Dt1IsOpenDate = 0x1,
            Dt2IsOpenDate = 0x2,

            RangeNothingKnown = Dt1IsOpenDate | Dt2IsOpenDate,
            RangeStartIsKnown = Dt2IsOpenDate,
            RangeEndIsKnown = Dt1IsOpenDate,
            RangeAllIsKnown = 0x0
        };
        auto flags = 0;
        flags |= !dt1.isValid() ? Dt1IsOpenDate : 0;
        flags |= !dt2.isValid() ? Dt2IsOpenDate : 0;
        switch(flags)
        {
        case RangeEndIsKnown:
        case RangeNothingKnown: return false;
        case RangeStartIsKnown: return true;
        case RangeAllIsKnown: return dt1 <= dt2;
        default: Q_ASSERT(false); break;
        }
        return false;
    }

    template<typename DateOrTime>
    static bool IsInRange(const DateOrTime& dt, const DateOrTime& start, const DateOrTime& end)
    {
        enum RangeFlags {
            StartIsOpenDate = 0x1,
            EndIsOpenDate = 0x2,

            RangeOpenStart = StartIsOpenDate,
            RangeFullyOpened = StartIsOpenDate | EndIsOpenDate,
            RangeOpenEnd = EndIsOpenDate,
            RangeFullyClosed = 0x0
        };
        auto flags = 0;
        flags |= start.isValid() ? 0 : StartIsOpenDate;
        flags |= end.isValid() ? 0 : EndIsOpenDate;
        switch (flags) {
        case RangeFullyOpened: return true;
        case RangeOpenStart: return !dt.isValid() ? true : dt <= end;
        case RangeOpenEnd: return !dt.isValid() ? true : dt >= start;
        case RangeFullyClosed: return !dt.isValid() ? false : (dt >= start && dt <= end);
        default: Q_ASSERT(false); break;
        }
        return false;
    }

    template<class Date>
    static bool IsSequential(const Date& dt1, const Date& dt2, const Date& dt3)
    {
        enum RangeFlags {
            Dt1IsOpenDate = 0x1,
            Dt2IsOpenDate = 0x2,
            Dt3IsOpenDate = 0x4,

            RangeNothingKnown = Dt1IsOpenDate | Dt2IsOpenDate | Dt3IsOpenDate,
            RangeMiddleKnown = Dt1IsOpenDate | Dt3IsOpenDate,
            RangeStartIsKnown = Dt2IsOpenDate | Dt3IsOpenDate,
            RangeEndIsKnown = Dt1IsOpenDate | Dt2IsOpenDate,
            RangeStartMiddleIsKnown = Dt3IsOpenDate,
            RangeStartEndIsKnown = Dt2IsOpenDate,
            RangeMiddleEndIsKnown = Dt1IsOpenDate,
            RangeAllIsKnown = 0x0
        };
        auto flags = 0;
        flags |= !dt1.isValid() ? Dt1IsOpenDate : 0;
        flags |= !dt2.isValid() ? Dt2IsOpenDate : 0;
        flags |= !dt3.isValid() ? Dt3IsOpenDate : 0;
        switch(flags)
        {
        case RangeNothingKnown:
        case RangeMiddleKnown:
        case RangeStartIsKnown:
        case RangeStartEndIsKnown:
        case RangeMiddleEndIsKnown:
        case RangeEndIsKnown: return false;
        case RangeStartMiddleIsKnown: return dt2 >= dt1;
        case RangeAllIsKnown: return dt1 <= dt2 && dt2 <= dt3;
        default: Q_ASSERT(false); break;
        }
        return false;
    }
};

using PropertyFromLocalPropertyContainer = QVector<SharedPointer<Property>>;

template<class Property, class Validator = typename Property::FValidator>
void LocalPropertySetFromVariant(Property& value, const QVariant&, const Validator& handler);

template<>
inline void LocalPropertySetFromVariant<LocalPropertyDouble>(LocalPropertyDouble& property, const QVariant& value, const LocalPropertyDouble::FValidator& handler)
{
    property = handler(value.toDouble());
}

template<>
inline void LocalPropertySetFromVariant<LocalPropertyInt>(LocalPropertyInt& property, const QVariant& value, const LocalPropertyInt::FValidator& handler)
{
    property = handler(value.toInt());
}

template<>
inline void LocalPropertySetFromVariant<LocalPropertyString>(LocalPropertyString& property, const QVariant& value, const LocalPropertyString::FValidator& handler)
{
    property = handler(value.toString());
}

namespace adapters {

inline void ForEachDispatcher(const LocalPropertyBoolCommutator& t2, const std::function<void (Dispatcher& d)>& handler)
{
    handler(const_cast<Dispatcher&>(t2.AsProperty().OnChanged));
}

inline void ForEachDispatcher(const LocalPropertyIntCommutator& t2, const std::function<void (Dispatcher& d)>& handler)
{
    handler(const_cast<Dispatcher&>(t2.AsProperty().OnChanged));
}

inline void ForEachDispatcher(const Dispatcher& t2, const std::function<void (Dispatcher& d)>& handler)
{
    handler(const_cast<Dispatcher&>(t2));
}

template<typename Function, typename ... Args>
void ForEachDispatcher(const CommonDispatcher<Args...>& t2, const Function& handler)
{
    handler(const_cast<CommonDispatcher<Args...>&>(t2));
}

template<class T2>
void ForEachDispatcher(const LocalProperty<T2>& t2, const std::function<void (Dispatcher& d)>& handler)
{
    handler(const_cast<Dispatcher&>(t2.OnChanged));
}

template<class T2>
void ForEachDispatcher(const LocalPropertyOptional<T2>& t2, const std::function<void (Dispatcher& d)>& handler)
{
    handler(const_cast<Dispatcher&>(t2.Value.OnChanged));
    handler(const_cast<Dispatcher&>(t2.IsValid.OnChanged));
}


template<typename ... Args>
void ResetThread(const Args&... args)
{
#ifdef QT_DEBUG
    adapters::Combine([](auto& p){
        ForEachDispatcher(p, [](auto& d){ d.ResetThread(); });
    }, args...);
#endif
}

template<typename ... Args>
void SetThreadSafe(Args&... args)
{
    adapters::Combine([](auto& p){
        ForEachDispatcher(p, [](auto& d){ d.SetAutoThreadSafe(); });
    }, args...);
}

}

template<class Property>
struct LocalPropertyOptional
{
    using value_type = typename Property::value_type;
    using InitParams = LocalPropertyDescInitializationParams<value_type>;
    using FValidator = typename Property::FValidator;

    Property Value;
    LocalPropertyBool IsValid;

    LocalPropertyOptional(const InitParams& params)
        : Value(params)
        , IsValid(params.InitialValue.has_value())
    {
        Value.Subscribe([this]{
            IsValid = true;
        });
    }

    LocalPropertyOptional()
        : IsValid(false)
    {
        Value.Subscribe([this]{
            IsValid = true;
        });
    }

    void SetSilent(const LocalPropertyOptional<Property>& another)
    {
        Value.EditSilent() = another.Value.Native();
        IsValid.EditSilent() = another.IsValid.Native();
    }

    void SetSilent(const value_type& value)
    {
        Value.EditSilent() = value;
        IsValid.EditSilent() = true;
    }

    void SetSilentWithValidators(const std::optional<value_type>& value)
    {
        if(value.has_value()) {
            Value.SetSilentWithValidators(value.value());
            IsValid.EditSilent() = true;
        } else {
            IsValid.EditSilent() = false;
        }
    }

    DispatcherConnections ConnectAction(const char* locationInfo, const FAction& action) const
    {
        DispatcherConnections connections;
        connections += Value.OnChanged.Connect(locationInfo, action);
        connections += IsValid.OnChanged.Connect(locationInfo, action);
        return connections;
    }

    DispatcherConnections Connect(const char* locationInfo, const std::function<void (const std::optional<value_type>&)>& action) const
    {
        DispatcherConnections connections;
        auto invoke = [action, this]{
            action(ToStdOptional());
        };
        connections += Value.OnChanged.Connect(locationInfo, invoke);
        connections += IsValid.OnChanged.Connect(locationInfo, invoke);
        return connections;
    }

    DispatcherConnections ConnectAndCall(const char* locationInfo, const std::function<void (const std::optional<value_type>&)>& action) const
    {
        action(ToStdOptional());
        return Connect(locationInfo, action);
    }

    template<class Another, typename T = typename Property::value_type, typename T2 = typename Another::value_type, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>, typename... Dispatchers>
    DispatcherConnections ConnectBoth(const char* locationInfo, Another& another, const Evaluator& anotherEvaluator = [](const T& v) { return v; }, const ThisEvaluator& thisEvaluator = [](const T2& v) { return v; }, Dispatchers&... dispatchers)
    {
        DispatcherConnections result;
        if(IsValid) {
            another.Value = anotherEvaluator(Value);
        }
        another.IsValid = IsValid.Native();
        auto sync = ::make_shared<std::atomic_bool>(false);

        auto setThis = [this, thisEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                if(another.IsValid) {
                    Value = thisEvaluator(another.Value);
                }
                IsValid = another.IsValid.Native();
                *sync = false;
            }
        };

        result += another.IsValid.OnChanged.ConnectCombined(locationInfo, setThis, another.Value);
        auto setAnother = [this, anotherEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                if(IsValid) {
                    another.Value = anotherEvaluator(Value);
                }
                another.IsValid = IsValid.Native();
                *sync = false;
            }
        };

        result += Value.OnChanged.ConnectCombined(locationInfo, setAnother, IsValid, dispatchers...);
        return result;
    }

    DispatcherConnections ConnectFrom(const char* locationInfo, const LocalPropertyOptional& another)
    {
        DispatcherConnections connections;
        connections += Value.ConnectFrom(locationInfo, another.Value);
        connections += IsValid.ConnectFrom(locationInfo, another.IsValid);
        return connections;
    }

    template<typename ... Args, typename Function>
    DispatcherConnections ConnectFrom(const char* locationInfo, const Function& handler, const Args&... args)
    {        
        DispatcherConnections connections;
        auto update = [locationInfo, this, handler, &args...]{
            this->operator=(handler(args.Native()...));
        };
        adapters::Combine([&](const auto& property){
            connections += property.ConnectAction(locationInfo, update);
        }, args...);
        update();
        return connections;
    }

    template<typename ... Args, typename Function>
    DispatcherConnections ConnectFromOptional(const char* locationInfo, const Function& handler, const Args&... args)
    {
        DispatcherConnections connections;
        auto update = [locationInfo, this, handler, &args...]{
            bool isValid = true;
            for(bool valid : {args.IsValueValid()...}) {
                if(!valid) {
                    isValid = false;
                    break;
                }
            }
            if(isValid) {
                Value = handler(args.Value.Native()...);
            }
            IsValid = isValid;
        };
        adapters::Combine([&](const auto& property){
            connections += property.ConnectAction(locationInfo, update);
        }, args...);
        update();
        return connections;
    }

    LocalPropertyOptional& operator=(const std::optional<value_type>& value)
    {
        if(value.has_value()) {
            Value = value.value();
        }
        IsValid = value.has_value();
        return *this;
    }
    LocalPropertyOptional& operator=(const value_type& value) { Value = value; IsValid = true; return *this; }
    void FromVariant(const QVariant& value, const FValidator& handler = [](const value_type& value) { return value; })
    {
        if(!value.isValid() || (value.type() == QVariant::String && value.toString().isEmpty())) {
            IsValid = false;
        } else {
            LocalPropertySetFromVariant(Value, value, handler);
            IsValid = true;
        }
    }
    bool IsValueValid() const { return IsValid; }
    value_type ValueOr(const value_type& defaultValue) const { return IsValid ? Value : defaultValue; }
    std::optional<value_type> Native() const { return ToStdOptional(); }
    std::optional<value_type> ToStdOptional() const { return IsValid ? std::make_optional(Value.Native()) : std::nullopt; }
    QVariant ToVariant() const { return IsValid ? QVariant(Value.Native()) : QVariant(); }
    QVariant ToVariant(const FValidator& unitsHandler) const { return IsValid ? QVariant(unitsHandler(Value.Native())) : QVariant(); }
    QVariant ToVariantUi(const std::function<QString (value_type)>& unitsHandler = [](value_type v){return QString::number(v); }) const { return IsValid ? QVariant(unitsHandler(Value.Native())) : QVariant(DASH); }
private:
    template<typename T>
    static void dispatcherExtractor(QVector<Dispatcher*>& dispatchers, T& property){
        dispatchers.append(&property.OnChanged);
    }
};

using LocalPropertyDoubleOptional = LocalPropertyOptional<LocalPropertyDouble>;
using LocalPropertyIntOptional = LocalPropertyOptional<LocalPropertyInt>;
using LocalPropertyStringOptional = LocalPropertyOptional<LocalPropertyString>;
using LocalPropertyDateOptional = LocalPropertyOptional<LocalPropertyDate>;

template<class Property>
class LocalPropertyPreviousValue
{
public:
    using value_type = typename Property::value_type;

    struct LocalPropertyPreviousValueData
    {
        value_type m_value;
        value_type m_currentValue;
        Property* m_target;
        DispatcherConnectionSafePtr m_connection;

        LocalPropertyPreviousValueData(Property* target)
            : m_target(target)
        {
            m_connection = target->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
                m_value = m_currentValue;
                m_currentValue = *m_target;
                OnChanged(m_currentValue, m_value);
            }).MakeSafe();
            m_currentValue = m_value = *m_target;
        }

        CommonDispatcher<const value_type& /*current*/, const value_type& /*previous*/> OnChanged;

        void Reset() { m_value = *m_target; }
        const value_type& GetValue() const { return m_value; }
    };

    LocalPropertyPreviousValue(Property* target)
        : m_data(::make_shared<LocalPropertyPreviousValueData>(target))
    {}

    const LocalPropertyPreviousValueData& GetData() const { return *m_data; }
    const value_type& Native() const { return m_data->GetValue(); }
    operator const value_type&() const { return m_data->GetValue(); }

private:
    SharedPointer<LocalPropertyPreviousValueData> m_data;
};

template<class Property>
LocalPropertyPreviousValue<Property> LocalPropertyPreviousValueCreate(Property* property)
{
    return LocalPropertyPreviousValue<Property>(property);
}

struct PropertyFromLocalProperty
{
    template<typename Enum>
    static SharedPointer<Property> Create(const Name& name, LocalPropertySequentialEnum<Enum>& enumProperty);
    template<class T>
    static SharedPointer<Property> Create(const Name& name, T& localProperty);
    template<class T>
    inline static SharedPointer<Property> Create(const QString& name, T& localProperty) { return Create(Name(name), localProperty); }

    template<class T>
    inline static SharedPointer<Property> CreatePointer(const Name& name, LocalPropertyPtr<T>& localProperty)
    {
        auto property = ::make_shared<PointerProperty<T>>(name, localProperty.Native());
        auto* pProperty = property.get();
        auto sync = ::make_shared<std::atomic_bool>(false);
        auto setProperty = [pProperty, &localProperty, sync]{
            if(!*sync) {
                *sync = true;
                *pProperty = localProperty;
                *sync = false;
            }
        };
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, setProperty).MakeSafe();
        pProperty->Subscribe([&localProperty, connection, pProperty, sync]{
            if(!*sync) {
                *sync = true;
                localProperty = *pProperty;
                *sync = false;
            }
        });
        return property;
    }

    template<class T>
    inline static SharedPointer<Property> CreatePointer(const Name& name, LocalPropertySharedPtr<T>& localProperty)
    {
        auto property = ::make_shared<SharedPointerProperty<T>>(name, localProperty.Native());
        auto* pProperty = property.get();
        LocalPropertiesConnectBoth(CONNECTION_DEBUG_LOCATION, { &localProperty.GetDispatcher() }, [&localProperty, pProperty]{
            *pProperty = localProperty;
        }, { &property->GetDispatcher() }, [pProperty, &localProperty]{
            localProperty = *pProperty;
        });
        return property;
    }

    template<class T>
    static SharedPointer<Property> CreateVariant(const Name& name, T& localProperty)
    {
        auto property = ::make_shared<VariantProperty<typename T::value_type>>(name, "");
        auto* pProperty = property.get();
        auto sync = ::make_shared<std::atomic_bool>(false);
        auto setProperty = [pProperty, &localProperty, sync]{
            if(!*sync) {
                *sync = true;
                *pProperty = localProperty;
                *sync = false;
            }
        };
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, setProperty).MakeSafe();
        pProperty->Subscribe([&localProperty, connection, pProperty, sync]{
            if(!*sync) {
                *sync = true;
                localProperty = *pProperty;
                *sync = false;
            }
        });
        setProperty();
        return property;
    }

private:
    template<class T, class T2>
    static void connectProperty(T* pProperty, T2& localProperty)
    {
        auto sync = ::make_shared<std::atomic_bool>(false);
        auto setProperty = [pProperty, &localProperty, sync]{
            if(!*sync) {
                *sync = true;
                pProperty->SetValue(localProperty.Native());
                *sync = false;
            }
        };
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, setProperty).MakeSafe();
        pProperty->Subscribe([&localProperty, connection, pProperty, sync]{
            if(!*sync) {
                *sync = true;
                localProperty = *pProperty;
                *sync = false;
            }
        });
    }
};

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyFilePath& localProperty)
{
    auto property = ::make_shared<FileNameProperty>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<QImage>& localProperty)
{
    auto property = ::make_shared<TProperty<QImage>>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyLimitedDecimal<int>& localProperty)
{
    auto property = ::make_shared<IntProperty>(name, localProperty.Native(), localProperty.GetMin(), localProperty.GetMax());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyString& localProperty)
{
    auto property = ::make_shared<StringProperty>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyBool& localProperty)
{
    auto property = ::make_shared<BoolProperty>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyLimitedDecimal<double>& localProperty)
{
    auto property = ::make_shared<DoubleProperty>(name, localProperty.Native(), localProperty.GetMin(), localProperty.GetMax());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalPropertyLimitedDecimal<float>& localProperty)
{
    auto property = ::make_shared<FloatProperty>(name, localProperty.Native(), localProperty.GetMin(), localProperty.GetMax());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}

#ifdef QT_GUI_LIB

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<QColor>& localProperty)
{
    auto property = ::make_shared<ColorProperty>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}
#endif

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

template<>
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<QLocale>& localProperty)
{
    auto property = ::make_shared<TProperty<QLocale>>(name, localProperty.Native());
    auto* pProperty = property.get();
    connectProperty(pProperty, localProperty);
    return property;
}




#endif // LOCALPROPERTY_H
