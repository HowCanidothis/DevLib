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

//DispatcherConnections LocalPropertyBoolCommutator::ConnectFromDispatchers(const char* locationInfo, const std::function<bool ()>& thisEvaluator, const QVector<Dispatcher*>& dispatchers)
//{
//    DispatcherConnections result;
//    m_handlers.append(thisEvaluator);
//    for(const auto& dispatcher : dispatchers){
//        result += m_commutator.ConnectFrom(locationInfo, *dispatcher);
//    }
//    result += DispatcherConnection([this]{ Q_ASSERT_X(m_handlers.isEmpty(), DEBUG_LOCATION, "Commutator must be Reseted before disconnection"); });
//    m_commutator.Invoke();
//    return result;
//}

LocalPropertyDate::LocalPropertyDate(const QDate& value, const QDate& min, const QDate& max)
    : Super(applyRange(value, min, max))
    , m_min(min)
    , m_max(max)
{}

void LocalPropertyDate::SetMinMax(const QDate& min, const QDate& max)
{
    if(Comparator<QDate>::NotEqual(m_max, max) || Comparator<QDate>::NotEqual(m_min, min)) {
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
    return QDate::fromJulianDay(::clamp(cur.toJulianDay(), ValidatedMin(min).toJulianDay(), ValidatedMax(max).toJulianDay()));
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
    if(Comparator<QTime>::NotEqual(m_max, max) || Comparator<QTime>::NotEqual(m_min, min)) {
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
    return QTime::fromMSecsSinceStartOfDay(::clamp(cur.msecsSinceStartOfDay(), ValidatedMin(min).msecsSinceStartOfDay(), ValidatedMax(max).msecsSinceStartOfDay()));
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
    if(Comparator<QDateTime>::NotEqual(m_max, max) || Comparator<QDateTime>::NotEqual(m_min, min)) {
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
    return QDateTime::fromMSecsSinceEpoch(::clamp(cur.toMSecsSinceEpoch(), ValidatedMin(min).toMSecsSinceEpoch(), ValidatedMax(max).toMSecsSinceEpoch()), cur.timeSpec(), cur.offsetFromUtc());
}

void LocalPropertyDateTime::validate(QDateTime& value) const
{
    value = applyRange(value, m_min, m_max);
}

LocalPropertyBool::LocalPropertyBool(bool state)
    : Super(state)
{

}
