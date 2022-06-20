#ifndef FORCEDECLARATIONS_H
#define FORCEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_FORCE = "Force";

namespace ForceUnits
{
    static const MeasurementUnit Newton("Newton", []{return QObject::tr("newton");}, []{ return QObject::tr("N"); }, 1.0);
    static const MeasurementUnit Kilonewton("Kilonewton", []{return QObject::tr("kilonewton");}, []{ return QObject::tr("kN"); }, 1000.0);
    static const MeasurementUnit PoundsForce("PoundsForce", []{return QObject::tr("pounds-force");}, []{ return QObject::tr("kip"); }, 4.44822);
    static const MeasurementUnit KiloGrammForce("KiloGrammForce", []{return QObject::tr("kilogram-force");}, []{ return QObject::tr("kgf"); }, 9.806650029);
    static const MeasurementUnit KiloPoundsForce("KiloPoundsForce", []{return QObject::tr("1000 pounds-force");}, []{ return QObject::tr("kip"); }, 4448.22);
};

#define MEASUREMENT_FORCE_STRING              MEASUREMENT_STRING(MEASUREMENT_FORCE)
#define MEASUREMENT_FORCE_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_FORCE)
#define MEASUREMENT_FORCE_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_FORCE, x)
#define MEASUREMENT_FORCE_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_FORCE, x)
#define MEASUREMENT_FORCE_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_FORCE, x)
#define MEASUREMENT_FORCE_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_FORCE)

#endif // FORCEDECLARATIONS_H
