#ifndef METRICUNITMANAGER_H
#define METRICUNITMANAGER_H

#include <PropertiesModule/internal.hpp>

static const Name METRIC_UNIT_RADIAN = "RADIAN";
static const Name METRIC_UNIT_DEGREE = "DEGREE";
static const Name METRIC_UNIT_METER = "METER";
static const Name METRIC_UNIT_USFT = "USFOOT";
static const Name METRIC_UNIT_FT = "FOOT";

static constexpr double METERS_TO_FEETS_MULTIPLIER = 3.280839895;

struct DistanceUnits
{
    const TranslatedString* Meters = new TranslatedString([]{ return QObject::tr("m"); });
    const TranslatedString* Feets = new TranslatedString([]{ return QObject::tr("ft"); });
    const TranslatedString* USFeets = new TranslatedString([]{ return QObject::tr("usft"); });
};

struct AngleUnits
{
    const TranslatedString* Radians = new TranslatedString([]{ return QObject::tr("rad"); });
    const TranslatedString* Degrees = new TranslatedString([]{ return "°"; });
};

struct MetricUnit
{
    using FTransform = std::function<double (double)>;
    const TranslatedString* MetricName;
    FTransform UnitToBaseValue;
    FTransform BaseValueToUnit;
};

using MetricUnitPtr = SharedPointer<MetricUnit>;

class MetricSystem
{
public:
    MetricSystem();

    void SetCurrent(const Name& id);
    MetricSystem& AddMetricUnit(const Name& id, const TranslatedString* label, const MetricUnit::FTransform& unitToBaseValue, const MetricUnit::FTransform& baseValueToUnit);
    const MetricUnitPtr& GetMetricUnit(const Name& metric) const;

    const MetricUnit::FTransform& GetCurrentUnitToBaseValueConverter() const;
    const MetricUnit::FTransform& GetCurrentBaseValueToUnitConverter() const;

    LocalPropertyString CurrentLabel;

    Dispatcher OnCurrentChanged;

private:
    friend class MetricUnitManager;
    const MetricUnit* getCurrentMetricUnit() const;

    QHash<Name, MetricUnitPtr> m_metricUnits;
    MetricUnit* m_currentUnit;
};

using MetricSystemPtr = SharedPointer<MetricSystem>;

static const Name METRIC_SYSTEM_ANGLES = "ANGLES";
static const Name METRIC_SYSTEM_DISTANCES = "DISTANCES";

class MetricUnitManager
{
    Q_DECLARE_TR_FUNCTIONS(MetricUnitManager)
    MetricUnitManager();
public:
    const MetricSystemPtr& GetMetricSystem(const Name& name) const;
    static MetricUnitManager& GetInstance();

    const MetricUnit* GetCurrentUnit(const Name& systemName) const;

    static constexpr double MetersToFeets(double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToMeters(double feets) { return feets / METERS_TO_FEETS_MULTIPLIER; }

private:
    MetricSystem& addMetricSystem(const Name& name);

private:
    QHash<Name, MetricSystemPtr> m_metricSystems;
    DistanceUnits m_distances;
    AngleUnits m_angles;
};

class MetricProperty
{
public:
    MetricProperty(const Name& systemName);

    void Connect(LocalPropertyDouble* baseValueProperty);
    LocalPropertyDouble Value;

private:
    LocalPropertyDouble* m_currentValue;
    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionSafePtr m_systemConnection;
    MetricSystemPtr m_metricSystem;
};

class MetricTranslatedString : public TranslatedString
{
    using Super = TranslatedString;
public:
    MetricTranslatedString(const std::function<QString ()>& translationHandler, const QVector<Name>& metrics);
};

#define METRIC_DISTANCE_UNIT_TO_BASE(x) \
    MetricUnitManager::GetInstance().GetCurrentUnit(METRIC_SYSTEM_DISTANCES)->UnitToBaseValue(x)
#define METRIC_DISTANCE_BASE_TO_UNIT(x) \
    MetricUnitManager::GetInstance().GetCurrentUnit(METRIC_SYSTEM_DISTANCES)->BaseValueToUnit(x)

#define METRIC_ANGLES_UNIT_TO_BASE(x) \
    MetricUnitManager::GetInstance().GetCurrentUnit(METRIC_SYSTEM_ANGLES)->UnitToBaseValue(x)
#define METRIC_ANGLES_BASE_TO_UNIT(x) \
    MetricUnitManager::GetInstance().GetCurrentUnit(METRIC_SYSTEM_ANGLES)->BaseValueToUnit(x)

#endif // METRICUNITMANAGER_H
