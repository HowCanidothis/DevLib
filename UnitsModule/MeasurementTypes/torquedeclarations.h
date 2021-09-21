#ifndef TORQUEDECLARATIONS_H
#define TORQUEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_TORQUE = "Torque";

namespace TorqueUnits
{
    static const MeasurementUnit NewtonMeters       ("NewtonMeters      ", []{return QObject::tr("newton meters");       }, []{ return QObject::tr("Nm");    }, 0.737562);
    static const MeasurementUnit KilonewtonMeters   ("KilonewtonMeters  ", []{return QObject::tr("kilonewton meters");   }, []{ return QObject::tr("kNm");   }, 737.562 );
    static const MeasurementUnit PoundForceFeet     ("PoundForceFeet    ", []{return QObject::tr("pound-force feet");    }, []{ return QObject::tr("ftlbf"); }, 1       );
    static const MeasurementUnit KilopoundForceFeet ("KilopoundForceFeet", []{return QObject::tr("kilopound-force feet");}, []{ return QObject::tr("kftlbs");}, 1000    );
}

#define MEASUREMENT_TORQUE_STRING              MEASUREMENT_STRING(MEASUREMENT_TORQUE)
#define MEASUREMENT_TORQUE_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_TORQUE)
#define MEASUREMENT_TORQUE_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_TORQUE, x)
#define MEASUREMENT_TORQUE_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_TORQUE, x)
#define MEASUREMENT_TORQUE_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_TORQUE, x)

#endif // TORQUEDECLARATIONS_H
