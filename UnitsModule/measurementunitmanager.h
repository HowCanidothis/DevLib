#ifndef MEASUREMENTUNITMANAGER_H
#define MEASUREMENTUNITMANAGER_H

#include <PropertiesModule/internal.hpp>
#include <qmath.h>

#include "measurementdeclarations.h"

class Measurement
{
public:
    Measurement(const Name& key, const FTranslationHandler& label);
    
    Measurement& AddUnit(const class MeasurementUnit* unit);
    const MeasurementUnit* FindUnit(const Name& metric) const;
    
    const Name& GetName() const { return m_key; }

    double FromUnitToBase(double currentUnit) const;
    double FromBaseToUnit(double baseValue) const;
    
    const MeasurementUnit* GetCurrentUnit() const { return m_currentUnit; }
    const WPSCUnitTableWrapperPtr& GetTableWrapper() const { return m_wrapper; }
    QString FromBaseToUnitUi(double value) const;
    const QString& CurrentUnitString() const { return CurrentUnitLabel; }
    
    TranslatedString Label;
    LocalPropertyInt CurrentPrecision;
    LocalPropertyDouble CurrentStep;
    LocalPropertyDouble CurrentEpsilon;
    
    LocalPropertyName CurrentUnitId;
    LocalPropertyString CurrentUnitLabel;

    DispatcherConnection ConnectAction(const char* location, const FAction& action) const
    {
        return OnChanged.ConnectAction(location, action);
    }

    const Measurement& Native() const { return *this; }
    
    DispatchersCommutator OnChanged;
    
private:
    friend class MeasurementManager;
    
    WPSCUnitTableWrapperPtr m_wrapper;
    QHash<Name, const MeasurementUnit*> m_metricUnits;
    
    const MeasurementUnit* m_currentUnit;
    DispatcherConnectionsSafe m_currentConnections;
    ScopedPointer<class FTSDictionary> m_idsDictionary;
    ScopedPointer<class FTSObject> m_idsCache;
    Name m_key;
};
using MeasurementPtr = SharedPointer<Measurement>;
Q_DECLARE_METATYPE(SharedPointer<Measurement>);
Q_DECLARE_METATYPE(const Measurement*);

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
    MeasurementSystem();
    MeasurementSystem(const Name& label);
    
    Name Id;
    bool DefaultSystem;
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
    
public:
    static MeasurementManager& GetInstance();
    
    void Initialize();

    void SetSystems(const QVector<MeasurementSystemPtr>& systems);
    Measurement& AddMeasurement(const MeasurementPtr& measurement);
    MeasurementSystem& AddSystem(const Name& systemName, bool defaultSys = false);
    void AddSystem(const MeasurementSystemPtr& system);
    bool RemoveSystem(const Name& systemName);
    qint32 GetDefaultSystemsCount() const { return m_defaultSystemsCount; }
    const MeasurementPtr& GetMeasurement(const Name& measurementName) const;
    const MeasurementSystemPtr& GetSystem(const Name& systemName) const;
    
    const MeasurementUnit* GetCurrentUnit(const Name& measurementName) const;

    QString FromBaseToUnitUi(const Name& measurementName, double value) const;
    
    DispatcherConnections AttachConverter(const Measurement* measurementName, LocalProperty<MeasurementUnit::FTransform>* property, LocalPropertyInt* precision = nullptr);

    static constexpr double UsFeetsToFeets(double meters) { return meters * USFEETS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToUsFeets(double feets) { return feets / USFEETS_TO_FEETS_MULTIPLIER; }

    static constexpr double MetersToFeets(double meters) { return meters * METERS_TO_FEETS_MULTIPLIER; }
    static constexpr double FeetsToMeters(double feets) { return feets / METERS_TO_FEETS_MULTIPLIER; }
    
	static constexpr double DegreeToRadian(double degree) { return degree * DEGREES_TO_RADIANS; }
	static constexpr double RadianToDegree(double radian) { return radian / DEGREES_TO_RADIANS; }
    
    LocalPropertyName CurrentMeasurementSystem;
    
	static QStringList DefaultSystems();
    const WPSCUnitSystemTableWrapperPtr& GetSystemsWrapper() const { return m_systemWrapper; }
    const WPSCUnitMeasurementTableWrapperPtr& GetMeasurementWrapper() const { return m_measurmentWrapper; }
    
private:
    bool m_initialized;
    DispatcherConnectionsSafe m_connections;
    WPSCUnitSystemTableWrapperPtr m_systemWrapper;
    WPSCUnitMeasurementTableWrapperPtr m_measurmentWrapper;
    
    QHash<Name, MeasurementSystemPtr> m_metricSystems;
    QHash<Name, MeasurementPtr> m_metricMeasurements;
    qint32 m_defaultSystemsCount;
};

class MeasurementTranslatedString
{
public:
    static void AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics);
    static void AttachToTranslatedString(TranslatedString& string, const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics, DispatcherConnectionsSafe& connections);

private:
    static FTranslationHandler generateTranslationHandler(const FTranslationHandler& translationHandler, const QVector<const Measurement*>& metrics, const DispatcherConnectionsSafe& connections);
};

using MeasurementTranslatedStringPtr = SharedPointer<MeasurementTranslatedString>;

#define MEASUREMENT_UNIT_TO_BASE(system, x) \
	MeasurementManager::GetInstance().GetCurrentUnit(system)->FromUnitToBase(x)
#define MEASUREMENT_BASE_TO_UNIT(system, x) \
    MeasurementManager::GetInstance().GetCurrentUnit(system)->FromBaseToUnit(x)
#define MEASUREMENT_PRECISION(system) \
    MeasurementManager::GetInstance().GetMeasurement(system)->CurrentPrecision
#define MEASUREMENT_BASE_TO_UNIT_UI(system, x) \
    MeasurementManager::GetInstance().FromBaseToUnitUi(system, x)
#define MEASUREMENT_STRING(system) \
	MeasurementManager::GetInstance().GetMeasurement(system)->CurrentUnitLabel
#define MEASUREMENT_DISPATCHER(system) \
    &MeasurementManager::GetInstance().GetMeasurement(system)->OnChanged
	
#define ATTACH_MEASUREMENT(system, delegate, min, max) \
    delegate->OnEditorAboutToBeShown.Connect(CONNECTION_DEBUG_LOCATION, [](QDoubleSpinBox* sp, const QModelIndex&){\
        MeasurementDoubleSpinBoxWrapper wrapper(system, sp);\
        wrapper.SetRange(min, max);\
    });

#define ATTACH_MEASUREMENT_COLUMN(tv, column, system, min, max) {\
    auto delegate = new DelegatesDoubleSpinBox(tv); \
    ATTACH_MEASUREMENT(system, delegate, min, max) \
    tv->setItemDelegateForColumn(column, delegate); }

#endif // MEASUREMENTUNITMANAGER_H
