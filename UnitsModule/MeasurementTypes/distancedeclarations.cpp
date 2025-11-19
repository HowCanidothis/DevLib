#include "distancedeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DistanceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Meters,      TR(MeasurementTr::tr("meters")),        TR(MeasurementTr::tr("m")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Milimeters,  TR(MeasurementTr::tr("milimeters")),    TR(MeasurementTr::tr("mm")),    0.001)
    IMPLEMENT_MEASUREMENT_UNIT(Centimeters, TR(MeasurementTr::tr("centimeters")),   TR(MeasurementTr::tr("cm")),    0.01)
    IMPLEMENT_MEASUREMENT_UNIT(Kilometers,  TR(MeasurementTr::tr("kilometers")),    TR(MeasurementTr::tr("km")),    1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Feets,       TR(MeasurementTr::tr("feet")),          TR(MeasurementTr::tr("ft")),    1.0 / METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(USFeets,     TR(MeasurementTr::tr("US feet")),       TR(MeasurementTr::tr("usft")),  1.0 / METERS_TO_USFEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(Inches,      TR(MeasurementTr::tr("inches")),        TR(MeasurementTr::tr("in")),    1.0/12 / METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(Miles,       TR(MeasurementTr::tr("miles")),         TR(MeasurementTr::tr("miles")), 5280 / METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(OnePerThirtyTwoInches, TR(MeasurementTr::tr("1/32 inches")),TR(MeasurementTr::tr("1/32 in")), 1.0/384 / METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(Point, TR(MeasurementTr::tr("point")), TR(MeasurementTr::tr("pt")), 0.0003527778)

    IMPLEMENT_GLOBAL(double, DiameterPrecision, Inches.FromUnitToBase(1.e-4));
}

IMPLEMENT_MEASUREMENT(Distance, TR(MeasurementTr::tr("Distances")))
IMPLEMENT_MEASUREMENT(Diameter, TR(MeasurementTr::tr("Diameter")))
IMPLEMENT_MEASUREMENT(JetDiameter, TR(MeasurementTr::tr("Jet Diameter")))
IMPLEMENT_MEASUREMENT(CutterDiameter, TR(MeasurementTr::tr("Cutter Diameter")))
