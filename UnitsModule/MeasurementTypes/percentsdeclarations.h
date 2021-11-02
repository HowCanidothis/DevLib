#ifndef PERCENTSDECLARATIONS_H
#define PERCENTSDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_PERCENTS = "Percents";

namespace PercentsUnits
{
    static const MeasurementUnit Partial  ("Partial", []{return QObject::tr("partial"); }, []{ return QObject::tr("");  }, 1.0  );
    static const MeasurementUnit Percents ("Percents", []{return QObject::tr("percents");     }, []{ return "%";   }, 100.0  );
}

#define MEASUREMENT_PERCENTS_STRING              MEASUREMENT_STRING(MEASUREMENT_PERCENTS)
#define MEASUREMENT_PERCENTS_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_PERCENTS)
#define MEASUREMENT_PERCENTS_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_PERCENTS, x)
#define MEASUREMENT_PERCENTS_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_PERCENTS, x)
#define MEASUREMENT_PERCENTS_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_PERCENTS, x)
#define MEASUREMENT_PERCENTS_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_PERCENTS)
#define ATTACH_PERCENTS_MEASUREMENT(delegate, min, max, step) ATTACH_MEASUREMENT(MEASUREMENT_PERCENTS, delegate, min, max, step)


#endif // PERCENTSDECLARATIONS_H
