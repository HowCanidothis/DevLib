#ifndef MEASUREMENTUNITMANAGER_H
#define MEASUREMENTUNITMANAGER_H

#include <PropertiesModule/internal.hpp>
#include <WidgetsModule/internal.hpp>
#include <qmath.h>

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
using WPSCUnitTableWrapper = TModelsTableWrapper<QVector<const MeasurementUnit*>>;
using WPSCUnitTableWrapperPtr = SharedPointer<WPSCUnitTableWrapper>;

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
    LocalPropertyInt CurrentPrecision;
    LocalPropertyDouble CurrentStep;
    
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
    MeasurementParams(const Name& id, int precision, double step = 1.0): UnitId(id), UnitPrecision(precision), UnitStep(step){}
    MeasurementParams(const Name& measurmentType);
    Name UnitId;
    qint32 UnitPrecision;
    double UnitStep;
    
    template<typename Buffer>
    void Serialize (Buffer& buffer) {
        buffer << UnitId;
        buffer << UnitPrecision;
        buffer << UnitStep;
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
};

using MeasurementSystemPtr = SharedPointer<MeasurementSystem>;
using WPSCUnitSystemTableWrapper = TModelsTableWrapper<QVector<MeasurementSystemPtr>>;
using WPSCUnitSystemTableWrapperPtr = SharedPointer<WPSCUnitSystemTableWrapper>;
Q_DECLARE_METATYPE(SharedPointer<MeasurementSystem>);

class MeasurementManager
{
    Q_DECLARE_TR_FUNCTIONS(MeasurementManager)
    MeasurementManager();
    friend struct Serializer<MeasurementManager>;
    
public:
    static MeasurementManager& GetInstance();
    
    Measurement& AddMeasurement(const Name& name);
    MeasurementSystem& AddSystem(const Name& name);
    void AddSystem(const MeasurementSystemPtr& system);
    const MeasurementPtr& GetMeasurement(const Name& name) const;
    const MeasurementSystemPtr& GetSystem(const Name& name) const;
    
    const MeasurementUnit* GetCurrentUnit(const Name& systemName) const;
    
    static constexpr double UsFeetsToFeets(double meters) { return meters * USFEETS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToUsFeets(double feets) { return feets / USFEETS_TO_FEETS_MULTIPLIER; }

    static constexpr double MetersToFeets(double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToMeters(double feets) { return feets / METERS_TO_FEETS_MULTIPLIER; }
    
	static constexpr double DegreeToRadian(double degree) { return degree * DEGREES_TO_RADIANS; }
	static constexpr double RadianToDegree(double radian) { return radian / DEGREES_TO_RADIANS; }
    
    LocalProperty<Name> CurrentMeasurementSystem;
    
	static QStringList DefaultSystems();
    const WPSCUnitSystemTableWrapperPtr& GetSystemsWrapper() const { return m_systemWrapper; }
    const WPSCUnitMeasurementTableWrapperPtr& GetMeasurementWrapper() const { return m_measurmentWrapper; }
    
private:
    DispatcherConnectionsSafe m_connections;
    WPSCUnitSystemTableWrapperPtr m_systemWrapper;
    WPSCUnitMeasurementTableWrapperPtr m_measurmentWrapper;
    
    QHash<Name, MeasurementSystemPtr> m_metricSystems;
    QHash<Name, MeasurementPtr> m_metricMeasurements;
};

class MeasurementProperty
{
public:
    MeasurementProperty(const Name& systemName);
    
    void Connect(LocalPropertyDouble* baseValueProperty);
    LocalPropertyDouble Value;
    LocalPropertyInt Precision;
    LocalPropertyDouble Step;
    
private:
    LocalPropertyDouble* m_currentValue;
    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionsSafe m_systemConnections;
    MeasurementPtr m_metricSystem;
};

class MeasurementTranslatedString
{
public:
    static void AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<Name>& metrics);

private:
    static FTranslationHandler generateTranslationHandler(const FTranslationHandler& translationHandler, const QVector<Name>& metrics, const DispatcherConnectionsSafe& connections);
};

using MeasurementTranslatedStringPtr = SharedPointer<MeasurementTranslatedString>;

class MeasurementDoubleSpinBoxWrapper
{
public:
    MeasurementDoubleSpinBoxWrapper(const Name& measurementType, QDoubleSpinBox* spinBox)
        : m_spinBox(spinBox)
    {
        const auto& measurement = MeasurementManager::GetInstance().GetMeasurement(measurementType);
        m_baseToUnitConverter = measurement->GetCurrentUnit()->GetBaseToUnitConverter();
        spinBox->setDecimals(measurement->CurrentPrecision);
        spinBox->setSingleStep(measurement->CurrentStep);
    }

    void SetRange(double min, double max)
    {
        m_spinBox->setRange(m_baseToUnitConverter(min),m_baseToUnitConverter(max));
    }

    void SetMinimum(double min, double offset = 0.0)
    {
        m_spinBox->setMinimum(m_baseToUnitConverter(min) + offset);
    }

    void SetMaximum(double max, double offset = 0.0)
    {
        m_spinBox->setMaximum(m_baseToUnitConverter(max) + offset);
    }

protected:
    QDoubleSpinBox* m_spinBox;
    MeasurementUnit::FTransform m_baseToUnitConverter;
};

#define MEASUREMENT_UNIT_TO_BASE(system, x) \
	MeasurementManager::GetInstance().GetCurrentUnit(system)->FromUnitToBase(x)
#define MEASUREMENT_BASE_TO_UNIT(system, x) \
    MeasurementManager::GetInstance().GetCurrentUnit(system)->FromBaseToUnit(x)
#define MEASUREMENT_PRECISION(system) \
    MeasurementManager::GetInstance().GetMeasurement(system)->CurrentPrecision
#define MEASUREMENT_BASE_TO_UNIT_UI(system, x) \
    QString::number(MEASUREMENT_BASE_TO_UNIT(system, x), 'f', MEASUREMENT_PRECISION(system))
#define MEASUREMENT_STRING(system) \
	MeasurementManager::GetInstance().GetMeasurement(system)->CurrentUnitLabel
#define MEASUREMENT_DISPATCHER(system) \
    &MeasurementManager::GetInstance().GetMeasurement(system)->OnChanged
	
#define ATTACH_MEASUREMENT(system, delegate, min, max) \
    delegate->OnEditorAboutToBeShown.Connect(this, [](QDoubleSpinBox* sp, const QModelIndex&){\
        MeasurementDoubleSpinBoxWrapper wrapper(system, sp);\
        wrapper.SetRange(min, max);\
    });

#define ATTACH_MEASUREMENT_COLUMN(tv, column, system, min, max) {\
    auto delegate = new DelegatesDoubleSpinBox(tv); \
    ATTACH_MEASUREMENT(system, delegate, min, max) \
    tv->setItemDelegateForColumn(column, delegate); }

#endif // MEASUREMENTUNITMANAGER_H
