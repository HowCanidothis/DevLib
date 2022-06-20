#ifndef FUNNELVISCOSITYDECLARATIONS_H
#define FUNNELVISCOSITYDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_FUNNEL_VISCOSITY = "FunnelViscosity";

namespace FunnelViscosityUnits
{
    static const MeasurementUnit SecondsForCubicMeter("SecondsForCubicMeter", []{return QObject::tr("seconds/cubic meter");}, []{ return QObject::tr("sec/m3"); }, 1.0);
    static const MeasurementUnit SecondsForQuart("SecondsForQuart", []{return QObject::tr("seconds/quart");}, []{ return QObject::tr("sec/qt"); }, 1.0 / 0.000946353);
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
