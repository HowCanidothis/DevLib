#ifndef FUNNELVISCOSITYDECLARATIONS_H
#define FUNNELVISCOSITYDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_FUNNEL_VISCOSITY = "FunnelViscosity";

namespace FunnelViscosityUnits
{
    static const MeasurementUnit SecondsForQuart("SecondsForQuart", []{return QObject::tr("seconds/quart");}, []{ return QObject::tr("sec/qt"); }, 1.0);
};

#define MEASUREMENT_FUNNEL_VISCOSITY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FUNNEL_VISCOSITY)->FromUnitToBase(x)
#define MEASUREMENT_FUNNEL_VISCOSITY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FUNNEL_VISCOSITY)->FromBaseToUnit(x)
#define MEASUREMENT_FUNNEL_VISCOSITY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FUNNEL_VISCOSITY)->Precision
#define MEASUREMENT_FUNNEL_VISCOSITY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_FUNNEL_VISCOSITY_BASE_TO_UNIT(x), 'f', MEASUREMENT_FUNNEL_VISCOSITY_PRECISION())

#define MEASUREMENT_FUNNEL_VISCOSITY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FUNNEL_VISCOSITY)->CurrentUnitLabel

#endif // FUNNELVISCOSITYDECLARATIONS_H
