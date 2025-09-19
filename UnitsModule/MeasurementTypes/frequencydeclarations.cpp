#include "frequencydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrequencyUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerMinute, TR(MeasurementTr::tr("rpm")), TR(MeasurementTr::tr("rpm")), 1./60)
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerSecond, TR(MeasurementTr::tr("rps")), TR(MeasurementTr::tr("1/sec")), 1.0)

    IMPLEMENT_MEASUREMENT_UNIT(StrokePerSecond, TR(MeasurementTr::tr("sps")), TR(MeasurementTr::tr("1/sec")), 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(StrokePerMinute, TR(MeasurementTr::tr("spm")), TR(MeasurementTr::tr("spm")), 1.0 / 60);
}

IMPLEMENT_MEASUREMENT(Frequency, TR(MeasurementTr::tr("Frequency")))
IMPLEMENT_MEASUREMENT(StrokeFrequency, TR(MeasurementTr::tr("Stroke Frequency")))
