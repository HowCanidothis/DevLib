#ifndef METRICUNITMANAGER_H
#define METRICUNITMANAGER_H

#include <PropertiesModule/internal.hpp>

#include <qmath.h>

static constexpr double METERS_TO_FEETS_MULTIPLIER = 3.280839895;

class MeasurementUnit
{
public:
    using FTransform = std::function<double (double)>;
    using FTranslationHandler = std::function<QString ()>;
    MeasurementUnit(const FTranslationHandler& translationHandler, double multiplierUnitToBase);

    double FromUnitToBase(double unitValue) const;
    double FromBaseToUnit(double baseValue) const;

    FTransform GetUnitToBaseConverter() const { return [this](double unit) { return FromUnitToBase(unit); }; }
    FTransform GetBaseToUnitConverter() const { return [this](double base) { return FromBaseToUnit(base); }; }

    mutable TranslatedString Label;
private:
    double m_multiplier;
};

namespace DistanceUnits
{
    static const MeasurementUnit Meters([]{ return QObject::tr("m"); }, 3.280839895);
    static const MeasurementUnit Feets([]{ return QObject::tr("ft"); }, 1.0);
    static const MeasurementUnit USFeets([]{ return QObject::tr("usft"); }, 1.000002);
};

namespace AngleUnits
{
    static const MeasurementUnit Radians([]{ return QObject::tr("rad"); }, 1.0);
    static const MeasurementUnit Degrees([]{ return "°"; }, M_PI / 180.0);
};

namespace FieldStrengthUnits
{
    static const MeasurementUnit NanoTeslas([]{ return QObject::tr("nT"); }, 1.0);
};

class Measurement
{
public:
    Measurement();

    void SetCurrent(const Name& id);
    Measurement& AddUnit(const Name& id, const MeasurementUnit* unit);
    const MeasurementUnit* FindUnit(const Name& metric) const;

    double CurrentUnitToBaseValue(double currentUnit) const;
    double BaseValueToCurrentUnit(double baseValue) const;

    LocalPropertyString CurrentLabel;
    LocalPropertyInt CurrentPrecision;

    Dispatcher OnCurrentChanged;

private:
    friend class MeasurementManager;
    const MeasurementUnit* getCurrentMeasurementUnit() const;

    QHash<Name, const MeasurementUnit*> m_metricUnits;
    const MeasurementUnit* m_currentUnit;
    DispatcherConnectionsSafe m_currentConnections;
};

using MeasurementPtr = SharedPointer<Measurement>;

static const Name MEASUREMENT_ANGLES = "ANGLES";
static const Name MEASUREMENT_DISTANCES = "DISTANCES";
static const Name MEASUREMENT_FIELD_STRENGTH = "FIELD_STRENGTH";

struct MeasurementParams
{
    Name Measurement;
    qint32 precision;
};

using MeasurementSystem = QHash<Name,MeasurementParams>;

class MeasurementManager
{
    Q_DECLARE_TR_FUNCTIONS(MeasurementManager)
    MeasurementManager();
public:

    Measurement& AddMeasurement(const Name& name);
    const MeasurementPtr& GetMeasurement(const Name& name) const;
    static MeasurementManager& GetInstance();

    const MeasurementUnit* GetCurrentUnit(const Name& systemName) const;

    static constexpr double MetersToFeets(double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToMeters(double feets) { return feets / METERS_TO_FEETS_MULTIPLIER; }

private:
    QHash<Name, MeasurementPtr> m_metricSystems;
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

class MeasurementTranslatedString : public TranslatedString
{
    using Super = TranslatedString;
public:
    MeasurementTranslatedString(const std::function<QString ()>& translationHandler, const QVector<Name>& metrics);
};

#define METRIC_DISTANCE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromUnitToBase(x)
#define METRIC_DISTANCE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromBaseToUnit(x)

#define METRIC_ANGLES_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromUnitToBase(x)
#define METRIC_ANGLES_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromBaseToUnit(x)

#endif // METRICUNITMANAGER_H