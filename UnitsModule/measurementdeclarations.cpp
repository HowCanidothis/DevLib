#include "measurementdeclarations.h"

#include "measurementunitmanager.h"

MeasurementUnit::MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, double multiplierUnitToBase)
    : MeasurementUnit(id, fullLabelTrHandler, translationHandler, [multiplierUnitToBase](double unitValue){ return unitValue * multiplierUnitToBase; }, [multiplierUnitToBase](double baseValue){ return baseValue / multiplierUnitToBase; })
{

}

MeasurementUnit::MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, const FTransform& unitToBase, const FTransform& baseToUnit)
    : Id(id)
    , LabelFull(fullLabelTrHandler)
    , Label(translationHandler)
    , m_unitToBase(unitToBase)
    , m_baseToUnit(baseToUnit)
{

}

double MeasurementUnit::FromUnitToBaseChange(double& unitValue) const
{
    unitValue = FromUnitToBase(unitValue);
    return unitValue;
}

double MeasurementUnit::FromBaseToUnitChange(double& baseValue) const
{
    baseValue = FromBaseToUnit(baseValue);
    return baseValue;
}

double MeasurementUnit::FromUnitToBase(double unitValue) const
{
    return m_unitToBase(unitValue);
}

double MeasurementUnit::FromBaseToUnit(double baseValue) const
{
    return m_baseToUnit(baseValue);
}

bool LocalPropertyNotEqual(const MeasurementUnit::FTransform&, const MeasurementUnit::FTransform&)
{
    return true;
}

void MeasurementProperty::Connect(LocalPropertyDouble* baseValueProperty)
{
    m_connections.clear();
    if(baseValueProperty != nullptr) {
        baseValueProperty->ConnectBoth(CONNECTION_DEBUG_LOCATION,Value, [this](double baseValue){
                                 return m_metricSystem->FromBaseToUnit(baseValue);
                             }, [this](double currentUnit){
                                 return m_metricSystem->FromUnitToBase(currentUnit);
                             }).MakeSafe(m_connections);

        auto updateMinMax = [baseValueProperty, this]{
            auto convertValue = [this](double source) {
                auto unitValue = m_metricSystem->FromBaseToUnit(source);
                return qIsInf(unitValue) ? source : unitValue;
            };
            Value.SetMinMax(convertValue(baseValueProperty->GetMin()), convertValue(baseValueProperty->GetMax()));
            Value = convertValue(baseValueProperty->Native());
        };
        baseValueProperty->OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, updateMinMax).MakeSafe(m_connections);
        updateMinMax();
    }
    m_currentValue = baseValueProperty;
}

MeasurementProperty::MeasurementProperty(const Measurement* measurement)
    : m_currentValue(nullptr)
    , m_metricSystem(measurement)
{
    m_metricSystem->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
                                                      Connect(m_currentValue);
                                     }).MakeSafe(m_systemConnections);

    Precision.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_metricSystem->CurrentPrecision).MakeSafe(m_systemConnections);
    Step.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_metricSystem->CurrentStep).MakeSafe(m_systemConnections);
}

MeasurementValueWrapper::MeasurementValueWrapper()
    : m_target(nullptr)
    , m_measurement(nullptr)
{}
MeasurementValueWrapper::MeasurementValueWrapper(const Measurement* measurement, double* value, double min, double max)
    : m_target(value)
    , m_measurement(measurement)
    , m_min(min)
    , m_max(max)
{}

SharedPointer<LocalPropertyDouble> MeasurementValueWrapper::CreateProperty() const
{
    return ::make_shared<LocalPropertyDouble>(0.0, m_min, m_max);
}

void MeasurementValueWrapper::SetCurrentUnit(double value)
{
    *m_target = clamp(m_measurement->FromUnitToBase(value), m_min, m_max);
}
QString MeasurementValueWrapper::GetCurrentUnitUi() const
{
    return m_measurement->FromBaseToUnitUi(*m_target);
}
double MeasurementValueWrapper::GetCurrentUnit() const
{
    return m_measurement->FromBaseToUnit(*m_target);
}
MeasurementValueWrapper& MeasurementValueWrapper::operator=(double value)
{
    *m_target = clamp(value, m_min, m_max);
    return *this;
}
