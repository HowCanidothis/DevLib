#include "measurementunitmanager.h"

#include "MeasurementTypes/angledeclarations.h"
#include "MeasurementTypes/distancedeclarations.h"
#include "MeasurementTypes/doglegdeclarations.h"
#include "MeasurementTypes/flowspeeddeclarations.h"
#include "MeasurementTypes/magneticdeclarations.h"
#include "MeasurementTypes/massdeclarations.h"
#include "MeasurementTypes/pressuredeclarations.h"
#include "MeasurementTypes/speeddeclarations.h"
#include "MeasurementTypes/torquedeclarations.h"
#include "MeasurementTypes/funnelviscositydeclarations.h"
#include "MeasurementTypes/percentsdeclarations.h"
#include "MeasurementTypes/mudweightdeclaration.h"
#include "MeasurementTypes/viscositydeclaration.h"
#include "MeasurementTypes/temperaturedeclarations.h"
#include "MeasurementTypes/densitydeclarations.h"
#include "MeasurementTypes/weightperlengthdeclarations.h"
#include "MeasurementTypes/thermalexpansiondeclarations.h"
#include "MeasurementTypes/areadeclarations.h"
#include "MeasurementTypes/forcedeclarations.h"
#include "MeasurementTypes/motorspeeddeclarations.h"
#include "MeasurementTypes/timedeclarations.h"
#include "MeasurementTypes/volumeperlengthdeclarations.h"
#include "MeasurementTypes/frequencydeclarations.h"
#include "MeasurementTypes/thermalconductivitydeclarations.h"
#include "MeasurementTypes/specificheatcapacitydeclarations.h"
#include "MeasurementTypes/powerdeclarations.h"

static const Name UNIT_SYSTEM_API         = "API";
static const Name UNIT_SYSTEM_API_USFT    = "API USFT";
static const Name UNIT_SYSTEM_SI          = "SI";

Measurement::Measurement(const FTranslationHandler& label)
    : Label(label)
    , CurrentPrecision(2)
    , CurrentStep(1.0)
    , m_wrapper(::make_shared<WPSCUnitTableWrapper>())
    , m_currentUnit(nullptr)
    , m_idsDictionary(::make_scoped<FTSDictionary>())
    , m_idsCache(::make_scoped<FTSObject>(m_idsDictionary.get()))
{
    CurrentUnitId.Subscribe([this]{
        m_currentConnections.clear();
        auto foundIt = m_metricUnits.find(CurrentUnitId);
        if(foundIt != m_metricUnits.end()) {
            m_currentUnit = m_metricUnits[CurrentUnitId];
        } else {
            auto match = m_idsDictionary->Match(CurrentUnitId.Native().AsString());
            qint32 offset = 0;
            if(!match.isEmpty()) {
                match.Sort();
                offset = match.first().Row.Id;
            }
            m_currentUnit = *(m_metricUnits.begin() + offset);
            CurrentUnitId.EditSilent() = m_currentUnit->Id;
        }
        CurrentUnitLabel.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_currentUnit->Label).MakeSafe(m_currentConnections);
    });
    
    OnChanged.Subscribe(CONNECTION_DEBUG_LOCATION, {&Label.OnChanged,
                           &CurrentUnitId.OnChanged,
                           &CurrentUnitLabel.OnChanged,
                           &CurrentPrecision.OnChanged,
                           &CurrentStep.OnChanged});
}

Measurement& Measurement::AddUnit(const MeasurementUnit* unit)
{
    Q_ASSERT(!m_metricUnits.contains(unit->Id));
    
    if(m_currentUnit == nullptr) {
        m_currentUnit = unit;
        CurrentUnitLabel.ConnectFrom(CONNECTION_DEBUG_LOCATION, unit->Label).MakeSafe(m_currentConnections);
    }
    
    m_metricUnits.insert(unit->Id, unit);
    m_idsCache->AddRow(unit->Id.AsString(), Name::FromValue(m_wrapper->GetSize()));
    m_wrapper->Append(unit);
    return *this;
}

double Measurement::CurrentUnitToBaseValue(double currentUnit) const
{
    return m_currentUnit->FromUnitToBase(currentUnit);
}

double Measurement::BaseValueToCurrentUnit(double baseValue) const
{
    return m_currentUnit->FromBaseToUnit(baseValue);
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
    : m_systemWrapper(::make_shared<WPSCUnitSystemTableWrapper>())
      , m_measurmentWrapper(::make_shared<WPSCUnitMeasurementTableWrapper>())
{
    AddMeasurement(MeasurementAngle::NAME, []{ return tr("Angles"); })
            .AddUnit(&AngleUnits::Degrees)
            .AddUnit(&AngleUnits::Radians);

    AddMeasurement(MeasurementDistance::NAME, []{ return tr("Distances"); })
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Meters);

    AddMeasurement(MeasurementFunnelViscosity::NAME, []{ return tr("Funnel Viscosity"); })
            .AddUnit(&FunnelViscosityUnits::SecondsForQuart);

    AddMeasurement(MeasurementPercents::NAME, []{ return tr("Percents"); })
            .AddUnit(&PercentsUnits::Partial)
            .AddUnit(&PercentsUnits::Percents);

    AddMeasurement(MeasurementDiameter::NAME, []{ return tr("Diameter"); })
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::Miles)
            .AddUnit(&DistanceUnits::Meters)
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Kilometers);

    AddMeasurement(MeasurementMagneticField::NAME, []{ return tr("Magnetic Field"); })
            .AddUnit(&FieldStrengthUnits::MicroTeslas)
            .AddUnit(&FieldStrengthUnits::Gauss)
            .AddUnit(&FieldStrengthUnits::NanoTeslas);

    AddMeasurement(MeasurementDLS::NAME, []{ return tr("Dogleg Severity"); })
            .AddUnit(&DLSUnits::DegreeUSFeet)
            .AddUnit(&DLSUnits::DegreeFeet)
            .AddUnit(&DLSUnits::DegreeMeter)
            .AddUnit(&DLSUnits::RadMeter);

    AddMeasurement(MeasurementFlowSpeed::NAME, []{ return tr("Flow Speed"); })
            .AddUnit(&FlowSpeedUnits::CubicMetersPerSecond)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerMinute)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerHour  )
            .AddUnit(&FlowSpeedUnits::CubicMetersPerDay   )
            .AddUnit(&FlowSpeedUnits::LitersPerSecond     )
            .AddUnit(&FlowSpeedUnits::LitersPerMinute     )
            .AddUnit(&FlowSpeedUnits::GallonsPerMinute    )
            .AddUnit(&FlowSpeedUnits::BarrelsPerMinute    );

    AddMeasurement(MeasurementThermalConductivity::NAME, []{ return tr("Thermal Conductivity"); })
            .AddUnit(&ThermalConductivityUnits::WattMeterCelsius)
            .AddUnit(&ThermalConductivityUnits::FootHourSquareFootFahrenheit);

    AddMeasurement(MeasurementSpecificHeatCapacity::NAME, []{ return tr("Specific Heat Capacity"); })
            .AddUnit(&SpecificHeatCapacityUnits::JouleKilogramCelsius)
            .AddUnit(&SpecificHeatCapacityUnits::PoundFahrenheit);

    AddMeasurement(MeasurementMass::NAME, []{ return tr("Mass"); })
            .AddUnit(&MassUnits::Kilograms )
            .AddUnit(&MassUnits::Grams     )
            .AddUnit(&MassUnits::Tonnes    )
            .AddUnit(&MassUnits::Pounds    )
            .AddUnit(&MassUnits::Kilopounds);

    AddMeasurement(MeasurementPressure::NAME, []{ return tr("Pressure"); })
            .AddUnit(&PressureUnits::Pascals                    )
            .AddUnit(&PressureUnits::Kilopascals                )
            .AddUnit(&PressureUnits::Bars                       )
            .AddUnit(&PressureUnits::Megapascals                )
            .AddUnit(&PressureUnits::Atmospheres                )
            .AddUnit(&PressureUnits::KilogramPerSquareCentimeter)
            .AddUnit(&PressureUnits::KilogramPerSquareMeter     )
            .AddUnit(&PressureUnits::PoundsPerSquareInch        )
            .AddUnit(&PressureUnits::KilopoundsPerSquareInch    )
            .AddUnit(&PressureUnits::PoundsPerSquareFeet     )
            .AddUnit(&PressureUnits::PoundsPer100SquareFeet     );

    AddMeasurement(MeasurementSpeed::NAME, []{ return tr("Speed"); })
            .AddUnit(&SpeedUnits::MetersPerSecond  )
            .AddUnit(&SpeedUnits::MetersPerMinute  )
            .AddUnit(&SpeedUnits::MetersPerHour    )
            .AddUnit(&SpeedUnits::KilometersPerHour)
            .AddUnit(&SpeedUnits::FeetPerHour      )
            .AddUnit(&SpeedUnits::USfeetPerHour    )
            .AddUnit(&SpeedUnits::FeetPerMinute    )
            .AddUnit(&SpeedUnits::USfeetPerMinute  )
            .AddUnit(&SpeedUnits::FeetPerSecond    )
            .AddUnit(&SpeedUnits::USfeetPerSecond  )
            .AddUnit(&SpeedUnits::MilesPerHour     );

    AddMeasurement(MeasurementTorque::NAME, []{ return tr("Torque"); })
            .AddUnit(&TorqueUnits::NewtonMeters      )
            .AddUnit(&TorqueUnits::DecaNewtonMeters  )
            .AddUnit(&TorqueUnits::KilonewtonMeters  )
            .AddUnit(&TorqueUnits::PoundForceFeet    )
            .AddUnit(&TorqueUnits::KilopoundForceFeet)
            .AddUnit(&TorqueUnits::PoundFoot);

    AddMeasurement(MeasurementMudWeight::NAME, []{ return tr("Mud Weight"); })
            .AddUnit(&MudWeightUnits::PoundPerGallon      )
            .AddUnit(&MudWeightUnits::KilogramPerCubicMeter);

    AddMeasurement(MeasurementViscosity::NAME, []{ return tr("Viscosity"); })
            .AddUnit(&ViscosityUnits::Centipoise       )
            .AddUnit(&ViscosityUnits::MilliPascalSecond);

    AddMeasurement(MeasurementYieldPoint::NAME, []{ return tr("Yield Point"); })
            .AddUnit(&PressureUnits::Pascals)
            .AddUnit(&PressureUnits::PoundsPerSquareFeet)
            .AddUnit(&PressureUnits::PoundsPer100SquareFeet)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::KilopoundsPerSquareInch);

    AddMeasurement(MeasurementTemperature::NAME, []{ return tr("Temperature"); })
            .AddUnit(&TemperatureUnits::Celsius   )
            .AddUnit(&TemperatureUnits::Fahrenheit);

    AddMeasurement(MeasurementTemperaturePerDistance::NAME, []{ return tr("Temperature Per Distance"); })
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPer100Feet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPer100Feet);

    AddMeasurement(MeasurementDensity::NAME, []{ return tr("Density"); })
            .AddUnit(&DensityUnits::KilogramsPerCubicMeters)
            .AddUnit(&DensityUnits::KilogramsPerLiter)
            .AddUnit(&DensityUnits::PoundsPerGallon)
            .AddUnit(&DensityUnits::PoundsPerCubicFeet);

    AddMeasurement(MeasurementWeightPerLength::NAME, []{ return tr("Weight Per Length"); })
            .AddUnit(&WeightPerLengthUnits::KilogramPerMeter)
            .AddUnit(&WeightPerLengthUnits::KilogramPerCantimeter)
            .AddUnit(&WeightPerLengthUnits::PoundPerFoot)
            .AddUnit(&WeightPerLengthUnits::PoundPerInch);

    AddMeasurement(MeasurementYoungModulus::NAME, []{ return tr("Young Modulus"); })
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MeasurementYieldStrength::NAME, []{ return tr("Yield Strength"); })
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MeasurementThermalExpansion::NAME, []{ return tr("Thermal Expansion"); })
            .AddUnit(&ThermalExpansionUnits::PerDegreeFahrenheit)
            .AddUnit(&ThermalExpansionUnits::PerDegreeCelsius);

    AddMeasurement(MeasurementJetDiameter::NAME, []{ return tr("Jet Diameter"); })
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::OnePerThirtyTwoInches);

    AddMeasurement(MeasurementArea::NAME, []{ return tr("Area"); })
            .AddUnit(&AreaUnits::SqMeters)
            .AddUnit(&AreaUnits::SqCentimeters)
            .AddUnit(&AreaUnits::SqMilimeters)
            .AddUnit(&AreaUnits::SqFeets)
            .AddUnit(&AreaUnits::SqInches);

    AddMeasurement(MeasurementForce::NAME, []{ return tr("Force"); })
            .AddUnit(&ForceUnits::Newton)
            .AddUnit(&ForceUnits::Kilonewton)
            .AddUnit(&ForceUnits::PoundsForce)
            .AddUnit(&ForceUnits::KiloGrammForce)
            .AddUnit(&ForceUnits::KiloPoundsForce);

    AddMeasurement(MeasurementMotorSpeed::NAME, []{ return tr("Motor Speed"); })
        .AddUnit(&MotorSpeedUnits::RevolutionPerGallon);

    AddMeasurement(MeasurementTime::NAME, []{ return tr("Time"); })
            .AddUnit(&TimeUnits::MSeconds)
            .AddUnit(&TimeUnits::Seconds)
            .AddUnit(&TimeUnits::Minutes)
            .AddUnit(&TimeUnits::Hours);

    AddMeasurement(MeasurementLinearCapacity::NAME, []{ return tr("Linear Capacity"); })
            .AddUnit(&VolumePerLengthUnits::CubicMeterPerMeter)
            .AddUnit(&VolumePerLengthUnits::BarrelPerFoot)
            .AddUnit(&VolumePerLengthUnits::LiterPerMeter);

    AddMeasurement(MeasurementFrequency::NAME, []{ return tr("Frequency"); })
            .AddUnit(&FrequencyUnits::RevolutionPerMinute);

    AddMeasurement(MeasurementCutterDiameter::NAME, []{ return tr("Cutter Diameter"); })
            .AddUnit(&DistanceUnits::Milimeters);

    AddMeasurement(MeasurementPower::NAME, []{ return tr("Power"); })
            .AddUnit(&PowerUnits::Watt)
            .AddUnit(&PowerUnits::JoulePerSecond)
            .AddUnit(&PowerUnits::KiloWatt)
            .AddUnit(&PowerUnits::HoursePower);

    AddMeasurement(MeasurementAnnularVelocity::NAME, []{ return tr("Annular Velocity"); })
            .AddUnit(&SpeedUnits::MetersPerSecond  )
            .AddUnit(&SpeedUnits::MetersPerMinute  )
            .AddUnit(&SpeedUnits::MetersPerHour    )
            .AddUnit(&SpeedUnits::KilometersPerHour)
            .AddUnit(&SpeedUnits::FeetPerHour      )
            .AddUnit(&SpeedUnits::USfeetPerHour    )
            .AddUnit(&SpeedUnits::FeetPerMinute    )
            .AddUnit(&SpeedUnits::USfeetPerMinute  )
            .AddUnit(&SpeedUnits::FeetPerSecond    )
            .AddUnit(&SpeedUnits::USfeetPerSecond  )
            .AddUnit(&SpeedUnits::MilesPerHour     );

    AddMeasurement(MeasurementROP::NAME, []{ return tr("Rate Of Penetration"); })
            .AddUnit(&SpeedUnits::MetersPerSecond  )
            .AddUnit(&SpeedUnits::MetersPerMinute  )
            .AddUnit(&SpeedUnits::MetersPerHour    )
            .AddUnit(&SpeedUnits::KilometersPerHour)
            .AddUnit(&SpeedUnits::FeetPerHour      )
            .AddUnit(&SpeedUnits::USfeetPerHour    )
            .AddUnit(&SpeedUnits::FeetPerMinute    )
            .AddUnit(&SpeedUnits::USfeetPerMinute  )
            .AddUnit(&SpeedUnits::FeetPerSecond    )
            .AddUnit(&SpeedUnits::USfeetPerSecond  )
            .AddUnit(&SpeedUnits::MilesPerHour     );

    AddSystem(UNIT_SYSTEM_API_USFT)
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Inches.Id,                     3, 0.125 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::USFeets.Id,                    2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::DegreeUSFeet.Id,                    2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {MudWeightUnits::PoundPerGallon.Id,            2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::USfeetPerHour.Id,                 2})
            .AddParameter(MeasurementAnnularVelocity::NAME,  {SpeedUnits::USfeetPerMinute.Id,               2})
            .AddParameter(MeasurementTemperature::NAME,       {TemperatureUnits::Fahrenheit.Id,              1})
            .AddParameter(MeasurementThermalExpansion::NAME, {ThermalExpansionUnits::PerDegreeFahrenheit.Id,2})
            .AddParameter(MeasurementTime::NAME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MeasurementTorque::NAME,            {TorqueUnits::KilopoundForceFeet.Id,           1})
            .AddParameter(MeasurementViscosity::NAME,         {ViscosityUnits::Centipoise.Id,                2})
            .AddParameter(MeasurementWeightPerLength::NAME, {WeightPerLengthUnits::PoundPerFoot.Id,        2})
            .AddParameter(MeasurementYieldPoint::NAME,       {PressureUnits::PoundsPer100SquareFeet.Id,     2})
            .AddParameter(MeasurementYieldStrength::NAME,    {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MeasurementYoungModulus::NAME,     {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MeasurementPower::NAME,             {PowerUnits::HoursePower.Id,                   3})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MeasurementThermalConductivity::NAME, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});

    AddSystem(UNIT_SYSTEM_API)
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Inches.Id,                     2, 0.125 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::Feets.Id,                      2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::DegreeFeet.Id,                      2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     2})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {MudWeightUnits::PoundPerGallon.Id,            2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::FeetPerHour.Id,                   2})
            .AddParameter(MeasurementAnnularVelocity::NAME,  {SpeedUnits::USfeetPerMinute.Id,               2})
            .AddParameter(MeasurementTemperature::NAME,       {TemperatureUnits::Fahrenheit.Id,              1})
            .AddParameter(MeasurementThermalExpansion::NAME, {ThermalExpansionUnits::PerDegreeFahrenheit.Id,2})
            .AddParameter(MeasurementTime::NAME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MeasurementTorque::NAME,            {TorqueUnits::KilopoundForceFeet.Id,           1})
            .AddParameter(MeasurementViscosity::NAME,         {ViscosityUnits::Centipoise.Id,                2})
            .AddParameter(MeasurementWeightPerLength::NAME, {WeightPerLengthUnits::PoundPerFoot.Id,        2})
            .AddParameter(MeasurementYieldPoint::NAME,       {PressureUnits::PoundsPer100SquareFeet.Id,        2})
            .AddParameter(MeasurementYieldStrength::NAME,    {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MeasurementYoungModulus::NAME,     {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MeasurementPower::NAME,             {PowerUnits::HoursePower.Id,                   3})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MeasurementThermalConductivity::NAME, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});

    
    AddSystem(UNIT_SYSTEM_SI)
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqCentimeters.Id,                  3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::KilogramsPerCubicMeters.Id,     2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Milimeters.Id,                 2, 1.0 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::Meters.Id,                     2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::DegreeMeter.Id,                     2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::LitersPerSecond.Id,           0})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::Kilonewton.Id,                    2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::CubicMeterPerMeter.Id,  2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::Milimeters.Id,                 0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Tonnes.Id,                         0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {MudWeightUnits::KilogramPerCubicMeter.Id,     2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::Bars.Id,                       0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::MetersPerHour.Id,                 2})
            .AddParameter(MeasurementAnnularVelocity::NAME,  {SpeedUnits::MetersPerSecond.Id,               2})
            .AddParameter(MeasurementTemperature::NAME,       {TemperatureUnits::Celsius.Id,                 1})
            .AddParameter(MeasurementThermalExpansion::NAME, {ThermalExpansionUnits::PerDegreeCelsius.Id,   2})
            .AddParameter(MeasurementTime::NAME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MeasurementTorque::NAME,            {TorqueUnits::KilonewtonMeters.Id,             1})
            .AddParameter(MeasurementViscosity::NAME,         {ViscosityUnits::MilliPascalSecond.Id,         2})
            .AddParameter(MeasurementWeightPerLength::NAME, {WeightPerLengthUnits::KilogramPerMeter.Id,    2})
            .AddParameter(MeasurementYieldPoint::NAME,       {PressureUnits::Pascals.Id,                    2})
            .AddParameter(MeasurementYieldStrength::NAME,    {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MeasurementYoungModulus::NAME,     {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MeasurementPower::NAME,             {PowerUnits::Watt.Id,                          3})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::JouleKilogramCelsius.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::CelsiusPerMeter.Id,  5})
            .AddParameter(MeasurementThermalConductivity::NAME, {ThermalConductivityUnits::WattMeterCelsius.Id,    2});

		
    
    CurrentMeasurementSystem.SetAndSubscribe([this]{
        const auto& system = GetSystem(CurrentMeasurementSystem);
        Q_ASSERT(system != nullptr);
        
        m_connections.clear();
        for(auto iter = system->begin(); iter != system->end(); ++iter){
            const auto& measurement = GetMeasurement(iter.key());
            auto* parameters = &iter.value();
            Q_ASSERT(measurement != nullptr);
            measurement->CurrentStep = parameters->UnitStep;
            measurement->CurrentUnitId = parameters->UnitId;
            measurement->CurrentPrecision = parameters->UnitPrecision;
            
            measurement->CurrentStep.OnChanged.Connect(this, [parameters, measurement]{parameters->UnitStep = measurement->CurrentStep; }).MakeSafe(m_connections);
            measurement->CurrentUnitId.OnChanged.Connect(this, [measurement, parameters]{parameters->UnitId = measurement->CurrentUnitId; }).MakeSafe(m_connections);
            measurement->CurrentPrecision.OnChanged.Connect(this, [parameters, measurement]{parameters->UnitPrecision = measurement->CurrentPrecision; }).MakeSafe(m_connections);
        }
    });
}

const MeasurementPtr& MeasurementManager::GetMeasurement(const Name& name) const
{
    static MeasurementPtr defaultResult;
    auto foundIt = m_metricMeasurements.find(name);
    if(foundIt != m_metricMeasurements.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

DispatcherConnections MeasurementManager::AttachConverter(const Name& measurementName, LocalProperty<MeasurementUnit::FTransform>* property, LocalPropertyInt* precision)
{
    auto* measurement = GetMeasurement(measurementName).get();
    DispatcherConnections result;
    result = property->ConnectFrom(CONNECTION_DEBUG_LOCATION, [measurement]{
        return [measurement](double value) { return measurement->BaseValueToCurrentUnit(value); };
    }, { &measurement->OnChanged });
    if(precision != nullptr) {
        result += precision->ConnectFrom(CONNECTION_DEBUG_LOCATION, measurement->CurrentPrecision);
    }
    return result;
}

const MeasurementSystemPtr& MeasurementManager::GetSystem(const Name& name) const
{
    static MeasurementSystemPtr defaultResult;
    auto foundIt = m_metricSystems.find(name);
    if(foundIt != m_metricSystems.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

Measurement& MeasurementManager::AddMeasurement(const Name &name, const FTranslationHandler& translationHandler)
{
    Q_ASSERT(!m_metricMeasurements.contains(name));
    auto result = ::make_shared<Measurement>(translationHandler);
    m_metricMeasurements.insert(name, result);
    int index = m_measurmentWrapper->GetSize();
    m_measurmentWrapper->Append(result);
    result->OnChanged.Connect(this, [this, index]{m_measurmentWrapper->Edit(index, [](const MeasurementPtr& ){ }); });
    return *result;
}

MeasurementSystem & MeasurementManager::AddSystem(const Name& name)
{
    Q_ASSERT(!m_metricSystems.contains(name));
    auto result = ::make_shared<MeasurementSystem>(name);
    AddSystem(result);
    return *result;
}

void MeasurementManager::AddSystem(const MeasurementSystemPtr & system) {
    Name key (system->Label.Native());
    m_metricSystems.insert(key, system);
    m_systemWrapper->Append(system);
    
    if(CurrentMeasurementSystem.Native().IsNull()) {
        CurrentMeasurementSystem = key;
    }
}

MeasurementManager& MeasurementManager::GetInstance()
{
    static MeasurementManager result;
    return result;
}

const MeasurementUnit* MeasurementManager::GetCurrentUnit(const Name& measurementName) const
{
    Q_ASSERT(m_metricMeasurements.contains(measurementName));
    return m_metricMeasurements[measurementName]->GetCurrentUnit();
}

QString MeasurementManager::FromBaseToUnitUi(const Name& systemName, double value) const
{
    const auto& measurement = GetMeasurement(systemName);
    return QString::number(measurement->GetCurrentUnit()->GetBaseToUnitConverter()(value), 'f', measurement->CurrentPrecision);
}

QStringList MeasurementManager::DefaultSystems()
{
	return {UNIT_SYSTEM_API.AsString(), UNIT_SYSTEM_API_USFT.AsString(), UNIT_SYSTEM_SI.AsString()};
}

void MeasurementProperty::Connect(LocalPropertyDouble* baseValueProperty)
{
    m_connections.clear();
    if(baseValueProperty != nullptr) {
        baseValueProperty->ConnectBoth(CONNECTION_DEBUG_LOCATION,Value, [this](double baseValue){
                                 return m_metricSystem->BaseValueToCurrentUnit(baseValue);
                             }, [this](double currentUnit){
                                 return m_metricSystem->CurrentUnitToBaseValue(currentUnit);
                             }).MakeSafe(m_connections);
        
        auto updateMinMax = [baseValueProperty, this]{
            auto convertValue = [this](double source) {
                auto unitValue = m_metricSystem->BaseValueToCurrentUnit(source);
                return qIsInf(unitValue) ? source : unitValue;
            };
            Value.SetMinMax(convertValue(baseValueProperty->GetMin()), convertValue(baseValueProperty->GetMax()));
            Value = convertValue(baseValueProperty->Native());
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
    m_metricSystem->OnChanged.Connect(this, [this]{
                                                      Connect(m_currentValue);
                                                  }).MakeSafe(m_systemConnections);

    Precision.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_metricSystem->CurrentPrecision).MakeSafe(m_systemConnections);
    Step.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_metricSystem->CurrentStep).MakeSafe(m_systemConnections);
}

void MeasurementTranslatedString::AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<Name>& metrics)
{
    DispatcherConnectionsSafe connections;
    AttachToTranslatedString(string, translationHandler, metrics, connections);
}

void MeasurementTranslatedString::AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<Name>& metrics, DispatcherConnectionsSafe& connections)
{
    QSet<Name> uniqueMetrics;
    for(const auto& metric : metrics) {
        auto foundIt = uniqueMetrics.find(metric);
        if(foundIt == uniqueMetrics.end()) {
            string.Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, MeasurementManager::GetInstance().GetMeasurement(metric)->CurrentUnitLabel.OnChanged).MakeSafe(connections);
            uniqueMetrics.insert(metric);
        }
    }
    string.SetTranslationHandler(generateTranslationHandler(translationHandler, metrics, connections));
}

FTranslationHandler MeasurementTranslatedString::generateTranslationHandler(const FTranslationHandler& translationHandler, const QVector<Name>& metrics, const DispatcherConnectionsSafe& connections)
{
    return [translationHandler, metrics, connections]{
        THREAD_ASSERT_IS_MAIN()
        static QRegExp regExp("%un");
        auto string = translationHandler();
        qint32 index = 0, stringIndex = 0;
        auto it = metrics.begin();
        QString resultString;
        while((index = regExp.indexIn(string, index)) != -1 && it != metrics.end()) {
            resultString.append(QStringView(string.begin() + stringIndex, string.begin() + index).toString());
            auto metricSystem = MeasurementManager::GetInstance().GetMeasurement(*it);
            resultString.append(metricSystem->CurrentUnitLabel);
            it++;
            index += regExp.matchedLength();
            stringIndex = index;
        }
        resultString.append(QStringView(string.begin() + stringIndex, string.end()).toString());

        return resultString;
    };
}

MeasurementSystem & MeasurementSystem::AddParameter(const Name& measurmentType, const MeasurementParams& param){
    Q_ASSERT(!contains(measurmentType));
    insert(measurmentType, param);
    return *this;
}

const MeasurementParams & MeasurementSystem::GetParameter(const Name& measurmentType) const
{
    auto iter = find(measurmentType);
    Q_ASSERT(iter != end());
    return iter.value();
}

MeasurementParams::MeasurementParams(const Name& measurmentType){
    const auto& system = MeasurementManager::GetInstance().GetSystem(UNIT_SYSTEM_API);
    *this = system->GetParameter(measurmentType);
}
