#include "distancedeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DistanceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Meters, []{return QObject::tr("meters");}, []{ return QObject::tr("m"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Milimeters, []{return QObject::tr("milimeters");}, []{ return QObject::tr("mm"); }, 0.001);
    IMPLEMENT_MEASUREMENT_UNIT(Centimeters, []{return QObject::tr("centimeters");}, []{ return QObject::tr("cm"); }, 0.01);
    IMPLEMENT_MEASUREMENT_UNIT(Kilometers, []{return QObject::tr("kilometers");}, []{ return QObject::tr("km"); }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(Feets, []{return QObject::tr("feet");},  []{ return QObject::tr("ft"); }, 1.0 / METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(USFeets, []{return QObject::tr("US feet");},[]{ return QObject::tr("usft"); }, 1.0 / METERS_TO_FEETS_MULTIPLIER * USFEETS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(Inches, []{return QObject::tr("inches");},[]{ return QObject::tr("in"); }, 1.0/12 / METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(Miles, []{return QObject::tr("miles");},[]{ return QObject::tr("miles"); }, 5280 / METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(OnePerThirtyTwoInches, []{return QObject::tr("1/32 inches");},[]{ return QObject::tr("1/32 in"); }, 1.0/384 / METERS_TO_FEETS_MULTIPLIER);
};

IMPLEMENT_MEASUREMENT(Distance, TR(MeasurementTr::tr("Distances")))
IMPLEMENT_MEASUREMENT(Diameter, TR(MeasurementTr::tr("Diameter")))
IMPLEMENT_MEASUREMENT(JetDiameter, TR(MeasurementTr::tr("Jet Diameter")))
IMPLEMENT_MEASUREMENT(CutterDiameter, TR(MeasurementTr::tr("Cutter Diameter")))
