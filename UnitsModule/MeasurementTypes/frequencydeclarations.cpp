#include "frequencydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrequencyUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerMinute, TR(MeasurementTr::tr("rpm")), TR(MeasurementTr::tr("rpm")), 1./60)
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerSecond, TR(MeasurementTr::tr("rps")), TR(MeasurementTr::tr("1/sec")), 1.0)
//    static const MeasurementUnit RevolutionPerSecond("RevolutionPerSecond", TR(MeasurementTr::tr("rps")), TR(MeasurementTr::tr("1/sec")), 1.0);
}

IMPLEMENT_MEASUREMENT(Frequency, TR(MeasurementTr::tr("Frequency")))
