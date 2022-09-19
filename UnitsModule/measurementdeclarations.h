#ifndef MEASUREMENTDECLARATIONS_H
#define MEASUREMENTDECLARATIONS_H

#include <qmath.h>

#define DECLARE_MEASUREMENT_UNIT(x) extern const MeasurementUnit x;
#define IMPLEMENT_MEASUREMENT_UNIT(x, ...) const MeasurementUnit x(Name(QT_STRINGIFY(x)), __VA_ARGS__);

class Measurement;

#define DECLARE_MEASUREMENT(name) \
namespace Measurement##name { \
extern const Name NAME; \
double CurrentUnitToBase(double x); \
double BaseToCurrentUnit(double x); \
QString BaseToCurrentUnitUi(double x); \
double CurrentUnitPrecision(); \
const QString& CurrentUnitString(); \
extern const FMeasurementGetter Get; \
}

#define IMPLEMENT_MEASUREMENT(name) \
namespace Measurement##name { \
const Name NAME(QT_STRINGIFY(name)); \
double CurrentUnitToBase(double x) { return Get()->FromUnitToBase(x); } \
double BaseToCurrentUnit(double x) { return Get()->FromBaseToUnit(x); } \
QString BaseToCurrentUnitUi(double x) { return Get()->FromBaseToUnitUi(x); } \
double CurrentUnitPrecision() { return Get()->CurrentPrecision; } \
const QString& CurrentUnitString() { return Get()->CurrentUnitLabel.Native(); } \
static const FMeasurementGetter Get([]{ return MeasurementManager::GetInstance().GetMeasurement(NAME).get(); }); \
}

static constexpr double METERS_TO_FEETS_MULTIPLIER = 3.280839895;
static constexpr double USFEETS_TO_FEETS_MULTIPLIER = 1.000002;
static constexpr double SQ_USFEETS_TO_FEETS_MULTIPLIER = USFEETS_TO_FEETS_MULTIPLIER*USFEETS_TO_FEETS_MULTIPLIER;
static constexpr double SQ_METERS_TO_FEETS_MULTIPLIER = METERS_TO_FEETS_MULTIPLIER*METERS_TO_FEETS_MULTIPLIER;
static constexpr double CU_USFEETS_TO_FEETS_MULTIPLIER = USFEETS_TO_FEETS_MULTIPLIER*USFEETS_TO_FEETS_MULTIPLIER*USFEETS_TO_FEETS_MULTIPLIER;
static constexpr double CU_METERS_TO_FEETS_MULTIPLIER = METERS_TO_FEETS_MULTIPLIER*METERS_TO_FEETS_MULTIPLIER*METERS_TO_FEETS_MULTIPLIER;
static constexpr double DEGREES_TO_RADIANS = M_PI / 180.0;

class MeasurementUnit
{
public:
    using FTransform = std::function<double (double)>;
    using FTranslationHandler = std::function<QString ()>;
    MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, double multiplierUnitToBase);
    MeasurementUnit(const Name& id, const FTranslationHandler& fullLabelTrHandler, const FTranslationHandler& translationHandler, const FTransform& unitToBase, const FTransform& baseToUnit);

    double FromUnitToBaseChange(double& unitValue) const;
    double FromBaseToUnitChange(double& baseValue) const;
    double FromUnitToBase(double unitValue) const;
    double FromBaseToUnit(double baseValue) const;

    FTransform GetUnitToBaseConverter() const { return m_unitToBase; }
    FTransform GetBaseToUnitConverter() const { return m_baseToUnit; }

    const Name Id;
    mutable TranslatedString LabelFull;
    mutable TranslatedString Label;

private:
    FTransform m_unitToBase;
    FTransform m_baseToUnit;
};
Q_DECLARE_METATYPE(const MeasurementUnit*)

template<class T> class TModelsTableWrapper;
using WPSCUnitTableWrapper = TModelsTableWrapper<QVector<const MeasurementUnit*>>;
using WPSCUnitTableWrapperPtr = SharedPointer<WPSCUnitTableWrapper>;

bool LocalPropertyNotEqual(const MeasurementUnit::FTransform&, const MeasurementUnit::FTransform&);

class MeasurementProperty
{
public:
    MeasurementProperty(const Measurement* measurement);

    void Connect(LocalPropertyDouble* baseValueProperty);
    LocalPropertyDouble Value;
    LocalPropertyInt Precision;
    LocalPropertyDouble Step;

private:
    LocalPropertyDouble* m_currentValue;
    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionsSafe m_systemConnections;
    const Measurement* m_metricSystem;
};

class MeasurementValueWrapper
{
public:
    MeasurementValueWrapper();
    MeasurementValueWrapper(const Measurement* measurement, double* value, double min = std::numeric_limits<double>().lowest(), double max = std::numeric_limits<double>().max());

    void SetCurrentUnit(double value);

    SharedPointer<LocalPropertyDouble> CreateProperty() const;

    QString GetCurrentUnitUi() const;
    double GetCurrentUnit() const;
    double GetMin() const { return m_min; }
    double GetMax() const { return m_max; }

    const Measurement* GetMeasurement() const { return m_measurement; }

    MeasurementValueWrapper& operator=(double value);

    operator double() { return *m_target; }

private:
    double* m_target;
    const Measurement* m_measurement;
    double m_min;
    double m_max;
};

#endif // MEASUREMENTDECLARATIONS_H
