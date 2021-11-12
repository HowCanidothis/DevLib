#ifndef TORQUEDECLARATIONS_H
#define TORQUEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_TORQUE = "Torque";

namespace TorqueUnits
{
    static const MeasurementUnit NewtonMeters       ("NewtonMeters", []{return QObject::tr("newton meters");       }, []{ return QObject::tr("Nm");    }, 1.0);
    static const MeasurementUnit KilonewtonMeters   ("KilonewtonMeters", []{return QObject::tr("kilonewton meters");   }, []{ return QObject::tr("kNm");   }, 1000.0 );
    static const MeasurementUnit PoundForceFeet     ("PoundForceFeet", []{return QObject::tr("pound-force feet");    }, []{ return QObject::tr("ftlbf"); }, 1.355817952003);
    static const MeasurementUnit KilopoundForceFeet ("KilopoundForceFeet", []{return QObject::tr("kilopound-force feet");}, []{ return QObject::tr("kftlbs");}, 1355.817952);
}

#define MEASUREMENT_TORQUE_STRING              MEASUREMENT_STRING(MEASUREMENT_TORQUE)
#define MEASUREMENT_TORQUE_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_TORQUE)
#define MEASUREMENT_TORQUE_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_TORQUE, x)
#define MEASUREMENT_TORQUE_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_TORQUE, x)
#define MEASUREMENT_TORQUE_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_TORQUE, x)
#define MEASUREMENT_TORQUE_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_TORQUE)
#define ATTACH_TORQUE_MEASUREMENT(delegate, min, max, step) ATTACH_MEASUREMENT(MEASUREMENT_TORQUE, delegate, min, max, step)

#endif // TORQUEDECLARATIONS_H
