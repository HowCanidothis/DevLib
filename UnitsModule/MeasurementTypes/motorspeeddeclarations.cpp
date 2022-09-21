#include "motorspeeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MotorSpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerGallon, []{return QObject::tr("revolution per gallon");}, []{ return QObject::tr("rev/gal"); }, 264.172052);
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerLiter, []{return QObject::tr("revolution per liter");}, []{ return QObject::tr("rev/l"); }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerCubicMeter, []{return QObject::tr("revolution per cubic meter");}, []{ return QObject::tr("rev/m3"); }, 1.0);
};

IMPLEMENT_MEASUREMENT(MotorSpeed, TR(MeasurementTr::tr("Motor Speed")))
