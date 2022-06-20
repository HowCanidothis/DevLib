#include "measurementdeclarations.h"

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
