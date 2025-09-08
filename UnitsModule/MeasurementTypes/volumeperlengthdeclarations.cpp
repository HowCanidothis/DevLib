#include "volumeperlengthdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace VolumePerLengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMeterPerMeter,      TR(MeasurementTr::tr("cubic meter per meter")),     TR(MeasurementTr::tr("m3/m")),      1.)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMilimeterPerMeter,  TR(MeasurementTr::tr("cubic milimeter per meter")), TR(MeasurementTr::tr("mm3/m")),     1.e-6)
    IMPLEMENT_MEASUREMENT_UNIT(BarrelPerFoot,           TR(MeasurementTr::tr("barrel per foot")),           TR(MeasurementTr::tr("bbl/ft")),    0.52161187664042)
    IMPLEMENT_MEASUREMENT_UNIT(LiterPerMeter,           TR(MeasurementTr::tr("liter per meter")),           TR(MeasurementTr::tr("L/m")),       0.001)
    IMPLEMENT_MEASUREMENT_UNIT(CubicInchPerFoot,        TR(MeasurementTr::tr("cubic meter per foot")),      TR(MeasurementTr::tr("in3/ft")),    0.000016387064 * METERS_TO_FEETS_MULTIPLIER )

}

IMPLEMENT_MEASUREMENT(LinearCapacity, TR(MeasurementTr::tr("Linear Capacity")))
IMPLEMENT_MEASUREMENT(VolumePerLength, TR(MeasurementTr::tr("Volume per Length")))
IMPLEMENT_MEASUREMENT(CasingWearVolume, TR(MeasurementTr::tr("Casing Wear Volume")))
