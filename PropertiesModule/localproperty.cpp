#include "localproperty.h"

DispatcherConnections LocalPropertiesConnectBoth(const char* debugLocation, const QVector<Dispatcher*>& dispatchers1, const FAction& evaluator1, const QVector<Dispatcher*>& dispatchers2, const FAction& evaluator2){
    DispatcherConnections result;
    auto sync = ::make_shared<std::atomic_bool>(false);
    auto eval1 = [debugLocation, evaluator1, sync]{
        if(!*sync) {
            *sync = true;
            evaluator1();
            *sync = false;
        }
    };
    auto eval2 = [debugLocation, evaluator2, sync]{
        if(!*sync) {
            *sync = true;
            evaluator2();
            *sync = false;
        }
    };
    for(auto* dispatcher : dispatchers1) {
        result += dispatcher->Connect(CONNECTION_DEBUG_LOCATION, eval1);
    }
    for(auto* dispatcher : dispatchers2) {
        result += dispatcher->Connect(CONNECTION_DEBUG_LOCATION, eval2);
    }
    eval1();
    return result;
}

LocalPropertyBoolCommutator::LocalPropertyBoolCommutator(bool defaultState, const DelayedCallObjectParams& params)
    : Super(defaultState)
    , m_commutator(params)
    , m_defaultState(defaultState)
{
    m_commutator += { this, [this]{
        Update();
    }};
}

void LocalPropertyBoolCommutator::ClearProperties()
{
    m_properties.clear();
}

void LocalPropertyBoolCommutator::Update()
{
    bool result = m_defaultState;
    bool oppositeState = !result;
    for(auto* property : ::make_const(m_properties)) {
        if(*property == oppositeState) {
            result = oppositeState;
            break;
        }
    }
    SetValue(result);
}

DispatcherConnections LocalPropertyBoolCommutator::AddProperty(const char* connectionInfo, const LocalPropertyBool& property)
{
    m_properties.append(&property);
    m_commutator.Invoke();
    return m_commutator.ConnectFrom(connectionInfo, property.OnChanged);
}

DispatcherConnections LocalPropertyBoolCommutator::AddProperties(const char* connectionInfo, const QVector<const LocalPropertyBool*>& properties)
{
    DispatcherConnections result;
    for(auto* property : properties) {
        result += m_commutator.ConnectFrom(connectionInfo, property->OnChanged);
    }
    m_properties += properties;
    m_commutator.Invoke();
    return result;
}

LocalPropertyDate::LocalPropertyDate(const QDate& value, const QDate& min, const QDate& max)
    : Super(applyRange(value, min, max))
    , m_min(min)
    , m_max(max)
{}

void LocalPropertyDate::SetMinMax(const QDate& min, const QDate& max)
{
    if(!LocalPropertyEqual(m_max, max) || !LocalPropertyEqual(m_min, min)) {
        m_min = min;
        m_max = max;
        SetValue(Super::m_value);
        OnMinMaxChanged();
    }
}

QDate LocalPropertyDate::applyRange(const QDate& cur, const QDate& min, const QDate& max)
{
    if(!cur.isValid()) {
        return cur;
    }
    return QDate::fromJulianDay(::clamp(cur.toJulianDay(), validatedMin(min).toJulianDay(), validatedMax(max).toJulianDay()));
}

QDate LocalPropertyDate::applyMinMax(const QDate& value) const
{
    return applyRange(value, m_min, m_max);
}

void LocalPropertyDate::validate(QDate& value) const
{
    value = applyMinMax(value);
}

LocalPropertyTime::LocalPropertyTime(const QTime& value, const QTime& min, const QTime& max)
    : Super(applyRange(value, min, max))
    , m_min(min)
    , m_max(max)
{
}

void LocalPropertyTime::SetMinMax(const QTime& min, const QTime& max)
{
    if(!LocalPropertyEqual(m_max, max) || !LocalPropertyEqual(m_min, min)) {
        m_min = min;
        m_max = max;
        SetValue(Super::m_value);
        OnMinMaxChanged();
    }
}

QTime LocalPropertyTime::applyRange(const QTime& cur, const QTime& min, const QTime& max)
{
    if(!cur.isValid()) {
        return cur;
    }
    return QTime::fromMSecsSinceStartOfDay(::clamp(cur.msecsSinceStartOfDay(), validatedMin(min).msecsSinceStartOfDay(), validatedMax(max).msecsSinceStartOfDay()));
}

QTime LocalPropertyTime::applyMinMax(const QTime& value) const
{
    return applyRange(value, m_min, m_max);
}

void LocalPropertyTime::validate(QTime& value) const
{
    value = applyMinMax(value);
}

LocalPropertyDateTime::LocalPropertyDateTime(const QDateTime& value, const QDateTime& min, const QDateTime& max)
    : Super(applyRange(value, min, max))
    , m_min(min)
    , m_max(max)
{
}

void LocalPropertyDateTime::SetMinMax(const QDateTime& min, const QDateTime& max)
{
    if(!LocalPropertyEqual(m_max, max) || !LocalPropertyEqual(m_min, min)) {
        m_min = min;
        m_max = max;
        SetValue(Super::m_value);
        OnMinMaxChanged();
    }
}

QDateTime LocalPropertyDateTime::applyRange(const QDateTime& cur, const QDateTime& min, const QDateTime& max)
{
    if(!cur.isValid()) {
        return cur;
    }
    return QDateTime::fromMSecsSinceEpoch(::clamp(cur.toMSecsSinceEpoch(), validatedMin(min).toMSecsSinceEpoch(), validatedMax(max).toMSecsSinceEpoch()));
}

void LocalPropertyDateTime::validate(QDateTime& value) const
{
    value = applyRange(value, m_min, m_max);
}

LocalPropertyBool::LocalPropertyBool(bool state)
    : Super(state)
{

}

LocalPropertyBoolCommutator2::LocalPropertyBoolCommutator2(bool defaultState, const DelayedCallObjectParams& params)
    : Super(defaultState)
    , m_commutator(params)
    , m_defaultState(defaultState)
{
    m_commutator += { this, [this]{
        Update();
    }};
}

void LocalPropertyBoolCommutator2::ClearProperties()
{
    m_handlers.clear();
}

DispatcherConnections LocalPropertyBoolCommutator2::AddProperty(const char* locationInfo, LocalPropertyBool* p){
    m_handlers.append([p]{ return p->Native(); });
    auto result = m_commutator.ConnectFrom(locationInfo, p->OnChanged);
    m_commutator.Invoke();
    return result;
}

void LocalPropertyBoolCommutator2::Update()
{
    bool result = m_defaultState;
    bool oppositeState = !result;
    for(const auto& handler : ::make_const(m_handlers)) {
        if(handler() == oppositeState) {
            result = oppositeState;
            break;
        }
    }
    SetValue(result);
}
