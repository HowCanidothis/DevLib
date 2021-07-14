#ifndef MEASUREMENTUNITMANAGER_H
#define MEASUREMENTUNITMANAGER_H

#include <PropertiesModule/internal.hpp>
#include <WidgetsModule/internal.hpp>
#include <qmath.h>

static constexpr double METERS_TO_FEETS_MULTIPLIER = 3.280839895;
static constexpr double USFEETS_TO_FEETS_MULTIPLIER = 1.000002;
static constexpr double DEGREES_TO_RADIANS = M_PI / 180.0;

static const Name MEASUREMENT_ANGLES         = "ANGLES";
static const Name MEASUREMENT_DISTANCES      = "DISTANCES";
static const Name MEASUREMENT_FIELD_STRENGTH = "FIELD_STRENGTH";
static const Name MEASUREMENT_DLS            = "DLS";

class MeasurementUnit
{
public:
    using FTransform = std::function<double (double)>;
    using FTranslationHandler = std::function<QString ()>;
    MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, double multiplierUnitToBase);
    
    double FromUnitToBase(double unitValue) const;
    double FromBaseToUnit(double baseValue) const;
    
    FTransform GetUnitToBaseConverter() const { return [this](double unit) { return FromUnitToBase(unit); }; }
    FTransform GetBaseToUnitConverter() const { return [this](double base) { return FromBaseToUnit(base); }; }
    
    const Name Id;
    mutable TranslatedString LabelFull;
    mutable TranslatedString Label;
    
private:
    double m_multiplier;
};
Q_DECLARE_METATYPE(const MeasurementUnit*)
using WPSCUnitTableWrapper = TModelsTableWrapper<QVector<const MeasurementUnit*>>;
using WPSCUnitTableWrapperPtr = SharedPointer<WPSCUnitTableWrapper>;

namespace DistanceUnits
{
    static const MeasurementUnit Meters ("Meters", []{return QObject::tr("meters");}, []{ return QObject::tr("m"); }, 3.280839895);
    static const MeasurementUnit Feets  ("Feets"  , []{return QObject::tr("feets");},  []{ return QObject::tr("ft"); }, 1.0);
    static const MeasurementUnit USFeets("Usfeets", []{return QObject::tr("usfeets");},[]{ return QObject::tr("usft"); }, USFEETS_TO_FEETS_MULTIPLIER);
};

namespace AngleUnits
{
    static const MeasurementUnit Radians("Radian", []{return QObject::tr("radians");}, []{ return QObject::tr("rad"); }, 1.0);
    static const MeasurementUnit Degrees("Degree", []{return QObject::tr("degrees");}, []{ return "°"; }, DEGREES_TO_RADIANS);
};

namespace FieldStrengthUnits
{
    static const MeasurementUnit NanoTeslas("NanoTeslas", []{return QObject::tr("nano tesla");}, []{ return QObject::tr("nT"); }, 1.0);
};

namespace DLSUnits
{
    static const MeasurementUnit RadFeet     ("RadianFeet"  , []{return QObject::tr("radians / 100 feets");}, []{ return QObject::tr("rad/100ft"); }, 1.0);
    static const MeasurementUnit RadMeter    ("RadianMeter" , []{return QObject::tr("radians / 30 meters");}, []{ return QObject::tr("rad/30m"); }, DEGREES_TO_RADIANS / (100.0 / 30.0));
    static const MeasurementUnit DegreeFeet  ("DegreeFeet"  , []{return QObject::tr("degrees / 100 feets");}, []{ return QObject::tr("В°/100ft"); }, DEGREES_TO_RADIANS);
    static const MeasurementUnit DegreeUSFeet("DegreeUSFeet", []{return QObject::tr("degrees / 100 usfeets");}, []{ return QObject::tr("В°/100usft"); }, DEGREES_TO_RADIANS / USFEETS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit DegreeMeter ("DegreeMeter" , []{return QObject::tr("degrees / 30 meters");}, []{ return QObject::tr("В°/30m"); }, DEGREES_TO_RADIANS * (100.0 / METERS_TO_FEETS_MULTIPLIER) / 30.0);
}

class Measurement
{
public:
    Measurement(const QString& label);
    
    Measurement& AddUnit(const MeasurementUnit* unit);
    const MeasurementUnit* FindUnit(const Name& metric) const;
    
    double CurrentUnitToBaseValue(double currentUnit) const;
    double BaseValueToCurrentUnit(double baseValue) const;
    
    const MeasurementUnit* GetCurrentUnit() const { return m_currentUnit; }
    const WPSCUnitTableWrapperPtr& GetTableWrapper() const { return m_wrapper; }
    
    LocalPropertyString Label;
    LocalPropertyInt Precision;
    
    LocalProperty<Name> CurrentUnitId;
    LocalPropertyString CurrentUnitLabel;
    
    DispatchersCommutator OnChanged;
    
private:
    friend class MeasurementManager;
    
    WPSCUnitTableWrapperPtr m_wrapper;
    QHash<Name, const MeasurementUnit*> m_metricUnits;
    
    const MeasurementUnit* m_currentUnit;
    DispatcherConnectionsSafe m_currentConnections;
};
using MeasurementPtr = SharedPointer<Measurement>;
Q_DECLARE_METATYPE(SharedPointer<Measurement>);
using WPSCUnitMeasurementTableWrapper = TModelsTableWrapper<QVector<MeasurementPtr>>;
using WPSCUnitMeasurementTableWrapperPtr = SharedPointer<WPSCUnitMeasurementTableWrapper>;


struct MeasurementParams
{
    MeasurementParams(){}
    MeasurementParams(const Name& id, int precision): UnitId(id), UnitPrecision(precision){}
    MeasurementParams(const Name& measurmentType);
    Name UnitId;
    qint32 UnitPrecision;
    
    template<typename Buffer>
    void Serialize (Buffer& buffer) {
        buffer << UnitId;
        buffer << UnitPrecision;
    }
};
class MeasurementSystem : public QHash<Name,MeasurementParams>
{
    using Super = QHash<Name,MeasurementParams>;
public:
    MeasurementSystem(){}
    MeasurementSystem(const Name& label) : Label(label.AsString()) {}
    
    LocalPropertyString Label;
    MeasurementSystem& AddParameter(const Name& measurmentType, const MeasurementParams& param);
    const MeasurementParams& GetParameter(const Name& measurmentType) const;
    
public:
    template<typename Buffer>
    void Serialize (Buffer& buffer) {
        buffer << Label;
        buffer << *reinterpret_cast<Super*>(this);
    }
};

using MeasurementSystemPtr = SharedPointer<MeasurementSystem>;
using WPSCUnitSystemTableWrapper = TModelsTableWrapper<QVector<MeasurementSystemPtr>>;
using WPSCUnitSystemTableWrapperPtr = SharedPointer<WPSCUnitSystemTableWrapper>;
Q_DECLARE_METATYPE(SharedPointer<MeasurementSystem>);

class MeasurementManager
{
    Q_DECLARE_TR_FUNCTIONS(MeasurementManager)
    MeasurementManager();
public:
    static MeasurementManager& GetInstance();
    
    Measurement& AddMeasurement(const Name& name);
    MeasurementSystem& AddSystem(const Name& name);
    const MeasurementPtr& GetMeasurement(const Name& name) const;
    const MeasurementSystemPtr& GetSystem(const Name& name) const;
    
    const MeasurementUnit* GetCurrentUnit(const Name& systemName) const;
    
    static constexpr double MetersToFeets(double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToMeters(double feets) { return feets / METERS_TO_FEETS_MULTIPLIER; }
    
    LocalProperty<Name> CurrentMeasurementSystem;
    
    
    const WPSCUnitSystemTableWrapperPtr& GetSystemsWrapper() const { return m_systemWrapper; }
    const WPSCUnitMeasurementTableWrapperPtr& GetMeasurementWrapper() const { return m_measurmentWrapper; }
    
private:
    DispatcherConnectionsSafe m_connections;
    WPSCUnitSystemTableWrapperPtr m_systemWrapper;
    WPSCUnitMeasurementTableWrapperPtr m_measurmentWrapper;
    
    QHash<Name, MeasurementSystemPtr> m_metricSystems;
    QHash<Name, MeasurementPtr> m_metricMeasurements;
    
public:
    template<typename Buffer>
    void Serialize (Buffer& buffer) {
        buffer << m_metricSystems;
        buffer << CurrentMeasurementSystem;
    }
};

class MeasurementProperty
{
public:
    MeasurementProperty(const Name& systemName);
    
    void Connect(LocalPropertyDouble* baseValueProperty);
    LocalPropertyDouble Value;
    
private:
    LocalPropertyDouble* m_currentValue;
    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionSafePtr m_systemConnection;
    MeasurementPtr m_metricSystem;
};

class MeasurementTranslatedString
{
public:
    static void AttachToTranslatedString(TranslatedString& string, const TranslatedString::FTranslationHandler& translationHandler, const QVector<Name>& metrics);

private:
    static TranslatedString::FTranslationHandler generateTranslationHandler(const TranslatedString::FTranslationHandler& translationHandler, const QVector<Name>& metrics, const DispatcherConnectionsSafe& connections);
};

using MeasurementTranslatedStringPtr = SharedPointer<MeasurementTranslatedString>;

#define MEASUREMENT_DISTANCE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromUnitToBase(x)
#define MEASUREMENT_DISTANCE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromBaseToUnit(x)

#define MEASUREMENT_ANGLES_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromUnitToBase(x)
#define MEASUREMENT_ANGLES_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromBaseToUnit(x)

#define MEASUREMENT_DLS_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DLS)->FromUnitToBase(x)
#define MEASUREMENT_DLS_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DLS)->FromBaseToUnit(x)

#define MEASUREMENT_DISTANCE_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DISTANCES)->CurrentUnitLabel
#define MEASUREMENT_ANGLES_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_ANGLES)->CurrentUnitLabel
#define MEASUREMENT_DLS_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DLS)->CurrentUnitLabel

#endif // MEASUREMENTUNITMANAGER_H
