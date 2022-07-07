#ifndef LOCALPROPERTY_H
#define LOCALPROPERTY_H

#include <limits>
#include <optional>

#include "property.h"
#include "externalproperty.h"


template<class T>
inline bool LocalPropertyNotEqual(const T& v1, const T& v2) { return v1 != v2; }
inline bool LocalPropertyNotEqual(const double& v1, const double& v2) { return !qFuzzyCompare(v1, v2); }
inline bool LocalPropertyNotEqual(const float& v1, const float& v2) { return !qFuzzyCompare(v1, v2); }

template<class value_type>
struct LocalPropertyDescInitializationParams
{
    LocalPropertyDescInitializationParams(const std::optional<value_type>& initial = std::nullopt)
        : InitialValue(initial)
    {}

    LocalPropertyDescInitializationParams& SetMin(const value_type& value) { Min = value; return *this; }
    LocalPropertyDescInitializationParams& SetMax(const value_type& value) { Max = value; return *this; }

    std::optional<value_type> InitialValue;
    std::optional<value_type> Min;
    std::optional<value_type> Max;
};

using LocalPropertyDoubleParams = LocalPropertyDescInitializationParams<double>;
using LocalPropertyIntParams = LocalPropertyDescInitializationParams<qint32>;

DispatcherConnections LocalPropertiesConnectBoth(const char* debugLocation, const QVector<Dispatcher*>& dispatchers1, const FAction& evaluator1, const QVector<Dispatcher*>& dispatchers2, const FAction& evaluator2);

template<class T, class StorageType = T>
class LocalProperty
{
    using FSetter = std::function<void ()>;
    using FSetterHandler = std::function<void (const FSetter&)>;

public:
    using FValidator = std::function<T (const T&)>;
    using value_type = T;

protected:
    StorageType m_value;
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

    void Invoke()
    {
        m_setterHandler([this]{
            if(m_subscribes != nullptr) {
                m_subscribes();
            }
            OnChanged.Invoke();
        });
    }

    bool Change(const std::function<bool (StorageType& type)>& changeHandler)
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
        validate(validatedValue);
        if(LocalPropertyNotEqual(validatedValue, m_value)) {
            m_setterHandler([validatedValue, this]{
                m_value = validatedValue;
                Invoke();
            });
        }
    }

    void SetValueForceInvoke(const T& value)
    {
        auto validatedValue = m_validator(value);
        validate(validatedValue);
        m_setterHandler([validatedValue, this]{
            m_value = validatedValue;
            Invoke();
        });
    }

    DispatcherConnection Connect(const std::function<void (const T&)>& handler)
    {
        return OnChanged.Connect(this, [handler, this]{ handler(*this); });
    }

    DispatcherConnection ConnectAndCall(const std::function<void (const T&)>& handler)
    {
        return OnChanged.ConnectAndCall(this, [handler, this]{ handler(*this); });
    }

    DispatcherConnections ConnectFromResetToIf(const char* connectionInfo, const T& resetTo, const T& expectedValue, const LocalProperty<bool>& condition)
    {
        return ConnectFrom(connectionInfo, condition, [this, expectedValue, resetTo](bool ok){
            if(Native() == expectedValue && !ok) {
                return resetTo;
            }
            return Native();
        });
    }

    template<class T2, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>>
    DispatcherConnections ConnectFrom(const char* locationInfo, const LocalProperty<T2>& another, const Evaluator& thisEvaluator, const QVector<Dispatcher*>& dispatchers = {})
    {
        DispatcherConnections result;
        auto& nonConst = const_cast<LocalProperty<T2>&>(another);
        *this = thisEvaluator(nonConst.Native());
        auto onChange = [this, thisEvaluator, &another, locationInfo]{
            *this = thisEvaluator(another.Native());
        };
        for(auto* dispatcher : dispatchers) {
            result += dispatcher->Connect(this, onChange);
        }
        result += nonConst.OnChanged.Connect(this, onChange);
        return result;
    }

    DispatcherConnections ConnectFrom(const char* locationInfo, const std::function<T ()>& thisEvaluator, const QVector<Dispatcher*>& dispatchers)
    {
        DispatcherConnections result;
        *this = thisEvaluator();
        auto onChange = [this, thisEvaluator, locationInfo]{
            *this = thisEvaluator();
        };
        for(auto* dispatcher : dispatchers) {
            result += dispatcher->Connect(this, onChange);
        }
        return result;
    }

    DispatcherConnection ConnectFrom(const char* locationInfo, const LocalProperty& another)
    {
        *this = another.Native();
        auto& nonConst = const_cast<LocalProperty&>(another);
        return nonConst.OnChanged.Connect(this, [this, &another, locationInfo]{
            *this = another.Native();
        });
    }

    template<class Property>
    DispatcherConnection ConnectTo(const char* locationInfo, Property& another)
    {
        another = Native();
        return OnChanged.Connect(this, [this, &another, locationInfo]{
            another = Native();
        });
    }

    template<class Property, typename T2 = typename Property::value_type, typename Evaluator = std::function<T2 (const T&)>, typename ThisEvaluator = std::function<T(const T2&)>>
    DispatcherConnections ConnectBoth(const char* locationInfo, Property& another, const Evaluator& anotherEvaluator = [](const T& v) { return v; }, const ThisEvaluator& thisEvaluator = [](const T2& v) { return v; }, const QVector<Dispatcher*>& dispatchers = {})
    {
        DispatcherConnections result;
        another = anotherEvaluator(Native());
        auto sync = ::make_shared<std::atomic_bool>(false);
        result += another.OnChanged.Connect(this, [this, thisEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                *this = thisEvaluator(another);
                *sync = false;
            }
        });
        result += OnChanged.Connect(this, [this, anotherEvaluator, &another, sync, locationInfo]{
            if(!*sync) {
                *sync = true;
                another = anotherEvaluator(*this);
                *sync = false;
            }
        });
        for(auto* dispatcher : dispatchers) {
            result += dispatcher->Connect(this, [this, anotherEvaluator, &another, sync, locationInfo]{
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
    Dispatcher& GetDispatcher() const { return OnChanged; }

    void SetFromSilent(const LocalProperty<T>& another)
    {
        m_value = another.m_value;
    }

    DispatcherConnections OnChangedImpl(const char* locationInfo, const FAction& action)
    {
        DispatcherConnections connections;
        connections += OnChanged.Connect(locationInfo, action);
        return connections;
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
        if(LocalPropertyNotEqual(m_max, max) || LocalPropertyNotEqual(m_min, min)) {
            m_min = min;
            m_max = max;
            Super::SetValue(Super::m_value);
            OnMinMaxChanged();
        }
    }

    LocalPropertyLimitedDecimal& FlagRemove(const T& flags)
    {
        T result = Native();
        *this = FlagsHelpers<T>::Add(result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& FlagAdd(const T& flags)
    {
        T result = Native();
        *this = FlagsHelpers<T>::Remove(result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& FlagChange(bool add, const T& flags)
    {
        T result = Native();
        *this = FlagsHelpers<T>::ChangeFromBoolean(add, result, flags);
        return *this;
    }

    LocalPropertyLimitedDecimal& operator-=(const T& value) { Super::SetValue(Super::Native() - value); return *this; }
    LocalPropertyLimitedDecimal& operator+=(const T& value) { Super::SetValue(Super::Native() + value); return *this; }
    LocalPropertyLimitedDecimal& operator=(const T& value) { Super::SetValue(value); return *this; }

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

    FAction SetterFromString(const QString& value)
    {
        auto index = TranslatorManager::GetInstance().GetEnumNames<Enum>().indexOf(value);
        if(TranslatorManager::GetInstance().IsValid<Enum>(index)) {
            SetValue(index);
            return [this, index]{ SetValue(index); };
        }
        return nullptr;
    }
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
    bool operator==(Enum value) const { return Super::m_value == (qint32)value; }
    bool operator!=(Enum value) const { return Super::m_value != (qint32)value; }

    Enum Native() const { return (Enum)Super::m_value; }

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

class LocalPropertyBoolCommutator : public LocalProperty<bool>
{
    using Super = LocalProperty<bool>;
public:
    LocalPropertyBoolCommutator(bool defaultState = false, qint32 msecs = 0, const ThreadHandlerNoThreadCheck& threadHandler = ThreadHandlerNoCheckMainLowPriority);

    void ClearProperties();
    void Update();
    DispatcherConnections AddProperties(const char* connectionInfo, const QVector<LocalProperty<bool>*>& properties);

private:
    DispatchersCommutator m_commutator;
    ThreadHandlerNoThreadCheck m_threadHandler;
    QVector<LocalProperty<bool>*> m_properties;
    bool m_defaultState;
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
using LocalPropertyBool = LocalProperty<bool>;
using LocalPropertyColor = LocalProperty<QColor>;
using LocalPropertyString = LocalProperty<QString>;
using LocalPropertyLocale = LocalProperty<QLocale>;

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

    QDate GetMinValid() const { return validatedMin(m_min); }
    QDate GetMaxValid() const { return validatedMax(m_max); }

    Dispatcher OnMinMaxChanged;

private:
    static QDate validatedMin(const QDate& min) { return !min.isValid() ? QDate::currentDate().addYears(-1000) : min; }
    static QDate validatedMax(const QDate& max) { return !max.isValid() ? QDate::currentDate().addYears(1000) : max; }

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

    QTime GetMinValid() const { return validatedMin(m_min); }
    QTime GetMaxValid() const { return validatedMax(m_max); }

    Dispatcher OnMinMaxChanged;

private:
    static QTime validatedMin(const QTime& min) { return !min.isValid() ? QTime(0,0) : min; }
    static QTime validatedMax(const QTime& max) { return !max.isValid() ? QTime(23,59,59,999) : max; }

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

    QDateTime GetMinValid() const { return validatedMin(m_min); }
    QDateTime GetMaxValid() const { return validatedMax(m_max); }

    Dispatcher OnMinMaxChanged;

private:
    static QDateTime validatedMin(const QDateTime& min) { return !min.isValid() ? QDateTime::currentDateTime().addYears(-1000) : min; }
    static QDateTime validatedMax(const QDateTime& max) { return !max.isValid() ? QDateTime::currentDateTime().addYears(1000) : max; }

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

        result += start->OnChanged.Connect(end, updateMinMax);
        result += end->OnChanged.Connect(end, updateMinMax);

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

    void SetFromSilent(const LocalPropertyOptional<Property>& another)
    {
        Value.EditSilent() = another.Value.Native();
        IsValid.EditSilent() = another.IsValid.Native();
    }

    DispatcherConnections OnChangedImpl(const char* locationInfo, const FAction& action)
    {
        DispatcherConnections connections;
        connections += Value.OnChanged.Connect(locationInfo, action);
        connections += IsValid.OnChanged.Connect(locationInfo, action);
        return connections;
    }

    DispatcherConnections ConnectFrom(const char* locationInfo, const LocalPropertyOptional& another)
    {
        DispatcherConnections connections;
        connections += Value.ConnectFrom(locationInfo, another.Value);
        connections += IsValid.ConnectFrom(locationInfo, another.IsValid);
        return connections;
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
    QVariant ToVariant() const { return IsValid ? QVariant(Value.Native()) : QVariant(); }
    QVariant ToVariant(const FValidator& unitsHandler) const { return IsValid ? QVariant(unitsHandler(Value.Native())) : QVariant(); }
    QVariant ToVariantUi(const std::function<QString (value_type)>& unitsHandler = [](value_type v){return QString::number(v); }) const { return IsValid ? QVariant(unitsHandler(Value.Native())) : QVariant("-"); }
};

using LocalPropertyDoubleOptional = LocalPropertyOptional<LocalPropertyDouble>;
using LocalPropertyIntOptional = LocalPropertyOptional<LocalPropertyInt>;
using LocalPropertyStringOptional = LocalPropertyOptional<LocalPropertyString>;

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
            m_connection = target->OnChanged.Connect(this, [this]{
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

    LocalPropertyPreviousValueData& GetData() { return *m_data; }
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

template<class T>
class LocalPropertySharedPtrDispatchers
{
public:
    using FHandler = std::function<void (T& value, DispatchersCommutator* connector, DispatcherConnectionsSafe& connections)>;

    struct LocalPropertySharedPtrDispatchersData
    {
    public:
        LocalPropertySharedPtrDispatchersData(LocalPropertySharedPtr<T>* property)
            : m_property(property)
        {
            property->OnChanged.Connect(this, [this, property]{
                for(const auto& pair : m_handlers) {
                    pair.first->Invoke();
                }
                m_dispatchersConnections.clear();
                if(*property != nullptr) {
                    for(auto& handler : m_handlers) {
                        connectHandler(handler.first, handler.second);
                    }
                    Q_ASSERT(!m_dispatchersConnections.isEmpty());
                }
            }).MakeSafe(m_connections);
        }

        SharedPointer<DispatchersCommutator> AddDispatcher(const FHandler& handler, qint32 msecs = 0, const ThreadHandlerNoThreadCheck& threadHandler = ThreadHandlerNoCheckMainLowPriority)
        {
            auto result = ::make_shared<DispatchersCommutator>(msecs, threadHandler);
            if(*m_property != nullptr) {
                connectHandler(result, handler);
                Q_ASSERT(!m_dispatchersConnections.isEmpty());
            }
            m_handlers.append(std::make_pair(result, handler));
            return result;
        }

    private:
        void connectHandler(SharedPointer<DispatchersCommutator>& dispatcher, const FHandler& handler)
        {
            handler(*m_property->Native(), dispatcher.get(), m_dispatchersConnections);
        }

    private:
        DispatcherConnectionsSafe m_connections;
        DispatcherConnectionsSafe m_dispatchersConnections;
        QVector<std::pair<SharedPointer<DispatchersCommutator>, FHandler>> m_handlers;
        LocalPropertySharedPtr<T>* m_property;
    };

    LocalPropertySharedPtrDispatchers(LocalPropertySharedPtr<T>* property)
        : m_data(::make_shared<LocalPropertySharedPtrDispatchersData>(property))
    {

    }

    LocalPropertySharedPtrDispatchersData& GetData() const { return *m_data; }

private:
    SharedPointer<LocalPropertySharedPtrDispatchersData> m_data;
};

template<class T>
LocalPropertySharedPtrDispatchers<T> LocalPropertySharedPtrDispatchersCreate(LocalPropertySharedPtr<T>* property)
{
    return LocalPropertySharedPtrDispatchers<T>(property);
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
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(nullptr, setProperty).MakeSafe();
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
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(nullptr, setProperty).MakeSafe();
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
        DispatcherConnectionSafePtr connection = localProperty.OnChanged.Connect(nullptr, setProperty).MakeSafe();
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
inline SharedPointer<Property> PropertyFromLocalProperty::Create(const Name& name, LocalProperty<bool>& localProperty)
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

template<typename ... Args> template<class T, class T2>
inline DispatcherConnection DelayedCallDispatchersCommutator<Args...>::Subscribe(const char* connectionInfo, LocalProperty<T, T2>& property)
{
    return Subscribe(connectionInfo, &property.OnChanged);
}

template<typename ... Args> template<class T>
inline DispatcherConnections DelayedCallDispatchersCommutator<Args...>::Subscribe(const char* connectionInfo, LocalPropertyOptional<T>& property)
{
    return Subscribe(connectionInfo, { &property.Value.OnChanged, &property.IsValid.OnChanged });
}

#endif // LOCALPROPERTY_H
