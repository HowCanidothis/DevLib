#include "areadeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace AreaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SqMeters, []{return QObject::tr("square meters");}, []{ return QObject::tr("m2"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(SqCentimeters, []{return QObject::tr("square centimeters");}, []{ return QObject::tr("cm2"); }, 0.0001);
    IMPLEMENT_MEASUREMENT_UNIT(SqMilimeters, []{return QObject::tr("square milimeters");}, []{ return QObject::tr("mm2"); }, 0.000001);
    IMPLEMENT_MEASUREMENT_UNIT(SqFeets, []{return QObject::tr("square feet");}, []{ return QObject::tr("ft2"); }, 1.0 / SQ_METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(SqUSFeets, []{return QObject::tr("square US feet");}, []{ return QObject::tr("usft2"); }, SQ_USFEETS_TO_FEETS_MULTIPLIER / SQ_METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(SqInches, []{return QObject::tr("square inches");}, []{ return QObject::tr("in2"); }, 1.0 / 144.0 / SQ_METERS_TO_FEETS_MULTIPLIER);
};

IMPLEMENT_MEASUREMENT(Area)
