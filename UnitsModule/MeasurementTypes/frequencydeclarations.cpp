#include "frequencydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrequencyUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerMinute, []{return QObject::tr("rpm");}, []{ return QObject::tr("rpm"); }, 1./60);
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerSecond, []{return QObject::tr("rps");}, []{ return QObject::tr("1/sec"); }, 1.0);
//    static const MeasurementUnit RevolutionPerSecond("RevolutionPerSecond", []{return QObject::tr("rps");}, []{ return QObject::tr("1/sec"); }, 1.0);
};

IMPLEMENT_MEASUREMENT(Frequency, TR(MeasurementTr::tr("Frequency")))
