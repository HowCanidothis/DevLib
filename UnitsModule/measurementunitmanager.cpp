#include "measurementunitmanager.h"

#include <qmath.h>

static const Name UNIT_SYSTEM_API         = "API";
static const Name UNIT_SYSTEM_API_USFT    = "API USFT";
static const Name UNIT_SYSTEM_SI          = "SI";

MeasurementUnit::MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, double multiplierUnitToBase)
    : Id(id)
      , LabelFull(fullLabelTrHandler)
      , Label(translationHandler)
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

Measurement::Measurement(const QString& label)
    : Label(label)
      , Precision(2)
      , m_wrapper(::make_shared<WPSCUnitTableWrapper>())
      , m_currentUnit(nullptr)
{
    CurrentUnitId.Subscribe([this]{
        m_currentConnections.clear();
        Q_ASSERT(m_metricUnits.contains(CurrentUnitId));
        m_currentUnit = m_metricUnits[CurrentUnitId];
        CurrentUnitLabel.ConnectFrom(m_currentUnit->Label).MakeSafe(m_currentConnections);
    });
    
    OnChanged.Subscribe({&Label.OnChange,
                           &CurrentUnitId.OnChange,
                           &CurrentUnitLabel.OnChange,
                           &Precision.OnChange});
}

Measurement& Measurement::AddUnit(const MeasurementUnit* unit)
{
    Q_ASSERT(!m_metricUnits.contains(unit->Id));
    
    if(m_currentUnit == nullptr) {
        m_currentUnit = unit;
        CurrentUnitLabel.ConnectFrom(unit->Label).MakeSafe(m_currentConnections);
    }
    
    m_metricUnits.insert(unit->Id, unit);
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
    
    AddMeasurement(MEASUREMENT_FIELD_STRENGTH)
        .AddUnit(&FieldStrengthUnits::NanoTeslas);
    
    AddMeasurement(MEASUREMENT_DLS)
        .AddUnit(&DLSUnits::DegreeUSFeet)
        .AddUnit(&DLSUnits::DegreeFeet)
        .AddUnit(&DLSUnits::DegreeMeter)
        .AddUnit(&DLSUnits::RadMeter);
    
    AddSystem(UNIT_SYSTEM_API)
        .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Degrees.Id,            2})
        .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::Feets.Id,           2})
        .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id, 2})
        .AddParameter(MEASUREMENT_DLS,               {DLSUnits::DegreeFeet.Id,           2});
    
    AddSystem(UNIT_SYSTEM_API_USFT)
        .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Degrees.Id,            2})
        .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::USFeets.Id,         2})
        .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id, 2})
        .AddParameter(MEASUREMENT_DLS,               {DLSUnits::DegreeUSFeet.Id,         2});
    
    AddSystem(UNIT_SYSTEM_SI)
        .AddParameter(MEASUREMENT_ANGLES,            {AngleUnits::Radians.Id,           2})
        .AddParameter(MEASUREMENT_DISTANCES,         {DistanceUnits::Meters.Id,         2})
        .AddParameter(MEASUREMENT_FIELD_STRENGTH,    {FieldStrengthUnits::NanoTeslas.Id,2})
        .AddParameter(MEASUREMENT_DLS,               {DLSUnits::RadMeter.Id,            2});
    
    CurrentMeasurementSystem.SetAndSubscribe([this]{
        const auto& system = GetSystem(CurrentMeasurementSystem);
        Q_ASSERT(system != nullptr);
        
        m_connections.clear();
        for(auto iter = system->begin(); iter != system->end(); ++iter){
            const auto& measurement = GetMeasurement(iter.key());
            auto* parameters = &iter.value();
            Q_ASSERT(measurement != nullptr);
            measurement->CurrentUnitId = parameters->UnitId;
            measurement->Precision = parameters->UnitPrecision;
            
            measurement->CurrentUnitId.OnChange.Connect(this, [measurement, parameters]{parameters->UnitId = measurement->CurrentUnitId; }).MakeSafe(m_connections);
            measurement->Precision.OnChange.Connect(this, [parameters, measurement]{parameters->UnitPrecision = measurement->Precision; }).MakeSafe(m_connections);
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

const MeasurementSystemPtr& MeasurementManager::GetSystem(const Name & name) const
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
    result->OnChanged.Connect(this, [this, index]{m_measurmentWrapper->Edit(index, [](const MeasurementPtr& p){ qDebug() << "Change " << p->Label; }); });
    return *result;
}

MeasurementSystem & MeasurementManager::AddSystem(const Name& name)
{
    Q_ASSERT(!m_metricSystems.contains(name));
    auto result = ::make_shared<MeasurementSystem>(name);
    m_metricSystems.insert(name, result);
    m_systemWrapper->Append(result);
    
    if(CurrentMeasurementSystem.Native().IsNull()) {
        CurrentMeasurementSystem = name;
    }
    
    return *result;
}

MeasurementManager& MeasurementManager::GetInstance()
{
    static MeasurementManager result;
    return result;
}

const MeasurementUnit* MeasurementManager::GetCurrentUnit(const Name& systemName) const
{
    Q_ASSERT(m_metricMeasurements.contains(systemName));
    return m_metricMeasurements[systemName]->GetCurrentUnit();
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
    m_systemConnection = m_metricSystem->OnChanged.Connect(this, [this]{
                                                      Connect(m_currentValue);
                                                  }).MakeSafe();
}

void MeasurementTranslatedString::AttachToTranslatedString(TranslatedString& string, const TranslatedString::FTranslationHandler& translationHandler, const QVector<Name>& metrics)
{
    DispatcherConnectionsSafe connections;
    QSet<Name> uniqueMetrics;
    for(const auto& metric : metrics) {
        auto foundIt = uniqueMetrics.find(metric);
        if(foundIt == uniqueMetrics.end()) {
            string.Retranslate.ConnectFrom(MeasurementManager::GetInstance().GetMeasurement(metric)->CurrentUnitLabel.OnChange).MakeSafe(connections);
            uniqueMetrics.insert(metric);
        }
    }
    string.SetTranslationHandler(generateTranslationHandler(translationHandler, metrics, connections));
}

TranslatedString::FTranslationHandler MeasurementTranslatedString::generateTranslationHandler(const TranslatedString::FTranslationHandler& translationHandler, const QVector<Name>& metrics, const DispatcherConnectionsSafe& connections)
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

MeasurementSystem & MeasurementSystem::AddParameter(const Name & measurmentType, const MeasurementParams & param){
    Q_ASSERT(!contains(measurmentType));
    insert(measurmentType, param);
    return *this;
}

const MeasurementParams & MeasurementSystem::GetParameter(const Name & measurmentType) const
{
    auto iter = find(measurmentType);
    Q_ASSERT(iter != end());
    return iter.value();
}

MeasurementParams::MeasurementParams(const Name & measurmentType){
    const auto& system = MeasurementManager::GetInstance().GetSystem(UNIT_SYSTEM_API);
    *this = system->GetParameter(measurmentType);
}
