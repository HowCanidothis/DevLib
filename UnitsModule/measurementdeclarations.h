#ifndef MEASUREMENTDECLARATIONS_H
#define MEASUREMENTDECLARATIONS_H

#include <qmath.h>

#define DECLARE_MEASUREMENT_UNIT(x) extern const MeasurementUnit x;
#define IMPLEMENT_MEASUREMENT_UNIT(x, ...) const MeasurementUnit x(Name(QT_STRINGIFY(x)), __VA_ARGS__);

#define DECLARE_MEASUREMENT(name) \
namespace Measurement##name { \
extern const Name NAME; \
double CurrentUnitToBase(double x); \
double BaseToCurrentUnit(double x); \
QString BaseToCurrentUnitUi(double x); \
double CurrentUnitPrecision(); \
const QString& CurrentUnitString(); \
}

#define IMPLEMENT_MEASUREMENT(name) \
namespace Measurement##name { \
const Name NAME(QT_STRINGIFY(name)); \
double CurrentUnitToBase(double x) { return MeasurementManager::GetInstance().GetCurrentUnit(NAME)->FromUnitToBase(x); } \
double BaseToCurrentUnit(double x) { return MeasurementManager::GetInstance().GetCurrentUnit(NAME)->FromBaseToUnit(x); } \
QString BaseToCurrentUnitUi(double x) { return QString::number(BaseToCurrentUnit(x), 'f', CurrentUnitPrecision()); } \
double CurrentUnitPrecision() { return MeasurementManager::GetInstance().GetMeasurement(NAME)->CurrentPrecision; } \
const QString& CurrentUnitString() { return MeasurementManager::GetInstance().GetMeasurement(NAME)->CurrentUnitLabel.Native(); } \
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


#endif // MEASUREMENTDECLARATIONS_H
