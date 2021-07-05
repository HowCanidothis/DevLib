#include "metricunitmanager.h"

#include <qmath.h>

MetricSystem::MetricSystem()
    : m_currentUnit(nullptr)
{

}

MetricSystem& MetricSystem::AddMetricUnit(const Name& id, const TranslatedString* label, const MetricUnit::FTransform& unitToValueMultiplier, const MetricUnit::FTransform& valueToUnitMultiplier)
{
    Q_ASSERT(!m_metricUnits.contains(id));

    auto result = ::make_shared<MetricUnit>();
    result->MetricName = label;
    result->UnitToBaseValue = unitToValueMultiplier;
    result->BaseValueToUnit = valueToUnitMultiplier;

    if(m_currentUnit == nullptr) {
        m_currentUnit = result.get();
        CurrentLabel.ConnectFrom(*label);
    }

    m_metricUnits.insert(id, result);

    return *this;
}

void MetricSystem::SetCurrent(const Name& id)
{
    Q_ASSERT(m_metricUnits.contains(id));
    m_currentUnit = m_metricUnits[id].get();
    CurrentLabel.ConnectFrom(*m_currentUnit->MetricName);
    OnCurrentChanged();
}

const MetricUnit* MetricSystem::getCurrentMetricUnit() const
{
    return m_currentUnit;
}

const MetricUnit::FTransform& MetricSystem::GetCurrentUnitToBaseValueConverter() const
{
    return m_currentUnit->UnitToBaseValue;
}

const MetricUnit::FTransform& MetricSystem::GetCurrentBaseValueToUnitConverter() const
{
    return m_currentUnit->BaseValueToUnit;
}

const MetricUnitPtr& MetricSystem::GetMetricUnit(const Name& metric) const
{
    static MetricUnitPtr defaultResult;
    auto foundIt = m_metricUnits.find(metric);
    if(foundIt != m_metricUnits.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

MetricUnitManager::MetricUnitManager()
{
    addMetricSystem(METRIC_SYSTEM_ANGLES)
            .AddMetricUnit(METRIC_UNIT_DEGREE, m_angles.Degrees, [](double degrees) { return qDegreesToRadians(degrees); }, [](double rad) { return qRadiansToDegrees(rad); })
            .AddMetricUnit(METRIC_UNIT_RADIAN, m_angles.Radians, [](double rad) { return rad; }, [](double rad) { return rad; });

    addMetricSystem(METRIC_SYSTEM_DISTANCES)
            .AddMetricUnit(METRIC_UNIT_USFT, m_distances.USFeets, [](double usft) { return usft * 1.000002; }, [](double ft) { return ft / 1.000002; })
            .AddMetricUnit(METRIC_UNIT_FT, m_distances.Feets, [](double ft) { return ft; }, [](double ft) { return ft; })
            .AddMetricUnit(METRIC_UNIT_METER, m_distances.Meters, [](double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }, [](double ft) { return ft / METERS_TO_FEETS_MULTIPLIER; });
}

const MetricSystemPtr& MetricUnitManager::GetMetricSystem(const Name& name) const
{
    static MetricSystemPtr defaultResult;
    auto foundIt = m_metricSystems.find(name);
    if(foundIt != m_metricSystems.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

MetricSystem& MetricUnitManager::addMetricSystem(const Name& name)
{
    Q_ASSERT(!m_metricSystems.contains(name));
    auto result = ::make_shared<MetricSystem>();
    m_metricSystems.insert(name, result);
    return *result;
}

MetricUnitManager& MetricUnitManager::GetInstance()
{
    static MetricUnitManager result;
    return result;
}

const MetricUnit* MetricUnitManager::GetCurrentUnit(const Name& systemName) const
{
    Q_ASSERT(m_metricSystems.contains(systemName));
    return m_metricSystems[systemName]->getCurrentMetricUnit();
}

void MetricProperty::Connect(LocalPropertyDouble* baseValueProperty)
{
    m_connections.clear();
    if(baseValueProperty != nullptr) {
        baseValueProperty->ConnectBoth(Value, [this](double baseValue){
            return m_metricSystem->GetCurrentBaseValueToUnitConverter()(baseValue);
        }, [this](double currentUnit){
            return m_metricSystem->GetCurrentUnitToBaseValueConverter()(currentUnit);
        }).MakeSafe(m_connections);

        auto updateMinMax = [baseValueProperty, this]{
            Value.SetMinMax(m_metricSystem->GetCurrentBaseValueToUnitConverter()(baseValueProperty->GetMin()), m_metricSystem->GetCurrentBaseValueToUnitConverter()(baseValueProperty->GetMax()));
        };
        baseValueProperty->OnMinMaxChanged.Connect(this, updateMinMax).MakeSafe(m_connections);
        updateMinMax();

    }
    m_currentValue = baseValueProperty;
}

MetricProperty::MetricProperty(const Name& systemName)
    : m_currentValue(nullptr)
{
    m_metricSystem = MetricUnitManager::GetInstance().GetMetricSystem(systemName);
    m_systemConnection = m_metricSystem->OnCurrentChanged.Connect(this, [this]{
        Connect(m_currentValue);
    }).MakeSafe();
}

MetricTranslatedString::MetricTranslatedString(const std::function<QString ()>& translationHandler, const QVector<Name>& metrics)
    : Super([translationHandler, metrics]{
        THREAD_ASSERT_IS_MAIN()
        static QRegExp regExp("%metric");
        auto string = translationHandler();
        qint32 index = 0, stringIndex = 0;
        auto it = metrics.begin();
        QString resultString;
        while((index = regExp.indexIn(string, index)) != -1 && it != metrics.end()) {
            resultString.append(QStringView(string.begin() + stringIndex, string.begin() + index).toString());
            auto metricSystem = MetricUnitManager::GetInstance().GetMetricSystem(*it);
            resultString.append(metricSystem->CurrentLabel);
            it++;
            index += regExp.matchedLength();
            stringIndex = index;
        }
        resultString.append(QStringView(string.begin() + stringIndex, string.end()).toString());

        return resultString;
    })
{
    QSet<Name> uniqueMetrics;
    for(const auto& metric : metrics) {
        auto foundIt = uniqueMetrics.find(metric);
        if(foundIt == uniqueMetrics.end()) {
            MetricUnitManager::GetInstance().GetMetricSystem(metric)->CurrentLabel.OnChange.Connect(this, [this]{
                retranslate();
            }).MakeSafe(m_connections);
            uniqueMetrics.insert(metric);
        }
    }
}
