#ifndef FREQUENCYDECLARATIONS_H
#define FREQUENCYDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace FrequencyUnits
{
    DECLARE_MEASUREMENT_UNIT(RevolutionPerMinute);
    DECLARE_MEASUREMENT_UNIT(RevolutionPerSecond);
    DECLARE_MEASUREMENT_UNIT(StrokePerSecond);
    DECLARE_MEASUREMENT_UNIT(StrokePerMinute);
};

DECLARE_MEASUREMENT(Frequency)
DECLARE_MEASUREMENT(StrokeFrequency)

#endif // FREQUENCYDECLARATIONS_H
