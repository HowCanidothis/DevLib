#ifndef FREQUENCYDECLARATIONS_H
#define FREQUENCYDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace FrequencyUnits
{
    DECLARE_MEASUREMENT_UNIT(RevolutionPerMinute);
//    static const MeasurementUnit RevolutionPerSecond("RevolutionPerSecond", []{return QObject::tr("rps");}, []{ return QObject::tr("1/sec"); }, 1.0);
};

DECLARE_MEASUREMENT(Frequency)

#endif // FREQUENCYDECLARATIONS_H
