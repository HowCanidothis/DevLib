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
#include "MeasurementTypes/yieldpointdeclarations.h"
#include "MeasurementTypes/temperaturedeclarations.h"
#include "MeasurementTypes/densitydeclarations.h"
#include "MeasurementTypes/weightperlengthdeclarations.h"
#include "MeasurementTypes/youngmodulusdeclarations.h"
#include "MeasurementTypes/yieldstrenghtdeclarations.h"
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

Measurement::Measurement(const QString& label)
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
    AddMeasurement(MEASUREMENT_ANGLES)
            .AddUnit(&AngleUnits::Degrees)
            .AddUnit(&AngleUnits::Radians);

    AddMeasurement(MEASUREMENT_DISTANCES)
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Meters);

    AddMeasurement(MEASUREMENT_FUNNEL_VISCOSITY)
            .AddUnit(&FunnelViscosityUnits::SecondsForQuart);

    AddMeasurement(MEASUREMENT_PERCENTS)
            .AddUnit(&PercentsUnits::Partial)
            .AddUnit(&PercentsUnits::Percents);

    AddMeasurement(MEASUREMENT_DIAMETER)
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::Miles)
            .AddUnit(&DistanceUnits::Meters)
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Kilometers);

    AddMeasurement(MEASUREMENT_FIELD_STRENGTH)
            .AddUnit(&FieldStrengthUnits::MicroTeslas)
            .AddUnit(&FieldStrengthUnits::Gauss)
            .AddUnit(&FieldStrengthUnits::NanoTeslas);

    AddMeasurement(MEASUREMENT_DLS)
            .AddUnit(&DLSUnits::DegreeUSFeet)
            .AddUnit(&DLSUnits::DegreeFeet)
            .AddUnit(&DLSUnits::DegreeMeter)
            .AddUnit(&DLSUnits::RadMeter);

    AddMeasurement(MEASUREMENT_FLOW_SPEED)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerSecond)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerMinute)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerHour  )
            .AddUnit(&FlowSpeedUnits::CubicMetersPerDay   )
            .AddUnit(&FlowSpeedUnits::LitersPerSecond     )
            .AddUnit(&FlowSpeedUnits::LitersPerMinute     )
            .AddUnit(&FlowSpeedUnits::GallonsPerMinute    )
            .AddUnit(&FlowSpeedUnits::BarrelsPerMinute    );

    AddMeasurement(MEASUREMENT_THERMAL_CONDUCTIVITY)
            .AddUnit(&ThermalConductivityUnits::WattMeterCelsius)
            .AddUnit(&ThermalConductivityUnits::FootHourSquareFootFahrenheit);

    AddMeasurement(MEASUREMENT_SPECIFIC_HEAT_CAPACITY)
            .AddUnit(&SpecificHeatCapacityUnits::JouleKilogramCelsius)
            .AddUnit(&SpecificHeatCapacityUnits::PoundFahrenheit);

    AddMeasurement(MEASUREMENT_MASS)
            .AddUnit(&MassUnits::Kilograms )
            .AddUnit(&MassUnits::Grams     )
            .AddUnit(&MassUnits::Tonnes    )
            .AddUnit(&MassUnits::Pounds    )
            .AddUnit(&MassUnits::Kilopounds);

    AddMeasurement(MEASUREMENT_PRESSURE)
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

    AddMeasurement(MEASUREMENT_SPEED)
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

    AddMeasurement(MEASUREMENT_TORQUE)
            .AddUnit(&TorqueUnits::NewtonMeters      )
            .AddUnit(&TorqueUnits::DecaNewtonMeters  )
            .AddUnit(&TorqueUnits::KilonewtonMeters  )
            .AddUnit(&TorqueUnits::PoundForceFeet    )
            .AddUnit(&TorqueUnits::KilopoundForceFeet)
            .AddUnit(&TorqueUnits::PoundFoot);

    AddMeasurement(MEASUREMENT_MUD_WEIGHT)
            .AddUnit(&MudWeightUnits::PoundPerGallon      )
            .AddUnit(&MudWeightUnits::KilogramPerCubicMeter);

    AddMeasurement(MEASUREMENT_VISCOSITY)
            .AddUnit(&ViscosityUnits::Centipoise       )
            .AddUnit(&ViscosityUnits::MilliPascalSecond);

    AddMeasurement(MEASUREMENT_YIELD_POINT)
            .AddUnit(&PressureUnits::Pascals)
            .AddUnit(&PressureUnits::PoundsPerSquareFeet)
            .AddUnit(&PressureUnits::PoundsPer100SquareFeet)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::KilopoundsPerSquareInch);

    AddMeasurement(MEASUREMENT_TEMPERATURE)
            .AddUnit(&TemperatureUnits::Celsius   )
            .AddUnit(&TemperatureUnits::Fahrenheit);

    AddMeasurement(MEASUREMENT_TEMPERATURE_PER_DISTANCE)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPer100Feet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPer100Feet);

    AddMeasurement(MEASUREMENT_DENSITY)
            .AddUnit(&DensityUnits::KilogramsPerCubicMeters)
            .AddUnit(&DensityUnits::KilogramsPerLiter)
            .AddUnit(&DensityUnits::PoundsPerGallon)
            .AddUnit(&DensityUnits::PoundsPerCubicFeet);

    AddMeasurement(MEASUREMENT_WEIGHT_PER_LENGTH)
            .AddUnit(&WeightPerLengthUnits::KilogramPerMeter)
            .AddUnit(&WeightPerLengthUnits::KilogramPerCantimeter)
            .AddUnit(&WeightPerLengthUnits::PoundPerFoot)
            .AddUnit(&WeightPerLengthUnits::PoundPerInch);

    AddMeasurement(MEASUREMENT_YOUNG_MODULUS)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MEASUREMENT_YIELD_STRENGTH)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MEASUREMENT_THERMAL_EXPANSION)
            .AddUnit(&ThermalExpansionUnits::PerDegreeFahrenheit)
            .AddUnit(&ThermalExpansionUnits::PerDegreeCelsius);

    AddMeasurement(MEASUREMENT_JET_DIAMETER)
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::OnePerThirtyTwoInches);

    AddMeasurement(MEASUREMENT_AREA)
            .AddUnit(&AreaUnits::SqMeters)
            .AddUnit(&AreaUnits::SqCentimeters)
            .AddUnit(&AreaUnits::SqMilimeters)
            .AddUnit(&AreaUnits::SqFeets)
            .AddUnit(&AreaUnits::SqInches);

    AddMeasurement(MEASUREMENT_FORCE)
            .AddUnit(&ForceUnits::Newton)
            .AddUnit(&ForceUnits::Kilonewton)
            .AddUnit(&ForceUnits::PoundsForce)
            .AddUnit(&ForceUnits::KiloGrammForce)
            .AddUnit(&ForceUnits::KiloPoundsForce);

    AddMeasurement(MEASUREMENT_MOTOR_SPEED)
        .AddUnit(&MotorSpeedUnits::RevolutionPerGallon);

    AddMeasurement(MEASUREMENT_TIME)
            .AddUnit(&TimeUnits::MSeconds)
            .AddUnit(&TimeUnits::Seconds)
            .AddUnit(&TimeUnits::Minutes)
            .AddUnit(&TimeUnits::Hours);

    AddMeasurement(MEASUREMENT_LINEAR_CAPACITY)
            .AddUnit(&VolumePerLengthUnits::CubicMeterPerMeter)
            .AddUnit(&VolumePerLengthUnits::BarrelPerFoot)
            .AddUnit(&VolumePerLengthUnits::LiterPerMeter);

    AddMeasurement(MEASUREMENT_FRENQUENCY)
            .AddUnit(&FrequencyUnits::RevolutionPerMinute);

    AddMeasurement(MEASUREMENT_CUTTER_DIAMETER)
            .AddUnit(&DistanceUnits::Milimeters);

    AddMeasurement(MEASUREMENT_POWER)
            .AddUnit(&PowerUnits::Watt)
            .AddUnit(&PowerUnits::JoulePerSecond)
            .AddUnit(&PowerUnits::KiloWatt)
            .AddUnit(&PowerUnits::HoursePower);

    AddMeasurement(MEASUREMENT_ANNULAR_VELOCITY)
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
            .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MEASUREMENT_AREA,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MEASUREMENT_DENSITY,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MEASUREMENT_DIAMETER,          {DistanceUnits::Inches.Id,                     3, 0.125 })
            .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::USFeets.Id,                    2})
            .AddParameter(MEASUREMENT_DLS,               {DLSUnits::DegreeUSFeet.Id,                    2})
            .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MEASUREMENT_FLOW_SPEED,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MEASUREMENT_FORCE,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MEASUREMENT_FRENQUENCY,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MEASUREMENT_FUNNEL_VISCOSITY,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MEASUREMENT_LINEAR_CAPACITY,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MEASUREMENT_JET_DIAMETER,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MEASUREMENT_CUTTER_DIAMETER,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MEASUREMENT_MASS,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MEASUREMENT_MOTOR_SPEED,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MEASUREMENT_MUD_WEIGHT,        {MudWeightUnits::PoundPerGallon.Id,            2})
            .AddParameter(MEASUREMENT_PERCENTS,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MEASUREMENT_PRESSURE,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MEASUREMENT_SPEED,             {SpeedUnits::USfeetPerHour.Id,                 2})
            .AddParameter(MEASUREMENT_ANNULAR_VELOCITY,  {SpeedUnits::USfeetPerMinute.Id,               2})
            .AddParameter(MEASUREMENT_TEMPERATURE,       {TemperatureUnits::Fahrenheit.Id,              1})
            .AddParameter(MEASUREMENT_THERMAL_EXPANSION, {ThermalExpansionUnits::PerDegreeFahrenheit.Id,2})
            .AddParameter(MEASUREMENT_TIME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MEASUREMENT_TORQUE,            {TorqueUnits::KilopoundForceFeet.Id,           1})
            .AddParameter(MEASUREMENT_VISCOSITY,         {ViscosityUnits::Centipoise.Id,                2})
            .AddParameter(MEASUREMENT_WEIGHT_PER_LENGTH, {WeightPerLengthUnits::PoundPerFoot.Id,        2})
            .AddParameter(MEASUREMENT_YIELD_POINT,       {PressureUnits::PoundsPer100SquareFeet.Id,     2})
            .AddParameter(MEASUREMENT_YIELD_STRENGTH,    {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MEASUREMENT_YOUNG_MODULUS,     {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MEASUREMENT_POWER,             {PowerUnits::HoursePower.Id,                   3})
            .AddParameter(MEASUREMENT_SPECIFIC_HEAT_CAPACITY, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MEASUREMENT_TEMPERATURE_PER_DISTANCE, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MEASUREMENT_THERMAL_CONDUCTIVITY, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});

    AddSystem(UNIT_SYSTEM_API)
            .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MEASUREMENT_AREA,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MEASUREMENT_DENSITY,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MEASUREMENT_DIAMETER,          {DistanceUnits::Inches.Id,                     2, 0.125 })
            .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::Feets.Id,                      2})
            .AddParameter(MEASUREMENT_DLS,               {DLSUnits::DegreeFeet.Id,                      2})
            .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MEASUREMENT_FLOW_SPEED,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MEASUREMENT_FORCE,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MEASUREMENT_FRENQUENCY,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MEASUREMENT_FUNNEL_VISCOSITY,  {FunnelViscosityUnits::SecondsForQuart.Id,     2})
            .AddParameter(MEASUREMENT_LINEAR_CAPACITY,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MEASUREMENT_JET_DIAMETER,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MEASUREMENT_CUTTER_DIAMETER,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MEASUREMENT_MASS,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MEASUREMENT_MOTOR_SPEED,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MEASUREMENT_MUD_WEIGHT,        {MudWeightUnits::PoundPerGallon.Id,            2})
            .AddParameter(MEASUREMENT_PERCENTS,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MEASUREMENT_PRESSURE,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MEASUREMENT_SPEED,             {SpeedUnits::FeetPerHour.Id,                   2})
            .AddParameter(MEASUREMENT_ANNULAR_VELOCITY,  {SpeedUnits::USfeetPerMinute.Id,               2})
            .AddParameter(MEASUREMENT_TEMPERATURE,       {TemperatureUnits::Fahrenheit.Id,              1})
            .AddParameter(MEASUREMENT_THERMAL_EXPANSION, {ThermalExpansionUnits::PerDegreeFahrenheit.Id,2})
            .AddParameter(MEASUREMENT_TIME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MEASUREMENT_TORQUE,            {TorqueUnits::KilopoundForceFeet.Id,           1})
            .AddParameter(MEASUREMENT_VISCOSITY,         {ViscosityUnits::Centipoise.Id,                2})
            .AddParameter(MEASUREMENT_WEIGHT_PER_LENGTH, {WeightPerLengthUnits::PoundPerFoot.Id,        2})
            .AddParameter(MEASUREMENT_YIELD_POINT,       {PressureUnits::PoundsPer100SquareFeet.Id,        2})
            .AddParameter(MEASUREMENT_YIELD_STRENGTH,    {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MEASUREMENT_YOUNG_MODULUS,     {PressureUnits::PoundsPerSquareInch.Id,        2})
            .AddParameter(MEASUREMENT_POWER,             {PowerUnits::HoursePower.Id,                   3})
            .AddParameter(MEASUREMENT_SPECIFIC_HEAT_CAPACITY, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MEASUREMENT_TEMPERATURE_PER_DISTANCE, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MEASUREMENT_THERMAL_CONDUCTIVITY, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});

    
    AddSystem(UNIT_SYSTEM_SI)
            .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MEASUREMENT_AREA,              {AreaUnits::SqCentimeters.Id,                  3})
            .AddParameter(MEASUREMENT_DENSITY,           {DensityUnits::KilogramsPerCubicMeters.Id,     2})
            .AddParameter(MEASUREMENT_DIAMETER,          {DistanceUnits::Milimeters.Id,                 2, 1.0 })
            .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::Meters.Id,                     2})
            .AddParameter(MEASUREMENT_DLS,               {DLSUnits::DegreeMeter.Id,                     2})
            .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MEASUREMENT_FLOW_SPEED,        {FlowSpeedUnits::LitersPerSecond.Id,           0})
            .AddParameter(MEASUREMENT_FORCE,             {ForceUnits::Kilonewton.Id,                    2})
            .AddParameter(MEASUREMENT_FRENQUENCY,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MEASUREMENT_FUNNEL_VISCOSITY,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MEASUREMENT_LINEAR_CAPACITY,   {VolumePerLengthUnits::CubicMeterPerMeter.Id,  2})
            .AddParameter(MEASUREMENT_JET_DIAMETER,      {DistanceUnits::Milimeters.Id,                 0})
            .AddParameter(MEASUREMENT_CUTTER_DIAMETER,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MEASUREMENT_MASS,              {MassUnits::Tonnes.Id,                         0})
            .AddParameter(MEASUREMENT_MOTOR_SPEED,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MEASUREMENT_MUD_WEIGHT,        {MudWeightUnits::KilogramPerCubicMeter.Id,     2})
            .AddParameter(MEASUREMENT_PERCENTS,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MEASUREMENT_PRESSURE,          {PressureUnits::Bars.Id,                       0})
            .AddParameter(MEASUREMENT_SPEED,             {SpeedUnits::MetersPerHour.Id,                 2})
            .AddParameter(MEASUREMENT_ANNULAR_VELOCITY,  {SpeedUnits::MetersPerSecond.Id,               2})
            .AddParameter(MEASUREMENT_TEMPERATURE,       {TemperatureUnits::Celsius.Id,                 1})
            .AddParameter(MEASUREMENT_THERMAL_EXPANSION, {ThermalExpansionUnits::PerDegreeCelsius.Id,   2})
            .AddParameter(MEASUREMENT_TIME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MEASUREMENT_TORQUE,            {TorqueUnits::KilonewtonMeters.Id,             1})
            .AddParameter(MEASUREMENT_VISCOSITY,         {ViscosityUnits::MilliPascalSecond.Id,         2})
            .AddParameter(MEASUREMENT_WEIGHT_PER_LENGTH, {WeightPerLengthUnits::KilogramPerMeter.Id,    2})
            .AddParameter(MEASUREMENT_YIELD_POINT,       {PressureUnits::Pascals.Id,                    2})
            .AddParameter(MEASUREMENT_YIELD_STRENGTH,    {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MEASUREMENT_YOUNG_MODULUS,     {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MEASUREMENT_POWER,             {PowerUnits::Watt.Id,                          3})
            .AddParameter(MEASUREMENT_SPECIFIC_HEAT_CAPACITY, {SpecificHeatCapacityUnits::JouleKilogramCelsius.Id, 2})
            .AddParameter(MEASUREMENT_TEMPERATURE_PER_DISTANCE, {TemperaturePerDistanceUnits::CelsiusPerMeter.Id,  5})
            .AddParameter(MEASUREMENT_THERMAL_CONDUCTIVITY, {ThermalConductivityUnits::WattMeterCelsius.Id,    2});

		
    
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

Measurement& MeasurementManager::AddMeasurement(const Name &name)
{
    Q_ASSERT(!m_metricMeasurements.contains(name));
    auto result = ::make_shared<Measurement>(name.AsString());
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

bool LocalPropertyNotEqual(const MeasurementUnit::FTransform&, const MeasurementUnit::FTransform&)
{
    return true;
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
