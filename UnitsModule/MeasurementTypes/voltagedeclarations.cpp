#include "voltagedeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace VoltageUnits {
    IMPLEMENT_MEASUREMENT_UNIT(Volt, []{return QObject::tr("volt");}, []{ return QObject::tr("V"); }, 1.0);
};

IMPLEMENT_MEASUREMENT(Voltage, TR(MeasurementTr::tr("Voltage")))
