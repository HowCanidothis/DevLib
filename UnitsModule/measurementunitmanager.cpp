#include "measurementunitmanager.h"

#include <QUuid>

#include "MeasurementTypes/accelerationdeclarations.h"
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
#include "MeasurementTypes/powerareadeclarations.h"
#include "MeasurementTypes/consistencyfactordeclarations.h"
#include "MeasurementTypes/concentrationdeclaration.h"
#include "MeasurementTypes/voltagedeclarations.h"

static const Name UNIT_SYSTEM_API         = "API";
static const Name UNIT_SYSTEM_API_USFT    = "API USFT";
static const Name UNIT_SYSTEM_SI          = "SI";

Measurement::Measurement(const Name& key, const FTranslationHandler& label)
    : Label(label)
    , CurrentPrecision(2)
    , CurrentStep(1.0)
    , m_wrapper(::make_shared<WPSCUnitTableWrapper>())
    , m_currentUnit(nullptr)
    , m_idsDictionary(::make_scoped<FTSDictionary>())
    , m_idsCache(::make_scoped<FTSObject>(m_idsDictionary.get()))
    , m_key(key)
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

    CurrentEpsilon.ConnectFrom(CONNECTION_DEBUG_LOCATION, [this](const auto&, qint32 precision) {
        if(m_currentUnit == nullptr) {
            return 1.0;
        }
        return m_currentUnit->FromUnitToBase(epsilon(precision)) * 0.9;
    }, CurrentUnitId, CurrentPrecision);
    
    OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, Label.OnChanged,
                           CurrentUnitId.OnChanged,
                           CurrentUnitLabel.OnChanged,
                           CurrentPrecision.OnChanged,
                           CurrentStep.OnChanged);
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

double Measurement::FromUnitToBase(double currentUnit) const
{
    return m_currentUnit->FromUnitToBase(currentUnit);
}

double Measurement::FromBaseToUnit(double baseValue) const
{
    return m_currentUnit->FromBaseToUnit(baseValue);
}

QString Measurement::FromBaseToUnitUi(double value) const
{
    return LanguageSettings::DoubleToString(GetCurrentUnit()->GetBaseToUnitConverter()(value), CurrentPrecision);
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
    : m_initialized(false)
    , m_systemWrapper(::make_shared<WPSCUnitSystemTableWrapper>())
    , m_measurmentWrapper(::make_shared<WPSCUnitMeasurementTableWrapper>())
    , m_defaultSystemsCount(0)
{

}

const MeasurementPtr& MeasurementManager::GetMeasurement(const Name& name) const
{
    Q_ASSERT(m_initialized);
    static MeasurementPtr defaultResult;
    auto foundIt = m_metricMeasurements.find(name);
    if(foundIt != m_metricMeasurements.end()) {
        return foundIt.value();
    }
    return defaultResult;
}

DispatcherConnections MeasurementManager::AttachConverter(const Measurement* measurement, LocalProperty<MeasurementUnit::FTransform>* property, LocalPropertyInt* precision)
{
    DispatcherConnections result;
    result = property->ConnectFromDispatchers(CONNECTION_DEBUG_LOCATION, [measurement]{
        return [measurement](double value) { return measurement->FromBaseToUnit(value); };
    }, measurement->OnChanged);
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

Measurement& MeasurementManager::AddMeasurement(const MeasurementPtr& measurement)
{
    Q_ASSERT(!m_metricMeasurements.contains(measurement->GetName()));
    m_metricMeasurements.insert(measurement->GetName(), measurement);
    int index = m_measurmentWrapper->GetSize();
    m_measurmentWrapper->Append(measurement);
    measurement->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, index]{m_measurmentWrapper->Edit(index, [](const MeasurementPtr& ){ }); });
    return *measurement;
}

MeasurementSystem & MeasurementManager::AddSystem(const Name& name, bool defaultSys)
{
    Q_ASSERT(!m_metricSystems.contains(name));
    auto result = ::make_shared<MeasurementSystem>(name);
    if(defaultSys) {
        ++m_defaultSystemsCount;
        result->Id = name;
    }

    result->DefaultSystem = defaultSys;
    AddSystem(result);
    return *result;
}

void MeasurementManager::AddSystem(const MeasurementSystemPtr& system) {
    if(system->Id.IsNull() || m_metricSystems.contains(system->Id)) {
        system->Id = Name(QUuid::createUuid().toString());
    }
    const auto& key = system->Id;
    m_metricSystems.insert(key, system);
    m_systemWrapper->Append(system);
    
    if(CurrentMeasurementSystem.Native().IsNull()) {
        CurrentMeasurementSystem = key;
    }
}

bool MeasurementManager::RemoveSystem(const Name& systemName)
{
    auto foundIt = m_metricSystems.find(systemName);
    if(foundIt == m_metricSystems.cend()) {
        return false;
    }
    auto indexOf = m_systemWrapper->IndexOf(*foundIt);
    if(indexOf < m_defaultSystemsCount) {
        return false;
    }
    if(CurrentMeasurementSystem == systemName) {
        CurrentMeasurementSystem = Name(m_systemWrapper->At(0)->Id);
    }

    m_metricSystems.erase(foundIt);
    m_systemWrapper->Remove({indexOf});
    return true;
}

MeasurementManager& MeasurementManager::GetInstance()
{
    static MeasurementManager result;
    return result;
}

void MeasurementManager::Initialize()
{
    Q_ASSERT(!m_initialized);
    m_initialized = true;
    AddMeasurement(MeasurementAcceleration::Value)
            .AddUnit(&AccelerationUnits::MetersPerSqSec)
            .AddUnit(&AccelerationUnits::MetersPerSqMinute)
            .AddUnit(&AccelerationUnits::MetersPerSqHour)
            .AddUnit(&AccelerationUnits::KilometersPerSqHour)
            .AddUnit(&AccelerationUnits::FeetsPerSqSec)
            .AddUnit(&AccelerationUnits::FeetsPerSqMinute)
            .AddUnit(&AccelerationUnits::FeetsPerSqHour)
            .AddUnit(&AccelerationUnits::USFeetsPerSqSec)
            .AddUnit(&AccelerationUnits::USFeetsPerSqMinute)
            .AddUnit(&AccelerationUnits::USFeetsPerSqHour);

    AddMeasurement(MeasurementAngle::Value)
            .AddUnit(&AngleUnits::Degrees)
            .AddUnit(&AngleUnits::Radians);

    AddMeasurement(MeasurementDistance::Value)
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Meters);

    AddMeasurement(MeasurementFunnelViscosity::Value)
            .AddUnit(&FunnelViscosityUnits::SecondsForQuart);

    AddMeasurement(MeasurementPercents::Value)
            .AddUnit(&PercentsUnits::Partial)
            .AddUnit(&PercentsUnits::Percents);

    AddMeasurement(MeasurementDiameter::Value)
            .AddUnit(&DistanceUnits::USFeets)
            .AddUnit(&DistanceUnits::Feets)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::Miles)
            .AddUnit(&DistanceUnits::Meters)
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Kilometers);

    AddMeasurement(MeasurementMagneticField::Value)
            .AddUnit(&FieldStrengthUnits::MicroTeslas)
            .AddUnit(&FieldStrengthUnits::Gauss)
            .AddUnit(&FieldStrengthUnits::NanoTeslas);

    AddMeasurement(MeasurementDLS::Value)
            .AddUnit(&DLSUnits::Degree100USFeet)
            .AddUnit(&DLSUnits::Degree100Feet)
            .AddUnit(&DLSUnits::Degree30Meter)
            .AddUnit(&DLSUnits::RadMeter)
            .AddUnit(&DLSUnits::Rad30Meter);

    AddMeasurement(MeasurementFlowSpeed::Value)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerSecond)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerMinute)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerHour  )
            .AddUnit(&FlowSpeedUnits::CubicMetersPerDay   )
            .AddUnit(&FlowSpeedUnits::LitersPerSecond     )
            .AddUnit(&FlowSpeedUnits::LitersPerMinute     )
            .AddUnit(&FlowSpeedUnits::GallonsPerMinute    )
            .AddUnit(&FlowSpeedUnits::BarrelsPerMinute    )
            .AddUnit(&FlowSpeedUnits::CubicFeetPerSecond  );

    AddMeasurement(MeasurementThermalConductivity::Value)
            .AddUnit(&ThermalConductivityUnits::WattMeterCelsius)
            .AddUnit(&ThermalConductivityUnits::FootHourSquareFootFahrenheit);

    AddMeasurement(MeasurementSpecificHeatCapacity::Value)
            .AddUnit(&SpecificHeatCapacityUnits::JouleKilogramCelsius)
            .AddUnit(&SpecificHeatCapacityUnits::PoundFahrenheit);

    AddMeasurement(MeasurementMass::Value)
            .AddUnit(&MassUnits::Kilograms )
            .AddUnit(&MassUnits::Grams     )
            .AddUnit(&MassUnits::Tonnes    )
            .AddUnit(&MassUnits::Pounds    )
            .AddUnit(&MassUnits::Kilopounds);

    AddMeasurement(MeasurementPressure::Value)
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

    AddMeasurement(MeasurementSpeed::Value)
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

    AddMeasurement(MeasurementTorque::Value)
            .AddUnit(&TorqueUnits::NewtonMeters      )
            .AddUnit(&TorqueUnits::DecaNewtonMeters  )
            .AddUnit(&TorqueUnits::KilonewtonMeters  )
            .AddUnit(&TorqueUnits::PoundForceFeet    )
            .AddUnit(&TorqueUnits::KilopoundForceFeet)
            .AddUnit(&TorqueUnits::PoundFoot);

    AddMeasurement(MeasurementMudWeight::Value)
            .AddUnit(&DensityUnits::PoundsPerGallon      )
            .AddUnit(&DensityUnits::PoundsPerCubicFeet   )
            .AddUnit(&DensityUnits::KilogramsPerCubicMeters);

    AddMeasurement(MeasurementViscosity::Value)
            .AddUnit(&ViscosityUnits::Centipoise       )
            .AddUnit(&ViscosityUnits::MilliPascalSecond);

    AddMeasurement(MeasurementYieldPoint::Value)
            .AddUnit(&PressureUnits::Pascals)
            .AddUnit(&PressureUnits::PoundsPerSquareFeet)
            .AddUnit(&PressureUnits::PoundsPer100SquareFeet)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::KilopoundsPerSquareInch);

    AddMeasurement(MeasurementTemperature::Value)
            .AddUnit(&TemperatureUnits::Kelvin   )
            .AddUnit(&TemperatureUnits::Celsius   )
            .AddUnit(&TemperatureUnits::Fahrenheit);

    AddMeasurement(MeasurementTemperaturePerDistance::Value)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::CelsiusPer100Feet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerMeter)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPerFeet)
            .AddUnit(&TemperaturePerDistanceUnits::FahrenheitPer100Feet);

    AddMeasurement(MeasurementDensity::Value)
            .AddUnit(&DensityUnits::KilogramsPerCubicMeters)
            .AddUnit(&DensityUnits::KilogramsPerLiter)
            .AddUnit(&DensityUnits::PoundsPerGallon)
            .AddUnit(&DensityUnits::PoundsPerCubicFeet)
            .AddUnit(&DensityUnits::SpecificGravity);

    AddMeasurement(MeasurementWeightPerLength::Value)
            .AddUnit(&WeightPerLengthUnits::KilogramPerMeter)
            .AddUnit(&WeightPerLengthUnits::KilogramPerCantimeter)
            .AddUnit(&WeightPerLengthUnits::PoundPerFoot)
            .AddUnit(&WeightPerLengthUnits::PoundPerInch);

    AddMeasurement(MeasurementYoungModulus::Value)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MeasurementYieldStrength::Value)
            .AddUnit(&PressureUnits::PoundsPerSquareInch)
            .AddUnit(&PressureUnits::Kilopascals);

    AddMeasurement(MeasurementThermalExpansion::Value)
            .AddUnit(&ThermalExpansionUnits::PerDegreeFahrenheit)
            .AddUnit(&ThermalExpansionUnits::PerDegreeCelsius);

    AddMeasurement(MeasurementJetDiameter::Value)
            .AddUnit(&DistanceUnits::Milimeters)
            .AddUnit(&DistanceUnits::Centimeters)
            .AddUnit(&DistanceUnits::Inches)
            .AddUnit(&DistanceUnits::OnePerThirtyTwoInches);

    AddMeasurement(MeasurementArea::Value)
            .AddUnit(&AreaUnits::SqMeters)
            .AddUnit(&AreaUnits::SqCentimeters)
            .AddUnit(&AreaUnits::SqMilimeters)
            .AddUnit(&AreaUnits::SqFeets)
            .AddUnit(&AreaUnits::SqInches);

    AddMeasurement(MeasurementForce::Value)
            .AddUnit(&ForceUnits::Newton)
            .AddUnit(&ForceUnits::Kilonewton)
            .AddUnit(&ForceUnits::PoundsForce)
            .AddUnit(&ForceUnits::KiloGrammForce)
            .AddUnit(&ForceUnits::KiloPoundsForce);

    AddMeasurement(MeasurementMotorSpeed::Value)
        .AddUnit(&MotorSpeedUnits::RevolutionPerGallon);

    AddMeasurement(MeasurementTime::Value)
            .AddUnit(&TimeUnits::MSeconds)
            .AddUnit(&TimeUnits::Seconds)
            .AddUnit(&TimeUnits::Minutes)
            .AddUnit(&TimeUnits::Hours);

    AddMeasurement(MeasurementLinearCapacity::Value)
            .AddUnit(&VolumePerLengthUnits::CubicMeterPerMeter)
            .AddUnit(&VolumePerLengthUnits::BarrelPerFoot)
            .AddUnit(&VolumePerLengthUnits::LiterPerMeter);

    AddMeasurement(MeasurementFrequency::Value)
            .AddUnit(&FrequencyUnits::RevolutionPerMinute)
            .AddUnit(&FrequencyUnits::RevolutionPerSecond);

    AddMeasurement(MeasurementCutterDiameter::Value)
            .AddUnit(&DistanceUnits::Milimeters);

    AddMeasurement(MeasurementPower::Value)
            .AddUnit(&PowerUnits::Watt)
            .AddUnit(&PowerUnits::JoulePerSecond)
            .AddUnit(&PowerUnits::KiloWatt)
            .AddUnit(&PowerUnits::HoursePower);

    AddMeasurement(MeasurementPowerArea::Value)
            .AddUnit(&PowerAreaUnits::WattPerSqMeter)
            .AddUnit(&PowerAreaUnits::HoursePowerPerSqMeter)
            .AddUnit(&PowerAreaUnits::HoursePowerPerSqInch);

    AddMeasurement(MeasurementAnnularVelocity::Value)
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

    AddMeasurement(MeasurementROP::Value)
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

    AddMeasurement(MeasurementConcentration::Value)
            .AddUnit(&ConcentrationUnits::MilliliterPerMilliliter   )
            .AddUnit(&ConcentrationUnits::PartsPerBillion           )
            .AddUnit(&ConcentrationUnits::CubicMetersPerCubicMeters )
            .AddUnit(&ConcentrationUnits::KilogrammPerCubicMeters   )
            .AddUnit(&ConcentrationUnits::MilligrammPerLiter        )
            .AddUnit(&ConcentrationUnits::GallonPerMegagallon       );

    AddMeasurement(MeasurementConcentrationMud::Value)
            .AddUnit(&ConcentrationUnits::MilliliterPerMilliliter   )
            .AddUnit(&ConcentrationUnits::PartsPerBillion           )
            .AddUnit(&ConcentrationUnits::CubicMetersPerCubicMeters )
            .AddUnit(&ConcentrationUnits::KilogrammPerCubicMeters   )
            .AddUnit(&ConcentrationUnits::MilligrammPerLiter        )
            .AddUnit(&ConcentrationUnits::GallonPerMegagallon       );

    AddMeasurement(MeasurementConcentrationAlkalinity::Value)
            .AddUnit(&ConcentrationUnits::MilliliterPerMilliliter   )
            .AddUnit(&ConcentrationUnits::PartsPerBillion           )
            .AddUnit(&ConcentrationUnits::CubicMetersPerCubicMeters )
            .AddUnit(&ConcentrationUnits::KilogrammPerCubicMeters   )
            .AddUnit(&ConcentrationUnits::MilligrammPerLiter        )
            .AddUnit(&ConcentrationUnits::GallonPerMegagallon       );

    AddMeasurement(MeasurementVoltage::Value)
            .AddUnit(&VoltageUnits::Volt);

    AddMeasurement(MeasurementSolidDensity::Value)
            .AddUnit(&DensityUnits::KilogramsPerCubicMeters)
            .AddUnit(&DensityUnits::KilogramsPerLiter)
            .AddUnit(&DensityUnits::PoundsPerGallon)
            .AddUnit(&DensityUnits::PoundsPerCubicFeet)
            .AddUnit(&DensityUnits::SpecificGravity);

    AddMeasurement(MeasurementFiltrate::Value)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerSecond)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerMinute)
            .AddUnit(&FlowSpeedUnits::CubicMetersPerHour  )
            .AddUnit(&FlowSpeedUnits::CubicMetersPerDay   )
            .AddUnit(&FlowSpeedUnits::LitersPerSecond     )
            .AddUnit(&FlowSpeedUnits::LitersPerMinute     )
            .AddUnit(&FlowSpeedUnits::GallonsPerMinute    )
            .AddUnit(&FlowSpeedUnits::BarrelsPerMinute    )
            .AddUnit(&FlowSpeedUnits::CubicFeetPerSecond  )
            .AddUnit(&FlowSpeedUnits::MilliliterPer30Min  );


    AddMeasurement(MeasurementFlowConsistencyFactor::Value)
            .AddUnit(&FlowConsistencyFactorUnits::FactorPascals  )
            .AddUnit(&FlowConsistencyFactorUnits::FactorPoundsPerSquareFeet  );

    AddSystem(UNIT_SYSTEM_API_USFT, true)
            .AddParameter(MeasurementAcceleration::NAME,     {AccelerationUnits::FeetsPerSqSec.Id,         2})
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MeasurementSolidDensity::NAME,      {DensityUnits::SpecificGravity.Id,          2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Inches.Id,                     3, 0.125 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::USFeets.Id,                    2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::Degree100USFeet.Id,                    2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MeasurementFiltrate::NAME,         {FlowSpeedUnits::MilliliterPer30Min.Id,          2})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {DensityUnits::PoundsPerGallon.Id,            2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::USfeetPerMinute.Id,                 2})
            .AddParameter(MeasurementROP::NAME,              {SpeedUnits::USfeetPerHour.Id,                 2})
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
            .AddParameter(MeasurementPowerArea::NAME,         {PowerAreaUnits::HoursePowerPerSqInch.Id,      3})
            .AddParameter(MeasurementFlowConsistencyFactor::NAME, {FlowConsistencyFactorUnits::FactorPoundsPerSquareFeet.Id, 4})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MeasurementConcentration::NAME,       {ConcentrationUnits::MilligrammPerLiter.Id,     3})
            .AddParameter(MeasurementConcentrationMud::NAME,       {ConcentrationUnits::PartsPerBillion.Id,     3})
            .AddParameter(MeasurementConcentrationAlkalinity::NAME,       {ConcentrationUnits::MilliliterPerMilliliter.Id,     3})
            .AddParameter(MeasurementVoltage::NAME,             {VoltageUnits::Volt.Id,     3})
            .AddParameter(MeasurementThermalConductivity::NAME, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});

    AddSystem(UNIT_SYSTEM_API, true)
            .AddParameter(MeasurementAcceleration::NAME,     {AccelerationUnits::FeetsPerSqSec.Id,         2})
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqInches.Id,                       3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::PoundsPerCubicFeet.Id,          2})
            .AddParameter(MeasurementSolidDensity::NAME,      {DensityUnits::SpecificGravity.Id,          2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Inches.Id,                     2, 0.125 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::Feets.Id,                      2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::Degree100Feet.Id,                      2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::GallonsPerMinute.Id,          0})
            .AddParameter(MeasurementFiltrate::NAME,         {FlowSpeedUnits::MilliliterPer30Min.Id,          2})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::KiloPoundsForce.Id,               2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerMinute.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     2})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::BarrelPerFoot.Id,       2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::OnePerThirtyTwoInches.Id,      0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Kilopounds.Id,                     0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {DensityUnits::PoundsPerGallon.Id,            2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::PoundsPerSquareInch.Id,        0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::FeetPerMinute.Id,                   2})
            .AddParameter(MeasurementAnnularVelocity::NAME,  {SpeedUnits::FeetPerMinute.Id,               2})
            .AddParameter(MeasurementROP::NAME,              {SpeedUnits::FeetPerHour.Id,                 2})
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
            .AddParameter(MeasurementPowerArea::NAME,         {PowerAreaUnits::HoursePowerPerSqInch.Id,      3})
            .AddParameter(MeasurementFlowConsistencyFactor::NAME, {FlowConsistencyFactorUnits::FactorPoundsPerSquareFeet.Id, 4})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::PoundFahrenheit.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::FahrenheitPer100Feet.Id,     3})
            .AddParameter(MeasurementConcentration::NAME,       {ConcentrationUnits::MilligrammPerLiter.Id,     3})
            .AddParameter(MeasurementConcentrationMud::NAME,       {ConcentrationUnits::PartsPerBillion.Id,     3})
            .AddParameter(MeasurementConcentrationAlkalinity::NAME,       {ConcentrationUnits::MilliliterPerMilliliter.Id,     3})
            .AddParameter(MeasurementVoltage::NAME,             {VoltageUnits::Volt.Id,     3})
            .AddParameter(MeasurementThermalConductivity::NAME, {ThermalConductivityUnits::FootHourSquareFootFahrenheit.Id,    2});


    AddSystem(UNIT_SYSTEM_SI, true)
            .AddParameter(MeasurementAcceleration::NAME,     {AccelerationUnits::MetersPerSqSec.Id,         2})
            .AddParameter(MeasurementAngle::NAME,            {AngleUnits::Degrees.Id,                       2})
            .AddParameter(MeasurementArea::NAME,              {AreaUnits::SqCentimeters.Id,                  3})
            .AddParameter(MeasurementDensity::NAME,           {DensityUnits::KilogramsPerCubicMeters.Id,     2})
            .AddParameter(MeasurementSolidDensity::NAME,      {DensityUnits::KilogramsPerCubicMeters.Id,          2})
            .AddParameter(MeasurementDiameter::NAME,          {DistanceUnits::Milimeters.Id,                 2, 1.0 })
            .AddParameter(MeasurementDistance::NAME,         {DistanceUnits::Meters.Id,                     2})
            .AddParameter(MeasurementDLS::NAME,               {DLSUnits::Degree30Meter.Id,                     2})
            .AddParameter(MeasurementMagneticField::NAME,    {FieldStrengthUnits::NanoTeslas.Id,            1})
            .AddParameter(MeasurementFlowSpeed::NAME,        {FlowSpeedUnits::LitersPerSecond.Id,           0})
            .AddParameter(MeasurementFiltrate::NAME,         {FlowSpeedUnits::LitersPerSecond.Id,          2})
            .AddParameter(MeasurementForce::NAME,             {ForceUnits::Kilonewton.Id,                    2})
            .AddParameter(MeasurementFrequency::NAME,        {FrequencyUnits::RevolutionPerSecond.Id,       0})
            .AddParameter(MeasurementFunnelViscosity::NAME,  {FunnelViscosityUnits::SecondsForQuart.Id,     1})
            .AddParameter(MeasurementLinearCapacity::NAME,   {VolumePerLengthUnits::CubicMeterPerMeter.Id,  2})
            .AddParameter(MeasurementJetDiameter::NAME,      {DistanceUnits::Milimeters.Id,                 0})
            .AddParameter(MeasurementCutterDiameter::NAME,   {DistanceUnits::Milimeters.Id,                 2})
            .AddParameter(MeasurementMass::NAME,              {MassUnits::Tonnes.Id,                         0})
            .AddParameter(MeasurementMotorSpeed::NAME,       {MotorSpeedUnits::RevolutionPerGallon.Id,      3})
            .AddParameter(MeasurementMudWeight::NAME,        {DensityUnits::KilogramsPerCubicMeters.Id,     2})
            .AddParameter(MeasurementPercents::NAME,          {PercentsUnits::Percents.Id,                   2})
            .AddParameter(MeasurementPressure::NAME,          {PressureUnits::Bars.Id,                       0})
            .AddParameter(MeasurementSpeed::NAME,             {SpeedUnits::MetersPerSecond.Id,               2})
            .AddParameter(MeasurementAnnularVelocity::NAME,  {SpeedUnits::MetersPerSecond.Id,               2})
            .AddParameter(MeasurementROP::NAME,              {SpeedUnits::MetersPerHour.Id,                 2})
            .AddParameter(MeasurementTemperature::NAME,       {TemperatureUnits::Kelvin.Id,                 1})
            .AddParameter(MeasurementThermalExpansion::NAME, {ThermalExpansionUnits::PerDegreeCelsius.Id,   2})
            .AddParameter(MeasurementTime::NAME,              {TimeUnits::Hours.Id,                          2})
            .AddParameter(MeasurementTorque::NAME,            {TorqueUnits::KilonewtonMeters.Id,             1})
            .AddParameter(MeasurementViscosity::NAME,         {ViscosityUnits::MilliPascalSecond.Id,         2})
            .AddParameter(MeasurementWeightPerLength::NAME, {WeightPerLengthUnits::KilogramPerMeter.Id,    2})
            .AddParameter(MeasurementYieldPoint::NAME,       {PressureUnits::Pascals.Id,                    2})
            .AddParameter(MeasurementYieldStrength::NAME,    {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MeasurementYoungModulus::NAME,     {PressureUnits::Kilopascals.Id,                2})
            .AddParameter(MeasurementPower::NAME,             {PowerUnits::Watt.Id,                          3})
            .AddParameter(MeasurementPowerArea::NAME,         {PowerAreaUnits::WattPerSqMeter.Id,      3})
            .AddParameter(MeasurementFlowConsistencyFactor::NAME, {FlowConsistencyFactorUnits::FactorPascals.Id, 4})
            .AddParameter(MeasurementSpecificHeatCapacity::NAME, {SpecificHeatCapacityUnits::JouleKilogramCelsius.Id, 2})
            .AddParameter(MeasurementTemperaturePerDistance::NAME, {TemperaturePerDistanceUnits::CelsiusPerMeter.Id,  5})
            .AddParameter(MeasurementConcentration::NAME,       {ConcentrationUnits::KilogrammPerCubicMeters.Id,     3})
            .AddParameter(MeasurementConcentrationMud::NAME,       {ConcentrationUnits::KilogrammPerCubicMeters.Id,     3})
            .AddParameter(MeasurementConcentrationAlkalinity::NAME,       {ConcentrationUnits::KilogrammPerCubicMeters.Id,     3})
            .AddParameter(MeasurementVoltage::NAME,             {VoltageUnits::Volt.Id,     3})
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

            measurement->CurrentStep.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [parameters, measurement]{parameters->UnitStep = measurement->CurrentStep; }).MakeSafe(m_connections);
            measurement->CurrentUnitId.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [measurement, parameters]{parameters->UnitId = measurement->CurrentUnitId; }).MakeSafe(m_connections);
            measurement->CurrentPrecision.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [parameters, measurement]{parameters->UnitPrecision = measurement->CurrentPrecision; }).MakeSafe(m_connections);
        }
    });
}

void MeasurementManager::SetSystems(const QVector<MeasurementSystemPtr>& systems)
{
    auto& hash = m_metricSystems;
    hash.clear();
    m_systemWrapper->Change([&](WPSCUnitSystemTableWrapper::container_type& data){
        data.resize(m_defaultSystemsCount);
    });
    for(const MeasurementSystemPtr& system : ::make_const(*m_systemWrapper)) {
        hash.insert(Name(system->Label), system);
    }

    for(const MeasurementSystemPtr& system : systems) {
        auto foundIt = hash.find(Name(system->Label));
        if(foundIt != hash.end() && foundIt.value()->DefaultSystem) {
            for(auto it(foundIt.value()->begin()), e(foundIt.value()->end()); it != e; ++it) {
                auto paramIt = system->constFind(it.key());
                if(paramIt != system->cend()) {
                    it->UnitPrecision = paramIt->UnitPrecision;
                    it->UnitStep = paramIt->UnitStep;
                }
            }
        } else {
            AddSystem(system);
        }
    }
}

const MeasurementUnit* MeasurementManager::GetCurrentUnit(const Name& measurementName) const
{
    return GetMeasurement(measurementName)->GetCurrentUnit();
}

QString MeasurementManager::FromBaseToUnitUi(const Name& systemName, double value) const
{
    return GetMeasurement(systemName)->FromBaseToUnitUi(value);
}

QStringList MeasurementManager::DefaultSystems()
{
	return {UNIT_SYSTEM_API.AsString(), UNIT_SYSTEM_API_USFT.AsString(), UNIT_SYSTEM_SI.AsString()};
}

void MeasurementTranslatedString::AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics)
{
    AttachToTranslatedString(string, translationHandler, metrics, string.Connections);
}

void MeasurementTranslatedString::AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics, DispatcherConnectionsSafe& connections)
{
    QSet<const Measurement*> uniqueMetrics;
    for(const auto* metric : metrics) {
        auto foundIt = uniqueMetrics.find(metric);
        if(foundIt == uniqueMetrics.end()) {
            string.Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, metric->CurrentUnitLabel.OnChanged).MakeSafe(connections);
            uniqueMetrics.insert(metric);
        }
    }
    string.SetTranslationHandler(generateTranslationHandler(translationHandler, metrics, connections));
}

FTranslationHandler MeasurementTranslatedString::generateTranslationHandler(const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics, const DispatcherConnectionsSafe& connections)
{
	Q_ASSERT(translationHandler != nullptr);
    return [translationHandler, metrics, connections]{
        THREAD_ASSERT_IS_MAIN()
        thread_local static QRegExp regExp(MEASUREMENT_UN);
        auto string = translationHandler();
        qint32 index = 0, stringIndex = 0;
        auto it = metrics.begin();
        QString resultString;
        while((index = regExp.indexIn(string, index)) != -1 && it != metrics.end()) {
            resultString.append(QStringView(string.begin() + stringIndex, string.begin() + index).toString());
            const auto* metricSystem = *it;
            resultString.append(metricSystem->CurrentUnitLabel);
            it++;
            index += regExp.matchedLength();
            stringIndex = index;
        }
        resultString.append(QStringView(string.begin() + stringIndex, string.end()).toString());

        return resultString;
    };
}

MeasurementSystem::MeasurementSystem()
    : DefaultSystem(false)
{}

MeasurementSystem::MeasurementSystem(const Name& label)
    : DefaultSystem(false)
    , Label(label.AsString())
{}

MeasurementSystem& MeasurementSystem::AddParameter(const Name& measurmentType, const MeasurementParams& param){
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
