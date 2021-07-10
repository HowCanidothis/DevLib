#include "metricunitmanager.h"

#include <qmath.h>

MeasurementUnit::MeasurementUnit(const FTranslationHandler& translationHandler, double multiplierUnitToBase)
    : Label(translationHandler)
    , m_multiplier(multiplierUnitToBase)
{

}

double MeasurementUnit::FromUnitToBase(double unitValue) const
{
    return unitValue * m_multiplier;
}

double MeasurementUnit::FromBaseToUnit(double baseValue) const
{
    return baseValue / m_multiplier;
}

Measurement::Measurement()
    : m_currentUnit(nullptr)
{

}

Measurement& Measurement::AddUnit(const Name& id, const MeasurementUnit* unit)
{
    Q_ASSERT(!m_metricUnits.contains(id));

    if(m_currentUnit == nullptr) {
        m_currentUnit = unit;
        CurrentLabel.ConnectFrom(unit->Label).MakeSafe(m_currentConnections);
    }

    m_metricUnits.insert(id, unit);

    return *this;
}

void Measurement::SetCurrent(const Name& id)
{
    Q_ASSERT(m_metricUnits.contains(id));
    m_currentUnit = m_metricUnits[id];
    CurrentLabel.ConnectFrom(m_currentUnit->Label).MakeSafe(m_currentConnections);
    OnCurrentChanged();
}

double Measurement::CurrentUnitToBaseValue(double currentUnit) const
{
    return m_currentUnit->FromUnitToBase(currentUnit);
}

double Measurement::BaseValueToCurrentUnit(double baseValue) const
{
    return m_currentUnit->FromBaseToUnit(baseValue);
}

const MeasurementUnit* Measurement::getCurrentMeasurementUnit() const
{
    return m_currentUnit;
}

const MeasurementUnit* Measurement::FindUnit(const Name &metric) const
{
    auto foundIt = m_metricUnits.find(metric);
    if(foundIt != m_metricUnits.end()) {
        return foundIt.value();
    }
    return nullptr;
}

MeasurementManager::MeasurementManager()
{
    AddMeasurement(MEASUREMENT_ANGLES)
            .AddUnit("Degree", &AngleUnits::Degrees)
            .AddUnit("Radian", &AngleUnits::Radians);

    AddMeasurement(MEASUREMENT_DISTANCES)
            .AddUnit("Usfeets", &DistanceUnits::USFeets)
            .AddUnit("Feets", &DistanceUnits::Feets)
            .AddUnit("Meters", &DistanceUnits::Meters);

    AddMeasurement(MEASUREMENT_FIELD_STRENGTH)
            .AddUnit("NanoTeslas", &FieldStrengthUnits::NanoTeslas);
}

const MeasurementPtr& MeasurementManager::GetMeasurement(const Name& name) const
{
    static MeasurementPtr defaultResult;
    auto foundIt = m_metricSystems.find(name);
    if(foundIt != m_metricSystems.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

Measurement& MeasurementManager::AddMeasurement(const Name &name)
{
    Q_ASSERT(!m_metricSystems.contains(name));
    auto result = ::make_shared<Measurement>();
    m_metricSystems.insert(name, result);
    return *result;
}

MeasurementManager& MeasurementManager::GetInstance()
{
    static MeasurementManager result;
    return result;
}

const MeasurementUnit* MeasurementManager::GetCurrentUnit(const Name& systemName) const
{
    Q_ASSERT(m_metricSystems.contains(systemName));
    return m_metricSystems[systemName]->getCurrentMeasurementUnit();
}

void MeasurementProperty::Connect(LocalPropertyDouble* baseValueProperty)
{
    m_connections.clear();
    if(baseValueProperty != nullptr) {
        baseValueProperty->ConnectBoth(Value, [this](double baseValue){
            return m_metricSystem->BaseValueToCurrentUnit(baseValue);
        }, [this](double currentUnit){
            return m_metricSystem->CurrentUnitToBaseValue(currentUnit);
        }).MakeSafe(m_connections);

        auto updateMinMax = [baseValueProperty, this]{
            Value.SetMinMax(m_metricSystem->BaseValueToCurrentUnit(baseValueProperty->GetMin()), m_metricSystem->BaseValueToCurrentUnit(baseValueProperty->GetMax()));
        };
        baseValueProperty->OnMinMaxChanged.Connect(this, updateMinMax).MakeSafe(m_connections);
        updateMinMax();

    }
    m_currentValue = baseValueProperty;
}

MeasurementProperty::MeasurementProperty(const Name& systemName)
    : m_currentValue(nullptr)
{
    m_metricSystem = MeasurementManager::GetInstance().GetMeasurement(systemName);
    m_systemConnection = m_metricSystem->OnCurrentChanged.Connect(this, [this]{
        Connect(m_currentValue);
    }).MakeSafe();
}

MeasurementTranslatedString::MeasurementTranslatedString(const std::function<QString ()>& translationHandler, const QVector<Name>& metrics)
    : Super([translationHandler, metrics]{
        THREAD_ASSERT_IS_MAIN()
        static QRegExp regExp("%metric");
        auto string = translationHandler();
        qint32 index = 0, stringIndex = 0;
        auto it = metrics.begin();
        QString resultString;
        while((index = regExp.indexIn(string, index)) != -1 && it != metrics.end()) {
            resultString.append(QStringView(string.begin() + stringIndex, string.begin() + index).toString());
            auto metricSystem = MeasurementManager::GetInstance().GetMeasurement(*it);
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
            MeasurementManager::GetInstance().GetMeasurement(metric)->CurrentLabel.OnChange.Connect(this, [this]{
                retranslate();
            }).MakeSafe(m_connections);
            uniqueMetrics.insert(metric);
        }
    }
}
